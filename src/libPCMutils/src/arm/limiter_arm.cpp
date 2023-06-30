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

/**************************** PCM utility library ******************************

   Author(s):   Arthur Tritthart, Fabian Bauer

   Description: Hard limiter for clipping prevention

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#ifndef __ARM_NEON__
#include "../../../FDK_tools/include/arm/FDK_neon_regs.h"
#endif

#if (PCM_LIM_BITS == 32)
  #define FUNCTION_applyLimiter_func3

  #ifdef FUNCTION_applyLimiter_func1
  #define FUNCTION_applyLimiter_func1_DBL
  #endif

  #ifdef FUNCTION_applyLimiter_func3
  #define FUNCTION_applyLimiter_func3_DBL
  #endif

#endif /* (PCM_LIM_BITS == 32) */


#endif /* defined(__ARM_NEON__)  */



#ifdef FUNCTION_applyLimiter_func1_DBL

FDK_ASM_ROUTINE_START(void, applyLimiter_func1,
   (FIXP_DBL*     p_delayBuf,
    PCM_LIM*      samplesIn,              /* 32    bit */
    INT_PCM*      samplesOut,             /* 16/32 bit */
    FIXP_DBL      additionalGain,
    UINT          additionalGainAvailable,
    FIXP_DBL      gain,
    INT           scaling,
    UINT          channels
    ))
  /* stack contents:
       0x20:     channels
       0x1C:     scaling
       0x18:     gain
       0x14:     additionalGainAvailable
       0x10:     r4
       0x0C:     r5
       0x08:     r6
       0x04:     r7
       0x00:     r8

     register contents:
       r0: p_delayBuf
       r1: samplesIn
       r2: samplesOut
       r3: additionalGain
       r4: additionalGainAvailable
       r5: gain
       r6: scaling
       r7: channels



     NEON registers:
       Q0: DelayBuff 1st 4 Samples
       Q1: Delay Buf 2nd 4 Samples
       Q2: samples in 1st 4
       Q3: samples in 2nd 4
       Q4: scaling
       Q5: rounding constant
       Q8: gain
       Q9: additional Gain
  */

#ifndef __ARM_NEON__
  FIXP_DBL  * r0 = p_delayBuf;
  PCM_LIM   * r1 = samplesIn;
  INT_PCM   * r2 = samplesOut;
  FIXP_DBL    r3 = additionalGain;
  UINT        r4;
  FIXP_DBL    r5;
  INT         r6;
  UINT        r7;
  INT         r8;
#endif

  FDK_mpush(r4, r8)

  FDK_ldrd(r6, r7, sp, 0x1C, scaling, channels)             // r6: scaling  r7: channels
  FDK_ldrd(r4, r5, sp, 0x14, additionalGainAvailable, gain) // r4: additionalGainAvailable  r5: gain

  FDK_mvpush(Q4, Q5)

  FDK_mov_imm(r8, 0x00008000)                               // rounding constant
  FDK_vdup_q_reg(32, Q5, r8)                                // duplicate rounding constant into all 4 Lanes of Q5


  FDK_vmov_i32(128, Q2, 0)                                  // Clear samples in Q2
  FDK_vmov_i32(128, Q0, 0)                                  // Clear samples in Q0
  FDK_vdup_q_reg(32, Q8, r5)                                // Q8: duplicated gain
  FDK_vshr_s32_q_imm(Q8, Q8, 1)                             // Q8: duplicated gain/2

  FDK_add_imm(r6, r6, 1, 0)                                 // scaling += 1
  FDK_vdup_q_reg(32, Q4, r6)                                // duplicate scaling into all 4 Lanes of Q4

  FDK_vdup_q_reg(32, Q9, r3)                                // duplicate additional gain into all 4 Lanes of Q9

  FDK_cmp_imm(r4, 0)
  FDK_branch(EQ, applyLimiter_func1_no_addGain)             // apply no additionalGain

  FDK_movs_asr_imm(r7, r7, 1)                               // r7: length / 2
  FDK_branch(CC, applyLimiter_func1_1_2x)

//FDK_label(applyLimiter_func1_1_1x)
  FDK_vld1(32, S0, r0)                                      // S0: read DelayBuff
  FDK_vqdmulh_s32_dd(D0, D0, D16)                           // DelayBuff * gain/2
  FDK_vqshl_s32(64, D0, D0, D8)                             // shl_sat DelayBuff
  FDK_vld1_ia(32, S8, r1)                                   // read SamplesIn
  FDK_vqdmulh_s32_dd(D4, D4, D18)                           // SamplesIn * addGain
  FDK_vst1_ia(32, S8, r0)                                   // store SamplesIn * addGain in DelayBuff
#if (SAMPLE_BITS == 16)
  FDK_vqadd_s32_d(D0, D0, D10)                              // D0[0,1] = sat(D0[0,1] + 0x00008000)
  FDK_vst1_ia(16, D0_1, r2)                                 // store shl_sat(DelayBuf * gain) in SamplesOut
#else
  FDK_vst1_ia(32, S0, r2)                                   // store shl_sat(DelayBuf * gain) in SamplesOut
#endif

FDK_label(applyLimiter_func1_1_2x)
  FDK_movs_asr_imm(r7, r7, 1)                               // r7: length / 4
  FDK_branch(CC, applyLimiter_func1_1_4x)
  // scale 2 samples
  FDK_vld1_1d(32, D0, r0)                                   // read DelayBuff
  FDK_vqdmulh_s32_dd(D0, D0, D16)                           // DelayBuff * gain/2
  FDK_vqshl_s32(64, D0, D0, D8)                             // shl_sat DelayBuff
  FDK_vld1_1d_ia(32, D4, r1)                                // read SamplesIn
  FDK_vqdmulh_s32_qq(Q2, Q2, Q9)                            // SamplesIn * addGain
  FDK_vst1_1d_ia(32, D4, r0)                                // store SamplesIn * addGain in DelayBuff
#if (SAMPLE_BITS == 16)
  FDK_vqadd_s32_d(D0, D0, D10)                              // D0[0,1] = sat(D0[0,1] + 0x00008000)
  FDK_vst1_ia(16, D0_1, r2)                                 // store shl_sat(DelayBuf * gain) in SamplesOut
  FDK_vst1_ia(16, D0_3, r2)                                 // store shl_sat(DelayBuf * gain) in SamplesOut
#else
  FDK_vst1_1d_ia(32, D0, r2)                                // store shl_sat(DelayBuf * gain) in SamplesOut
#endif

FDK_label(applyLimiter_func1_1_4x)
  FDK_movs_asr_imm(r7, r7, 1)                               // r7: length / 8
  FDK_branch(CC, applyLimiter_func1_1_loop_8x_check_z)
  // scale 4 samples
  FDK_vld1_2d(32, D0, D1, r0)                               // read DelayBuff
  FDK_vqdmulh_s32_qq(Q0, Q0, Q8)                            // DelayBuff * gain/2
  FDK_vqshl_s32(128, Q0, Q0, Q4)                            // shl_sat DelayBuff
  FDK_vld1_2d_ia(32, D4, D5, r1)                            // read SamplesIn
  FDK_vqdmulh_s32_qq(Q2, Q2, Q9)                            // SamplesIn * addGain
  FDK_vst1_2d_ia(32, D4, D5, r0)                            // store SamplesIn * addGain in DelayBuff
#if (SAMPLE_BITS == 16)
  FDK_vqadd_s32_q(Q0, Q0, Q5)                               // Q0[0,1,2,3] = sat(Q0[0,1,2,3] + 0x00008000)
  FDK_vuzp_d(16, D0, D1)                                    // D0: low-parts, D1: high-parts
  FDK_vst1_1d_ia(16, D1, r2)                                // store shl_sat(DelayBuf * gain) in SamplesOut
#else
  FDK_vst1_2d_ia(32, D0, D1, r2)                            // store shl_sat(DelayBuf * gain) in SamplesOut
#endif

FDK_label(applyLimiter_func1_1_loop_8x_check_z)
  FDK_branch(EQ, applyLimiter_func1_loop_8x_end)

FDK_label(applyLimiter_func1_1_loop_8x)
  // scale 8 samples
  FDK_subs_imm(r7, r7, 1)                                   // decrement 8x loop counter
  FDK_vld1_4d(32, D0, D1, D2, D3, r0)                       // read DelayBuff
  FDK_vqdmulh_s32_qq(Q0, Q0, Q8)                            // DelayBuff * gain/2
  FDK_vqdmulh_s32_qq(Q1, Q1, Q8)                            // DelayBuff * gain/2
  FDK_vqshl_s32(128, Q0, Q0, Q4)                            // shl_sat DelayBuff
  FDK_vqshl_s32(128, Q1, Q1, Q4)                            // shl_sat DelayBuff
  FDK_vld1_4d_ia(32, D4, D5, D6, D7, r1)                    // read SamplesIn
  FDK_vqdmulh_s32_qq(Q2, Q2, Q9)                            // SamplesIn * addGain
  FDK_vqdmulh_s32_qq(Q3, Q3, Q9)                            // SamplesIn * addGain
  FDK_vst1_4d_ia(32, D4, D5, D6, D7, r0)                    // store SamplesIn * addGain in DelayBuff
#if (SAMPLE_BITS == 16)
  FDK_vqadd_s32_q(Q0, Q0, Q5)                               // Q0[0,1,2,3] = sat(Q0[0,1,2,3] + 0x00008000)
  FDK_vqadd_s32_q(Q1, Q1, Q5)                               // Q1[0,1,2,3] = sat(Q1[0,1,2,3] + 0x00008000)
  FDK_vuzp_q(16, Q0, Q1)                                    // Q0: low-parts, Q1: high-parts
  FDK_vst1_2d_ia(16, D2, D3, r2)                            // store shl_sat(DelayBuf * gain) in SamplesOut
#else
  FDK_vst1_4d_ia(32, D0, D1, D2, D3, r2)
#endif

  FDK_branch(NE, applyLimiter_func1_1_loop_8x)

FDK_label(applyLimiter_func1_no_addGain)

  FDK_movs_asr_imm(r7, r7, 1)                               // r7: length / 2
  FDK_branch(CC, applyLimiter_func1_2_2x)

//FDK_label(applyLimiter_func1_2_1x)
  FDK_vld1(32, S0, r0)                                      // S0: read DelayBuff
  FDK_vqdmulh_s32_dd(D0, D0, D16)                           // DelayBuff * gain/2
  FDK_vqshl_s32(64, D0, D0, D8)                             // shl_sat DelayBuff
  FDK_vld1_ia(32, S8, r1)                                   // read SamplesIn
  FDK_vst1_ia(32, S8, r0)                                   // store SamplesIn in DelayBuff
#if (SAMPLE_BITS == 16)
  FDK_vqadd_s32_d(D0, D0, D10)                              // D0[0,1] = sat(D0[0,1] + 0x00008000)
  FDK_vst1_ia(16, D0_1, r2)                                 // store shl_sat(DelayBuf * gain) in SamplesOut
#else
  FDK_vst1_ia(32, S0, r2)                                   // store shl_sat(DelayBuf * gain) in SamplesOut
#endif

FDK_label(applyLimiter_func1_2_2x)
  FDK_movs_asr_imm(r7, r7, 1)                               // r7: length / 4
  FDK_branch(CC, applyLimiter_func1_2_4x)
  // scale 2 samples
  FDK_vld1_1d(32, D0, r0)                                   // read DelayBuff
  FDK_vqdmulh_s32_dd(D0, D0, D16)                           // DelayBuff * gain/2
  FDK_vqshl_s32(64, D0, D0, D8)                             // shl_sat DelayBuff
  FDK_vld1_1d_ia(32, D4, r1)                                // read SamplesIn
  FDK_vst1_1d_ia(32, D4, r0)                                // store SamplesIn in DelayBuff
#if (SAMPLE_BITS == 16)
  FDK_vqadd_s32_d(D0, D0, D10)                              // D0[0,1] = sat(D0[0,1] + 0x00008000)
  FDK_vst1_ia(16, D0_1, r2)                                 // store shl_sat(DelayBuf * gain) in SamplesOut
  FDK_vst1_ia(16, D0_3, r2)                                 // store shl_sat(DelayBuf * gain) in SamplesOut
#else
  FDK_vst1_1d_ia(32, D0, r2)                                // store shl_sat(DelayBuf * gain) in SamplesOut
#endif

FDK_label(applyLimiter_func1_2_4x)
  FDK_movs_asr_imm(r7, r7, 1)                               // r7: length / 8
  FDK_branch(CC, applyLimiter_func1_2_loop_8x_check_z)
  // scale 4 samples
  FDK_vld1_2d(32, D0, D1, r0)                               // read DelayBuff
  FDK_vqdmulh_s32_qq(Q0, Q0, Q8)                            // DelayBuff * gain/2
  FDK_vqshl_s32(128, Q0, Q0, Q4)                            // shl_sat DelayBuff
  FDK_vld1_2d_ia(32, D4, D5, r1)                            // read SamplesIn
  FDK_vst1_2d_ia(32, D4, D5, r0)                            // store SamplesIn in DelayBuff
#if (SAMPLE_BITS == 16)
  FDK_vqadd_s32_q(Q0, Q0, Q5)                               // Q0[0,1,2,3] = sat(Q0[0,1,2,3] + 0x00008000)
  FDK_vuzp_d(16, D0, D1)                                    // D0: low-parts, D1: high-parts
  FDK_vst1_1d_ia(16, D1, r2)                                // store shl_sat(DelayBuf * gain) in SamplesOut
#else
  FDK_vst1_2d_ia(32, D0, D1, r2)                            // store shl_sat(DelayBuf * gain) in SamplesOut
#endif

FDK_label(applyLimiter_func1_2_loop_8x_check_z)
  FDK_branch(EQ, applyLimiter_func1_loop_8x_end)

FDK_label(applyLimiter_func1_2_loop_8x)
  // scale 8 samples
  FDK_subs_imm(r7, r7, 1)                                   // decrement 8x loop counter
  FDK_vld1_4d(32, D0, D1, D2, D3, r0)                       // read DelayBuff
  FDK_vqdmulh_s32_qq(Q0, Q0, Q8)                            // DelayBuff * gain/2
  FDK_vqdmulh_s32_qq(Q1, Q1, Q8)                            // DelayBuff * gain/2
  FDK_vqshl_s32(128, Q0, Q0, Q4)                            // shl_sat DelayBuff
  FDK_vqshl_s32(128, Q1, Q1, Q4)                            // shl_sat DelayBuff
  FDK_vld1_4d_ia(32, D4, D5, D6, D7, r1)                    // read SamplesIn
  FDK_vst1_4d_ia(32, D4, D5, D6, D7, r0)                    // store SamplesIn in DelayBuff
#if (SAMPLE_BITS == 16)
  FDK_vqadd_s32_q(Q0, Q0, Q5)                               // Q0[0,1,2,3] = sat(Q0[0,1,2,3] + 0x00008000)
  FDK_vqadd_s32_q(Q1, Q1, Q5)                               // Q1[0,1,2,3] = sat(Q1[0,1,2,3] + 0x00008000)
  FDK_vuzp_q(16, Q0, Q1)                                    // Q0: low-parts, Q1: high-parts
  FDK_vst1_2d_ia(16, D2, D3, r2)                            // store shl_sat(DelayBuf * gain) in SamplesOut
#else
  FDK_vst1_4d_ia(32, D0, D1, D2, D3, r2)
#endif

  FDK_branch(NE, applyLimiter_func1_2_loop_8x)

FDK_label(applyLimiter_func1_loop_8x_end)

  FDK_mvpop(Q4, Q5)
  FDK_mpop(r4, r8)

  FDK_return()
FDK_ASM_ROUTINE_END()

#endif

#ifdef FUNCTION_applyLimiter_func3_DBL

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
FDK_ASM_ROUTINE_START(FIXP_DBL, applyLimiter_func3,
   (const PCM_LIM *samplesIn,
          UINT     channels
    ))
  /*
    stack contents:
    none

    register contents:
      r0: samplesIn                 format: FIXP_DBL * (resp. PCM_LIM *)
      r0: return value              format: FIXP_DBL
      r1: channels                  format: UINT


    NEON registers:
      Q0: 4x sampleIn[i+0..3]
      Q1: 4x sampleIn[i+4..7]
      Q2: 4x maximum
  */

#ifndef __ARM_NEON__
  const PCM_LIM  *  r0 = samplesIn;
  UINT              r1 = channels;

#endif


  /* Clear samples in Q0, Q1 */
  FDK_vmov_i32(128, Q1, 0)
  FDK_vmov_i32(128, Q0, 0)

  FDK_movs_asr_imm(r1, r1, 1)        // r1: length / 2
  FDK_branch(CC, applyLimiter_func3_loop_2x)
  FDK_vld1_ia(32, S0, r0)            // S0: samplesIn[0], i+=1



FDK_label(applyLimiter_func3_loop_2x)
  FDK_movs_asr_imm(r1, r1, 1)        // r1: length / 4
  FDK_branch(CC, applyLimiter_func3_loop_4x)
  FDK_vld1_1d_ia(32, D1, r0)       // D1: samplesIn[i+1]    samplesIn[i+0], i+=2


FDK_label(applyLimiter_func3_loop_4x)
  FDK_movs_asr_imm(r1, r1, 1)        // r1: length / 8
  FDK_branch(CC, applyLimiter_func3_loop_8x_check_z)
  FDK_vld1_2d_ia(32, D2, D3, r0)   // Q1: samplesIn[i+3] samplesIn[i+2] samplesIn[i+1] samplesIn[i+0], i+=4


FDK_label(applyLimiter_func3_loop_8x_check_z)

  FDK_vqabs_q(32, Q0, Q0)
  FDK_vqabs_q(32, Q1, Q1)
  FDK_vmax_s32(128, Q2, Q0, Q1)      // Q2: max3           max2           max1           max0

  FDK_branch(EQ, applyLimiter_func3_loop_8x_end)

FDK_label(applyLimiter_func3_loop_8x)
    FDK_subs_imm(r1, r1, 1)          // decrement 8x loop counter
    FDK_vld1_4d_ia(32,D0,D1,D2,D3,r0)// Q0: samplesIn[i+3] samplesIn[i+2] samplesIn[i+1] samplesIn[i+0],
                                     // Q1: samplesIn[i+7] samplesIn[i+6] samplesIn[i+5] samplesIn[i+4], i+=8
    FDK_vqabs_q(32, Q0, Q0)
    FDK_vqabs_q(32, Q1, Q1)
    FDK_vmax_s32(128, Q2, Q2, Q0)    // Q2: max3           max2           max1           max0
    FDK_vmax_s32(128, Q2, Q2, Q1)    // Q2: max3           max2           max1           max0
    FDK_branch(NE, applyLimiter_func3_loop_8x)

FDK_label(applyLimiter_func3_loop_8x_end)

  FDK_vmax_s32(64, D0, D4, D5)       // D0: max3,2      max1,0
  FDK_vpmax_s32(64, D0, D0, D0)      // D0: max3,2,1,0  max3,2,1,0

  FDK_vmov_reg(r0, S0)               // r0: max (of all)
  FDK_return()
FDK_ASM_ROUTINE_RETURN(FIXP_DBL)

#endif /* #ifdef FUNCTION_applyLimiter_func3_DBL */
