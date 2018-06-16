/*
  falab - free algorithm lab 
  Copyright (C) 2012 luolongzhi 罗龙智 (Chengdu, China)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.


  filename: fa_psychomodel1.c 
  version : v1.0.0
  time    : 2012/07/16 - 2012/07/18  
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "fa_psychomodel1.h"

#ifndef FA_PRINT 
//#define FA_PRINT(...)       
#define FA_PRINT       printf
#define FA_PRINT_ERR   FA_PRINT
#define FA_PRINT_DBG   FA_PRINT
#endif

static int nm_pos_geomean(int l, int u);

/*this psd normalizer is for the SPL estimation */
#define SAMPLE16
#ifdef SAMPLE16 
#define PSD_NORMALIZER  0  
#else
#define PSD_NORMALIZER  90.302
#endif

typedef struct _fa_psychomodel1_t {
    float fs;
    int   fft_len;
    int   psd_len;

    float *psd;
    float *psd_ath;
    float *psd_bark;
    float *ptm;
    float *pnm;
    int   *tone_flag;


    float *spread_effect;
    float *tone_thres;
    float *noise_thres;
    float *global_thres;

    int   cb_hopbin[CBANDS_NUM];
    float geomean_table[CBANDS_NUM];
    int   splnum1;
    int   splnum2;

}fa_psychomodel1_t;

#define DB_FAST

#define MAXDB  (300)
#define MINDB  (-300)
#define DBCNT  (MAXDB-MINDB+1)
static float db_table[DBCNT];

static void psy_dbtable_init()
{
    int i;
    float db;

    for (i = 0; i < DBCNT; i++) {
        db = (float)(i + MINDB);
        db_table[i] = pow(10, 0.1*db);
    }
}

static float db_pos(float power)
{
#if 0 //def DB_FAST
    int i;

    for (i = 0; i < DBCNT; i++) {
        if (power < db_table[i]) 
            return (i+MINDB);
    }

    return (DBCNT-1);

#else 
    return 10*log10(power);
#endif
}

/*#define TEST_MAXMINDB*/

#ifdef TEST_MAXMINDB
static float dbmax=-300.;
static float dbmin= 1000;
#endif

static float db_inv(float db)
{
#ifdef TEST_MAXMINDB
    /*printf("db=%f\n", db);*/
    if (db > dbmax)
        dbmax = db;
    if (db < dbmin)
        dbmin = db;
#endif

#ifdef DB_FAST
    int i;

    i = (int)db;

    return db_table[i-MINDB];
#else 
    return pow(10, 0.1*db);
#endif
}

/*input fft product(re,im)*/
static float psd_estimate(float re, float im)
{
    float power;
    float psd;

    power = re * re + im * im;
    /*use PSD_NORMALIZER to transform is to SPL estimation in dB*/
    psd = PSD_NORMALIZER + db_pos(power/64);

    return psd;
}


static float psd_estimate_usemdct(float mdct_line, float cof)
{
    float power;
    float psd;

    power = (mdct_line * mdct_line+1)/cof;
    /*use PSD_NORMALIZER to transform is to SPL estimation in dB*/
    psd = PSD_NORMALIZER + db_pos(power);

    return psd;
}



/*
 * input: freq in hz  
 * output: barks 
 */
static float freq2bark(float freq)
{
    if (freq < 0)
        freq = 0;
    freq = freq * 0.001;
    return 13.0 * atan(.76 * freq) + 3.5 * atan(freq * freq / (7.5 * 7.5));
}

/*cover f(Hz) to SPL(dB)*/
static float ath(float f)
{
    float ath;

    f /= 1000.;
 
    if (f > 0.0)
        ath = 3.64 * pow(f, -0.8) - 6.5 * exp(-0.6 * pow(f-3.3, 2)) + 0.001 * pow(f,4);
    else  {
        f = 0.01;
        ath = 3.64 * pow(f, -0.8) - 6.5 * exp(-0.6 * pow(f-3.3, 2)) + 0.001 * pow(f,4);
    }

    return ath;
}


/*freqbin = 0 means direct constant component*/
static float freqbin2freq(float fs, int fft_len, int freqbin)
{
    float delta_f;
    float f;

    delta_f = fs / fft_len; 

    f = freqbin * delta_f;

    return f;
}

static int freq2freqbin(float fs, int fft_len, float f)
{
    float delta_f;
    float freqbin;

    delta_f = fs / fft_len; 

    freqbin = f/delta_f;

    return freqbin;
}

static int caculate_cb_info(float fs, int psd_len, 
                            float *psd_ath, int cb_hopbin[CBANDS_NUM], float geomean_table[CBANDS_NUM],
                            float *psd_bark)
{
    int   i;
    int   k;
    float f;
    int   band;

    band = 0;
    memset(cb_hopbin, 0, sizeof(int)*CBANDS_NUM);
    for (i = 0; i < CBANDS_NUM; i++)
        geomean_table[i] = 1;

    for(k = 0; k < psd_len; k++) {
        f = freqbin2freq(fs, psd_len*2, k);
        psd_ath[k]  = ath(f);
        /*printf("psd_ath[%d]=%f\n", k, psd_ath[k]);*/
        psd_bark[k] = freq2bark(f);

        if((floor(psd_bark[k]) > band) &&
           (band < CBANDS_NUM)){
            cb_hopbin[band] = k;
            /*FA_PRINT("the hopbin of band[%d] = %d\n", band, k);*/
            if (band == 0)
                geomean_table[band] = nm_pos_geomean(1, cb_hopbin[band])-1;
            else 
                geomean_table[band] = nm_pos_geomean(cb_hopbin[band-1], cb_hopbin[band])-1;
            band++;
        }

        /*FA_PRINT("the %d freqbin's bark value psd_bark[%d] = %f\n", k, k, psd_bark[k]);*/
    }

    /*exit(0);*/
    return band;

}

/*fs: Hz*/
static int deltak_splitenum(float fs, int fft_len, int *splnum1, int *splnum2)
{
    int freqbin1, freqbin2;
/*
    deltak = 2     (0.17-5.5kHz)
             [2 3] (5.5-11kHz)
             [2 6] (11-max kHz)
*/
    freqbin1 = freq2freqbin(fs, fft_len, 5500);
    freqbin2 = freq2freqbin(fs, fft_len, 11000);

    *splnum1 = freqbin1;
    *splnum2 = freqbin2;

    return 0;
}

/*
    psd[0 : psd_len-1]
    St = P(k) {P(k) > P(k+), P(k) > P(k-1), P(k) > P(k+deltak)+7, P(k) > P(k-deltak)+7}
    deltak = 2     freq=[0.17~5.5kHz]
             [2 3] freq=[5.5~11kHz]
             [2 6] freq=[11 fmax]
    for example:
        fft_len=512, psd_len=512/2=256
        deltak = 2     2  <  k < 63
                 [2 3] 63 <= k < 127
                 [2 6] 127<= k <=256
 */
static int istone(float *psd, int psd_len, int freqbin, 
                  int splnum1, int splnum2, int *tone_flag)
{
    int i;
    int kr, kl;
    /*neighbour psd*/
    float psd_nr[6], psd_nl[6];
   
    if(freqbin == 0 || freqbin >= (psd_len-6))
        goto not_tone;

    /*0 means very low, minima*/
    for(i = 0; i < 6; i++) {
        kr = freqbin + (i + 1);
        kl = freqbin - (i + 1);
        if(kr < 0 || kr >= psd_len)
            psd_nr[i] = 0.0;
        else 
            psd_nr[i] = psd[kr];

        if(kl < 0 || kl >= psd_len)
            psd_nl[i] = 0.0;
        else 
            psd_nl[i] = psd[kl];
    }

    if(psd[freqbin] < psd_nr[0] || psd[freqbin] < psd_nl[0]) {
        goto not_tone;
    }else if(freqbin > 1 && freqbin < splnum1) {          //0.17~5.5 kHz
        if(psd[freqbin] <= (psd_nr[1]+7) || psd[freqbin] <= (psd_nl[1]+7))
            goto not_tone;
        tone_flag[freqbin] = 1;
        tone_flag[freqbin-1] = tone_flag[freqbin+1] = 1;
        tone_flag[freqbin-2] = tone_flag[freqbin+2] = 1;
    }else if(freqbin >= splnum1 && freqbin < splnum2) {   //5.5~11   kHz
        for(i = 1; i <= 2; i++) {
            if(psd[freqbin] <= (psd_nr[i]+7) || psd[freqbin] <= (psd_nl[i]+7))
                goto not_tone;
        }
        tone_flag[freqbin] = 1;
        tone_flag[freqbin-1] = tone_flag[freqbin+1] = 1;
        tone_flag[freqbin-2] = tone_flag[freqbin+2] = 1;
        tone_flag[freqbin-3] = tone_flag[freqbin+3] = 1;
    }else if(freqbin >= splnum2 && freqbin < psd_len){                                                 //11~max   kHz
        for(i = 1; i <= 5; i++) {
            if(psd[freqbin] <= (psd_nr[i]+7) || psd[freqbin] <= (psd_nl[i]+7))
                goto not_tone;
        }
        tone_flag[freqbin] = 1;
        tone_flag[freqbin-1] = tone_flag[freqbin+1] = 1;
        tone_flag[freqbin-2] = tone_flag[freqbin+2] = 1;
        tone_flag[freqbin-3] = tone_flag[freqbin+3] = 1;
        tone_flag[freqbin-4] = tone_flag[freqbin+4] = 1;
        tone_flag[freqbin-5] = tone_flag[freqbin+5] = 1;
        tone_flag[freqbin-6] = tone_flag[freqbin+6] = 1;
    }else {
        goto not_tone;
    }

    return 1;
not_tone:
    return 0;
}

/*caculate ptm*/
static int psd_tonemasker(float *psd , int psd_len, 
                          int splnum1, int splnum2, 
                          float *ptm , int *tone_flag)
{
    int k;

    memset(ptm, 0, sizeof(float)*psd_len);
    memset(tone_flag, 0, sizeof(int)*psd_len);

    for(k = 0; k < psd_len; k++) {
        if(istone(psd, psd_len, k, splnum1, splnum2, tone_flag)) {
            ptm[k] = db_pos(db_inv(psd[k-1]) + db_inv(psd[k]) + db_inv(psd[k+1]));
            /*printf("ptm[%d]=%f\n", k, ptm[k]);*/
        }
    }

    return 0;
}

static int nm_pos_geomean(int l, int u)
{
    int i;
    int n;
    double tmp;  //must be double, if use float(maybe not enough to store tmp)
    int pos;

    tmp = 1;
    n   = u - l + 1;

    for(i = l; i <=u; i++) 
        tmp *= i;

    pos = floor(pow(tmp, 1./n));

    return pos;

}

static int noisemasker_band(float *psd, int *tone_flag, 
                            int lowbin, int highbin,
                            float *nm , int *nm_pos)
{
    int k;
    float tmp;

    *nm  = 0;
    tmp = 0;
    for(k = lowbin; k <= highbin; k++) {
        if(!tone_flag[k]) {
            tmp += db_inv(psd[k]);
        }
    }

    *nm     = db_pos(tmp);
    *nm_pos = nm_pos_geomean(lowbin+1, highbin+1)-1;

    return 0;
}


static int noisemasker_band_fast(float *psd, int *tone_flag, 
                                 int lowbin, int highbin,
                                 int band, float *geomean_table,
                                 float *nm , int *nm_pos)
{
    int k;
    float tmp;

    *nm  = 0;
    tmp = 0;
    for(k = lowbin; k <= highbin; k++) {
        if(!tone_flag[k]) {
            tmp += db_inv(psd[k]);
        }
    }

#if 1 
    *nm = db_pos(tmp);
#else 
    if (tmp > 0)
        *nm = db_pos(tmp);
    else 
        *nm = 0;
#endif

    /**nm_pos = nm_pos_geomean(lowbin+1, highbin+1)-1;*/
    *nm_pos = geomean_table[band]; //nm_pos_geomean(lowbin+1, highbin+1)-1;

    return 0;
}



static int psd_noisemasker(float *psd, int psd_len,
                    int *tone_flag, int *cb_hopbin, 
                    float *geomean_table,
                    float *pnm)
{
    int band;
    int lowbin, highbin;
    float nm; 
    int   nm_pos;

    lowbin = 0;
    memset(pnm, 0, sizeof(float)*psd_len);
    for(band = 0; band < CBANDS_NUM; band++) {
        nm_pos = 0;
        if(cb_hopbin[band] == 0)
            break;

        highbin = cb_hopbin[band] - 1; 
        /*noisemasker_band(psd, tone_flag, lowbin, highbin, &nm, &nm_pos);*/
        noisemasker_band_fast(psd, tone_flag, lowbin, highbin, band, geomean_table, &nm, &nm_pos);
        lowbin  = highbin + 1;
        pnm[nm_pos] = nm;
        /*printf("pnm[%d]=%f\n", nm_pos, pnm[nm_pos]);*/
    }

    return 0;
}

static int ptm_pnm_filter_by_ath(float *ptm, float *pnm, float *psd_ath, int psd_len)
{
    int k;

    for(k = 0; k < psd_len; k++) {
        if(ptm[k] < psd_ath[k])
            ptm[k] = 0.;
        if(pnm[k] < psd_ath[k])
            pnm[k] = 0.;
    }

    return 0;
}

static int psd_bark2bin(float *psd_bark, int psd_len, float bark)
{
    int k;

    if(bark <= 0)
        return 0;

    if(bark >= CBANDS_NUM)
        return psd_len-1;

    for(k = 1; k < psd_len; k++) {
        if(psd_bark[k] > bark)
            return k-1;
    }

    return psd_len-1;
}

static int check_near_masker(float *ptm, float *pnm, 
                             float *psd_ath, float *psd_bark,
                             int   psd_len)
{
    int k, j;
    int tone_found, noise_found;
    float masker_bark;
    float bw_low_bark, bw_high_bark;
    int   bw_low_bin, bw_high_bin;


    ptm_pnm_filter_by_ath(ptm, pnm, psd_ath, psd_len);


    for(k = 0; k < psd_len; k++) {
        tone_found  = 0;
        noise_found = 0;

        if(ptm[k] > 0)
            tone_found = 1;

        if(pnm[k] > 0)
            noise_found = 1;

        if(tone_found || noise_found) {
            masker_bark = psd_bark[k];
            bw_low_bark = masker_bark - 0.5;
            bw_high_bark= masker_bark + 0.5;

            bw_low_bin  = psd_bark2bin(psd_bark, psd_len, bw_low_bark) + 1;
            bw_high_bin = psd_bark2bin(psd_bark, psd_len, bw_high_bark);

            for(j = bw_low_bin; j <= bw_high_bin; j++) {
                if(tone_found) {
                    if(j != k && ptm[k] < ptm[j]) {
                        ptm[k] = 0;
                        break;
                    }else if(j != k){
                        ptm[j] = 0;
                    }else {
                        ;
                    }

                    if(ptm[k] < pnm[j]) {
                        ptm[k] = 0;
                        break;
                    }else {
                        pnm[j] = 0;
                    }
                }else if(noise_found) {
                    if(j != k && pnm[k] < pnm[j]) {
                        pnm[k] = 0;
                        break;
                    }else if(j != k){
                        pnm[j] = 0;
                    }else {
                        ;
                    }

                    if(pnm[k] < ptm[j]) {
                        pnm[k] = 0;
                        break;
                    }else {
                        ptm[j] = 0;
                    }
                }else{
                    ;
                }
            }

        }

    }

    return 0;
}


static int spread_function(float power, float *psd_bark,  
                           int masker_bin, int low_bin, int high_bin,
                           float *spread_effect)
{
    int   i;
    float masker_bark;
    float maskee_bark;
    float delta_bark;

    masker_bark = psd_bark[masker_bin]; 

    for(i = low_bin; i <= high_bin; i++) {
        maskee_bark = psd_bark[i];
        delta_bark  = maskee_bark - masker_bark;

        if(delta_bark >= -3.5 && delta_bark < -1)
            spread_effect[i] = 17*delta_bark - 0.4*power + 11;
        else if(delta_bark >= -1 && delta_bark < 0)
            spread_effect[i] = (0.4*power + 6.)*delta_bark;
        else if(delta_bark >= 0 && delta_bark < 1)
            spread_effect[i] = -17*delta_bark;
        else if(delta_bark >= 1 && delta_bark < 8.5)
            spread_effect[i] = (0.15*power-17)*delta_bark - 0.15*power;
        else 
            ;
    }

    return 0;

}

static int tone_mask_threshold(float *ptm, float *psd_bark, 
                               float *spread_effect, float *tone_thres, int psd_len)
{
    int   k, j;
    float masker_bark;
    float low_bark, high_bark;
    int   low_bin , high_bin;

    memset(tone_thres, 0, sizeof(float)*psd_len);

    for(k = 0; k < psd_len; k++) {
        memset(spread_effect, 0, sizeof(float)*psd_len);
        if(ptm[k] > 0) {
            masker_bark = psd_bark[k];
            low_bark    = masker_bark - 3;
            high_bark   = masker_bark + 8;
            low_bin     = psd_bark2bin(psd_bark, psd_len, low_bark);
            high_bin    = psd_bark2bin(psd_bark, psd_len, high_bark);

            spread_function(ptm[k], psd_bark, k, low_bin, high_bin, spread_effect);
            for(j = low_bin; j <= high_bin; j++)
                tone_thres[j] += db_inv(ptm[k] - 0.275*masker_bark + spread_effect[j] - 6.025);
        }

    }

    return 0;
}

static int noise_mask_threshold(float *pnm, float *psd_bark, 
                                float *spread_effect, float *noise_thres, int psd_len)
{
    int   k, j;
    float masker_bark;
    float low_bark, high_bark;
    int   low_bin , high_bin;

    memset(noise_thres, 0, sizeof(float)*psd_len);

    for(k = 0; k < psd_len; k++) {
        memset(spread_effect, 0, sizeof(float)*psd_len);
        if(pnm[k] > 0) {
            masker_bark = psd_bark[k];
            low_bark    = masker_bark - 3;
            high_bark   = masker_bark + 8;
            low_bin     = psd_bark2bin(psd_bark, psd_len, low_bark);
            high_bin    = psd_bark2bin(psd_bark, psd_len, high_bark);

            spread_function(pnm[k], psd_bark, k, low_bin, high_bin, spread_effect);
            for(j = low_bin; j <= high_bin; j++)
                noise_thres[j] += db_inv(pnm[k] - 0.175*masker_bark + spread_effect[j] - 2.025);
        }

    }

    return 0;
}

/*this valued by dB*/
static int global_threshold(float *tone_thres, float *noise_thres, float *psd_ath,
                            float *global_thres, int psd_len)
{
    int k;

    for(k = 0; k < psd_len; k++) 
        global_thres[k] = db_pos(tone_thres[k] + noise_thres[k] + db_inv(psd_ath[k]));

    return 0;
}

/*this valued by magnitude*/
static int global_threshold_mag(float *tone_thres, float *noise_thres, float *psd_ath,
                                float *global_thres, int psd_len)
{
    int k;

    for(k = 0; k < psd_len; k++) 
        global_thres[k] = (tone_thres[k] + noise_thres[k] + db_inv(psd_ath[k]));

    return 0;
}

void fa_psychomodel1_rom_init()
{
    psy_dbtable_init();
}

uintptr_t fa_psychomodel1_init(int fs, int fft_len)
{
    int psd_len;
    fa_psychomodel1_t *f = NULL;

    f = (fa_psychomodel1_t *)malloc(sizeof(fa_psychomodel1_t));
    psd_len = fft_len >> 1;

    f->fs      = fs;
    f->fft_len = fft_len;
    f->psd_len = psd_len;

    f->psd        = (float *)malloc(sizeof(float)*psd_len);
    f->psd_ath    = (float *)malloc(sizeof(float)*psd_len);
    f->psd_bark   = (float *)malloc(sizeof(float)*psd_len);
    f->ptm        = (float *)malloc(sizeof(float)*psd_len);
    f->pnm        = (float *)malloc(sizeof(float)*psd_len);
    f->tone_flag  = (int   *)malloc(sizeof(int)*psd_len);

    f->spread_effect = (float *)malloc(sizeof(float)*psd_len);
    f->tone_thres    = (float *)malloc(sizeof(float)*psd_len);
    f->noise_thres   = (float *)malloc(sizeof(float)*psd_len);
    f->global_thres  = (float *)malloc(sizeof(float)*psd_len);


    deltak_splitenum(fs, fft_len, &f->splnum1, &f->splnum2);

    caculate_cb_info(fs, psd_len, f->psd_ath, f->cb_hopbin, f->geomean_table, f->psd_bark);


    return (uintptr_t)f;
}
void fa_psychomodel1_uninit(uintptr_t handle)
{
    fa_psychomodel1_t *f = (fa_psychomodel1_t *)handle;

    if(f) {
        if(f->psd) {
            free(f->psd);
            f->psd = NULL;
        }
        if(f->psd_ath) {
            free(f->psd_ath);
            f->psd_ath = NULL;
        }
        if(f->psd_bark) {
            free(f->psd_bark);
            f->psd_bark = NULL;
        }
        if(f->ptm) {
            free(f->ptm);
            f->ptm = NULL;
        }
        if(f->pnm) {
            free(f->pnm);
            f->pnm = NULL;
        }
        if(f->tone_flag) {
            free(f->tone_flag);
            f->tone_flag = NULL;
        }
        if(f->spread_effect) {
            free(f->spread_effect);
            f->spread_effect = NULL;
        }
        if(f->tone_thres) {
            free(f->tone_thres);
            f->tone_thres = NULL;
        }
        if(f->noise_thres) {
            free(f->noise_thres);
            f->noise_thres = NULL;
        }
        if(f->global_thres) {
            free(f->global_thres);
            f->global_thres = NULL;
        }

        free(f);
        f = NULL;
    }

}

void fa_psy_global_threshold(uintptr_t handle, float *fft_buf, float *gth)
{
    int k;
    int   frame_len;
    float re, im;
    fa_psychomodel1_t *f = (fa_psychomodel1_t *)handle;

    /*step1: psd estimate and normalize to SPL*/
    for(k = 0; k < f->psd_len; k++) {
        re = fft_buf[k+k];
        im = fft_buf[k+k+1];
        f->psd[k] = psd_estimate(re, im);
    }
    
    /*step2: tone and noise masker estimate*/
    psd_tonemasker(f->psd, f->psd_len, f->splnum1, f->splnum2, f->ptm, f->tone_flag);
    /*psd_noisemasker(f->psd, f->psd_len, f->tone_flag, f->cb_hopbin, f->pnm);*/
    psd_noisemasker(f->psd, f->psd_len, f->tone_flag, f->cb_hopbin, f->geomean_table, f->pnm);

    /*step3: check near masker, merge it if near*/
    check_near_masker(f->ptm, f->pnm, f->psd_ath, f->psd_bark, f->psd_len);

    /*step4: caculate individule threshold (tone, noise)*/
    tone_mask_threshold(f->ptm, f->psd_bark, f->spread_effect, f->tone_thres, f->psd_len);
    noise_mask_threshold(f->pnm, f->psd_bark, f->spread_effect, f->noise_thres, f->psd_len);

    /*step5: caculate global threshold*/
    global_threshold(f->tone_thres, f->noise_thres, f->psd_ath, f->global_thres, f->psd_len);
    memcpy(gth, f->global_thres, sizeof(float)*f->psd_len);

}

void fa_psy_global_threshold_usemdct(uintptr_t handle, float *mdct_buf, float *gth)
{
    int k;
    int   frame_len;
    float re;
    fa_psychomodel1_t *f = (fa_psychomodel1_t *)handle;
    float cof;

#ifdef TEST_MAXMINDB
    dbmax = -300.;
    dbmin = 1000;
#endif

    /*cof = (100. *  (float)f->psd_len)/1024.;*/
    cof = (64. *  (float)f->psd_len)/1024.;
    /*step1: psd estimate and normalize to SPL*/
    for(k = 0; k < f->psd_len; k++) {
        re = mdct_buf[k];
        f->psd[k] = psd_estimate_usemdct(re, cof);
    }
    
    /*step2: tone and noise masker estimate*/
    psd_tonemasker(f->psd, f->psd_len, f->splnum1, f->splnum2, f->ptm, f->tone_flag);
    psd_noisemasker(f->psd, f->psd_len, f->tone_flag, f->cb_hopbin, f->geomean_table, f->pnm);

    /*step3: check near masker, merge it if near*/
    check_near_masker(f->ptm, f->pnm, f->psd_ath, f->psd_bark, f->psd_len);

    /*step4: caculate individule threshold (tone, noise)*/
    tone_mask_threshold(f->ptm, f->psd_bark, f->spread_effect, f->tone_thres, f->psd_len);
    noise_mask_threshold(f->pnm, f->psd_bark, f->spread_effect, f->noise_thres, f->psd_len);

    /*step5: caculate global threshold*/
    global_threshold_mag(f->tone_thres, f->noise_thres, f->psd_ath, f->global_thres, f->psd_len);
    memcpy(gth, f->global_thres, sizeof(float)*f->psd_len);

#ifdef TEST_MAXMINDB
    printf("dbmax=%f, dbmin=%f\n", dbmax, dbmin);
#endif
}

