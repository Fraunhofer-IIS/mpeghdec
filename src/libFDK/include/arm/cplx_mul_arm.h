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

#if !defined(CPLX_MUL_ARM_H)
#define CPLX_MUL_ARM_H

#if defined(__arm__) && defined(__GNUC__)

#if defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_8__)

#define FUNCTION_cplxMultDiv2_32x32X2
#define FUNCTION_cplxMultDiv2_32x16X2
#define FUNCTION_cplxMultDiv2_32x16

#ifdef __ARM_ARCH_8__
#define FUNCTION_cplxMult_32x32X2
#define FUNCTION_cplxMult_32x16X2
//#define FUNCTION_cplxMult_32x16

#define FUNCTION_cplxMultAddDiv2_32x32X2
#define FUNCTION_cplxMultAddDiv2_32x16X2
//#define FUNCTION_cplxMultAddDiv2_32x16

#define FUNCTION_cplxMultSubDiv2_32x32X2
#define FUNCTION_cplxMultSubDiv2_32x16X2
//#define FUNCTION_cplxMultSubDiv2_32x16
#endif

#endif

#ifdef FUNCTION_cplxMultDiv2_32x32X2
inline void cplxMultDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                         const FIXP_DBL b_Re, const FIXP_DBL b_Im) {
#ifdef __ARM_ARCH_8__
  INT64 tmp1, tmp2;
  asm("smull  %x0, %w2, %w4; \n"      /* tmp1  = a_Re * b_Re */
      "smull  %x1, %w2, %w5; \n"      /* tmp2  = a_Re * b_Im */
      "smsubl %x0, %w3, %w5, %x0; \n" /* tmp1 -= a_Im * b_Im */
      "smaddl %x1, %w3, %w4, %x1; \n" /* tmp2 += a_Im * b_Re */
      "asr    %x0, %x0, #32; \n"
      "asr    %x1, %x1, #32; \n"
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"(b_Re), "r"(b_Im));
  *c_Re = (LONG)tmp1;
  *c_Im = (LONG)tmp2;
#elif defined(__ARM_ARCH_6__)
  LONG tmp1, tmp2;
  asm("smmul %0, %2, %4; \n"     /* tmp1  = a_Re * b_Re */
      "smmls %0, %3, %5, %0; \n" /* tmp1 -= a_Im * b_Im */
      "smmul %1, %2, %5; \n"     /* tmp2  = a_Re * b_Im */
      "smmla %1, %3, %4, %1; \n" /* tmp2 += a_Im * b_Re */
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"(b_Re), "r"(b_Im));
  *c_Re = tmp1;
  *c_Im = tmp2;
#else
  LONG tmp1, tmp2, discard;
  asm("smull %2, %0, %7, %6; \n" /* tmp1  = -a_Im * b_Im */
      "smlal %2, %0, %3, %5; \n" /* tmp1 +=  a_Re * b_Re */
      "smull %2, %1, %3, %6; \n" /* tmp2  =  a_Re * b_Im */
      "smlal %2, %1, %4, %5; \n" /* tmp2 +=  a_Im * b_Re */
      : "=&r"(tmp1), "=&r"(tmp2), "=&r"(discard)
      : "r"(a_Re), "r"(a_Im), "r"(b_Re), "r"(b_Im), "r"(-a_Im));
  *c_Re = tmp1;
  *c_Im = tmp2;
#endif
}
#endif /* FUNCTION_cplxMultDiv2_32x32X2 */

#ifdef FUNCTION_cplxMultDiv2_32x16X2
inline void cplxMultDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                         const FIXP_SGL b_Re, const FIXP_SGL b_Im) {
#ifdef __ARM_ARCH_8__
  INT64 tmp1, tmp2;
  asm("smull  %x0, %w2, %w4; \n"      /* tmp1  = a_Re * b_Re */
      "smull  %x1, %w2, %w5; \n"      /* tmp2  = a_Re * b_Im */
      "smsubl %x0, %w3, %w5, %x0; \n" /* tmp1 -= a_Im * b_Im */
      "smaddl %x1, %w3, %w4, %x1; \n" /* tmp2 += a_Im * b_Re */
      "asr    %x0, %x0, #16; \n"
      "asr    %x1, %x1, #16; \n"
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"((LONG)b_Re), "r"((LONG)b_Im));
  *c_Re = (LONG)tmp1;
  *c_Im = (LONG)tmp2;
#else
  LONG tmp1, tmp2;
  asm("smulwb %0, %3, %5;\n" /* %7   = -a_Im * b_Im */
      "rsb %1,%0,#0;\n"
      "smlawb %0, %2, %4, %1;\n" /* tmp1 =  a_Re * b_Re - a_Im * b_Im */
      "smulwb %1, %2, %5;\n"     /* %7   =  a_Re * b_Im */
      "smlawb %1, %3, %4, %1;\n" /* tmp2 =  a_Im * b_Re + a_Re * b_Im */
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"(b_Re), "r"(b_Im));
  *c_Re = tmp1;
  *c_Im = tmp2;
#endif
}
#endif /* FUNCTION_cplxMultDiv2_32x16X2 */

#ifdef FUNCTION_cplxMultDiv2_32x16
inline void cplxMultDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                         FIXP_SPK wpk) {
#ifdef __ARM_ARCH_8__
  cplxMultDiv2(c_Re, c_Im, a_Re, a_Im, wpk.v.re, wpk.v.im);
#else
  LONG tmp1, tmp2;
  const LONG w = wpk.w;
  asm("smulwt %0, %3, %4;\n"
      "rsb %1,%0,#0;\n"
      "smlawb %0, %2, %4, %1;\n"
      "smulwt %1, %2, %4;\n"
      "smlawb %1, %3, %4, %1;\n"
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"(w));
  *c_Re = tmp1;
  *c_Im = tmp2;
#endif
}
#endif /* FUNCTION_cplxMultDiv2_32x16 */

#ifdef __ARM_ARCH_8__
#ifdef FUNCTION_cplxMult_32x32X2
inline void cplxMult(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                     const FIXP_DBL b_Re, const FIXP_DBL b_Im) {
  INT64 tmp1, tmp2;
  asm("smull  %x0, %w2, %w4; \n"      /* tmp1  = a_Re * b_Re */
      "smull  %x1, %w2, %w5; \n"      /* tmp2  = a_Re * b_Im */
      "smsubl %x0, %w3, %w5, %x0; \n" /* tmp1 -= a_Im * b_Im */
      "smaddl %x1, %w3, %w4, %x1; \n" /* tmp2 += a_Im * b_Re */
      "asr    %x0, %x0, #31; \n"
      "asr    %x1, %x1, #31; \n"
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"((LONG)b_Re), "r"((LONG)b_Im));
  *c_Re = (LONG)tmp1;
  *c_Im = (LONG)tmp2;
}
#endif /* FUNCTION_cplxMult_32x32X2 */

#ifdef FUNCTION_cplxMult_32x16X2
inline void cplxMult(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re, const FIXP_DBL a_Im,
                     const FIXP_SGL b_Re, const FIXP_SGL b_Im) {
  INT64 tmp1, tmp2;
  asm("smull  %x0, %w2, %w4; \n"      /* tmp1  = a_Re * b_Re */
      "smull  %x1, %w2, %w5; \n"      /* tmp2  = a_Re * b_Im */
      "smsubl %x0, %w3, %w5, %x0; \n" /* tmp1 -= a_Im * b_Im */
      "smaddl %x1, %w3, %w4, %x1; \n" /* tmp2 += a_Im * b_Re */
      "asr    %x0, %x0, #15; \n"
      "asr    %x1, %x1, #15; \n"
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"((LONG)b_Re), "r"((LONG)b_Im));
  *c_Re = (LONG)tmp1;
  *c_Im = (LONG)tmp2;
}
#endif /* FUNCTION_cplxMult_32x16X2 */

#ifdef FUNCTION_cplxMultAddDiv2_32x32X2
inline void cplxMultAddDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re,
                            const FIXP_DBL a_Im, const FIXP_DBL b_Re, const FIXP_DBL b_Im) {
  INT64 tmp1, tmp2;
  asm("smull  %x0, %w2, %w4; \n"          /* tmp1  = a_Re * b_Re */
      "smull  %x1, %w2, %w5; \n"          /* tmp2  = a_Re * b_Im */
      "smsubl %x0, %w3, %w5, %x0; \n"     /* tmp1 -= a_Im * b_Im */
      "smaddl %x1, %w3, %w4, %x1; \n"     /* tmp2 += a_Im * b_Re */
      "add    %x0, %x6, %x0, asr #32; \n" /* tmp1 += *c_Re */
      "add    %x1, %x7, %x1, asr #32; \n" /* tmp2 += *c_Im */
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"(b_Re), "r"(b_Im), "r"((INT64)*c_Re), "r"((INT64)*c_Im));
  *c_Re = (LONG)tmp1;
  *c_Im = (LONG)tmp2;
}
#endif /* FUNCTION_cplxMultAddDiv2_32x32X2 */

#ifdef FUNCTION_cplxMultAddDiv2_32x16X2
inline void cplxMultAddDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re,
                            const FIXP_DBL a_Im, const FIXP_SGL b_Re, const FIXP_SGL b_Im) {
  INT64 tmp1, tmp2;
  asm("smull  %x0, %w2, %w4; \n"          /* tmp1  = a_Re * b_Re */
      "smull  %x1, %w2, %w5; \n"          /* tmp2  = a_Re * b_Im */
      "smsubl %x0, %w3, %w5, %x0; \n"     /* tmp1 -= a_Im * b_Im */
      "smaddl %x1, %w3, %w4, %x1; \n"     /* tmp2 += a_Im * b_Re */
      "add    %x0, %x6, %x0, asr #16; \n" /* tmp1 += *c_Re */
      "add    %x1, %x7, %x1, asr #16; \n" /* tmp2 += *c_Im */
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"((LONG)b_Re), "r"((LONG)b_Im), "r"((INT64)*c_Re),
        "r"((INT64)*c_Im));
  *c_Re = (LONG)tmp1;
  *c_Im = (LONG)tmp2;
}
#endif /* FUNCTION_cplxMultAddDiv2_32x16X2 */

#ifdef FUNCTION_cplxMultSubDiv2_32x32X2
inline void cplxMultSubDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re,
                            const FIXP_DBL a_Im, const FIXP_DBL b_Re, const FIXP_DBL b_Im) {
  INT64 tmp1, tmp2;
  asm("smull  %x0, %w2, %w4; \n"          /* tmp1  = a_Re * b_Re */
      "smull  %x1, %w2, %w5; \n"          /* tmp2  = a_Re * b_Im */
      "smsubl %x0, %w3, %w5, %x0; \n"     /* tmp1 -= a_Im * b_Im */
      "smaddl %x1, %w3, %w4, %x1; \n"     /* tmp2 += a_Im * b_Re */
      "sub    %x0, %x6, %x0, asr #32; \n" /* tmp1 -= *c_Re */
      "sub    %x1, %x7, %x1, asr #32; \n" /* tmp2 -= *c_Im */
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"(b_Re), "r"(b_Im), "r"((INT64)*c_Re), "r"((INT64)*c_Im));
  *c_Re = (LONG)tmp1;
  *c_Im = (LONG)tmp2;
}
#endif /* FUNCTION_cplxMultSubDiv2_32x32X2 */

#ifdef FUNCTION_cplxMultSubDiv2_32x16X2
inline void cplxMultSubDiv2(FIXP_DBL* c_Re, FIXP_DBL* c_Im, const FIXP_DBL a_Re,
                            const FIXP_DBL a_Im, const FIXP_SGL b_Re, const FIXP_SGL b_Im) {
  INT64 tmp1, tmp2;
  asm("smull  %x0, %w2, %w4; \n"          /* tmp1  = a_Re * b_Re */
      "smull  %x1, %w2, %w5; \n"          /* tmp2  = a_Re * b_Im */
      "smsubl %x0, %w3, %w5, %x0; \n"     /* tmp1 -= a_Im * b_Im */
      "smaddl %x1, %w3, %w4, %x1; \n"     /* tmp2 += a_Im * b_Re */
      "sub    %x0, %x6, %x0, asr #16; \n" /* tmp1 -= *c_Re */
      "sub    %x1, %x7, %x1, asr #16; \n" /* tmp2 -= *c_Im */
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"((LONG)b_Re), "r"((INT)b_Im), "r"((INT64)*c_Re),
        "r"((INT64)*c_Im));
  *c_Re = (LONG)tmp1;
  *c_Im = (LONG)tmp2;
}
#endif /* FUNCTION_cplxMultSubDiv2_32x16X2 */
#endif /* __ARM_ARCH_8__ */

#endif

#endif /* !defined(CPLX_MUL_ARM_H) */
