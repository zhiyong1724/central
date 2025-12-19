/* See LICENSE for license details. */

OUTPUT_ARCH( "riscv" )

ENTRY( do_reset )
USER_STACK_SIZE = 1 * 1024 * 1024;
USER_HEAP_SIZE = 8 * 1024 * 1024;
SYS_HEAP_SIZE = 8 * 1024 * 1024;
/* Specify the memory areas */
MEMORY
{
RAM (xrw)        : ORIGIN = 0x80000000, LENGTH = 64M
}

SECTIONS
{

  /*--------------------------------------------------------------------*/
  /* Code and read-only segment                                         */
  /*--------------------------------------------------------------------*/

  /* Begining of code and text segment */
  _ftext = .;

  .text :
  {
    *(.text.init)
  } >RAM

  /* text: Program code section */
  .text : 
  {
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t.*)
  } >RAM

  /* rodata: Read-only data */
  .rodata : 
  {
    *(.rdata)
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
  } >RAM

  /* End of code and read-only segment */
  . = ALIGN(0x1000);
  _etext = .;

  /*--------------------------------------------------------------------*/
  /* Initialized data segment                                           */
  /*--------------------------------------------------------------------*/

  /* Start of initialized data segment */
  . = ALIGN(16);
   _fdata = .;

  /* data: Writable data */
  .data : 
  {
    *(.data)
    *(.data.*)
    *(.srodata*)
    *(.gnu.linkonce.d.*)
    *(.comment)
    *(.dtb)
  } >RAM
  /* End of initialized data segment */
  . = ALIGN(16);
  _edata = .;

  /*--------------------------------------------------------------------*/
  /* Uninitialized data segment                                         */
  /*--------------------------------------------------------------------*/

  /* Start of uninitialized data segment */
  __bss_start = .;
  _end = .;
  _fbss = .;
  .bss : 
  {
    *(.bss)
    *(.bss.*)
    *(.sbss*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
  } >RAM
  . = ALIGN(16);
  _ebss = .;

  /* User_heap_stack section, used to check that there is enough RAM left */
  .user_stack :
  {
    _fuser_stack = .;
    . = . + USER_STACK_SIZE;
  } >RAM
  . = ALIGN(16);

  .user_heap :
  {
    _user_heap = .;
    . = . + USER_HEAP_SIZE;
  } >RAM
  . = ALIGN(16);

  .sys_heap :
  {
    _fsys_heap = .;
    . = . + SYS_HEAP_SIZE;
    _esys_heap = .;
  } >RAM
  . = ALIGN(16);
  __global_pointer$ = .;
  . = . + 4;
  . = ALIGN(16);
}
