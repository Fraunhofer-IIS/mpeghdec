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

   Author(s):   Arthur Tritthart

   Description: Scaling operations for ARM

*******************************************************************************/

/* clang-format off */
/* prevent multiple inclusion with re-definitions */
#ifndef __INCLUDE_SCALE_ARM__
#define __INCLUDE_SCALE_ARM__

#if defined(__GNUC__)
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wreturn-type"
//#pragma GCC diagnostic ignored "-Wattributes"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif /* #if defined(__GNUC__) */

#if defined(__ARM_NEON__)
#include "scale_arm_neon.cpp"
#endif

#if defined(__ARM_ARCH_5TE__)
#include "arm/FDK_arm_funcs.h"

#if !defined(FUNCTION_scaleValues_DBL)
#if defined(__GNUC__)
#define FUNCTION_scaleValues_DBL
/***************************************************/
/******** scaleValues, in/out vector identical *****/
/***************************************************/
FDK_ASM_ROUTINE_START(void,scaleValuesDBL,(
        FIXP_DBL *vector,
        INT len,
        INT scalefactor
        ))

  // save upper core registers according to call convention
  FDK_mpush(r4,r7)

  /* stack contents:
       0x0C:     r7
       0x08:     r6
       0x04:     r5
       0x00:     r4

     register contents:
       r0: vector
       r1: len
       r2: scalefactor
       r3: vector pointer copy for store

     register usage:
       r0: pointer to input
       r1: input vector length
       r2: scalefactor
       r3: vector pointer copy for store

       r4: tmp, then 1st output value
       r5: 2nd output value
       r6: 3rd output value
       r7: 4th output value

  */

  FDK_cmp_imm(r2, 0)
  FDK_mov_reg(r3, r0) // copy pointer for storing operation
  FDK_branch(LT, scaleValues_DBL_asr)

// left shift branch
  FDK_subs_imm(r4, r2, 31)      /* saturation */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_mov_cond_imm(GT, r2, 31)  /* saturation */
  FDK_subs_imm(r1, r1, 4)  /* subtract and check: if ... */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GE)
#endif
  FDK_ldrd_ia_cond(GE, r4, r5, r0, 8)
  FDK_branch(LT, scaleValues_DBL_lsl_loop_postroll)      /* ...len<4, goto postroll  */

FDK_label(scaleValues_DBL_lsl_loop_4x)
  // scale 4 samples
  FDK_lsl(r4, r4, r2)
  FDK_ldrd_ia(r6, r7, r0, 8)
  FDK_lsl(r5, r5, r2)
  FDK_subs_imm(r1, r1, 4) /* length-=4 */
  FDK_lsl(r6, r6, r2)
  FDK_strd_ia(r4, r5, r3, 8) /* store 2 values */
  FDK_lsl(r7, r7, r2)
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GE)
#endif
  FDK_ldrd_ia_cond(GE, r4, r5, r0, 8)
  FDK_strd_ia(r6, r7, r3, 8) /* store 2 values */
  FDK_branch(GE, scaleValues_DBL_lsl_loop_4x)

FDK_label(scaleValues_DBL_lsl_loop_postroll)
  FDK_adds_imm(r1, r1, 4)    /* add 4 that has been subtracted before */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r4, r0, 4)
  FDK_branch(LE, scaleValues_DBL_end)         /* if len<=0 goto loopEnd */

  // scale 1 sample
  FDK_lsl(r4, r4, r2)
  FDK_subs_imm(r1, r1, 1) /* counter-- */
  FDK_str_ia(r4, r3, 4) /* store 1 value */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r5, r0, 4)
  FDK_branch(LE, scaleValues_DBL_end)

  // scale 1 sample
  FDK_lsl(r5, r5, r2)
  FDK_subs_imm(r1, r1, 1) /* counter-- */
  FDK_str_ia(r5, r3, 4) /* store 1 value */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r6, r0, 4)
  FDK_branch(LE, scaleValues_DBL_end)

  // scale 1 sample
  FDK_lsl(r6, r6, r2)
  FDK_str_ia(r6, r3, 4) /* store 1 value */

  // restore upper core registers according to call convention
  FDK_mpop(r4,r7)
  FDK_return()

// right shift branch
FDK_label(scaleValues_DBL_asr)
  FDK_rsb_imm(r2, r2, 0) /* invert the scale */
  FDK_subs_imm(r4, r2, 31)      /* saturation */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_mov_cond_imm(GT, r2, 31)  /* saturation */

  FDK_subs_imm(r1, r1, 4)  /* subtract and check: if ... */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GE)
#endif
  FDK_ldrd_ia_cond(GE, r4, r5, r0, 8)
  FDK_branch(LT, scaleValues_DBL_asr_loop_postroll)      /* ...len<4, goto postroll  */

FDK_label(scaleValues_DBL_asr_loop_4x)
  // scale 4 samples
  FDK_asr(r4, r4, r2)
  FDK_ldrd_ia(r6, r7, r0, 8)
  FDK_asr(r5, r5, r2)
  FDK_subs_imm(r1, r1, 4) /* length-=4 */
  FDK_asr(r6, r6, r2)
  FDK_strd_ia(r4, r5, r3, 8) /* store 2 values */
  FDK_asr(r7, r7, r2)
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GE)
#endif
  FDK_ldrd_ia_cond(GE, r4, r5, r0, 8)
  FDK_strd_ia(r6, r7, r3, 8) /* store 2 values */
  FDK_branch(GE, scaleValues_DBL_asr_loop_4x)

FDK_label(scaleValues_DBL_asr_loop_postroll)
  FDK_adds_imm(r1, r1, 4)    /* add 4 that has been subtracted before */

#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r4, r0, 4)
  FDK_branch(LE, scaleValues_DBL_end)         /* if len<=0 goto loopEnd */

  // scale 1 sample
  FDK_asr(r4, r4, r2)
  FDK_subs_imm(r1, r1, 1) /* counter-- */
  FDK_str_ia(r4, r3, 4) /* store 1 value */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r5, r0, 4)
  FDK_branch(LE, scaleValues_DBL_end)

  // scale 1 sample
  FDK_asr(r5, r5, r2)
  FDK_subs_imm(r1, r1, 1) /* counter-- */
  FDK_str_ia(r5, r3, 4) /* store 1 value */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r6, r0, 4)
  FDK_branch(LE, scaleValues_DBL_end)

  // scale 1 sample
  FDK_asr(r6, r6, r2)
  FDK_str_ia(r6, r3, 4) /* store 1 value */
FDK_label(scaleValues_DBL_end)

  // restore upper core registers according to call convention
  FDK_mpop(r4,r7)
  FDK_return()
FDK_ASM_ROUTINE_END()

void scaleValues (
        FIXP_DBL *vector,
        INT len,
        INT scalefactor
        )
{
  scaleValuesDBL(vector, len, scalefactor);
}

#endif /* defined(__CC_ARM) || defined(__GNUC__) */
#endif /* #if !defined(FUNCTION_scaleValues_DBL_DBL) */

#if !defined(FUNCTION_scaleValues_DBLDBL)
#if defined(__GNUC__)
#define FUNCTION_scaleValues_DBLDBL
/***************************************************/
/******** scaleValues, in/out vector different *****/
/***************************************************/
FDK_ASM_ROUTINE_START(void,scaleValuesDBLDBL,(
        FIXP_DBL *dst,
        const FIXP_DBL *src,
        INT len,
        INT scalefactor
        ))

  // save upper core registers according to call convention
  FDK_mpush(r4,r7)

  /* stack contents:
       0x0C:     r7
       0x08:     r6
       0x04:     r5
       0x00:     r4

     register contents:
       r3: scalefactor
       r2: len
       r1: vector pointer for load
       r0: vector pointer for store

     register usage:
       r0: vector pointer copy for store
       r1: pointer to input
       r2: input vector length
       r3: scalefactor

       r4: tmp, then 1st output value
       r5: 2nd output value
       r6: 3rd output value
       r7: 4th output value

  */

  FDK_cmp_imm(r3, 0)
  FDK_branch(LT, scaleValues_DBLDBL_asr)

// left shift branch
  FDK_subs_imm(r4, r3, 31)      /* saturation */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_mov_cond_imm(GT, r3, 31)  /* saturation */

  FDK_subs_imm(r2, r2, 4)  /* subtract and check: if ... */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GE)
#endif
  FDK_ldrd_ia_cond(GE, r4, r5, r1, 8)
  FDK_branch(LT, scaleValues_DBLDBL_lsl_loop_postroll)      /* ...len<4, goto postroll  */

FDK_label(scaleValues_DBLDBL_lsl_loop_4x)
  // scale 4 samples
  FDK_lsl(r4, r4, r3)
  FDK_ldrd_ia(r6, r7, r1, 8)
  FDK_lsl(r5, r5, r3)
  FDK_subs_imm(r2, r2, 4) /* length-=4 */
  FDK_lsl(r6, r6, r3)
  FDK_strd_ia(r4, r5, r0, 8) /* store 2 values */
  FDK_lsl(r7, r7, r3)
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GE)
#endif
  FDK_ldrd_ia_cond(GE, r4, r5, r1, 8)
  FDK_strd_ia(r6, r7, r0, 8) /* store 2 values */
  FDK_branch(GE, scaleValues_DBLDBL_lsl_loop_4x)

FDK_label(scaleValues_DBLDBL_lsl_loop_postroll)
  FDK_adds_imm(r2, r2, 4)    /* add 4 that has been subtracted before */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r4, r1, 4)
  FDK_branch(LE, scaleValues_DBLDBL_end)         /* if len<=0 goto loopEnd */

  // scale 1 sample
  FDK_lsl(r4, r4, r3)
  FDK_subs_imm(r2, r2, 1) /* counter-- */
  FDK_str_ia(r4, r0, 4) /* store 1 value */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r5, r1, 4)
  FDK_branch(LE, scaleValues_DBLDBL_end)

  // scale 1 sample
  FDK_lsl(r5, r5, r3)
  FDK_subs_imm(r2, r2, 1) /* counter-- */
  FDK_str_ia(r5, r0, 4) /* store 1 value */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r6, r1, 4)
  FDK_branch(LE, scaleValues_DBLDBL_end)

  // scale 1 sample
  FDK_lsl(r6, r6, r3)
  FDK_str_ia(r6, r0, 4) /* store 1 value */

  // restore upper core registers according to call convention
  FDK_mpop(r4,r7)
  FDK_return()

// right shift branch
FDK_label(scaleValues_DBLDBL_asr)
  FDK_rsb_imm(r3, r3, 0) /* invert the scale */
  FDK_subs_imm(r4, r3, 31)      /* saturation */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_mov_cond_imm(GT, r3, 31)  /* saturation */

  FDK_subs_imm(r2, r2, 4)  /* subtract and check: if ... */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GE)
#endif
  FDK_ldrd_ia_cond(GE, r4, r5, r1, 8)
  FDK_branch(LT, scaleValues_DBLDBL_asr_loop_postroll)      /* ...len<4, goto postroll  */

FDK_label(scaleValues_DBLDBL_asr_loop_4x)
  // scale 4 samples
  FDK_asr(r4, r4, r3)
  FDK_ldrd_ia(r6, r7, r1, 8)
  FDK_asr(r5, r5, r3)
  FDK_subs_imm(r2, r2, 4) /* length-=4 */
  FDK_asr(r6, r6, r3)
  FDK_strd_ia(r4, r5, r0, 8) /* store 2 values */
  FDK_asr(r7, r7, r3)
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GE)
#endif
  FDK_ldrd_ia_cond(GE, r4, r5, r1, 8)
  FDK_strd_ia(r6, r7, r0, 8) /* store 2 values */
  FDK_branch(GE, scaleValues_DBLDBL_asr_loop_4x)

FDK_label(scaleValues_DBLDBL_asr_loop_postroll)
  FDK_adds_imm(r2, r2, 4)    /* add 4 that has been subtracted before */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r4, r1, 4)
  FDK_branch(LE, scaleValues_DBLDBL_end)         /* if len<=0 goto loopEnd */

  // scale 1 sample
  FDK_asr(r4, r4, r3)
  FDK_subs_imm(r2, r2, 1) /* counter-- */
  FDK_str_ia(r4, r0, 4) /* store 1 value */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r5, r1, 4)
  FDK_branch(LE, scaleValues_DBLDBL_end)

  // scale 1 sample
  FDK_asr(r5, r5, r3)
  FDK_subs_imm(r2, r2, 1) /* counter-- */
  FDK_str_ia(r5, r0, 4) /* store 1 value */
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)  /* thumb: requires IT block */
  FDK_it(GT)
#endif
  FDK_ldr_ia_cond(GT, r6, r1, 4)
  FDK_branch(LE, scaleValues_DBLDBL_end)

  // scale 1 sample
  FDK_asr(r6, r6, r3)
  FDK_str_ia(r6, r0, 4) /* store 1 value */

FDK_label(scaleValues_DBLDBL_end)

  // restore upper core registers according to call convention
  FDK_mpop(r4,r7)
  FDK_return()
FDK_ASM_ROUTINE_END()

void scaleValues (
        FIXP_DBL *dst,
        const FIXP_DBL *src,
        INT len,
        INT scalefactor
        )
{
  scaleValuesDBLDBL(dst, src, len, scalefactor);
}

#endif /* defined(__CC_ARM) || defined(__GNUC__)*/
#endif /* !defined(FUNCTION_scaleValues_DBLDBL) */
#endif /* !defined(__ARM_ARCH_5TE__) */

#if defined(__ARM_ARCH_8__)

#ifndef RAM_BYTEALIGN
#if defined (_MSC_VER)
  /* Visual Studio */
#define RAM_BYTEALIGN(n) __declspec(align(n))
#else
  /* FDK_TOOLCHAIN=gcc */
#define RAM_BYTEALIGN(n) __attribute__ ((aligned(n)))
#endif
#else
  /* This alignment is probably limited to 8 bytes  */
  /* In some cases, when load/stores of Q registers */
  /* are used, this isn't sufficient, needs usually */
  /* 16-byte alignment for full speed performance   */
#define RAM_BYTEALIGN(x) RAM_ALIGN
#endif

#ifdef __ARM_AARCH64_NEON__
#include "arm/FDK_aarch64_neon_funcs.h"
#endif

#ifdef __ARM_AARCH64_NEON__
#define FUNCTION_scaleValuesSaturate_SGL_DBL
#define FUNCTION_scaleValuesSaturate_DBL_DBL
#endif
#ifdef FUNCTION_scaleValuesSaturate_DBL_DBL
#define FUNCTION_scaleValuesSaturate_DBL
#endif

#ifdef FUNCTION_scaleValuesSaturate_SGL_DBL
/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *  \param dst         destination buffer
 *  \param src         source buffer
 *  \param len         length of vector
 *  \param scalefactor amount of shifts to be applied
 *  \return void
 *
 */
A64_ASM_ROUTINE_START(void, scaleValuesSaturate_ARMneonV8_SD,
     (FIXP_SGL *dst,        /*!< Output */
      const FIXP_DBL *src,  /*!< Input  */
      const INT len,        /*!< Length */
      const INT scalefactor /*!< Scalefactor */
      ))

#ifndef __ARM_AARCH64_NEON__
  /* Assign call parameter to registers */
  X0 = (INT64)dst;
  X1 = (INT64)src;
  W2 = len;
  W3 = scalefactor;
#endif

  A64_mov_Xt_imm(W4, 0x00008000)

  A64_dup_Wt(32, 128, V6_4S, W3)
  A64_dup_Wt(32, 128, V7_4S, W4)

  A64_tbz_Wt(W2, 0, scaleValuesSaturate_SGL_2x)

  // scale 1 samples
  A64_ld1_lane_IA(32, V4_S, 0, X1, 4)
  A64_sqshl(32, 128, V4_4S, V4_4S, V6_4S)
  A64_sqadd(32, 128, V4_4S, V4_4S, V7_4S)
  A64_st1_lane_IA(16, V4_H, 1, X0, 2)

A64_label(scaleValuesSaturate_SGL_2x)

  A64_tbz_Wt(W2, 1, scaleValuesSaturate_SGL_4x)

  // scale 2 samples
  A64_ld1x1_IA(32, 64, V4_2S, X1, 8)
  A64_sqshl(32, 128, V4_4S, V4_4S, V6_4S)
  A64_sqadd(32, 128, V4_4S, V4_4S, V7_4S)
  A64_st1_lane_IA(16, V4_H, 1, X0, 2)
  A64_st1_lane_IA(16, V4_H, 3, X0, 2)

A64_label(scaleValuesSaturate_SGL_4x)

  A64_tbz_Wt(W2, 2, scaleValuesSaturate_SGL_loop_8x_check_z)

  // scale 4 samples
  A64_ld1x1_IA(32, 128, V4_4S, X1, 16)
  A64_sqshl(32, 128, V4_4S, V4_4S, V6_4S)
  A64_sqadd(32, 128, V4_4S, V4_4S, V7_4S)
  A64_uzp2(16, 128, V4_8H, V4_8H, V4_8H)
  A64_st1x1_IA(16, 64, V4_4H, X0, 8)

  A64_label(scaleValuesSaturate_SGL_loop_8x_check_z)

  A64_asr_Xt_imm(W2, W2, 3)          /* w2: length / 8 */
  A64_cmp_Wt_imm(W2, 0)

  A64_branch(EQ, scaleValuesSaturate_SGL_loop_8x_end)

A64_label(scaleValuesSaturate_SGL_loop_8x)

  // scale 8 samples
  A64_ld1x2_IA(64, 128, V4_2D, V5_2D, X1, 32)
  A64_sqshl(32, 128, V4_4S, V4_4S, V6_4S)
  A64_sqshl(32, 128, V5_4S, V5_4S, V6_4S)
  A64_sqadd(32, 128, V4_4S, V4_4S, V7_4S)
  A64_sqadd(32, 128, V5_4S, V5_4S, V7_4S)
  A64_uzp2(16, 128, V4_8H, V4_8H, V5_8H)
  A64_st1x1_IA(16, 128, V4_8H, X0, 16)
  A64_subs_imm(W2, W2, 1)            /* decrement 8x loop counter */
  A64_branch(GT, scaleValuesSaturate_SGL_loop_8x)

A64_label(scaleValuesSaturate_SGL_loop_8x_end)

  A64_subs_imm(W2, W2, 1)            /* dummy instruction, needed for branching */

A64_ASM_ROUTINE_END()

SCALE_INLINE void scaleValuesSaturate(
                  FIXP_SGL* dst,
                  const FIXP_DBL* src,
                  const INT len,
                  const INT scaleFactor    /* positive means: shift left */
                  )
{
  scaleValuesSaturate_ARMneonV8_SD(dst, src, len, fMax(fMin(scaleFactor, DFRACT_BITS-1), -(DFRACT_BITS-1)));
}
#endif /* FUNCTION_scaleValuesSaturate_SGL_DBL */

#ifdef FUNCTION_scaleValuesSaturate_DBL_DBL
/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *  \param dst         destination buffer
 *  \param src         source buffer
 *  \param len         length of vector
 *  \param scalefactor amount of shifts to be applied
 *  \return void
 *
 */
A64_ASM_ROUTINE_START(void, scaleValuesSaturate_ARMneonV8_DD,
       (FIXP_DBL *dst,        /*!< Output */
        const FIXP_DBL *src,  /*!< Input  */
        const INT len,        /*!< Length */
        const INT scalefactor /*!< Scalefactor */
        ))

#ifndef __ARM_AARCH64_NEON__
  /* Assign call parameter to registers */
  X0 = (INT64)dst;
  X1 = (INT64)src;
  W2 = len;
  W3 = scalefactor;
#endif

  A64_mov_Xt_imm(W4, 0x80000001)

  A64_dup_Wt(32, 128, V6_4S, W3)
  A64_dup_Wt(32, 128, V7_4S, W4)

  A64_tbz_Wt(W2, 0, scaleValuesSaturate_DBL_2x)

  // scale 1 samples
  A64_ld1_lane_IA(32, V4_S, 0, X1, 4)
  A64_sqshl(32, 128, V4_4S, V4_4S, V6_4S)
  A64_smax(32, 128, V4_4S, V4_4S, V7_4S)
  A64_st1_lane_IA(32, V4_S, 0, X0, 4)

A64_label(scaleValuesSaturate_DBL_2x)

  A64_tbz_Wt(W2, 1, scaleValuesSaturate_DBL_4x)

  // scale 2 samples
  A64_ld1x1_IA(32, 64, V4_2S, X1, 8)
  A64_sqshl(32, 128, V4_4S, V4_4S, V6_4S)
  A64_smax(32, 128, V4_4S, V4_4S, V7_4S)
  A64_st1x1_IA(32, 64, V4_2S, X0, 8)

A64_label(scaleValuesSaturate_DBL_4x)

  A64_tbz_Wt(W2, 2, scaleValuesSaturate_DBL_loop_8x_check_z)

  // scale 4 samples
  A64_ld1x1_IA(32, 128, V4_4S, X1, 16)
  A64_sqshl(32, 128, V4_4S, V4_4S, V6_4S)
  A64_smax(32, 128, V4_4S, V4_4S, V7_4S)
  A64_st1x1_IA(32, 128, V4_4S, X0, 16)

A64_label(scaleValuesSaturate_DBL_loop_8x_check_z)

  A64_asr_Xt_imm(W2, W2, 3)          /* w2: length / 8 */
  A64_cmp_Wt_imm(W2, 0)

  A64_branch(EQ, scaleValuesSaturate_DBL_loop_8x_end)

A64_label(scaleValuesSaturate_DBL_loop_8x)

  // scale 8 samples
  A64_ld1x2_IA(64, 128, V4_2D, V5_2D, X1, 32)
  A64_sqshl(32, 128, V4_4S, V4_4S, V6_4S)
  A64_sqshl(32, 128, V5_4S, V5_4S, V6_4S)
  A64_smax(32, 128, V4_4S, V4_4S, V7_4S)
  A64_smax(32, 128, V5_4S, V5_4S, V7_4S)
  A64_st1x2_IA(32, 128, V4_4S, V5_4S, X0, 32)
  A64_subs_imm(W2, W2, 1)            /* decrement 8x loop counter */
  A64_branch(GT, scaleValuesSaturate_DBL_loop_8x)

A64_label(scaleValuesSaturate_DBL_loop_8x_end)

  A64_subs_imm(W2, W2, 1)            /* dummy instruction, needed for branching */

A64_ASM_ROUTINE_END()
#endif /* FUNCTION_scaleValuesSaturate_DBL_DBL */

#ifdef FUNCTION_scaleValuesSaturate_DBL

#define FUNCTION_scaleValuesSaturate_DBL_DBL
SCALE_INLINE void scaleValuesSaturate(
                  FIXP_DBL* dst,
                  const INT len,
                  const INT scaleFactor    /* positive means: shift left */
                  )
{
  scaleValuesSaturate_ARMneonV8_DD(dst, (const FIXP_DBL*)dst, len, fMax(fMin(scaleFactor, DFRACT_BITS-1), -(DFRACT_BITS-1)));
}

SCALE_INLINE void scaleValuesSaturate(
                  FIXP_DBL* dst,
                  const FIXP_DBL* src,
                  const INT len,
                  const INT scaleFactor    /* positive means: shift left */
                  )
{
  scaleValuesSaturate_ARMneonV8_DD(dst, src, len, fMax(fMin(scaleFactor, DFRACT_BITS-1), -(DFRACT_BITS-1)));
}
#endif /* FUNCTION_scaleValuesSaturate_DBL */

#endif /* __ARM_ARCH_8__ */

#if !defined(FUNCTION_scaleValuesWithFactor_DBL)
#define FUNCTION_scaleValuesWithFactor_DBL
SCALE_INLINE
void scaleValuesWithFactor(
        FIXP_DBL *vector,
        FIXP_DBL factor,
        INT len,
        INT scalefactor
        )
{
#ifdef FUNCTION_scaleValuesWithFactor_DBL_func1
  scalefactor = fixmin_I(scalefactor, (INT)DFRACT_BITS-1);
  scalefactor = fixmax_I(scalefactor,-(INT)DFRACT_BITS+1);
  int i;
  for (i = 0; i < (len & 3); i++)
  {
      if (scalefactor >= 0)
        vector[i] = fMult(vector[i],factor) << scalefactor;
      else
        vector[i] = fMult(vector[i],factor) >> -scalefactor;
  }
  scaleValuesWithFactor_func1(&vector[i], factor, len>>2, scalefactor);
#else
  /* This code combines the fMult with the scaling             */
  /* It performs a fMultDiv2 and increments shift by 1         */
  int shift = scalefactor + 1;
  FIXP_DBL *mySpec = vector;

  shift = fixmin_I(shift,(INT)DFRACT_BITS-1);

  if (shift >= 0)
  {
    for (int i=0; i<(len>>2); i++)
    {
      FIXP_DBL tmp0 = mySpec[0];
      FIXP_DBL tmp1 = mySpec[1];
      FIXP_DBL tmp2 = mySpec[2];
      FIXP_DBL tmp3 = mySpec[3];
      tmp0 = fMultDiv2(tmp0, factor);
      tmp1 = fMultDiv2(tmp1, factor);
      tmp2 = fMultDiv2(tmp2, factor);
      tmp3 = fMultDiv2(tmp3, factor);
      tmp0 <<= shift;
      tmp1 <<= shift;
      tmp2 <<= shift;
      tmp3 <<= shift;
      *mySpec++ = tmp0;
      *mySpec++ = tmp1;
      *mySpec++ = tmp2;
      *mySpec++ = tmp3;
    }
    for (int i=len&3; i--;)
    {
      FIXP_DBL tmp0 = mySpec[0];
      tmp0 = fMultDiv2(tmp0, factor);
      tmp0 <<= shift;
      *mySpec++ = tmp0;
    }
  }
  else
  {
    shift = -shift;
    for (int i=0; i<(len>>2); i++)
    {
      FIXP_DBL tmp0 = mySpec[0];
      FIXP_DBL tmp1 = mySpec[1];
      FIXP_DBL tmp2 = mySpec[2];
      FIXP_DBL tmp3 = mySpec[3];
      tmp0 = fMultDiv2(tmp0, factor);
      tmp1 = fMultDiv2(tmp1, factor);
      tmp2 = fMultDiv2(tmp2, factor);
      tmp3 = fMultDiv2(tmp3, factor);
      tmp0 >>= shift;
      tmp1 >>= shift;
      tmp2 >>= shift;
      tmp3 >>= shift;
      *mySpec++ = tmp0;
      *mySpec++ = tmp1;
      *mySpec++ = tmp2;
      *mySpec++ = tmp3;
    }
    for (int i=len&3; i--;)
    {
      FIXP_DBL tmp0 = mySpec[0];
      tmp0 = fMultDiv2(tmp0, factor);
      tmp0 >>= shift;
      *mySpec++ = tmp0;
    }
  }
#endif /* FUNCTION_scaleValuesWithFactor_func1 */
}
#endif /* #if !defined(FUNCTION_scaleValuesWithFactor_DBL) */

#if defined(__GNUC__)
//#pragma GCC diagnostic pop
#endif /* #if defined(__GNUC__) */

#endif /* #ifndef __INCLUDE_SCALE_ARM__ */
