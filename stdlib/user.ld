/* User-mode processes linker */

OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
  . = 0x8000000;                 /* relocate to address 0 */

  .text ALIGN(4):
  {
      crt0.o(.text)     /* include the .text section of start.o */
      *(.text)           /* include all other .text sections */
  }

  .data ALIGN(4):
  {
      *(.data)
  }

  .rodata ALIGN(4):
  {
      *(.rodata*)
  }
}
