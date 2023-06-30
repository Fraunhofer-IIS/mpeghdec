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

   Description: STFTprocess subroutines tuned for ARMv7-NEON

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#define FUNCTION_filterbankSineWindowingSTFT_func1
#define FUNCTION_filterbankOverlapAddAudioFrames_func1
#endif

#ifdef FUNCTION_filterbankSineWindowingSTFT_func1
FDK_ASM_ROUTINE_START(void, filterbankSineWindowingSTFT_func1,
   (const FIXP_DBL * RESTRICT audioInputTime,
          FIXP_DBL * RESTRICT audioInputTimePrev,
          FIXP_DBL * RESTRICT audioOutputFreq,
          UINT                fftSize,
    const FIXP_SPK * RESTRICT sinetab))

  /* stack contents:
       0x1C      sinetab
       0x18:     r4
       0x14:     r5
       0x10:     r6
       0x0C:     r7
       0x08:     r8
       0x04:     r9
       0x00:     lr

     register contents:
       r0: audioInputTime       format: pointer to Q1.31
       r1: audioInputTimePrev   format: pointer to Q1.31
       r2: audioOutputFreq      format: pointer to Q1.31
       r3: fftSize              format: UINT (32-bit)

     register usage:
       r0: audioInputTime             used with post-increment ++4 (FIXP_DBL)
       r1: audioInputTimePrev         used with post-increment ++4 (FIXP_DBL)
       r2: audioOutputFreq            used with post-increment ++4 (FIXP_DBL)
       r3: fftSize                    loop counter, decremented by 16 per iteration
       r4: audioInputTime    +fS/2-4  used with post-decrement --4 (FIXP_DBL)
       r5: audioInputTimePrev+fS/2-4  used with post-decrement --4 (FIXP_DBL)
       r6: audioOutputFreq   +fS/2-4  used with post-decrement --4 (FIXP_DBL)
       r7: audioOutputFreq   +fS/2    used with post-increment ++4 (FIXP_DBL)
       r8: audioOutputFreq   +fS  -4  used with post-decrement --4 (FIXP_DBL)
       lr: sinetab                    used with post-increment ++4 (FIXP_SPK)

       Q0:  r0[0..3]      audioInputTime              +i
       Q1:  r4[0..3]      audioInputTime      +fS/2-1 -i
       Q2:  r1[0..3]      audioInputTimePrev          +i
       Q3:  r5[0..3]      audioInputTimePrev  +fS/2-1 -i
       Q4:  ------------------- unused -----------------
       Q5:  ------------------- unused -----------------
       Q6:  ------------------- unused -----------------
       Q7:  ------------------- unused -----------------
       Q8:  r2[0..3]      audioOutputFreq             +i
       Q9:  r6[0..3]      audioOutputFreq     +fS/2-1 -i
       Q10: r7[0..3]      audioOutputFreq     +fS/2   +i
       Q11: r8[0..3]      audioOutputFreq     +fS-1   -i
       Q12: lr[0..3].re   sinetab[i].v.re
       Q13; lr[0..3].im   sinetab[i].v.im
       Q14: ------------------- unused -----------------
       Q15: ------------------- unused -----------------
  */

#ifndef __ARM_NEON__
  const FIXP_DBL *r0 = audioInputTime;
        FIXP_DBL *r1 = audioInputTimePrev;
        FIXP_DBL *r2 = audioOutputFreq;
        INT       r3 = fftSize;
        FIXP_DBL *r4;
        FIXP_DBL *r5;
        FIXP_DBL *r6;
        FIXP_DBL *r7;
        FIXP_DBL *r8;
        INT       r9;
  const FIXP_SPK *lr;
#endif
    FDK_mpush_lr(r4, r9)

    FDK_add_op_lsl(r4, r0, r3, 1, 2)                         // r4: audioInputTime     +fS/2
    FDK_sub_imm(r4, r4, 16, 2)                               // r4: audioInputTime     +fS/2 -4
    FDK_add_op_lsl(r5, r1, r3, 1, 2)                         // r5: audioInputTimePrev +fS/2
    FDK_sub_imm(r5, r5, 16, 2)                               // r5: audioInputTimePrev +fS/2 -4


    FDK_add_op_lsl(r7, r2, r3, 1, 2)                         // r7: audioOutputFreq    +fS/2
    FDK_sub_imm(r6, r7, 16, 2)                               // r6: audioOutputFreq    +fS/2 -4

    FDK_add_op_lsl(r8, r2, r3, 2, 2)                         // r8: audioOutputFreq    +fS
    FDK_sub_imm(r8, r8, 16, 2)                               // r8: audioOutputFreq    +fS - 4

    FDK_mov_imm(r9, -16)

    FDK_ldr(lr, sp, 0x1C, sinetab)                           // lr: sinetab

FDK_label(filterbankSineWindowingSTFT_func1_loop)
    // Q0:  time0 = audioInputTime[j];
    // Q1:  time1 = audioInputTime[fftSize/2-1-j];           // lane    3      2        1      0
    FDK_vld1_2d_ia(32, D0, D1, r0)                           // Q0:   r0[3]   r0[2]   r0[1]   r0[0]  <-- time0 = audioInputTime +i
    FDK_vld1_2d_pu(32, D2, D3, r4, r9)                       // Q1:   r4[3]   r4[2]   r4[1]   r4[0]  <-- time1 = audioInputTime +fS/2-1-i (reversed order)
    FDK_vld1_2d(32, D4, D5, r1)                              // Q2:   r1[3]   r1[2]   r1[1]   r1[0]  <-- audioInputTimePrev +i
    FDK_vld1_2d(32, D6, D7, r5)                              // Q3:   r5[3]   r5[2]   r5[1]   r5[0]  <-- audioInputTimePrev +fS/2-1-i  (reversed order)
    FDK_vld1_2d_ia(32, D24, D25, lr)                         // Q12:    w3      w2      w1      w0   <-- sinetab[i] packed Q1.15x1.15
    FDK_vshll_s16_imm(Q13, D25, 16)                          // Q13:  w3.im   w3.re   w2.im   w2.re  <-- Q1.31
    FDK_vshll_s16_imm(Q12, D24, 16)                          // Q12:  w1.im   w1.re   w0.im   w0.re  <-- Q1.31
    FDK_vuzp_q(32, Q12, Q13)                                 // Q12:  w3.re   w2.re   w1.re   w0.re
                                                             // Q13:  w3.im   w2.im   w1.im   w0.im
    // Q8:  audioOutputFreq[j] = fMult(audioInputTimePrev[j], w.v.im);
    FDK_vqdmulh_s32_qq(Q8,  Q2, Q13)

    // audioInputTimePrev[j] = time0;
    FDK_vst1_2d_ia(32, D0, D1, r1)                           // Q0:   r0[3]   r0[2]   r0[1]   r0[0]  <-- audioInputTimePrev[j] = time0

    // Q10: audioOutputFreq[fftSize/2+j] = fMult(time0, w.v.re);
    FDK_vqdmulh_s32_qq(Q10, Q0, Q12)

    // audioInputTimePrev[fftSize/2-1-j] = time1;
    FDK_vst1_2d_pu(32, D2, D3, r5, r9)                        // Q1:   r4[3]   r4[2]   r4[1]   r4[0]  <-- audioInputTimePrev +fS/2-4+i = tme1 (reversed order)

    // Reverse order of re/im coefficients
    FDK_vswp(64, D24, D25)
    FDK_vst1_2d_ia(32, D16, D17, r2)                          // audioOutputFreq[j]
    FDK_vswp(64, D26, D27)
    FDK_vst1_2d_ia(32, D20, D21, r7)                          // audioOutputFreq[fftSize/2+j]
    FDK_vrev64_q(32, Q12, Q12)                                // Q12:  w0.re   w1.re   w2.re   w3.re
    FDK_vrev64_q(32, Q13, Q13)                                // Q13:  w0.im   w1.im   w2.im   w3.im
    // Q9:  audioOutputFreq[fftSize/2-1-j] = fMult(audioInputTimePrev[fftSize/2-1-j], w.v.re);
    // Q11: audioOutputFreq[fftSize-1-j]   = fMult(time1, w.v.im);
    FDK_vqdmulh_s32_qq(Q9,  Q3, Q12)
    FDK_vqdmulh_s32_qq(Q11, Q1, Q13)

    FDK_subs_imm(r3, r3, 16)

    FDK_vst1_2d_pu(32, D18, D19, r6, r9)                      // audioOutputFreq[fftSize/2-1-j] (reversed order)
    FDK_vst1_2d_pu(32, D22, D23, r8, r9)                      // audioOutputFreq[fftSize-1-j] (reversed order)



    FDK_branch(NE, filterbankSineWindowingSTFT_func1_loop)

    FDK_mpop_pc(r4, r9)
FDK_ASM_ROUTINE_END()

#endif /* #ifdef FUNCTION_filterbankSineWindowingSTFT_func1 */

#ifdef FUNCTION_filterbankOverlapAddAudioFrames_func1
FDK_ASM_ROUTINE_START(void, filterbankOverlapAddAudioFrames_func1,
   (const FIXP_DBL * RESTRICT audioInputTime,
          FIXP_DBL * RESTRICT audioInputTimePrev,
          FIXP_DBL * RESTRICT audioOutputTime,
          UINT                fftSize,
    const FIXP_SPK * RESTRICT sinetab))

  /* stack contents:
       0x1C      sinetab
       0x18:     r4
       0x14:     r5
       0x10:     r6
       0x0C:     r7
       0x08:     r8
       0x04:     r9
       0x00:     lr

     register contents:
       r0: audioInputTime       format: pointer to Q1.31
       r1: audioInputTimePrev   format: pointer to Q1.31
       r2: audioOutputFreq      format: pointer to Q1.31
       r3: fftSize              format: UINT (32-bit)

     register usage:
       r0: audioInputTime             used with post-increment ++4 (FIXP_DBL)
       r1: audioInputTimePrev         used with post-increment ++4 (FIXP_DBL)
       r2: audioOutputTime            used with post-increment ++4 (FIXP_DBL)
       r3: fftSize                    loop counter, decremented by 16 per iteration
       r4: audioInputTime    +fS/2-4  used with post-decrement --4 (FIXP_DBL)
       r5: audioInputTimePrev+fS/2-4  used with post-decrement --4 (FIXP_DBL)
       r6: audioOutputTime   +fS/2-4  used with post-decrement --4 (FIXP_DBL)
       r7: audioInputTime    +fS/2    used with post-increment ++4 (FIXP_DBL)
       r8: audioInputTime    +fS  -4  used with post-decrement --4 (FIXP_DBL)
       lr: sinetab                    used with post-increment ++4 (FIXP_SPK)

       Q0:  r0[0..3]      audioInputTime              +i
       Q1:  r4[0..3]      audioInputTime      +fS/2-1 -i
       Q2:  r1[0..3]      audioInputTimePrev          +i
       Q3:  r5[0..3]      audioInputTimePrev  +fS/2-1 -i
       Q4:  ------------------- unused -----------------
       Q5:  ------------------- unused -----------------
       Q6:  ------------------- unused -----------------
       Q7:  ------------------- unused -----------------
       Q8:  r2[0..3]      audioOutputTime             +i
       Q9:  r6[0..3]      audioOutputTime     +fS/2-1 -i
       Q10: r7[0..3]      audioInputTime      +fS/2   +i
       Q11: r8[0..3]      audioInputTime      +fS-1   -i
       Q12: lr[0..3].re   sinetab[i].v.re
       Q13; lr[0..3].im   sinetab[i].v.im
       Q14: ------------------- unused -----------------
       Q15: ------------------- unused -----------------
  */



#ifndef __ARM_NEON__
  const FIXP_DBL *r0 = audioInputTime;
        FIXP_DBL *r1 = audioInputTimePrev;
        FIXP_DBL *r2 = audioOutputTime;
        INT       r3 = fftSize;
  const FIXP_DBL *r4;
        FIXP_DBL *r5;
        FIXP_DBL *r6;
  const FIXP_DBL *r7;
  const FIXP_DBL *r8;
        INT       r9;
  const FIXP_SPK *lr;
#endif
    FDK_mpush_lr(r4, r9)

    FDK_add_op_lsl(r7, r0, r3, 1, 2)                   // r7: audioInputTime     +fS/2
    FDK_sub_imm(r4, r7, 16, 2)                         // r4: audioInputTime     +fS/2 -4
    FDK_add_op_lsl(r8, r4, r3, 1, 2)                   // r8: audioInputTime     +fS   -4

    FDK_add_op_lsl(r5, r1, r3, 1, 2)                   // r5: audioInputTimePrev +fS/2
    FDK_sub_imm(r5, r5, 16, 2)                         // r5: audioInputTimePrev +fS/2 -4

    FDK_add_op_lsl(r6, r2, r3, 1, 2)                   // r6: audioOutputTime    +fS/2
    FDK_sub_imm(r6, r6, 16, 2)                         // r6: audioOutputTime    +fS/2 -4

    FDK_mov_imm(r9, -16)

    FDK_ldr(lr, sp, 0x1C, sinetab)                     // lr: sinetab

FDK_label(filterbankOverlapAddAudioFrames_func1_loop)  // lane    3      2        1      0
    FDK_vld1_2d_ia(32, D0, D1, r0)                     // Q0:   r0[3]   r0[2]   r0[1]   r0[0]  <-- time0 = audioInputTime[j]
    FDK_vld1_2d_pu(32, D2, D3, r4, r9)                 // Q1:   r4[3]   r4[2]   r4[1]   r4[0]  <-- time1 = audioInputTime[fS/2-1-j]
    FDK_vld1_2d(32, D4, D5, r1)                        // Q2:   r1[3]   r1[2]   r1[1]   r1[0]  <-- audioInputTimePrev[j]
    FDK_vld1_2d(32, D6, D7, r5)                        // Q3:   r5[3]   r5[2]   r5[1]   r5[0]  <-- audioInputTimePrev[fS/2-1-j]
    FDK_vld1_2d_ia(32, D24, D25, lr)                   // Q12:    w3      w2      w1      w0   <-- sinetab[i] packed Q1.15x1.15
    FDK_vshll_s16_imm(Q13, D25, 16)                    // Q13:  w3.im   w3.re   w2.im   w2.re  <-- Q1.31
    FDK_vshll_s16_imm(Q12, D24, 16)                    // Q12:  w1.im   w1.re   w0.im   w0.re  <-- Q1.31
    FDK_vuzp_q(32, Q12, Q13)                           // Q12:  w3.re   w2.re   w1.re   w0.re
                                                       // Q13:  w3.im   w2.im   w1.im   w0.im
    FDK_vld1_2d(32, D16, D17, r2)                      // Q8:   r2[3]   r2[2]   r2[1]   r2[0]  <--  audioOutputTime[j]
    FDK_vld1_2d(32, D18, D19, r6)                      // Q9:   r6[3]   r6[2]   r6[1]   r6[0]  <--  audioOutputTime[fS/2-1-j]
    FDK_vld1_2d_ia(32, D20, D21, r7)                   // Q10:  r7[3]   r7[2]   r7[1]   r7[0]  <--  audioInputTime[fs/2+j]
    FDK_vld1_2d_pu(32, D22, D23, r8, r9)               // Q11:  r8[3]   r8[2]   r8[1]   r8[0]  <--  audioInputTime[fs-1-j]

    FDK_vshl_s32_q_imm(Q0,  Q0,  8)                    // Q0:  audioInputTime[j] << 8
    FDK_vshl_s32_q_imm(Q1,  Q1,  8)                    // Q1:  audioInputTime[fs/2-1-j] << 8
    FDK_vshl_s32_q_imm(Q10, Q10, 8)                    // Q10: audioInputTime[sf/2+j] << 8
    FDK_vshl_s32_q_imm(Q11, Q11, 8)                    // Q11: audioInputTime[sf-1-j] << 8

    FDK_vqdmulh_s32_qq(Q0,  Q0,  Q13)                  // Q0:  fMult(audioInputTime[j] << 8, w.v.im)
    FDK_vqdmulh_s32_qq(Q10, Q10, Q12)                  // Q10: fMult(audioInputTime[fs/2+j] << 8, w.v.re)

    FDK_vswp(64, D24, D25)                             // Reverse order of real/imag coefficients
    FDK_vswp(64, D26, D27)
    FDK_vrev64_q(32, Q12, Q12)                         // Q12:  w0.re   w1.re   w2.re   w3.re
    FDK_vrev64_q(32, Q13, Q13)                         // Q13:  w0.im   w1.im   w2.im   w3.im

    FDK_vqdmulh_s32_qq(Q1,  Q1,  Q12)                  // Q1:  fMult(audioInputTime[fs/2-1-j] << 8,w.v.re)
    FDK_vqdmulh_s32_qq(Q11, Q11, Q13)                  // Q11: Mult(audioInputTime[fs-1-j] << 8,w.v.im)

    FDK_vst1_2d_ia(32, D20, D21, r1)                   // Q10: audioInputTimePrev[j] = fMult(audioInputTime[fs/2+j] << 8, w.v.re)

    FDK_vadd_s32_q(Q8, Q8, Q2)                         // Q8:  audioOutputTime[j] + audioInputTimePrev[j]
    FDK_vadd_s32_q(Q9, Q9, Q3)                         // Q9:  audioOutput[fftSize/2-1-j] + audioInputTimePrev[fs/2-1-j];

    FDK_vst1_2d_pu(32, D22, D23, r5, r9)               // Q11: audioInputTimePrev[fS/2-1-j] = fMult(audioInputTime[fs-1-j] << 8, w.v.im)

    FDK_vadd_s32_q(Q8, Q8, Q0)                         // Q8:  audioOutputTime[j] + audioInputTimePrev[j] + fMult(audioInputTime[j] << 8, w.v.im)
    FDK_vadd_s32_q(Q9, Q9, Q1)                         // Q9:  audioOutput[fftSize/2-1-j] + audioInputTimePrev[fs/2-1-j] + fMult(audioInputTime[fs/2-1-j] << 8,w.v.re)

    FDK_vst1_2d_ia(32, D16, D17, r2)                   // Q8:  audioOutputTime[j] += audioInputTimePrev[j] + fMult(audioInputTime[j] << 8, w.v.im)
    FDK_vst1_2d_pu(32, D18, D19, r6, r9)               // Q9:  audioOutput[fftSize/2-1-j] += audioInputTimePrev[fs/2-1-j] + fMult(audioInputTime[fs/2-1-j] << 8,w.v.re)

    FDK_subs_imm(r3, r3, 16)
    FDK_branch(NE, filterbankOverlapAddAudioFrames_func1_loop)

    FDK_mpop_pc(r4, r9)
FDK_ASM_ROUTINE_END()

#endif /* #ifdef FUNCTION_filterbankOverlapAddAudioFrames_func1 */
