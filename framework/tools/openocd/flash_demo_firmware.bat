@echo off
set OPENOCD=C:/projects/stm32-framework/framework/tools/openocd/bin/openocd.exe
set SCRIPTS=C:/projects/stm32-framework/framework/tools/openocd/scripts

"%OPENOCD%" ^
 -s "%SCRIPTS%" ^
 -f interface/stlink.cfg ^
 -f target/stm32f1x.cfg ^
 -c "transport select swd" ^
 -c "init" ^
 -c "reset halt" ^
 -c "program C:/projects/stm32-framework/build/debug/demo_firmware.elf verify reset exit"
