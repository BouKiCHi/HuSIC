#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>

// 基本定義
#ifndef _PATH_MAX
#define _PATH_MAX  2048
#endif

#ifdef _WIN32
#define _PATH_DELIM '\\'
#else
#define _PATH_DELIM '/'
#endif

// 拡張子
#define EXT_PD8 ".PD8"
#define EXT_PD4 ".PD4"

// PCMカットサイズ
#define DEF_CUTSIZE (65535 - 1)

// デフォルトの出力PCM周波数
#define DEF_PCMFREQ 6992

// PCMバッファ定数
#define PCM_BUFBITS 8
#define PCM_BUFMASK 255

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

// pcm関連
struct pcm_t
{
  FILE *outfp; // ファイルポインタ
  float count; // 出力カウント
  float rate; // 出力レート

  int outfreq; // 出力周波数
  int outbit; // サンプルビットサイズ

  int cutsize; // 中断サイズ
  int outsize; // 出力サイズ


  int shift; // シフト

  int leftbits; // 現在のバッファの残りビット数
  BYTE buffer; // 出力バッファ

  long mix_buffer; // 入力サンプルバッファ
};

// WAV処理構造体
struct wavfmt_t
{
  FILE *fp; // ファイルポインタ
  DWORD pcmsize; // PCMサイズ
  int freq; // 入力周波数
  int bits; // 入力サンプルビット数
  int ch; // 入力チャンネル
	
	long len; // 長さ

  // PCM補間不使用
  int no_inter;

  // デバッグ
  int debug;

  // PCM構造体
  struct pcm_t pcm;
} wfmt;

// dword読み出し(mem)
DWORD read_dword_mem(BYTE *ptr)
{
    return
    ((DWORD) ptr[0]) |
    ((DWORD) ptr[1]) << 8 |
    ((DWORD) ptr[2]) << 16 |
    ((DWORD) ptr[3]) << 24;
}

// dword読み出し(file)
DWORD read_dword(FILE *fp)
{
    BYTE buf[8];

    fread(buf, 4, 1, fp);

  return read_dword_mem(buf);
}

// word読み出し(file)
WORD read_word_mem(BYTE *ptr)
{
    return ((DWORD)ptr[0]) | ((DWORD)ptr[1]) << 8;
}

// word読み出し(file)
WORD read_word(FILE *fp)
{
    BYTE buf[8];

    fread(buf, 2, 1, fp);

    return read_word_mem(buf);
}

// フォーマット読み出し(file)
int wav_getfmt(struct wavfmt_t *lf)
{
    DWORD f_pos;
    DWORD c_size;
    BYTE buf[16];

    // ファイルが開かれていない
    if (!lf->fp)
    {
      printf("Error: file is not opened!\n");
      return -1;
    }

    fread(buf, 4, 1, lf->fp);

    // RIFFではない
    if (memcmp(buf, "RIFF", 4) != 0)
    {
      printf("Error: not RIFF!\n");
      return -1;
    }

    read_dword(lf->fp); // size of RIFF

    // WAVEfmt部分
    fread(buf, 8, 1, lf->fp); // WAVEfmt

    // WAVEfmtではない
    if (memcmp(buf, "WAVEfmt", 7) != 0)
    {
      printf("Error: not WAVEfmt!\n");
      return -1;
    }

    // チャンクサイズと現在位置
    c_size = read_dword(lf->fp);
    f_pos = ftell(lf->fp);

    // PCMフォーマットではない
    if (read_word(lf->fp) != 1)
    {
      printf("Error: not PCM format!\n");
      return -1;
    }

    // チャンネル数
    // サンプリング周波数
    lf->ch = read_word(lf->fp);
    lf->freq = (int)read_dword(lf->fp);

    // データレート(byte / sec)
    read_dword(lf->fp);
    // ブロックサイズ
    read_word(lf->fp);

    // サンプル毎のビット数(bit / sample)
    lf->bits = read_word(lf->fp);

    if (lf->bits != 16 && lf->bits != 8)
    {
      printf("Error: sampling bits %d is not supported!!\n", lf->bits);
      return -1;
    }

    // ファイル位置を調整
    fseek(lf->fp, f_pos + c_size, SEEK_SET);

    printf("pos:%ld csize:%ld\n", f_pos, c_size);

    //	lf->freq=44100;
    return 0;
}

// データチャンクを検出
int wav_find_data(struct wavfmt_t *lf)
{
    BYTE buf[16];

    do
    {
        // dataチャンクを検出
        fread(buf, 4, 1, lf->fp);
        if (memcmp(buf, "data", 4) == 0)
        {
            lf->pcmsize = read_dword(lf->fp);
            return 0;
        }
        else
        {
            long len = read_dword(lf->fp);

            // チャンクサイズが正しくない
            if (len == 0)
                break;
            fseek(lf->fp, len, SEEK_CUR);
        }
    }while(!feof(lf->fp));

    return -1;
}

#if 0
// PCM出力
void wav_buffer_out(struct wavfmt_t *lf, long mix_buf)
{
	// ビット数
	int use_bits = (PCM_BUFBITS - lf->pcm.leftbits);
	
	// バッファにコピーする
	lf->pcm.buffer |= (mix_buf >> use_bits);
	
	// サンプリングビットを引く
	lf->pcm.leftbits -= lf->pcm.outbit;
	
	// 残りのビットを空にする
	if (lf->pcm.leftbits > 0)
	{
		lf->pcm.buffer &= (PCM_BUFMASK << lf->pcm.leftbits);
	}
	else
	{
		// バッファが埋まった
		fputc(lf->pcm.buffer, lf->pcm.outfp);
		
		// 出力サイズ増加
		lf->pcm.outsize++;
		
		// バッファクリア
		lf->pcm.buffer = 0;
		lf->pcm.leftbits += PCM_BUFBITS;
		
		// 入力サンプルのあまりを出力する
		lf->pcm.buffer |= (mix_buf << (PCM_BUFBITS - use_bits));
		lf->pcm.buffer &= (PCM_BUFMASK << (lf->pcm.leftbits));
		
	}
}
#endif



// PCMシフト出力
void wav_buffer_shift_out(struct wavfmt_t *lf, long mix_buf)
{
	// ビット数
	int use_bits = (PCM_BUFBITS - lf->pcm.leftbits);
	
	// バッファにコピーする
	lf->pcm.buffer |= (mix_buf >> use_bits);
	
	// サンプリングビットを引く
	lf->pcm.leftbits -= lf->pcm.outbit;
	
	// 残りのビットを空にする
	if (lf->pcm.leftbits > 0)
	{
		lf->pcm.buffer &= (PCM_BUFMASK << lf->pcm.leftbits);
	}
	else
	{
		// バッファが埋まった
		fputc(lf->pcm.buffer, lf->pcm.outfp);
		
		// 出力サイズ増加
		lf->pcm.outsize++;
		
		// バッファクリア
		lf->pcm.buffer = 0;
		lf->pcm.leftbits += PCM_BUFBITS;
		
		// 入力サンプルのあまりを出力する
		lf->pcm.buffer |= (mix_buf << (PCM_BUFBITS - use_bits));
		lf->pcm.buffer &= (PCM_BUFMASK << (lf->pcm.leftbits));
		
	}
}
// PCM出力
void wav_buffer_out(struct wavfmt_t *lf, long mix_buf)
{
  if (lf->pcm.outbit != 5) { 
    wav_buffer_shift_out(lf, mix_buf);
    return;
  }

	// ビット数
	int use_bits = (PCM_BUFBITS - lf->pcm.outbit);
	// バッファにコピーする
	lf->pcm.buffer = (mix_buf >> use_bits) & (0xff >> use_bits);

	fputc(lf->pcm.buffer, lf->pcm.outfp);
		
  // 出力サイズ増加
  lf->pcm.outsize++;
  
  // バッファクリア
  lf->pcm.buffer = 0;
  lf->pcm.leftbits += PCM_BUFBITS;  
}


// PCM出力準備
void wav_push_sample(struct wavfmt_t *lf)
{
  // カウントを加算する
  lf->pcm.count += lf->pcm.rate;

  // 出力可能ではない
  if (lf->pcm.count < 1)
    return;

	long mix_buf = 0;

  // サンプルビット数を8ビットに下げる
  if (lf->bits == 16)
  {
		mix_buf = lf->pcm.mix_buffer;
    mix_buf /= 256;
    mix_buf += (UCHAR_MAX / 2);
  }
	else
	{
		mix_buf = lf->pcm.mix_buffer;
	}

	if (lf->debug)
		printf("%ld\n", mix_buf >> (8 - lf->pcm.outbit));

  // カウントを減算する
  lf->pcm.count -= 1;

	// バッファ出力
	wav_buffer_out(lf, mix_buf);
}


// PCM終端出力
void wav_push_term(struct wavfmt_t *lf)
{
  // バッファを出力
  fputc(lf->pcm.buffer, lf->pcm.outfp);
  // 終端
  fputc(0x00, lf->pcm.outfp);
}

// WAVサンプル読み出し
void wav_read_sample(struct wavfmt_t *lf)
{
	BYTE buf[8];
	
	// ファイルポインタがNULLなら何もしない
	if (!lf->fp)
		return;

	// ステレオ処理
	if (lf->ch == 2)
	{
		if (lf->bits == 16)
		{
			// 16bit
			fread(buf, 4, 1, lf->fp);
			lf->len -= 4;
			
			// データ読み出し
			long buffer;
			buffer = (short)read_word_mem(buf);
			buffer += (short)read_word_mem(buf + 2);
			
			// 2つの波形の平均
			buffer /= 2;
			
			// PCM補間をしない
			if (lf->no_inter)
				lf->pcm.mix_buffer = buffer;
			else
				// ２つの波形の平均
				lf->pcm.mix_buffer = (lf->pcm.mix_buffer + buffer) / 2;
		}
		else if (lf->bits == 8)
		{
			// 8bit
			fread(buf, 2, 1, lf->fp);
			lf->len -= 2;
			
			// データ読み出し
			long buffer;
			buffer = *(BYTE *)buf;
			buffer += *(BYTE *)buf + 1;
			
			// 2つの波形の平均
			buffer /= 2;
			
			// PCM補間をしない
			if (lf->no_inter)
				lf->pcm.mix_buffer = buffer;
			else
				// ２つの波形の平均
				lf->pcm.mix_buffer = (lf->pcm.mix_buffer + buffer) / 2;
		}
	}
	else // モノラル
	{
		// 16bit
		if (lf->bits == 16)
		{
			fread(buf, 2, 1, lf->fp);
			lf->len -= 2;
			
			// データ読み出し
			long buffer = (short)read_word_mem(buf);
			
			// PCM補間をしない
			if (lf->no_inter)
				lf->pcm.mix_buffer = buffer;
			else
				// ２つの波形の平均
				lf->pcm.mix_buffer = (lf->pcm.mix_buffer + buffer) / 2;
			
		}
		else if (lf->bits == 8) // 8bit
		{
			fread(buf, 1, 1, lf->fp);
			lf->len -= 1;
			
			// データ読み出し
			long buffer = *(BYTE *)buf;
			
			// PCM補間をしない
			if (lf->no_inter)
				lf->pcm.mix_buffer = buffer;
			else
				// ２つの波形の平均
				lf->pcm.mix_buffer = (lf->pcm.mix_buffer + buffer) / 2;
		}
	}

}


// WAVデータ変換
void wav_convert(struct wavfmt_t *lf)
{

    // 表示カウンタ
    long info_count = 0;

    // PCMの周波数レートを設定
    lf->pcm.count = 0;
    lf->pcm.rate = lf->pcm.outfreq / (float)lf->freq;

    // 出力サイズ
    lf->pcm.outsize = 0;
    lf->pcm.shift = 0;

    // バッファクリア
    lf->pcm.buffer = 0;
    lf->pcm.mix_buffer = 0;
    lf->pcm.leftbits = PCM_BUFBITS;

		// 長さ設定
    lf->len = lf->pcmsize;

    while(lf->len > 0)
    {
			// サンプル読み出し
			wav_read_sample(lf);

			// PCM出力
			wav_push_sample(lf);

			// ファイルサイズが大きすぎるので中断する
			if (lf->pcm.outsize >= lf->pcm.cutsize)
				break;

			// 出力 パーセント表示
			if (info_count)
			{
					info_count--;
			}
			else
			{
				if (lf->pcmsize != lf->len)
				{
						printf("out : %3ld%%\r",
									 ( ( lf->pcmsize - lf->len ) * 100 ) / lf->pcmsize );
				}
				info_count = 1000;
			}

    } // ループ終了

    printf("out : 100%%\n");

    // 終端出力
    wav_push_term(lf);

}


// ファイル名作成
void make_filename(char *dest, const char *name, const char *ext)
{
	strcpy(dest, name);
	
	char *p = strrchr(dest, _PATH_DELIM);
	
	if (!p)
		p = dest;
	
	p = strchr(p, '.');
	
	if (p)
		strcpy(p, ext);
	else
		strcat(p, ext);
	
}

// ファイルを閉じる
void wav_close(struct wavfmt_t *lf)
{
	// 出力ファイル
	if (lf->pcm.outfp)
		fclose(lf->pcm.outfp);

	lf->pcm.outfp = NULL;
	
	// 入力ファイル
	if (lf->fp)
		fclose(lf->fp);
	
	lf->fp = NULL;
}


// ファイルを開く
void wav_write_pcm(struct wavfmt_t *lf, const char *inname, const char *ofn)
{
	
	// 出力ファイル名
	char outname_body[_PATH_MAX];
	// 出力ファイル名ポインタ
	const char *outname = NULL;
	
	// 出力ファイルを開く
	if (!ofn)
	{
		outname = outname_body;
		// 拡張子
		char *ext = (lf->pcm.outbit == 5 ? EXT_PD8 : EXT_PD4);
		
		// ファイル名
		make_filename(outname_body, inname, ext);
	}
	else
	{
		outname = ofn;
	}
	
	// 表示
	printf("PCM:%s Freq:%d %dBits\n",
				 outname,
				 lf->pcm.outfreq,
				 lf->pcm.outbit);
	
	// ファイル開く
	lf->pcm.outfp = fopen(outname, "wb");
	
}

// 仕様説明
void usage(void)
{
  printf(
    "Usage WAV2PD4 [options...] files... \n"
    "\n"
    " options...\n"
    " --5bit   use 5bit output mode\n"
		" --limits <n> limits output size to n\n"
		" --maketest  make testdata file\n"
    " -n, --ni   no use interpolation\n"
    " -f <n>, --freq <n>  set output frequency to freq\n"
    " -o <filename>  output to filename\n"
    "\n"
    " -h, --help  display this text\n"
  );
}

// メイン
int main(int argc, char *argv[])
{
  char   *ofn = NULL;
  struct wavfmt_t wavf;
  int   i;
  int flag_use5bit = 0;

  printf("WAV to PD4 comverter ver 0.5 by BouKiCHi\n");
  printf("Built at %s\n", __DATE__);

  if (argc == 1)
  {
    usage();
    return -1;
  }

  // 初期化
  memset(&wavf, 0 , sizeof(wavf));

  // 初期設定
  wavf.pcm.cutsize = DEF_CUTSIZE;
  wavf.pcm.outfreq = DEF_PCMFREQ;

  wavf.freq = 11025;
  wavf.pcm.outbit = 4;
	
	int flag_maketest = 0;

  // オプション処理
  while(1)
  {
  	struct option long_opts[] =
  	{
  		{"5bit", 0, NULL, 0},
      {"debug", 0, NULL, 1},
      {"limits", 1, NULL, 2},
			{"maketest", 0, NULL, 3},
      {"ni", 0, NULL, 'n'},
  		{"freq", 1, NULL, 'f'},
  		{"help", 0, NULL, 'h'},
  		{0, 0, 0, 0}
  	};

    int opt_idx = 0;
    int opt = getopt_long(argc, argv, "no:f:h", long_opts, &opt_idx);

    if (opt == -1)
      break;

		
    switch(opt)
    {
      case 0: // --5bit mode
        flag_use5bit = 1;
      break;
      case 1: // --debug
        wavf.debug = 1;
      break;
      case 2: // --limits
				wavf.pcm.cutsize = atoi(optarg);
      break;
			case 3: // --testdata
				flag_maketest = 1;
				break;
      case 'n': // --ni
        wavf.no_inter = 1;
      break;
				
      case 'o': // -o
        ofn = optarg;
      break;
      case 'f': // --freq
        wavf.pcm.outfreq = atoi(optarg);
      break;
      case 'h':
      default:
        usage();
        return -1;
      break;
    }
  }
	
	// テストデータ
	if (flag_maketest) {
		wavf.pcm.outbit = 5;

		// 出力ファイルを開く
		wav_write_pcm(&wavf, "testdata.pd5", NULL);
		
		//　ファイルポインタチェック
		if (!wavf.pcm.outfp) goto err_outfile;
		
		// バッファ出力
		for(i = 0; i < 5120; i++) wav_buffer_out(&wavf, (i & 0x1f) << 3); 
		
		// 閉じる
		wav_close(&wavf);
		return 0;
	}

  // 逐次処理
  for(i = optind; i < argc; i++) {
    char *inname = argv[i];

    // ファイルを開く
    printf("File:%s\n", inname);

    wavf.fp = fopen(inname, "rb");
    if (!wavf.fp) {
        printf("Error : File open error\n");
        goto err_end;
    }

    // フォーマットの取得
    if (wav_getfmt(&wavf)) {
        printf("Error : Unsupported format file\n");
        goto err_end;
    }

    // 出力表示
    printf(
				"Input: Freq:%d Bits:%d CH:%d\n",
				(int)wavf.freq, (int)wavf.bits, (int)wavf.ch
    );

    // データチャンク検索
    if (wav_find_data(&wavf)) {
        printf("Error : data chank not found\n");
        goto err_end;
    }
		
		// 出力サンプルビットサイズ
		wavf.pcm.outbit = flag_use5bit ? 5 : 4;

		// 出力ファイルを開く
		wav_write_pcm(&wavf, inname, ofn);
		
		//　ファイルポインタチェック
    if (!wavf.pcm.outfp) goto err_outfile;

    // 変換
    wav_convert(&wavf);

		// ファイルを閉じる
		wav_close(&wavf);

  } // ..for

  return 0;

// エラー終了
err_outfile:
	printf("Error : Could not open outfile\n");
	goto err_end;
	
err_end:
	wav_close(&wavf);

  return -1;
}
