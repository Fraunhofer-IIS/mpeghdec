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

#if !defined(FIXMUL_ARM_H)
#define FIXMUL_ARM_H

#if defined(__arm__)

#if defined(__GNUC__) && defined(__arm__)
/* ARM with GNU compiler */

#define FUNCTION_fixmuldiv2_DD

#define FUNCTION_fixmuldiv2BitExact_DD
#ifdef FUNCTION_fixmuldiv2BitExact_DD
#define fixmuldiv2BitExact_DD(a, b) fixmuldiv2_DD(a, b)
#endif /* #ifdef FUNCTION_fixmuldiv2BitExact_DD */

#define FUNCTION_fixmulBitExact_DD
#ifdef FUNCTION_fixmulBitExact_DD
#define fixmulBitExact_DD(a, b) (fixmuldiv2BitExact_DD(a, b) << 1)
#endif /* #ifdef FUNCTION_fixmulBitExact_DD */

#define FUNCTION_fixmuldiv2BitExact_DS
#ifdef FUNCTION_fixmuldiv2BitExact_DS
#define fixmuldiv2BitExact_DS(a, b) fixmuldiv2_DS(a, b)
#endif /* #ifdef FUNCTION_fixmuldiv2BitExact_DS */

#define FUNCTION_fixmulBitExact_DS
#ifdef FUNCTION_fixmulBitExact_DS
#define fixmulBitExact_DS(a, b) fixmul_DS(a, b)
#endif /* #ifdef FUNCTION_fixmulBitExact_DS */

#ifdef FUNCTION_fixmuldiv2_DD
inline INT fixmuldiv2_DD(const INT a, const INT b) {
#if defined(__ARM_ARCH_8__)
  INT64 result;
  __asm__(
      "smull %x0, %w1, %w2;\n"
      "asr %x0, %x0, #32;    "
      : "=r"(result)
      : "r"(a), "r"(b));
  return (INT)result;
#elif defined(__ARM_ARCH_6__) || defined(__TARGET_ARCH_7E_M)
  INT result;
  __asm__("smmul %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
  return result;
#else
  INT result, discard;
  __asm__("smull %0, %1, %2, %3" : "=&r"(discard), "=r"(result) : "r"(a), "r"(b));
  return result;
#endif
}
#endif /* #ifdef FUNCTION_fixmuldiv2_DD */

#if defined(__ARM_ARCH_8__)
#define FUNCTION_fixmuldiv2_SD
#ifdef FUNCTION_fixmuldiv2_SD
inline INT fixmuldiv2_SD(const SHORT a, const INT b) {
  INT64 result;
  __asm__(
      "smull %x0, %w1, %w2;\n"
      "asr %x0, %x0, #16;    "
      : "=r"(result)
      : "r"((INT)a), "r"(b));
  return (INT)result;
}
#endif /* #ifdef FUNCTION_fixmuldiv2_SD */
#elif defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_6__)
#define FUNCTION_fixmuldiv2_SD
#ifdef FUNCTION_fixmuldiv2_SD
inline INT fixmuldiv2_SD(const SHORT a, const INT b) {
  INT result;
  __asm__("smulwb %0, %1, %2" : "=r"(result) : "r"(b), "r"(a));
  return result;
}
#endif /* #ifdef FUNCTION_fixmuldiv2_SD */
#endif

#define FUNCTION_fixmul_DD
#ifdef FUNCTION_fixmul_DD
#if defined(__ARM_ARCH_8__)
inline INT fixmul_DD(const INT a, const INT b) {
  INT64 result;
  __asm__(
      "smull %x0, %w1, %w2;\n"
      "asr %x0, %x0, #31;    "
      : "=r"(result)
      : "r"(a), "r"(b));
  return (INT)result;
}
#else
inline INT fixmul_DD(const INT a, const INT b) {
  return (fixmuldiv2_DD(a, b) << 1);
}
#endif /* __ARM_ARCH_8__ */
#endif /* #ifdef FUNCTION_fixmul_DD */

#if defined(__ARM_ARCH_8__)
#define FUNCTION_fixmul_SD
#ifdef FUNCTION_fixmul_SD
inline INT fixmul_SD(const SHORT a, const INT b) {
  INT64 result;
  __asm__(
      "smull %x0, %w1, %w2;\n"
      "asr %x0, %x0, #15;    "
      : "=r"(result)
      : "r"((INT)a), "r"(b));
  return (INT)result;
}
#endif /* #ifdef FUNCTION_fixmul_SD */
#endif /* __ARM_ARCH_8__ */

#if defined(__ARM_ARCH_8__)
#define FUNCTION_fixpow2_D
#ifdef FUNCTION_fixpow2_D
inline LONG fixpow2_D(const LONG a) {
  INT64 result;
  __asm__(
      "smull %x0, %w1, %w1;\n"
      "asr %x0, %x0, #31;    "
      : "=r"(result)
      : "r"(a));
  return (INT)result;
}
#endif /* #ifdef FUNCTION_fixpow2_D */
#endif /* __ARM_ARCH_8__ */

#if defined(__ARM_ARCH_6__)
#define FUNCTION_fixmuldiv2_QQ
#ifdef FUNCTION_fixmuldiv2_QQ
/**
 * \brief        This function performs a 64 x 64 bit multiplication divided by 2
 *
 * \description: INT64 a_x_b_hi  = a_hi * b_hi;
 *               INT64 a_x_b_mid = (a_hi * b_lo) >> DFRACT_BITS;
 *               INT64 b_x_a_mid = (b_hi * a_lo) >> DFRACT_BITS;
 *               INT64 result    = a_x_b_hi + a_x_b_mid + b_x_a_mid;
 *
 * \param        INT64 a [i] 64 bit input value
 * \param        INT64 b [i] 64 bit input value
 *
 * \return       Product of 64 x 64 bit multiplication divided by 2
 *
 *               ASM interface:
 *               - input:  %2 = a_hi
 *                         %3 = a_lo
 *                         %4 = b_hi
 *                         %5 = b_lo
 *
 *               - return: %0 = result_lo
 *                         %1 = result_hi
 */
inline FIXP_QDL fixmuldiv2_QQ(const INT64 a, const INT64 b) {
  union {
    struct {
      LONG lo;
      LONG hi;
    } v;
    INT64 w;
  } result;

  __asm__(
      "umull  r4, r5, %2, %5 \n" /* a_hi * b_lo => store result in r5:r4 (low part in r4 is not
                                    used) */
      "asr    r4, %2, #31    \n" /* extract sign of a_hi => r4 */
      "mla    r5, r4, %5, r5 \n" /* a_hi * b_lo + sign extension(a_hi) * b_lo = a_x_b_mid => r5 */
      "umull  r4, r2, %4, %3 \n" /* b_hi * a_lo => store result in r2:r4 (low part in r4 is not
                                    used) */
      "asr    r4, %4, #31    \n" /* extract sign of b_hi => r4 */
      "mla    r2, %3, r4, r2 \n" /* b_hi * a_lo + sign extension(b_hi) * a_lo = b_x_a_mid => r2 */
      "asr    r4, r5, #31    \n" /* extract sign of a_x_b_mid => r4 */
      "smlal  r5, r4, %2, %4 \n" /* a_x_b_mid + a_hi * b_hi */
      "asr    %1, r2, #31    \n" /* extract sign of b_x_a_mid => r1 */
      "adds   %0, r2, r5     \n" /* a_hi * b_hi + a_x_b_mid + b_x_a_mid */
      "adc    %1, %1, r4     \n" /* a_hi * b_hi + a_x_b_mid + b_x_a_mid */
      : "=r"(result.v.lo), "=r"(result.v.hi)
      : "r"((LONG)(a >> DFRACT_BITS)), "r"((ULONG)a), "r"((LONG)(b >> DFRACT_BITS)), "r"((ULONG)b)
      : "cc", "r2", "r4", "r5");

  return result.w;
}
#endif

#define FUNCTION_fixmul_QQ
#ifdef FUNCTION_fixmul_QQ
inline FIXP_QDL fixmul_QQ(const FIXP_QDL a, const FIXP_QDL b) {
  { return fixmuldiv2_QQ(a, b) << 1; }
}
#endif

#define FUNCTION_fixmuldiv2_QD
#ifdef FUNCTION_fixmuldiv2_QD
/**
 * \brief        This function performs a 64 x 32 bit multiplication divided by 2
 *
 * \description: INT64 a_x_b_hi  = a_hi * b;
 *               INT64 b_x_a_mid = (b * a_lo) >> DFRACT_BITS;
 *               INT64 result    = a_x_b_hi + b_x_a_mid;
 *
 * \param        INT64 a [i] 64 bit input value
 * \param        LONG  b [i] 32 bit input value
 *
 * \return       Product of 64 x 32 bit multiplication divided by 2
 *
 *               ASM interface:
 *               - input:  %2 = a_hi
 *                         %3 = a_lo
 *                         %4 = b
 *
 *               - return: %0 = result_lo
 *                         %1 = result_hi
 */
inline FIXP_QDL fixmuldiv2_QD(INT64 a, LONG b) {
  union {
    struct {
      LONG lo;
      LONG hi;
    } v;
    INT64 w;
  } result;

  __asm__(
      "umull  r5, r4, %4, %3 \n" /* b * a_lo => store result in r4:r5 (low part in r5 is not used)
                                  */
      "asr    r5, %4, #31    \n" /* extract sign of b => r5 */
      "mla    r6, %3, r5, r4 \n" /* b * a_lo + sign extension(b) * a_lo = b_x_a_mid => r6 */
      "asr    r5, r6, #31    \n" /* extract sign of b_x_a_mid => r5 */
      "smlal  r6, r5, %4, %2 \n" /* b_x_a_mid + b * a_hi */
      "mov    %0, r6         \n" /* result_lo => r0 */
      "mov    %1, r5         \n" /* result_hi => r1 */
      : "=r"(result.v.lo), "=r"(result.v.hi)
      : "r"((LONG)(a >> DFRACT_BITS)), "r"((ULONG)a), "r"(b)
      : "cc", "r4", "r5", "r6");

  return result.w;
}
#endif

#define FUNCTION_fixmul_QD
#ifdef FUNCTION_fixmul_QD
/**
 * \brief        This function performs a 64 x 32 bit multiplication
 *
 * \description: INT64 a_x_b_hi  = a_hi * b;
 *               INT64 b_x_a_mid = (b * a_lo) >> DFRACT_BITS;
 *               INT64 result    = (a_x_b_hi + b_x_a_mid) << 1;
 *
 * \param        INT64 a [i] 64 bit input value
 * \param        LONG  b [i] 32 bit input value
 *
 * \return       Product of 64 x 32 bit multiplication
 *
 *               ASM interface:
 *               - input:  %2 = a_hi
 *                         %3 = a_lo
 *                         %4 = b
 *
 *               - return: %0 = result_lo
 *                         %1 = result_hi
 */
inline FIXP_QDL fixmul_QD(const FIXP_QDL a, const LONG b) {
  union {
    struct {
      LONG lo;
      LONG hi;
    } v;
    INT64 w;
  } result;

  __asm__(
      "umull  r5, r4, %4, %3 \n" /* b * a_lo => store result in r4:r5 (low part in r5 is not used)
                                  */
      "asr    r5, %4, #31    \n" /* extract sign of b => r5 */
      "mla    r6, %3, r5, r4 \n" /* b * a_lo + sign extension(b) * a_lo = b_x_a_mid => r6 */
      "asr    r5, r6, #31    \n" /* extract sign of b_x_a_mid => r5 */
      "smlal  r6, r5, %4, %2 \n" /* b_x_a_mid + b * a_hi */
      "adds   %0, r6, r6     \n" /* result << 1 */
      "adc    %1, r5, r5     \n" /* result << 1 */
      : "=r"(result.v.lo), "=r"(result.v.hi)
      : "r"((LONG)(a >> DFRACT_BITS)), "r"((ULONG)a), "r"(b)
      : "cc", "r4", "r5", "r6");

  return result.w;
}
#endif
#endif /* __ARM_ARCH_6__ */

#if defined(__ARM_ARCH_8__)
#define FUNCTION_fixmuldiv2_QQ
#ifdef FUNCTION_fixmuldiv2_QQ
inline FIXP_QDL fixmuldiv2_QQ(const FIXP_QDL a, const FIXP_QDL b) {
  INT64 result;
  __asm__("smulh %x0, %x1, %x2;\n" : "=r"(result) : "r"(a), "r"(b));
  return (FIXP_QDL)result;
}
#endif

#define FUNCTION_fixmul_QQ
#ifdef FUNCTION_fixmul_QQ
inline FIXP_QDL fixmul_QQ(const FIXP_QDL a, const FIXP_QDL b) {
  INT64 result;
  __asm__(
      "smulh %x0, %x1, %x2;\n"
      "lsl %x0, %x0, #1;     "
      : "=r"(result)
      : "r"(a), "r"(b));
  return (FIXP_QDL)result;
}
#endif

#define FUNCTION_fixmuldiv2_QD
#ifdef FUNCTION_fixmuldiv2_QD
inline FIXP_QDL fixmuldiv2_QD(const FIXP_QDL a, const LONG b) {
  { return fixmuldiv2_QQ(a, (FIXP_QDL)b << DFRACT_BITS); }
}
#endif

#define FUNCTION_fixmul_QD
#ifdef FUNCTION_fixmul_QD
inline FIXP_QDL fixmul_QD(const FIXP_QDL a, const LONG b) {
  { return fixmul_QQ(a, (FIXP_QDL)b << DFRACT_BITS); }
}
#endif
#endif /* __ARM_ARCH_8__ */

#endif /* defined(__GNUC__) && defined(__arm__) */

#endif /* __arm__ */

#endif /* !defined(FIXMUL_ARM_H) */
