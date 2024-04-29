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
//#define FUNCTION_activeDmxProcess_STFT_func2  -- replaced by func4/func5
#define FUNCTION_activeDmxProcess_STFT_func3
#define FUNCTION_activeDmxProcess_STFT_func4
#define FUNCTION_activeDmxProcess_STFT_func5

#ifdef FUNCTION_activeDmxProcess_STFT_func1

FDK_ASM_ROUTINE_START(void, activeDmxProcess_STFT_func1,
       (FIXP_DBL  * __restrict inputBuffer,       /* r0 */
        FIXP_DBL  * __restrict realizedSig,       /* r1 */
        FIXP_DBL  * __restrict targetEneArr,      /* r2 */
  const UINT      * __restrict erbFreqIdx,        /* r3 */
  const FIXP_DBL  * __restrict eq_ptr,
  const FIXP_DMX_H             dmxMatrixL_FDK,
  const FIXP_DMX_H             dmxMatrixH_FDK,
  const UINT                   erb_is4GVH_L,
  const UINT                   erb_is4GVH_H,
  const INT                    chOut_exp,
  const INT                    dmx_iterations,
  const INT       * __restrict inBufStftHeadroom,
  const INT       * __restrict erb_freq_idx_256_58_exp))

#ifndef __ARM_NEON__
    FIXP_DBL        * r0 = inputBuffer;
    FIXP_DBL        * r1 = realizedSig;
    FIXP_DBL        * r2 = targetEneArr;
    UINT            * r3 = erbFreqIdx;
    INT             * r4;  /* = inBufStftHeadroom */
    FIXP_DBL        * r5;  /* = eq_ptr */
    FIXP_DMX_H        r6;  /* = dmxMatrixL_FDK;  dmxMatrixH_FDK; */
    UINT              r7;  /* = erb_is4GVH_L; */
    UINT              r8;  /* = erb_is4GVH_H; */
    INT               r9;  /* = chOut_exp; */
    INT               r10; /* = dmx_iterations; */
    UINT              r11; /* = erbFreqIdx[erb]; */
    INT             * r12; /* = erb_freq_idx_256_58_exp; */
    FIXP_DBL          savIm0 = 0;
    FIXP_DBL          lr;
#endif

    /*
     NEON register layout:
     lane:       3         2         1         0
     ----------------------------------------------
     Q15:   dmx_coeff_mtx                          (duplicated)
     Q14:   eq_ptr3   eq_ptr2   eq_ptr1   eq_ptr0
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
     Q1:                        eq[erb]   eq[erb] (duplicated)
     Q0:    realSig  inputBuf    savIm    -------

     Core register usage:
     r0: inputBuffer            FIXP_DBL*   read cplx for each fftBand iteration
     r1: realizedSig            FIXP_DBL*   read/write cplx for each fftBand iteration
     r2: targetEneArr           FIXP_DBL*   read/write for each erb iteration
     r3: erbFreqIdx             UINT*       read for each erb iteration
     r4: inBufStftHeadroom      INT *       read for each erb iteration
     r5: eq_ptr                 FIXP_DBL*   read for each erb iteration
     r6: it                     INT         loop counter for iterations: 2->1->0
     r7: diverse stack variable, constants
     r8: erb_is4GVH_L, maxErb   UINT        loop counter for erb
     r9: erb_is4GVH_H           UINT
    r10: FftBand                UINT        erbFreqIdx[erb-1]: starts with 0
    r11: maxFftBand             UINT        erbFreqIdx[erb]
    r12: erb_freq_idx_256_58_exp INT*       read for each erb iteration
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
    FDK_vld1_1d(32, D0, r0)                                             /* Q0,D0: S1:savIm0=inBuf[1]     S0: inputBuffer[0]  */
    FDK_mov_imm(lr, 0)
    FDK_ldr(r5, sp, 0x28, eq_ptr)                                       /* r5 = (FIXP_DBL *) *eq_ptr */
    FDK_str(lr, r0, 4, inputBuffer[1])                                  /* inputBuffer[1] = FIXP_DBL(0) */
    FDK_vmov_dsn(S2, S3, r0, r1)                                        /* Q0,D1: S3: &realizedSig[0]    S2: &inputBuffer[0] */
    FDK_ldr( r6, sp, 0x2C, dmxMatrixL_FDK)                              /* r6 = (FIXP_SGL) dmx_coeff_mtx = dmxMatrixL_FDK */
    FDK_ldrd(r8, r9, sp, 0x34, erb_is4GVH_L, erb_is4GVH_H)              /* r8 = (UINT) max_erb = erb_is4GVH_L || r9 = *erb_is4GVH_H */
    FDK_ldr(r4, sp, 0x44, inBufStftHeadroom)                            /* r4 = (INT *) inBufStftHeadroom */
    FDK_ldr(r11, sp, 0x3C, chOut_exp)                                   /* r11 = (INT) chOut_exp[chOut] */
    FDK_ldr(r12, sp, 0x48, erb_freq_idx_256_58_exp)                     /* r12 = erb_freq_idx_256_58_exp */
    FDK_vdup_q_reg(32, Q11, r11)                                        /* Q11: chOut_exp[chOut] chOut_exp[chOut] chOut_exp[chOut] chOut_exp[chOut] */
    FDK_vdup_d_reg(16, D30, r6)                                         /* D30: dmx_coeff_mtx dmx_coeff_mtx, r6 free to use again */
    FDK_vneg_q(32, Q11, Q11)                                            /* Q11: -chOut_exp[chOut] -chOut_exp[chOut] -chOut_exp[chOut] -chOut_exp[chOut] */
    FDK_vshll_s16_imm(Q15, D30, 16)                                     /* Q15: dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx */

    /* Take into account the first lr*4 iterations */
    FDK_lsr_imm(lr, r8, 2)                                              /* lr: Loop counter / 4 for first max 32 erb */
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
    FDK_vld1_2d_ia(32, D28, D29, r5)                                    /* Q14:eq_ptr[erb+3] eq_ptr[erb+2] eq_ptr[erb+1] eq_ptr[erb+0] */
    FDK_vld2_4d_ia(32, D4, D5, D6, D7, r0)                              /* Q2: bufRe3        bufRe2        bufRe1        bufRe0        */
                                                                        /* Q3: bufIm3        bufIm2        bufIm1        bufIm0        */
    FDK_vld2_4d(32, D16, D17, D18, D19, r1)                             /* Q8: SigRe3        SigRe2        SigRe1        SigRe0        */
                                                                        /* Q9: SigIm3        SigIm2        SigIm1        SigIm0        */
    FDK_vld1_2d_ia(32, D20, D21, r4)                                    /* Q10: inputHdr3    inputHdr2     inputHdr1     inputHdr0     */
    FDK_vqdmulh_s32_qq(Q14, Q15, Q14)                                   /* Q14:dmx_coeff3    dmx_coeff2    dmx_coeff1    dmx_coeff0    */
    FDK_vshl_s32_q(Q2, Q2, Q10)                                         /* Q2: tmpRe << inBufStftHeadroom ... */
    FDK_vshl_s32_q(Q3, Q3, Q10)                                         /* Q3: tmpIm << inBufStftHeadroom ... */
    FDK_vneg_q(32, Q10, Q10)                                            /* Q10:-inputHdr3   -inputHdr2    -inputHdr1    -inputHdr0     */
    FDK_vqdmulh_s32_qq(Q2, Q2, Q14)                                     /* Q2: bufRe3*c3     bufRe2*c2     bufRe1*c1      bufRe0*c0    */
    FDK_vqdmulh_s32_qq(Q3, Q3, Q14)                                     /* Q3: bufIm3*c3     bufIm2*c2     bufIm1*c1      bufIm0*c0    */
    FDK_vshl_s32_q(Q1, Q2, Q10)                                         /* Q1: tmpRe3*c3>>H  tmpRe2*c2>>H  tmpRe1*c1>>H   tmpRe0*c0>>H */
    FDK_vadd_s32_q(Q8, Q8, Q1)                                          /* realizedSig_chOut[fftBand*2+0] += tmpRe>>Hdr ... */
    FDK_vshl_s32_q(Q1, Q3, Q10)                                         /* Q1: tmpIm3*c3>>H  tmpIm2*c2>>H  tmpIm1*c1>>H   tmpIm0*c0>>H */
    FDK_vadd_s32_q(Q9, Q9, Q1)                                          /* realizedSig_chOut[fftBand*2+1] += tmpIm>>Hdr ... */
    FDK_vst2_4d_ia(32, D16, D17, D18, D19, r1)                          /* Store Q8, Q9 with pointer update   */
    FDK_vqdmulh_s32_qq(Q2, Q2, Q2)                                      /* Q2: tmpRe = fPow2(tmpRe) ...       */
    FDK_vqdmulh_s32_qq(Q3, Q3, Q3)                                      /* Q3: tmpIm = fPow2(tmpIm) ...       */
    FDK_vhadd_s32_q(Q2, Q2, Q3)                                         /* Q2: result = tmpRe^2 + tmpIm^2 ... */
    FDK_vshl_s32_q(Q2, Q2, Q11)                                         /* Q2: result >>= chOut_exp ...       */
    FDK_vld1_2d(32, D16, D17, r2)                                       /* Q8: targetEne3 targetEne2 targetEne1 targetEne0 */
    FDK_vadd_s32_q(Q8, Q8, Q2)                                          /* Q8: Add the calculated energies */
    FDK_subs_imm(lr, lr, 1)                                             /* lr: Update loop counter */
    FDK_vst1_2d_ia(32, D16, D17, r2)                                    /* Q8: Store targetEne's with pointer update */
    FDK_branch(NE, activeDmxProcess_STFT_func1_first32Samples)          /* Repeat max. 8 times */
/* Done processing the very first 32 samples */
FDK_label(activeDmxProcess_STFT_func1_first32Samples_end)

    FDK_mov_imm(r6, 0)                                                  /* r6 = it = 0 */
    FDK_cmp_imm(r8, 0)                                                  /* r8: remaining iterations after up to 32 first erb */
    FDK_branch(EQ, activeDmxProcess_STFT_func1_loop_erb_end)


/* Outermost loop with dmx_iterations */
FDK_label(activeDmxProcess_STFT_func1_loop_dmx_iterations)

FDK_label(activeDmxProcess_STFT_func1_loop_erb)                         /* Second loop for each erb index                      */
    FDK_ldr_ia(r11, r3, 4)                                              /* r11: maxfftBand -> *h->erbFreqIdx[erb]              */
    FDK_vld1dup_2d_ia(32, D28, D29, r5)                                 /* Q14: eq_ptr[erb] (duplicated)                       */
    FDK_sub(lr, r11, r10)                                               /* lr: fftBands to process                             */
    FDK_vqdmulh_s32_qq(Q14, Q15, Q14)                                   /* Q14: dmx_coeff (duplicated)                         */
    FDK_mov_reg(r10, r11)                                               /* r10: Save old maxfftBand to r10                     */
    FDK_vld1dup_2d_ia(32, D24, D25, r12)                                /* Q12: target_exp[erb] (duplicated)                   */
    FDK_vsub_s32_q(Q12, Q11, Q12)                                       /* Q12: -chOut_exp-target_exp                          */
    FDK_vld1dup_2d_ia(32, D20, D21, r4)                                 /* Q10: inputHeadroom[erb] (duplicated)                */
    FDK_vneg_q(32, Q1, Q10)                                             /* Q1: -inputHeadroom[erb] (duplicated)                */


//FDK_label(activeDmxProcess_STFT_func1_loop_maxfftBands)
    FDK_vmov_i32(128, Q13, 0)                                           /* Q13: clear accu for targetEne */

    FDK_movs_asr_imm(lr, lr, 1)                                         /* Check if loop length is odd or even and lr >> 1 */
    FDK_branch(CC, activeDmxProcess_STFT_func1_loop_maxfftBands_2ComplexSamples)

    /* Process one complex sample if length is odd */
    /* Energy calculation */
    FDK_vld1_1d_ia(32, D4, r0)                                          /* D4:  bufIm             bufRe           */
    FDK_vld1_1d(32, D16, r1)                                            /* D16: realizedSigIm0    realizedSigRe0  */
    FDK_vshl_s32_d(D4, D4, D20)                                         /* D4:  bufIm<<Hdr        bufRe<<Hdr      */
    FDK_vqdmulh_s32_dd(D4, D4, D28)                                     /* D4:  (bufIm<<Hdr)*c    (bufRe<<Hdr)*c  */
    FDK_vshl_s32_d(D6, D4, D2)                                          /* D6:  tmpIm>>Hdr        tmpRe>>Hdr      */
    FDK_vadd_s32_d(D16, D16, D6)                                        /* D16  realizedSigIm0    realizedSigRe0  */
    FDK_vst1_1d_ia(32, D16, r1)                                         /* Store realizedSig with pointer update  */
    FDK_vqdmulh_s32_dd(D4, D4, D4)                                      /* D4:  fPow2(tmpIm)      fPow2(tmpRe)     */
    FDK_vzip_d(32, D4, D26)                                             /* D26: 0x00000000        fPow2(tmpIm)     */
                                                                        /* D4:  0x00000000        fPow2(tmpRe)     */
    FDK_vhadd_s32_d(D4, D4, D26)                                        /* D4:  0x00000000    (tmpRe^2+tmpIm^2)/2  */
    FDK_vshl_s32_d(D26, D4, D24)                                        /* D26: 0x00000000    (tmpRe^2+tmpIm^2)>>t */
    FDK_branch(EQ, activeDmxProcess_STFT_func1_loop_fft_end)            /* Finish, if we had only one iteration   */

    /* Continue with loop with two complex samples */
FDK_label(activeDmxProcess_STFT_func1_loop_maxfftBands_2ComplexSamples)
    FDK_subs_imm(lr, lr, 1)                                             /* lr = lr - 1 */
    /* Energy calculation */
    FDK_vld1_2d_ia(32, D4, D5, r0)                                      /* Q2:   bufIm1     bufRe1     bufIm0     bufRe0   */
    FDK_vld1_2d(32, D16, D17, r1)                                       /* Q8: realSigIm1 realSigRe1 realSigIm0 realSigRe0 */
    FDK_vshl_s32_q(Q2, Q2, Q10)                                         /* D4:  bufIm1<H   bufRe1<<H  bufIm0<H   bufRe0<<H */
    FDK_vqdmulh_s32_qq(Q2, Q2, Q14)                                     /* Q2:   tmpIm1     tmpRe1     tmpIm0     tmpRe0   */
    FDK_vshl_s32_q(Q3, Q2, Q1)                                          /* Q3:  tmpIm1>>H  tmpRe1>>H  tmpIm0>>H  tmpRe0>>H */

    FDK_vadd_s32_q(Q8, Q8, Q3)                                          /* Q8: Add calculated values to realizedSig        */
    FDK_vst1_2d_ia(32, D16, D17, r1)                                    /* Q8: Store realizedSig with pointer update       */
    FDK_vqdmulh_s32_qq(Q2, Q2, Q2)                                      /* Q2:  tmpIm1^2   tmpRe1^2   tmpIm1^2   tmpRe1^2  */
    FDK_vzip_d(32, D4, D5)                                              /* Q2: pow2(im1)  pow2(im0)  pow2(re1)  pow2(re0)  */
    FDK_vhadd_s32_d(D4, D4, D5)                                         /* Q2,D4:      tmp1/2            tmp0/2            */
    FDK_vshl_s32_d(D4, D4, D24)                                         /* D4:  energy of fftband[2*i] and fftband[2*i+1]  */
    FDK_vadd_s32_d(D26, D26, D4)                                        /* D26: accumulator for erb energy */
    FDK_branch(NE, activeDmxProcess_STFT_func1_loop_maxfftBands_2ComplexSamples)     /* Repeat loop */

FDK_label(activeDmxProcess_STFT_func1_loop_fft_end)
    /* Postprocessing: add energies and store them */
    FDK_vld1(32, S11, r2)                                               /* S11: targetEne[erb] loaded */
    FDK_vpadd_s32(D26, D26, D26)                                        /* D26: res res */
    FDK_vadd_s32_d(D5, D5, D26)                                         /* S11: Add the calculated energy */
    FDK_subs_imm(r8, r8, 1)                                             /* erb-- */
    FDK_vst1_ia(32, S11, r2)                                            /* Store the target energy */

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
    FDK_vdup_d_reg(16, D30, r7)                                         /* D30: dmx_coeff_mtx dmx_coeff_mtx, r7 free to use again */
    FDK_vshll_s16_imm(Q15, D30, 16)                                     /* Q15: dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx */
    FDK_branch(AL, activeDmxProcess_STFT_func1_loop_dmx_iterations)

FDK_label(activeDmxProcess_STFT_func1_it1)
    FDK_mov_imm(r7, 58)
    FDK_ldr(r8, sp, 0x38, erb_is4GVH_H)
    FDK_sub(r8, r7, r8)
    FDK_ldr(r7, sp, 0x2C, dmxMatrixL_FDK)                               /* r7 = dmx_coeff_mtx = dmxMatrixL_FDK */
    FDK_vdup_d_reg(16, D30, r7)                                         /* D30: dmx_coeff_mtx dmx_coeff_mtx, r7 free to use again */
    FDK_vshll_s16_imm(Q15, D30, 16)                                     /* Q15: dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx dmx_coeff_mtx */
    FDK_branch(AL, activeDmxProcess_STFT_func1_loop_dmx_iterations)

FDK_label(activeDmxProcess_STFT_func1_end)
    /* Process loop run-out*/
    FDK_vmov_dreg(r0, r1, S2, S3)                                       /* r1: &realizedSig[0]         r0: &inputBuffer[0]  */
    FDK_add_imm(r0, r0, 4, 2)                                           /* r0: &inputBuffer[1]                              */
    FDK_vst1(32, S1, r0)                                                /* S1: inputBuffer[1] = savIm0                      */
    FDK_vshl_s32_d(D0, D0, D20)                                         /* D0,S1:  savIm0 << inBufStftHeadroom[erb]         */
    FDK_vqdmulh_s32_qq(D6, D0, D28)                                     /* D6,S13: (savIm0<<Hdr)*c   ------                 */
    FDK_vshl_s32_d(D0, D6, D2)                                          /* D0,S1: ((savIm0<<Hdr)*c)>>Hdr                    */
    FDK_vmov_reg(r4, S1)                                                /* r4: tmpIm */
    FDK_ldr(lr, r1, 4, realizedSig[1])                                  /* lr: realizedSig[1] */
    FDK_add(lr, lr, r4)                                                 /* lr: realizedSig[1] += tmpIm */
    FDK_str(lr, r1, 4, realizedSig[1])                                  /* lr: realizedSig[1] */

    FDK_vqdmulh_s32_dd(D6, D6, D6)                                      /* S13: fPow2(tmpIm) */
    FDK_vshl_s32_q(D6, D6, D24)                                         /* S13: fPow2(tmpIm) >> target_exp */
    FDK_vshr_s32_d_imm(D6, D6, 1)                                       /* S13: fPow2(tmpIm) >> (target_exp + 1) */

    FDK_vadd_s32_d(D6, D6, D5)                                          /* S13: Add targetEne from last iteration (S11) */
    FDK_sub_imm(r2, r2, 4, 2)                                           /* r2:  Decrement targetEneArr pointer by 1: -> &targetEneArr[57] */
    FDK_vst1(32, S13, r2)                                               /* S13: targetEneArr[57] += targetEne */

    FDK_mpop_pc(r4,r12)
FDK_ASM_ROUTINE_END()


#endif /* #ifdef FUNCTION_activeDmxProcess_STFT_func1 */


#ifdef FUNCTION_activeDmxProcess_STFT_func2

FDK_ASM_ROUTINE_START(void, activeDmxProcess_STFT_func2,
         (FIXP_DBL **  __restrict  realizedSig,     /* r0 */
          FIXP_DBL *   __restrict  targetEneArr,    /* r1 */
          INT *        __restrict  chOut_exp,       /* r2 */
          const UINT               numOutChans,     /* r3 */
          const INT *  __restrict  chOut_count,
          FIXP_DBL **  __restrict  h_realizedEnePrev,
          FIXP_DBL **  __restrict  h_targetEnePrev,
          const UINT               h_numErbBands,
          const UINT * __restrict  h_erbFreqIdx,
          const INT *  __restrict  erb_freq_idx_256_58_exp,
          const FIXP_DBL           Alpha,
          const FIXP_DBL           One_subAlpha,
                INT * __restrict realizedSigHeadroom,
                INT * __restrict exp_diff_realized,
                INT * __restrict exp_diff,
                INT * __restrict minHeadroomRealizedEne,
                INT * __restrict minHeadroomTargetEne))

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
     Q5: ---------------- reserved -----------------
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
     0x58:     minHeadroomTargetEne
     0x54:     minHeadroomRealizedEne
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
    FIXP_DBL          r10; /* = Alpha             ->Q14 */
    FIXP_DBL          r11; /* = One_subAlpha      ->Q15 */
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

  FDK_mvpush(Q6, Q7)                                  // Q6,7: saved on stack
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
      FDK_vhadd_s32_q(Q2, Q2, Q3)                     // Q2: (fPow2(tmpRe<<realizedSigHeadroom) + fPow2(tmpIm<<realizedSigHeadroom)) / 2
      FDK_vshl_s32_q(Q2, Q2, Q0)                      // Q2: realizEne3  realizEne2  realizEne1  realizEne0  >> chOut_exp
      FDK_vld1_2d(32, D6, D7, r10)                    // Q3: realEnePr3  realEnePr2  realEnePr1  realEnePr0 (=h->realizedEnePrev[chOut][erb])
      FDK_vqdmulh_s32_qq(Q2, Q2, Q14)                 // Q2: summand1 = fMult(Alpha, realizedEne)
      FDK_vqdmulh_s32_qq(Q3, Q3, Q15)                 // Q3: summand2 = fMult(One_subAlpha, h->realizedEnePrev[chOut][erb])
      FDK_vshl_s32_q(Q2, Q2, Q11)                     // Q2: summand1 >> -exp_diff_realized or 0
      FDK_vshl_s32_q(Q3, Q3, Q12)                     // Q3: summand2 >>  0 or exp_diff_realized
      FDK_vadd_s32_q(Q2, Q2, Q3)                      // Q2: realizEne3  realizEne2  realizEne1  realizEne0
      FDK_vst1_2d_ia(32, D4, D5, r10)                 // Q2: h->realizedEnePrev[chOut][erb] = realizedEne
      FDK_vmax_s32(128, Q7, Q7, Q2)                   // Q7: maxRealizedEne = fMax(maxRealizedEne, realizedEne)

      FDK_vld1_2d_ia(32, D4, D5, r1)                  // Q2: targetEne3  targetEne2  targetEne1  targetEne0
      FDK_vld1_2d(32, D6, D7, r11)                    // Q3: tgEnePrev3  tgEnePrev2  tgEnePrev1  tgEnePrev0
      FDK_vqdmulh_s32_qq(Q2, Q2, Q14)                 // Q2: summand1 = fMult(Alpha,        targetEnergy[erb])
      FDK_vqdmulh_s32_qq(Q3, Q3, Q15)                 // Q3: summand2 = fMult(One_subAlpha, h->targetEnePrev[chOut][erb])
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
  FDK_mvpop(Q6, Q7)
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
     Q3:   realSim3  realSim2  realSim1  realSim0   4 samples (imag) from realizedSig
     Q2:   realSre3  realSre2  realSre1  realSre0   4 samples (real) from realizedSig
     Q1:    EQ_exp3   EQ_exp2   EQ_exp1   EQ_exp0
     Q0:      EQ3      EQ2        EQ1      EQ0

     NEON register layout for second loop:
     lane:       3         2         1         0
     ----------------------------------------------
    [Q3:   realSim3  realSre3  realSim2  realSre2   2x2 samples (real/imag) from realizedSig]
     Q2:  [realSim1  realSre1] realSim0  realSre0  [1,2]x2 samples (real/imag) from realizedSig
     Q1:    EQ_expi   EQ_expi   EQ_expi   EQ_expi
     Q0:      EQi       EQi       EQi       EQi

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
    const INT                   numErbBands,
    const SCHAR    * __restrict inBufStftHeadroomPrev,
    const SCHAR    * __restrict realizedSigHeadroomPrev,
          INT                   eq_e)
{

    INT erb, EQ_exp;
    FIXP_DBL EQ;
    UINT fftBand = 0;
    UINT runs;

    /* Compute all 58 EQ coefficients in advance */
    for(erb = 0; erb < numErbBands; erb++)
    {
      INT diffExp = realizedSigHeadroomPrev[erb] - inBufStftHeadroomPrev[erb];
      EQ = computeEQAndClip(targetEne_ChOut[erb], realizedEne_ChOut[erb], diffExp, eq_e, &EQ_exp);
      EQ_vector[3*erb+0] = EQ;
      EQ_vector[3*erb+1] = EQ_exp;
      runs = h_erbFreqIdx[erb] - fftBand;
      EQ_vector[3*erb+2] = runs;
      fftBand += runs;
    }

    activeDmxProcess_STFT_neonv7_func3(realizedSig__chOut, EQ_vector, numErbBands);

}
#endif /* FUNCTION_activeDmxProcess_STFT_func3 */


#ifdef FUNCTION_activeDmxProcess_STFT_func4
FDK_ASM_ROUTINE_START(void ,activeDmxProcess_STFT_func4,
         (FIXP_DBL* __restrict EnergyPrev,   /* in/out */
    const FIXP_DBL* __restrict Energy,       /* in */
    const INT *     __restrict exp_diff,     /* in */
          INT*      __restrict minHeadroom,  /* in,out */
    const FIXP_DBL             Alpha,        /* in */
    const FIXP_DBL             One_subAlpha))/* in */

    /*
     NEON register layout:
     lane:     3          2          1          0
     ----------------------------------------------
     Q15:  1-Alpha    1-Alpha    1-Alpha    1-Alpha
     Q14:   Alpha      Alpha      Alpha      Alpha
     Q13:      1          1          1          1        <- constant
     Q12:      0          0          0          0        <- constant
     Q11:   clz[3]     clz[2]     clz[1]     clz[0]      <- clz(Ene)
     Q10:   Hdr[3]     Hdr[2]     Hdr[1]     Hdr[0]      <-> r3
     Q9 :  shl2[3]    shl2[2]     shl2[1]     shl2[0]    <- shl for summand2
     Q8:   shl1[3]    shl1[2]     shl1[1]     shl1[0]    <- shl for summand1
     Q7: ---------------- reserved -----------------
     Q6: ---------------- reserved -----------------
     Q5: ---------------- reserved -----------------
     Q4: ---------------- reserved -----------------
     Q3: sumand2[3]  sumand2[2]  sumand2[1]  sumand2[0]  <-(1-Alpha)*PrevNrg
     Q2: sumand1[3]  sumand1[2]  sumand1[1]  sumand1[0]  <- Alpha*Energy
     Q1: PrevNrg[3]  PrevNrg[2]  PrevNrg[1]  PrevNrg[0]  <-> r0
     Q0:  Energy[3]   Energy[2]   Energy[1]   Energy[0]  <- r1

    Stack contents:
    0x0C : One_subAlpha
    0x08 : Alpha
    0x04 : r4
    0x00 : r5
*/


#ifndef __ARM_NEON__
          FIXP_DBL* __restrict EnergyPrev = r0;
    const FIXP_DBL* __restrict Energy = r1;
    const INT* __restrict exp_diff = r2;
          INT* __restrict minHeadroom = r3;
#endif

  FDK_mpush(r4, r5)

  FDK_ldrd(r4, r5, sp, 0x08, Alpha, One_subAlpha)
  FDK_vmov_i32(128, Q12, 0)                   // Q12: 0 (duplicated)
  FDK_vmov_i32(128, Q13, 1)                   // Q13: 1 (duplicated)
  FDK_vdup_q_reg(32, Q14, r4)                 // Q14: Alpha (duplicated)
  FDK_vdup_q_reg(32, Q15, r5)                 // Q15: 1-Alpha (duplicated)

  FDK_mov_imm(r4, STFT_ERB_BANDS-2)           // r4: 58-2=56

FDK_label(activeDmxProcess_STFT_func4_loop4x)   // 56/4=14 iterations
    FDK_vld1_2d_ia(32, D0, D1, r1)              // Q0: Energy[3..0]
    FDK_vld1_2d(32, D2, D3, r0)                 // Q1: PrevNrg[3..0]
    FDK_vld1_2d_ia(32, D16, D17, r2)            // Q8: exp_diff[3..0]
    FDK_vld1_2d(32, D20, D21, r3)               // Q10: minHdr[3..0]
    FDK_vneg_q(32, Q9, Q8)                      // Q9:-exp_diff[3..0]
    FDK_vmin_s32(128, Q8, Q8, Q12)              // Q8: shl1[3..0]
    FDK_vmin_s32(128, Q9, Q9, Q12)              // Q9: shl2[3..0]
    FDK_vqdmulh_s32_qq(Q2, Q0, Q14)             // Q2: sum1=Energy*Alpha
    FDK_vqdmulh_s32_qq(Q3, Q1, Q15)             // Q3: sum1=PrevNrg*(1-Alpha)
    FDK_vshl_s32_q(Q2, Q2, Q8)                  // Q2: sum1 >>= sh1
    FDK_vshl_s32_q(Q3, Q3, Q9)                  // Q3: sum2 >>= sh2
    FDK_vadd_s32_q(Q1, Q2, Q3)                  // Q1: sum1>>sh1 + sum2>>sh2
    FDK_vclz_q(32, Q0, Q1)                      // Q0: clz[3..0]
    FDK_vsub_s32_q(Q0, Q0, Q13)                 // Q0: clz[3..0] - 1
    FDK_vmin_s32(128, Q10, Q10, Q0)             // Q10: fMin(minHdr, Hdr)[3..0]
    FDK_vst1_2d_ia(32, D2, D3, r0)              // Q1: update PrevNrg[3..0]
    FDK_vst1_2d_ia(32, D20, D21, r3)            // Q10: update minHdr[3..0]
    FDK_subs_imm(r4, r4, 4)
    FDK_branch(NE, activeDmxProcess_STFT_func4_loop4x)

  /* Process half loop body for last 2 erb */
  FDK_vld1_1d_ia(32, D0, r1)                    // D0: Energy[1,0]
  FDK_vld1_1d(32, D2, r0)                       // D2: PrevNrg[1,0]
  FDK_vld1_1d_ia(32, D16, r2)                   // D16: exp_diff[1,0]
  FDK_vld1_1d(32, D20, r3)                      // D20: minHdr[1,0]
  FDK_vneg_d(32, D18, D16)                      // D18:-exp_diff[1,0]
  FDK_vmin_s32(64, D16, D16, D24)               // D16: shl1[1,0]
  FDK_vmin_s32(64, D18, D18, D24)               // D18: shl2[1,0]
  FDK_vqdmulh_s32_dd(D4, D0, D28)               // D4: sum1=Energy*Alpha
  FDK_vqdmulh_s32_dd(D6, D2, D30)               // D6: sum1=PrevNrg*(1-Alpha)
  FDK_vshl_s32_d(D4, D4, D16)                   // D4: sum1 >>= sh1
  FDK_vshl_s32_d(D6, D6, D18)                   // D6: sum2 >>= sh2
  FDK_vadd_s32_d(D2, D4, D6)                    // D2: sum1>>sh1 + sum2>>sh2
  FDK_vclz_d(32, D0, D2)                        // D0: clz[1,0]
  FDK_vsub_s32_q(D0, D0, D26)                   // D0: clz[1,0] - 1
  FDK_vmin_s32(64, D20, D20, D0)                // D20: fMin(minHdr, Hdr)[1,0]
  FDK_vst1_1d_ia(32, D2, r0)                    // D2: update PrevNrg[1,0]
  FDK_vst1_1d_ia(32, D20, r3)                   // D20: update minHdr[1,0]

  FDK_mpop(r4, r5)

FDK_return()

FDK_ASM_ROUTINE_END()


#endif /* FUNCTION_activeDmxProcess_STFT_func4 */

#ifdef FUNCTION_activeDmxProcess_STFT_func5

FDK_ASM_ROUTINE_START(void, activeDmxProcess_STFT_func5,
  ( FIXP_DBL        * __restrict  realizedSig,              /*SI, SR*/    /*in*/
    INT             *             realizedSigHeadroom,      /*Hr*/        /*in*/
    FIXP_DBL        * __restrict  realizedEnergy,                         /*out*/
    UINT      const *             erb_freq_idx,                           /*in*/
    INT       const * __restrict  erb_freq_idx_exp,         /*fIdxExp*/   /*in*/
    INT       const               chOut_exp                 /*chOut_exp*/ /*in*/
))  
   /*
   NEON register layout for first loop:
   lane:       3         2         1         0
   ----------------------------------------------
   Q10:                       realSig1 realSig0   NyquistRe  DCfreqRe
   Q8:      hdr3      hdr2      hdr1      hdr0
   Q7:             ----- reserved -----
   Q6:             ----- reserved -----
   Q5:             ----- reserved -----
   Q4:             ----- reserved -----
   Q3:   realSim3  realSim2  realSim1  realSim0   4 samples (imag) from realizedSig
   Q2:   realSre3  realSre2  realSre1  realSre0   4 samples (real) from realizedSig
   Q1:  -chOutExp -chOutExp -chOutExp -chOutExp   negated chOutExp
   Q0:   (unused)

   NEON register layout for second loop:
   lane:       3         2         1         0
   ----------------------------------------------
   Q11:  realEne3  realEne2  realEne1  realEne0   accu for realizedEnergy
   Q10:                       realSig1 realSig0   NyquistRe  DCfreqRe
   Q9:     mask3     mask2     mask1   FFFFFFFF   mask for unused energy values in 1st iteration
   Q8:   headroom  headroom  headroom  headroom   4 x realizedSigHeadroom[erb]
   Q7:             ----- reserved -----
   Q6:             ----- reserved -----
   Q5:             ----- reserved -----
   Q4:             ----- reserved -----
   Q3:   realSim3  realSre3  realSim2  realSre2   2x2 samples (real/imag) from realizedSig[8*i+4]
   Q2:   realSim1  realSre1  realSim0  realSre0   2x2 samples (real/imag) from realizedSig]8*i+0]
   Q1:  -chOutExp -chOutExp -chOutExp -chOutExp   4 x negated chOutExp
   Q0:  -realExp  -realExp  -realExp  -realExp    4 x neagted (chOutExp + realExp) 
  */

/*Stack contents :
  0x1C : chOut_exp
  0x18 : erb_freq_idx_exp
  0x14 : r4
  0x10 : r5
  0x0C : r6
  0x08 : r7
  0x04 : r8
  0x00 : r9
*/


#ifndef __ARM_NEON__
        FIXP_DBL  * __restrict  r0 = realizedSig;
        INT       *             r1 = realizedSigHeadroom;
        FIXP_DBL  * __restrict  r2 = realizedEnergy;
        INT       *             r3 = erb_freq_idx;
  const INT       * __restrict  r4; /*  erb_freq_idx_exp; */
        INT                     r5; /*  chOut_exp; */
        INT                     r6; /* fft loop: 32 (= 4 << 3) default pointer increment (r0) */
        INT                     r7; /* 1st loop: erb/fftBand counter  32 .. 0, 2nd loop: erb counter (58-33) ... 0 */
        INT                     r8; /* fft loop: erb_freq_idx[erb] - erb_freq_idx[erb - 1] */
        INT                     r9; /* fft loop: fftBand = erb_freq_idx[erb - 1] */
#endif
  
  FDK_mpush(r4, r9)
  FDK_ldrd(r4, r5, sp, 0x18, erb_freq_idx_exp, chOut_exp)

  FDK_add_imm(r3, r3, 128, 2)                         // r3: &erb_freq_idx[32]
  FDK_add_imm(r4, r4, 132, 2)                         // r4: &erb_freq_idx_exp[33]

  FDK_mov_imm(r7, 32)                                 // r7 : 32 = number of samples for 1st loop
  FDK_vdup_q_reg(32, Q1, r5)                          // Q1 : 4 x (chOut_exp)
  FDK_vneg_q(32, Q1, Q1)                              // Q1 : 4 x (-chOut_exp)

  /* DC frequency, only real part */
  FDK_vld1_1d_ia(32, D20, r0)                         // D20: --------          --------               realSig[1]        realSig[0]      //realizedSig
  FDK_vld1_ia(32, S32, r1)                            // D16: --------          --------               --------           Hr0            //realizedSigHeadroom
  FDK_vshl_s32_d(D0, D20, D16)                        // D0 : --------          --------               --------          SR0<<Hr0
  FDK_vqdmulh_s32_dd(D0, D0, D0)                      // D0 : --------          --------               --------          (SR0<<Hr0)^2
  FDK_vshr_s32_d_imm(D0, D0, 1)                       // D0 : --------          --------               --------          ((SR0<<Hr0)^2)>>1
  FDK_vshl_s32_d(D0, D0, D2)                          // D0 : --------          --------               --------          (((SR0<<Hr0)^2)>>1)>>chOutExp
  FDK_vst1_ia(32, S0, r2)                             // Store realizedEnergy[0]
 
  /*----- Process erb [1..32] each with only 1 fft band -----*/
FDK_label(activeDmxProcess_STFT_func5_loop4x_erb32)   // 32/4=8 iterations
  FDK_vld2_4d_ia(32, D4, D5, D6, D7, r0)              // Q2 : SR4               SR3                     SR2               SR1            //realizedSig[erb].real
                                                      // Q3 : SI4               SI3                     SI2               SI1            //realizedSig[erb].imag
  FDK_vld1_2d_ia(32, D16, D17, r1)                    // Q8 : Hr4               Hr3                     Hr2               Hr1            //realizedSigHeadroom
  FDK_vshl_s32_q(Q2, Q2, Q8)                          // Q2 : SR4<<Hr4          SR3<<Hr3                SR2<<Hr2          SR1<<Hr1
  FDK_vshl_s32_q(Q3, Q3, Q8)                          // Q3 : SI4<<Hr4          SI3<<Hr3                SI2<<Hr2          SI1<<Hr1
  FDK_vqdmulh_s32_qq(Q2, Q2, Q2)                      // Q2 : (SR4<<Hr4)^2      (SR3<<Hr3)^2            (SR2<<Hr2)^2      (SR1<<Hr1)^2
  FDK_vqdmulh_s32_qq(Q3, Q3, Q3)                      // Q3 : (SI4<<Hr4)^2      (SI3<<Hr3)^2            (SI2<<Hr2)^2      (SI1<<Hr1)^2
  FDK_vhadd_s32_q(Q2, Q2, Q3)                         // Q2 : 4 x (tmpRe + tmpIm) / 2
  FDK_vshl_s32_q(Q2, Q2, Q1)                          // Q2  : 4 x ((tmpRe + tmpIm) / 2) >> chOutExp
  FDK_vst1_2d_ia(32, D4, D5, r2)                      // Store realizedENergy[1..4]
  FDK_subs_imm(r7, r7, 4)
  FDK_branch(NE, activeDmxProcess_STFT_func5_loop4x_erb32)


  /*----- Process erb [33..57] with 3..19 fft bands -----*/
  /*Init and check for remaining 1,2,3 fft bands*/
  FDK_mov_imm(r7, STFT_ERB_BANDS - 33)                // r7 : STFT_ERB_BANDS - 33
FDK_label(activeDmxProcess_STFT_func5_loop4x_erbx)
  FDK_vld1dup_2d_ia(32, D0, D1, r4)                   // Q0 : 4 x erb_freq_idx_exp[erb]
  FDK_vsub_s32_q(Q0, Q1, Q0)                          // Q0 : 4 x (-cOutExp-erb_freq_idx_exp[erb])
  FDK_ldr_ia(r9, r3, 4)                               // r9 : erb_freq_idx[erb-1]
  FDK_ldr_ia(r8, r3, 0)                               // r8 : erb_freq_idx[erb]
  FDK_sub(r8, r8, r9)                                 // r8 : loops = erb_freq_idx[erb] - erb_freq_idx[erb-1]
  FDK_vmov_i32(128, Q11, 0)                           // Q11: 0x00000000   0x00000000   0x00000000   0x00000000  <- realizedEnergy[erb]
  FDK_and_imm(r9, r8, 3)                              // r9 : loops & 3
  FDK_tst_imm(r9, 3)
  FDK_mov_imm(r6, 32)                                 // r6 : 32 (= 4 << 3) default pointer increment in erb loop
  FDK_vmvn_i32(128, Q9, 0)                            // Q9 : 0xFFFFFFFF   0xFFFFFFFF   0xFFFFFFFF   0xFFFFFFFF
  FDK_vld1dup_2d_ia(32, D16, D17, r1)                 // Q8 : Hr[erb]      Hr[erb]      Hr[erb]      Hr[erb]     <-realizedSigHeadroom[erb]
  FDK_branch(EQ, activeDmxProcess_STFT_func5_loop4x_erbx_innerloop) // no 1,2,3 bands to do? jump to innerloop

    /*Prepare V5 matrix for AND operation for fft bands 1,2,3*/
  FDK_cmp_imm(r9, 2)
  FDK_branch(GT, activeDmxProcess_STFT_func5_lastmask)
  FDK_vext_q(8, Q9, Q9, Q11, 4)                        // Q9 : 0x00000000   0xFFFFFFFF   0xFFFFFFFF   0xFFFFFFFF
  FDK_branch(EQ, activeDmxProcess_STFT_func5_lastmask)
  FDK_vext_q(8, Q9, Q9, Q11, 4)                        // Q9 : 0x00000000   0x00000000   0xFFFFFFFF   0xFFFFFFFF
FDK_label(activeDmxProcess_STFT_func5_lastmask)       
  FDK_vext_q(8, Q9, Q9, Q11, 4)                        // Q9 : 0x00000000   0x00000000   0x00000000   0xFFFFFFFF
  FDK_lsl_imm(r6, r9, 3)                               // r6 : (r8 & 3) << 3

  /* 1st iteration: Process first 1,2,3 or 4 samples of 3..19 fft bands */
  /* following iterations: Process remaining n*4 fftBands of erb */
FDK_label(activeDmxProcess_STFT_func5_loop4x_erbx_innerloop)
    FDK_vld2_4d_pu(32,D4, D5, D6, D7, r0, r6)           // Q2 : SR[2*fftband+3] SR[2*fftband+2] SR[2*fftband+1] SR[2*fftband+0] // realizedSig_real
                                                        // Q3 : SI[2*fftband+3] SI[2*fftband+2] SI[2*fftband+1] SI[2*fftband+0] // realizedSig_imag
    FDK_vshl_s32_q(Q2, Q2, Q8)                          // Q2 :   SR4<<Hr4        SR3<<Hr3        SR2<<Hr2          SR1<<Hr1
    FDK_vshl_s32_q(Q3, Q3, Q8)                          // Q3 :   SI4<<Hr4        SI3<<Hr3        SI2<<Hr2          SI1<<Hr1
    FDK_vqdmulh_s32_qq(Q2, Q2, Q2)                      // Q2 : (SR4<<Hr4)^2    (SR3<<Hr3)^2    (SR2<<Hr2)^2      (SR1<<Hr1)^2
    FDK_vqdmulh_s32_qq(Q3, Q3, Q3)                      // Q3 : (SI4<<Hr4)^2    (SI3<<Hr3)^2    (SI2<<Hr2)^2      (SI1<<Hr1)^2
    FDK_vhadd_s32_q(Q2, Q2, Q3)                         // Q2 : 4 x (Re^2 + Im^2) / 2
    FDK_vshl_s32_q(Q2, Q2, Q0)                          // Q2 : 4 x (Re^2 + Im^2)/2)>>realExp
    FDK_vand(128, Q2, Q2, Q9)                           // Q2 : V2 & V9 <- mask out unused energy values in 1st iteration
    FDK_vadd_s32_q(Q11, Q11, Q2)                        // Q11:  realEne3        realEne2        realEne1        realEne0 
    FDK_mov_imm(r6, 32)                                 // r6 : 32 (pointer increment register)
    FDK_vdup_q_32(Q9, S36)                              // Q9 : 0xFFFFFFFF   0xFFFFFFFF   0xFFFFFFFF   0xFFFFFFFF
    FDK_subs_imm(r8, r8, 4)
    FDK_branch(GT, activeDmxProcess_STFT_func5_loop4x_erbx_innerloop)
  
  FDK_vpadd_s32(D22, D22, D23)                          // D22:    realEne3+realEne1   realEne2+realEne0
  FDK_vpadd_s32(D22, D22, D22)                          // D22.0:  realEne = realEne3+realEne1+realEne2+realEne0
  FDK_vst1_ia(32, S44, r2)                              // Store realizedEnergy[erb]
  FDK_subs_imm(r7, r7, 1)
  FDK_branch(NE, activeDmxProcess_STFT_func5_loop4x_erbx)

  /* Add Nyquist energy to last erb
     Nyquist value in V6.1  -> EXT to element V6.0
     headroom in V4
     realizedEnergy[erb] in V7.0
     add Nyquist-Energy
     decrement pointer X2
     overwrite value with new Energy
  */
                                                        // erb:    last used erb
  FDK_vext_q(8, Q10, Q10, Q10, 4)                       // Q10.0:  realSig[1]    //realizedSig
  FDK_vshl_s32_q(Q2, Q10,  Q8)                          // Q2.0 :  Re=realSig[1] << Hr[erb]
  FDK_vqdmulh_s32_qq(Q2, Q2, Q2)                        // Q2.0 : (realSig[1] << Hr[erb])^2
  FDK_vshr_s32_q_imm(Q2, Q2, 1)                         // Q2.0 : (Re^2) / 2
  FDK_vshl_s32_q(Q2, Q2, Q0)                            // Q2.0 : (Re^2)/2)>>realExp
  FDK_vadd_s32_q(Q11, Q11, Q2)                          // Q11.0: realizedEnergy[erb]+Nyquist energy
  FDK_sub_imm(r2, r2, 4, 0)                             // r2 : &realizedEnergy[--erb]
  FDK_vst1(32, S44, r2)                                 // Store realizedEnergy[erb]+Nyquist energy
  FDK_mpop(r4, r9)
  FDK_return()

FDK_ASM_ROUTINE_END()

#endif /*FUNCTION_activeDmxProcess_STFT_func5*/
#endif /* __ARM_NEON__ */

#ifndef RAM_BYTEALIGN
  #if defined (_MSC_VER)
 /* VIsual Studio */
    #define RAM_BYTEALIGN(n) __declspec(align(n))
  #else
  /* FDK_TOOLCHAIN=gcc */
    #define RAM_BYTEALIGN(n) __attribute__((aligned(n)))
  #endif
#else
  /* This alignment is probably limited to 8 bytes  */
  /* In some cases, when load/stores of Q registers */
  /* are used, this isn't sufficient, needs usually */
  /* 16-byte alignment for full speed performance   */
  #define RAM_BYTEALIGN(x) RAM_ALIGN
#endif

#if defined(__ARM_AARCH64_NEON__)
#include "arm/FDK_aarch64_neon_funcs.h"
#ifndef __ARM_AARCH64_NEON__
  /* Activate ARM NEON simulation for development in Visual Studio */
  #include "arm/FDK_aarch64_neon_regs.h"
#endif

#define FUNCTION_activeDmxProcess_STFT_func1
#define FUNCTION_activeDmxProcess_STFT_func3  
#define FUNCTION_activeDmxProcess_STFT_func4
#define FUNCTION_activeDmxProcess_STFT_func5

#ifdef  FUNCTION_activeDmxProcess_STFT_func1
A64_ASM_ROUTINE_START(void, activeDmxProcess_STFT_func1,
     (FIXP_DBL * __restrict inputBuffer,       /* X0 */
      FIXP_DBL * __restrict realizedSig,       /* X1 */
      FIXP_DBL * __restrict targetEneArr,      /* X2 */
const UINT     *            erbFreqIdx,        /* X3 */
const FIXP_DBL *            eq_ptr,            /* X4 */
const FIXP_DMX_H            dmxMatrixL_FDK,    /* X5 */
const FIXP_DMX_H            dmxMatrixH_FDK,    /* X6 */
const UINT                  erb_is4GVH_L,      /* X7 */
const UINT                  erb_is4GVH_H,
const INT                   chOut_exp,
const INT                   dmx_iterations,
      INT      *            inBufStftHeadroom,
const INT      *            erb_freq_idx_256_58_exp))


#ifndef __ARM_AARCH64_NEON__
    /* Assign call parameter 1..8 to registers */
      FIXP_DBL * X0 = inputBuffer;
      FIXP_DBL * X1 = realizedSig;
      FIXP_DBL * X2 = targetEneArr;
const UINT     * X3 = erbFreqIdx;      /* erb_freq_idx_256_58 */
const FIXP_DBL * X4 = eq_ptr;
      FIXP_DBL W5 = (FIXP_DBL) dmxMatrixL_FDK;  /* dmxMatrixL_FDK[chOut] */
      FIXP_DBL W6 = (FIXP_DBL) dmxMatrixH_FDK;  /* dmxMatrixH_FDK[chOut] */
      UINT     W7 = erb_is4GVH_L;

      /* Reserve more registers (caller saved) for further parameters */
      INT      W8;  /* = iteration: 0 .. dmx_iteration, counting upwards */
      UINT     W9;  /* = erb_is4GVH_H */
      INT      W10; /* = chOut_exp, inner loop counter */
      INT      W11; /* = dmx_iterations */
      INT *    X12; /* = inBufStftHeadroom */
const INT *    X13; /* = erb_freq_idx_256_58_exp */
      UINT     W14; /* = max_erb: erb_is4GVH_L, erb_is4GVH_H, 58 */
      UINT     W15; /* = erb loops: counting downwards */

      /* Callee-saved registers */
      UINT     W19; /* = max_fftBand */
      UINT     W20; /* = (last) max fftBand */
      UINT     W21; /* = pointer update 2nd loop */
      UINT     W22; /* = dummy register */
#endif

    /* Load W9..X13 from stack */
    A64_ldr_Wt_I(W9,  SP, 0x00, erb_is4GVH_H)
    A64_ldr_Wt_I(W10, SP, 0x08, chOut_exp)
    A64_ldr_Wt_I(W11, SP, 0x10, dmx_iterations)
    A64_ldp_Xt_I(X12, X13, SP, 0x18, inBufStftHeadroom, erb_freq_idx_256_58_exp)

    A64_lsl_Wt(W5, W5, 16)                      // dmxMatrixL_FDK: Convert SGL -> DBL
    A64_lsl_Wt(W6, W6, 16)                      // dmxMatrixH_FDK: Convert SGL -> DBL

    A64_pushp(X21, X22)                         // push X21,X22 are callee-saved
    A64_pushp(X19, X20)                         // push X19,X20 are callee-saved
    A64_pushp(X0,  X1)                          // push X0: inputBuffer X1: realizedSig => for post-processing Nyquist data

    A64_sub_Wt(W10, WZR, W10)                   // W10: -chOut_exp
    A64_dup_Wt(32, 128, V20_4S, W10)            // V20: -chOut_exp (duplicated)
    A64_dup_Wt(32, 128, V19_4S, W5)             // V19: dmx_coeff_mtx = dmxMatrixL_FDK (duplicated)

    A64_movi(32, 128, V23_4S, 0)                // V23: 0x00000000 (duplicated)
                                             
    A64_ldr_Wt_I(W21, X0, 4, inputBuffer[1])    // W21: FIXP_DBL savIm0 = inputBuffer[1];
    A64_mov_Wt_to_lane(32, 128, V24_S, 0, W21)  // V24: ---- ---- ---- savIm0
    A64_mov_Wt_imm(W22, 0)
    A64_str_Wt_I(W22, X0, 4, inputBuffer[1])    // inputBuffer[1] = 0

    A64_asr_Wt_imm(W15, W7, 2)                  // W15: erb_is4GVH_L / 4
    A64_cmp_Wt_imm(W15, 8)
    A64_branch(LE, activeDmxProcess_STFT_func1_W15_set)
        A64_mov_Wt_imm(W15, 8)                  // W15: maximum is 8 -> 32 samples
A64_label(activeDmxProcess_STFT_func1_W15_set)
    A64_adds_Wt_lsl_imm(W15, WZR, W15, 2)       // W15: runs = 4*n in range [0,4..32]
    A64_mov_Xt(W20, W15)                        // W20: fftBand (for 2nd loop)
    A64_branch(EQ, activeDmxProcess_STFT_func1_first32Samples_end)

    A64_sub_Wt(W14, W7, W15)                    // W14: max_erb for 2nd loop: erb_is4GVH_L - W15
    A64_adds_Wt_lsl_imm(W10,  WZR, W15, 2)      // W10: 4*runs+ in range [4...128]
    A64_add_Xt_sxtw(X13, X13, W10, 2)           // X13: erb_freq_idx_256_58_exp += runs
    A64_add_Xt_sxtw(X3, X3, W10, 2)             // X3: erbFreqIdx += runs

A64_label(activeDmxProcess_STFT_func1_first32)
      A64_ld1x1_IA(32, 128, V18_4S, X4, 16)        // V18: eq_ptr[3]  eq_ptr[2]  eq_ptr[1]  eq_ptr[0]
      A64_ld2x2_IA(32, 128, V0_4S, V1_4S, X0, 32)  // V0:   bufRe3     bufRe2     bufRe1     bufRe0
                                                   // V1:   bufIm3     bufIm2     bufIm1     bufIm0
      A64_ld2x2(32, 128, V2_4S, V3_4S, X1)         // V2:   SigRe3     SigRe2     SigRe1     SigRe0
                                                   // V3:   SigIm3     SigIm2     SigIm1     SigIm0 
      A64_ld1x1_IA(32, 128, V22_4S, X12, 16)       // V22: inputHdr3  inputHdr2  inputHdr1  inputHdr0
      A64_sqdmulh(32, 128, V18_4S, V18_4S, V19_4S) // V18:  dmxcff3    dmxcff2    dmxcff1    dmxcff0
      A64_sshl(32, 128, V0_4S, V0_4S, V22_4S)      // V0: bufRe<<inputHdr ...
      A64_sshl(32, 128, V1_4S, V1_4S, V22_4S)      // V1: bufIm<<inputHdr ...
      A64_sub(32, 128, V21_4S, V23_4S, V22_4S)     // V21:-inputHdr3 -inputHdr2 -inputHdr1 -inputHdr0
      A64_sqdmulh(32, 128, V4_4S, V0_4S, V18_4S)   // V4:   tmpRe3     tmpRe2     tmpRe1     tmpRe0
      A64_sqdmulh(32, 128, V5_4S, V1_4S, V18_4S)   // V5:   tmpIm3     tmpIm2     tmpIm1     tmpIm0
      A64_sshl(32, 128, V0_4S, V4_4S, V21_4S)      // V0:tmpRe3>>Hdr tmpRe2>>Hdr tmpRe1>>Hdr tmpRe0>>Hdr
      A64_sshl(32, 128, V1_4S, V5_4S, V21_4S)      // V1:tmpIm3>>Hdr tmpIm2>>Hdr tmpIm1>>Hdr tmpIm0>>Hdr
      A64_add(32, 128, V2_4S, V2_4S, V0_4S)        // V2:    SigRe3     SigRe2     SigRe1     SigRe0
      A64_add(32, 128, V3_4S, V3_4S, V1_4S)        // V3:    SigIm3     SigIm2     SigIm1     SigIm0
      A64_st2x2_IA(32, 128, V2_4S, V3_4S, X1, 32)  // Store 4x realizedSig interleaved re/im
      A64_ld1x1(32, 128, V16_4S, X2)               // V16: trgtEne3 trgtEne2 trgtEne1 trgtEne0
      A64_sqdmulh(32, 128, V4_4S, V4_4S, V4_4S)    // V4:    tmpRe3^2   tmpRe2^2   tmpRe1^2   bufRe0^2
      A64_sqdmulh(32, 128, V5_4S, V5_4S, V5_4S)    // V5:    tmpIm3^2   tmpIm2^2   tmpIm1^2   bufIm0^2
      A64_sshl(32, 128, V4_4S, V4_4S, V20_4S)      // V4: Re=tmpRe^2>>(trgExp = chOut_exp[chOut])
      A64_sshl(32, 128, V5_4S, V5_4S, V20_4S)      // V5: Im=tmpIm^2>>(trgExp = chOut_exp[chOut])
      A64_shadd(32, 128, V4_4S, V4_4S, V5_4S)      // V4: (Re+Im)/2
      A64_add(32, 128, V16_4S, V16_4S, V4_4S)      // V16: trgtEne += (Re+Im)/2
      A64_st1x1_IA(32, 128, V16_4S, X2, 16)        // store V16: trgtEne3 trgtEne2 trgtEne1 trgtEne0
      A64_subs_imm(W15, W15, 4)
      A64_branch(NE, activeDmxProcess_STFT_func1_first32)

A64_label(activeDmxProcess_STFT_func1_first32Samples_end)

    /* remaining erb: 58 - 33 = 25 in W14 (depends upon erb_is4GVH_L) */
    A64_mov_Wt_imm(W8, 0)                           // W8: iteration = 0 
    A64_adds_Wt_lsl_imm(W15, WZR, W14, 0)           // W15: remaining erb iterations == 0 ?
    A64_branch(EQ, activeDmxProcess_STFT_func1_loop_erb_end)

    /* Outermost loop with dmx_iterations */
A64_label(activeDmxProcess_STFT_func1_loop_dmx_iterations)
      /* Middle loop over erb [32 ..57] with [1..19] fftBands */

A64_label(activeDmxProcess_STFT_func1_loop_erb)
      /* Preload targetEnergy[erb] into V16.0 */
      A64_movi(32, 128, V16_4S, 0)                    // V16:      0          0          0         0
      A64_ld1_lane(32, V16_S, 0, X2)                  // V16:      0          0          0     trgEne[erb]

      A64_ld1rx1_IA(32, 128, V22_4S, X12, 4)          // V22:  inputHdr   inputHdr   inputHdr   inputHdr 
      A64_sub(32, 128, V21_4S, V23_4S, V22_4S)        // V21: -inputHdr  -inputHdr  -inputHdr  -inputHdr 

      A64_ldr_Wt_IA(W19, X3, 4)                       // W19: maxfftBand = erbFreqIdx[erb]
      A64_ld1rx1_IA(32, 128, V18_4S, X4, 4)           // V18: eq_ptr[erb] (duplicated)
      A64_sqdmulh(32, 128, V18_4S, V18_4S, V19_4S)    // V18: dmx_coeff   (duplicated)

      A64_ld1rx1_IA(32, 128, V17_4S, X13, 4)          // V17: freqexp       (duplicated)
      /* target_exp = erb_freq_idx_256_58_exp[erb] + chOut_exp[chOut] (we need it negated) */
      A64_sub(32, 128, V17_4S, V20_4S, V17_4S)        // V17: target_exp (negated) = -chOut_exp - erb_freq_idx_256_58_exp[erb]

      A64_sub_Wt(W10, W19, W20)                       // W10: fftBands to process in range [1..19]

      A64_movi(8, 128, V25_16B, 0xFF)                 // V25: 0xFFFFFFFF (duplicated)

      /* Round-up W10 to n*4 each odd number of fftBands e.g. 5 -> 8 */
      A64_ands_Wt_imm(W21, W10, 3)                    // W21 in range [0,1,2,3]
      A64_branch(EQ, activeDmxProcess_STFT_func1_set_fft4x)
      /* W21 in range [1,2,3] */
      A64_cmp_Wt_imm(W21, 2)
      A64_branch(GT, activeDmxProcess_STFT_func1_lastmask)
      A64_ext(8, 128, V25_16B, V25_16B, V23_16B, 4)   // V25: 0000 FFFF FFFF FFFF
      A64_branch(EQ, activeDmxProcess_STFT_func1_lastmask)
      A64_ext(8, 128, V25_16B, V25_16B, V23_16B, 4)   // V25: 0000 0000 FFFF FFFF
A64_label(activeDmxProcess_STFT_func1_lastmask)
      A64_ext(8, 128, V25_16B, V25_16B, V23_16B, 4)   // V25: 0000 0000 0000 FFFF

//A64_label(activeDmxProcess_STFT_func1_setincr)
      A64_sub_Wt(W10, W10, W21)
      A64_add_Wt_imm(W10, W10, 4)
      A64_lsl_Wt_imm(W21, W21, 3)                     // W21: 1,2,3->8,16,24
      A64_sxtw_Xt(X21, W21)
      A64_branch(AL, activeDmxProcess_STFT_func1_loop4x)

A64_label(activeDmxProcess_STFT_func1_set_fft4x)
      A64_mov_Wt_imm(X21, 32)                         // W21: 32

        /* Inner loop over [1..19] fftBands, with common dmx_coeff, headroom, target_exp etc. */
        /* during 1st iteration, up to 3 upper lanes might be not significant and get masked. */
A64_label(activeDmxProcess_STFT_func1_loop4x)
        A64_ld2x2_PU(32, 128, V0_4S, V1_4S, X0, X21)  // V0:   bufRe3     bufRe2      bufRe1     bufRe0
                                                      // V1:   bufIm3     bufIm2      bufIm1     bufIm0
        A64_ld2x2(32, 128, V2_4S, V3_4S, X1)          // V2:   SigRe3     SigRe2      SigRe1     SigRe0
                                                      // V3:   SigIm3     SigIm2      SigIm1     SigIm0
        /* Here we have to apply the active-lane-masks (V25) for the 1st iteration */
        A64_and(8, 128, V0_16B, V0_16B, V25_16B)
        A64_and(8, 128, V1_16B, V1_16B, V25_16B)
        A64_sshl(32, 128, V0_4S, V0_4S, V22_4S)       // V0: bufRe<<inputHdr ...
        A64_sshl(32, 128, V1_4S, V1_4S, V22_4S)       // V1: bufIm<<inputHdr ...
        A64_sqdmulh(32, 128, V4_4S, V0_4S, V18_4S)    // V4:   tmpRe3     tmpRe2     tmpRe1     tmpRe0
        A64_sqdmulh(32, 128, V5_4S, V1_4S, V18_4S)    // V5:   tmpIm3     tmpIm2     tmpIm1     tmpIm0
        A64_sshl(32, 128, V0_4S, V4_4S, V21_4S)       // V0:tmpRe3>>Hdr tmpRe2>>Hdr tmpRe1>>Hdr tmpRe0>>Hdr
        A64_sshl(32, 128, V1_4S, V5_4S, V21_4S)       // V1:tmpIm3>>Hdr tmpIm2>>Hdr tmpIm1>>Hdr tmpIm0>>Hdr
        A64_add(32, 128, V2_4S, V2_4S, V0_4S)         // V2:    SigRe3     SigRe2     SigRe1     SigRe0
        A64_add(32, 128, V3_4S, V3_4S, V1_4S)         // V3:    SigIm3     SigIm2     SigIm1     SigIm0
        A64_st2x2_PU(32, 128, V2_4S, V3_4S, X1, X21)  // Store 4x realizedSig interleaved re/im
        A64_sqdmulh(32, 128, V4_4S, V4_4S, V4_4S)     // V4:    tmpRe3^2   tmpRe2^2   tmpRe1^2   bufRe0^2
        A64_sqdmulh(32, 128, V5_4S, V5_4S, V5_4S)     // V5:    tmpIm3^2   tmpIm2^2   tmpIm1^2   bufIm0^2
        A64_sshl(32, 128, V4_4S, V4_4S, V17_4S)       // V4: Re=tmpRe^2>>trgExp
        A64_sshl(32, 128, V5_4S, V5_4S, V17_4S)       // V5: Im=tmpIm^2>>trgExp
        A64_shadd(32, 128, V4_4S, V4_4S, V5_4S)       // V4: (Re+Im)/2
        A64_add(32, 128, V16_4S, V16_4S, V4_4S)       // V16: trgtEne+=(Re+Im)/2
        A64_movi(8, 128, V25_16B, 0xFF)               // V25: 0xFFFFFFFF (duplicated)
        A64_mov_Wt_imm(W21, 32)                       // W21: 32
        A64_subs_imm(W10, W10, 4)                     // W10: fftBand loop counter -= 4
        A64_branch(NE, activeDmxProcess_STFT_func1_loop4x)
    
    A64_subs_imm(W15, W15, 1)                         // W15:  erb--
    A64_addv(32, 128, S16, V16_4S)                    // S16: targetEnergy[erb] = targetEne
    A64_st1_lane_IA(32, V16_S, 0, X2, 4)              // Store the target energy
    A64_mov_Wt(W20, W19)                              // W20: fftBand
    A64_branch(NE, activeDmxProcess_STFT_func1_loop_erb)                /* Go back to beginning of erb loop */

A64_label(activeDmxProcess_STFT_func1_loop_erb_end)
        A64_add_Wt_imm(W8, W8, 1)
        A64_cmp_Wt(W11, W8)
        A64_branch(EQ, activeDmxProcess_STFT_func1_end) // If last iteration reached

        A64_cmp_Wt_imm(W8, 1)
        A64_branch(NE, activeDmxProcess_STFT_func1_it1)

//A64_label(activeDmxProcess_STFT_func1_it0)
        A64_sub_Wt(W15, W9, W7)                    // W15: erb_is4GVH_H - erb_is4GVH_L
        A64_dup_Wt(32, 128, V19_4S, W6)            // V19: dmx_coeff_mtx = dmxMatrixH_FDK (duplicated)
        A64_branch(AL, activeDmxProcess_STFT_func1_loop_dmx_iterations)

A64_label(activeDmxProcess_STFT_func1_it1)
       A64_mov_Wt_imm(W7, 58)                     // W7: 58
       A64_sub_Wt(W15, W7, W9)                    // W15: 58 - erb_is4GVH_H
       A64_dup_Wt(32, 128, V19_4S, W5)            // V19: dmx_coeff_mtx = dmxMatrixL_FDK (duplicated)
       A64_branch(AL, activeDmxProcess_STFT_func1_loop_dmx_iterations)

A64_label(activeDmxProcess_STFT_func1_end)
    A64_popp(X0, X1)                                 // X0 (inputBuffer), X1 (realizedSig)
    A64_add_Xt_imm(X0, X0, 4)                        // X0: &inputBuffer[1]
    A64_st1_lane(32, V24_S, 0, X0)                   // Store inputBuffer[1] = savIm0

    // erb--;
    // tmpIm = fMult(dmx_coeff, savIm0 << inBufStftHeadroom[erb]);
    // realizedSig_chOut[1] += (tmpIm >> inBufStftHeadroom[erb]);
    // targetEnergy[erb] += (fPow2Div2(tmpIm) >> target_exp);

    A64_sshl(32, 64, V1_2S, V24_2S, V22_2S)          // V1.0: savIm0<<inHdr
    A64_sqdmulh(32, 64, V5_2S, V1_2S, V18_2S)        // V5.0:   tmpIm=fMult(dmx_coeff,savIm << inBufStftHeadroom[erb])
    A64_sshl(32, 64, V1_2S, V5_2S, V21_2S)           // V1.0:   tmpIm>>inHdr
    A64_add_Xt_imm(X1, X1, 4)                        // X1: &realizedSig[1]
    A64_ld1_lane(32, V3_S, 0, X1)                    // V3.0: realizedSig[1]
    A64_add(32, 64, V3_2S, V3_2S, V1_2S)             // V3.0:   SigIm0
    A64_st1_lane(32, V3_S, 0, X1)                    // Store [imag]: realizedSig[1]

    A64_sqdmulh(32, 64, V5_2S, V5_2S, V5_2S)         // V5.0: fPow2(tmpIm)
    A64_sshl(32, 64, V5_2S, V5_2S, V17_2S)           // V5.0: Im=tmpIm^2>>trgExp
    A64_shadd(32, 64, V4_2S, V5_2S, V23_2S)          // V4,0: Im/2
    A64_add(32, 64, V16_2S, V16_2S, V4_2S)           // V16.0: trgtEnergy +=Im/2
    A64_sub_Xt_imm(X2, X2, 4)                        // X2: &targetEnergy[erb]
    A64_st1_lane(32, V16_S, 0, X2)                   // Store targetEnergy[erb] (updated)
    A64_popp(X19, X20)                               // restore X19, X20
    A64_popp(X21, X22)                               // restore X21, X22

A64_ASM_ROUTINE_END()
#endif /* FUNCTION_activeDmxProcess_STFT_func1 */

#ifdef FUNCTION_activeDmxProcess_STFT_func3



A64_ASM_ROUTINE_START(void, activeDmxProcess_STFT_func3_neonv8,
  (FIXP_DBL * realizedSig__chOut,
   INT *      EQ_vector,
   INT        numErbBands))

    /*
       NEON register layout for first loop:
       lane:       3         2         1         0
       ----------------------------------------------
       V18:    ----      ----   realSig[1] realSig[0]
       V17:                     EQ_exp[57] EQ_exp[0]
       V16:                       EQ[57]    EQ[0]
       V3:   realSim3  realSim2  realSim1  realSim0   4 samples (imag) from realizedSig
       V2:   realSre3  realSre2  realSre1  realSre0   4 samples (real) from realizedSig
       V1:    EQ_exp3   EQ_exp2   EQ_exp1   EQ_exp0
       V0:      EQ3      EQ2        EQ1      EQ0

       NEON register layout for second loop:
       lane:       3         2         1         0
       ----------------------------------------------
       V3:   realSim3  realSre3  realSim2  realSre2   2x2 samples (real/imag) from realizedSig[8*i+4]
       V2:   realSim1  realSre1  realSim0  realSre0   2x2 samples (real/imag) from realizedSig]8*i+0]
       V1:    EQ_expi   EQ_expi   EQ_expi   EQ_expi
       V0:      EQi       EQi       EQi       EQi

       Core register usage:
       X0: realizedSig            FIXP_DBL*
       X1: EQ_vector              (FIXP_DBL / INT / INT) *
       W2: erb                    INT
       W3: runs                   INT
       X4: copy of realizedSig__chOut (X0)

       Stack contents:
        - none -
    */


#ifndef __ARM_AARCH64_NEON__
  /* Assign call parameter to registers */
  FIXP_DBL *X0 = realizedSig__chOut;
  INT      *X1 = EQ_vector;
  INT       W2 = numErbBands;
  INT       W3;
  FIXP_DBL *X4;
#endif
  /* Preprocess erb=0 */
  A64_ld2_lane_IA(32, V16_S, V17_S, 0, X1, 8)               // V16.0: EQ[0]   V17.0: EQ_exp[0]
  A64_ldr_Wt_IA(W3, X1, 4)                                  // W3: runs = 1 , just increment X1 to next triplet
  A64_mov_Xt(X4, X0)                                        // X4: &realizedSig[0]
  A64_ld1x1_IA(32, 64, V18_2S, X0, 8)                       // V18: ----  ----  imSig0    reSig0

  /* Process erb=1..32 */
  A64_mov_Wt_imm(W3, 32)
A64_label(activeDmxProcess_STFT_func3_neonv8_loop1)
    A64_ld3x3_IA(32, 128, V0_4S, V1_4S, V2_4S, X1, 48)      // V2:  runs3  runs2  runs1  runs0  <- don't care
                                                            // V1:  exp3   exp2   exp1   exp0
                                                            // V0:   EQ3    EQ2    EQ1    EQ0
    A64_ld2x2(32, 128, V2_4S, V3_4S, X0)                    // V3:  imSi3  imSi2  imSi1  imSi0
                                                            // V2:  reSi3  reSi2  reSi1  reSi0
    A64_sqdmulh(32, 128, V2_4S, V2_4S, V0_4S)               // V2: all reSi*EQ
    A64_sqdmulh(32, 128, V3_4S, V3_4S, V0_4S)               // V3: all imSi*EQ
    A64_sshl(32, 128, V2_4S, V2_4S, V1_4S)                  // V2: all reSi<<exp
    A64_sshl(32, 128, V3_4S, V3_4S, V1_4S)                  // V3: all imSi<<exp
    A64_st2x2_IA(32, 128, V2_4S, V3_4S, X0, 32)             // store 4x: realizedSig__chOut[2*fftBand+1]  realizedSig__chOut[2*fftBand+0]
    A64_subs_imm(W3, W3, 4)
    A64_branch(NE, activeDmxProcess_STFT_func3_neonv8_loop1)

  /* Process erb=33..57 */
  A64_sub_Wt_imm(W2, W2, 33)
A64_label(activeDmxProcess_STFT_func3_neonv8_loop2)
    A64_ld2_lane_IA(32, V0_S, V1_S, 0, X1, 8)               // V0.0: EQ   V1.0: EQ_exp
    A64_ldr_Wt_IA(W3, X1, 4)                                // W3: runs
    A64_dup_lane(32, 128, V0_4S, V0_S, 0)                   // V0: EQ duplicated 4x
    A64_dup_lane(32, 128, V1_4S, V1_S, 0)                   // V1: EQ_exp duplicated 4x

    A64_tbz_Wt(W3, 0, activeDmxProcess_STFT_func3_neonv8_loop2_chk2x)
//A64_label(activeDmxProcess_STFT_func3_neonv8_loop2_1x)
    /* Process 1 iteration of inner loop */
    A64_ld1x1(32, 64, V2_2S, X0)                            // V2: ----  ----  imSig    reSig
    A64_sqdmulh(32, 64, V2_2S, V2_2S, V0_2S)                // V2: ----  ---- imSig*EQ reSig*EQ
    A64_sshl(32, 64, V2_2S, V2_2S, V1_2S)                   // V2: ----  ---- imSig*EQ<<exp reSig*EQ<<exp
    A64_st1x1_IA(32, 64, V2_2S, X0, 8)                      // store 1x: realizedSig__chOut[2*fftBand+1]  realizedSig__chOut[2*fftBand+0] 

A64_label(activeDmxProcess_STFT_func3_neonv8_loop2_chk2x)
    A64_tbz_Wt(W3, 1, activeDmxProcess_STFT_func3_neonv8_loop2_chk4x)

//A64_label(activeDmxProcess_STFT_func3_neonv8_loop2_2x)
    /* Process 2 iterations of inner loop */
    A64_ld1x1(32, 128, V2_4S, X0)                           // V2: imSi1  reSi1  imSi0  reSi0
    A64_sqdmulh(32, 128, V2_4S, V2_4S, V0_4S)               // V2: all multiplied by EQ
    A64_sshl(32, 128, V2_4S, V2_4S, V1_4S)                  // V2: all shifted-left by EQ_exp
    A64_st1x1_IA(32, 128, V2_4S, X0, 16)                    // store 2x: realizedSig__chOut[2*fftBand+1]  realizedSig__chOut[2*fftBand+0] 

A64_label(activeDmxProcess_STFT_func3_neonv8_loop2_chk4x)
    A64_asr_Wt_imm(W3, W3, 2)
    A64_cmp_Wt_imm(W3, 0)
    A64_branch(EQ, activeDmxProcess_STFT_func3_neonv8_loop2_end)

A64_label(activeDmxProcess_STFT_func3_neonv8_loop2_4x)
    /* Process 4 iterations of inner loop */
    A64_ld1x2(32, 128, V2_4S, V3_4S, X0)                    // V3: imSi3  reSi3  imSi2  reSi2
                                                            // V2: imSi1  reSi1  imSi0  reSi0
    A64_sqdmulh(32, 128, V2_4S, V2_4S, V0_4S)               // V2: all multiplied by EQ
    A64_sqdmulh(32, 128, V3_4S, V3_4S, V0_4S)               // V3: all multiplied by EQ
    A64_sshl(32, 64, V2_4S, V2_4S, V1_4S)                   // V2: all shifted-left by EQ_exp
    A64_sshl(32, 64, V3_4S, V3_4S, V1_4S)                   // V3: all shifted-left by EQ_exp
    A64_st1x2_IA(32, 128, V2_4S, V3_4S, X0, 32)             // store 4x: realizedSig__chOut[2*fftBand+1]  realizedSig__chOut[2*fftBand+0] 
    A64_subs_imm(W3, W3, 1)
    A64_branch(NE, activeDmxProcess_STFT_func3_neonv8_loop2_4x)

A64_label(activeDmxProcess_STFT_func3_neonv8_loop2_end)
  A64_subs_imm(W2, W2, 1)
  A64_branch(NE, activeDmxProcess_STFT_func3_neonv8_loop2)

//A64_label(activeDmxProcess_STFT_func3_postprocess)
  /* Postprocess erb=0 (DC), erb=58 (Nyquist) */
  A64_sub_Xt_imm(X1, X1, 12)     // Reset X1 to &EQ_vector[3*(numErbBands -1=57)]
  A64_ld2_lane_IA(32, V16_S, V17_S, 1, X1, 8)               // V16.1: EQ57    V17.1: EQ_exp57
  A64_sqdmulh(32, 64, V18_2S, V18_2S, V16_2S)               // V18: ----  ---- imSig0*EQ57        reSig0*EQ0
  A64_sshl(32, 64, V18_2S, V18_2S, V17_2S)                  // V18: ----  ---- imSig0*EQ57<<exp57 reSig0*EQ0<<exp0
  A64_st1x1_IA(32, 64, V18_2S, X4, 8)                       // store 1x:   realizedSig__chOut[1]  realizedSig__chOut[0] 

A64_ASM_ROUTINE_END()


static void activeDmxProcess_STFT_func3(
    FIXP_DBL* __restrict targetEne_ChOut,
    FIXP_DBL* __restrict realizedSig__chOut,
    FIXP_DBL* __restrict realizedEne_ChOut,
    FIXP_DBL* __restrict EQ_vector,
    const UINT* __restrict h_erbFreqIdx,
    const INT                numErbBands,
    const SCHAR* __restrict inBufStftHeadroomPrev,
    const SCHAR* __restrict realizedSigHeadroomPrev,
    INT                      eq_e)
{
    INT erb, EQ_exp;
    for (erb = 0; erb < 33; erb++)
    {
        INT diffExp = realizedSigHeadroomPrev[erb] - inBufStftHeadroomPrev[erb];
        FIXP_DBL EQ = computeEQAndClip(targetEne_ChOut[erb], realizedEne_ChOut[erb], diffExp, eq_e, &EQ_exp);
        EQ_vector[3 * erb + 0] = EQ;
        EQ_vector[3 * erb + 1] = EQ_exp;
        EQ_vector[3 * erb + 2] = 1;
    }
    UINT fftBand = erb;
    for (; erb < STFT_ERB_BANDS; erb++)
    {
        INT diffExp = realizedSigHeadroomPrev[erb] - inBufStftHeadroomPrev[erb];
        FIXP_DBL EQ = computeEQAndClip(targetEne_ChOut[erb], realizedEne_ChOut[erb], diffExp, eq_e, &EQ_exp);
        UINT runs = erb_freq_idx_256_58[erb] - fftBand;
        fftBand += runs;
        EQ_vector[3 * erb + 0] = EQ;
        EQ_vector[3 * erb + 1] = EQ_exp;
        EQ_vector[3 * erb + 2] = runs;  /* in range [3..19] */
    }
    activeDmxProcess_STFT_func3_neonv8(realizedSig__chOut, (INT *) EQ_vector, STFT_ERB_BANDS);
}

#endif /* FUNCTION_activeDmxProcess_STFT_func3 */


#ifdef FUNCTION_activeDmxProcess_STFT_func4

A64_ASM_ROUTINE_START(void, activeDmxProcess_STFT_func4,
    (       FIXP_DBL *  __restrict  EnergyPrev,     /* in/out */
      const FIXP_DBL *  __restrict  Energy,         /* in */
      const INT *       __restrict  exp_diff,       /* in */
            INT *       __restrict  minHeadroom,    /* in,out */
      const FIXP_DBL                Alpha,          /* in */
      const FIXP_DBL                One_subAlpha))  /* in */
  
  
#ifndef __ARM_AARCH64_NEON__
        FIXP_DBL  * X0 = EnergyPrev;
  const FIXP_DBL  * X1 = Energy;
  const INT       * X2 = exp_diff;
        INT       * X3 = minHeadroom;
  const FIXP_DBL    W4 = Alpha;
  const FIXP_DBL    W5 = One_subAlpha;
        INT         W6;
#endif


  A64_movi(32, 128, V12_4S, 0)                  // V12: 0 (duplicated)
  A64_movi(32, 128, V13_4S, 1)                  // V13: 1 (duplicated)
  A64_dup_Wt(32, 128, V14_4S, W4)               // V14: Alpha (duplicated)
  A64_dup_Wt(32, 128, V15_4S, W5)               // V15: 1-Alpha (duplicated)
  A64_mov_Wt_imm(W6, STFT_ERB_BANDS - 2)        // W6: 58-2=56

A64_label(activeDmxProcess_STFT_func4_loop4x)   // 56/4=14 iterations
    A64_ld1x1_IA(32, 128, V0_4S, X1, 16)        // V0_4S: Energy[3..0]
    A64_ld1x1(32, 128, V1_4S, X0)               // V1_4S: PrevNrg[3..0]
    A64_ld1x1_IA(32, 128, V8_4S, X2, 16)        // V8_4S: exp_diff[3..0]<
    A64_ld1x1(32, 128, V10_4S, X3)              // V10_4S: minHdr[3..0]
    A64_sub(32, 128, V9_4S, V12_4S, V8_4S)      // V9_4S:-exp_diff[3..0]
    A64_smin(32, 128, V8_4S, V8_4S, V12_4S)     // V8_4S: shl1[3..0]
    A64_smin(32, 128, V9_4S, V9_4S, V12_4S)     // V9_4S: shl2[3..0]
    A64_sqdmulh(32, 128, V2_4S, V0_4S, V14_4S)  // V2_4S: sum1=Energy*Alpha
    A64_sqdmulh(32, 128, V3_4S, V1_4S, V15_4S)  // V3_4S: sum1=PrevNrg*(1-Alpha)
    A64_sshl(32, 128, V2_4S, V2_4S, V8_4S)      // Q2_4S: sum1 >>= sh1
    A64_sshl(32, 128, V3_4S, V3_4S, V9_4S)      // V3_4S: sum2 >>= sh2
    A64_add(32, 128, V1_4S, V2_4S, V3_4S)       // V1_4S: sum1>>sh1 + sum2>>sh2
    A64_clz(32, 128, V0_4S, V1_4S)              // V0_4S: clz[3..0]
    A64_sub(32, 128, V0_4S, V0_4S, V13_4S)      // V0_4S: clz[3..0] - 1
    A64_smin(32, 128, V10_4S, V10_4S, V0_4S)    // V10_4S: fMin(minHdr, Hdr)[3..0]
    A64_st1x1_IA(32, 128, V1_4S, X0, 16)        // V1_4S: update PrevNrg[3..0]
    A64_st1x1_IA(32, 128, V10_4S, X3, 16)       // V10_4S: update minHdr[3..0]
    A64_subs_imm(W6, W6, 4)
    A64_branch(NE, activeDmxProcess_STFT_func4_loop4x)
    
    /* Process half loop body for last 2 erb */
    A64_ld1x1_IA(32, 64, V0_2S, X1, 8)            // V0_2S: Energy[1,0]
    A64_ld1x1(32, 64, V1_2S, X0)                  // V1_2S: PrevNrg[1,0]
    A64_ld1x1_IA(32, 64, V8_2S, X2, 8)            // V8_2S: exp_diff[1,0]
    A64_ld1x1(32, 64, V10_2S, X3)                 // V10_2S: minHdr[1,0]
    A64_sub(32, 64, V9_2S, V12_2S, V8_2S)         // V9_2S:-exp_diff[1,0]
    A64_smin(32, 64, V8_2S, V8_2S, V12_2S)        // V8_2S: shl1[3..0]
    A64_smin(32, 64, V9_2S, V9_2S, V12_2S)        // V9_2S: shl2[3..0]
    A64_sqdmulh(32, 64, V2_2S, V0_2S, V14_2S)     // V2_2S: sum1=Energy*Alpha
    A64_sqdmulh(32, 64, V3_2S, V1_2S, V15_2S)     // V3_2S: sum1=PrevNrg*(1-Alpha)
    A64_sshl(32, 64, V2_2S, V2_2S, V8_2S)         // Q2_4S: sum1 >>= sh1
    A64_sshl(32, 64, V3_2S, V3_2S, V9_2S)         // V3_2S: sum2 >>= sh2
    A64_add(32, 64, V1_2S, V2_2S, V3_2S)          // V1_2S: sum1>>sh1 + sum2>>sh2
    A64_clz(32, 64, V0_2S, V1_2S)                 // V0_2S: clz[3..0]
    A64_sub(32, 64, V0_2S, V0_2S, V13_2S)         // V0_2S: clz[3..0] - 1
    A64_smin(32, 64, V10_2S, V10_2S, V0_2S)       // V10_2S: fMin(minHdr, Hdr)[3..0]
    A64_st1x1_IA(32, 64, V1_2S, X0, 8)            // V1_2S: update PrevNrg[3..0]
    A64_st1x1_IA(32, 64, V10_2S, X3, 8)           // V10_2S: update minHdr[3..0]

A64_return()

A64_ASM_ROUTINE_END()

#endif /*FUNCTION_activeDmxProcess_STFT_func4*/

#ifdef FUNCTION_activeDmxProcess_STFT_func5

/*(realizedSig[chOut], realizedSigHeadroom, realizedEnergy, erb_freq_idx_256_58, erb_freq_idx_256_58_exp, chOut_exp[chOut])*/

A64_ASM_ROUTINE_START(void, activeDmxProcess_STFT_func5,
  ( FIXP_DBL        * __restrict  realizedSig,              /*SI, SR*/    /*in*/
    INT             *             realizedSigHeadroom,      /*Hr*/        /*in*/
    FIXP_DBL        * __restrict  realizedEnergy,                         /*out*/
    UINT      const * __restrict  erb_freq_idx,                           /*in*/
    INT       const * __restrict  erb_freq_idx_exp,         /*fIdxExp*/   /*in*/
    INT       const               chOut_exp                 /*chOut_exp*/ /*in*/
))  
   /*
   NEON register layout for first loop:
   lane:       3         2         1         0
   ----------------------------------------------
   V6:                        realSig1 realSig0   NyquistRe  DCfreqRe
   V4:      hdr3      hdr2      hdr1      hdr0
   V3:   realSim3  realSim2  realSim1  realSim0   4 samples (imag) from realizedSig
   V2:   realSre3  realSre2  realSre1  realSre0   4 samples (real) from realizedSig
   V1:  -chOutExp -chOutExp -chOutExp -chOutExp   negated chOutExp
   V0:   (unused)

   NEON register layout for second loop:
   lane:       3         2         1         0
   ----------------------------------------------
   V7:   realEne3  realEne2  realEne1  realEne0   accu for realizedEnergy
   V6:                        realSig1 realSig0   NyquistRe  DCfreqRe
   V5:     mask3     mask2     mask1   FFFFFFFF   mask for unused energy values in 1st iteration
   V4:   headroom  headroom  headroom  headroom   4 x realizedSigHeadroom[erb]
   V3:   realSim3  realSre3  realSim2  realSre2   2x2 samples (real/imag) from realizedSig[8*i+4]
   V2:   realSim1  realSre1  realSim0  realSre0   2x2 samples (real/imag) from realizedSig]8*i+0]
   V1:  -chOutExp -chOutExp -chOutExp -chOutExp   negated chOutExp
   V0:  
  */

#ifndef __ARM_AARCH64_NEON__
        FIXP_DBL  * __restrict  X0 = realizedSig;
        INT       *             X1 = realizedSigHeadroom;
        FIXP_DBL  * __restrict  X2 = realizedEnergy;
  const UINT      * __restrict  X3 = erb_freq_idx;
  const INT       * __restrict  X4 = erb_freq_idx_exp;
        INT                     W5 = chOut_exp;
        INT64                   X6;
        INT                     W7;
        INT                     W8;
        INT64                   W9;
#endif
  
  A64_add_Xt_imm(X3, X3, 128 /*32<<2*/)
  A64_add_Xt_imm(X4, X4, 132 /*33<<2*/)

  A64_mov_Wt_imm(W7, 32)                              // W7 : 32
  A64_movi(32, 128, V4_16B, 0)                        // V4 : 0                 0                     0                 0
  A64_dup_Wt(32, 128, V1_4S, W5)                      // V1 : 4 x (chOut_exp)
  A64_sub(32, 128, V1_4S, V4_4S, V1_4S)               // V1 : 4 x (0-chOut_exp)

  /* DC frequency, only real part */
  A64_ld1x1_IA(32, 64, V6_2S, X0, 8)                  // V6 : 0                 0                       realSig[1]        realSig[0]      //realizedSig
  A64_ld1_lane_IA(32, V4_S, 0, X1, 4)                 // V4 : 0                 0                       0                 Hr0            //realizedSigHeadroom
  A64_sshl(32,  64, V0_2S, V6_2S, V4_2S)              // V0 : 0                 0                       --------          SR0<<Hr0
  A64_sqdmulh(32,  64, V0_2S, V0_2S, V0_2S)           // V0 : 0                 0                       --------          (SR0<<Hr0)^2
  A64_sshr_imm(32,  64, V0_2S, V0_2S, 1)              // V0 : 0                 0                       --------          ((SR0<<Hr0)^2)>>1
  A64_sshl(32, 128, V0_4S, V0_4S, V1_4S)              // V0 : 0                 0                       --------          (((SR0<<Hr0)^2)>>1)>>chOutExp
  A64_st1_lane_IA(32, V0_S, 0, X2, 4)                 // Store realizedEnergy[0]
 
  /*----- Process erb [1..32] each with only 1 fft band -----*/
A64_label(activeDmxProcess_STFT_func5_loop4x_erb32)   // 32/4=8 iterations
  A64_ld2x2_IA(32, 128, V2_4S, V3_4S, X0, 32)         // V2 : SR4               SR3                     SR2               SR1            //realizedSig[erb].real
                                                      // V3 : SI4               SI3                     SI2               SI1            //realizedSig[erb].imag
  A64_ld1x1_IA(32, 128, V4_4S, X1, 16)                // V4 : Hr4               Hr3                     Hr2               Hr1            //realizedSigHeadroom
  A64_sshl(32, 128, V2_4S, V2_4S, V4_4S)              // V2 : SR4<<Hr4          SR3<<Hr3                SR2<<Hr2          SR1<<Hr1
  A64_sshl(32, 128, V3_4S, V3_4S, V4_4S)              // V3 : SI4<<Hr4          SI3<<Hr3                SI2<<Hr2          SI1<<Hr1
  A64_sqdmulh(32, 128, V2_4S, V2_4S, V2_4S)           // V2 : (SR4<<Hr4)^2      (SR3<<Hr3)^2            (SR2<<Hr2)^2      (SR1<<Hr1)^2
  A64_sqdmulh(32, 128, V3_4S, V3_4S, V3_4S)           // V3 : (SI4<<Hr4)^2      (SI3<<Hr3)^2            (SI2<<Hr2)^2      (SI1<<Hr1)^2
  A64_shadd(32, 128, V2_4S, V2_4S, V3_4S)             // V2 : 4 x (tmpRe + tmpIm) / 2
  A64_sshl(32, 128, V2_4S, V2_4S, V1_4S)              // V2  : 4 x ((tmpRe + tmpIm) / 2) >> chOutExp
  A64_st1x1_IA(32, 128, V2_4S, X2, 16)                // Store realizedENergy[1..4]
  A64_subs_imm(W7, W7, 4)
  A64_branch(NE, activeDmxProcess_STFT_func5_loop4x_erb32)


  /*----- Process erb [33..57] with 3..19 fft bands -----*/
  /*Init and check for remaining 1,2,3 fft bands*/
  A64_mov_Wt_imm(W7, STFT_ERB_BANDS - 33)             // W7 : STFT_ERB_BANDS - 33
A64_label(activeDmxProcess_STFT_func5_loop4x_erbx)
  A64_ld1rx1_IA(32, 128, V0_4S, X4, 4)                // V0 : 4 x erb_freq_idx_exp[erb]
  A64_sub(32, 128, V0_4S, V1_4S, V0_4S)               // V0 : 4 x (-cOutExp-erb_freq_idx_exp[erb]
  A64_ldr_Wt_IA(W9, X3, 4)                            // W9 : erb_freq_idx[erb-1]
  A64_ldr_Wt(W8, X3)                                  // W8 : erb_freq_idx[erb]
  A64_sub_Wt(W8, W8, W9)                              // W8 : loops = erb_freq_idx[erb] - erb_freq_idx[erb-1]
  A64_movi(8, 128, V7_4S, 0)                          // V7 : 0x00000000   0x00000000   0x00000000   0x00000000  <- realizedEnergy[erb]
  A64_ands_Wt_imm(W9, W8, 3)                          // W9 : loops & 3
  A64_mov_Xt_imm(X6, 32)                              // X6 : 32 (= 4 << 3) default pointer increment in erb loop
  A64_movi(8, 128, V5_16B, 0xFF)                      // V5 : 0xFFFFFFFF   0xFFFFFFFF   0xFFFFFFFF   0xFFFFFFFF
  A64_ld1rx1_IA(32, 128, V4_4S, X1, 4)                // V4 : Hr[erb]      Hr[erb]      Hr[erb]      Hr[erb]      //realizedSigHeadroom
  A64_branch(EQ, activeDmxProcess_STFT_func5_loop4x_erbx_innerloop) // no 1,2,3 bands to do? jump to innerloop

    /*Prepare V5 matrix for AND operation for fft bands 1,2,3*/
  A64_cmp_Wt_imm(W9, 2)
  A64_branch(GT, activeDmxProcess_STFT_func5_lastmask)
  A64_ext(8, 128, V5_16B, V5_16B, V7_16B, 4)          // V5 : 0x00000000   0xFFFFFFFF   0xFFFFFFFF   0xFFFFFFFF
  A64_branch(EQ, activeDmxProcess_STFT_func5_lastmask)
  A64_ext(8, 128, V5_16B, V5_16B, V7_16B, 4)          // V5 : 0x00000000   0x00000000   0xFFFFFFFF   0xFFFFFFFF
A64_label(activeDmxProcess_STFT_func5_lastmask)       
  A64_ext(8, 128, V5_16B, V5_16B, V7_16B, 4)          // V5 : 0x00000000   0x00000000   0x00000000   0xFFFFFFFF
  A64_lsl_Wt_imm(W9, W9, 3)
  A64_sxtw_Xt(X6, W9)                                 // X6 : (W8 & 3) << 3

  /* 1st iteration: Process first 1,2,3 or 4 samples of 3..19 fft bands */
  /* following iterations: Process remaining n*4 fftBands of erb */
A64_label(activeDmxProcess_STFT_func5_loop4x_erbx_innerloop)
    A64_ld2x2_PU(32, 128, V2_4S, V3_4S, X0, X6)         // V2 : SR[2*fftband+3]   SR[2*fftband+2]         SR[2*fftband+1]   SR[2*fftband+0]    //realizedSig_real
                                                        // V3 : SI[2*fftband+3]   SI[2*fftband+2]         SI[2*fftband+1]   SI[2*fftband+0]    //realizedSig_imag
    A64_sshl(32, 128, V2_4S, V2_4S, V4_4S)              // V2 : SR4<<Hr4          SR3<<Hr3                SR2<<Hr2          SR1<<Hr1
    A64_sshl(32, 128, V3_4S, V3_4S, V4_4S)              // V3 : SI4<<Hr4          SI3<<Hr3                SI2<<Hr2          SI1<<Hr1
    A64_sqdmulh(32, 128, V2_4S, V2_4S, V2_4S)           // V2 :(SR4<<Hr4)^2      (SR3<<Hr3)^2            (SR2<<Hr2)^2      (SR1<<Hr1)^2
    A64_sqdmulh(32, 128, V3_4S, V3_4S, V3_4S)           // V3 :(SI4<<Hr4)^2      (SI3<<Hr3)^2            (SI2<<Hr2)^2      (SI1<<Hr1)^2
    A64_shadd(32, 128, V2_4S, V2_4S, V3_4S)             // V2 : 4 x (Re^2 + Im^2) / 2
    A64_sshl(32, 128, V2_4S, V2_4S, V0_4S)              // V2 : 4 x (Re^2 + Im^2)/2)>>realExp
    A64_and(8, 128, V2_16B, V2_16B, V5_16B)             // V2 : V2 & V5 <- mask out unused energy vaues in 1st iteration
    A64_add(32, 128, V7_4S, V7_4S, V2_4S)               // V7 :  realEne3    realEne2     realEne1     realEne0 
    A64_mov_Xt_imm(X6, 32)                              // X6 : 32 (pointer increment register)
    A64_movi(8, 128, V5_16B, 0xFF)                      // V5 : 0xFFFFFFFF   0xFFFFFFFF   0xFFFFFFFF   0xFFFFFFFF
    A64_subs_imm(W8, W8, 4)
    A64_branch(GT, activeDmxProcess_STFT_func5_loop4x_erbx_innerloop)
  
  A64_addv(32, 128, S7, V7_4S)                          // S7 : realEne
  A64_st1_lane_IA(32, V7_S, 0, X2, 4)                   // Store realizedEnergy[erb]

  A64_subs_imm(W7, W7, 1)
  A64_branch(NE, activeDmxProcess_STFT_func5_loop4x_erbx)

  /* Add Nyquist energy to last erb
     Nyquist value in V6.1  -> EXT to element V6.0
     headroom in V4
     realizedEnergy[erb] in V7.0
     add Nyquist-Energy
     decrement pointer X2
     overwrite value with new Energy
  */
  A64_ext(8,128, V6_16B, V6_16B, V6_16B, 4)             // V6 : 0                 0                       0                SR1              //realizedSig
  A64_sshl(32, 128, V2_4S, V6_4S, V4_4S)                // V2 : 0                 0                       0                (SR1<<Hr[erb])
  A64_sqdmulh(32, 128, V2_4S, V2_4S, V2_4S)             // V2 : 0                 0                       0                (SR1<<Hr[erb])^2
  A64_sshr_imm(32, 128, V2_4S, V2_4S, 1)                // V2 : 0                 0                       0                (Re^2) / 2
  A64_sshl(32, 128, V2_4S, V2_4S, V0_4S)                // V2 : 0                 0                       0                (Re^2)/2)>>realExp
  A64_add(32, 128, V7_4S, V7_4S, V2_4S)                 // V7 : 0                 0                       0                realizedEnergy[erb]+Nyquist energy
  A64_sub_Xt_imm(X2, X2, 4)
  A64_st1_lane(32, V7_S, 0, X2)                         // Store realizedEnergy[erb]+Nyquist energy
A64_return()

A64_ASM_ROUTINE_END()

#endif /*FUNCTION_activeDmxProcess_STFT_func5*/
#endif /* #ifdef __ARM_AARCH64_NEON__ */
