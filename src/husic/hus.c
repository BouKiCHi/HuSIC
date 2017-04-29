/* -----------------------------------------------------------
 HuSIC by BouKiCHi
 多目的サウンドドライバ since 2004/02/29

 このコードはmckのコードを参考に書かれています。
 ライセンスはパッケージにあるドキュメントを参照してください。

 Please see license in the package to use this code.
 ----------------------------------------------------------- */

/********************************************

  PCエンジン音源の詳細
  0-1ch 波形音源
  このチャンネルは周波数変調の音源として利用することが可能です。

  2-3ch 波形音源
  通常の波形音源です。

  4-5ch 波形音源
  このチャンネルはノイズジェネレータとしても使用可能です。

  波形データの詳細
  符号付き5bit 32バイト
  精度は12ビット

*********************************************/

#define SND_SEL 0x0800 /* Channel Selector */
#define SND_VOL 0x0801 /* Global Volume */
#define SND_FIN 0x0802 /* Fine Freq(FREQ LOW) */
#define SND_ROU 0x0803 /* Rough Freq(FREQ HI) */
#define SND_MIX 0x0804 /* Mixer */
#define SND_PAN 0x0805 /* Pan Volume */
#define SND_WAV 0x0806 /* Wave Buffer */
#define SND_NOI 0x0807 /* Noise Mode */
#define SND_LFO 0x0808 /* LFO freq */
#define SND_LTR 0x0809 /* LFO trig/control */


/*
SND_MIX 0x0804 ミキサレジスタ
bit7 on(output enable)
bit6 Direct D/A(dda)
bit4 - bit0 : volume
*/


extern int sound_dat[];
extern int xpcmdata;

extern int pcewav[];

/* ノイズ = 3.58MHz / 64 * ( 5bit XOR 31) */

/* o4 = 2 ,o3 = 1 ,o2 = 0 */

/********************

 音階データの下位4bit(基本的にMCKと同等)
 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
 C  C#  D D#  E  F F#  G G#  A A#  B     A A#  B

 0D-0Fは1オクターブ下の音になります。

********************/

/* 周波数テーブル */
#include "ftable.h"

/* チャンネル数 */
#define MAX_CH 6

/* エフェクト用フラグ */
#define EFX_SLAR (1)
#define EFX_PORT (2)
#define EFX_PORT_ST (4)

/* エンベロープリセット用フラグ */
#define RI_TE 1 /* TONE */
#define RI_NE 2 /* NOTE */
#define RI_PE 4 /* PITCH */
#define RI_LFO 8 /* LFO */
#define RI_PAN 16 /* PAN */

char	reg_ch;

char	ch_nowbank;
char	ch_topbank;

char	*seq_ptr;

char	ch_lastcmd[MAX_CH];

char	ch_cnt[MAX_CH];
char	ch_vol[MAX_CH];
char	ch_bank[MAX_CH];

char	ch_lastvol[MAX_CH];
char	ch_lasttone[MAX_CH];

char  ch_efx[MAX_CH];
char  ch_rst[MAX_CH];

char  ch_porthi[MAX_CH];
char  ch_portlo[MAX_CH];
char  ch_portcnt[MAX_CH];

char	note_data[MAX_CH];

char  multienv_sw[MAX_CH];
char	tone_sw[MAX_CH];
char	pitch_sw[MAX_CH];
char	note_sw[MAX_CH];

char	detune[MAX_CH];
char	panpod[MAX_CH];
char	loop_cnt[MAX_CH];

int		seq_pos[MAX_CH];
int		seq_freq[MAX_CH];

int		tone_envadr[MAX_CH];
int		pitch_envadr[MAX_CH];
int		volume_envadr[MAX_CH];
int		note_envadr[MAX_CH];

int		multi_envadr[MAX_CH];
int		multi_envcnt[MAX_CH];


char	lfo_sw[MAX_CH]; /* 0xff = no_effect*/
char	lfo_cnt[MAX_CH]; /* speed cnt */
char	lfo_ct2[MAX_CH]; /* step cnt */
char	lfo_stp[MAX_CH]; /* step of 1level */
char	lfo_lev[MAX_CH]; /* up/down level of 1step */
char	lfo_step[MAX_CH];

char	noise_freq[2];
char	noise_sw[2];

int song_track_table; /* 0 */
int song_bank_table; /* 7 */
int song_loop_table; /* 1 */
int song_loop_bank; /* 28*/

/* PCMドライバ */
#include "xpcmdrv.c"

#asm
; CBANKを切り替え
	.proc _chg_cbank
	txa
	tam #2
	rts
	.endp

; 現在のCBANKを取得
	.proc _now_cbank
	tma #2
	tax
	cla
	rts
	.endp

; デフォルトCBANKの設定
	.proc _def_cbank
	lda #CONST_BANK+_bank_base
	tam #2
	rts
	.endp

; VSYNC割り込みフック設定
	.proc _drv_setintr
  sei
	stw   #_vsync_drv, vsync_hook
	smb   #4,<irq_m		; enable new code
	smb   #5,<irq_m		; disable system card code
	cli
	rts
	.endp

; 割り込み禁止
  .proc _disable_irq
	sei
	rts
	.endp
	
; 割り込み許可
	.proc _enable_irq
	cli
	rts
	.endp

; 曲数
	.proc _get_songmax
	lda #0
	ldx #TOTAL_SONGS
	rts
	.endp


#endasm

/* チャンネル音量設定 */
mixvol(val)
char val;
{
 /* PCMチャンネルであれば違う処理をする */
 if ( pcm_check ( reg_ch ) )
 {
	poke( SND_MIX , 0xC0 | val );
	return;
 }

  if (val)
		val |= 0x80;

	poke(SND_MIX, val);

	ch_lastvol[reg_ch] = val;
}

/* デフォルトのノコギリ波形 */
snd_saw( ch )
int ch;
{
	char i;

	poke( SND_MIX, 0x00 );

	for( i = 0; i < 32; i++ )
	{
		poke( SND_WAV , i );
	}

	poke(SND_PAN, panpod[ch]);
	poke(SND_MIX, 0x80); /* on */
}

/* 音色切り替え */
snd_chg(num)
int num;
{
	int	i;
	char	*pcmpos;

	/* 前回との比較 */
	if (ch_lasttone[reg_ch] == num)
		return;

	ch_lasttone[reg_ch] = num;

	pcmpos = *(pcewav[0] + (num << 1));

	/* オフにしないと正しく書き込めない */
	poke(SND_MIX, 0x00);

	for(i = 0; i < 32; i++)
	{
		poke(SND_WAV, *pcmpos);
		pcmpos++;
	}

	/* 最後の音量に復帰する */
	i = ch_lastvol[reg_ch];
	poke(SND_MIX, i);
}

/* 周波数 = 3.58MHz / 32 x (12bit value) */
int song_addr_base;
int song_addr_table;

/* HuSIC 初期化 */
drv_init_song(songno)
char songno;
{
	int i;
	char *seq_data;
	int song_addr_diff;

	#asm
	; drv_init_song()
	#endasm
		

	disable_irq();

	songno = songno % get_songmax();

	song_addr_table = sound_dat[15];
	song_addr_base = *(song_addr_table);
	song_addr_diff = *(song_addr_table + (songno << 1)) - song_addr_base;
	
	song_track_table = sound_dat[0] + song_addr_diff;
	song_bank_table = sound_dat[7] + song_addr_diff;
	song_loop_table = sound_dat[1] + song_addr_diff;
	song_loop_bank = sound_dat[8] + song_addr_diff;
	
	ch_topbank = now_cbank();
	seq_ptr = 0x1234;

	for ( i = 0; i < 6; i++ )
	{
		reg_ch = i;
		poke( SND_SEL, i );

		tone_sw[i] = 0xff;
		lfo_sw[i] = 0xff;
		note_sw[i] = 0xff;
		panpod[i] = 0xff;
		pitch_sw[i] = 0xff;
		multienv_sw[i] = 0xff;
		ch_lasttone[i] = 0xff;

		loop_cnt[i] = 0x00;
		detune[i] = 0x00;
		tone_envadr[i] = 0x00;
		note_envadr[i] = 0x00;
		pitch_envadr[i] = 0x00;
		volume_envadr[i] = 0x00;
		multi_envadr[i] = 0x00;

		seq_pos[i] = *(song_track_table + (i<<1));
		ch_bank[i] = *(song_bank_table + i);

		snd_saw(i);
		ch_cnt[i] = 0;
		ch_efx[i] = 0;
		ch_rst[i] = 0;
		ch_lastvol[i] = 0x00;
	}

	for( i = 0; i < 2; i++ )
	{
		noise_sw[i] = 0x00;
		noise_freq[i] = 0x00;
	}

	/* 全体ボリュームは最大 */
	poke(SND_VOL, 0xFF);
	
	enable_irq();
}

/* HuSIC 初期化 */
drv_init(songno)
char songno;
{
#asm
; drv_init()
#endasm
  drv_init_song(songno);
	init_pcmdrv();
	drv_setintr();
}


/*
　音量セット
*/
set_vol(ch, val)
int ch, val;
{
	/* ボリューム設定(>0x80) or エンベロープ設定  */
	if (val & 0x80)
	{
		volume_envadr[ch] = 0;
		mixvol(val & 0x1F);
	}
	else
	{
		val &= 0x7F;
		chg_cbank(ch_topbank);
		volume_envadr[ch]= *(sound_dat[2] + (val << 1));
		chg_cbank(ch_nowbank);
	}
}

/*
　ノート周波数セット
*/
set_note(ch, data)
char ch, data;
{
	char i;

	i = (data + 3) & 0x0F;
	seq_freq[ch] = freq[i & 0x0F] >> (data >> 4);
}

extern int seqproc[];




/* コマンド読み出し&実行 */
do_seq(ch)
int ch;
{
	int sd;
	int i, j;
  int seq_adrs, tmp;

#asm
; do_seq()
#endasm


    chg_cbank(ch_nowbank);

    while(1)
    {
#asm
;
; start command loop (do_seq)
;
#endasm

        /* カウンタ>0で待つ処理を行う */
        if (ch_cnt[ch])
            return;

				/* コマンドの取得 */
        sd = *seq_ptr;

				ch_lastcmd[ch] = sd;

#asm
;
; check key on
;
#endasm
        /* ノートであればキーオン処理 */
        if (sd < 0x90)
            break;


	/* コマンド確認とジャンプ */
	if (sd >= 0xe6)
	{
		/* E8以上であればテーブルからジャンプする */
		tmp = seqproc[sd - 0xe6];
#asm
;
; jump to command routine
; sax

  pha
  sax
  pha
  rts
#endasm

#asm
; $FF : トラック終了(現在未使用)
SEQ_FF:
#endasm
		/* $FF: トラック終了 */
		chg_cbank(ch_topbank);

		seq_ptr = *( song_loop_table + ( ch << 1 ) );
		ch_nowbank = *( song_loop_bank + ch );

		chg_cbank(ch_nowbank);

#asm
		jmp endpoint
#endasm

#asm
; $FE : 音色設定
SEQ_FE:
#endasm
/* $FE:音色設定 */
   j = *(++seq_ptr);
   seq_ptr++;
   if (j >= 0x80)
      j = 0xff;
   tone_sw[ch] = j;
	 reset_te(ch);

#asm
	jmp endpoint
#endasm

#asm
; $F9: ハードウェアスイープ(未使用)
SEQ_F9:
#endasm
    seq_ptr += 2;
#asm
	jmp endpoint
#endasm

#asm
; $FD: 音量設定
SEQ_FD:
#endasm
/* $FD:音量設定 引数:1 */
		j = *(++seq_ptr); seq_ptr++;
		ch_vol[ch] = j;
		set_vol(ch, j);

#asm
	jmp endpoint
#endasm

#asm
; $FC: 休符(REST)
SEQ_FC:
#endasm
/* $FC:休符 */
    j = *(++seq_ptr);
    seq_ptr++;

    mixvol(0);

    if (pcm_check(reg_ch))
        pcm_stop(reg_ch);

    volume_envadr[ch] = 0x00; /* envelope off */
    ch_cnt[ch] = j;
		return;
#asm
	jmp endpoint
#endasm

#asm
; $FB: LFOスイッチ　引数:1
SEQ_FB:
#endasm
    /* $FB: LFOスイッチ 引数:1 */
    j = *(++seq_ptr); seq_ptr++;
	lfo_sw[ch] = j;
	reset_lfo(ch);
#asm
	jmp endpoint
#endasm

#asm
; $FA: デチューン設定 引数:1
SEQ_FA:
#endasm

 /* $FA:デチューン設定 引数:1 */
	j = *( ++seq_ptr ); seq_ptr++;

	if (j == 0xff)
		detune[ch] = 0x00;
	else
		detune[ch] = j;

#asm
	jmp endpoint
#endasm

#asm
; $F8: ピッチエンベロープ
SEQ_F8:
#endasm
 /* $F8:ピッチエンベロープ */

 j = *(++seq_ptr);
 seq_ptr++;
 pitch_sw[ch] = j;
 reset_pe(ch);
#asm
	jmp endpoint
#endasm

#asm
; $F7: ノートエンベロープ
SEQ_F7:
#endasm
  /* $F7:ノートエンベロープ */
  j = *(++seq_ptr);
  seq_ptr++;
  note_sw[ch] = j;
	reset_ne(ch);
#asm
	jmp endpoint
#endasm

#asm
; $F4: ウェイト 引数:カウント
SEQ_F4:
#endasm
 /* $F4:ウェイト */
		j = *(++seq_ptr); seq_ptr++;
		ch_cnt[ch] = j;

#asm
	jmp endpoint
#endasm

#asm
; $F2: ノイズコマンド
SEQ_F2:
#endasm
/* $F2: ノイズコマンド */
		j = *(++seq_ptr); seq_ptr++;

		if (ch > 3)
		{
			if (j)
			{
				noise_sw[ ch - 4 ] = 0x01;
			}
			else
			{
				noise_sw[ ch - 4 ] = 0x00;
				poke( SND_NOI , 0 );
			}
		}

#asm
	jmp endpoint
#endasm

#asm
; $F1: 波形変更コマンド
SEQ_F1:
#endasm
 /* $F1: PCE 波形変更コマンド */
		j = *(++seq_ptr); seq_ptr++;
		chg_cbank( ch_topbank );
		snd_chg(j);
		chg_cbank( ch_nowbank );
        tone_sw[ch] = 0xff;

#asm
	jmp endpoint
#endasm

#asm
; $F0: パンコマンド
SEQ_F0:
#endasm

/* $F0: PCE パンコマンド */
		j = *(++seq_ptr); seq_ptr++;
		panpod[ ch ] = j;
		poke( SND_PAN, j );

#asm
	jmp endpoint
#endasm

#asm
; $EF: XPCMスイッチ
SEQ_EF:
#endasm

 /* $EF: XPCM switch */
    j = *(++seq_ptr); seq_ptr++;
	pcm_switch(ch, j);

#asm
	jmp endpoint
#endasm

#asm
; $EE: バンク切り替え(ジャンプ) コマンド
SEQ_EE:
#endasm

 /* $EE: バンク切り替えコマンド  */
		j = *(++seq_ptr); seq_ptr++;
		seq_adrs = *(seq_ptr) | *(seq_ptr+1)<<8;
		seq_ptr = seq_adrs;

		chg_cbank( j );
		ch_nowbank = j;

#asm
	jmp endpoint
#endasm

#asm
; $ED: HWLFOモード(FMLFO)
SEQ_ED:
#endasm
 /* $ED: HW LFO mode */
		j = *(++seq_ptr); seq_ptr++;
		if ( j == 0xff )
		{
			/* off */
			poke( SND_LTR , 0x00 );
		}
		else
		{
			/* bit7 as high is turned on the hardware LFO */
			poke( SND_LTR , j & 0x03 );
		}

#asm
	jmp endpoint
#endasm

#asm
; $EB: ポルタメント　引数:hi lo
SEQ_EB:
#endasm
	/* $EB: ポルタメント 引数:hi lo */
	j = *(++seq_ptr);
	ch_porthi[ch] = j;
	j = *(++seq_ptr);
	ch_portlo[ch] = j;

	ch_portcnt[ch] = 0;

	ch_efx[ch] |= EFX_PORT_ST;

	seq_ptr++;

#asm
	jmp endpoint
#endasm


#asm
; $E9: スラー: 引数無し
SEQ_E9:
#endasm
/* $E9: スラー : 引数なし */
	ch_efx[ch] |= EFX_SLAR;
	seq_ptr++;

#asm
	jmp endpoint
#endasm

#asm
; $E8: マスターボリューム
SEQ_E8:
#endasm

/* $E8: マスターボリューム 引数:*/
 j = *(++seq_ptr);
 poke(SND_VOL, j);
 seq_ptr++;

#asm
	jmp endpoint
#endasm

#asm
; $E7: リセット無視 引数:フラグ
SEQ_E7:
#endasm

/* $E7: リセット無視 引数:フラグ*/
 j = *(++seq_ptr);
 ch_rst[ch] = j;
 seq_ptr++;

#asm
	jmp endpoint
#endasm

#asm
; $E6: パンエンベロープ 引数:フラグ
SEQ_E6:
#endasm

/* $E6: パンエンベロープ 引数:フラグ*/
 j = *(++seq_ptr);
 multienv_sw[ch] = j;
 reset_multienv(ch);
 seq_ptr++;

#asm
	jmp endpoint
#endasm

#asm
; $EC: HW LFO周波数
SEQ_EC:
#endasm

 /* $EC: HW LFO(FMLFO) 周波数 */
		j = *(++seq_ptr); seq_ptr++;
		poke( SND_LFO , j );

#asm
SEQ_EA:
endpoint:
#endasm
    } /* if ... */
	else
	{
        /* リピートコマンド */
		if (sd == 0xa1)
		{
			/* repeat2 command */
			seq_ptr++;
			if (loop_cnt[ ch ] == 1)
			{
				loop_cnt[ ch ] = 0;

				seq_adrs = *(seq_ptr) | *(seq_ptr+1)<<8;
				seq_ptr  = seq_adrs;
			}
			else
			{
				seq_ptr += 2;
			}
		}
		if (sd == 0xa0)
		{
			/* repeat command */
			j = *(++seq_ptr); seq_ptr++;

			tmp = *( seq_ptr++ ); /* bank */
			seq_adrs =  *( seq_ptr ) | *( seq_ptr + 1 ) << 8;
			seq_ptr  += 2;

			if ( loop_cnt[ch] ==  1 )
			{
				loop_cnt[ch] = 0;
			}
			else
			{
				if ( loop_cnt[ch] > 1 )
					loop_cnt[ch]--;
				else
					loop_cnt[ch] = j-1;

				seq_ptr = seq_adrs;
				chg_cbank( tmp );
				ch_nowbank = tmp;
			}
		}
	}

    } /* while */

/************
  音階データ
 ***********/

#asm
;
; start key-on process
;
#endasm

		/* ノート番号とカウントのセット */
    note_data[ch] = sd;
    j = *(++seq_ptr); seq_ptr++;
		ch_cnt[ch] = j;

		/* PCMであるかどうか？ */
    if (pcm_check(ch))
    {
        chg_cbank( ch_topbank );

        tmp = xpcmdata + ( sd << 3 );
        pcm_play_data(ch, *(tmp), *(tmp + 2), *(tmp + 4));

        chg_cbank( ch_nowbank );

        set_vol(ch, ch_vol[ch]);

        return;
    }

    /***** エンベロープの設定 *****/
		if (!(ch_efx[ch] & EFX_SLAR))
		{
			if (!(ch_rst[ch] & RI_TE))
				reset_te(ch); /* 音色 */
			if (!(ch_rst[ch] & RI_NE))
				reset_ne(ch); /* ノート */
			if (!(ch_rst[ch] & RI_PE))
				reset_pe(ch); /* ピッチ */
			if (!(ch_rst[ch] & RI_LFO))
				reset_lfo(ch); /* LFO */
			if (!(ch_rst[ch] & RI_PAN))
				reset_multienv(ch); /* マルチエンベロープ */
		}

		/********/

		/* バンクをトップに */
		chg_cbank(ch_topbank);

    /* ノイズモード */
    if (ch > 3 && noise_sw[ch - 4])
    {
        noise_freq[ch - 4] = sd & 0x1F;
        poke(SND_NOI, 0x80 | sd & 0x1F);

        set_vol(ch, ch_vol[ch]);

				/* バンクを戻して */
				chg_cbank(ch_nowbank);
				return;
    }

    /* 0x20 = o4c , 0x30 = o5c */

    /* j =  0x0c * (sd>>4);
       j += (sd+3) & 0x0f; */

    /* 音程設定 */
    set_note(ch, sd);

    /* デチューン */
    if (detune[ch])
    {
        if (detune[ch] & 0x80)
            seq_freq[ch] -= detune[ch] & 0x7f;
        else
            seq_freq[ch] += detune[ch] & 0x7f;
    }

    /* 周波数データをレジスタにセットする */
    poke( SND_ROU, ( seq_freq[ ch ] >> 8 ) & 0x0f );
    poke( SND_FIN,   seq_freq[ ch ]        & 0xff );

		/* スラーでなければ音量設定 */
		if (!(ch_efx[ch] & EFX_SLAR))
		{
    	/* 音量設定 */
    	set_vol(ch, ch_vol[ch]);
		}

		/* ポルタメント開始かどうか */
		/* ポルタメント開始 */
		if (ch_efx[ch] & EFX_PORT_ST)
		{
					ch_efx[ch] = (ch_efx[ch] & ~(EFX_PORT_ST)) | EFX_PORT;
		}
		else
		{
			ch_efx[ch] &= ~(EFX_PORT | EFX_PORT_ST);
		}

		/* スラーフラグリセット */
		ch_efx[ch] &= ~(EFX_SLAR);

		/* バンクを戻して終了 */
		chg_cbank(ch_nowbank);
}

/* ジャンプテーブル */
#asm
	.proc _seqproc
; RTSを利用したジャンプ。RTSはアドレス+1から実行される。
_seqproc:
	dw   SEQ_E6 - 1
	dw   SEQ_E7 - 1
	dw   SEQ_E8 - 1
	dw   SEQ_E9 - 1
	dw   SEQ_EA - 1
	dw	 SEQ_EB - 1
	dw   SEQ_EC - 1
	dw   SEQ_ED - 1
	dw   SEQ_EE - 1
	dw   SEQ_EF - 1
	dw   SEQ_F0 - 1
	dw   SEQ_F1 - 1
	dw   SEQ_F2 - 1
	dw   endpoint - 1 ; f3
	dw   SEQ_F4 - 1
	dw   endpoint - 1 ; f5
	dw   endpoint - 1 ; f6
	dw   SEQ_F7 - 1   ; f7
	dw   SEQ_F8 - 1
	dw   SEQ_F9 - 1
	dw   SEQ_FA - 1
	dw   SEQ_FB - 1
	dw   SEQ_FC - 1
	dw   SEQ_FD - 1
	dw   SEQ_FE - 1
	dw   SEQ_FF - 1
	.endp

#endasm




/* 音色エンベロープリセット */
reset_te(ch)
int ch;
{
	/* 音色エンベロープ */
	if (tone_sw[ch] == 0xff)
			tone_envadr[ch] = 0;
	else
	{
		chg_cbank(ch_topbank);
		tone_envadr[ch] = *(sound_dat[11] + (tone_sw[ch]<<1));
		chg_cbank(ch_nowbank);
	}
}

/* マルチエンベロープリセット */
reset_multienv(ch)
int ch;
{
	int tmp;

	if (multienv_sw[ch] == 0xff)
			multi_envadr[ch] = 0;
	else
	{
		chg_cbank(ch_topbank);
		tmp = *(sound_dat[13] + (multienv_sw[ch]<<1));
		multi_envcnt[ch] = *(tmp);
		multi_envadr[ch] = tmp + 4;

		chg_cbank(ch_nowbank);
	}
}


/* ノートエンベロープリセット */
reset_ne(ch)
int ch;
{
  /* ノートエンベロープ */
  if (note_sw[ch] == 0xff)
        note_envadr[ch] = 0;
  else
	{
		chg_cbank(ch_topbank);
		note_envadr[ch] = *(sound_dat[9] + (note_sw[ch]<<1));
		chg_cbank(ch_nowbank);
	}
}

/* ピッチエンベロープリセット */
reset_pe(ch)
int ch;
{
	/* ピッチエンベロープ */
	if (pitch_sw[ch] == 0xff)
			pitch_envadr[ch] = 0;
	else
	{
		chg_cbank(ch_topbank);
		pitch_envadr[ch] = *(sound_dat[4] + (pitch_sw[ch]<<1));
		chg_cbank(ch_nowbank);
	}
}

/* LFOリセット */
reset_lfo(ch)
int ch;
{
    /* LFO */
    if (lfo_sw[ch] == 0xff)
        lfo_cnt[ch] = 0;
    else
    {
			chg_cbank(ch_topbank);
      lfo_step[ch] = 0;
      lfo_cnt[ch] = *(sound_dat[6] + (lfo_sw[ch]<<2));
			chg_cbank(ch_nowbank);
    }
}

/********************

********************/

divxt(x,y)
char x,y;
{
	int i,j;

	j = 0;
	for( i = 0; j < x; i++)
		j += y;

	return i;
}


/********************
 ノートエンベロープ
*********************/
drv_noteenv(sch)
int sch;
{
    int tmp;
    char val, mod, num;

    val = *note_envadr[sch];
    note_envadr[ sch ]++;

    /* エンベロープ終端 */
    if (val == 0xff)
    {
        note_envadr[ sch ] =
            *( sound_dat[ 10 ] + ( note_sw[ sch ] << 1 ) );
        val = *note_envadr[ sch ];
        note_envadr[ sch ]++;
    }

    mod = note_data[ sch ];
    num = val & 0x7f;
    tmp = ( num / 12 ) << 4;

    if ( val & 0x80 )
    {
        mod -= tmp;
        tmp = num % 12;
        if ((mod & 0x0f) >= tmp)
        { mod -= tmp; }
        else
        { mod -= 0x04;
          mod -= tmp; }
    }
    else
    {
        mod += tmp;
        tmp = num % 12;
        mod += tmp;
        if ((mod & 0x0f) >= 0x0c)
        { mod += 0x04; }
    }

    note_data[sch] = mod;

    set_note(sch, mod);
}




/********************
 音色エンベロープ
 *********************/
drv_toneenv(sch)
int sch;
{
    int tmp;
    char val;

    val = *tone_envadr[sch];
    tone_envadr[sch]++;

    /* エンベロープ終端 */
    if (val == 0xff)
    {
        tone_envadr[sch] = *(sound_dat[12] + (tone_sw[sch] << 1));
        val = *tone_envadr[ sch ];
        tone_envadr[sch]++;
    }

		/* 波形変更 */
    snd_chg(val);
}

/********************
 パンエンベロープ
 *********************/
drv_panenv(sch)
int sch;
{
    int tmp;
    char val;

		/* カウントが0ならループ位置へ移動 */
		if (!multi_envcnt[sch])
		{
			tmp = *(sound_dat[13] + (multienv_sw[sch] << 1));
			multi_envcnt[sch] = *(tmp + 2);
			multi_envadr[sch] = *(sound_dat[14] + (multienv_sw[sch] << 1));
		}

		/* カウントが0でなければ読みだして書き込む */
		if (multi_envcnt[sch])
		{
    	val = *(multi_envadr[sch]++);
			multi_envcnt[sch]--;
			/* パン変更 */
			panpod[sch] = val;
			poke(SND_PAN, val);
		}
}

/********************
 ピッチエンベロープ
*********************/
drv_pitchenv( sch )
int sch;
{
	char val;
	/* type 'char' is important!! */

    val = *pitch_envadr[ sch ];
    pitch_envadr[ sch ]++;

    if (val == 0xff)
    {
        pitch_envadr[ sch ] = *(sound_dat[5] + (pitch_sw[sch] << 1));
        val = *pitch_envadr[ sch ];
        pitch_envadr[ sch ]++;
    }


    if (sch > 3 && noise_sw[ sch - 4 ] )
    {
        if (val & 0x80)
            noise_freq[ sch - 4 ] -= val & 0x7f;
        else
            noise_freq[ sch - 4 ] += val;

        poke(SND_NOI,0x80 | noise_freq[ sch - 4 ] & 0x1f);
    }

    if (val & 0x80)
        seq_freq[ sch ] -= val & 0x7f;
    else
        seq_freq[ sch ] += val;
}


/********************
 LFO
*********************/
drv_lfoseq( sch )
int sch;
{
    int tmp;

    tmp = sound_dat[6] + (lfo_sw[sch] << 2);

    /* tmp+1 = speed
		   tmp+2 = depth */

    switch( lfo_step[ sch ] )
    {
        case 0x00:
            if ( lfo_cnt[ sch ] )
                lfo_cnt[ sch ]--;
            else
            {
                lfo_step[ sch ]  = 1;
                lfo_cnt[ sch ] = *(tmp+1);

                lfo_ct2[ sch ] = divxt(*(tmp+1),*(tmp+2));
                lfo_stp[ sch ] = lfo_ct2[ sch ];

                lfo_lev[ sch ] = divxt(*(tmp+2),*(tmp+1));

                if (!lfo_lev[ sch ])
                    lfo_lev[ sch ] = 1;

                /* 1step = lfo_speed/lfo_depth
                 1level = lfo_depth/lfo_speed */

            }
            break;
        case 0x01:
            if (lfo_cnt[ sch ])
                lfo_cnt[ sch ]--;

            if (lfo_ct2[ sch ])
                lfo_ct2[ sch ]--;
            else
            {
                seq_freq[ sch ] -= lfo_lev[ sch ];
                lfo_ct2[ sch ]   = lfo_stp[ sch ];

                if (!lfo_cnt[ sch ])
                {
                    lfo_cnt[ sch ]= *( tmp + 1 );
                    lfo_step[ sch ] = 0x02;
                }
            }
            break;
        case 0x02:

            if (lfo_cnt[ sch ])
                lfo_cnt[ sch ]--;

            if (lfo_ct2[ sch ])
                lfo_ct2[ sch ]--;
            else
            {

                seq_freq[ sch ] += lfo_lev[ sch ];
                lfo_ct2[ sch ]   = lfo_stp[ sch ];

                if (!lfo_cnt[ sch ])
                {
                    lfo_cnt[ sch ] = *(tmp+1);
                    lfo_step[ sch ]  = 0x03;
                }
            }
            break;
        case 0x03:
            if (lfo_cnt[ sch ])
                lfo_cnt[ sch ]--;

            if (lfo_ct2[sch])
                lfo_ct2[sch]--;
            else
            {
                seq_freq[sch] += lfo_lev[sch];
                lfo_ct2[sch]  =  lfo_stp[sch];

                if (!lfo_cnt[sch])
                {
                    lfo_cnt[sch] = *(tmp+1);
                    lfo_step[sch]  = 0x04;
                }
            }
            break;
        case 0x04:
            if (lfo_cnt[sch])
                lfo_cnt[sch]--;

            if (lfo_ct2[sch])
                lfo_ct2[sch]--;
            else
            {
                seq_freq[sch] -= lfo_lev[sch];
                lfo_ct2[sch]   = lfo_stp[sch];

                if (!lfo_cnt[sch])
                {
                    lfo_cnt[sch] = *(tmp+1);
                    lfo_step[sch]  = 0x01;
                }
            }
            break;
    } /* switch */
}

/********************
 フレーム実行部分
*********************/
drv_intr()
{
    int   sch;
    char  val;
		char  snd_update;
		int   diff;

    seq_ptr = 0xfffe;

    for(sch = 0; sch < 6; sch++)
    {

#asm
;
; TODO: check this
;
#endasm

        /* チャンネル選択 */
        reg_ch = sch;
        poke(SND_SEL, sch);

        /* カウンタが0の場合、次のシーケンスを実行 */
        if (ch_cnt[sch] == 0)
        {
	       	seq_ptr = seq_pos[sch];

            ch_nowbank = ch_bank[sch];

            do_seq(sch);
            seq_pos[sch] = seq_ptr;

            ch_bank[sch] = ch_nowbank;
            chg_cbank(ch_topbank);
        }

        /* カウンタを減算する */
        ch_cnt[ sch ]--;

        /* 音量エンベロープ */
        if (volume_envadr[sch])
        {
            val = *volume_envadr[sch];
            volume_envadr[sch]++;

            if (val == 0xff)
            {
                val = ch_vol[ sch ] & 0x7f;
                volume_envadr[ sch ] = *(sound_dat[3] + (val << 1));
                val = *volume_envadr[ sch ];
                volume_envadr[ sch ]++;
            }

            mixvol(val & 0x1f);
        }

				snd_update = 0;

        /* エンベロープ機能 */
        if (note_envadr[sch])
				{
					snd_update = 1;
          drv_noteenv(sch);
				}

        if (tone_envadr[sch])
				{
					snd_update = 1;
          drv_toneenv(sch);
				}

        if (pitch_envadr[sch])
				{
					snd_update = 1;
          drv_pitchenv(sch);
				}

				if (multi_envadr[sch])
				{
          drv_panenv(sch);
				}

        if (lfo_sw[sch] != 0xff)
				{
					snd_update = 1;
          drv_lfoseq(sch);
				}

				/* ポルタメント */
				if (ch_efx[sch] & EFX_PORT)
				{
						snd_update = 1;
						/* porthiは符号付き */
						if (ch_porthi[sch] & 0x80)
							diff = (0xFF00 | ch_porthi[sch]);
						else
							diff = ch_porthi[sch];

						/* portloは1/128カウンタの値 */
						ch_portcnt[sch] += (ch_portlo[sch] & 0x7F);
						if (ch_portcnt[sch] & 0x80)
						{
							if (ch_portlo[sch] & 0x80)
								diff--;
							else
								diff++;

							ch_portcnt[sch] &= 0x7F;
						}
						seq_freq[sch] += diff;
				}

				/* 周波数変化につき更新 */
				if (snd_update)
				{
					poke( SND_ROU, (seq_freq[sch]>>8) & 0x0f );
					poke( SND_FIN,  seq_freq[sch]     & 0xff );
				}

    } /* end of for(..) */
}
