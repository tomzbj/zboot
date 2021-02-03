@echo off

:start
cls
echo Select MCU: 
echo 1. STM32F030F4
echo 2. STM32F042F6
echo 3. STM32F070CB
echo 4. STM32F072CB
echo 5. STM32F103RE
echo 6. STM32F303CC
echo 7. STM32F401RC
echo 8. GD32F350CB 
echo 9. GD32F130F8
echo A. GD32F330F4 
echo B. STM32F100C8 
echo C. STM32F407VE
echo D. STM32F051C8
echo E. STM32F030xC
echo Q. Exit
echo Please enter:
choice /c:123456789ABCDEQ

if errorlevel 15 goto :end
if errorlevel 14 make.exe -Bsj -f "_makefiles\Makefile.stm32f030xC"
if errorlevel 13 make.exe -Bsj -f "_makefiles\Makefile.stm32f051"
if errorlevel 12 make.exe -Bsj -f "_makefiles\Makefile.stm32f407"
if errorlevel 11 make.exe -Bsj -f "_makefiles\Makefile.stm32f10x_md_vl"
if errorlevel 10 make.exe -Bsj -f "_makefiles\Makefile.gd32f330"
if errorlevel 9 make.exe -Bsj -f "_makefiles\Makefile.gd32f130_150"
if errorlevel 8 make.exe -Bsj -f "_makefiles\Makefile.gd32f350"
if errorlevel 7 make.exe -Bsj -f "_makefiles\Makefile.stm32f401"
if errorlevel 6 make.exe -Bsj -f "_makefiles\Makefile.stm32f303"
if errorlevel 5 make.exe -Bsj -f "_makefiles\Makefile.stm32f10x_hd"
if errorlevel 4 make.exe -Bsj -f "_makefiles\Makefile.stm32f072"
if errorlevel 3 make.exe -Bsj -f "_makefiles\Makefile.stm32f070xb"
if errorlevel 2 make.exe -Bsj -f "_makefiles\Makefile.stm32f042"
if errorlevel 1 make.exe -Bsj -f "_makefiles\Makefile.stm32f030"
goto end

:end
echo Done.
pause
