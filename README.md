openocd
make
gdb-multiarch -ex "target extended-remote localhost:3333" main.elf

