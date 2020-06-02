@echo off

:start
cls
echo Select MCU: 
echo 1. STM32F030F4
echo 2. STM32F042F6
echo 3. STM32F072CB
echo 4. STM32F103RE
echo 5. STM32F303CC
echo 6. STM32F401RC
echo 7. GD32F350CB 
echo 8. GD32F130F8
echo 9. GD32F330F4 
echo A. STM32F100C8 
echo B. STM32F407VE
echo Q. Exit
echo Please enter:
choice /c:123456789ABQ

if errorlevel 12 goto :end
if errorlevel 11 make.exe -Bsj -f "_makefiles\Makefile.stm32f407"
if errorlevel 10 make.exe -Bsj -f "_makefiles\Makefile.stm32f10x_md_vl"
if errorlevel 9 make.exe -Bsj -f "_makefiles\Makefile.gd32f330"
if errorlevel 8 make.exe -Bsj -f "_makefiles\Makefile.gd32f130_150"
if errorlevel 7 make.exe -Bsj -f "_makefiles\Makefile.gd32f350"
if errorlevel 6 make.exe -Bsj -f "_makefiles\Makefile.stm32f401"
if errorlevel 5 make.exe -Bsj -f "_makefiles\Makefile.stm32f303"
if errorlevel 4 make.exe -Bsj -f "_makefiles\Makefile.stm32f10x_hd"
if errorlevel 3 make.exe -Bsj -f "_makefiles\Makefile.stm32f072"
if errorlevel 2 make.exe -Bsj -f "_makefiles\Makefile.stm32f042"
if errorlevel 1 make.exe -Bsj -f "_makefiles\Makefile.stm32f030"
goto end

:end
echo Done.
pause
