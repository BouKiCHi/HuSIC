/* XPCMDRV.C */
/* Please see license in document of HuSIC to use this code */

#define XPCM_CH  5
#define XPCM2_CH 4

#define XPCM_FLAG 0x01
#define XPCM2_FLAG 0x02

#define XPCM_MASK 0xfe
#define XPCM2_MASK 0xfd

int  xpcm_addr[2];
int  xpcm_len[2];
char xpcm_shift[2];
char xpcm_bank[2];
char xpcm_buf[2];
char xpcm_nextbuf[2];

char xpcm_bank_save;

/* XPCM使用フラグ */
char xpcm_flags;
char xpcm_play_flags;

/*
  XPCM 変数表 version 0.1

  xpcm_ptr = アドレス
  xpcm_len = 長さ
  xpcm_bank = バンク
  xpcm_shift = シフト

  0x00  ... stop
  0x01  ... play
*/

pcm_play_data ( ch , addr , len , bank )
int ch , addr , len;
char bank;
{
	int idx;

	switch ( ch )
	{
		case XPCM2_CH:
			idx = 1;
		break;
		case XPCM_CH:
		default:
			idx = 0;
		break;
	}

	xpcm_addr[idx] = addr;
	xpcm_len[idx]  = len;
	xpcm_bank[idx] = bank;
	xpcm_shift[idx] = 0;
	xpcm_nextbuf[idx] = 0;

	pcm_on( ch );
}

pcm_on( ch )
char ch;
{
	if (!xpcm_play_flags) ena_irq_tmr(); 

	if ( ch == XPCM_CH ) xpcm_play_flags |= XPCM_FLAG;
	if ( ch == XPCM2_CH ) xpcm_play_flags |= XPCM2_FLAG;
}


pcm_off( ch )
char ch;
{
	if ( ch == XPCM_CH ) xpcm_play_flags &= XPCM_MASK;
  if ( ch == XPCM2_CH ) xpcm_play_flags &= XPCM2_MASK;

	if (!xpcm_play_flags ) dis_irq_tmr();
}

pcm_stop ( ch )
char ch;
{
/*	switch ( ch )
	{
		case XPCM_CH:
			xpcm_len[0]  = 0;
		break;
		case XPCM2_CH:
			xpcm_len[1]  = 0;
		break;
	}
*/
	pcm_off ( ch );
}


pcm_check ( ch )
char ch;
{
	if ( ch == XPCM_CH ) return xpcm_flags & XPCM_FLAG;
	if ( ch == XPCM2_CH ) return xpcm_flags & XPCM2_FLAG;
	return 0;
}


pcm_switch ( ch , mode )
char ch;
char mode;
{
	if ( mode ) {
		poke( SND_SEL , ch );
		poke( SND_MIX , 0xDF ); /* on */
		poke( SND_SEL , reg_ch ); /* restore ch */
		if ( ch == XPCM_CH ) xpcm_flags |= XPCM_FLAG;
		if ( ch == XPCM2_CH) xpcm_flags |= XPCM2_FLAG;
	} else {
		if ( ch == XPCM_CH ) xpcm_flags &= XPCM_MASK;
		if ( ch == XPCM2_CH ) xpcm_flags &= XPCM2_MASK;
	}
}

#asm

	REG_SEL: .equ $0800
	REG_DAC: .equ $0806


	; _xpeeki_b <addr>
	;
	; in  : addr = address
	; out : reg.a

	.macro _xpeeki_b
		__ldwi	\1
		__stw	<__ptr
		lda		[__ptr]

	.endm

	; _xpeek_b <addr>
	;
	; in  : addr = address
	; out : reg.a

	.macro _xpeek_b
		__ldw	\1
		__stw	<__ptr
		lda		[__ptr]

	.endm


	; _xpokei_b <addr>
	;
	; in : addr = address
	;    : reg.a = value

	.macro _xpokei_b
		pha
		__ldwi	\1
		__stw	<__ptr
		pla
		sta		[__ptr]
	.endm

	; _xpoke_b <addr>
	;
	; in : addr = address
	;    : reg.a = value

	.macro _xpoke_b
		pha
		__ldw	\1
		__stw	<__ptr
		pla
		sta		[__ptr]
	.endm


	;_pcm_proc < ch , index >
	;
	; in : ch = 物理チャンネル
	;    : index = 変数の相対位置

	.macro _pcm_proc

		; チャンネル選択
		lda		#\1
		_xpokei_b REG_SEL

		; バンク切り替え
		tma		#3
		sta		_xpcm_bank_save

		lda		_xpcm_bank + \2
		tam		#3

		; 4bitシフト
		lda		_xpcm_shift + \2
		beq		.high_\2

	.low_\2:

		_xpeek_b _xpcm_addr + (\2 * 2)
		asl		A
		and		#$1e

		pha
		__ldw   _xpcm_addr + (\2 * 2)
		__addwi 1
		__stw	_xpcm_addr + (\2 * 2)

		__ldw   _xpcm_len + (\2 * 2)
		__subwi 1
		__stw	_xpcm_len + (\2 * 2)


		pla

		jmp		.store_\2

	.high_\2:

		_xpeek_b _xpcm_addr + (\2 * 2)
		lsr		A
		lsr		A
		lsr		A
		and		#$1e

	.store_\2:
		; DACへ出力
		_xpokei_b REG_DAC

		; バンク切り替えを戻す
		lda		_xpcm_bank_save
		tam		#3

		; シフトフラグを反転
		lda		_xpcm_shift + \2
		eor		#1
		sta		_xpcm_shift + \2

		__ldw   _xpcm_len + (\2 * 2)
		stx		<__temp
		ora		<__temp

		bne		.end_\2

		; PCMをオフにする
		__ldwi	\1
		call	_pcm_off

	.end_\2:
	.endm

	.if 0

	;_pcm_proc_5bit < ch , index >
	;
	; in : ch = 物理チャンネル
	;    : index = 変数の相対位置

	.macro _pcm_proc_5bit

		; チャンネル選択
		lda		#\1
		_xpokei_b REG_SEL

		; バンク切り替え
		tma		#3
		sta		_xpcm_bank_save
		lda		_xpcm_bank + \2
		tam		#3

		; バッファクリア
		lda   #0
		sta		_xpcm_buf + \2

		; シフト数を設定
		lda		#5

	; ループ
	.shift_lp_\2:
		pha
		; バッファのビット数を読みだし
		lda		_xpcm_shift + \2
		; 0なら読み出し
		bne		.shift_bit_\2

		; バッファに読み出し
		_xpeek_b _xpcm_addr + (\2 * 2)
		sta		_xpcm_nextbuf + \2

		; アドレスポインタ加算
		__ldw   _xpcm_addr + (\2 * 2)
		__addwi 1
		__stw	_xpcm_addr + (\2 * 2)

		; 残りサイズ減算
		__ldw   _xpcm_len + (\2 * 2)
		__subwi 1
		__stw	_xpcm_len + (\2 * 2)

		; ビット数を補充
		lda		#8
		sta		_xpcm_shift + \2

	; ビットシフトを行う
	.shift_bit_\2:
		asl		_xpcm_nextbuf + \2
		rol		_xpcm_buf + \2
		; バッファのビット数を減算
		dec		_xpcm_shift + \2

		; シフト数を減算
		pla
		dec		A
		bne		.shift_lp_\2

	; 出力
	.store_\2:
		; バッファ読み出し
		lda		_xpcm_buf + \2
		; DACへ出力
		_xpokei_b REG_DAC

		; バンク切り替えを戻す
		lda		_xpcm_bank_save
		tam		#3

		; 残りサイズの確認
		__ldw   _xpcm_len + (\2 * 2)
		stx		<__temp
		ora		<__temp

		; まだサイズがあるのでスキップする
		bne		.skip_pcmoff_\2

		; PCMをオフにする
		__ldwi	\1
		call	_pcm_off

	;　終了
	.skip_pcmoff_\2:
	.endm

	.endif

	.if (USE_5BITPCM)

	;_pcm_proc_8bit < ch , index >
	;
	; in : ch = 物理チャンネル
	;    : index = メモリ相対位置

	.macro _pcm_proc_8bit

		; チャンネル選択
		lda		#\1
		_xpokei_b REG_SEL

		; バンク切り替え
		tma		#3
		sta		_xpcm_bank_save
		lda		_xpcm_bank + \2
		tam		#3

		; バッファに読み出し
		_xpeek_b _xpcm_addr + (\2 * 2)
		sta		_xpcm_buf + \2

		; アドレスポインタ加算
		__ldw   _xpcm_addr + (\2 * 2)
		__addwi 1
		__stw	_xpcm_addr + (\2 * 2)

		; 残りサイズ減算
		__ldw   _xpcm_len + (\2 * 2)
		__subwi 1
		__stw	_xpcm_len + (\2 * 2)

	; 出力
	.store_\2:
		; バッファ読み出し
		lda		_xpcm_buf + \2
		; DACへ出力
		_xpokei_b REG_DAC

		; バンク切り替えを戻す
		lda		_xpcm_bank_save
		tam		#3

		; 残りサイズの確認
		__ldw   _xpcm_len + (\2 * 2)
		stx		<__temp
		ora		<__temp

		; まだサイズがあるのでスキップする
		bne		.skip_pcmoff_\2

		; PCMをオフにする
		__ldwi	\1
		call	_pcm_off

	;　終了
	.skip_pcmoff_\2:

	.endm
	.endif

	;
	; pcm_intr
	;

	.proc _pcm_intr
		; 再生フラグチェックで各PCM再生

		lda		_xpcm_play_flags
		and		#$01
		beq		.end_ch1
		jmp		.ch1_proc
	.end_ch1:
		lda		_xpcm_play_flags
		and		#$02
		beq		.end_ch2
		jmp		.ch2_proc
	.end_ch2:

		; チャンネルを戻して終了
		lda		_reg_ch
		_xpokei_b REG_SEL
		rts

	; PCM出力
	.if (USE_5BITPCM)
	; 5ビット

	.ch1_proc:
		_pcm_proc_8bit 5, 0
		jmp		.end_ch1

	.ch2_proc:
		_pcm_proc_8bit 4, 1
		jmp		.end_ch2

	.else

	; 4ビット

	.ch1_proc:
		_pcm_proc 5, 0
		jmp		.end_ch1

	.ch2_proc:
		_pcm_proc 4, 1
		jmp		.end_ch2

	.endif
	.endp

#endasm

#asm
	; 割り込み有効
    .proc _ena_irq_tmr
	smb   #2,<irq_m
	rts
	.endp

	; 割り込み無効
    .proc _dis_irq_tmr
	rmb   #2,<irq_m
	rts
	.endp

	; 割り込みタイマー設定
	.proc _set_pcmintr

	; タイマー割り込みを禁止にする
	rmb   #2, <irq_m
	stz   irq_status
	stw   #_timer_pcm, timer_jmp

	; V = 1
	; (7.159090 / 1024) / V = 6991.29Hz

	lda   #0
	sta   timer_cnt

	lda   #$1
	sta   timer_ctrl

	cli

	rts
	.endp

	; PCMバンク
	.proc _chg_pcmbank
	txa
	tam #3
	rts
	.endp

#endasm


init_pcmdrv()
{
	int i;
	for ( i = 0; i < 2; i++ )
	{
		/* int */
		xpcm_addr[i] =
		xpcm_len[i]  = 0x0000;

		/* char */
		xpcm_shift[i] =
		xpcm_bank[i] = 0;
	}

	xpcm_play_flags = 0x00;
	xpcm_flags = 0x00;

	set_pcmintr();
}
