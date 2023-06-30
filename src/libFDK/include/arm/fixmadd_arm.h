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

#if !defined(FIXMADD_ARM_H)
#define FIXMADD_ARM_H

#if defined(__arm__)

/* ############################################################################# */
#if defined(__GNUC__) && defined(__arm__)
/* ############################################################################# */
/* ARM GNU GCC */

#ifdef __ARM_ARCH_8__

#define FUNCTION_fixmadddiv2_DD
#ifdef FUNCTION_fixmadddiv2_DD
inline FIXP_DBL fixmadddiv2_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  INT64 result;
  asm("smull  %x0, %w1, %w2; \n"
      "add    %x0, %x3, %x0, asr #32; \n"
      : "=&r"(result)
      : "r"(a), "r"(b), "r"((INT64)x));
  return (INT)result;
}
#endif /* #ifdef FUNCTION_fixmadddiv2_DD */

#define FUNCTION_fixmadddiv2_SD
#ifdef FUNCTION_fixmadddiv2_SD
inline FIXP_DBL fixmadddiv2_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
  INT64 result;
  asm("smull  %x0, %w1, %w2; \n"
      "add    %x0, %x3, %x0, asr #16; \n"
      : "=&r"(result)
      : "r"((INT)a), "r"(b), "r"((INT64)x));
  return result;
}
#endif /* #ifdef FUNCTION_fixmadddiv2_SD */

#define FUNCTION_fixmadd_DD
#ifdef FUNCTION_fixmadd_DD
inline FIXP_DBL fixmadd_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  INT64 result;
  asm("smull  %x0, %w1, %w2; \n"
      "add    %x0, %x3, %x0, asr #31; \n"
      : "=&r"(result)
      : "r"(a), "r"(b), "r"((INT64)x << 1));
  return (INT)result;
}
#endif /* FUNCTION_fixmadd_DD */

#define FUNCTION_fixmadd_SD
#ifdef FUNCTION_fixmadd_SD
inline FIXP_DBL fixmadd_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
  INT64 result;
  asm("smull  %x0, %w1, %w2; \n"
      "add    %x0, %x3, %x0, asr #15; \n"
      : "=&r"(result)
      : "r"((INT)a), "r"(b), "r"((INT64)x << 1));
  return (INT)result;
}
#endif /* FUNCTION_fixmadd_SD */

#define FUNCTION_fixmsubdiv2_DD
#ifdef FUNCTION_fixmsubdiv2_DD
inline FIXP_DBL fixmsubdiv2_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  INT64 result;
  asm("smull  %x0, %w1, %w2; \n"
      "sub    %x0, %x3, %x0, asr #32; \n"
      : "=&r"(result)
      : "r"(a), "r"(b), "r"((INT64)x));
  return (INT)result;
}
#endif /* #ifdef FUNCTION_fixmsubdiv2_DD */

#define FUNCTION_fixmsubdiv2_SD
#ifdef FUNCTION_fixmsubdiv2_SD
inline FIXP_DBL fixmsubdiv2_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
  INT64 result;
  asm("smull  %x0, %w1, %w2; \n"
      "sub    %x0, %x3, %x0, asr #16; \n"
      : "=&r"(result)
      : "r"((INT)a), "r"(b), "r"((INT64)x));
  return (INT)result;
}
#endif /* #ifdef FUNCTION_fixmsubdiv2_SD */

#define FUNCTION_fixmsub_DD
#ifdef FUNCTION_fixmsub_DD
inline FIXP_DBL fixmsub_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  INT64 result;
  asm("smull  %x0, %w1, %w2; \n"
      "sub    %x0, %x3, %x0, asr #31; \n"
      : "=&r"(result)
      : "r"(a), "r"(b), "r"((INT64)x << 1));
  return (INT)result;
}
#endif /* FUNCTION_fixmsub_DD */

#define FUNCTION_fixmsub_SD
#ifdef FUNCTION_fixmsub_SD
inline FIXP_DBL fixmsub_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
  INT64 result;
  asm("smull  %x0, %w1, %w2; \n"
      "sub    %x0, %x3, %x0, asr #15; \n"
      : "=&r"(result)
      : "r"((INT)a), "r"(b), "r"(((INT64)x) << 1));
  return (INT)result;
}
#endif /* FUNCTION_fixmsub_SD */

#elif defined(__ARM_ARCH_6__)
#define FUNCTION_fixmadddiv2_DD
#ifdef FUNCTION_fixmadddiv2_DD
inline FIXP_DBL fixmadddiv2_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  INT result;
  asm("smmla %0, %1, %2, %3;\n" : "=r"(result) : "r"(a), "r"(b), "r"(x));
  return result;
}
#endif /* #ifdef FUNCTION_fixmadddiv2_DD */

#define FUNCTION_fixmsubdiv2_DD
#ifdef FUNCTION_fixmsubdiv2_DD
inline FIXP_DBL fixmsubdiv2_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  INT result;
  asm("smmls %0, %1, %2, %3;\n" : "=r"(result) : "r"(a), "r"(b), "r"(x));
  return result;
}
#endif /* #ifdef FUNCTION_fixmsubdiv2_DD */

#else /* __ARM_ARCH_6__ */
#define FUNCTION_fixmadddiv2_DD
#ifdef FUNCTION_fixmadddiv2_DD
inline FIXP_DBL fixmadddiv2_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  INT discard = 0;
  INT result = x;
  asm("smlal %0, %1, %2, %3;\n" : "+r"(discard), "+r"(result) : "r"(a), "r"(b));
  return result;
}
#endif /* #ifdef FUNCTION_fixmadddiv2_DD */
#endif /* __ARM_ARCH_6__ */

#if defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_6__)

#define FUNCTION_fixmadddiv2_DS
#ifdef FUNCTION_fixmadddiv2_DS
inline FIXP_DBL fixmadddiv2_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
  INT result;
  asm("smlawb %0, %1, %2, %3 " : "=r"(result) : "r"(a), "r"(b), "r"(x));
  return result;
}
#endif /* #ifdef FUNCTION_fixmadddiv2_DS */

#endif /* defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_6__) */

#define FUNCTION_fixmadddiv2BitExact_DD
#ifdef FUNCTION_fixmadddiv2BitExact_DD
#define fixmadddiv2BitExact_DD(a, b, c) fixmadddiv2_DD(a, b, c)
#endif /* #ifdef FUNCTION_fixmadddiv2BitExact_DD */

#define FUNCTION_fixmsubdiv2BitExact_DD
#ifdef FUNCTION_fixmsubdiv2BitExact_DD
inline FIXP_DBL fixmsubdiv2BitExact_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return x - fixmuldiv2BitExact_DD(a, b);
}
#endif /* #ifdef FUNCTION_fixmsubdiv2BitExact_DD */

#define FUNCTION_fixmadddiv2BitExact_DS
#ifdef FUNCTION_fixmadddiv2BitExact_DS
#define fixmadddiv2BitExact_DS(a, b, c) fixmadddiv2_DS(a, b, c)
#endif /* #ifdef FUNCTION_fixmadddiv2BitExact_DS */

#define FUNCTION_fixmsubdiv2BitExact_DS
#ifdef FUNCTION_fixmsubdiv2BitExact_DS
inline FIXP_DBL fixmsubdiv2BitExact_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
  return x - fixmuldiv2BitExact_DS(a, b);
}
#endif /* #ifdef FUNCTION_fixmsubdiv2BitExact_DS */

/* ############################################################################# */
#endif /* toolchain */
/* ############################################################################# */

#endif /* __arm__ */

#endif /* !defined(FIXMADD_ARM_H) */
