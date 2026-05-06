@echo off
set ELF=C:/projects/stm32-framework/build/debug/my_app.elf

openocd ^
 -s C:/projects/stm32-framework/framework_v2/tools/openocd/scripts ^
 -f interface/stlink.cfg ^
 -f target/stm32f1x.cfg ^
 -c "transport select swd" ^
 -c "gdb_port 3333" ^
 -c "init" ^
 -c "reset halt"