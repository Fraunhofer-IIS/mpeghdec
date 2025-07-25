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

   Author(s):   Tobias Chalupka

   Description: (ARM optimized) stereo processing

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_ARCH_5TE__) && !defined(__THUMBEL__)
#if defined(__GNUC__)
#include "arm/FDK_arm_funcs.h"
#define FUNCTION_CJointStereo_GenerateMSOutput
#endif
#endif

#if defined(FUNCTION_CJointStereo_GenerateMSOutput)
FDK_ASM_ROUTINE_START(void,CJointStereo_GenerateMSOutput,(
        FIXP_DBL *pSpecLCurrBand,
        FIXP_DBL *pSpecRCurrBand,
        UINT     leftScale,
        UINT     rightScale,
        UINT     nSfbBands
        ))

  // save upper core registers according to call convention
  FDK_mpush_lr(r4,r12)

  /* stack contents:
       0x28:     nSfbBands
       0x24:     lr
       0x20:     r12
       0x1C:     r11
       0x18:     r10
       0x14:     r9
       0x10:     r8
       0x0C:     r7
       0x08:     r6
       0x04:     r5
       0x00:     r4

     register usage:
        r0: pointer to input/output spectrum for current band, left channel
        r1: pointer to input/output spectrum for current band, right channel
        r2: scaling for current band, left channel
        r3: scaling for current band, right channel

        r4: spectral line 0 of current iteration, left channel
        r5: spectral line 1 of current iteration, left channel
        r6: spectral line 2 of current iteration, left channel
        r7: spectral line 3 of current iteration, left channel
        r8: spectral line 0 of current iteration, right channel
        r9: spectral line 1 of current iteration, right channel
       r10: spectral line 2 of current iteration, right channel
       r11: spectral line 3 of current iteration, right channel

       r12: counter
        lr: tmp
  */

  /* Load arguments */
  FDK_ldr(r12, sp, 0x28, nSfbBands)

  FDK_cmp_imm (r12, 0)
  FDK_branch(LE, CJointStereo_GenerateMSOutput_end)

FDK_label(CJointStereo_GenerateMSOutput_loop)

    FDK_ldrd(  r4,  r5, r0,  0, *(pSpecLCurrBand  ), *(pSpecLCurrBand+1))
    FDK_ldrd(  r6,  r7, r0,  8, *(pSpecLCurrBand+2), *(pSpecLCurrBand+3))
    FDK_ldrd(  r8,  r9, r1,  0, *(pSpecRCurrBand  ), *(pSpecRCurrBand+1))
    FDK_ldrd( r10, r11, r1,  8, *(pSpecRCurrBand+2), *(pSpecRCurrBand+3))

    FDK_asr( r4,  r4, r2)
    FDK_asr( r5,  r5, r2)
    FDK_asr( r6,  r6, r2)
    FDK_asr( r7,  r7, r2)

#if defined(__ARM_ARCH_7EM__)
    FDK_asr(r8, r8, r3)
    FDK_add(lr, r4, r8)
    FDK_sub(r8, r4, r8)
#else
    FDK_add_op_asr_reg( lr,  r4,  r8, r3, 0)
    FDK_sub_op_asr_reg( r8,  r4,  r8, r3, 0)
#endif
    FDK_str_ia (lr, r0, 4)

#if defined(__ARM_ARCH_7EM__)
    FDK_asr(r9, r9, r3)
    FDK_add(r4, r5, r9)
    FDK_sub(r9, r5, r9)
#else
    FDK_add_op_asr_reg( r4,  r5,  r9, r3, 0)
    FDK_sub_op_asr_reg( r9,  r5,  r9, r3, 0)
#endif
    FDK_str_ia (r4, r0, 4)

#if defined(__ARM_ARCH_7EM__)
    FDK_asr(r10, r10, r3)
    FDK_add(r5,  r6, r10)
    FDK_strd_ia( r8,  r9, r1, 8)
    FDK_sub(r10, r6, r10)
#else
    FDK_add_op_asr_reg( r5,  r6, r10, r3, 0)
    FDK_strd_ia( r8,  r9, r1, 8)
    FDK_sub_op_asr_reg(r10,  r6, r10, r3, 0)
#endif
    FDK_str_ia (r5, r0, 4)

#if defined(__ARM_ARCH_7EM__)
    FDK_asr(r11, r11, r3)
    FDK_add( r6, r7, r11)
    FDK_sub(r11, r7, r11)
#else
    FDK_add_op_asr_reg( r6,  r7, r11, r3, 0)
    FDK_sub_op_asr_reg(r11,  r7, r11, r3, 0)
#endif
    FDK_str_ia (r6, r0, 4)

    FDK_subs_imm(r12, r12, 4)
    FDK_strd_ia(r10, r11, r1, 8)
  FDK_branch(GT, CJointStereo_GenerateMSOutput_loop)

FDK_label(CJointStereo_GenerateMSOutput_end)
  FDK_mpop_pc(r4,r12)
FDK_ASM_ROUTINE_END()
#endif /* defined(FUNCTION_CJointStereo_GenerateMSOutput) */
