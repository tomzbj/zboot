import serial
import time
#import threading
#import random
import zlib
import os
import sys
import bincopy 
from tkinter.filedialog import *

VIEWCFG = False
#VIEWCFG = True 


class Cli: 

    def __init__(self, ser):
        self.ser = ser

    def CalcLRC(self, msg):
        # 计算LRC校验和
        # LRC为除LRC外所有字节之和取1补码; 带LRC的字节串按字节求和后低字节为零
        lrc = 0
        for c in msg:
            lrc += c
        lrc = lrc % 256    
        lrc = 256 - lrc
        if lrc == 256: 
            lrc = 0
        return lrc 
   
    def GetMaxAppSize(self):
        # 0x80 读取APP可写入的区域大小 
        cmd = b'\x80\x00\x80'
        ret = self.SendCmd(cmd)
        return int.from_bytes(ret, byteorder='little') 
    
    def GetAppSize(self):
        # 0x82 读取APP区域实际大小, 向上取整到2字节
        cmd = b'\x82\x00\x7e'
        ret = self.SendCmd(cmd)
        return int.from_bytes(ret, byteorder='little') 
    
    def SendCmd(self, cmd):
        if VIEWCFG == True:
            print("CMD: ", end="")
            count = 0
            for i in cmd:
                print("%02x " % i, end="")
                count += 1
                if count > 16:
                    break
            print()
        self.ser.write(cmd)
        timeout = time.time() + 1.5
        while self.ser.in_waiting == 0 and time.time() < timeout:
            time.sleep(0.02)
        time.sleep(0.02)
        ret = self.ser.read_all()
        if VIEWCFG == True:
            print("RET: ", ret)
        return ret

    def ReadImage(self, bin_size):
        # 0x81 读取APP区域 81 00 size(4字节) pos(4字节) 校验(1字节) 
        size = 256
        pos = 0
        ret = b''
        count = 0
        while bin_size > 0:
            if bin_size < size:
                size = bin_size
            msg = b'\x81\x00' + size.to_bytes(4, byteorder='little')
            msg += pos.to_bytes(4, byteorder='little') 
            msg += self.CalcLRC(msg).to_bytes(1, byteorder='little') 
            ret += self.SendCmd(msg)
            bin_size -= size
            pos += size
            count += 1
            print(count)
        return ret

    def EraseImage(self, size): 
        # 0xc0 擦除APP区域
        msg = b'\xc0\x00'
        msg += self.CalcLRC(msg).to_bytes(1, byteorder='little') 
        ret = self.SendCmd(msg)
        return ret

    def ReadCRC32(self, size): 
        # 0x83 计算APP区域CRC32校验值
        msg = b'\x83\x00'
        msg += self.CalcLRC(msg).to_bytes(1, byteorder='little')
        while True:
            ret = self.SendCmd(msg)
            if ret != b'':
                break
        return ret

    def WriteImage(self, img):
        # 0xc1 写入APP区域 c1 00 size(2字节) pos(4字节) content(最多1024字节)  校验(1字节) (需要预先擦除) 
        size = 512
        count = 0
        pos = 0

        totalsize = len(img)
        while len(img) % 4 != 0:
            img += b'\xff'
        while len(img) > 0:  # transfer 
            sys.stdout.flush()
            perc = (1 - len(img) / totalsize) * 100
            sys.stdout.write('%.0f%%       ' % perc)
            sys.stdout.flush()
            sys.stdout.write('\r')
            sys.stdout.flush()
            pb = img[0:size]
            msg = b'\xc1\x00' + len(pb).to_bytes(2, byteorder='little') + pos.to_bytes(4, byteorder='little') + pb
            msg += self.CalcLRC(msg).to_bytes(1, byteorder='little')
            count += 1
            if pb != (b'\xff' * size):  # 全0xff则跳过
                while True:
                    ret = self.SendCmd(msg)
                    if ret == b'\x00':
                        break
            img = img[size:]
            pos += len(pb) 
            if img == (b'\xff' * len(img)):  # 全0xff则跳过剩余部分
                break
        print('100%')

    def WriteAppValidFlag(self): 
        # 0xc2 写APP有效标记(CRC32校验值)到FLASH末尾
        msg = b'\xc2\x00'
        msg += self.CalcLRC(msg).to_bytes(1, byteorder='little') 
        self.SendCmd(msg)
        
    def JumpToApp(self): 
        # 0x40 跳转至APP区域 
        msg = b'\xd0\x00'
        msg += self.CalcLRC(msg).to_bytes(1, byteorder='little') 
        self.SendCmd(msg)

    def JumpToBootloader(self): 
        # 0x42 跳转至BOOTLOADER区域 
        msg = b'## reboot'
#        msg += self.CalcLRC(msg).to_bytes(1, byteorder='little') 
        ret = self.SendCmd(msg)
        return ret 


def FlashOpFromCmdLine(port, baudrate, file):
    ser = serial.Serial(port, baudrate, timeout=0.2)

    """
    try:
        f = open(file, mode='rb')
    except:
        print("Failed to open binary file.")
        exit()
    img = f.read()
    f.close() 
    """

    try:
        f = bincopy.BinFile(file)
    except:
        print("Failed to open ihex file.")
        exit()
    img = f.as_binary()
    orig_len = len(img)
    
    a = Cli(ser) 
    print('Jumping to bootloader...')
    sys.stdout.flush()
    a.JumpToBootloader()
    time.sleep(0.1) 
    print('Erasing...')
    sys.stdout.flush()
    a.EraseImage(len(img))
    time.sleep(0.5) 
    print('Writing...')
    sys.stdout.flush()
    a.WriteImage(img)
    print('Verifying...')
    sys.stdout.flush()
    img = img[:orig_len]
    crc = int.from_bytes(a.ReadCRC32(len(img)), byteorder='little')
    crc2 = zlib.crc32(img)
    print('Written %d bytes.' % len(img)) 
    print('Original crc32: %08x' % crc2)
    print('Written crc32:  %08x' % crc)
    sys.stdout.flush()
    if crc == crc2:
        a.JumpToApp()
        print('ok') 
        sys.stdout.flush() 
    else: 
#        f = open('dump_img.bin', mode='wb')
#        f.write(img)
        pass
    ser.close() 

def ReadImage(port, baudrate):
    ser = serial.Serial(port, baudrate, timeout=0.1)
    a = Cli(ser)
    size = a.GetAppSize()
    print(size) 
    img = a.ReadImage(size)
    print(len(img))
    path = asksaveasfilename()
    f = open(path, mode='wb')
    f.write(img)
    f.close()
    ser.close()


def scan():
    '''scan for available ports '''
    ports = []
    baudrates = [500000]
    for i in range(32):
        try:
            port = 'COM' + str(i + 1)
            s = serial.Serial(port)
            ports.append(port)
            s.close()
        except serial.SerialException:
            pass
    print(ports)
    for port in ports:
        for baudrate in baudrates:
            try:
                ser = serial.Serial(port, baudrate, timeout=0.05)
            except:
                continue
            a = Cli(ser)
            for i in range(3):
                ret = a.SendCmd(b'## test')
                if ret != b'':
                    return port, baudrate
            ser.close()
 
    return None, None 


def test(port, baudrate):
    ser = serial.Serial(port, baudrate, timeout=0.2) 
    """
    try:
        f = open(file, mode='rb')
    except:
        print("Failed to open binary file.")
        exit()
    img = f.read()
    f.close() 
    """
    
    a = Cli(ser) 
    cmd = b'\x81\x00\x00\x01\x00\x38\x46'
    ret = a.SendCmd(cmd)
    for i in ret:
        print("%02x" % i, end=" ")


if __name__ == '__main__':
    # 以下用于单元测试
#    print(sys.argv)
    port, baudrate = scan() 
    if port == None:
        print('Serial port not deteced.')
        exit()
    
    if len(sys.argv) != 2: 
        print('Usage: iap_test.py xxx.bin')
        exit()

    file = sys.argv[1]

#     test(port, baudrate)
    FlashOpFromCmdLine(port, baudrate, file)
#     ReadImage(port, baudrate)
