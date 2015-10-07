;
; PLEASE DO NOT REPLACE STARTUP.ASM IN INCLUDE DIR
;

  ifdef MAKE_HES
		include "sup_hes.asm"
  else
  	include "sup_pce.asm"
  endif
