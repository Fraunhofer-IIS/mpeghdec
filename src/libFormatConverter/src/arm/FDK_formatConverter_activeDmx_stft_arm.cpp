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

/******************** MPEG-H 3DA channel rendering library *********************

   Author(s):

   Description:

*******************************************************************************/

/***********************************************************************************

 This software module was originally developed by

 Alexander Adami (AudioLabs) AND Fraunhofer IIS

 in the course of development of the ISO/IEC 23008-3 for reference purposes and its
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard
 and which satisfy any specified conformance criteria. Those intending to use this
 software module in products are advised that its use may infringe existing patents.
 ISO/IEC have no liability for use of this software module or modifications thereof.
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3
 standard.

 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using
 the code for products that do not conform to MPEG-related ITU Recommendations and/or
 ISO/IEC International Standards.

 This copyright notice must be included in all copies or derivative works.

 Copyright (c) ISO/IEC 2015.

 ***********************************************************************************/

/* clang-format off */
#if defined(__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#define FUNCTION_activeDmxProcess_STFT_func1
#define FUNCTION_activeDmxProcess_STFT_func2
#define FUNCTION_activeDmxProcess_STFT_func3
#endif

#ifdef FUNCTION_activeDmxProcess_STFT_func1

FDK_ASM_ROUTINE_START(void, activeDmxProcess_STFT_func1,
   (FIXP_DBL *inputBuffer,       /* r0 */
    FIXP_DBL *realizedSig,       /* r1 */
    FIXP_DBL *targetEneArr,      /* r2 */
    UINT *erbFreqIdx,            /* r3 */
    FIXP_DBL *eq_ptr,
    FIXP_DMX_H dmxMatrixL_FDK,
    FIXP_DMX_H dmxMatrixH_FDK,
    UINT erb_is4GVH_L,
    UINT erb_is4GVH_H,
    INT chOut_exp,
    INT dmx_iterations,
    INT inBufStftHeadroom,
    INT *erb_freq_idx_256_58_exp))

#ifndef __ARM_NEON__
    FIXP_DBL        * r0 = inputBuffer;
    FIXP_DBL        * r1 = realizedSig;
    FIXP_DBL        * r2 = targetEneArr;
    UINT            * r3 = erbFreqIdx;
    FIXP_DBL          r4;  /* = savIm0 = inputBuffer[1] */
    FIXP_DBL        * r5;  /* = eq_ptr */
    FIXP_DMX_H        r6;  /* = dmxMatrixL_FDK;  dmxMatrixH_FDK; */
    UINT              r7;  /* = erb_is4GVH_L; */
    UINT              r8;  /* = erb_is4GVH_H; */
    INT               r9;  /* = chOut_exp; */
    INT               r10; /* = dmx_iterations; */
    INT               r11; /* = inBufStftHeadroom; */
    INT             * r12; /* = erb_freq_idx_256_58_exp; */
    FIXP_DBL          savIm0 = 0;
    FIXP_DBL          lr;
#endif

    /*
     NEON register layout:
     lane:       3         2         1         0
     ----------------------------------------------
     Q15:        r0        r0        r1        r1
     Q14:         1         1         1         1   LOWPRECISIONMASK
     Q13:  targetE3  targetE2  targetE1  targetE0
     Q12:  -chOut_exp-target_exp                   (duplicated)
     Q11:  -chOut_exp                              (duplicated)
     Q10:  inBufStftHeadroom                       (duplicated)
     Q9:   rlSigIm3  rlSigIm2  rlSigIm1  rlSigIm0  realizedSig, imag parts
     Q8:   rlSigRe3  rlSigRe2  rlSigRe1  rlSigRe0  realizedSig, real parts
     Q7:   reserved  reserved  reserved  reserved
     Q6:   reserved  reserved  reserved  reserved
     Q5:   reserved  reserved  reserved  reserved
     Q4:   reserved  reserved  reserved  reserved
     Q3:    buf_im3   buf_im2   buf_im1   buf_im0
     Q2:    buf_re3   buf_re2   buf_re1   buf_re0
     Q1:    eq_ptr3   eq_ptr2   eq_ptr1   eq_ptr0
     Q0:   dmx_coeff_mtx                           (duplicated, modified per iteration)

     Core register usage:
     r0: inputBuffer            FIXP_DBL*
     r1: realizedSig            FIXP_DBL*
     r2: targetEneArr           FIXP_DBL*
     r3: erbFreqIdx             UINT*
     r4: savIm0                 FIXP_DBL
     r5: eq_ptr                 FIXP_DBL*
     r6: it                     INT         loop counter for iterations: 0,1,2
     r7: diverse stack variable, constants
     r8: erb_is4GVH_L, maxErb   UINT        loop counter for erb
     r9: erb_is4GVH_H           UINT
    r10: FftBand                UINT        erbFreqIdx[erb+0]
    r11: maxFftBand             UINT        erbFreqIdx[erb+1]
    r12: erb_freq_idx_256_58_exp INT*
     lr: fftBand                UINT        loop counter for fftBands: (r11-r10), ...0

     Stack contents:
     0x48:     erb_freq_idx_256_58_exp
     0x44:     inBufStftHeadroom
     0x40:     dmx_iterations
     0x3C:     chOut_exp
     0x38:     erb_is4GVH_H
     0x34:     erb_is4GVH_L
     0x30:     dmxMatrixH_FDK
     0x2C:     dmxMatrixL_FDK
     0x28:     eq_ptr
     0x24:     r4
     0x20:     r5
     0x1C:     r6
     0x18:     r7
     0x14:     r8
     0x10:     r9
     0x0C:     r10
     0x08:     r11
     0x04:     r12
     0x00:     lr
  */

    FDK_mpush_lr(r4,r12)

    /* Preprocessing */
    FDK_vmov_i32(128, Q14, 1)                                           /* Q14 : 1  1  1  1                                    */

    FDK_ldr(r4, r0, 4, inputBuffer[1])                                  /* r4 = savIm0 = inputBuffer[1] */
    FDK_mov_imm(lr, 0)
    FDK_ldr(r5, sp, 0x28, eq_ptr)                                       /* r5 = *eq_ptr */
    FDK_str(lr, r0, 4, inputBuffer[1])                                  /* inputBuffer[1] = FIXP_DBL(0) */
    FDK_vdup_d_reg(32, D30, r1)                                         /* S60/S61: &realizedSig[0] */
    FDK_vdup_d_reg(32, D31, r0)                                         /* S62/S63: &inputBuffer[0] */

    FDK_ldr( r6, sp, 0x2C, dmxMatrixL_FDK)                              /* r6 = dmx_coeff_mtx = dmxMatrixL_FDK */
    FDK_ldrd(r8, r9, sp, 0x34, erb_is4GVH_L, erb_is4GVH_H)              /* r8 = max_erb = erb_is4GVH_L || r9 = *erb_is4GVH_H */
    FDK_ldr(r10, sp, 0x44, inBufStftHeadroom)                           /* r10 = inBufStftHeadroom */
    FDK_ldr(r11, sp, 0x3C, chOut_exp)                                   /* r11 = chOut_exp[chOut] */
    FDK_ldr(r12, sp, 0x48, erb_freq_idx_256_58_exp)                     /* r12 = erb_freq_idx_256_58_exp */
    FDK_vdup_q_reg(32, Q10, r10)                                        /* Q10: inBufStftHeadroom inBufStftHeadroom inBufStftHeadroom inBufStftHeadroom, r10 free to use again */
    FDK_vdup_q_reg(32, Q11, r11)                                        /* Q11: chOut_exp[chOut] chOut_exp[chOut] chOut_exp[chOut] chOut_exp[chOut] */
    FDK_vdup_d_reg(16, D0, r6)                                          /* D0: dmx_coeff_mtx dmx_coeff_mtx, r6 free to use again */
    FDK_vneg_q(32, Q11, Q11)                                            /* Q11: -chOut_exp[chOut] -chOut_exp[chOut] -chOut_exp[chOut] -chOut_exp[chOut] */
    FDK_vshll_s16_imm(Q0, D0, 16)                                       /* Q0: dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx */

     /* Take into account the first lr*4 iterations */
    FDK_lsr_imm(lr, r8, 2)                                              /* lr: Loop counter for first max 32 erb */
    FDK_cmp_imm(lr, 0x8)
    FDK_mov_cond_imm(GT, lr, 8)                                         /* lr: maximum 8*4 samples optimized erb iterations */
    FDK_lsl_imm(r10, lr, 2)                                             /* r10: save number of first optimized erb iterations */
    FDK_add_op_lsl(r12, r12, r10, 2, 2)                                 /* r12: &erb_freq_idx_256_58_exp[4*lr]  ->Skip reading (lr*4)x 0 from array */
    FDK_add_op_lsl(r3,  r3,  r10, 2 ,2)                                 /* r3 = r3 + r10 (erbFreqIdx)*/
    FDK_sub(r8, r8, r10)                                                /* r8 = max_erb - r10 since we will proces the first r10 pairs */

    FDK_cmp_imm(lr, 0x0)
    FDK_branch(EQ, activeDmxProcess_STFT_func1_first32Samples_end)


/* First 32 iterations of erb (second loop) have (maxfftBand - fftband) == 1 -> We are processing them manually */
FDK_label(activeDmxProcess_STFT_func1_first32Samples)
    /* Processing 4 complex pairs */

    FDK_vld1_2d_ia(32, D2, D3, r5)                                      /* Q1: eq_ptr[erb+3] eq_ptr[erb+2] eq_ptr[erb+1] eq_ptr[erb+0] */
    FDK_vld2_4d_ia(32, D4, D5, D6, D7, r0)                              /* Q2: bufRe3        bufRe2        bufRe1        bufRe0        */
                                                                        /* Q3: bufIm3        bufIm2        bufIm1        bufIm0        */
    FDK_vld2_4d(32, D16, D17, D18, D19, r1)                             /* Q8: SigRe3        SigRe2        SigRe1        SigRe0        */
                                                                        /* Q9: SigIm3        SigIm2        SigIm1        SigIm0        */
    FDK_vqdmulh_s32_qq(Q1, Q0, Q1)                                      /* Q1: dmx_coeff3    dmx_coeff2    dmx_coeff1    dmx_coeff0    */

    FDK_vbic(128, Q1, Q1, Q14)

    FDK_vqdmulh_s32_qq(Q2, Q2, Q1)                                      /* Q2: bufRe3*c3     bufRe2*c2     bufRe1*c1      bufRe0*c0    */
    FDK_vqdmulh_s32_qq(Q3, Q3, Q1)                                      /* Q2: bufIm3*c3     bufIm2*c2     bufIm1*c1      bufIm0*c0    */

    FDK_vbic(128, Q2, Q2, Q14)
    FDK_vbic(128, Q3, Q3, Q14)

    FDK_vadd_s32_q(Q8, Q8, Q2)                                          /* realizedSig_chOut[fftBand*2+0] += tmpRe ... */
    FDK_vadd_s32_q(Q9, Q9, Q3)                                          /* realizedSig_chOut[fftBand*2+1] += tmpIm ... */

    FDK_vshl_s32_q(Q2, Q2, Q10)                                         /* tmpRe << inBufStftHeadroom ... */
    FDK_vst2_4d_ia(32, D16, D17, D18, D19, r1)                          /* Store Q8, Q9 with pointer update */
    FDK_vshl_s32_q(Q3, Q3, Q10)                                         /* tmpIm << inBufStftHeadroom ... */

    FDK_vqdmulh_s32_qq(Q2, Q2, Q2)                                      /* fPow2Div2(...) */
    FDK_vqdmulh_s32_qq(Q3, Q3, Q3)                                      /* fPow2Div2(...) */

    FDK_vbic(128, Q2, Q2, Q14)
    FDK_vbic(128, Q3, Q3, Q14)

    FDK_vhadd_s32_q(Q2, Q2, Q3)                                         /* Half-add of re and im */
    FDK_vshl_s32_q(Q2, Q2, Q11)                                         /* result >> target_exp */

    FDK_vld1_2d(32, D16, D17, r2)                                       /* Q8: targetEne3 targetEne2 targetEne1 targetEne0 */

    FDK_vadd_s32_q(Q8, Q8, Q2)                                          /* Add the calculated energies */
    FDK_subs_imm(lr, lr, 1)                                             /* Update loop counter */
    FDK_vst1_2d_ia(32, D16, D17, r2)                                    /* Store targetEne's with pointer update */

    FDK_branch(NE, activeDmxProcess_STFT_func1_first32Samples)          /* Repeat 8 times */
/* Done processing the very first 32 samples */
FDK_label(activeDmxProcess_STFT_func1_first32Samples_end)

    FDK_mov_imm(r6, 0)                                                  /* r6 = it = 0 */

    FDK_cmp_imm(r8, 0)
    FDK_branch(EQ, activeDmxProcess_STFT_func1_loop_erb_end)


/* Outermost loop with dmx_iterations */
FDK_label(activeDmxProcess_STFT_func1_loop_dmx_iterations)

FDK_label(activeDmxProcess_STFT_func1_loop_erb)                         /* Second loop with erb index */
    FDK_ldr_ia(r11, r3, 4)                                              /* r11: maxfftBand -> *h->erbFreqIdx[erb] */
    FDK_vld1dup_1d_ia(32, D2, r5)                                       /* D2: eq_ptr[erb] eq_ptr[erb] */
    FDK_sub(lr, r11, r10)                                               /* lr: fftBands to process */
    FDK_vqdmulh_s32_dd(D2, D0, D2)                                      /* D2: dmx_coeff dmx_coeff */
    FDK_mov_reg(r10, r11)                                               /* Save old maxfftBand to r10 */

    FDK_vld1dup_2d_ia(32, D24, D25, r12)                                /* Q12:  target_exp                                    */
    FDK_vsub_s32_q(Q12, Q11, Q12)                                       /* Q12: -chOut_exp--target_exp                         */

//FDK_label(activeDmxProcess_STFT_func1_loop_maxfftBands)
    FDK_vmov_i32(128, Q13, 0)                                           /* Zero init registers where result is stored */

    FDK_movs_asr_imm(lr, lr, 1)                                         /* Check if loop length is odd or even and lr >> 1 */
    FDK_branch(CC, activeDmxProcess_STFT_func1_loop_maxfftBands_2ComplexSamples)

    /* Process one complex sample if length is odd */
    /* Energy calculation */
    FDK_vld1_1d_ia(32, D4, r0)                                          /* D4:  bufIm             bufRe           */
    FDK_vld1_1d(32, D16, r1)                                            /* D16: realizedSigIm0    realizedSigRe0  */

    FDK_vqdmulh_s32_ds(D4, D4, S4)                                      /* D4:  bufIm*c           bufRe*c         */

    FDK_vbic(64, D4, D4, D28)

    FDK_vadd_s32_d(D16, D16, D4)                                        /* D16  realizedSigIm0    realizedSigRe0  */
    FDK_vst1_1d_ia(32, D16, r1)                                         /* Store realizedSig with pointer update  */
    FDK_vshl_s32_d(D4, D4, D20)                                         /* D4:  t mpRe/Im << inBufStftHeadroom ... */
    FDK_vqdmulh_s32_dd(D4, D4, D4)                                      /* D4:  fPow2Div2(tmpIm)  fPowDiv2(tmpRe) */
    FDK_vbic(64, D4, D4, D28)

    FDK_vrev64_d(32, D5, D4)                                            /* D5:  fPow2Div2(tmpRe)  fPowDiv2(tmpIm) */
    FDK_vhadd_s32_d(D4, D4, D5)                                         /* D4: (tmpRe^2+tmpIm^2)  (tmpRe^2+tmpIm^2) */
    FDK_vpadd_s32(D4, D4, D26)
    FDK_vshl_s32_d(D26, D4, D24)                                        /* D26: 0 | ( tmpRe^2 + tmpIm^2 ) >>  t   */
    FDK_vshr_s32_d_imm(D26, D26, 1)
    FDK_branch(EQ, activeDmxProcess_STFT_func1_loop_fft_end)            /* Finish, if we had only one iteration   */

    /* Continue with loop with two complex samples */
FDK_label(activeDmxProcess_STFT_func1_loop_maxfftBands_2ComplexSamples)
    FDK_subs_imm(lr, lr, 1)                                             /* lr = lr - 1 */

    /* Energy calculation */
    FDK_vld1_2d_ia(32, D4, D5, r0)                                      /* Q2: bufIm1 bufRe1 bufIm0 bufRe0 */
    FDK_vld1_2d(32, D16, D17, r1)                                       /* Q8: realizedSigIm1 realizedSigRe1 realizedSigIm0 realizedSigRe0 */
    FDK_vqdmulh_s32_qs(Q2, Q2, S4)                                      /* Q2:[bufIm1 bufRe1 bufIm0 bufRe0] * dmxCoeff */

    FDK_vbic(128, Q2, Q2, Q14)

    FDK_vadd_s32_q(Q8, Q8, Q2)                                          /* Q8: Add calculated values to realizedSig */
    FDK_vst1_2d_ia(32, D16, D17, r1)                                    /* Q8: Store realizedSig with pointer update */
    FDK_vshl_s32_q(Q2, Q2, Q10)                                         /* Q2: tmpRe/Im << inBufStftHeadroom ... */
    FDK_vqdmulh_s32_qq(Q2, Q2, Q2)                                      /* Q2: fPow2Div2(...) */
    FDK_vbic(128, Q2, Q2, Q14)

    FDK_vzip_d(32, D4, D5)                                              /* Q2: pow2(im1)  pow2(im0)  pow2(re1)  pow2(re0) */

    FDK_vhadd_s32_d(D4, D4, D5)                                         /* Q2: First Add re+im */
    FDK_vshl_s32_d(D4, D4, D24)                                         /* D4:  energy of fftband[2*i] and fftband[2*i+1] */
    FDK_vadd_s32_d(D26, D26, D4)                                        /* D26: accumulator for erb energy */
    FDK_branch(NE, activeDmxProcess_STFT_func1_loop_maxfftBands_2ComplexSamples)     /* Repeat loop */

FDK_label(activeDmxProcess_STFT_func1_loop_fft_end)
    /* Postprocessing: add energies and store them */
    FDK_vld1(32, S10, r2)                                               /* S10: targetEneLoaded */
    FDK_vpadd_s32(D26, D26, D26)                                        /* D26: res res */
    FDK_vadd_s32_d(D5, D5, D26)                                         /* S10: Add the calculated energy */
    FDK_subs_imm(r8, r8, 1)                                             /* maxErb = maxErb - 1 */
    FDK_vst1_ia(32, S10, r2)                                            /* Store the energy */

    FDK_branch(NE, activeDmxProcess_STFT_func1_loop_erb)                /* Go back to beginning of erb loop */

FDK_label(activeDmxProcess_STFT_func1_loop_erb_end)
    FDK_ldr(lr, sp, 0x40, dmx_iterations)                               /* lr = dmx_iterations */
    FDK_add_imm(r6, r6, 1, 0)                                           /* it = it + 1 */
    FDK_cmp(lr, r6)
    FDK_branch(EQ, activeDmxProcess_STFT_func1_end)                     /* If last iteration reached */

    FDK_cmp_imm(r6, 1)
    FDK_branch(NE, activeDmxProcess_STFT_func1_it1)
    /* it == 0 */
    FDK_ldr(r7, sp, 0x34, erb_is4GVH_L)
    FDK_ldr(r8, sp, 0x38, erb_is4GVH_H)
    FDK_sub(r8, r8, r7)
    FDK_ldr(r7, sp, 0x30, dmxMatrixH_FDK)                               /* r7 = dmx_coeff_mtx = dmxMatrixH_FDK */
    FDK_vdup_d_reg(16, D0, r7)                                          /* D0: dmx_coeff_mtx dmx_coeff_mtx, r7 free to use again */
    FDK_vshll_s16_imm(Q0, D0, 16)                                       /* Q0: dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx */
    FDK_branch(AL, activeDmxProcess_STFT_func1_loop_dmx_iterations)

FDK_label(activeDmxProcess_STFT_func1_it1)
    FDK_mov_imm(r7, 58)
    FDK_ldr(r8, sp, 0x38, erb_is4GVH_H)
    FDK_sub(r8, r7, r8)
    FDK_ldr(r7, sp, 0x2C, dmxMatrixL_FDK)                               /* r7 = dmx_coeff_mtx = dmxMatrixL_FDK */
    FDK_vdup_d_reg(16, D0, r7)                                          /* D0: dmx_coeff_mtx dmx_coeff_mtx, r7 free to use again */
    FDK_vshll_s16_imm(Q0, D0, 16)                                       /* Q0: dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx */
    FDK_branch(AL, activeDmxProcess_STFT_func1_loop_dmx_iterations)

FDK_label(activeDmxProcess_STFT_func1_end)
    /* Process loop run-out*/
    FDK_vmov_reg(r0, D30_0)                                             /* S60: &realizedSig[0] */
    FDK_vmov_reg(r1, D31_0)                                             /* S62: &inputBuffer[0] */
    FDK_vdup_q_reg(32, Q3, r4)                                          /* Q3:  s15:savIm0 s14:savIm0 s13:savIm0 s12:savIm0 */
    FDK_str(r4, r1, 4, inputBuffer[1])                                  /* inputBuffer[1] = savIm0 */
    FDK_vqdmulh_s32_qs(Q3, Q3, S4)                                      /* tmpIm: savIm0 * dmxCoeff */
    FDK_vbic(128, Q3, Q3, Q14)
    FDK_vmov_reg(r4, S14)                                               /* r4: tmpIm */
    FDK_ldr(lr, r0, 4, realizedSig[1])                                  /* lr: realizedSig[1] */
    FDK_vshl_s32_q(Q3, Q3, Q10)                                         /* S14: tmpIm << inBufStftHeadroom */
    FDK_add(lr, lr, r4)                                                 /* lr: realizedSig[1] += tmpIm */
    FDK_vqdmulh_s32_qq(Q3, Q3, Q3)                                      /* S14: fPow2(tmpIm) */
    FDK_vbic(128, Q3, Q3, Q14)
    FDK_str(lr, r0, 4, realizedSig[1])                                  /* lr: realizedSig[1] */
    FDK_vshl_s32_q(Q3, Q3, Q12)                                         /* S14: fPow2(tmpIm) >> target_exp */
    FDK_vshr_s32_q_imm(Q3, Q3, 1)                                       /* S14: fPow2(tmpIm) >> (target_exp + 1) */
    FDK_vadd_s32_d(D7, D7, D5)                                          /* S14: Add targetEne from last iteration */
    FDK_sub_imm(r2, r2, 4, 2)                                           /* r2:  Decrement targetEneArr pointer by 1: -> &targetEneArr[57] */
    FDK_vst1(32, S14, r2)                                               /* S14: targetEneArr[57] += targetEne */

    FDK_mpop_pc(r4,r12)
FDK_ASM_ROUTINE_END()


#endif /* #ifdef FUNCTION_activeDmxProcess_STFT_func1 */


#ifdef FUNCTION_activeDmxProcess_STFT_func2

FDK_ASM_ROUTINE_START(void, activeDmxProcess_STFT_func2,
         (FIXP_DBL **  __restrict realizedSig,     /* r0 */
          FIXP_DBL *   __restrict targetEneArr,    /* r1 */
          INT *        __restrict chOut_exp,       /* r2 */
          const UINT              numOutChans,     /* r3 */
          const INT *  __restrict chOut_count,
          FIXP_DBL **  __restrict h_realizedEnePrev,
          FIXP_DBL **  __restrict h_targetEnePrev,
          const UINT              h_numErbBands,
          const UINT * __restrict h_erbFreqIdx,
          const INT *  __restrict erb_freq_idx_256_58_exp,
          const FIXP_DBL          Alpha,
          const FIXP_DBL          One_subAlpha,
          const INT               realizedSigHeadroom,
          const INT               exp_diff_realized,
          const INT               exp_diff,
          FIXP_DBL *              pMaxRealizedEne,
          FIXP_DBL *              pMaxTargetEne))

  /*
     NEON register layout:
     lane:    3          2          1          0
     ----------------------------------------------
     Q15:  1-Alpha    1-Alpha    1-Alpha    1-Alpha
     Q14:   Alpha      Alpha      Alpha      Alpha
     Q13: reSigHdrom reSigHdrom reSigHdrom reSigHdrom
     Q12: exp_d_real exp_d_real exp_d_real exp_d_real  for summand2 or 0x00000000
     Q11: exp_d_real exp_d_real exp_d_real exp_d_real  for summand1 or 0x00000000
     Q10:  exp_diff   exp_diff   exp_diff   exp_diff   for summand2 or 0x00000000
     Q9:   exp_diff   exp_diff   exp_diff   exp_diff   for summand1 or 0x00000000
     Q8:  MaxTgEne3  MaxTgEne2  MaxTgEne1  MaxTgEne0
     Q7:  MaxReEne3  MaxReEne2  MaxReEne1  MaxReEne0
     Q6: -exp_diff -exp_diff_r +exp_diff +exp_diff_r
     Q5:      1          1         1            1     Precision mask
     Q4: ---------------- reserved -----------------
     Q3: variable used
     Q2: variable used
     Q1: variable used
     Q0: -chOut_e[i] -chOut_e[i] -chOut_e[i] -chOut_e[i]

     Core register usage:
     r0: realizedSig               FIXP_DBL**
     r1: targetEneArr              FIXP_DBL*
     r2: chOut_exp                 INT *
     r3: numOutChans               UINT
     r4: chOut_count               INT *
     r5: h_realizedEnePrev         FIXP_DBL **
     r6: h_targetEnePrev           FIXP_DBL **
     r7: h_numErbBands             INT
     r8: h_erbFreqIdx              UINT *
     r9: erb_freq_idx_256_58_exp   INT *
     r10: h_realizedEnePrev[chOut] FIXP_DBL *
     r11: h_targetEnePrev[chOut]   FIXP_DBL *
     r12: realizedSig[chOut]       FIXP_DBL *
     lr:  fftBand loop counter, other temporary variables    any


     Stack contents:
     0x58:     pMaxTargetEne
     0x54:     pMaxRealizedEne
     0x50:     exp_diff
     0x4C:     exp_diff_realized
     0x48:     realizedSigHeadroom
     0x44:     One_subAlpha
     0x40:     Alpha
     0x3C:     erb_freq_idx_256_58_exp
     0x38:     h_erbFreqIdx
     0x34:     h_numErbBands
     0x30:     h_targetEnePrev
     0x2C:     h_realizedEnePrev
     0x28:     chOut_count
     0x24:     r4
     0x20:     r5
     0x1C:     r6
     0x18:     r7
     0x14:     r8
     0x10:     r9
     0x0C:     r10
     0x08:     r11
     0x04:     r12
     0x00:     lr
  */

#ifndef __ARM_NEON__
    FIXP_DBL       ** r0 = realizedSig;
    FIXP_DBL        * r1 = targetEneArr;
    INT             * r2 = chOut_exp;
    UINT              r3 = numOutChans;
    INT             * r4;  /* = chOut_count             */
    FIXP_DBL       ** r5;  /* = h_realizedEnePrev       */
    FIXP_DBL       ** r6;  /* = h_targetEnePrev         */
    UINT              r7;  /* = h_numErbBands           */
    UINT            * r8;  /* = h_erbFreqIdx            */
    INT             * r9;  /* = erb_freq_idx_256_58_exp */
    INT               r10; /* =                         */
    INT               r11; /* =                         */
    INT               r12; /* =                         */
    INT                lr; /* =                         */
#endif

  FDK_mpush_lr(r4,r12)

  FDK_ldrd(r4, r5, sp, 0x28, chOut_count,     h_realizedEnePrev)
  FDK_ldrd(r6, r7, sp, 0x30, h_targetEnePrev, h_numErbBands)
  FDK_ldrd(r8, r9, sp, 0x38, h_erbFreqIdx,    erb_freq_idx_256_58_exp)
  FDK_ldrd(r10,r11,sp, 0x40, Alpha,           One_subAlpha)

  FDK_vdup_q_reg(32, Q15, r11)                        // Q15:  1-Alpha    1-Alpha    1-Alpha    1-Alpha
  FDK_vdup_q_reg(32, Q14, r10)                        // Q14:   Alpha      Alpha      Alpha      Alpha
  FDK_ldrd(r10,r11,sp, 0x48, realizedSigHeadroom,exp_diff_realized)
  FDK_vdup_q_reg(32, Q13, r10)                        // Q13: reSigHdrom reSigHdrom reSigHdrom reSigHdrom
  FDK_rsbs_imm(r12, r11, 0)                           // r11: exp_diff_realized  r12: -exp_diff_realized
  FDK_branch(GT, activeDmxProcess_STFT_func2_exp_diff_realized_LT)
//FDK_label(activeDmxProcess_STFT_func2_exp_diff_realized_GE)
    FDK_vmov_i32(128, Q11, 0)                         // Q11: 0x00000000 0x00000000  0x00000000 0x00000000  for summand1
    FDK_vdup_q_reg(32, Q12, r12)                      // Q12: -exp_diff_realized (=negative: shift right)   for summand2
    FDK_branch(AL, activeDmxProcess_STFT_func2_exp_diff_realized_done)
FDK_label(activeDmxProcess_STFT_func2_exp_diff_realized_LT)
    FDK_vdup_q_reg(32, Q11, r11)                      // Q11:  exp_diff_realized (=negative: shift right)   for summand1
    FDK_vmov_i32(128, Q12, 0)                         // Q12: 0x00000000 0x00000000  0x00000000 0x00000000  for summand2
FDK_label(activeDmxProcess_STFT_func2_exp_diff_realized_done)

  FDK_ldr(r11, sp, 0x50, exp_diff)                    // r11:  exp_diff
  FDK_rsbs_imm(r12, r11, 0)                           // r12: -exp_diff
  FDK_branch(GT, activeDmxProcess_STFT_func2_exp_diff_LT)
//FDK_label(activeDmxProcess_STFT_func2_exp_diff_GE)
    FDK_vmov_i32(128, Q9, 0)                          // Q9: 0x00000000 0x00000000  0x00000000 0x00000000   for summand1
    FDK_vdup_q_reg(32, Q10, r12)                      // Q10:-exp_diff (=negative: shift right)             for summand2
    FDK_branch(AL, activeDmxProcess_STFT_func2_exp_diff_done)
FDK_label(activeDmxProcess_STFT_func2_exp_diff_LT)
    FDK_vdup_q_reg(32, Q9, r11)                       // Q9: exp_diff  (=negative: shift right)             for summand1
    FDK_vmov_i32(128, Q10, 0)                         // Q10:0x00000000 0x00000000  0x00000000 0x00000000   for summand2
FDK_label(activeDmxProcess_STFT_func2_exp_diff_done)

  FDK_mvpush(Q5, Q7)                                   // Q6,7: saved on stack
  FDK_vmov_i32(128, Q5, 1)                             // Q5 : 1  1  1  1
  FDK_vmov_i32(128, Q8, 0)                            // Q8:  MaxTgEne3  MaxTgEne2  MaxTgEne1  MaxTgEne0   = 0
  FDK_vmov_i32(128, Q7, 0)                            // Q7:  MaxReEne3  MaxReEne2  MaxReEne1  MaxReEne0   = 0

  FDK_vext_q(32, Q6, Q10, Q6, 3)                      // Q6: ---------- ----------  ---------- -exp_diff (or 0) for summand2
  FDK_vext_q(32, Q6, Q12, Q6, 3)                      // Q6: ---------- ----------  -exp_diff  -exp_diff_realized (or 0) for summand2
  FDK_vext_q(32, Q6, Q9,  Q6, 3)                      // Q6: ---------- -exp_diff  -exp_diff_r +exp_diff (or 0) for summand1
  FDK_vext_q(32, Q6, Q11, Q6, 3)                      // Q6: -exp_diff  -exp_diff_r +exp_diff  +exp_diff_realized (or 0) for summand1

  /* Skip use of first 32 table entries */
  FDK_add_imm(r8, r8, 128, 2)                         // r8: &h_erbFreqIdx[32]             ->[0..31]: 1,2,3,...31
  FDK_add_imm(r9, r9, 128, 2)                         // r9: &erb_freq_idx_256_58_exp[32]  ->[0..31]: 0,0,0,...0

  /* Loop over chOut: for(chOut = 0; chOut < numOutChans; chOut++) */

FDK_label(activeDmxProcess_STFT_func2_loop_chOut)
    FDK_ldr_ia(lr,  r4, 4)                            // lr:  chOut_count[chOut]
    FDK_ldr_ia(r10, r5, 4)                            // r10: &h->realizedEnePrev[chOut][0]
    FDK_ldr_ia(r11, r6, 4)                            // r11: &h->h_targetEnePrev[chOut][0]
    FDK_ldr_ia(r12, r0, 4)                            // r12: &realizedSig[chOut][0]
    FDK_vld1dup_2d_ia(32, D0, D1, r2)                 // Q0:  chOut_e[i]  chOut_e[i]  chOut_e[i]  chOut_e[i]
    FDK_vneg_q(32, Q0, Q0)                            // Q0: -chOut_e[i] -chOut_e[i] -chOut_e[i] -chOut_e[i]
    FDK_movs_reg(lr, lr)                              // lr:  test, if (chOut_count[chOut] != 0), jump on EQ
    FDK_add_cond_op_lsl(EQ, r1, r1, r7, 2, 2)         // r1:  targetEneArr += h_numErbBands
    FDK_branch(EQ, activeDmxProcess_STFT_func2_loop_chOut_end)

    FDK_vdup_q_32(Q7, S32)                            // Q7: maxRealizedEne (duplicated)
    FDK_vdup_q_32(Q8, S33)                            // Q8: maxTargetEne (duplicated)

    FDK_mpush(r4,r9)

    FDK_ldr(r4, r12, 4, realizedSig[chOut][1])        // r4: savIm0 = realizedSig[chOut][1]
    FDK_mov_reg(r6, r12)                              // r6: realizedSig_chOut[chOut]
    FDK_mov_imm(r5, 0)
    FDK_str(r5, r12, 4, realizedSig[chOut][1])        // r5: realizedSig_chOut[1] = 0

    FDK_mov_imm(lr, 8)                                // Loop over erb 0..31, each index covers one fftBand only

FDK_label(activeDmxProcess_STFT_func2_erb_loop1)      // Comments only for first iteration (for erb=0,1,2,3)
      FDK_vld2_4d_ia(32, D4, D5, D6, D7, r12)         // Q3: realSigIm3  realSigIm2  realSigIm1  realSigIm0
                                                      // Q2: realSigRe3  realSigRe2  realSigRe1  realSigRe0
      FDK_vshl_s32_q(Q2, Q2, Q13)                     // Q2: realSigRe3  realSigRe2  realSigRe1  realSigRe0  << realizedSigHeadroom
      FDK_vshl_s32_q(Q3, Q3, Q13)                     // Q3: realSigIm3  realSigIm2  realSigIm1  realSigIm0  << realizedSigHeadroom
      FDK_vqdmulh_s32_qq(Q2, Q2, Q2)                  // Q2: fPow2(tmpRe<<realizedSigHeadroom)
      FDK_vqdmulh_s32_qq(Q3, Q3, Q3)                  // Q3: fPow2(tmpIm<<realizedSigHeadroom)
      FDK_vbic(128, Q2, Q2, Q5)                       // delete LSB - less accuracy - to match reference
      FDK_vhadd_s32_q(Q2, Q2, Q3)                     // Q2: (fPow2(tmpRe<<realizedSigHeadroom) + fPow2(tmpIm<<realizedSigHeadroom)) / 2
      FDK_vshl_s32_q(Q2, Q2, Q0)                      // Q2: realizEne3  realizEne2  realizEne1  realizEne0  >> chOut_exp
      FDK_vld1_2d(32, D6, D7, r10)                    // Q3: realEnePr3  realEnePr2  realEnePr1  realEnePr0 (=h->realizedEnePrev[chOut][erb])
      FDK_vqdmulh_s32_qq(Q2, Q2, Q14)                 // Q2: summand1 = fMult(Alpha, realizedEne)
      FDK_vqdmulh_s32_qq(Q3, Q3, Q15)                 // Q3: summand2 = fMult(One_subAlpha, h->realizedEnePrev[chOut][erb])
      FDK_vbic(128, Q2, Q2, Q5)
      FDK_vbic(128, Q3, Q3, Q5)
      FDK_vshl_s32_q(Q2, Q2, Q11)                     // Q2: summand1 >> -exp_diff_realized or 0
      FDK_vshl_s32_q(Q3, Q3, Q12)                     // Q3: summand2 >>  0 or exp_diff_realized
      FDK_vadd_s32_q(Q2, Q2, Q3)                      // Q2: realizEne3  realizEne2  realizEne1  realizEne0
      FDK_vst1_2d_ia(32, D4, D5, r10)                 // Q2: h->realizedEnePrev[chOut][erb] = realizedEne
      FDK_vmax_s32(128, Q7, Q7, Q2)                   // Q7: maxRealizedEne = fMax(maxRealizedEne, realizedEne)

      FDK_vld1_2d_ia(32, D4, D5, r1)                  // Q2: targetEne3  targetEne2  targetEne1  targetEne0
      FDK_vld1_2d(32, D6, D7, r11)                    // Q3: tgEnePrev3  tgEnePrev2  tgEnePrev1  tgEnePrev0
      FDK_vqdmulh_s32_qq(Q2, Q2, Q14)                 // Q2: summand1 = fMult(Alpha,        targetEnergy[erb])
      FDK_vqdmulh_s32_qq(Q3, Q3, Q15)                 // Q3: summand2 = fMult(One_subAlpha, h->targetEnePrev[chOut][erb])
      FDK_vbic(128, Q2, Q2, Q5)
      FDK_vbic(128, Q3, Q3, Q5)
      FDK_vshl_s32_q(Q2, Q2, Q9)                      // Q2: summand1 >> -exp_diff or 0
      FDK_vshl_s32_q(Q3, Q3, Q10)                     // Q3: summand2 >>  0 or exp_diff
      FDK_vadd_s32_q(Q2, Q2, Q3)                      // Q2: targetEne3  targetEne2  targetEne1  targetEne0
      FDK_vst1_2d_ia(32, D4, D5, r11)                 // Q2: h->targetEnePrev[chOut][erb] = targetEne
      FDK_vmax_s32(128, Q8, Q8, Q2)                   // Q8: maxTargetEne = fMax(maxTargetEne, targetEne)
      FDK_subs_imm(lr, lr, 1)
      FDK_branch(NE, activeDmxProcess_STFT_func2_erb_loop1)

    /* Combine 4x maxTargetEne and 4x maxRealizedEne into D16 */
    FDK_vpmax_s32(64, D16, D16, D17)                  // D16: MaxTgEne1,0  MaxTgEne3,2
    FDK_vpmax_s32(64, D14, D14, D15)                  // D14: MaxReEne1,0  MaxReEne3,2
    FDK_vpmax_s32(64, D16, D14, D16)                  // D16: MaxTargetEne MaxRealizEne


    FDK_sub_imm(r7, r7, 33, 0)                        // r7: number of remaining erb loops: h_numErbBands - 32, last loop adds savIm in S11
    FDK_mov_imm(r5, 32)                               // r5: fftBand
    FDK_vswp(64, D30, D29)                            // Q15: 1-Alpha    1-Alpha      Alpha*     Alpha*
                                                      // Q14: 1-Alpha*   1-Alpha*     Alpha      Alpha
    FDK_vmov_i32(128, Q2, 0)                          // Q2: 0x00000000  0x00000000  0x00000000  0x00000000  4x realized_Sig=0

FDK_label(activeDmxProcess_STFT_func2_erb_loop2)      // Comments only for 1st iteration (for erb=32)
      FDK_vmov_i32(128, Q3, 0)                        // Q3: 0x00000000  0x00000000  0x00000000  0x00000000  4x realized_Ene=0
      FDK_mov_reg(lr, r5)                             // lr: fftBand
      FDK_ldr_ia(r5, r8, 4)                           // r5: h->erbFreqIdx[erb]
      FDK_sub(lr, r5, lr)                             // lr: number of fftBands to process
      FDK_vld1dup_2d_ia(32, D2, D3, r9)               // Q1: erb_freq_idx_256_58_exp[erb] (duplicated)
      FDK_vsub_s32_q(Q1, Q0, Q1)                      // Q1: realized_exp = -chOut_exp[chOut] - erb_freq_idx_256_58_exp[erb]
      FDK_movs_asr_imm(lr, lr, 1)                     // Check if loop length is odd or even and lr >>= 1 */
      FDK_branch(CC, activeDmxProcess_STFT_func2_loop_maxfftBands_2ComplexSamples)

//FDK_label(activeDmxProcess_STFT_func2_loop_maxfftBands_1ComplexSample)
      FDK_vld1_1d_ia(32, D4, r12)                     // Q2: savIm0 | 0  0x00000000  realSigIm0  realSigRe0
      FDK_vshl_s32_q(Q2, Q2, Q13)                     // Q2: 0x00000000  0x00000000  realSigIm0  realSigRe0  << realizedSigHeadroom
      FDK_vqdmulh_s32_qq(Q2, Q2, Q2)                  // Q2: 0x00000000  0x00000000   tmpIm0^2    tmpRe^2
      FDK_vshr_s32_q_imm(Q2, Q2, 1)
      FDK_vpadd_s32(D4, D4, D5)                       // Q2: 0x00000000  0x00000000  0x00000000  EnRe0+EnIm0
      FDK_vshl_s32_d(D6, D4, D2)                      // Q3: 0x00000000  0x00000000  0x00000000  realEneRe0 >> -chOut_exp-realized_exp
      FDK_branch(EQ, activeDmxProcess_STFT_func2_loop2_smooth)

FDK_label(activeDmxProcess_STFT_func2_loop_maxfftBands_2ComplexSamples)
      FDK_subs_imm(lr, lr, 1)
      FDK_vld1_2d_ia(32, D4, D5, r12)                 // Q2: realSigIm1  realSigRe1  realSigIm0  realSigRe0
      FDK_vshl_s32_q(Q2, Q2, Q13)                     // Q2: realSigIm1  realSigRe1  realSigIm0  realSigRe0  << realizedSigHeadroom
      FDK_vqdmulh_s32_qq(Q2, Q2, Q2)                  // Q2:  tmpIm1^2    tmpRe1^2    tmpIm0^2    tmpRe^2
      FDK_vshr_s32_q_imm(Q2, Q2, 1)
      FDK_vpadd_s32(D4, D4, D5)                       // Q2: dontcare    dontcare    EnIm1+EnRe1 EnIm0+EnRe0
      FDK_vshl_s32_d(D4, D4, D2)                      // Q2: dontcare  dontcare      EnIm1+EnRe1 EnIm1+EnRe1 >> -chOut_exp-realized_exp
      FDK_vadd_s32_d(D6, D6, D4)                      // Q3: dontcare  dontcare      EnImX+EnReX EnImY+EnReY <-accumulated energies of realizedSig
      FDK_branch(NE, activeDmxProcess_STFT_func2_loop_maxfftBands_2ComplexSamples)

FDK_label(activeDmxProcess_STFT_func2_loop2_smooth)   // Q3: realEneImY  realEneReY  realEneImX  realEneReX
      FDK_subs_imm(r7, r7, 1)                         // r7: loop counter decement: erb--
      FDK_vpadd_s32(D6, D6, D6)                       // Q3: ----------  ----------  realizedEne realizedEne
      FDK_vld1_ia(32, S13, r1)                        // Q3: ----------  ----------  targetEne   realizedEne
      FDK_vld1(32, S14, r10)                          // Q3: ----------  realEnePr   targetEne   realizedEne
      FDK_vld1(32, S15, r11)                          // Q3: tgtEneprev  realEnePr   targetEne   realizedEne
      FDK_vqdmulh_s32_qq(Q3, Q3, Q15)                 // Q3:(tgtEneprev realEnePr)*(1-Alpha)
                                                      //                             (targetEne   realizedEne)*Alpha
      FDK_vbic(128, Q3, Q3, Q5)
      FDK_vshl_s32_q(Q3, Q3, Q6)                      // Q3: tgtEneprev >> exp_diff (or 0)
                                                      //                 realEnePr >> exp_diff_realized (or 0)
                                                      //                             targetEne>>-exp_diff (or 0)
                                                      //                                        realizedEne>>-exp_diff_realized (or 0)

      FDK_vadd_s32_d(D6, D6, D7)                      // D6:                         targetEne   realizedEne  (smoothed)
      FDK_vst1_ia(32, S12, r10)                       // S12: h->realizedEnePrev[chOut][erb] = realizedEne
      FDK_vst1_ia(32, S13, r11)                       // S13: h->targetEnePrev[chOut][erb]   = targetEne
      FDK_vmax_s32(64, D16, D16, D6)                  // D16: MaxTargetEne MaxRealizEne
      FDK_vmov_i32(128, Q2, 0)                        // Q2: 0x00000000  0x00000000  0x00000000  0x00000000  4x realized_Sig=0

      FDK_branch(GT, activeDmxProcess_STFT_func2_erb_loop2)  /* with r7 in [24..1] */
      /* For last iteration (erb=57), we preload savIm0 into S11, last erb iterates over 15 fftBands (=odd) */
      FDK_vmov_sn(S11, r4)                            // Q2:  savIm0    0x00000000  0x00000000  0x00000000
      FDK_branch(EQ, activeDmxProcess_STFT_func2_erb_loop2) /* with r7 = 0 (last iteration) */

      /* After last iteration (erb=57), we restore realizedSig[chOut][1] = savIm0 */
      FDK_str(r4, r6, 4, realizedSig[chOut][1])       // r6: points to realized_Sig[chOut][0]
      FDK_mpop(r4,r9)
      FDK_vswp(64, D30, D29)                          // Q15: 1-Alpha    1-Alpha    1-Alpha*   1-Alpha*
                                                      // Q14:   Alpha*     Alpha*     Alpha      Alpha

FDK_label(activeDmxProcess_STFT_func2_loop_chOut_end)
    FDK_subs_imm(r3, r3, 1)
    FDK_branch(NE, activeDmxProcess_STFT_func2_loop_chOut)

//FDK_label(activeDmxProcess_STFT_func2_end)
      FDK_mvpop(Q5, Q7)
  FDK_ldr(r0, sp, 0x54, pMaxRealizedEne)              // r0: pMaxRealizedEne
  FDK_ldr(r1, sp, 0x58, pMaxTargetEne)                // r1: pMaxTargetEne
  FDK_vst1(32, S32, r0)                               // *pMaxRealizedEne = maxRealizedEne
  FDK_vst1(32, S33, r1)                               // *pMaxTargetEne   = maxTargetEne

  FDK_mpop_pc(r4, r12)
  FDK_return()

FDK_ASM_ROUTINE_END()



#endif /* #ifdef FUNCTION_activeDmxProcess_STFT_func2 */



#ifdef FUNCTION_activeDmxProcess_STFT_func3

FDK_ASM_ROUTINE_START(void, activeDmxProcess_STFT_neonv7_func3,
   (FIXP_DBL *realizedSig,       /* r0 */
    FIXP_DBL *EQ_vector,         /* r1 */
    INT       numErbBands))      /* r2 */
#ifndef __ARM_NEON__
    FIXP_DBL        * r0 = realizedSig;
    FIXP_DBL        * r1 = EQ_vector;
    INT               r2 = numErbBands;
    INT               r3;
#endif

  /*
     NEON register layout for first loop:
     lane:       3         2         1         0
     ----------------------------------------------
     Q3:    EQ_exp3   EQ_exp2   EQ_exp1   EQ_exp0
     Q2:      EQ3      EQ2        EQ1      EQ0
     Q1:   realSim3  realSim2  realSim1  realSim0   4 samples (imag) from realizedSig
     Q0:   realSre3  realSre2  realSre1  realSre0   4 samples (real) from realizedSig

     NEON register layout for second loop:
     lane:       3         2         1         0
     ----------------------------------------------
     Q2:    EQ_exp1   EQ_exp1   EQ_exp0   EQ_exp0
     Q1:      EQ1      EQ1       EQ0       EQ0
     Q0:   realSim1  realSre1  realSim0  realSre0   2x2 samples (real/imag) from realizedSig

     Core register usage:
     r0: realizedSig            FIXP_DBL*
     r1: EQ_vector              (FIXP_DBL / INT / INT) *
     r2: erb                    INT
     r3: runs                   INT

     Stack contents:
      - none -
  */

  FDK_vdup_d_reg(32, D16, r0)                         /* D16:  r0       r0        */
  FDK_vld1_1d_ia(32, D17, r0)                         /* D17:  rSigIm0  rSigRe0   */
  FDK_vld1_1d_ia(32, D18, r1)                         /* D18:  EQ_exp0  EQ0       */
  FDK_vld1dup_1d_ia(32, D0, r1)                       /* D0:   just increment r1  */

  FDK_mov_imm(r3, 8)
FDK_label(activeDmxProcess_STFT_neonv7_func3_loop1)
    FDK_vld3_ia(32, D0, D2, D4, r1)                   /* Q0:  EQ3      EQ2      EQ1      EQ0      */
    FDK_vld3_ia(32, D1, D3, D5, r1)                   /* Q1:  EQ_exp3  EQ_exp2  EQ_exp1  EQ_exp0  */
                                                      /* Q2:  run3     run2     run1     run0     */
    FDK_vld2_4d(32, D4, D5, D6, D7, r0)               /* Q2:  rSigRe3  rSigRe2  rSigRe1  rSigRe0  */
                                                      /* Q3:  rSigIm3  rSigIm2  rSigIm1  rSigIm0  */
    FDK_vqdmulh_s32_qq(Q2, Q2, Q0)                    /* Q2:  Re3*EQ3  Re2*EQ2  Re1*EQ1  Re0*EQ0  */
    FDK_vqdmulh_s32_qq(Q3, Q3, Q0)                    /* Q3:  Im3*EQ3  Im2*EQ2  Im1*EQ1  Im0*EQ0  */
    FDK_vshl_s32_q(Q2, Q2, Q1)                        /* Q2:  Re3<<e3  Re2<<e2  Re1<<e1  Re0<<e0  */
    FDK_vshl_s32_q(Q3, Q3, Q1)                        /* Q3:  Im3<<e3  Im2<<e2  Im1<<e1  Im0<<e0  */
    FDK_vst2_4d_ia(32, D4, D5, D6, D7, r0)            /* Q2,Q3: store interleaved                 */
    FDK_subs_imm(r3, r3, 1)
    FDK_branch(NE, activeDmxProcess_STFT_neonv7_func3_loop1)

  FDK_sub_imm(r2, r2, 33, 0)                          /* r2: remaining erb loops: numErbBands-33  */
FDK_label(activeDmxProcess_STFT_neonv7_func3_loop2)
    FDK_vld1dup_2d_ia(32, D0, D1, r1)                 /* Q0:  EQi      EQi      EQi      EQi      */
    FDK_vld1dup_2d_ia(32, D2, D3, r1)                 /* Q1:  EQ_expi  EQ_expi  EQ_expi  EQ_expi  */

    FDK_ldr_ia(r3, r1, 4)                             /* r3:  runs in range [1..19]               */
    FDK_movs_asr_imm(r3, r3, 1)                       /* r3: runs>>1   %carry: 1 or 0             */
    FDK_branch(CC, activeDmxProcess_STFT_neonv7_func3_loop2_2x)

    /* Run over 1 fftband if (runs & 1) */
    FDK_vld1_1d(32, D4, r0)                           /* D4:                    rSigImi  rSigRei  */
    FDK_vqdmulh_s32_dd(D4, D4, D0)                    /* D4:                    Imi*EQi  Rei*EQi  */
    FDK_vshl_s32_d(D4, D4, D2)                        /* D4:                    Imi<<ei  Rei<<ei  */
    FDK_vst1_1d_ia(32, D4, r0)                        /* D4: store linear       rSigImi  rSigRei  */
    FDK_branch(EQ, activeDmxProcess_STFT_neonv7_func3_loop2_4x_end)

FDK_label(activeDmxProcess_STFT_neonv7_func3_loop2_2x)
    FDK_movs_asr_imm(r3, r3, 1)                       /* r3: runs>>2   %carry: 1 or 0             */
    FDK_branch(CC, activeDmxProcess_STFT_neonv7_func3_loop2_4x)

    /* Run over 2 fftbands if (runs & 2) */
    FDK_vld1_2d(32, D4, D5, r0)                       /* Q2: rSigIm1  rSigRe1   rSigIm0  rSigRe0  */
    FDK_vqdmulh_s32_dd(Q2, Q2, Q0)                    /* Q2: Im1*EQi  Re1*EQi   Im0*EQi  Re0*EQi  */
    FDK_vshl_s32_q(Q2, Q2, Q1)                        /* Q2: Im1<<ei  Re1<<ei   Im0<<ei  Re0<<ei  */
    FDK_vst1_2d_ia(32, D4, D5, r0)                    /* Q2: store linear       rSigIm0  rSigRe0  */
    FDK_branch(EQ, activeDmxProcess_STFT_neonv7_func3_loop2_4x_end)

FDK_label(activeDmxProcess_STFT_neonv7_func3_loop2_4x)
      FDK_vld1_4d(32, D4, D5, D6, D7, r0)             /* Q3,2: rSigIm3  rSigRe3   rSigIm2  rSigRe2  rSigIm1  rSigRe1   rSigIm0  rSigRe0  */
      FDK_vqdmulh_s32_dd(Q2, Q2, Q0)
      FDK_vqdmulh_s32_dd(Q3, Q3, Q0)                  /* Q3,2: Im3*EQi  Re3*EQi   Im2*EQi  Re2*EQi  Im1*EQi  Re1*EQi   Im0*EQi  Re0*EQi  */
      FDK_vshl_s32_q(Q2, Q2, Q1)
      FDK_vshl_s32_q(Q3, Q3, Q1)                      /* Q3,2: Im3<<ei  Re3<<ei   Im2<<ei  Re2<<ei  Im1<<ei  Re1<<ei   Im0<<ei  Re0<<ei  */
      FDK_vst1_4d_ia(32, D4, D5, D6, D7, r0)          /* Q3,2: store linear                                                              */
      FDK_subs_imm(r3, r3, 1)
      FDK_branch(NE, activeDmxProcess_STFT_neonv7_func3_loop2_4x)

FDK_label(activeDmxProcess_STFT_neonv7_func3_loop2_4x_end)
    FDK_subs_imm(r2, r2, 1)
    FDK_branch(NE, activeDmxProcess_STFT_neonv7_func3_loop2)

  /* Finalize realizedSig[0..1] */
  FDK_vmov_reg(r0, S32)                               /* r0: realizedSig          */
                                                      /* D2:   EQ_exp57 EQ_exp57  */
                                                      /* D17:  rSigIm0  rSigRe0   */

                                                      /* D0:   EQ57     EQ57      */
                                                      /* D18:  EQ_exp0  EQ0       */
  FDK_vzip_d(32, D18, D0)                             /* D0:   EQ57     EQ_exp0   */
                                                      /* D18:  EQ57     EQ0       */


  FDK_vqdmulh_s32_dd(D4, D17, D18)                    /* D4:   Im0*EQ57 Re0*EQ0   */

                                                      /* D2:   EQ_exp57 EQ_exp57  */
                                                      /* D0:   EQ57     EQ_exp0   */
  FDK_vzip_d(32, D0, D2)                              /* D2:   EQ_exp57 EQ57      */
                                                      /* D0:   EQ_exp57 EQ_exp0   */


  FDK_vshl_s32_d(D4, D4, D0)                          /* D4:   Im0<<e57 Re0<<e0   */
  FDK_vst1_1d(32, D4, r0)                             /* D4:   rSigIm0  rSigRe0   */

  FDK_return()

FDK_ASM_ROUTINE_END()

static void activeDmxProcess_STFT_func3(
          FIXP_DBL * __restrict targetEne_ChOut,
          FIXP_DBL * __restrict realizedSig__chOut,
          FIXP_DBL * __restrict realizedEne_ChOut,
          FIXP_DBL * __restrict EQ_vector,
    const UINT     * __restrict h_erbFreqIdx,
    const FIXP_DBL              eqLimitMin,
    const FIXP_DBL              eqLimitMax,
    const FIXP_DBL              fixpAES,
    const INT                   numErbBands,
    const INT                   inBufStftHeadroomPrev,
    const INT                   realizedSigHeadroomPrev,
          INT                   eq_e)
{

    INT erb, EQ_exp;
    INT diffExp = realizedSigHeadroomPrev - inBufStftHeadroomPrev;
    FIXP_DBL EQ;
    UINT fftBand = 0;
    UINT runs;

    /* Compute all 58 EQ coefficients in advance */
    for(erb = 0; erb < numErbBands; erb++)
    {
      EQ = computeEQAndClip(targetEne_ChOut[erb], realizedEne_ChOut[erb], diffExp, eqLimitMin, eqLimitMax, fixpAES, eq_e, &EQ_exp);
      EQ_vector[3*erb+0] = EQ;
      EQ_vector[3*erb+1] = EQ_exp;
      runs = h_erbFreqIdx[erb] - fftBand;
      EQ_vector[3*erb+2] = runs;
      fftBand += runs;
    }

    activeDmxProcess_STFT_neonv7_func3(realizedSig__chOut, EQ_vector, numErbBands);

}
#endif /* FUNCTION_activeDmxProcess_STFT_func3 */
