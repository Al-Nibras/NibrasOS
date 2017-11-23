OS=$(shell uname)
CC=
LD=
OBJCOPY=
ASM=nasm
CFLAGS=-w -std=gnu99 -ggdb3 -O0 -fno-use-cxa-atexit -fno-rtti -fno-exceptions -fno-leading-underscore -nostdinc -fno-builtin -I ./include -c
LDFLAGS=-T ./src/link.ld
AFLAGS=-g -felf

ifeq ($(OS),Darwin)
CC +=i386-elf-gcc
LD +=i386-elf-ld
OBJCOPY +=i386-elf-objcopy
else
CC +=gcc
LD +=ld
OBJCOPY +=objcopy
CFLAGS += -m32
LDFLAGS += -melf_i386
endif

EMU=qemu-system-i386
EMUFLAGS=-m 8G -initrd iso/boot/initrd.img -drive file=disk.img,format=raw -kernel
EMUDEBUGFLAGS=-m 8G -s -S -monitor stdio -initrd iso/boot/initrd.img -drive file=disk.img,format=raw -kernel

TARGET=NibrasOS
OBJECTS = $(patsubst ./src/%.c, ./src/%.o, $(wildcard ./src/*.c))
OBJECTS += $(patsubst ./src/%.asm, ./src/%.o, $(wildcard ./src/*.asm))

.PHONY: all
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)
	$(RM) ./src/*.o *.iso *.sym
	$(OBJCOPY) --only-keep-debug $(TARGET) kernel.sym
%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@
%.o: %.asm
	$(ASM) $(AFLAGS) $^ -o $@

.PHONY: run
run: $(TARGET)
	$(EMU) $(EMUFLAGS) $(TARGET)
.PHONY: debug
debug: $(TARGET)
	$(EMU) $(EMUDEBUGFLAGS) $(TARGET)




.PHONY: clean
clean:
	$(RM) $(TARGET) *.sym ./src/*.o
