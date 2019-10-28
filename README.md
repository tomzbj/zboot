# zboot
Generic STM32 serial bootloader, with a simple command-line interface

某次@xjtuecho提到他做的一个bootloader, 支持N多型号, 只要改下配置文件就能适应不同型号；此外还提供了简单的命令解释器, 可以在串口命令行实现查看/擦除任意位置flash, 模拟eeprom读写(与app共享)等功能. 是不是挺实用? 于是打算自己也实现一个.

## 开发环境

arm-none-eabi-gcc 4.9.3. Keil或IAR用户可能需要手动修改链接脚本和Makefile之类.

## 支持MCU列表(更新中)

1. STM32F072CB, 实测正常使用.
2. STM32F303CC, 实测正常使用.
3. GD32F350CB, 实测正常使用.
4. STM32F030C8, 可正常编译, 未测试.
5. STM32F103RC, 可正常编译, 未测试.

## 空间占用情况

- 在FLASH每页1K的MCU上共占用8K FLASH, 其中EEPROM占1页, 提供256字节EEPROM空间.
- 在FLASH每页2K的MCU上共占用10K FLASH, 提供512字节EEPROM空间.

## 时钟及波特率

为了时钟配置简单起见, bootloader在8M HSI时钟运行. 串口波特率定为500kbps.

## 使用方法

选择对应型号的Makefile, 并在usart.c里修改用到的usart外设和管脚即可. 
上位机见根目录的iap.py, python 3.6.4和python 3.7.1实测正常工作. 
需要zlib和bincopy两个第三方库. (前者用于计算CRC32校验, 后者用于把hex格式转为binary.)

app这边需要做的:

1. 写入bootloader后先在串口命令行执行## sysinfo, 取得app区的入口地址, 应该是0x08002000或0x08002800.
2. 修改链接文件(.ld或.lds, 在不同开发环境下可能不同), 把FLASH区的起始地址改为上面的入口地址,  LENGTH要根据页大小减去8K或10K.
3. Makefile或者其他类似指定了FLASH大小的场合, 要减去8K或10K.
4. main.c在最前面加上两行, 其中VECT_TAB_OFFSET的值是0x2000或0x2800.

```
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, VECT_TAB_OFFSET);
    __enable_irq()
```

5. 如果是stm32f0系列呢? 它们不提供重定向中断向量表的功能, 所以NVIC_SetVectorTable就没用了. 
不过bootloader还是能用的, 稍微麻烦一点, 需要在main.c最前面加上这几行, 把中断向量表复制到SRAM的起始位置,  然后把复位地址改为指向SRAM起始位置. 

```
    memcpy((void*)(0x20000000), (void*)APP_BASE, 256);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
    __enable_irq()
```    

APP_BASE的值是前面提到过的0x08002000或0x08002800. __enable_irq()则还是需要的. 此外, .ld/.lds文件里SRAM起始位置需要改为0x20000100, LENGTH要减去256.

5. 上电启动同时执行py iap.py xxx.hex即可. (启动延迟约1s后自动跳转到app.) 

6. 如果需要一键iap, 应该怎么操作呢? 需要在app里响应命令"## REBOOT"后, 先向串口输出任意字符, 然后执行NVIC_SystemReset()即可. 如果是stm32f0xx, 则还要先执行SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_Flash), 把系统起始位置改回FLASH. 

7. 可以在app的Makefile里增加一条目标iap, 这样更新程序时只要执行make iap即可, 省太多事了.

```
PYTHON := "py.exe"
IAP  := $(PYTHON) "iap.py
...
iap:
	$(IAP) xxx.hex
```

8. 如果要在app里使用eeprom呢? 只要在app里加上flash_eeprom.h和flash_eeprom.c两个文件, 并在使用前先初始化即可:

```
    FLASH_EEPROM_Config(APP_BASE - PAGE_SIZE, PAGE_SIZE);
```

致谢:
xjtuecho
elm-chan
