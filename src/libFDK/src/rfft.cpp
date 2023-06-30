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

   Author(s):   Josef Hoepfl, DSP Solutions

   Description: Fix point RFFT

*******************************************************************************/

#include "rfft.h"

#include "fixpoint_math.h"
#include "fft.h"
#include "FDK_tools_rom.h"

/*------------------ IRFFT constants -----------------------*/

/*--------------- function declarations --------------------*/
#if !defined(FUNCTION_fft_postsort)
static void fft_postsort(FIXP_DBL* const buf, const INT N, INT* const scalefactor);
#endif /* FUNCTION_fft_postsort */

#if !defined(FUNCTION_fft_presort)
static void fft_presort(FIXP_DBL* const buf, const INT N, INT* const scalefactor);
#endif /* FUNCTION_fft_presort */

#if defined(__arm__)
#include "arm/rfft_arm.cpp"
#endif

/*------------- function definitions ----------------*/
#ifndef FUNCTION_rfft
INT rfft(const UINT fftLength, FIXP_DBL* const pBuffer, const UINT bufferSize,
         INT* const scalefactor) {
  FDK_ASSERT(pBuffer != NULL);
  FDK_ASSERT(scalefactor != NULL);
  FDK_ASSERT(bufferSize >= sizeof(FIXP_DBL) * fftLength);

  fft(fftLength / 2, pBuffer, scalefactor);

  fft_postsort(pBuffer, fftLength, scalefactor);

  if (bufferSize >= sizeof(FIXP_DBL) * (fftLength + 2)) {
    /* im[0] contains re[n], because im[0] is zero. This allows use of N+1 length buffer instead of
     * N.  */
    pBuffer[fftLength] = pBuffer[1];
    pBuffer[fftLength + 1] = FL2FXCONST_DBL(0.f);
    pBuffer[1] = FL2FXCONST_DBL(0.f);
  }
  return 0;
}
#endif /* #ifndef FUNCTION_rfft */

#ifndef FUNCTION_irfft
INT irfft(const UINT fftLength, FIXP_DBL* const pBuffer, const UINT bufferSize,
          INT* const scalefactor) {
  FDK_ASSERT(pBuffer != NULL);
  FDK_ASSERT(scalefactor != NULL);
  FDK_ASSERT(bufferSize >= sizeof(FIXP_DBL) * fftLength);

  INT fft_e = 0;

  if (bufferSize >= sizeof(FIXP_DBL) * (fftLength + 1)) {
    /* im[0] contains re[n], because im[0] is zero. This allows use of N length buffer instead of
     * N+1. */
    pBuffer[1] = pBuffer[fftLength];
  }

  fft_presort(pBuffer, fftLength, &fft_e);

  fft(fftLength / 2, pBuffer, &fft_e);

  return 0;
}
#endif /*FUNCTION_irfft*/

void getSineTab(const UINT length, const FIXP_STP** ppSineTab, UINT* pStep) {
  switch (length) {
    case 64:
      *ppSineTab = SineTable512;
      *pStep = 1 << (9 - 4);
      break;
    case 128:
      *ppSineTab = SineTable512;
      *pStep = 1 << (9 - 5);
      break;
    case 256:
      *ppSineTab = SineTable512;
      *pStep = 1 << (9 - 6);
      break;
    case 512:
      *ppSineTab = SineTable512;
      *pStep = 1 << (9 - 7);
      break;
    default:
      FDK_ASSERT(0 && "Unsupported length in getSineTab().");
      *ppSineTab = NULL;
      *pStep = 0;
      break;
  }
}

#if !defined(FUNCTION_fft_postsort)
static void fft_postsort(FIXP_DBL* const buf, const INT N, INT* const scalefactor) {
  const FIXP_STP* w;
  UINT step;

  *scalefactor += 1;

  getSineTab(N, &w, &step);

#ifdef FUNCTION_fft_postsort_func1
  fft_postsort_func1(buf, N, w, step);
#else
  FIXP_DBL tmp1, tmp2;
  INT n;
  tmp1 = buf[0];
  buf[0] = ((tmp1 >> 1) + (buf[1] >> 1));
  buf[1] = ((tmp1 >> 1) - (buf[1] >> 1)); /* imag part of first spec value X[0] = 0   */

  FIXP_DBL* pBuf_0 = &buf[2];
  FIXP_DBL* pBuf_1 = &buf[N - 2];
  FIXP_DBL* pBuf_2 = &buf[N / 2 - 2];
  FIXP_DBL* pBuf_3 = &buf[N / 2 + 2];

  /* Postsorting algorithm */

  for (n = ((N >> 3) - 1), w += step; n != 0;
       n--, pBuf_0 += 2, pBuf_1 -= 2, pBuf_2 -= 2, pBuf_3 += 2, w += step) {
    FIXP_DBL sre, sim, dre, dim;

    sre = ((pBuf_0[0] >> 1) + (pBuf_1[0] >> 1)); /* sum re  */
    sim = ((pBuf_0[1] >> 1) + (pBuf_1[1] >> 1)); /* sum im  */
    dre = ((pBuf_0[0] >> 1) - (pBuf_1[0] >> 1)); /* diff re */
    dim = ((pBuf_0[1] >> 1) - (pBuf_1[1] >> 1)); /* diff im */

    cplxMultDiv2(&tmp1, &tmp2, sim, dre, *w);

    pBuf_0[0] = ((sre >> 1) + tmp1);
    pBuf_0[1] = ((dim >> 1) - tmp2);
    pBuf_1[0] = ((sre >> 1) - tmp1);
    pBuf_1[1] = -((dim >> 1) + tmp2);

    sre = ((pBuf_2[0] >> 1) + (pBuf_3[0] >> 1)); /* sum re  */
    sim = ((pBuf_2[1] >> 1) + (pBuf_3[1] >> 1)); /* sum im  */
    dre = ((pBuf_2[0] >> 1) - (pBuf_3[0] >> 1)); /* diff re */
    dim = ((pBuf_2[1] >> 1) - (pBuf_3[1] >> 1)); /* diff im */

    cplxMultDiv2(&tmp1, &tmp2, dre, sim, *w);

    pBuf_2[0] = ((sre >> 1) - tmp1);
    pBuf_2[1] = ((dim >> 1) - tmp2);
    pBuf_3[0] = ((sre >> 1) + tmp1);
    pBuf_3[1] = -((dim >> 1) + tmp2);
  }

  {
    FIXP_DBL sre, sim, dre, dim;

    sre = ((pBuf_2[0] >> 1) + (pBuf_3[0] >> 1)); /* sum re  */
    sim = ((pBuf_2[1] >> 1) + (pBuf_3[1] >> 1)); /* sum im  */
    dre = ((pBuf_2[0] >> 1) - (pBuf_3[0] >> 1)); /* diff re */
    dim = ((pBuf_2[1] >> 1) - (pBuf_3[1] >> 1)); /* diff im */

    cplxMultDiv2(&tmp1, &tmp2, sim, dre, *w);

    pBuf_0[0] = ((sre >> 1) + tmp1);
    pBuf_0[1] = ((dim >> 1) - tmp2);
    pBuf_1[0] = ((sre >> 1) - tmp1);
    pBuf_1[1] = -((dim >> 1) + tmp2);
  }

  buf[(N >> 1)] = (buf[(N >> 1)]) >> 1;
  buf[(N >> 1) + 1] = -(buf[(N >> 1) + 1]) >> 1;

#endif /* FUNCTION_fft_postsort_func1 */
}
#endif

#if !defined(FUNCTION_fft_presort)
static void fft_presort(FIXP_DBL* const buf, const INT N, INT* const scalefactor) {
  const FIXP_STP* w;
  UINT step;

  *scalefactor += 1;
  getSineTab(N, &w, &step);

#ifdef FUNCTION_fft_presort_func1
  fft_presort_func1(buf, N, w, step);
#else
  INT n;
  FIXP_DBL tmp1, tmp2;

  tmp1 = buf[0];
  buf[0] = ((tmp1 >> 1) + (buf[1] >> 1)) >> 1;
  buf[1] = ((tmp1 >> 1) - (buf[1] >> 1)) >> 1;

  FIXP_DBL* pBuf_0 = &buf[2];
  FIXP_DBL* pBuf_1 = &buf[N - 2];
  FIXP_DBL* pBuf_2 = &buf[N / 2 - 2];
  FIXP_DBL* pBuf_3 = &buf[N / 2 + 2];

  /* Presorting algorithm */
  for (n = ((N >> 3) - 1), w += step; n != 0;
       n--, pBuf_0 += 2, pBuf_1 -= 2, pBuf_2 -= 2, pBuf_3 += 2, w += step) {
    FIXP_DBL sre, sim, dre, dim;

    sre = ((pBuf_0[0] >> 1) + (pBuf_1[0] >> 1)); /* sum re  */
    sim = ((pBuf_0[1] >> 1) + (pBuf_1[1] >> 1)); /* sum im  */
    dre = ((pBuf_0[0] >> 1) - (pBuf_1[0] >> 1)); /* diff re */
    dim = ((pBuf_0[1] >> 1) - (pBuf_1[1] >> 1)); /* diff im */

    cplxMultDiv2(&tmp2, &tmp1, dre, sim, *w);

    pBuf_0[0] = ((sre >> 1) + tmp1);
    pBuf_0[1] = (tmp2 - (dim >> 1));
    pBuf_1[0] = ((sre >> 1) - tmp1);
    pBuf_1[1] = (tmp2 + (dim >> 1));

    sre = ((pBuf_2[0] >> 1) + (pBuf_3[0] >> 1)); /* sum re  */
    sim = ((pBuf_2[1] >> 1) + (pBuf_3[1] >> 1)); /* sum im  */
    dre = ((pBuf_2[0] >> 1) - (pBuf_3[0] >> 1)); /* diff re */
    dim = ((pBuf_2[1] >> 1) - (pBuf_3[1] >> 1)); /* diff im */

    cplxMultDiv2(&tmp2, &tmp1, sim, dre, *w);

    pBuf_2[0] = ((sre >> 1) + tmp1);
    pBuf_2[1] = -((dim >> 1) + tmp2);
    pBuf_3[0] = ((sre >> 1) - tmp1);
    pBuf_3[1] = ((dim >> 1) - tmp2);
  }

  {
    FIXP_DBL sre, sim, dre, dim;

    sre = ((pBuf_2[0] >> 1) + (pBuf_3[0] >> 1)); /* sum re  */
    sim = ((pBuf_2[1] >> 1) + (pBuf_3[1] >> 1)); /* sum im  */
    dre = ((pBuf_2[0] >> 1) - (pBuf_3[0] >> 1)); /* diff re */
    dim = ((pBuf_2[1] >> 1) - (pBuf_3[1] >> 1)); /* diff im */

    cplxMultDiv2(&tmp2, &tmp1, sim, dre, *w);

    pBuf_2[0] = ((sre >> 1) + tmp1);
    pBuf_2[1] = -((dim >> 1) + tmp2);
    pBuf_3[0] = ((sre >> 1) - tmp1);
    pBuf_3[1] = ((dim >> 1) - tmp2);
  }

  buf[(N >> 1) + 1] = -buf[(N >> 1) + 1] >> 1;
  buf[(N >> 1)] = buf[(N >> 1)] >> 1;

#endif /* FUNCTION_fft_presort_func1 */
}
#endif /*FUNCTION_fft_presort*/
