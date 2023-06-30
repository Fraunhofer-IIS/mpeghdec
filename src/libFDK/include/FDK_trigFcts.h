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

   Author(s):   Haricharan Lakshman, Manuel Jander

   Description: Trigonometric functions fixed point fractional implementation.

*******************************************************************************/

#if !defined(FDK_TRIGFCTS_H)
#define FDK_TRIGFCTS_H

#include "common_fix.h"

#include "FDK_tools_rom.h"

FIXP_DBL fixp_cos(FIXP_DBL x, int scale);
FIXP_DBL fixp_sin(FIXP_DBL x, int scale);
void fixp_cos_sin(FIXP_DBL x, int scale, FIXP_DBL* cos, FIXP_DBL* sin);

#define FIXP_COS_SIN

#include "FDK_tools_rom.h"

#define SINETAB SineTable512
#define LD 9

#ifndef FUNCTION_inline_fixp_cos_sin

#define FUNCTION_inline_fixp_cos_sin

/*
 * Calculates coarse lookup index and sign for sine.
 * Returns delta x residual.
 */
static inline FIXP_DBL fixp_sin_cos_residual_inline(FIXP_DBL x, int scale, FIXP_DBL* sine,
                                                    FIXP_DBL* cosine) {
  FIXP_DBL residual;
  int s;
  int shift = (31 - scale - LD - 1);
  int ssign = 1;
  int csign = 1;

  residual = fMult(x, FL2FXCONST_DBL(1.0 / M_PI));
  s = ((LONG)residual) >> shift;

  residual &= ((1 << shift) - 1);
  residual = fMult(residual, FL2FXCONST_DBL(M_PI / 4.0)) << 2;
  residual <<= scale;

  /* Sine sign symmetry */
  if (s & ((1 << LD) << 1)) {
    ssign = -ssign;
  }
  /* Cosine sign symmetry */
  if ((s + (1 << LD)) & ((1 << LD) << 1)) {
    csign = -csign;
  }

  s = fAbs(s);

  s &= (((1 << LD) << 1) - 1); /* Modulo PI */

  if (s > (1 << LD)) {
    s = ((1 << LD) << 1) - s;
  }

  {
    LONG sl, cl;
    /* Because of packed table */
    if (s > (1 << (LD - 1))) {
      FIXP_STP tmp;
      /* Cosine/Sine simetry for angles greater than PI/4 */
      s = (1 << LD) - s;
      tmp = SINETAB[s];
      sl = (LONG)tmp.v.re;
      cl = (LONG)tmp.v.im;
    } else {
      FIXP_STP tmp;
      tmp = SINETAB[s];
      sl = (LONG)tmp.v.im;
      cl = (LONG)tmp.v.re;
    }

#ifdef SINETABLE_16BIT
    *sine = (FIXP_DBL)((sl * ssign) << (DFRACT_BITS - FRACT_BITS));
    *cosine = (FIXP_DBL)((cl * csign) << (DFRACT_BITS - FRACT_BITS));
#else
    /* scale down by 1 for overflow prevention. This is undone at the calling function. */
    *sine = (FIXP_DBL)(sl * ssign) >> 1;
    *cosine = (FIXP_DBL)(cl * csign) >> 1;
#endif
  }

  return residual;
}

/**
 * \brief Calculate cosine and sine value each of 2 angles different angle values.
 * \param x1 first angle value
 * \param x2 second angle value
 * \param scale exponent of x1 and x2
 * \param out pointer to 4 FIXP_DBL locations, were the values cos(x1), sin(x1), cos(x2), sin(x2)
 *            will be stored into.
 */
static inline void inline_fixp_cos_sin(FIXP_DBL x1, FIXP_DBL x2, const int scale, FIXP_DBL* out) {
  FIXP_DBL residual, error0, error1, sine, cosine;
  residual = fixp_sin_cos_residual_inline(x1, scale, &sine, &cosine);
  error0 = fMultDiv2(sine, residual);
  error1 = fMultDiv2(cosine, residual);

#ifdef SINETABLE_16BIT
  *out++ = cosine - (error0 << 1);
  *out++ = sine + (error1 << 1);
#else
  /* Undo downscaling by 1 which was done at fixp_sin_cos_residual_inline */
  *out++ = SATURATE_LEFT_SHIFT(cosine - (error0 << 1), 1, DFRACT_BITS);
  *out++ = SATURATE_LEFT_SHIFT(sine + (error1 << 1), 1, DFRACT_BITS);
#endif

  residual = fixp_sin_cos_residual_inline(x2, scale, &sine, &cosine);
  error0 = fMultDiv2(sine, residual);
  error1 = fMultDiv2(cosine, residual);

#ifdef SINETABLE_16BIT
  *out++ = cosine - (error0 << 1);
  *out++ = sine + (error1 << 1);
#else
  *out++ = SATURATE_LEFT_SHIFT(cosine - (error0 << 1), 1, DFRACT_BITS);
  *out++ = SATURATE_LEFT_SHIFT(sine + (error1 << 1), 1, DFRACT_BITS);
#endif
}
#endif

#endif /* !defined(FDK_TRIGFCTS_H) */
