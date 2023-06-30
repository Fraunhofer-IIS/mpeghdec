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

/************************* MPEG-H 3DA decoder library **************************

   Author(s):   Arthur Tritthart

   Description: TCX/AAC Frequency Domain Prediction - ARMv7-NEON routines

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#define FUNCTION_FDP_DecodeBins_func1
#endif

#if defined(FUNCTION_FDP_DecodeBins_func1)
FDK_ASM_ROUTINE_START(int, FDP_DecodeBins_func1,
   (const INT   * __restrict quantSpecCurr,          /* r0  */
          SHORT * __restrict quantSpecPrev1,         /* r1  */
          SHORT * __restrict quantSpecPrev2,         /* r2  */
    const SHORT * __restrict sfbOffsets,             /* r3  */
          INT       sfb,
    const INT       scf16,                          /* in range 0...65536 */
    const INT       harmonicSpacing,
          FIXP_DBL *  __restrict fdp_int,
    const USHORT   *  __restrict fdp_exp))

  /* stack contents:
       0x24:   fdp_exp
       0x20:   fdp_int
       0x1C:   harmonicSpacing
       0x18:   scf16
       0x14:   sfb
       0x10:   r4
       0x0C:   r5
       0x08:   r6
       0x04:   r7
       0x00:   lr

     register contents:
       r0:  quantSpecCurr    INT *
       r1:  quantSpecPrev1   SHORT *
       r2:  quantSpecPrev2   SHORT *
       r3:  sfbOffsets       SHORT *

     register usage:
       r0:  sfbOffsets[sfb+1]   <-- return value
       r1:  quantSpecPrev1   SHORT *
       r2:  quantSpecPrev2   SHORT *
       r3:  sfbOffsets       SHORT *
       r3:  &fdp_exp[x_int]
       r4:  loop counter
       r5:  quantSpecCurr    INT *
       r6:  sfb              INT
       r7:  scf16            INT
       r6:  harmonicSpacing  INT
       r7:  fdp_int          INT *
       lr:  &fdp_exp[x_int]

       Q0:                quantSpecCurr[i+0..3] (INT)
       D2:               quantSpecPrev1[i+0..3] (SHORT)
       Q2:                    x_int[i+0..3
       Q3:
       Q4:  ------------------- unused -----------------
       Q5:  ------------------- unused -----------------
       Q6:  ------------------- unused -----------------
       Q7:  ------------------- unused -----------------
       Q8:  ------------------- unused -----------------
       Q9:      512       512          512       512
       Q10:  fpd_exp   fpd_exp      fpd_exp   fpd_exp
       Q11:     181       181          181       181
       Q12:    scf16     scf16        scf16     scf16
       Q13:     -10       -10          -10       -10
       Q14:    31775     31775        31775     31775
       Q15;   -31775    -31775       -31775    -31775
  */
  FDK_mpush_lr(r4,r7)
  FDK_ldrd(r4, r5, sp, 0x14, sfb, scf16)
  FDK_ldr(r7, sp, 0x24, fdp_exp)                           // r7:   fdp_exp
  FDK_vmov_i32(128, Q9, 512)                               // Q9:   512
  FDK_vdup_q_reg(32, Q10, r7)                              // Q10:  fdp_exp
  FDK_vmov_i32(128, Q11, 181)                              // Q11:  181
  FDK_vdup_q_reg(32, Q12, r5)                              // Q12:  scf16
  FDK_vmvn_i32(128, Q13,  0x00000009)                      // Q13: -10
  FDK_vmov_i32( 32, Q14,  0x00007C00)                      // Q14:  0x00007C00
  FDK_vmov_i32( 32, Q15,  0x0000001F)                      // Q15:  0x0000001F
  FDK_vadd_s32_q(Q14, Q14, Q15)                            // Q14:  0x00007C1F (31775)
  FDK_vneg_q(32, Q15, Q14)                                 // Q15:  -31775
  FDK_add_op_lsl(r3, r3, r4, 1, 1)                         // r3:   &sfbOffset[0] -> &sfbOffset[sfb]

  FDK_ldrh(r4, r3, 0, sfbOffset[sfb+0])                    // r4:  i = sfbOffset[sfb+0]
  FDK_add_op_lsl(r5, r0, r4, 2, 2)                         // r5:  &quantSpecCurr[i]
  FDK_ldrh(r0, r3, 2, sfbOffset[sfb+1])                    // r0:  sfbOffset[sfb+1]
  FDK_ldrd(r6, r7, sp, 0x1C, harmonicSpacing, fdp_int)
  FDK_add_op_lsl(r1, r1, r4, 1, 1)                         // r1:  &quantSpecPrev1[i]
  FDK_add_op_lsl(r2, r2, r4, 1, 1)                         // r2:  &quantSpecPrev2[i]
  FDK_add_op_lsl(r7, r7, r4, 2, 2)                         // r7:  &fdp_int[i]
  FDK_sub(r4, r0, r4)                                      // r4:  sfbOffset[sfb+1] - sfbOffset[sfb]  <-- loop counter

FDK_label(FDP_DecodeBins_func1_loop)
    FDK_vld1_2d_ia(32, D0, D1, r5)                         // Q0:  quantSpecCurr[i+0..3]
    FDK_vabs_q(32, Q2, Q0)                                 // Q2:  fAbs(quantSpecCurr[i+0..3])
    FDK_vmin_s32(128, Q2, Q2, Q11)                         // Q2:  fMin(fAbs(quantSpecCurr[i+0..3]), 181)
    FDK_vshl_s32_q_imm(Q2, Q2, 1)                          // Q2:  fMin(fAbs(quantSpecCurr[i+0..3]), 181) << 1 (for USHORT index into fdp_exp)
    FDK_vadd_s32_q(Q3, Q2, Q10)                            // Q3:  &fdp_exp[fMin(fAbs((INT)quantSpecCurr[i+0..3]), 181)]
    FDK_vshr_s32_q_imm(Q0, Q0, 31)                         // Q0:  sign(quantSpecCurr[i+0..3])
    FDK_vmov_i32(128, Q2, 0)                               // Q2:  cleared
    FDK_vld1_1d(16, D2, r1)                                // D2:  quantSpecPrev1[i+0..3]
    FDK_vmov_dreg(r3, lr, s12, s13)                        // r3:  fMin(fAbs(quantSpecCurr[i+0]), 181), lr: fMin(fAbs(quantSpecCurr[i+1]), 181)
    FDK_vld1_ia(16, D4_0, r3)                              // Q2:  0000 0000    0000 0000    0000 0000    0000 x_int0
    FDK_vld1_ia(16, D4_2, lr)                              // Q2:  0000 0000    0000 0000    0000 x_int1  0000 x_int0
    FDK_vmov_dreg(r3, lr, s14, s15)                        // r3:  fMin(fAbs(quantSpecCurr[i+2]), 181), r6: fMin(fAbs(quantSpecCurr[i+3]), 181)
    FDK_vld1_ia(16, D5_0, r3)                              // Q2:  0000 0000    0000 xint2   0000 xint1   0000 xint0
    FDK_vld1_ia(16, D5_2, lr)                              // Q2:  0000 xint3   0000 xint2   0000 xint1   0000 xint0

    FDK_vmul_s32_q(Q2, Q2, Q12)                            // Q2: {0000 xint3   0000 xint2   0000 xint1   0000 xint0} * scf16
    FDK_vst1_1d_ia(16, D2, r2)                             // D2:  quantSpecPrev2[i+0..3]  = quantSpecPrev1[i+0..3]
    FDK_vadd_s32_q(Q2, Q2, Q9)                             // Q2: {0000 xint3   0000 xint2   0000 xint1   0000 xint0} * scf16 + 512
    FDK_vshr_s32_q_imm(Q2, Q2, 10)                         // Q2:({0000 xint3   0000 xint2   0000 xint1   0000 xint0} * scf16 + 512) >> 10
    FDK_veor(128, Q2, Q2, Q0)                              // Q2: x_int[i] ^ quantSpecCurr[i] >> 31    <--- One's complement
    FDK_vsub_s32_q(Q2, Q2, Q0)                             // Q2: -x_int[i], if(quantSpecCurr[i] < 0)  <--- Two's complement
    FDK_cmp_imm(r6, 0)                                     // r6:  harmonicSpacing != 0 ?
    FDK_branch(EQ, FDP_DecodeBins_func1_harmonicSpacing_done)
      FDK_vld1_2d_ia(32, D6, D7, r7)                       // Q3: fdp_int[i+0..3]
      FDK_vadd_s32_q(Q2, Q2, Q3)                           // Q2: x_int[i] += fdp_int[i]
FDK_label(FDP_DecodeBins_func1_harmonicSpacing_done)
    FDK_subs_imm(r4, r4, 4)                                // r4:  loop counter decrement by 4
    FDK_vmax_s32(128, Q2, Q2, Q15)                         // Q2:  x_int = fMax(x_int, -31775)    format: INT
    FDK_vmin_s32(128, Q2, Q2, Q14)                         // Q8:  x_int = fMin(x_int,  31775)    format: INT
    FDK_vuzp_d(16, D4, D5)                                 // D4:  x_int                          format: USHORT
    FDK_vst1_1d_ia(16, D4, r1)                             // D4: quantSpecPrev1[i+0..3] = x_int[0..3]
    FDK_branch(NE, FDP_DecodeBins_func1_loop)
  FDK_mpop_pc(r4,r7)
FDK_ASM_ROUTINE_RETURN(int)

#endif
