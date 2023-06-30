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

   Author(s):   Arthur Tritthart, Jack Olivieri

   Description: NEON implementations of fixedpoint functions

*******************************************************************************/

/* clang-format off */
#include "arm/FDK_arm_funcs.h"
#if defined (__ARM_NEON__)
#include "arm/FDK_neon_funcs.h"
#endif

/* 16-NOV-2009 A. Tritthart
   - added dedicated division with 16-bit precision, often used in HE-AAC (e.g. deCorrelateSlotBased)
     This procedure is bit-exact to:  FIXP_DBL div = schur_div (FIXP_DBL num, FIXP_DBL denum, 16);
     Preconditions:
     a) denum >= num
     b) denum > 0
     c) num >= 0,
     d) count = 16

     Note: In previous version, 1 divided by 2 resulted in 0x0000.0000 (instead of 0x4000.0000) due to initial
           shift right operation of num
           This version now skips the shift-right operation at the beginning, the result is corrected.
*/

#ifndef FUNCTION_schur_div
/* Cortex-M3 and Cortex-M4 are unsupported.
   All compilers except rcvt and gcc are unsupported. */
#if (!defined(__ARM_ARCH_7M__) && !defined(__ARM_ARCH_7EM__)) && \
    (defined(__GNUC__) && defined(__arm__))
#define FUNCTION_schur_div
#endif

#ifdef FUNCTION_schur_div
#ifdef __ARM_ARCH_8__
#define USE_ARMV8_64BIT_DIVISION
/* for every instruction, refer to ARM DDI 0487A.g (Reference Manual for ARMv8)
   http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0406c/index.html
   AArch64 SIMD (NEON) arithmetics are IEEE 754-2008 compliant, says above doc */
#if defined(__GNUC__) && defined(__arm__)
FIXP_DBL schur_div(FIXP_DBL num, FIXP_DBL denum, const INT count)
{
  FIXP_DBL result;
  __asm__ (
#ifdef USE_ARMV8_64BIT_DIVISION
           "scvtf d0, %w1, #31 \n\t"    /* C7.3.207 convert fixed-p Q1.31 num to double   */
           "scvtf d1, %w2, #31 \n\t"    /* C7.3.207 convert fixed-p Q1.31 denum to double */
           "fdiv d0, d0, d1 \n\t"       /* C7.3.85 double division                        */
           "fcvtzs %w0, d0, #31 \n\t"   /* C7.3.77 convert double result to fixed-p Q1.31 */
#else /* USE_ARMV8_64BIT_DIVISION */
           "scvtf s0, %w1, #31 \n\t"    /* C7.3.207 convert fixed-p Q1.31 num to float   */
           "scvtf s1, %w2, #31 \n\t"    /* C7.3.207 convert fixed-p Q1.31 denum to float */
           "fdiv s0, s0, s1 \n\t"       /* C7.3.85 float division                        */
           "fcvtzs %w0, s0, #31 \n\t"   /* C7.3.77 convert float result to fixed-p Q1.31 */
#endif /* USE_ARMV8_64BIT_DIVISION */
           : "=r" (result)
           :"r" (num), "r" (denum));
  return result;
}
#endif  /* __CC_ARM */
#elif defined(__ARM_NEON__) /* __ARM_ARCH_8__ */
#if defined(__GNUC__) && defined(__arm__)
FIXP_DBL schur_div(FIXP_DBL num, FIXP_DBL denum, const INT count)
{
  FIXP_DBL result;
  __asm__ (
           "VMOV d0, %1, %2 \n\t"
           "VCVT.F32.S32 d0, d0, #31 \n\t"
           "VDIV.F32 s0, s0, s1 \n\t"
           "VCVT.S32.F32 s0, s0, #31 \n\t"
           "VMOV %0, s0 \n\t"
           : "=r" (result)
           :"r" (num), "r" (denum));
  return result;
}
#endif /* __CC_ARM */
#else /* __ARM_NEON__ */
#if defined(__GNUC__) && defined(__arm__)
/* GCC, clang version */
FIXP_DBL schur_div(FIXP_DBL num, FIXP_DBL denum, const INT count)
{
    INT div     = 0;                         /* bit #15 is always 0 */
    ULONG L_num   = (ULONG) num << 1;
    ULONG L_denum = (ULONG) denum;

    __asm__ (
             "CMP      %1, %2\t\n"                   /* generate C carry flag, if (unsigned) L_num greater/equal L_denum */
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #14: div = div+div+carry = div<<1 + carry */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #13 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #12 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #11 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #10 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #9 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #8 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #7 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #6 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #5 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #4 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #3 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #2 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "SUBHS    %1, %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #1 */

             "MOV      %1, %1, LSL #1\t\n"
             "CMP      %1, %2\t\n"
             "ADC      %0, %0, %0\t\n"               /* bit #0 */

             :"+r" (div)
             :"r" (L_num), "r" (L_denum)
             :"cc");

    return (FIXP_DBL)(div << (DFRACT_BITS - 16));
}
#endif /* __CC_ARM */
#endif /* __ARM_NEON__ */
#endif /* ifdef FUNCTION_schur_div */
#endif /* ifndef FUNCTION_schur_div */


#ifdef __ARM_NEON__
//#define FUNCTION_FDK_vsqrt
#endif

#if defined(FUNCTION_FDK_vsqrt)
/* Square root vector function for fixed-point data in range 0x0000.0000 to 0x7FFF.FFFF */
/* For other input values, the result is invalid.                                       */
/* The length must be a multiple of 4 and passed to the function divided by 4           */
FDK_ASM_ROUTINE_START(void, FDK_vsqrt,(FIXP_DBL *r0, INT r1))
                  /* r0: pointer to array             */
                  /* r1: length of array divided by 4 */
FDK_label(FDK_vsqrt_loop)
    FDK_vldm1(128,Q0,r0)
    FDK_vcvt_f32_s32_q(Q0,Q0,31)
    FDK_vsqrt_f32(s0,s0)
    FDK_vsqrt_f32(s1,s1)
    FDK_vsqrt_f32(s2,s2)
    FDK_vsqrt_f32(s3,s3)
    FDK_vcvt_s32_f32_q(Q0,Q0,31)
    FDK_vstm1_ia(128,Q0,r0)
    FDK_subs_imm (r1,r1,1)
  FDK_branch(NE, FDK_vsqrt_loop)
    FDK_return()
FDK_ASM_ROUTINE_END()
#endif /* FUNCTION_FDK_vsqrt */



#if defined(POW2COEFF_16BIT) && defined(SINETABLE_16BIT) && defined(__ARM_ARCH_5TE__)
#if defined(__GNUC__) && defined(__arm__)
#define FUNCTION_f2Pow_func1
#endif /* defined(__CC_ARM) || (defined(__GNUC__) && defined(__arm__)) */
#endif
#if defined (FUNCTION_f2Pow_func1)

#if defined(__GNUC__) && defined(__arm__)
/* GCC, clang version */
inline INT FXM_SMLAWB(const LONG accu, const LONG a, const LONG b)
{
  INT result ;
  __asm__ ("smlawb %0, %1, %2,%3"
     : "=r" (result)
     : "r" (a), "r" (b), "r" (accu)) ;
  return result;
}
inline INT FXM_SMLAWT(const LONG accu, const LONG a, const LONG b)
{
  INT result ;
  __asm__ ("smlawt %0, %1, %2,%3"
     : "=r" (result)
     : "r" (a), "r" (b), "r" (accu)) ;
  return result;
}
#endif /* defined(__CC_ARM) */

#if defined(__GNUC__) && defined(__arm__)
FIXP_DBL f2Pow_func1(FIXP_DBL p0, const LONG * RESTRICT p_coeffs)
{
    FIXP_DBL p1, p2;
    FIXP_DBL result_m = FL2FXCONST_DBL(1.0f/2.0f);

    LONG coeff;   /* contains of a pair of coefficients */

    coeff = p_coeffs[0];           // coeff.B: pow2Coeff[0]  coeff.T: pow2Coeff[1]
    p1 = fMult(p0, p0);            // p1: p0^2
    result_m = FXM_SMLAWB(result_m, p0, coeff);  // p0: p0^1
    p2 = fMult(p0, p1);            // p2: p0^3
    result_m = FXM_SMLAWT(result_m, p1, coeff);  // p1: p0^2
    coeff = p_coeffs[1];           // coeff.B: pow2Coeff[2]  coeff.T: pow2Coeff[3]
    p0 = fMult(p1,p1);             // p0: p0^4
    result_m = FXM_SMLAWB(result_m, p2, coeff);  // p2: p0^3
    p1 = fMult(p1, p2);            // p1: p0^5
    result_m = FXM_SMLAWT(result_m, p0, coeff);  // p0: p0^4
    coeff = p_coeffs[2];           // coeff.B: pow2Coeff[4]  coeff.T: pow2Coeff[5]
    result_m = FXM_SMLAWB(result_m, p1, coeff);  // p1: p0^5
#if (POW2_PRECISION == 8)
    p1 = fMult(p0, p0);            // p1: p0^8
    p0 = fMult(p0, p2);            // p0: p0^7
    p2 = fMult(p2, p2);            // p2: p0^6
    result_m = FXM_SMLAWT(result_m, p2, coeff);  // p2: p0^6
    coeff = p_coeffs[3];           // coeff.B: pow2Coeff[6]  coeff.T: pow2Coeff[7]
    result_m = FXM_SMLAWB(result_m, p0, coeff);  // p0: p0^7
    result_m = FXM_SMLAWT(result_m, p1, coeff);  // p2: p0^8
#endif
    return result_m;
}
#endif /* defined(__CC_ARM) || (defined(__GNUC__) && defined(__arm__)) */
#endif /* #if defined (FUNCTION_f2Pow_func1) */

