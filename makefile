objcopy --input binary \
        --output elf32-i386 \
        --binary-architecture i386 res/prestobin_rc.bin output/prestobin_rc.o

gcc -DPRESTOBINAPI -D__LINUX -I./inc src/prestobin.c output/prestobin_rc.o -o output/prestobin
