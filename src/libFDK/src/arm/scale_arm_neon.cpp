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

   Description: Scaling operations for ARM NEON

*******************************************************************************/

/* clang-format off */
#ifndef __INCLUDE_SCALE_ARM_NEON__
#define __INCLUDE_SCALE_ARM_NEON__

#include "arm/FDK_arm_funcs.h"
#if defined (__ARM_NEON__)
#include "arm/FDK_neon_funcs.h"
#endif

#define FUNCTION_scaleValuesWithFactor_DBL_func1
#define FUNCTION_scaleValues_DBLDBL
#define FUNCTION_scaleValuesSaturate_SGL_DBL
#define FUNCTION_scaleCplxValues
#define FUNCTION_scaleValues_DBL
#define FUNCTION_getScalefactor_DBL
#define FUNCTION_getScalefactor_SGL

#define FUNCTION_scaleValuesSaturate_DBL_DBL
#ifdef FUNCTION_scaleValuesSaturate_DBL_DBL
#define FUNCTION_scaleValuesSaturate_DBL
#endif

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
FDK_ASM_ROUTINE_START(void, scaleValuesSaturate_ARMneonV7,
       (FIXP_DBL *dst,       /*!< Output */
        const FIXP_DBL *src, /*!< Input   */
        INT len,             /*!< Length */
        INT scalefactor      /*!< Scalefactor */
        ))
  /* stack contents:
       none

     register contents:
       r0: dst                       format: <pointer to Q1.31>
       r1: src                       format: <pointer to Q1.31>
       r2: len                       format: int
       r3: scalefactor               format: int, positive: shift-left, negative: shift-right

     NEON registers:
       Q0: vector data: 1st 4 values
       Q1: vector data: 2nd 4 values
       Q2: scalefactor (duplicated)
       Q3: 0x8000 0001 (duplicated)
  */
  FDK_vmov_i32(128, Q3, 0x80000000)    // Q3: 0x80000000 (duplicated)
  FDK_vmov_i32(128, Q2, 0x00000001)    // Q2: 0x00000001 (duplicated)
  FDK_vadd_s32_q(Q3, Q3, Q2)           // Q3: 0x80000001 (duplicated)

  FDK_cmp_imm(r3, 0x1F)
  FDK_mov_cond_imm(GT, r3, 0x1F)
  FDK_cmn_imm(r3, 0x1F)
  FDK_mvn_cond_imm(LT, r3, 0x1F)
  FDK_vdup_q_reg(32, Q2, r3)           // Duplicate "scalefactor" into all 4 lanes of Q2/D4
  FDK_movs_asr_imm(r2, r2, 1)          // r3: length / 2
  FDK_branch(CC, scaleValuesSaturate_DBL_loop_2x)
    // scale 1 samples
    FDK_vld1_ia(32, S0, r1)
    FDK_vqshl_s32(128, D0, D0, D4)     // D0 = D0 << scalefactor
    FDK_vmax_s32(128, D0, D0, D6)
    FDK_vst1_ia(32, S0, r0)

FDK_label(scaleValuesSaturate_DBL_loop_2x)
  FDK_movs_asr_imm(r2, r2, 1)          // r3: length / 4
  FDK_branch(CC, scaleValuesSaturate_DBL_loop_4x)
    // scale 2 samples
    FDK_vld1_1d_ia(32, D0, r1)
    FDK_vqshl_s32(128, D0, D0, D4)     // D0 = D0 << scalefactor
    FDK_vmax_s32(128, D0, D0, D6)
    FDK_vst1_1d_ia(32, D0, r0)

FDK_label(scaleValuesSaturate_DBL_loop_4x)
  FDK_movs_asr_imm(r2, r2, 1)          // r3: length / 8
  FDK_branch(CC, scaleValuesSaturate_DBL_loop_8x_check_z)
    // scale 4 samples
    FDK_vld1_2d_ia(32, D0, D1, r1)
    FDK_vqshl_s32(128, Q0, Q0, Q2)     // Q0 = Q0 << scalefactor
    FDK_vmax_s32(128, Q0, Q0, Q3)
    FDK_vst1_2d_ia(32, D0, D1, r0)

FDK_label(scaleValuesSaturate_DBL_loop_8x_check_z)
  FDK_branch(EQ, scaleValuesSaturate_DBL_loop_8x_end)

FDK_label(scaleValuesSaturate_DBL_loop_8x)
    // scale 8 samples
    FDK_vld1_4d_ia(32, D0, D1, D2, D3, r1)
    FDK_subs_imm(r2, r2, 1)            // decrement 8x loop counter
    FDK_vqshl_s32(128, Q0, Q0, Q2)     // Q0 = Q0 << scalefactor
    FDK_vqshl_s32(128, Q1, Q1, Q2)     // Q1 = Q1 << scalefactor
    FDK_vmax_s32(128, Q0, Q0, Q3)
    FDK_vmax_s32(128, Q1, Q1, Q3)
    FDK_vst1_4d_ia(32, D0, D1, D2, D3, r0)
    FDK_branch(NE, scaleValuesSaturate_DBL_loop_8x)

FDK_label(scaleValuesSaturate_DBL_loop_8x_end)
  FDK_return()
FDK_ASM_ROUTINE_END()

#endif /* FUNCTION_scaleValuesSaturate_DBL_DBL */


#ifdef FUNCTION_scaleValuesSaturate_DBL
LNK_SECTION_CODE_L1
SCALE_INLINE void scaleValuesSaturate(
                  FIXP_DBL *dst,
                  INT len,
            const INT scaleFactor)    /* positive means: shift left */
{
  scaleValuesSaturate_ARMneonV7(dst, (const FIXP_DBL *)dst, len, scaleFactor);
}

LNK_SECTION_CODE_L1
SCALE_INLINE void scaleValuesSaturate(
  FIXP_DBL *dst,
  const FIXP_DBL *src,
  INT len,
  const INT scaleFactor)    /* positive means: shift left */
{
  scaleValuesSaturate_ARMneonV7(dst, src, len, scaleFactor);
}
#endif


#ifdef FUNCTION_scaleValuesWithFactor_DBL_func1
FDK_ASM_ROUTINE_START(void, scaleValuesWithFactor_func1,
       (FIXP_DBL *r0,      /* vector, */
        FIXP_DBL r1,       /* factor, */
        INT r2,            /* len4x,  */
        INT r3             /* scalefactor */
        ))
  /* stack contents:
       none

     register contents:
       r0: vector                    format: <pointer to Q1.31>
       r1: factor                    format: Q1.31
       r2: len4x                     format: int, actual length divided by 4
       r3: scalefactor               format: int, positive: shift-left, negative: shift-right

     NEON registers:
       Q0: vector data: 1st 4 values
       Q1: vector data: 2nd 4 values
       Q2: factor
       Q3: scalefactor
  */
  FDK_vdup_q_reg(32, Q2, r1)  // Duplicate "factor" into all 4 lanes of Q2
  FDK_vdup_q_reg(32, Q3, r3)  // Duplicate "scalefactor" into all 4 lanes of Q3
  FDK_movs_asr_imm(r2, r2, 1) // r2: len4x / 2
  FDK_branch(CC, scaleValuesWithFactor_func1_loop_8x_check_z)
    // scale 4 samples
    FDK_vld1_2d(32, D0, D1, r0)
    FDK_vqdmulh_s32_qq(Q0, Q0, Q2) // Q0 = fMult(Q0, Q2);
    FDK_vshl_s32_q(Q0, Q0, Q3)     // Q0 = Q0 << scalefactor
    FDK_vst1_2d_ia(32, D0, D1, r0)

FDK_label(scaleValuesWithFactor_func1_loop_8x_check_z)
  FDK_branch(EQ, scaleValuesWithFactor_func1_loop_8x_end)

FDK_label(scaleValuesWithFactor_func1_loop_8x)
    FDK_vld1_4d(32, D0, D1, D2, D3, r0)
    FDK_subs_imm(r2, r2, 1)   // decrement 8x loop counter
    FDK_vqdmulh_s32_qq(Q0, Q0, Q2) // Q0 = fMult(Q0, Q2);
    FDK_vqdmulh_s32_qq(Q1, Q1, Q2) // Q1 = fMult(Q1, Q2);
    FDK_vshl_s32_q(Q0, Q0, Q3)     // Q0 = Q0 << scalefactor
    FDK_vshl_s32_q(Q1, Q1, Q3)     // Q1 = Q1 << scalefactor
    FDK_vst1_4d_ia(32, D0, D1, D2, D3, r0)
    FDK_branch(NE, scaleValuesWithFactor_func1_loop_8x)

FDK_label(scaleValuesWithFactor_func1_loop_8x_end)
  FDK_return()
FDK_ASM_ROUTINE_END()
#endif /* FUNCTION_scaleValuesWithFactor_DBL_func1 */

#ifdef FUNCTION_scaleValues_DBLDBL
FDK_ASM_ROUTINE_START(void, scaleValues_neon,
       (FIXP_DBL *r0,     /* dst, */
  const FIXP_DBL *r1,     /* src, */
        INT r2,           /* len, */
        INT r3            /* scalefactor */
        ))
  /* stack contents:
       none

     register contents:
       r0: dst                       format: <pointer to Q1.31>
       r1: src                       format: <pointer to Q1.31>
       r2: length                    format: int, in range [0...]
       r3: scalefactor               format: int, positive: shift-left, negative: shift-right, limited to 0..31

     NEON registers:
       Q0: src/dst data: 1st 4 values
       Q1: src/dst data: 2nd 4 values
       Q2: scalefactor
  */
  FDK_cmp_imm(r3, 0x1F)
  FDK_mov_cond_imm(GT, r3, 0x1F)
  FDK_cmn_imm(r3, 0x1F)
  FDK_mvn_cond_imm(LT, r3, 0x1F)
  FDK_vdup_q_reg(32, Q2, r3)       // Duplicate "scalefactor" into all 4 lanes of Q2/D4
  FDK_movs_asr_imm(r2, r2, 1)      // r2: length / 2
  FDK_branch(CC, scaleValues_func1_loop_2x)
    // scale 1 samples
    FDK_vld1_ia(32, S0, r1)
    FDK_vshl_s32_d(D0, D0, D4)     // D0[0] = D0[0] << scalefactor
    FDK_vst1_ia(32, S0, r0)

FDK_label(scaleValues_func1_loop_2x)
  FDK_movs_asr_imm(r2, r2, 1)      // r2: length / 4
  FDK_branch(CC, scaleValues_func1_loop_4x)
    // scale 2 samples
    FDK_vld1_1d_ia(32, D0, r1)
    FDK_vshl_s32_d(D0, D0, D4)     // D0 = D0 << scalefactor
    FDK_vst1_1d_ia(32, D0, r0)

FDK_label(scaleValues_func1_loop_4x)
  FDK_movs_asr_imm(r2, r2, 1)      // r2: length / 8
  FDK_branch(CC, scaleValues_func1_loop_8x_check_z)
    // scale 4 samples
    FDK_vld1_2d_ia(32, D0, D1, r1)
    FDK_vshl_s32_q(Q0, Q0, Q2)     // Q0 = Q0 << scalefactor
    FDK_vst1_2d_ia(32, D0, D1, r0)

FDK_label(scaleValues_func1_loop_8x_check_z)
  FDK_branch(EQ, scaleValues_func1_loop_8x_end)

FDK_label(scaleValues_func1_loop_8x)
    // scale 8 samples
    FDK_vld1_4d_ia(32, D0, D1, D2, D3, r1)
    FDK_subs_imm(r2, r2, 1)        // decrement 8x loop counter
    FDK_vshl_s32_q(Q0, Q0, Q2)     // Q0 = Q0 << scalefactor
    FDK_vshl_s32_q(Q1, Q1, Q2)     // Q1 = Q1 << scalefactor
    FDK_vst1_4d_ia(32, D0, D1, D2, D3, r0)
    FDK_branch(NE, scaleValues_func1_loop_8x)

FDK_label(scaleValues_func1_loop_8x_end)
  FDK_return()
FDK_ASM_ROUTINE_END()

void scaleValues(
        FIXP_DBL *dst,
  const FIXP_DBL *src,
        INT  len,
        INT scalefactor)
{
    scaleValues_neon (dst, (const FIXP_DBL *) src, len, scalefactor);
}

#ifdef FUNCTION_scaleValues_DBL
void scaleValues(
        FIXP_DBL *dst,
        INT  len,
        INT scalefactor)
{
    scaleValues_neon (dst, (const FIXP_DBL *) dst, len, scalefactor);
}
#endif /* FUNCTION_scaleValues_DBL */

#endif /* FUNCTION_scaleValues_DBLDBL */


#ifdef FUNCTION_scaleValuesSaturate_SGL_DBL
/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *  \param dst         destination buffer (FIXP_SGL)
 *  \param src         source buffer (FIXP_DBL)
 *  \param len         length of vector
 *  \param scalefactor amount of shifts to be applied
 *  \return void
 *
 */
 FDK_ASM_ROUTINE_START(void, scaleValuesSaturate_neon,
       (FIXP_SGL *r0,     /* dst, */
  const FIXP_DBL *r1,     /* src, */
        INT r2,           /* len, */
        INT r3            /* scalefactor */
        ))
  /* stack contents:
       none

     register contents:
       r0: dst                       format: <pointer to Q1.15>
       r1: src                       format: <pointer to Q1.31>
       r2: length                    format: int, in range [0...]
       r3: scalefactor               format: int, positive: shift-left, negative: shift-right, limited to 0..31

     NEON registers:
       Q0: src/dst data: 1st 4 values
       Q1: src/dst data: 2nd 4 values
       Q2: 4x scalefactor
       Q3: 4x rounding constant 0x00008000
  */
  FDK_cmp_imm(r3, 0x1F)
  FDK_mov_cond_imm(GT, r3, 0x1F)
  FDK_cmn_imm(r3, 0x1F)
  FDK_mvn_cond_imm(LT, r3, 0x1F)
  FDK_vdup_q_reg(32, Q2, r3)       // Duplicate "scalefactor" into all 4 lanes of Q2
  FDK_mov_imm(r3, 0x00008000)
  FDK_vdup_q_reg(32, Q3, r3)       // Duplicate "rounding constant" into all 4 lanes of Q3
  FDK_movs_asr_imm(r2, r2, 1)      // r2: length / 2
  FDK_branch(CC, scaleValuesSaturate_loop_2x)
    // scale 1 samples
    FDK_vld1_ia(32, S0, r1)
    FDK_vqshl_s32(64, D0, D0, D4)   // D0[0] = sat(D0[0] << scalefactor)
    FDK_vqadd_s32_d(D0, D0, D6)     // D0[0] = sat(D0[0] + 0x00008000)
    FDK_vst1_ia(16, D0_1, r0)       // store upper half of S0

FDK_label(scaleValuesSaturate_loop_2x)
  FDK_movs_asr_imm(r2, r2, 1)      // r2: length / 4
  FDK_branch(CC, scaleValuesSaturate_loop_4x)
    // scale 2 samples
    FDK_vld1_1d_ia(32, D0, r1)      // S1=src[1]   S0=src[0]
    FDK_vqshl_s32(64, D0, D0, D4)   // D0[0,1] = sat(D0[0,1] << scalefactor)
    FDK_vqadd_s32_d(D0, D0, D6)     // D0[0,1] = sat(D0[0,1] + 0x00008000)
    FDK_vst1_ia(16, D0_1, r0)       // store upper half of S0
    FDK_vst1_ia(16, D0_3, r0)       // store upper half of S1

FDK_label(scaleValuesSaturate_loop_4x)
  FDK_movs_asr_imm(r2, r2, 1)      // r2: length / 8
  FDK_branch(CC, scaleValuesSaturate_loop_8x_check_z)
    // scale 4 samples
    FDK_vld1_2d_ia(32, D0, D1, r1)
    FDK_vqshl_s32(128, Q0, Q0, Q2)  // Q0[0-3] = sat(Q0[0-3] << scalefactor)
    FDK_vqadd_s32_q(Q0, Q0, Q3)     // Q0[0-3] = sat(Q0[0-3] + 0x00008000)
    FDK_vuzp_d(16, D0, D1)          // D1: S3.H  S2.H  S1.H  S0.H   D0: S3.L  S2.L  S1.L  S0.L
    FDK_vst1_1d_ia(16, D1, r0)

FDK_label(scaleValuesSaturate_loop_8x_check_z)
  FDK_branch(EQ, scaleValuesSaturate_loop_8x_end)

FDK_label(scaleValuesSaturate_loop_8x)
    // scale 8 samples
    FDK_vld1_4d_ia(32, D0, D1, D2, D3, r1)
    FDK_subs_imm(r2, r2, 1)         // decrement 8x loop counter
    FDK_vqshl_s32(128, Q0, Q0, Q2)  // Q0[0-3] = sat(Q0[0-3] << scalefactor)
    FDK_vqshl_s32(128, Q1, Q1, Q2)  // Q1[0-3] = sat(Q1[0-3] << scalefactor)
    FDK_vqadd_s32_q(Q0, Q0, Q3)     // Q0[0-3] = sat(Q0[0-3] + 0x00008000)
    FDK_vqadd_s32_q(Q1, Q1, Q3)     // Q1[0-3] = sat(Q1[0-3] + 0x00008000)
    FDK_vuzp_d(16, Q0, Q1)          // Q1:S7.H S6.H S5.H S4.H S3.H S2.H S1.H S0.H
    FDK_vst1_2d_ia(16, D2, D3, r0)
    FDK_branch(NE, scaleValuesSaturate_loop_8x)

FDK_label(scaleValuesSaturate_loop_8x_end)
  FDK_return()
FDK_ASM_ROUTINE_END()

void scaleValuesSaturate(
        FIXP_SGL *dst,
        const FIXP_DBL *src,
        INT  len,
        INT scalefactor)
{
    scaleValuesSaturate_neon (dst, src, len, scalefactor);
}

#endif /* FUNCTION_scaleValuesSaturate_SGL_DBL */





#ifdef FUNCTION_scaleCplxValues

FDK_ASM_ROUTINE_START(void, scaleCplxValues_neon,
       (FIXP_DBL *r_dst,
        FIXP_DBL *i_dst,
  const FIXP_DBL *r_src,
  const FIXP_DBL *i_src,
        INT len,
        INT scalefactor ))
  /* stack contents:
       0x0C:     scalefactor
       0x08:     len
       0x04:     r4
       0x00:     r5

     register contents:
       r0: r_dst                     real destination address, format: <pointer to Q1.31>
       r1: i_dst                     imag destination address, format: <pointer to Q1.31>
       r2: r_src                     real source address, format: <pointer to Q1.31>
       r3: i_src                     imag source address, format: <pointer to Q1.31>
       r4: length                    format: int, in range [0...]
       r5: scalefactor               format: int, positive: shift-left, negative: shift-right, limited to 0..31

     NEON registers:
       Q0: r_src/r_dst data: 1st 4 values
       Q1: r_src/r_dst data: 2nd 4 values
       Q2: i_src/i_dst data: 1st 4 values
       Q3: i_src/i_dst data: 2nd 4 values
       Q8: scalefactor
  */
  FDK_mpush(r4,r5)

  FDK_ldrd(r4, r5, sp, 0x8, length, scalefactor)    // r4: length   r5: scalefactor
  FDK_cmp_imm(r5, 0x1F)
  FDK_mov_cond_imm(GT, r5, 0x1F)
  FDK_cmn_imm(r5, 0x1F)
  FDK_mvn_cond_imm(LT, r5, 0x1F)
  FDK_vdup_q_reg(32, Q8, r5)       // Duplicate "scalefactor" into all 4 lanes of Q8/D16
  FDK_movs_asr_imm(r4, r4, 1)      // r4: length / 2
  FDK_branch(CC, scaleCplxValues_func1_loop_2x)
    // scale 1 samples
    FDK_vld1_ia(32, S0, r2)                 // load 1x r_src
    FDK_vld1_ia(32, S1, r3)                 // load 1x i_src
    FDK_vshl_s32_d(D0, D0, D16)             // D0 = D0 << scalefactor
    FDK_vst1_ia(32, S0, r0)                 // store 1x r_dst
    FDK_vst1_ia(32, S1, r1)                 // store 1x i_dst

FDK_label(scaleCplxValues_func1_loop_2x)
  FDK_movs_asr_imm(r4, r4, 1)               // r4: length / 4
  FDK_branch(CC, scaleCplxValues_func1_loop_4x)
    // scale 2 samples
    FDK_vld1_1d_ia(32, D0, r2)              // load 2x r_src
    FDK_vld1_1d_ia(32, D1, r3)              // load 2x r_src
    FDK_vshl_s32_q(Q0, Q0, Q8)              // Q0 = Q0 << scalefactor
    FDK_vst1_1d_ia(32, D0, r0)              // store 2x i_dst
    FDK_vst1_1d_ia(32, D1, r1)              // store 2x i_dst

FDK_label(scaleCplxValues_func1_loop_4x)
  FDK_movs_asr_imm(r4, r4, 1)               // r4: length / 8
  FDK_branch(CC, scaleCplxValues_func1_loop_8x_check_z)
    // scale 4 samples
    FDK_vld1_2d_ia(32, D0, D1, r2)          // load 4x r_src
    FDK_vld1_2d_ia(32, D2, D3, r3)          // load 4x r_src
    FDK_vshl_s32_q(Q0, Q0, Q8)              // Q0 = Q0 << scalefactor
    FDK_vshl_s32_q(Q1, Q1, Q8)              // Q0 = Q0 << scalefactor
    FDK_vst1_2d_ia(32, D0, D1, r0)          // store 8x r_dst
    FDK_vst1_2d_ia(32, D2, D3, r1)          // store 8x i_dst

FDK_label(scaleCplxValues_func1_loop_8x_check_z)
  FDK_branch(EQ, scaleCplxValues_func1_loop_8x_end)

FDK_label(scaleCplxValues_func1_loop_8x)
    // scale 8 samples
    FDK_vld1_4d_ia(32, D0, D1, D2, D3, r2)  // load 8x r_src
    FDK_vld1_4d_ia(32, D4, D5, D6, D7, r3)  // load 8x i_src
    FDK_subs_imm(r4, r4, 1)                 // decrement 8x loop counter
    FDK_vshl_s32_q(Q0, Q0, Q8)              // Q0 = Q0 << scalefactor
    FDK_vshl_s32_q(Q1, Q1, Q8)              // Q1 = Q1 << scalefactor
    FDK_vshl_s32_q(Q2, Q2, Q8)              // Q2 = Q2 << scalefactor
    FDK_vshl_s32_q(Q3, Q3, Q8)              // Q3 = Q3 << scalefactor
    FDK_vst1_4d_ia(32, D0, D1, D2, D3, r0)  // store 8x r_dst
    FDK_vst1_4d_ia(32, D4, D5, D6, D7, r1)  // store 8x i_dst
    FDK_branch(NE, scaleCplxValues_func1_loop_8x)

FDK_label(scaleCplxValues_func1_loop_8x_end)
  FDK_mpop(r4,r5)
  FDK_return()
FDK_ASM_ROUTINE_END()

void scaleCplxValues(
        FIXP_DBL *r_dst,
        FIXP_DBL *i_dst,
  const FIXP_DBL *r_src,
  const FIXP_DBL *i_src,
        INT  len,
        INT scalefactor)
{
    scaleCplxValues_neon (r_dst, i_dst, r_src, i_src, len, scalefactor);
}

#endif /* FUNCTION_scaleCplxValues */

#ifdef FUNCTION_getScalefactor_DBL
FDK_ASM_ROUTINE_START(INT, getScalefactorDBL_neon,(const FIXP_DBL *vector, INT len))
  /* stack contents:
       none

     register contents:
       r0: vector                    format: <pointer to Q1.31>
       r1: length                    format: int, in range [0...]
       r2: retval (arm-gcc only)     format: <pointer to INT

     NEON registers:
       Q0: vector data: 1st 4 values
       Q1: vector data: 2nd 4 values
       Q2: maximum
       Q3: minimum

     return value:
       r0: logical-or of all data[i]^data[i]>>31
  */
  FDK_vmov_i32(128, Q0, 0)    // current maximum
  FDK_vmov_i32(128, Q1, 0)    // current minimum
  FDK_vmov_i32(128, Q2, 0)    // working register for intro values

  FDK_movs_asr_imm(r1, r1, 1) // r1: length / 2
  FDK_branch(CC, getScalefactor_DBL_2x)
    // test 1 samples
    FDK_vld1_ia(32, S8, r0)   // load Q2[0]
    FDK_vmax_s32(128, Q0, Q0, Q2)
    FDK_vmin_s32(128, Q1, Q1, Q2)

FDK_label(getScalefactor_DBL_2x)
  FDK_movs_asr_imm(r1, r1, 1) // r1: length / 4
  FDK_branch(CC, getScalefactor_DBL_4x)
    // test 2 samples
    FDK_vld1_1d_ia(32, D5, r0)
    FDK_vmax_s32(128, Q0, Q0, Q2)
    FDK_vmin_s32(128, Q1, Q1, Q2)

FDK_label(getScalefactor_DBL_4x)
  FDK_movs_asr_imm(r1, r1, 1) // r1: length / 8
  FDK_branch(CC, getScalefactor_DBL_8x_check_z)
    // test 4 samples
    FDK_vld1_2d_ia(32, D4, D5, r0)
    FDK_vmax_s32(128, Q0, Q0, Q2)
    FDK_vmin_s32(128, Q1, Q1, Q2)

FDK_label(getScalefactor_DBL_8x_check_z)
  FDK_branch(EQ, getScalefactor_DBL_8x_end)

FDK_label(getScalefactor_DBL_8x)
    // test 8 samples
    FDK_vld1_4d_ia(32, D4, D5, D6, D7, r0)
    FDK_subs_imm(r1, r1, 1)
    FDK_vmax_s32(128, Q0, Q0, Q2)
    FDK_vmin_s32(128, Q1, Q1, Q2)
    FDK_vmax_s32(128, Q0, Q0, Q3)
    FDK_vmin_s32(128, Q1, Q1, Q3)
    FDK_branch(NE, getScalefactor_DBL_8x)

FDK_label(getScalefactor_DBL_8x_end)
  FDK_vshr_s32_q_imm(Q2, Q0, 31)   // positive maximum >> 31
  FDK_vshr_s32_q_imm(Q3, Q1, 31)   // negative minimum >> 31
  FDK_veor(128, Q0, Q0, Q2)        // absolute(maximum)
  FDK_veor(128, Q1, Q1, Q3)        // absolute(minimum)
  FDK_vmax_s32(128, Q0, Q0, Q1)    // max(abs(maximum), abs(minimum)
  FDK_vmax_s32(64,  D0, D0, D1)    // D0: maximum of S0,S1 and S2,S3
  FDK_vpmax_s32(64, D0, D0, D0)    // D0[0]: maximum of S0, S1
  FDK_vmov_reg(r0, s0)             // return value in r0
  FDK_clz(r0, r0)                  // r0: in range 1..32
  FDK_subs_imm(r0, r0, 1)          // r0: in range 0..31
  FDK_return()
FDK_ASM_ROUTINE_RETURN(INT)

INT getScalefactor(const FIXP_DBL *vector, INT len)
{
   return getScalefactorDBL_neon(vector, len);
}

#endif /* FUNCTION_getScalefactor_DBL */

#ifdef FUNCTION_getScalefactor_SGL
FDK_ASM_ROUTINE_START(INT, getScalefactorSGL_neon,(const FIXP_SGL *vector, INT len))
  /* stack contents:
       none

     register contents:
       r0: vector                    format: <pointer to Q1.15>
           return value (armcc-only) format: INT, in range
       r1: length                    format: INT, in range [0...]
       r2: retval (arm-gcc only)     format: <pointer to INT

     NEON registers:
       Q0: maximum
       Q1: minimum
       Q2,Q3: loaded values

     return value:
       r0: logical-or of all data[i]^data[i]>>31
  */
  FDK_vmov_i32(128, Q0, 0)    // current maximum
  FDK_vmov_i32(128, Q1, 0)    // current maximum
  FDK_vmov_i32(128, Q2, 0)    // working register for intro values

  FDK_movs_asr_imm(r1, r1, 1) // r1: length / 2
  FDK_branch(CC, getScalefactor_SGL_2x)
    // test 1 samples
    FDK_vld1_ia(16, D4_0, r0)   // load Q2[0]
    FDK_vmax_s16(64, D0, D0, D4)
    FDK_vmin_s16(64, D2, D2, D4)

FDK_label(getScalefactor_SGL_2x)
  FDK_movs_asr_imm(r1, r1, 1) // r1: length / 4
  FDK_branch(CC, getScalefactor_SGL_4x)
    // test 2 samples
    FDK_vld1_ia(16, D4_0, r0)   // load Q2[0]
    FDK_vld1_ia(16, D4_1, r0)   // load Q2[1]
    FDK_vmax_s16(64, D0, D0, D4)
    FDK_vmin_s16(64, D2, D2, D4)

FDK_label(getScalefactor_SGL_4x)
  FDK_movs_asr_imm(r1, r1, 1) // r1: length / 8
  FDK_branch(CC, getScalefactor_SGL_8x_check_z)
    // test 4 samples
    FDK_vld1_1d_ia(16, D4, r0)
    FDK_vmax_s16(64, D0, D0, D4)
    FDK_vmin_s16(64, D2, D2, D4)

FDK_label(getScalefactor_SGL_8x_check_z)
  FDK_branch(EQ, getScalefactor_SGL_8x_end)

FDK_label(getScalefactor_SGL_8x)
    // test 8 samples
    FDK_vld1_2d_ia(16, D4, D5, r0)
    FDK_subs_imm(r1, r1, 1)
    FDK_vmax_s16(128, Q0, Q0, Q2)
    FDK_vmin_s16(128, Q1, Q1, Q2)
    FDK_branch(NE, getScalefactor_SGL_8x)

FDK_label(getScalefactor_SGL_8x_end)
  FDK_vshr_s16_q_imm(Q2, Q0, 15)   // Q2: maximum >> 31
  FDK_vshr_s16_q_imm(Q3, Q1, 15)   // Q3: minimum >> 31
  FDK_veor(128, Q0, Q0, Q2)        // Q0: abs(maximum)
  FDK_veor(128, Q1, Q1, Q3)        // Q1: abs(minimum)
  FDK_vmax_s16(128, Q0, Q0, Q1)    // Q0: max( abs(maximum),abs(minimum) )
  FDK_vmax_s16( 64, D0, D0, D1)    // D0: 4x maximum
  FDK_vpmax_s16(64, D0, D0, D0)    // D0[0,1]: 2x maximum
  FDK_vpmax_s16(64, D0, D0, D0)    // D0[0]: maximum
  FDK_vmovl_s(16, Q0, D0)          // sign extend 16-bit value in D0[0] into S0
  FDK_vmov_reg(r0, s0)             // return value in r0
  FDK_clz(r0, r0)                  // r0: in range 17..32
  FDK_subs_imm(r0, r0, 17)         // r0: in range 0..15
  FDK_return()
FDK_ASM_ROUTINE_RETURN()

INT getScalefactor(const FIXP_SGL *vector, INT len)
{
   return getScalefactorSGL_neon(vector, len);
}
#endif /* FUNCTION_getScalefactor_SGL */
#endif /* #ifndef __INCLUDE_SCALE_ARM_NEON__ */
