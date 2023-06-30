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

   Description: fixed point intrinsics

*******************************************************************************/

#if !defined(CPLX_MUL_H)
#define CPLX_MUL_H

#include "common_fix.h"

#if defined(__arm__) || defined(_M_ARM)
#include "arm/cplx_mul_arm.h"

#endif /* #if defined all cores: bfin, arm, etc. */

/* ############################################################################# */

/* Fallback generic implementations */

#if !defined(FUNCTION_cplxMultDiv2_32x16X2)
#define FUNCTION_cplxMultDiv2_32x16X2

inline void cplxMultDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                         const FIXP_SGL b_Re, const FIXP_SGL b_Im) {
  *c_Re = fMultDiv2(a_Re, b_Re) - fMultDiv2(a_Im, b_Im);
  *c_Im = fMultDiv2(a_Re, b_Im) + fMultDiv2(a_Im, b_Re);
}
#endif

#if !defined(FUNCTION_cplxMultDiv2_16x16X2)
#define FUNCTION_cplxMultDiv2_16x16X2

inline void cplxMultDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_SGL a_Re, const FIXP_SGL a_Im,
                         const FIXP_SGL b_Re, const FIXP_SGL b_Im) {
  *c_Re = fMultDiv2(a_Re, b_Re) - fMultDiv2(a_Im, b_Im);
  *c_Im = fMultDiv2(a_Re, b_Im) + fMultDiv2(a_Im, b_Re);
}

inline void cplxMultDiv2(FIXP_SGL* c_Re, FIXP_SGL* c_Im, const FIXP_SGL a_Re, const FIXP_SGL a_Im,
                         const FIXP_SGL b_Re, const FIXP_SGL b_Im) {
  *c_Re = FX_DBL2FX_SGL(fMultDiv2(a_Re, b_Re) - fMultDiv2(a_Im, b_Im));
  *c_Im = FX_DBL2FX_SGL(fMultDiv2(a_Re, b_Im) + fMultDiv2(a_Im, b_Re));
}
#endif

#if !defined(FUNCTION_cplxMultDiv2_32x16)
#define FUNCTION_cplxMultDiv2_32x16

inline void cplxMultDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                         const FIXP_SPK w) {
  cplxMultDiv2(c_Re, c_Im, a_Re, a_Im, w.v.re, w.v.im);
}
#endif

#if !defined(FUNCTION_cplxMultDiv2_16x16)
#define FUNCTION_cplxMultDiv2_16x16

inline void cplxMultDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_SGL a_Re, const FIXP_SGL a_Im,
                         const FIXP_SPK w) {
  cplxMultDiv2(c_Re, c_Im, a_Re, a_Im, w.v.re, w.v.im);
}

inline void cplxMultDiv2(FIXP_SGL* c_Re, FIXP_SGL* c_Im, const FIXP_SGL a_Re, const FIXP_SGL a_Im,
                         const FIXP_SPK w) {
  cplxMultDiv2(c_Re, c_Im, a_Re, a_Im, w.v.re, w.v.im);
}
#endif

#if !defined(FUNCTION_cplxMultAddDiv2_32x16X2)
#define FUNCTION_cplxMultAddDiv2_32x16X2

inline void cplxMultAddDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re,
                            const FIXP_DBL a_Im, const FIXP_SGL b_Re, const FIXP_SGL b_Im) {
  *c_Re += fMultDiv2(a_Re, b_Re) - fMultDiv2(a_Im, b_Im);
  *c_Im += fMultDiv2(a_Re, b_Im) + fMultDiv2(a_Im, b_Re);
}
#endif

#if !defined(FUNCTION_cplxMultAddDiv2_32x16)
#define FUNCTION_cplxMultAddDiv2_32x16

inline void cplxMultAddDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re,
                            const FIXP_DBL a_Im, const FIXP_SPK w) {
  cplxMultAddDiv2(c_Re, c_Im, a_Re, a_Im, w.v.re, w.v.im);
}
#endif

#if !defined(FUNCTION_cplxMultSubDiv2_32x16X2)
#define FUNCTION_cplxMultSubDiv2_32x16X2

inline void cplxMultSubDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re,
                            const FIXP_DBL a_Im, const FIXP_SGL b_Re, const FIXP_SGL b_Im) {
  *c_Re -= fMultDiv2(a_Re, b_Re) - fMultDiv2(a_Im, b_Im);
  *c_Im -= fMultDiv2(a_Re, b_Im) + fMultDiv2(a_Im, b_Re);
}
#endif

#if !defined(FUNCTION_cplxMultDiv2_32x32X2)
#define FUNCTION_cplxMultDiv2_32x32X2

inline void cplxMultDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                         const FIXP_DBL b_Re, const FIXP_DBL b_Im) {
  *c_Re = fMultDiv2(a_Re, b_Re) - fMultDiv2(a_Im, b_Im);
  *c_Im = fMultDiv2(a_Re, b_Im) + fMultDiv2(a_Im, b_Re);
}
#endif

#if !defined(FUNCTION_cplxMultDiv2_32x32)
#define FUNCTION_cplxMultDiv2_32x32

inline void cplxMultDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                         const FIXP_DPK w) {
  cplxMultDiv2(c_Re, c_Im, a_Re, a_Im, w.v.re, w.v.im);
}
#endif

#if !defined(FUNCTION_cplxMultSubDiv2_32x32X2)
#define FUNCTION_cplxMultSubDiv2_32x32X2

inline void cplxMultSubDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re,
                            const FIXP_DBL a_Im, const FIXP_DBL b_Re, const FIXP_DBL b_Im) {
  *c_Re -= fMultDiv2(a_Re, b_Re) - fMultDiv2(a_Im, b_Im);
  *c_Im -= fMultDiv2(a_Re, b_Im) + fMultDiv2(a_Im, b_Re);
}
#endif

#if !defined(FUNCTION_cplxMultAddDiv2_32x32X2)
#define FUNCTION_cplxMultAddDiv2_32x32X2

inline void cplxMultAddDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re,
                            const FIXP_DBL a_Im, const FIXP_DBL b_Re, const FIXP_DBL b_Im) {
  *c_Re += fMultDiv2(a_Re, b_Re) - fMultDiv2(a_Im, b_Im);
  *c_Im += fMultDiv2(a_Re, b_Im) + fMultDiv2(a_Im, b_Re);
}
#endif

/* ############################################################################# */

#if !defined(FUNCTION_cplxMult_32x16X2)
#define FUNCTION_cplxMult_32x16X2

inline void cplxMult(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                     const FIXP_SGL b_Re, const FIXP_SGL b_Im) {
  *c_Re = fMult(a_Re, b_Re) - fMult(a_Im, b_Im);
  *c_Im = fMult(a_Re, b_Im) + fMult(a_Im, b_Re);
}
inline void cplxMult(FIXP_SGL* c_Re, FIXP_SGL* c_Im, const FIXP_SGL a_Re, const FIXP_SGL a_Im,
                     const FIXP_SGL b_Re, const FIXP_SGL b_Im) {
  *c_Re = FX_DBL2FX_SGL(fMult(a_Re, b_Re) - fMult(a_Im, b_Im));
  *c_Im = FX_DBL2FX_SGL(fMult(a_Re, b_Im) + fMult(a_Im, b_Re));
}
#endif

#if !defined(FUNCTION_cplxMult_32x16)
#define FUNCTION_cplxMult_32x16

inline void cplxMult(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                     const FIXP_SPK w) {
  cplxMult(c_Re, c_Im, a_Re, a_Im, w.v.re, w.v.im);
}
#endif

#if !defined(FUNCTION_cplxMult_32x32X2)
#define FUNCTION_cplxMult_32x32X2

inline void cplxMult(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                     const FIXP_DBL b_Re, const FIXP_DBL b_Im) {
  *c_Re = fMult(a_Re, b_Re) - fMult(a_Im, b_Im);
  *c_Im = fMult(a_Re, b_Im) + fMult(a_Im, b_Re);
}
#endif

#if !defined(FUNCTION_cplxMult_32x32)
#define FUNCTION_cplxMult_32x32
inline void cplxMult(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                     const FIXP_DPK w) {
  cplxMult(c_Re, c_Im, a_Re, a_Im, w.v.re, w.v.im);
}
#endif

/* ############################################################################# */

#endif /* CPLX_MUL_H */
