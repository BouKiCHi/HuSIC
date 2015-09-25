#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct wav_fmt
{
    unsigned long pcmsize;
    unsigned int freq;
    unsigned int bit;
    unsigned int ch;
} wfmt;

unsigned long dword_le(unsigned char *ptr)
{
    return
    ((unsigned long) ptr[0] ) |
    ((unsigned long) ptr[1] ) << 8 |
    ((unsigned long) ptr[2] ) << 16 |
    ((unsigned long) ptr[3] ) << 24;
}



unsigned long read_dword(FILE *fp)
{
    unsigned char buf[8];
    
    fread(buf, 4, 1, fp);
    
    return dword_le(buf);
}

unsigned short word_le(unsigned char *ptr)
{
    return ((unsigned long)ptr[0]) |
    ((unsigned long)ptr[1]) << 8;
}


unsigned short read_word(FILE *fp)
{
    unsigned char buf[8];
    
    fread(buf, 2, 1, fp);
    
    return word_le(buf);
}


int wav_getfmt(FILE *fp,struct wav_fmt *lf)
{
    unsigned long f_pos;
    unsigned long c_size;
    unsigned char buf[10];
    
    if (!fp) return -1;
    
    fread(buf,4,1,fp);
    if (memcmp(buf,"RIFF",4) !=0 ) return -1; // not RIFF
    read_dword(fp); // size of RIFF
    
    fread(buf,8,1,fp); // WAVEfmt
    
    if (memcmp(buf,"WAVE",4) != 0 ) return -1; // not WAVE
    
    c_size = read_dword(fp); // chunk size
    f_pos = ftell(fp); // current position
    
    if ( read_word(fp) != 1 ) return -1; // not PCM
    
    lf->ch = read_word(fp); // Number of channels
    lf->freq = read_dword(fp); // Sampling frequency
    
    read_dword(fp);  // Data rate ( bytes/sec )
    read_word(fp); // Block size
    
    lf->bit = read_word(fp); // Bit per sample
    
    fseek(fp, f_pos + c_size, SEEK_SET);
    
    printf("pos:%ld csize:%ld\n",f_pos, c_size);
    
    //	lf->freq=44100;
    return 0;
}

int wav_setpos_dat(FILE *fp,struct wav_fmt *lf)
{
    unsigned char buf[10];
    long lslen;
    
    do
    {
        fread(buf,4,1,fp);
        if (memcmp(buf,"data",4) == 0)
        {
            lf->pcmsize = read_dword(fp);
            return 0;
        }
        else
        {
            lslen = read_dword(fp);
            
            if (lslen == 0)
                break; // chunk size is not correct
            fseek(fp, lslen, SEEK_CUR);
        }
        
    } while( ! feof(fp) );
    return -1;
}

void wav_convert(
    FILE *fp, struct wav_fmt *lf, FILE *wp, long freq, long cutsize)
{
    long mixch, len, cnt, outptr;
    unsigned char chbuf;
    unsigned char mxsft, outch;
    unsigned char buf[8];
    float frq_rate, frq_cnt=0;
    
    frq_rate = freq/(float)lf->freq;
    
    //	printf("%d:%d = %f\n",freq,lf->freq,frq_rate);
    
    outptr = cnt = mxsft = 0;
    len = lf->pcmsize;
    
    while(len > 0)
    {
        
        if (lf->ch == 2)
        {
            // stereo
            
            if (lf->bit == 16)
            { // 16bit
                fread(buf, 4, 1, fp);
                len -= 4;
                mixch  = word_le(buf);
                mixch += word_le(buf + 2);
                mixch >>= 13;
                
            }
            else
            { // probably 8bit
                fread(buf, 2, 1, fp);
                len -= 2;
                mixch = *(unsigned char *)buf;
                mixch += *(unsigned char *)buf + 1;
                mixch >>= 5;
            }
        }
        else
        {
            // mono
            if (lf->bit == 16)
            {
                // 16bit
                fread(buf, 2, 1, fp);
                len -= 2;
                mixch = word_le(buf);
                mixch >>= 12;
                
            }
            else
            {
                // probably 8bit
                fread(buf, 1, 1, fp);
                len -= 1;
                mixch = *(unsigned char *)buf;
                mixch >>= 4;
            }
        }
        
        frq_cnt += frq_rate;
        
        while(frq_cnt >= 1)
        {
            frq_cnt -= 1;
            outch = mixch;
            
            if (mxsft)
            {
                mxsft = 0;
                chbuf = (chbuf << 4) | ((outch + 0x08) & 0x0f);
                fputc(chbuf, wp);
                outptr++;
                
                if (outptr >= cutsize)
                    return;
            }
            else
            {
                mxsft = 1;
                chbuf = (outch + 0x08) & 0x0f;
            }
        }
        
        if (cnt)
        {
            cnt--;
        }
        else
        {
            if (lf->pcmsize != len)
            {
                printf("out : %3ld%%\r",
                       ( ( lf->pcmsize - len ) * 100 ) / lf->pcmsize );
            }
            cnt = 1000;
        }        
        
    } // end of loop
   
    printf("out : 100%%\n");
    
    // terminal
    
    if (mxsft)
    {
        chbuf = (chbuf << 4) | 0;
        fputc(chbuf, wp);
        outptr++;
        
        if (outptr >= cutsize)
            return;
    }
    
    fputc(0x00, wp); // blank
}


int main(int argc,char *argv[])
{
    FILE   *dp,*wp;
    char   *ofn;
    long   outfreq;
    long   cutsize;
    struct wav_fmt wavf;
    char   out_fnam[256];
    int   i;
    
    cutsize = 65535;
    
    printf("WAV to P4D comverter ver 0.3 by BKC\n");
    
    if (argc == 1 )
    {
        printf("Usage WAV2P4D file [outfile] [outfreq]\n");
        return -1;
    }
    
    dp = fopen(argv[1],"rb");
    
    if (!dp)
    {
        printf("Error : File open error\n");
        return -1;
    }
    
    if (argc > 3)
        sscanf(argv[3],"%ld",&outfreq);
    else
        outfreq = 6992;
    
    
    wavf.freq = 11025;
    
    if ( wav_getfmt(dp,&wavf) )
    {
        printf("Error : Unsupported format file\n");
        fclose(dp);
        return -1;
    }
    
    printf(
           "frq = %d bit = %d ch = %d\n",
           (int)wavf.freq,
           (int)wavf.bit,
           (int)wavf.ch
           );
    
    if (wav_setpos_dat(dp, &wavf))
    {
        printf( "Error : data chank not found\n" );
        fclose(dp);
        return -1;
    }
    
    //	printf("pcm size in this file = %x\n",wavf.pcmsize);
    if (argc < 3)
    {
        strcpy(out_fnam, argv[1]);
        for(i = strlen(out_fnam); i > 0; i--)
        {
            if (out_fnam[i] == '.')
            {
                strcpy(out_fnam+i, ".pd4");
                break;
            }
        }
        
        if (!i) strcat(out_fnam, ".pd4");
        
        ofn = out_fnam;
        
    }
    else
    {
        ofn = argv[2];
    }
    
    wp = fopen(ofn, "wb");
    
    if (!wp) 
    {
        printf("Error : Could not open outfile\n");
        fclose(dp);
        return -1;
    }
    
    printf("output file = %s , freq=%ld \n",
           ofn,
           outfreq
           );
    
    wav_convert(dp, &wavf, wp, outfreq, cutsize);
    
    fclose(wp);
    fclose(dp);
    return 0;
}

