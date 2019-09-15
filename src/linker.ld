SECTIONS {
  /* below 0x44f88 reserved for/by RedBoot */
  . = 0x100000;

  /* mark BSS segment */
  __bss_start = ADDR(.bss);
  __bss_end   = ADDR(.bss) + SIZEOF(.bss);

  /* needed for certain newlib routines that (potentially) call _sbrk */
  end = __bss_end;
}
