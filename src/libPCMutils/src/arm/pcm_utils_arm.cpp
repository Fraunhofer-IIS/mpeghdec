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

   Description: Arm /Arm Neon Versions for
                - FDK_interleave

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_NEON__)
#define FUNCTION_FDK_interleave_DBL_LONG
#endif


#if defined(FUNCTION_FDK_interleave_DBL_LONG)
FDK_ASM_ROUTINE_START(void, FDK_interleave_ARMv7NEON,
  (   const FIXP_DBL  *RESTRICT pIn,
            LONG      *RESTRICT pOut,
            UINT                channels,
            UINT                frameSize,
            UINT                length
    ))

#ifndef __ARM_NEON__
      const FIXP_DBL  * RESTRICT  r0 = pIn;
      LONG            * RESTRICT  r1 = pOut;
      UINT                        r2 = channels;
      UINT                        r3 = frameSize;
      UINT                        r4;
      const FIXP_DBL  *           r5;
      const FIXP_DBL  *           r6;
      const FIXP_DBL  *           r7;
      const FIXP_DBL  *           r8;
      UINT                        r9;
      UINT                        r10;
      UINT                        r11;
      LONG*                       lr;
#endif

  /* stack contents:
  0x24:     length
  0x20:     r4
  0x1c:     r5
  0x18:     r6
  0x14:     r7
  0x10:     r8
  0x0c:     r9
  0x08:     r10
  0x04:     r11
  0x00:     lr

  register contents:
  r0:   pIn                       input, 32 bit
  r1:   pOut                      output, 32 bit
  r2:   channels
  r3:   frameSize
  r4:   length
  r5:   channelpointer ch0
  r6:   channelpointer ch1
  r7:   channelpointer ch2
  r8:   channelpointer ch3
  r9:   loop counter 4Channel loop
  r10:  store-stepsize
  r11:  length (counter for loops)
  lr:   backup pOut

  NEON registers:
  Q0: in/out buffer
  Q1: in/out buffer
  Q2: in/out buffer
  Q3: in/out buffer
  */

  FDK_mpush_lr(r4,r11)

  FDK_ldr(r4, sp, 0x24, length)                   // r4: length

  /* some inits: */
  FDK_add_imm(r4, r4, 3, 0)                       // r4: length+3
  FDK_asrs_imm(r4, r4, 2)                         // r4: (length+3)>>2 <--- number of inner loops
  FDK_branch(EQ, FDK_interleave_DBL_LONG_end)     // (len==0) ? exit
  FDK_lsl_imm(r10, r2, 2)                         // r10: channels * 4 (stepsize for store)
  FDK_mov_reg(lr, r1)                             // lr:  Backup pOut

    /* ----------------------------------- */
    /* check/loop 4Channels -------------- */
    /* ----------------------------------- */
  FDK_asrs_imm(r9, r2, 2)
  FDK_branch(EQ, FDK_interleave_DBL_LONG_check_2Channels)

  FDK_mov_reg(r11, r4)                            // r11: inner loops

FDK_label(FDK_interleave_DBL_LONG_4Channels_outerloop)
    /* setup Channel pointers */
  FDK_mov_reg(r5, r0)                             // r5: channelpointer ch0
  FDK_add_op_lsl(r6, r0, r3, 2, 2)                // r6: channelpointer ch1
  FDK_add_op_lsl(r7, r6, r3, 2, 2)                // r7: channelpointer ch2
  FDK_add_op_lsl(r8, r7, r3, 2, 2)                // r8: channelpointer ch3
  FDK_add_op_lsl(r0, r8, r3, 2, 2)                // r0: new channelpointer

FDK_label(FDK_interleave_DBL_LONG_4Channels_loop)
    // process 4 Channels
    FDK_subs_imm(r11, r11, 1)                     // decrement loop counter

    FDK_vld4_ia(32, S0, S4, S8, S12, r5)          // load 4 Samples from ch0
    FDK_vld4_ia(32, S1, S5, S9, S13, r6)          // load 4 Samples from ch1
    FDK_vld4_ia(32, S2, S6, S10, S14, r7)         // load 4 Samples from ch2
    FDK_vld4_ia(32, S3, S7, S11, S15, r8)         // load 4 Samples from ch3
    FDK_vst1_2d_pu(32, D0, D1, r1, r10)           // store 4 Samples
    FDK_vst1_2d_pu(32, D2, D3, r1, r10)           // store 4 Samples
    FDK_vst1_2d_pu(32, D4, D5, r1, r10)           // store 4 Samples
    FDK_vst1_2d_pu(32, D6, D7, r1, r10)           // store 4 Samples

    FDK_branch(NE, FDK_interleave_DBL_LONG_4Channels_loop)

  FDK_subs_imm(r9, r9, 1)
  FDK_mov_reg(r1, lr) /* recover pOut and increment by ChannelsProcessed * DatatypeSize */
  FDK_add_imm(r1, r1, 16, 2)
  FDK_mov_reg(lr, r1)                             // Backup pOut
  FDK_mov_reg(r11, r4)                            // r11: recover loop counter: length

  FDK_branch(NE, FDK_interleave_DBL_LONG_4Channels_outerloop)

    /* ----------------------------------- */
    /*check/loop 2Channels  -------------- */
    /* ----------------------------------- */
FDK_label(FDK_interleave_DBL_LONG_check_2Channels)
  FDK_tst_imm(r2, 2)                              // r2: channels & 2 ?
  FDK_branch(EQ, FDK_interleave_DBL_LONG_check_1Channel)

  FDK_mov_reg(r11, r4)                            // r11: inner loops
  /* setup Channel pointers */
  FDK_mov_reg(r5, r0)                             // r5: channelpointer ch0
  FDK_add_op_lsl(r6, r0, r3, 2, 2)                // r6: channelpointer ch1
  FDK_add_op_lsl(r0, r6, r3, 2, 2)                // r0: new channelpointer

FDK_label(FDK_interleave_DBL_LONG_2Channels_loop)
    // process 2 Channels
    FDK_subs_imm(r11, r11, 1)                     // decrement 4x loop counter (Framelength)

    FDK_vld4_ia(32, S0, S4, S8, S12, r5)          // load 4 Samples from ch0
    FDK_vld4_ia(32, S1, S5, S9, S13, r6)          // load 4 Samples from ch1
    FDK_vst1_1d_pu(32, D0, r1, r10)               // store 2 Samples
    FDK_vst1_1d_pu(32, D2, r1, r10)               // store 2 Samples
    FDK_vst1_1d_pu(32, D4, r1, r10)               // store 2 Samples
    FDK_vst1_1d_pu(32, D6, r1, r10)               // store 2 Samples

    FDK_branch(NE, FDK_interleave_DBL_LONG_2Channels_loop)

  FDK_mov_reg(r1, lr) /* recover pOut and increment by ChannelsProcessed * DatatypeSize */
  FDK_add_imm(r1, r1, 8, 2)
  FDK_mov_reg(lr, r1)                             // Backup pOut
  FDK_mov_reg(r11, r4)                            // r11: recover loop counter: length


  /* ----------------------------------- */
  /*check/loop 1Channel  --------------- */
  /* ----------------------------------- */
FDK_label(FDK_interleave_DBL_LONG_check_1Channel)
  FDK_tst_imm(r2, 1)                              // r2: channels & 1 ?
  FDK_branch(EQ, FDK_interleave_DBL_LONG_end)

  FDK_mov_reg(r11, r4)                            // r11: inner loops
FDK_label(FDK_interleave_DBL_LONG_1Channel_loop)
    // process 1 Channel
    FDK_subs_imm(r11, r11, 1)                     // decrement 4x loop counter (Framelength)

    FDK_vld1_2d_ia(32, D0, D1, r0)                // load 4 Samples from ch0
    FDK_vst1_pu(32, S0, r1, r10)                  // store 1 Sample
    FDK_vst1_pu(32, S1, r1, r10)                  // store 1 Sample
    FDK_vst1_pu(32, S2, r1, r10)                  // store 1 Sample
    FDK_vst1_pu(32, S3, r1, r10)                  // store 1 Sample

    FDK_branch(NE, FDK_interleave_DBL_LONG_1Channel_loop)

    /* END */
FDK_label(FDK_interleave_DBL_LONG_end)
  FDK_mpop_pc(r4,r11 )
  FDK_ASM_ROUTINE_END()


void FDK_interleave(
      const FIXP_DBL  *RESTRICT pIn,
            LONG      *RESTRICT pOut,
            UINT                channels,
            UINT                frameSize,
            UINT                length)
{
    FDK_interleave_ARMv7NEON(pIn, pOut, channels, frameSize, length);
}
#endif /* defined(FUNCTION_FDK_interleave_DBL_LONG) */
