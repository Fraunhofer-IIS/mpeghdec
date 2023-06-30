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

/******************** MPEG-H 3DA object rendering library **********************

   Author(s):   A. Tritthart

   Description:

*******************************************************************************/

/* clang-format off */
#include "arm/FDK_arm_funcs.h"

#if defined(__ARM_NEON__)    /* ARMv7 32-Bit version only */
#include "arm/FDK_neon_funcs.h"
#define FUNCTION_gVBAPRenderer_RenderFrame_Time_func1
#define FUNCTION_gVBAPRenderer_RenderFrame_Time_func2

#ifdef FUNCTION_gVBAPRenderer_RenderFrame_Time_func1
FDK_ASM_ROUTINE_START(void, gVBAPRenderer_RenderFrame_Time_func1,(FIXP_DBL *__restrict pOut, FIXP_DBL *__restrict pIn, UINT length, UINT startSamplePosition, FIXP_DBL scale, FIXP_DBL step, FIXP_DBL scaleState, FIXP_DBL stepState, INT s, INT s1))

  FDK_mpush(r4,r5)

  /* stack contents:
       0x1c:     s1
       0x18:     s
       0x14:     stepState
       0x10:     scaleState
       0x0c:     step
       0x08:     scale
       0x04:     r5
       0x00:     r4

     register contents:
       r0: pOut                 format: Q1.31
       r1: pIn                  format: FIXP_DBL (32-bit)
       r2: length               format: UINT, (multiple of 8, unequal 0)
       r3: startSamplePosition  format: UINT, (multiple of 8)

     register usage:
       r0: pOut
       r1: pIn
       r2: length
       r3: startSamplePosition, used as decremental loop counter
       r4: scaleState, scale    format: FIXP_DBL
       r5: stepState, step      format: FIXP_DBL

       Q0: pOut[i+0..i+3]  4 samples of pOut in format FIXP_DBL
       Q1: pOut[i+4..i+7]  4 samples of pOut in format FIXP_DBL
       Q2: scale factor    scale+4*step, scale+3*step, scale+2*step, scale+1*step
       Q3: 4x step         4 times 4*step in format FIXP_DBL
       Q8: pIn[i+0..i+3]   4 samples of pIn (16bit), shifted left by 11 into long format FIXP_DBL
       Q9: pIn[i+4..i+3]   4 samples of pIn (16bit), shifted left by 11 into long format FIXP_DBL
       Q10: 4x s1  with s1 = (s < 0) ? s1 - 1 : -s1 - 1
  */
  FDK_ldrd(r4, r5, sp, 0x18, s,  s1)            // r4: s, r5: s1
  FDK_sub(r2, r2, r3)                           // r2: (length - startSamplePosition) = iterations for 2nd loop
  FDK_cmp_imm(r4, 0)                            // s < 0 ?  shl : shr
  FDK_rsb_cond_imm(GE, r5, r5, 0)               // s >= 0 ? shr -> r5 = -s1
  FDK_sub_imm(r5, r5, 1, 0)                     // r5: s1
  FDK_vdup_q_reg(32, Q10, r5)                   // Q10:  4x +/-s1


  FDK_ldrd(r4, r5, sp, 0x10, scaleState, stepState)   // r4: scaleState, r5: stepState

FDK_label(gVBAPRenderer_RenderFrame_Time_func1_loop2)

  FDK_cmp_imm(r3, 0)
  FDK_branch(EQ, gVBAPRenderer_RenderFrame_Time_func1_loop1_end)

  FDK_vdup_q_reg(32, Q2, r4)                    // Q2:  4x scaleState, scale
                                                // lane    3      2       1       0
  FDK_vdup_q_reg(32, Q0, r5)                    // Q0:  1*step  1*step  1*step  1*step
  FDK_vadd_s32_q(Q1, Q0, Q0)                    // Q1:  2*step  2*step  2*step  2*step
  FDK_vadd_s32_q(Q3, Q1, Q1)                    // Q3:  4*step  4*step  4*step  4*step
  FDK_vzip_d(32, D0, D2)                        // D0:                  2*step  1*step
  FDK_vadd_s32_d(D1, D0, D3)                    // Q1:  4*step  3*step  2*step  1*step
  FDK_vadd_s32_q(Q2, Q2, Q0)                    // Q2:  sc+4*s, sc+3*s, sc+2*s, sc+1*s

FDK_label(gVBAPRenderer_RenderFrame_Time_func1_loop1)
    FDK_vld1_4d(32, D0, D1, D2, D3, r0)          // Q0:  pOut[3] pOut[2] pOut[1] pOut[0] in Q1.31
                                                 // Q1:  pOut[7] pOut[6] pOut[5] pOut[4] in Q1.31
    FDK_vld1_4d_ia(32, D16, D17, D18, D19, r1)   // Q8:  pIn[3]  pIn[2]  pIn[1]  pIn[0]  in Q1.31
                                                 // Q9:  pIn[7]  pIn[6]  pIn[5]  pIn[4]  in Q1.31
    FDK_subs_imm(r3, r3, 8)                      // r3:  decrement loop counter by 8
    FDK_vqdmulh_s32_qq(Q8, Q8, Q2)               // Q8:  4x fMult(pIn[i],sc+i*step) (i=0,1,2,3)
    FDK_vadd_s32_q(Q2, Q2, Q3)                   // Q2: sc+4*s, sc+3*s, sc+2*s, sc+1*s all ++4*step
    FDK_vqdmulh_s32_qq(Q9, Q9, Q2)               // Q9:  4x fMult(pIn[i],sc+i*step) (i=4,5,6,7)
    FDK_vadd_s32_q(Q2, Q2, Q3)                   // Q2: sc+4*s, sc+3*s, sc+2*s, sc+1*s all ++4*step
    FDK_vqshl_s32(128, Q8, Q8, Q10)              // Q0:  4x fMult(pIn[i],sc+i*step) (i=0,1,2,3) << s1
    FDK_vqshl_s32(128, Q9, Q9, Q10)              // Q1:  4x fMult(pIn[i],sc+i*step) (i=4,5,6,7) << s1
    FDK_vadd_s32_q(Q8, Q0, Q8)                   // Q8:  pOut[3] pOut[2] pOut[1] pOut[0] in Q1.31
    FDK_vadd_s32_q(Q9, Q1, Q9)                   // Q9:  pOut[7] pOut[6] pOut[5] pOut[4] in Q1.31
    FDK_vst1_4d_ia(32, D16, D17, D18, D19, r0)   // store Q8,Q9 via pOut (pOut[0]..pOut[7])
    FDK_branch(NE, gVBAPRenderer_RenderFrame_Time_func1_loop1)

FDK_label(gVBAPRenderer_RenderFrame_Time_func1_loop1_end)
  FDK_ldrd(r4, r5, sp, 0x08, scale, step)        // r4: scale  r5: step
  FDK_subs_imm(r3, r2, 0)
  FDK_mov_imm(r2, 0)
  FDK_branch(NE, gVBAPRenderer_RenderFrame_Time_func1_loop2)

  FDK_mpop(r4,r5)
  FDK_return()

FDK_ASM_ROUTINE_END()
#endif /* #ifdef FUNCTION_gVBAPRenderer_RenderFrame_Time_func1 */


#ifdef FUNCTION_gVBAPRenderer_RenderFrame_Time_func2
FDK_ASM_ROUTINE_START(void, gVBAPRenderer_RenderFrame_Time_func2, (FIXP_DBL * __restrict pOut, FIXP_DBL * __restrict pIn, UINT length, INT shl))

  /* stack contents:
       0x00:     n.a.

     register contents/usage:
       r0: pOut                 format: Q1.31 (FIXP_DBL)
       r1: pIn                  format: Q1.31 (FIXP_DBL)
       r2: length               format: UINT (multiple of 8, unequal 0)
       r3: shl (shift left)     format: INT in range [1..9]

       register usage:
       Q0:  pOut[i+0..3]  4 samples of pOut in format FIXP_DBL, exp = 0
       Q1:  pOut[i+4..7]  4 samples of pOut in format FIXP_DBL, exp = 0
       Q2:  pIn [i+0..3]  4 samples of pin in format FIXP_DBL, exp = shl
       Q3:  pIn [i+4..7]  4 samples of pin in format FIXP_DBL, exp = shl
       Q8:  4x  shl in format INT, exp = 0
  */

  FDK_vdup_q_reg(32, Q8, r3)                   // Q8:  shl     shl     shl     shl

FDK_label(gVBAPRenderer_RenderFrame_Time_func2_loop)
  FDK_vld1_4d(32, D0, D1, D2, D3, r0)          // Q0:  pOut[3] pOut[2] pOut[1] pOut[0]
                                               // Q1:  pOut[7] pOut[6] pOut[5] pOut[4]
  FDK_vld1_4d_ia(32, D4, D5, D6, D7, r1)       // Q2:   pIn[3]  pIn[2]  pin[1]  pIn[0]
                                               // Q3:   pIn[7]  pIn[6]  pIn[5]  pIn[4]
  FDK_subs_imm(r2, r2, 8)                      // length: -= 8
  FDK_vqshl_s32(128, Q2, Q2, Q8)               // Q2: 4x sat(pIn[i+0..3]<<shl)
  FDK_vqshl_s32(128, Q3, Q3, Q8)               // Q3: 4x sat(pIn[i+4..7]<<shl)
  FDK_vqadd_s32_q(Q0, Q0, Q2)                  // Q0: 4x sat(pOut[i+0..3]+sat(pIn[i+0..3]<<shl))
  FDK_vqadd_s32_q(Q1, Q1, Q3)                  // Q0: 4x sat(pOut[i+4..7]+sat(pIn[i+4..7]<<shl))
  FDK_vst1_4d_ia(32, D0, D1, D2, D3, r0)       // store Q0,Q1 via pOut (pOut[0]..pOut[7])
  FDK_branch(NE, gVBAPRenderer_RenderFrame_Time_func2_loop)
FDK_label(gVBAPRenderer_RenderFrame_Time_func2_end)

  FDK_return()

FDK_ASM_ROUTINE_END()
#endif /* #ifdef FUNCTION_gVBAPRenderer_RenderFrame_Time_func2 */

#elif defined(__ARM_AARCH64_NEON__)
#include "arm/FDK_aarch64_neon_funcs.h"
#define FUNCTION_gVBAPRenderer_RenderFrame_Time_func1
#define FUNCTION_gVBAPRenderer_RenderFrame_Time_func2

#ifdef FUNCTION_gVBAPRenderer_RenderFrame_Time_func1
A64_ASM_ROUTINE_START(void, gVBAPRenderer_RenderFrame_Time_func1_neonv8, (
    FIXP_DBL * __restrict pOut, 
    FIXP_DBL * __restrict pIn, 
    UINT64   length_and_start,   /* (length << 32) + startSamplePosition */
    FIXP_DBL scale, 
    FIXP_DBL step, 
    FIXP_DBL scaleState, 
    FIXP_DBL stepState, 
    INT      shlr))  /* positive = shift-left, negative = shift-right */

#ifndef __ARM_AARCH64_NEON__
    X0 = (INT64) pOut;
    X1 = (INT64) pIn;
    X2 = (length << 32) | startSamplePosition;
    W3 = scale;
    W4 = step;
    W5 = scaleState;
    W6 = stepState;
    W7 = shlr;    /* (s < 0) ? shift = (s1 - 1) : (-s1 - 1);  */
#endif
/* stack contents:
     none

   register usage: X0..W7 are call parameters
     X0: pOut
     X1: pIn
     X2: startSamplePosition (iterations of 1st inner loop run)
     W3: scale
     W4: step
     W5: scaleState
     W6: stepState
     W7: shlr  positive is shift-left, negative is shift-right

     X8: length - startSamplePosition (iterations of 2nd inner loop run)
     W9: abs(shlr)

     V0: pOut[i+0..i+3]  4 samples of pOut in format FIXP_DBL
     V1: pOut[i+4..i+7]  4 samples of pOut in format FIXP_DBL
     V2: scale factor    scale+4*step, scale+3*step, scale+2*step, scale+1*step
     V3: 4x step         4 times 4*step in format FIXP_DBL
     V4: pIn[i+0..i+3]   4 samples of pIn in format FIXP_DBL
     V5: pIn[i+4..i+7]   4 samples of pIn in format FIXP_DBL
     V6: 4x shlr
*/
  A64_adds_Wt_asr_imm(W9, W7, WZR, 0)               // W9: shift (for shift-left)
A64_branch(GE, gVBAPRenderer_RenderFrame_Time_func1_shift_ok)
    A64_sub_Wt(W9, WZR, W7)                         // W9: -W7: for shift-right
    A64_mov_Wt_imm(W8, 0x80000000)                  // W8: 0x8000.0000
    A64_lsrv_Wt(W9, W8, W9)                         // W9: 0x8000.0000 >> W9
A64_label(gVBAPRenderer_RenderFrame_Time_func1_shift_ok)


  A64_dup_Wt(32, 128, V6_4S, W9)                    // V6: abs(sh)  abs(sh)  abs(sh)  abs(sh)
  A64_asr_Xt_imm(X8, X2, 32)                        // X8:  length
  A64_and_Xt_imm(X2, X2, 0xFFFF)                    // X2:  startSamplePosition (less than 64k)
  A64_sub_Xt_lsr_imm(X8, X8, X2, 0)                 // X8:  length - startSamplePosition

A64_label(gVBAPRenderer_RenderFrame_Time_func1_outer_loop)
    A64_cmp_Xt_imm(X2, 0)
    A64_branch(EQ, gVBAPRenderer_RenderFrame_Time_func1_inner_loop_end)
                                                    // lane    3      2       1       0
    A64_dup_Wt(32, 128, V2_4S, W5)                  // V2:  4x scaleState, scale
    A64_dup_Wt(32, 128, V0_4S, W6)                  // V0:  4x stepState, step
    A64_add(32, 128, V1_4S, V0_4S, V0_4S)           // V1:  2*step  2*step  2*step  2*step
    A64_add(32, 128, V3_4S, V1_4S, V1_4S)           // V3:  4*step  4*step  4*step  4*step
    A64_zip1(32, 128, V0_4S, V0_4S, V1_4S)          // V0:  2*step  1*step  2*step  1*step
    A64_add(32, 128, V1_4S, V0_4S, V1_4S)           // V1:  4*step  3*step  4*step  3*step
    A64_zip1(64, 128, V0_2D, V0_2D, V1_2D)          // V0:  4*step  3*step  2*step  1*step
    A64_add(32, 128, V2_4S, V2_4S, V0_4S)           // V2: sc+4*st sc+3*st sc+2*st sc+1*step

    A64_cmp_Wt_imm(W7, 0)
    A64_branch(LT, gVBAPRenderer_RenderFrame_Time_func1_inner_loop_shr)

A64_label(gVBAPRenderer_RenderFrame_Time_func1_inner_loop_shl)
      A64_ld1x2_IA(32, 128, V4_4S, V5_4S, X1, 32)   // V4:  pIn[3]  pIn[2]  pIn[1]  pIn[0]  in Q1.31
                                                    // V5:  pIn[7]  pIn[6]  pIn[5]  pIn[4]  in Q1.31
      A64_ld1x2(32, 128, V0_4S, V1_4S, X0)          // V0:  pOut[3] pOut[2] pOut[1] pOut[0] in Q1.31
                                                    // V1:  pOut[7] pOut[6] pOut[5] pOut[4] in Q1.31
      A64_sqdmulh(32, 128, V4_4S, V4_4S, V2_4S)     // V4:  4x fMult(pIn[i],sc+i*step) (i=0,1,2,3)
      A64_add(32, 128, V2_4S, V2_4S, V3_4S)         // V2: sc+4*s, sc+3*s, sc+2*s, sc+1*s all ++4*step
      A64_sqdmulh(32, 128, V5_4S, V5_4S, V2_4S)     // V5:  4x fMult(pIn[i],sc+i*step) (i=4,5,6,7)
      A64_add(32, 128, V2_4S, V2_4S, V3_4S)         // V2: sc+8*s, sc+7*s, sc+6*s, sc+5*s all ++4*step
      A64_sqshl(32, 128, V4_4S, V4_4S, V6_4S)       // V4:  4x fMult(pIn[i],sc+i*step) (i=0,1,2,3) << shl
      A64_sqshl(32, 128, V5_4S, V5_4S, V6_4S)       // V5:  4x fMult(pIn[i],sc+i*step) (i=4,5,6,7) << shl
      A64_add(32, 128, V4_4S, V4_4S, V0_4S)         // V4:  pOut[3] pOut[2] pOut[1] pOut[0] in Q1.31
      A64_add(32, 128, V5_4S, V5_4S, V1_4S)         // V5:  pOut[7] pOut[6] pOut[5] pOut[4] in Q1.31
      A64_subs_imm(X2, X2, 8)                       // X2:  decrement inner loop counter by 8
      A64_st1x2_IA(32, 128, V4_4S, V5_4S, X0, 32)   // store V8,V9 via pOut (pOut[0]..pOut[7])
      A64_branch(NE, gVBAPRenderer_RenderFrame_Time_func1_inner_loop_shl)
      A64_branch(AL, gVBAPRenderer_RenderFrame_Time_func1_inner_loop_end)

A64_label(gVBAPRenderer_RenderFrame_Time_func1_inner_loop_shr)
      A64_ld1x2(32, 128, V0_4S, V1_4S, X0)          // V0:  pOut[3] pOut[2] pOut[1] pOut[0] in Q1.31
                                                    // V1:  pOut[7] pOut[6] pOut[5] pOut[4] in Q1.31
      A64_ld1x2_IA(32, 128, V4_4S, V5_4S, X1, 32)   // V4:  pIn[3]  pIn[2]  pIn[1]  pIn[0]  in Q1.31
                                                    // V5:  pIn[7]  pIn[6]  pIn[5]  pIn[4]  in Q1.31
      A64_subs_imm(X2, X2, 8)                       // X2:  decrement inner loop counter by 8
      A64_sqdmulh(32, 128, V4_4S, V4_4S, V2_4S)     // V4:  4x fMult(pIn[i],sc+i*step) (i=0,1,2,3)
      A64_add(32, 128, V2_4S, V2_4S, V3_4S)         // V2: sc+4*s, sc+3*s, sc+2*s, sc+1*s all ++4*step
      A64_sqdmulh(32, 128, V5_4S, V5_4S, V2_4S)     // V5:  4x fMult(pIn[i],sc+i*step) (i=4,5,6,7)
      A64_add(32, 128, V2_4S, V2_4S, V3_4S)         // V2: sc+8*s, sc+7*s, sc+6*s, sc+5*s all ++4*step

      A64_sqdmulh(32, 128, V4_4S, V4_4S, V6_4S)     // V4:  4x fMult(pIn[i],sc+i*step) (i=0,1,2,3) >> shr
      A64_sqdmulh(32, 128, V5_4S, V5_4S, V6_4S)     // V5:  4x fMult(pIn[i],sc+i*step) (i=4,5,6,7) >> shr
      A64_add(32, 128, V4_4S, V4_4S, V0_4S)         // V4:  pOut[3] pOut[2] pOut[1] pOut[0] in Q1.31
      A64_add(32, 128, V5_4S, V5_4S, V1_4S)         // V5:  pOut[7] pOut[6] pOut[5] pOut[4] in Q1.31
      A64_st1x2_IA(32, 128, V4_4S, V5_4S, X0, 32)   // store V8,V9 via pOut (pOut[0]..pOut[7])
      A64_branch(NE, gVBAPRenderer_RenderFrame_Time_func1_inner_loop_shr)

A64_label(gVBAPRenderer_RenderFrame_Time_func1_inner_loop_end)

    A64_subs_imm(X2, X8, 0)                         // X2: length of 2nd loop iteration
    A64_mov_Xt_imm(X8, 0)                           // X8: length = 0 (exit after 2 inner loops)
    A64_sub_Wt_imm(W5, W3, 0)                       // W5: scaleState = scale
    A64_sub_Wt_imm(W6, W4, 0)                       // W6: stepState  = step
    A64_branch(NE, gVBAPRenderer_RenderFrame_Time_func1_outer_loop)

  A64_return()

A64_ASM_ROUTINE_END()

static void gVBAPRenderer_RenderFrame_Time_func1(
    FIXP_DBL * pOut,
    FIXP_DBL * pIn,
    INT       length,                /* multiple of 4 */
    INT       startSamplePosition,   /* multiple of 4 */
    FIXP_DBL  scale,
    FIXP_DBL  step,
    FIXP_DBL  scaleState,
    FIXP_DBL  stepState,
    INT       s,
    INT       s1)  /* assumes use of fMultDiv2, positive value */
{
    /* Here, we have in total 10 call parameters:
       We pack 2x2 parameters to limit number of parameters to 8. 
       Thus, any use of stack is avoided for calling func1.
    */
    INT64 tmp = ((INT64) length << 32) | (INT64) startSamplePosition;
    INT shlr = (s < 0) ? (s1 - 1) : (-s1 - 1);
    gVBAPRenderer_RenderFrame_Time_func1_neonv8(pOut, pIn, tmp, scale, step, scaleState, stepState, shlr);
}
#endif /* #ifdef FUNCTION_gVBAPRenderer_RenderFrame_Time_func1 */

#ifdef FUNCTION_gVBAPRenderer_RenderFrame_Time_func2
A64_ASM_ROUTINE_START(void, gVBAPRenderer_RenderFrame_Time_func2, (
    FIXP_DBL* pOut,
    const FIXP_DBL* pIn,
    INT length,
    INT shl))  /* shift left saturating */

#ifndef __ARM_AARCH64_NEON__
    X0 = (INT64) pOut;
    X1 = (INT64) pIn;
    W2 = length;
    W3 = shl;
#endif

  A64_dup_Wt(32, 128, V4_4S, W3)

  A64_cmp_Wt_imm(W3, 0)
  A64_branch(NE, gVBAPRenderer_RenderFrame_Time_func2_loop_shl)

A64_label(gVBAPRenderer_RenderFrame_Time_func2_loop)
    A64_ld1x2_IA(32, 128, V2_4S, V3_4S, X1, 32)   // V2:  pIn[3]  pIn[2]  pIn[1]  pIn[0]  in Q1.31
                                                  // V3:  pIn[7]  pIn[6]  pIn[5]  pIn[4]  in Q1.31
    A64_ld1x2(32, 128, V0_4S, V1_4S, X0)          // V0:  pOut[3] pOut[2] pOut[1] pOut[0] in Q1.31
                                                  // V1:  pOut[7] pOut[6] pOut[5] pOut[4] in Q1.31
    A64_sqadd(32, 128, V0_4S, V2_4S, V0_4S)       // V0:  pOut[3] pOut[2] pOut[1] pOut[0] in Q1.31
    A64_sqadd(32, 128, V1_4S, V3_4S, V1_4S)       // V1:  pOut[7] pOut[6] pOut[5] pOut[4] in Q1.31
    A64_subs_imm(W2, W2, 8)                       // W2:  decrement inner loop counter by 8
    A64_st1x2_IA(32, 128, V0_4S, V1_4S, X0, 32)   // store V0,V1 via pOut (pOut[0]..pOut[7])
    A64_branch(NE, gVBAPRenderer_RenderFrame_Time_func2_loop)
  A64_return()

A64_label(gVBAPRenderer_RenderFrame_Time_func2_loop_shl)
    A64_ld1x2_IA(32, 128, V2_4S, V3_4S, X1, 32)   // V2:  pIn[3]  pIn[2]  pIn[1]  pIn[0]  in Q1.31
                                                  // V3:  pIn[7]  pIn[6]  pIn[5]  pIn[4]  in Q1.31
    A64_ld1x2(32, 128, V0_4S, V1_4S, X0)          // V0:  pOut[3] pOut[2] pOut[1] pOut[0] in Q1.31
                                                  // V1:  pOut[7] pOut[6] pOut[5] pOut[4] in Q1.31
    A64_sqshl(32, 128, V2_4S, V2_4S, V4_4S)       // V2:  4x pIn[i] (i=0,1,2,3) << shl
    A64_sqshl(32, 128, V3_4S, V3_4S, V4_4S)       // V3:  4x pIn[i] (i=4,5,6,7) << shl
    A64_sqadd(32, 128, V0_4S, V2_4S, V0_4S)       // V0:  pOut[3] pOut[2] pOut[1] pOut[0] in Q1.31
    A64_sqadd(32, 128, V1_4S, V3_4S, V1_4S)       // V1:  pOut[7] pOut[6] pOut[5] pOut[4] in Q1.31
    A64_subs_imm(W2, W2, 8)                       // W2:  decrement inner loop counter by 8
    A64_st1x2_IA(32, 128, V0_4S, V1_4S, X0, 32)   // store V0,V1 via pOut (pOut[0]..pOut[7])
    A64_branch(NE, gVBAPRenderer_RenderFrame_Time_func2_loop_shl)
  A64_return()

A64_ASM_ROUTINE_END()
#endif /* #ifdef FUNCTION_gVBAPRenderer_RenderFrame_Time_func2 */

#endif  /* defined(__ARM_AARCH64_NEON__) && !defined(__CC_ARM) */
