@echo off

set ELF=C:/projects/stm32-framework/build/debug/demo_firmware.elf

echo Starting OpenOCD...

start "" /MIN openocd ^
 -s C:/projects/stm32-framework/framework/tools/openocd/scripts ^
 -f interface/stlink.cfg ^
 -f target/stm32f1x.cfg ^
 -c "transport select swd" ^
 -c "gdb_port 3333" ^
 -c "init" ^
 -c "reset halt"