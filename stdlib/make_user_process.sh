i386-elf-gcc -m32 -I . -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs stdlib.c -o stdlib.o -c
i386-elf-gcc -m32 -I . -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs $1 -o $2.o -c

nasm -f elf crt0.s -o crt0.o
nasm -f elf crti.s -o crti.o
nasm -f elf crtn.s -o crtn.o

i386-elf-ld -T user.ld -melf_i386 crt0.o crti.o crtn.o stdlib.o $2.o -o $2
