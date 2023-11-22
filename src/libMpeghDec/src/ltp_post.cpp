/*-----------------------------------------------------------------------------
Software License for The Fraunhofer FDK MPEG-H Software

Copyright (c) 2018 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten
Forschung e.V. and Contributors
All rights reserved.

1. INTRODUCTION

The "Fraunhofer FDK MPEG-H Software" is software that implements the ISO/MPEG
MPEG-H 3D Audio standard for digital audio or related system features. Patent
licenses for necessary patent claims for the Fraunhofer FDK MPEG-H Software
(including those of Fraunhofer), for the use in commercial products and
services, may be obtained from the respective patent owners individually and/or
from Via LA (www.via-la.com).

Fraunhofer supports the development of MPEG-H products and services by offering
additional software, documentation, and technical advice. In addition, it
operates the MPEG-H Trademark Program to ease interoperability testing of end-
products. Please visit www.mpegh.com for more information.

2. COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification,
are permitted without payment of copyright license fees provided that you
satisfy the following conditions:

* You must retain the complete text of this software license in redistributions
of the Fraunhofer FDK MPEG-H Software or your modifications thereto in source
code form.

* You must retain the complete text of this software license in the
documentation and/or other materials provided with redistributions of
the Fraunhofer FDK MPEG-H Software or your modifications thereto in binary form.
You must make available free of charge copies of the complete source code of
the Fraunhofer FDK MPEG-H Software and your modifications thereto to recipients
of copies in binary form.

* The name of Fraunhofer may not be used to endorse or promote products derived
from the Fraunhofer FDK MPEG-H Software without prior written permission.

* You may not charge copyright license fees for anyone to use, copy or
distribute the Fraunhofer FDK MPEG-H Software or your modifications thereto.

* Your modified versions of the Fraunhofer FDK MPEG-H Software must carry
prominent notices stating that you changed the software and the date of any
change. For modified versions of the Fraunhofer FDK MPEG-H Software, the term
"Fraunhofer FDK MPEG-H Software" must be replaced by the term "Third-Party
Modified Version of the Fraunhofer FDK MPEG-H Software".

3. No PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without
limitation the patents of Fraunhofer, ARE GRANTED BY THIS SOFTWARE LICENSE.
Fraunhofer provides no warranty of patent non-infringement with respect to this
software. You may use this Fraunhofer FDK MPEG-H Software or modifications
thereto only for purposes that are authorized by appropriate patent licenses.

4. DISCLAIMER

This Fraunhofer FDK MPEG-H Software is provided by Fraunhofer on behalf of the
copyright holders and contributors "AS IS" and WITHOUT ANY EXPRESS OR IMPLIED
WARRANTIES, including but not limited to the implied warranties of
merchantability and fitness for a particular purpose. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE for any direct, indirect,
incidental, special, exemplary, or consequential damages, including but not
limited to procurement of substitute goods or services; loss of use, data, or
profits, or business interruption, however caused and on any theory of
liability, whether in contract, strict liability, or tort (including
negligence), arising in any way out of the use of this software, even if
advised of the possibility of such damage.

5. CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS
Attention: Division Audio and Media Technologies - MPEG-H FDK
Am Wolfsmantel 33
91058 Erlangen, Germany
www.iis.fraunhofer.de/amm
amm-info@iis.fraunhofer.de
-----------------------------------------------------------------------------*/

/************************* MPEG-H 3DA decoder library **************************

   Author(s):   Matthias Neusinger

   Description: TCX/AAC Long Term Prediction Postfilter

*******************************************************************************/

#include "ltp_post.h"
#include "FDK_lpc.h"
#include "mdct.h" /* Required for PCM_OUT_HEADROOM */

#define LTP_LPC_ORDER 24
#define TCXLTP_MAX_DELAY 3

#define PIT_MIN_12k8 34  /* Minimum pitch lag with resolution 1/4      */
#define PIT_FR2_12k8 128 /* Minimum pitch lag with resolution 1/2      */
#define PIT_FR1_12k8 160 /* Minimum pitch lag with resolution 1        */
#define PIT_MAX_12k8 231 /* Maximum pitch lag                          */

#define L_FRAME_1024 1024
#define L_FRAME_MAX 1024
#define FSCALE_DENOM 12800 /* filter into decim_split.h */

#define ALPHA FL2FXCONST_DBL(0.95)

const FIXP_SGL aac_ltp_post_inter[] = {
    FL2FXCONST_SGL(0.2674597f), FL2FXCONST_SGL(0.2642256f), FL2FXCONST_SGL(0.2547232f),
    FL2FXCONST_SGL(0.2395331f), FL2FXCONST_SGL(0.2195613f), FL2FXCONST_SGL(0.1959555f),
    FL2FXCONST_SGL(0.1700032f), FL2FXCONST_SGL(0.1430240f), FL2FXCONST_SGL(0.1162701f),
    FL2FXCONST_SGL(0.0908452f), FL2FXCONST_SGL(0.0676508f), FL2FXCONST_SGL(0.0473628f),
    FL2FXCONST_SGL(0.0304386f), FL2FXCONST_SGL(0.0171484f), FL2FXCONST_SGL(0.0076226f),
    FL2FXCONST_SGL(0.0019050f), FL2FXCONST_SGL(0.0000000f),
};
const FIXP_SGL aac_ltp_post_tilt[] = {
    FL2FXCONST_SGL(0.27150189f),  FL2FXCONST_SGL(0.44286013f),  FL2FXCONST_SGL(0.23027992f),
    FL2FXCONST_SGL(0.05759155f),  FL2FXCONST_SGL(-0.00172290f), FL2FXCONST_SGL(-0.00045168f),
    FL2FXCONST_SGL(-0.00005891f), FL2FXCONST_SGL(0.27581838f),  FL2FXCONST_SGL(0.44682277f),
    FL2FXCONST_SGL(0.22783915f),  FL2FXCONST_SGL(0.05410054f),  FL2FXCONST_SGL(-0.00353758f),
    FL2FXCONST_SGL(-0.00092331f), FL2FXCONST_SGL(-0.00011995f), FL2FXCONST_SGL(0.28044685f),
    FL2FXCONST_SGL(0.45103979f),  FL2FXCONST_SGL(0.22519192f),  FL2FXCONST_SGL(0.05037740f),
    FL2FXCONST_SGL(-0.00545541f), FL2FXCONST_SGL(-0.00141719f), FL2FXCONST_SGL(-0.00018336f),
    FL2FXCONST_SGL(0.28543320f),  FL2FXCONST_SGL(0.45554676f),  FL2FXCONST_SGL(0.22230634f),
    FL2FXCONST_SGL(0.04638935f),  FL2FXCONST_SGL(-0.00749011f), FL2FXCONST_SGL(-0.00193612f),
    FL2FXCONST_SGL(-0.00024943f),
};
typedef struct AAC_LTP_FILTER {
  const FIXP_SGL* filt;
  int length;
} AAC_LTP_FILTER;
const AAC_LTP_FILTER aacLtpFilters[1] = {
    {aac_ltp_post_inter, AAC_LTP_FILTERS_LENGTH},
};
const AAC_LTP_FILTER aacLtpFilters2[1] = {
    {aac_ltp_post_tilt, AAC_LTP_FILTERS2_LENGTH},
};

#if defined(__arm__)
#include "arm/ltp_post_arm.cpp"
#endif

/*-------------------------------------------------------------------
 * ltp_get_lpc()
 *
 *
 *-------------------------------------------------------------------*/
#if !defined(FUNCTION_ltp_get_lpc)

static void ltp_get_lpc(FIXP_SGL* input, INT length, FIXP_SGL* A, INT* A_e, INT lpcorder) {
  int i, j, s;
  FIXP_DBL rtmp;
  FIXP_DBL r[LTP_LPC_ORDER + 1];

  FDK_ASSERT(length <= L_FRAME_MAX);

  /* calc r[0], determine shift */
  s = 0;
  rtmp = (FIXP_DBL)0;
  for (j = 0; j < length; j++) {
    rtmp += fPow2Div2(input[j]) >> s;
    if (rtmp >= (FIXP_DBL)0x40000000) {
      s++;
      rtmp >>= 1;
    }
  }

  rtmp = fMax(rtmp, (FIXP_DBL)100);

  INT sh2 = fNorm(rtmp);
  const FIXP_DBL one_0001 = (FIXP_DBL)0x4001a36e; /* constant 1.0001 in Q1.30 */
  rtmp = fMult(rtmp << sh2, one_0001);
  sh2--;
  FDK_ASSERT(sh2 >= 0);
  r[0] = rtmp;

  /* calc r[1...] */
  for (i = 1; i <= lpcorder; i++) {
    rtmp = (FIXP_DBL)0;

    for (j = 0; j < length - i; j++) {
      rtmp += fMultDiv2(input[j], input[j + i]) >> s;
    }
    r[i] = rtmp << sh2;
  }

  CLpc_AutoToLpc(A, A_e, r, 0, lpcorder, NULL, 1);

  return;
}
#endif /* #if !defined(FUNCTION_ltp_get_lpc) */

/*-------------------------------------------------------------------
 * ltp_get_zir()
 *
 *
 *-------------------------------------------------------------------*/

static void ltp_get_zir(FIXP_DBL* zir, int length, FIXP_SGL* synth_ltp, FIXP_SGL* synth,
                        FIXP_SGL* A, int A_e, int lpcorder, FIXP_SGL gain, int gainIdx,
                        int pitch_int, int pitch_fr, int pitres, int filtIdx) {
  FIXP_DBL buf[LTP_LPC_ORDER];
  FIXP_SGL alpha, step;
  FIXP_SGL *x0, *x1;
  FIXP_SGL* y0;
  FIXP_DBL s, s2;
  const FIXP_SGL *w0, *w1, *v0;
  int i, j, k, L, L2;
  int stIdx = 0;
  LONG tmp;

  x0 = &synth_ltp[-pitch_int];
  x1 = x0 - 1;
  y0 = synth;

  FDK_ASSERT(filtIdx >= 0);

  w0 = &aacLtpFilters[filtIdx].filt[pitch_fr];
  w1 = &aacLtpFilters[filtIdx].filt[pitres - pitch_fr];
  L = aacLtpFilters[filtIdx].length;
  L2 = aacLtpFilters2[filtIdx].length;
  v0 = &aacLtpFilters2[filtIdx].filt[gainIdx * L2];

  for (j = 0; j < lpcorder; j++) {
    s = (FIXP_DBL)0;
    s2 = (FIXP_DBL)0;

    for (i = 0, k = 0; i < L; i++, k += pitres) {
      s += fMultDiv2(w0[k], x0[i]) + fMultDiv2(w1[k], x1[-i]);
    }

    for (i = 0; i < L2; i++) {
      s2 += fMultDiv2(v0[i], y0[-i]);
    }

    s2 = fMult(s2, ALPHA);

    tmp = (s >> 1) - (s2 >> 1);
    tmp = (((LONG)synth[j] - (LONG)synth_ltp[j]) << 14) + (LONG)fMult(gain, (FIXP_DBL)tmp);
    buf[lpcorder - 1 - j] = tmp;

    x0++;
    x1++;
    y0++;
  }

  FDKmemclear(zir, length * sizeof(FIXP_DBL));

  CLpc_Synthesis(zir, length, 1, A, A_e, lpcorder, buf, &stIdx);

  alpha = (FIXP_SGL)MAXVAL_SGL;
  step = (FIXP_SGL)(MAXVAL_SGL / (length >> 1));

  for (j = length >> 1; j < length; j++) {
    zir[j] = fMult(zir[j], alpha);
    alpha -= step;
  }

  return;
}

/*-------------------------------------------------------------------
 * ltp_synth_filter()
 *
 *
 *-------------------------------------------------------------------*/

static void ltp_synth_filter(FIXP_SGL* synth_ltp, FIXP_SGL* synth, int length, int pitch_int,
                             int pitch_fr, FIXP_SGL gain, int gainIdx, int pitch_res,
                             FIXP_DBL* zir, /* can be NULL */
                             int fade,      /* 0=normal, +1=fade-in, -1=fade-out */
                             short filtIdx) {
  if (gain > (FIXP_SGL)0) {
#if defined(FUNCTION_ltp_synth_filter_aac)
    ltp_synth_filter_aac(synth_ltp, synth, length, pitch_int, pitch_fr, gain, gainIdx, pitch_res,
                         zir, fade, filtIdx);
#else
#if defined(FUNCTION_ltp_synth_filter_func1)
    const FIXP_SGL* v0;
    const FIXP_SGL *w0, *w1;
    FIXP_DBL alpha, step = 0;
    int L2;
#else
    FIXP_SGL *x0, *x1;
    FIXP_SGL* y0;
    FIXP_DBL s, s2;
    const FIXP_SGL* v0;
    const FIXP_SGL *w0, *w1;
    int i, j, k, L, L2;
    FIXP_DBL alpha, step = 0;
    FIXP_DBL tmp;
    x0 = &synth_ltp[-pitch_int];
    x1 = x0 - 1;
    y0 = synth;
#endif

    FDK_ASSERT(filtIdx >= 0);
    FDK_ASSERT((length & 3) == 0);

    w0 = &aacLtpFilters[filtIdx].filt[pitch_fr];
    w1 = &aacLtpFilters[filtIdx].filt[pitch_res - pitch_fr];
#if defined(FUNCTION_ltp_synth_filter_func1)
    FDK_ASSERT(aacLtpFilters[filtIdx].length == 4);  /* hard-coded for ARM-NEON and ADSP21k */
    FDK_ASSERT(aacLtpFilters2[filtIdx].length == 7); /* hard-coded for ARM-NEON and ADSP21k */
#else
    L = aacLtpFilters[filtIdx].length;
#endif
    L2 = aacLtpFilters2[filtIdx].length;
    v0 = &aacLtpFilters2[filtIdx].filt[gainIdx * L2];

    alpha = (fade <= 0) ? (FIXP_DBL)MINVAL_DBL : (FIXP_DBL)0;
    FDK_ASSERT((length == 512) || (length == 128) || (length == 384));
    if (length == 384)
      step = fade * FL2FX_DBL(1.0 / 384); /* +/- 0x0055.5555 */
    else if (length == 512)
      step = (FIXP_DBL)fade << (6 + 16); /* +/- 0x0040.0000 */
    else                                 /* if (length == 128) */
      step = (FIXP_DBL)fade << (8 + 16); /* +/- 0x0100.0000 */
    gain = -gain;

#if defined(FUNCTION_ltp_synth_filter_func1)
    ltp_synth_filter_func1(synth_ltp, synth, zir, length, pitch_res, w0, w1, v0, pitch_int, alpha,
                           step, gain, ALPHA);
#else
    for (j = 0; j < length; j++) {
      s = (FIXP_DBL)0;
      s2 = (FIXP_DBL)0;

      for (i = 0, k = 0; i < L; i++, k += pitch_res) {
        s += fMultDiv2(w0[k], x0[i]) + fMultDiv2(w1[k], x1[-i]);
      }

      for (i = 0; i < L2; i++) {
        s2 += fMultDiv2(v0[i], y0[-i]);
      }

      s2 = fMultDiv2(s2, ALPHA);
      tmp = (s >> 1) - s2;
      tmp = (FIXP_DBL)((LONG)synth[j] << 13) + fMultDiv2(fMult(gain, alpha), tmp);
      if (zir) {
        tmp -= zir[j] >> 1;
      }
      synth_ltp[j] = (FIXP_SGL)(SATURATE_RIGHT_SHIFT(tmp, 13, FRACT_BITS));
      alpha -= step;
      x0++;
      x1++;
      y0++;
    }
#endif /* defined(FUNCTION_ltp_synth_filter_func1) */

#endif /* defined(FUNCTION_ltp_synth_filter_aac) */
  } else {
    FDKmemcpy(synth_ltp, synth, length * sizeof(FIXP_SGL));
  }

  return;
}

/*-------------------------------------------------------------------
 * ltp_decode_params()
 *
 *
 *-------------------------------------------------------------------*/

static void ltp_decode_params(int* ltp_param, int* pitch_int, int* pitch_fr, FIXP_SGL* gain,
                              int pitmin, int pitfr1, int pitfr2, int pitmax, int pitres) {
  int gainbits = 2;

  /* Decode Pitch and Gain */
  if ((ltp_param) && (ltp_param[0])) {
    if (ltp_param[1] < ((pitfr2 - pitmin) * pitres)) {
      *pitch_int = pitmin + (ltp_param[1] / pitres);
      *pitch_fr = ltp_param[1] - (*pitch_int - pitmin) * pitres;
    } else if (ltp_param[1] < ((pitfr2 - pitmin) * pitres + (pitfr1 - pitfr2) * (pitres >> 1))) {
      *pitch_int = pitfr2 + ((ltp_param[1] - (pitfr2 - pitmin) * pitres) / (pitres >> 1));
      *pitch_fr =
          (ltp_param[1] - (pitfr2 - pitmin) * pitres) - (*pitch_int - pitfr2) * (pitres >> 1);
      *pitch_fr = *pitch_fr << 1; /* was *= (pitres>>1); */
    } else {
      *pitch_int = ltp_param[1] + pitfr1 - ((pitfr2 - pitmin) * pitres) -
                   ((pitfr1 - pitfr2) * (pitres >> 1));
      *pitch_fr = 0;
    }

    if (ltp_param[2] >= 3) {
      *gain = (FIXP_SGL)MAXVAL_SGL;
    } else {
      *gain = (FIXP_SGL)(ltp_param[2] + 1) << (15 - gainbits);
    }

  } else {
    *pitch_int = 0;
    *pitch_fr = 0;
    *gain = (FIXP_SGL)0;
  }

  return;
}

int readLtpParam(HANDLE_FDK_BITSTREAM hBs, int* ltp_param, const int isLFE) {
  ltp_param[0] = FDKreadBit(hBs); /* ltpf_data_present */
  if (ltp_param[0]) {
    if (isLFE) {
      return 1;
    }
    ltp_param[1] = FDKreadBits(hBs, 9); /* ltpf_pitch_lag_index */
    ltp_param[2] = FDKreadBits(hBs, 2); /* ltpf_gain_index */
  }
  return 0;
}

/*-------------------------------------------------------------------
 * ltp_post()
 *
 *
 *-------------------------------------------------------------------*/

void ltp_post(FIXP_DBL* sig, int L_frame, int fs, int* ltp_param, int* pitch_int_past,
              int* pitch_fr_past, FIXP_SGL* gain_past, int* gainIdx_past, FIXP_SGL* mem_in,
              FIXP_SGL* mem_out) {
  int tmp, L_transition, lpcorder, filtIdx, pitch_int, pitch_fr, pitres;
  int pit_min, pit_max, pit_fr1, pit_fr2, i;
  int enc_dec_delay;
  FIXP_DBL zir[L_FRAME_1024 / 4];
  FIXP_SGL A[LTP_LPC_ORDER + 1], gain;
  FIXP_SGL buf_in[L_FRAME_1024 + LTP_MEM_IN_SIZE + TCXLTP_MAX_DELAY +
                  3]; /* need +3 for xtensa optimisation (length divisible by 4) */
  FIXP_SGL buf_out[L_FRAME_1024 + LTP_MEM_OUT_SIZE];
  FIXP_SGL *sig_in, *sig_out;
  int A_e;

  /* Check if ltp post filter can run in pass-through mode */
  if ((*gain_past == (FIXP_SGL)0) && (ltp_param[0] == (int)0)) {
    FDK_ASSERT((L_frame >= LTP_MEM_IN_SIZE) && (L_frame >= LTP_MEM_OUT_SIZE));

    /* Update mem_in states */
    scaleValuesSaturate(mem_in, sig + L_frame - LTP_MEM_IN_SIZE, LTP_MEM_IN_SIZE, PCM_OUT_HEADROOM);

    /* Update mem_out states */
    scaleValuesSaturate(mem_out, sig + L_frame - LTP_MEM_OUT_SIZE, LTP_MEM_OUT_SIZE,
                        PCM_OUT_HEADROOM);

    /* Update past values */
    *gain_past = (FIXP_SGL)0;
    *gainIdx_past = 0;
    *pitch_fr_past = 0;
    *pitch_int_past = 0;

    return;
  }

  /******** Init ********/

  /* Parameters */
  filtIdx = 0;
  enc_dec_delay = L_frame >> 1;
  L_transition = L_frame / 8;
  lpcorder = LTP_LPC_ORDER;
  i = ((((fs / 2) * PIT_MIN_12k8) + (FSCALE_DENOM / 2)) / FSCALE_DENOM) - PIT_MIN_12k8;
  pit_min = PIT_MIN_12k8 + i;
  pit_fr2 = PIT_FR2_12k8 - i;
  pit_fr1 = PIT_FR1_12k8;
  pit_max = PIT_MAX_12k8 + (6 * i);
  pitres = 4;

  /* Input buffer */
  sig_in = buf_in + LTP_MEM_IN_SIZE;
  FDKmemcpy(buf_in, mem_in, LTP_MEM_IN_SIZE * sizeof(FIXP_SGL));
  scaleValuesSaturate(sig_in, sig, L_frame, PCM_OUT_HEADROOM);
  FDKmemcpy(mem_in, sig_in + L_frame - LTP_MEM_IN_SIZE, LTP_MEM_IN_SIZE * sizeof(FIXP_SGL));

  /* Output buffer */
  sig_out = buf_out + LTP_MEM_OUT_SIZE;
  FDKmemcpy(buf_out, mem_out, LTP_MEM_OUT_SIZE * sizeof(FIXP_SGL));

  /* LTP parameters: integer pitch, fractional pitch, gain */
  ltp_decode_params(ltp_param, &pitch_int, &pitch_fr, &gain, pit_min, pit_fr1, pit_fr2, pit_max,
                    pitres);
  tmp = pitch_int * pitres + pitch_fr;
  tmp = tmp * 2;
  pitch_int = tmp / pitres;
  pitch_fr = tmp % pitres;
  gain >>= 2;

  /******** Previous-frame part ********/
  if (enc_dec_delay) {
    ltp_synth_filter(sig_out, sig_in, enc_dec_delay, *pitch_int_past, *pitch_fr_past, *gain_past,
                     *gainIdx_past, pitres, NULL, 0, filtIdx);
  }

  /******** Transition part ********/
  if (gain == (FIXP_SGL)0 && *gain_past == (FIXP_SGL)0) {
    FDKmemcpy(sig_out + enc_dec_delay, sig_in + enc_dec_delay, L_transition * sizeof(FIXP_SGL));
  } else if (*gain_past == (FIXP_SGL)0) {
    ltp_synth_filter(sig_out + enc_dec_delay, sig_in + enc_dec_delay, L_transition, pitch_int,
                     pitch_fr, gain, ltp_param[2], pitres, NULL, 1, filtIdx);
  } else if (gain == (FIXP_SGL)0) {
    ltp_synth_filter(sig_out + enc_dec_delay, sig_in + enc_dec_delay, L_transition, *pitch_int_past,
                     *pitch_fr_past, *gain_past, *gainIdx_past, pitres, NULL, -1, filtIdx);
  } else if (gain == *gain_past && pitch_int == *pitch_int_past && pitch_fr == *pitch_fr_past) {
    ltp_synth_filter(sig_out + enc_dec_delay, sig_in + enc_dec_delay, L_transition, pitch_int,
                     pitch_fr, gain, ltp_param[2], pitres, NULL, 0, filtIdx);
  } else {
    ltp_get_lpc(sig_out + enc_dec_delay - (L_frame >> 2), (L_frame >> 2), A, &A_e, lpcorder);

    ltp_get_zir(zir, L_transition, sig_out + enc_dec_delay - lpcorder,
                sig_in + enc_dec_delay - lpcorder, A, A_e, lpcorder, gain, ltp_param[2], pitch_int,
                pitch_fr, pitres, filtIdx);

    ltp_synth_filter(sig_out + enc_dec_delay, sig_in + enc_dec_delay, L_transition, pitch_int,
                     pitch_fr, gain, ltp_param[2], pitres, zir, 0, filtIdx);
  }

  /******** Current-frame part ********/
  ltp_synth_filter(sig_out + enc_dec_delay + L_transition, sig_in + enc_dec_delay + L_transition,
                   L_frame - enc_dec_delay - L_transition, pitch_int, pitch_fr, gain, ltp_param[2],
                   pitres, NULL, 0, filtIdx);

  /******** Output ********/

  /* copy to output */
  for (i = 0; i < L_frame; i++) {
    sig[i] = FX_SGL2FX_DBL(sig_out[i]) >> PCM_OUT_HEADROOM;
  }

  /* Update */
  *pitch_int_past = pitch_int;
  *pitch_fr_past = pitch_fr;
  *gain_past = gain;
  *gainIdx_past = ltp_param[2];
  FDKmemcpy(mem_out, buf_out + L_frame, LTP_MEM_OUT_SIZE * sizeof(FIXP_SGL));

  return;
}
