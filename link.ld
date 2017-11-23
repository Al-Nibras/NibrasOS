OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

ENTRY(start)
SECTIONS
{

   . = 0x00100000;

    .text ALIGN (0x1000) :
    {
      code = .; _code = .; __code = .;
      *(.multiboot)
      *(.text)
    }

    .rodata ALIGN (0x1000) :
    {
      rodata = .; _rodata = .; __rodata = .;
      *(.rodata*)
    }

    .data ALIGN (0x1000) :
    {
      data = .; _data = .; __data = .;
      *(.data)
    }

    .bss ALIGN (0x1000) :
    {
      bss = .; _bss = .; __bss = .;
      *(COMMON)           /* all COMMON sections from all files */
      *(.bss)             /* all bss sections from all files */
    }

    end = .; _end = .; __end = .; /* needed to setup Kernel Heap */

    /DISCARD/ :
    {
      *(.fini_array*)
      *(.comment)
    }


}
