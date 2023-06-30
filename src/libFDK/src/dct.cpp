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

/******************* Library for basic calculation routines ********************

   Author(s):

   Description: DCT Implementations
                Library functions to calculate standard DCTs. This will most
                likely be replaced by hand-optimized functions for the
                specific target processor.
                Three different implementations of the dct type II and the
                dct type III transforms are provided.
                By default implementations which are based on a single,
                standard complex FFT-kernel are used (dctII_f() and
                dctIII_f()). These are specifically helpful in cases where
                optimized FFT libraries are already available. The FFT used
                in theseimplementation is FFT rad2 from FDK_tools.
                Of course, one might also use DCT-libraries should they be
                available. The DCT and DST type IV implementations are only
                available in a version based on a complex FFT kernel.

*******************************************************************************/

#include "dct.h"

#include "FDK_tools_rom.h"
#include "fft.h"

#if defined(__arm__)
#include "arm/dct_arm.cpp"
#endif

void dct_getTables(const FIXP_WTP** ptwiddle, const FIXP_STP** sin_twiddle, int* sin_step,
                   int length) {
  const FIXP_WTP* twiddle;
  int ld2_length;

  /* Get ld2 of length - 2 + 1
      -2: because first table entry is window of size 4
      +1: because we already include +1 because of ceil(log2(length)) */
  ld2_length = DFRACT_BITS - 1 - fNormz((FIXP_DBL)length) - 1;

  /* Extract sort of "eigenvalue" (the 4 left most bits) of length. */
  switch ((length) >> (ld2_length - 1)) {
    case 0x4: /* radix 2 */
      *sin_twiddle = SineTable1024;
      *sin_step = 1 << (10 - ld2_length);
      twiddle = windowSlopes[0][0][ld2_length - 1];
      break;
    default:
      *sin_twiddle = NULL;
      *sin_step = 0;
      twiddle = NULL;
      break;
  }

  if (ptwiddle != NULL) {
    FDK_ASSERT(twiddle != NULL);
    *ptwiddle = twiddle;
  }

  FDK_ASSERT(*sin_step > 0);
}

#if !defined(FUNCTION_dct_III)
void dct_III(FIXP_DBL* pDat, /*!< pointer to input/output */
             FIXP_DBL* tmp,  /*!< pointer to temporal working buffer */
             int L,          /*!< lenght of transform */
             int* pDat_e) {
  const FIXP_WTP* sin_twiddle;
  int i;
  FIXP_DBL xr, accu1, accu2;
  int inc, index;
  int M = L >> 1;

  FDK_ASSERT(L % 8 == 0);
  dct_getTables(NULL, &sin_twiddle, &inc, L);
  inc >>= 1;

  FIXP_DBL* pTmp_0 = &tmp[2];
  FIXP_DBL* pTmp_1 = &tmp[(M - 1) * 2];

  index = 4 * inc;

#ifdef FUNCTION_dct_III_func1
  if ((M & (M - 1)) == 0) /* Do not call with L=12,20,24 */
  {
    dct_III_func1(tmp, pDat, sin_twiddle, inc, M);
  } else
#endif
  {
    /* This loop performs multiplication for index i (i*inc) */
    for (i = 1; i < M >> 1; i++, pTmp_0 += 2, pTmp_1 -= 2) {
      FIXP_DBL accu3, accu4, accu5, accu6;

      cplxMultDiv2(&accu2, &accu1, pDat[L - i], pDat[i], sin_twiddle[i * inc]);
      cplxMultDiv2(&accu4, &accu3, pDat[M + i], pDat[M - i], sin_twiddle[(M - i) * inc]);
      accu3 >>= 1;
      accu4 >>= 1;

      /* This method is better for ARM926, that uses operand2 shifted right by 1 always */
      if (i < (M / 4)) {
        cplxMultDiv2(&accu6, &accu5, (accu3 - (accu1 >> 1)), ((accu2 >> 1) + accu4),
                     sin_twiddle[index]);
      } else {
        cplxMultDiv2(&accu6, &accu5, ((accu2 >> 1) + accu4), (accu3 - (accu1 >> 1)),
                     sin_twiddle[index]);
        accu6 = -accu6;
      }
      xr = (accu1 >> 1) + accu3;
      pTmp_0[0] = (xr >> 1) - accu5;
      pTmp_1[0] = (xr >> 1) + accu5;

      xr = (accu2 >> 1) - accu4;
      pTmp_0[1] = (xr >> 1) - accu6;
      pTmp_1[1] = -((xr >> 1) + accu6);

      /* Create index helper variables for (4*i)*inc indexed equivalent values of short tables. */
      if (i < (M / 4)) {
        index += 4 * inc;
      } else {
        index -= 4 * inc;
      }
    }

    xr = fMultDiv2(pDat[M], sin_twiddle[M * inc].v.re); /* cos((PI/(2*L))*M); */
    tmp[0] = ((pDat[0] >> 1) + xr) >> 1;
    tmp[1] = ((pDat[0] >> 1) - xr) >> 1;

    cplxMultDiv2(&accu2, &accu1, pDat[L - (M / 2)], pDat[M / 2], sin_twiddle[M * inc / 2]);
    tmp[M] = accu1 >> 1;
    tmp[M + 1] = accu2 >> 1;
  }

  /* dit_fft expects 1 bit scaled input values */
  fft(M, tmp, pDat_e);

#ifdef FUNCTION_dct_III_func2
  dct_III_func2(tmp, pDat, M);
#else
  /* ARM926: 12 cycles per 2-iteration, no overhead code by compiler */
  pTmp_1 = &tmp[L];
  for (i = M >> 1; i--;) {
    FIXP_DBL tmp1, tmp2, tmp3, tmp4;
    tmp1 = *tmp++;
    tmp2 = *tmp++;
    tmp3 = *--pTmp_1;
    tmp4 = *--pTmp_1;
    *pDat++ = tmp1;
    *pDat++ = tmp3;
    *pDat++ = tmp2;
    *pDat++ = tmp4;
  }
#endif

  *pDat_e += 2;
}

void dst_III(FIXP_DBL* pDat, /*!< pointer to input/output */
             FIXP_DBL* tmp,  /*!< pointer to temporal working buffer */
             int L,          /*!< lenght of transform */
             int* pDat_e) {
  int L2 = L >> 1;
  int i;
  FIXP_DBL t;

  /* note: DCT III is reused here, direct DST III implementation might be more efficient */

  /* mirror input */
  for (i = 0; i < L2; i++) {
    t = pDat[i];
    pDat[i] = pDat[L - 1 - i];
    pDat[L - 1 - i] = t;
  }

  /* DCT-III */
  dct_III(pDat, tmp, L, pDat_e);

  /* flip signs at odd indices */
  for (i = 1; i < L; i += 2) pDat[i] = -pDat[i];
}

#endif

#if !defined(FUNCTION_dct_II)
void dct_II(FIXP_DBL* pDat, /*!< pointer to input/output */
            FIXP_DBL* tmp,  /*!< pointer to temporal working buffer */
            int L,          /*!< lenght of transform (has to be a multiple of 8 (or 4 in case
                               DCT_II_L_MULTIPLE_OF_4_SUPPORT is defined) */
            int* pDat_e) {
  const FIXP_WTP* sin_twiddle;

  int inc;
  int M = L >> 1;

  FDK_ASSERT(L % 8 == 0);
  dct_getTables(NULL, &sin_twiddle, &inc, L);
  inc >>= 1;

#if defined(FUNCTION_dct_II_func1)
  {
    FDK_ASSERT((M & 3) ==
               0); /* dct_II_func1 loads 8 values and stores 4 in lower and 4 in upper area */
    dct_II_func1(tmp, pDat, M);
  }
#endif
#if !defined(FUNCTION_dct_II_func1)
  {
    for (int i = 0; i < M; i++) {
      tmp[i] = pDat[2 * i] >> 2;
      tmp[L - 1 - i] = pDat[2 * i + 1] >> 2;
    }
  }
#endif

  fft(M, tmp, pDat_e);

  *pDat_e += 2;
#ifdef FUNCTION_dct_II_func2
  if ((M & (M - 1)) == 0) /* Not not call with L=12,20,24 */
  {
    dct_II_func2(tmp, pDat, sin_twiddle, inc, M);
    return;
  }
#endif

  int index = 0;
  FIXP_DBL accu1, accu2;

  /* iteration for i = 0 */
  cplxMult(&accu1, &accu2, tmp[M], tmp[M + 1], sin_twiddle[(M / 2) * inc]);
  pDat[L - (M / 2)] = accu2;
  pDat[M / 2] = accu1;

  pDat[0] = tmp[0] + tmp[1];
  pDat[M] = fMult(tmp[0] - tmp[1], sin_twiddle[M * inc].v.re); /* cos((PI/(2*L))*M); */

  index = 4 * inc;

  FIXP_DBL* pTmp_0 = &tmp[2];
  FIXP_DBL* pTmp_1 = &tmp[(M - 1) * 2];

  for (int i = 1; i < M >> 1; i++, pTmp_0 += 2, pTmp_1 -= 2) {
    FIXP_DBL a1, a2;
    FIXP_DBL accu3, accu4;

    a1 = (pTmp_0[1] >> 1) + (pTmp_1[1] >> 1);
    a2 = (pTmp_1[0] >> 1) - (pTmp_0[0] >> 1);

    if (i < (M / 4)) {
      cplxMult(&accu1, &accu2, a2, a1, sin_twiddle[index]);
    } else {
      cplxMult(&accu1, &accu2, a1, a2, sin_twiddle[index]);
      accu1 = -accu1;
    }
    a1 = (pTmp_0[0] >> 1) + (pTmp_1[0] >> 1);
    a2 = (pTmp_0[1] >> 1) - (pTmp_1[1] >> 1);

    cplxMult(&accu3, &accu4, (accu1 + a2), (a1 + accu2), sin_twiddle[i * inc]);
    pDat[L - i] = -accu3;
    pDat[i] = accu4;

    cplxMult(&accu3, &accu4, (accu1 - a2), (a1 - accu2), sin_twiddle[(M - i) * inc]);
    pDat[M + i] = -accu3;
    pDat[M - i] = accu4;

    /* Create index helper variables for (4*i)*inc indexed equivalent values of short tables. */
    if (i < (M / 4)) {
      index += 4 * inc;
    } else {
      index -= 4 * inc;
    }
  }
}
#endif

#if !defined(FUNCTION_dct_IV)

void dct_IV(FIXP_DBL* pDat, int L, int* pDat_e) {
  int sin_step = 0;
  int M = L >> 1;

  const FIXP_WTP* twiddle;
  const FIXP_STP* sin_twiddle;

  FDK_ASSERT(L >= 4);

  FDK_ASSERT(L >= 4);

  dct_getTables(&twiddle, &sin_twiddle, &sin_step, L);

#if defined(FUNCTION_dct_IV_ADSP21K)
  if (L == 128) {
    *pDat_e += 5 + 2;
    dct_IV_ADSP21K(M >> 2, pDat, twiddle, sin_twiddle, sin_step);
    return;
  }
  if (L == 1024) {
    *pDat_e += 8 + 2;
    dct_IV_ADSP21K(M >> 2, pDat, twiddle, sin_twiddle, sin_step);
    return;
  }
#endif

#ifdef FUNCTION_dct_IV_func1
  if (M >= 4 && (M & 3) == 0) {
    /* ARM926: 44 cycles for 2 iterations = 22 cycles/iteration */
    dct_IV_func1(M >> 2, twiddle, &pDat[0], &pDat[L - 1]);
  } else
#endif /* FUNCTION_dct_IV_func1 */
  {
    FIXP_DBL* pDat_0 = &pDat[0];
    FIXP_DBL* pDat_1 = &pDat[L - 2];
    int i;

    /* 29 cycles on ARM926 */
    for (i = 0; i < M - 1; i += 2, pDat_0 += 2, pDat_1 -= 2) {
      FIXP_DBL accu1, accu2, accu3, accu4;

      accu1 = pDat_1[1];
      accu2 = pDat_0[0];
      accu3 = pDat_0[1];
      accu4 = pDat_1[0];

      cplxMultDiv2(&accu1, &accu2, accu1, accu2, twiddle[i]);
      cplxMultDiv2(&accu3, &accu4, accu4, accu3, twiddle[i + 1]);

      pDat_0[0] = accu2 >> 1;
      pDat_0[1] = accu1 >> 1;
      pDat_1[0] = accu4 >> 1;
      pDat_1[1] = -(accu3 >> 1);
    }
    if (M & 1) {
      FIXP_DBL accu1, accu2;

      accu1 = pDat_1[1];
      accu2 = pDat_0[0];

      cplxMultDiv2(&accu1, &accu2, accu1, accu2, twiddle[i]);

      pDat_0[0] = accu2 >> 1;
      pDat_0[1] = accu1 >> 1;
    }
  }

  fft(M, pDat, pDat_e);

#ifdef FUNCTION_dct_IV_func2
  if (M >= 4 && (M & 3) == 0) {
    /* ARM926: 42 cycles for 2 iterations = 21 cycles/iteration */
    dct_IV_func2(M >> 2, sin_twiddle, &pDat[0], &pDat[L], sin_step);
  } else
#endif /* FUNCTION_dct_IV_func2 */
  {
    FIXP_DBL* pDat_0 = &pDat[0];
    FIXP_DBL* pDat_1 = &pDat[L - 2];
    FIXP_DBL accu1, accu2, accu3, accu4;
    int idx, i;

    /* Sin and Cos values are 0.0f and 1.0f */
    accu1 = pDat_1[0];
    accu2 = pDat_1[1];

    pDat_1[1] = -pDat_0[1];
    /* pDat_0[0] = pDat_0[0]; */

    /* 28 cycles for ARM926 */
    for (idx = sin_step, i = 1; i < (M + 1) >> 1; i++, idx += sin_step) {
      FIXP_STP twd = sin_twiddle[idx];
      cplxMult(&accu3, &accu4, accu1, accu2, twd);
      pDat_0[1] = accu3;
      pDat_1[0] = accu4;

      pDat_0 += 2;
      pDat_1 -= 2;

      cplxMult(&accu3, &accu4, pDat_0[1], pDat_0[0], twd);

      accu1 = pDat_1[0];
      accu2 = pDat_1[1];

      pDat_1[1] = -accu3;
      pDat_0[0] = accu4;
    }

    if ((M & 1) == 0) {
      /* Last Sin and Cos value pair are the same */
      accu1 = fMult(accu1, WTC(0x5a82799a));
      accu2 = fMult(accu2, WTC(0x5a82799a));

      pDat_1[0] = accu1 + accu2;
      pDat_0[1] = accu1 - accu2;
    }
  }

  /* Add twiddeling scale. */
  *pDat_e += 2;
}
#endif /* defined (FUNCTION_dct_IV) */

#if !defined(FUNCTION_dst_IV)
void dst_IV(FIXP_DBL* pDat, int L, int* pDat_e) {
  int sin_step = 0;
  int M = L >> 1;

  const FIXP_WTP* twiddle;
  const FIXP_STP* sin_twiddle;

  FDK_ASSERT(L >= 4);

  FDK_ASSERT(L >= 4);

  dct_getTables(&twiddle, &sin_twiddle, &sin_step, L);

#if defined(FUNCTION_dst_IV_ADSP21K)
  if (L == 128) {
    *pDat_e += 5 + 2;
    dst_IV_ADSP21K(M >> 2, pDat, twiddle, sin_twiddle, sin_step);
    return;
  }
  if (L == 1024) {
    *pDat_e += 8 + 2;
    dst_IV_ADSP21K(M >> 2, pDat, twiddle, sin_twiddle, sin_step);
    return;
  }
#endif

#ifdef FUNCTION_dst_IV_func1
  if ((M >= 4) && ((M & 3) == 0)) {
    dst_IV_func1(M, twiddle, &pDat[0], &pDat[L]);
  } else
#endif
  {
    FIXP_DBL* pDat_0 = &pDat[0];
    FIXP_DBL* pDat_1 = &pDat[L - 2];
    int i;

    /* 34 cycles on ARM926 */
    for (i = 0; i < M - 1; i += 2, pDat_0 += 2, pDat_1 -= 2) {
      FIXP_DBL accu1, accu2, accu3, accu4;

      accu1 = pDat_1[1] >> 1;
      accu2 = -(pDat_0[0] >> 1);
      accu3 = pDat_0[1] >> 1;
      accu4 = -(pDat_1[0] >> 1);

      cplxMultDiv2(&accu1, &accu2, accu1, accu2, twiddle[i]);
      cplxMultDiv2(&accu3, &accu4, accu4, accu3, twiddle[i + 1]);

      pDat_0[0] = accu2;
      pDat_0[1] = accu1;
      pDat_1[0] = accu4;
      pDat_1[1] = -accu3;
    }
    if (M & 1) {
      FIXP_DBL accu1, accu2;

      accu1 = pDat_1[1];
      accu2 = -pDat_0[0];

      cplxMultDiv2(&accu1, &accu2, accu1, accu2, twiddle[i]);

      pDat_0[0] = accu2 >> 1;
      pDat_0[1] = accu1 >> 1;
    }
  }

  fft(M, pDat, pDat_e);

#ifdef FUNCTION_dst_IV_func2
  if ((M >= 4) && ((M & 3) == 0)) {
    dst_IV_func2(M >> 2, sin_twiddle + sin_step, &pDat[0], &pDat[L - 1], sin_step);
  } else
#endif /* FUNCTION_dst_IV_func2 */
  {
    FIXP_DBL* pDat_0;
    FIXP_DBL* pDat_1;
    FIXP_DBL accu1, accu2, accu3, accu4;
    int idx, i;

    pDat_0 = &pDat[0];
    pDat_1 = &pDat[L - 2];

    /* Sin and Cos values are 0.0f and 1.0f */
    accu1 = pDat_1[0];
    accu2 = pDat_1[1];

    pDat_1[1] = -pDat_0[0];
    pDat_0[0] = pDat_0[1];

    for (idx = sin_step, i = 1; i < (M + 1) >> 1; i++, idx += sin_step) {
      FIXP_STP twd = sin_twiddle[idx];

      cplxMult(&accu3, &accu4, accu1, accu2, twd);
      pDat_1[0] = -accu3;
      pDat_0[1] = -accu4;

      pDat_0 += 2;
      pDat_1 -= 2;

      cplxMult(&accu3, &accu4, pDat_0[1], pDat_0[0], twd);

      accu1 = pDat_1[0];
      accu2 = pDat_1[1];

      pDat_0[0] = accu3;
      pDat_1[1] = -accu4;
    }

    if ((M & 1) == 0) {
      /* Last Sin and Cos value pair are the same */
      accu1 = fMult(accu1, WTC(0x5a82799a));
      accu2 = fMult(accu2, WTC(0x5a82799a));

      pDat_0[1] = -accu1 - accu2;
      pDat_1[0] = accu2 - accu1;
    }
  }

  /* Add twiddeling scale. */
  *pDat_e += 2;
}
#endif /* !defined(FUNCTION_dst_IV) */
