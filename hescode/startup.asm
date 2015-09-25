;
; PLEASE DO NOT REPLACE STARTUP.ASM IN INCLUDE DIR
;

	.ifndef MAKE_PCE
	.include "sup_hes.asm"
     else
    .include "sup_pce.asm"
	.endif
