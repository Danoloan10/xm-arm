/*
 * $FILE$
 * 
 * ld script to build armv7 cortexA9
 * 
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 * 
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [12/01/16:XM-ARM-2:SPR-150615-01:#59]: Remove dead code.
 */

/* <track id="PARTITION_LINKER_SCRIPT"> */
ENTRY(start)
EXTERN(start)
EXTERN(xmImageHdr)

SECTIONS
{	
  _sguest = .;
  .text ALIGN (8): {
	. = ALIGN(4K);
	_sguest = .;
  	*(.text.init)
  	*(.text)

    __CTOR_LIST__ = .;
    LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
    *(.ctors)
    LONG(0)
    __CTOR_END__ = .;

    __DTOR_LIST__ = .;
    LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
    *(.dtors)
    LONG(0)
    __DTOR_END__ = .;

  }

  .rodata ALIGN (8) : {
	*(.rodata)
  	*(.rodata.*)
	*(.rodata.*.*)
  }

  .xmImageHdr ALIGN (8) : {
         *(.xmImageHdr)
  }

  .ARM.extab ALIGN(8) : {
        *(.ARM.extab)
  }

  .ARM.exidx ALIGN(8) : {
        __exidx_start = .;
        *(.ARM.exidx)
        __exidx_end = .;
  }

  
  .data ALIGN (8) : AT(ADDR(.ARM.exidx)+SIZEOF(.ARM.exidx)) {
    _spdata = LOADADDR(.data);
	_sdata = .;
	*(.data)
	_edata = .;
  }

  .bss ALIGN (8) : AT(LOADADDR(.data)+SIZEOF(.data)) {
  	*(.bss.noinit)
	_sbss = .;
	*(COMMON)
	*(.bss)
	_ebss = .;
  }

  _eguest = .;

  /DISCARD/ :
  {
	*(.note)
	*(.comment*)
  }
}
/* </track id="PARTITION_LINKER_SCRIPT"> */
