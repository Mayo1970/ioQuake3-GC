/* ld -r script for one native module: brackets .data and .bss so gc_modules.c can
   restore .data and zero .bss on every load, like a fresh QVM. */
SECTIONS
{
  .data : { gcmod_data_start = .; *(.data .data.*) gcmod_data_end = .; }
  .bss  : { gcmod_bss_start = .; *(.bss .bss.* COMMON) gcmod_bss_end = .; }
}
