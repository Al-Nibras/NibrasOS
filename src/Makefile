OS=$(shell uname)
CC=
LD=
OBJCOPY=
ASM=nasm
CFLAGS=-w -std=gnu99 -ggdb3 -O0 -fno-use-cxa-atexit -fno-rtti -fno-exceptions -fno-leading-underscore -nostdinc -fno-builtin -I ../include -c
LDFLAGS=-T ../src/link.ld
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
EMUFLAGS=-m 4G -kernel
EMUDEBUGFLAGS= -m 4G -s -S -kernel
TARGET=kernel-dbg
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
OBJECTS += $(patsubst %.asm, %.o, $(wildcard *.asm))

.PHONY: all
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(LD) -o ../$@ $^ $(LDFLAGS)
	$(RM) *.o ../*.iso ../*.sym
	$(OBJCOPY) --only-keep-debug ../$(TARGET) ../kernel.sym
%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@
%.o: %.asm
	$(ASM) $(AFLAGS) $^ -o $@

.PHONY: run
run: $(TARGET)
	$(EMU) $(EMUFLAGS) ../$(TARGET)

.PHONY: debug
debug: $(TARGET)
	$(EMU) $(EMUDEBUGFLAGS) ../$(TARGET)



.PHONY: clean
clean:
	$(RM) ../$(TARGET) ../*.sym *.o
