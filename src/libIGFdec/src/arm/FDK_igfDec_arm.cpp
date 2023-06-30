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

/********************** Intelligent gap filling library ************************

   Author(s):   Arthur Tritthart, Ferdinand Packi, Fabian Bauer

   Description: ARM optimized functions of Intelligent gap filling library

*******************************************************************************/

/* clang-format off */

#if defined(__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"
#define FUNCTION_iisIGFDecoderApplyGainsMonoNew_func1
#define FUNCTION_iisIGF_TNFfilter_func1
#define FUNCTION_Same_Exponent_Correlation_func1
#define FUNCTION_DotProduct_func1

#endif

#ifdef FUNCTION_Same_Exponent_Correlation_func1
FDK_ASM_ROUTINE_START(FIXP_DBL, Same_Exponent_Correlation_func1,
 (  const FIXP_DBL *X,
    const FIXP_DBL *Y,
    const INT       shift,
    const INT       loop_shift,
    const INT       correlation_length))

     /* stack contents:
        0x04: correlation_length
        0x00: r4

     register usage:
        r0:  X
        r1:  Y
        r2:  shift
        r3:  loop_shift
        r4:  correlation_length

      NEON register usage:
        Q15:        1          1         1          1  <- used for LSB masking
        Q14:
        Q13:
        Q12:
        Q11:
        Q10:   temp3      temp2     temp1      tewp0    <- fMult(X[i], Y[i]) >> loop_shift
        Q9:  -loop_shift (duplicated)
        Q8:  shift       (duplicated)
        Q7:  reserved   reserved   reserved   reserved
        Q6:  reserved   reserved   reserved   reserved
        Q5:  reserved   reserved   reserved   reserved
        Q4:  reserved   reserved   reserved   reserved
        Q3:
        Q2:  Y[i+3]     Y[i+2]     Y[i+1]     Y[i+0]
        Q1:  X[i+3]     X[i+2]     X[i+1]     X[i+0]
        Q0:   accu3      accu2      accu1      accu0
    */

#ifndef __ARM_NEON__
   FIXP_DBL *r0 = (FIXP_DBL*)X;
   FIXP_DBL *r1 = (FIXP_DBL*)Y;
   INT  r2 = shift;
   INT  r3 = loop_shift;
   INT       r4;     /* correlation_length */
#endif

    FDK_push(r4)
    FDK_ldr(r4, sp, 0x04, correlation_length)
    FDK_vmov_i32(128, Q0, 0)                     // Q0: clear accu
    FDK_vdup_q_reg(32, Q8 ,r2)
    FDK_rsb_imm(r3, r3, 0)
    FDK_vdup_q_reg(32, Q9, r3)

    FDK_tst_imm(r4, 0x3)
    FDK_branch(EQ, Same_Exponent_Correlation_func1_loop_check)

    FDK_vmov_i32(128, Q1, 0)                     // Q1: clear X register
    FDK_vmov_i32(128, Q2, 0)                     // Q2: clear Y register
    FDK_asrs_imm(r4, r4, 1)
    FDK_branch(CC, Same_Exponent_Correlation_func1_tst_b2)
      // process 1 X/Y sample
      FDK_vld1_ia(32, S4, r0)                    // S4: X[0]
      FDK_vld1_ia(32, S8, r1)                    // S8: Y[0]

FDK_label(Same_Exponent_Correlation_func1_tst_b2)
    FDK_asrs_imm(r4, r4, 1)
    FDK_lsl_imm(r4, r4, 2)
    FDK_branch(CC, Same_Exponent_Correlation_func1_acc)
      // process 2 X/Y samples
      FDK_vld1_1d_ia(32, D3, r0)                 // D5: X[i+0], X[i+1]
      FDK_vld1_1d_ia(32, D5, r1)                 // D7: Y[i+0], Y[i+1]

FDK_label(Same_Exponent_Correlation_func1_acc)
    FDK_vshl_s32_q(Q1, Q1, Q8)                  // Q1: X << shift
    FDK_vshl_s32_q(Q2, Q2, Q8)                  // Q2: Y << shift
    FDK_vqdmulh_s32_qq(Q10, Q1, Q2)             // Q10: temp = fMult(X << shift, Y << shift)
    FDK_vshl_s32_q(Q0, Q10, Q9)                 // Q10: temp >> loop_shift

FDK_label(Same_Exponent_Correlation_func1_loop_check)
    FDK_cmp_imm(r4, 0)
    FDK_branch(EQ, Same_Exponent_Correlation_func1_loop_end)

FDK_label(Same_Exponent_Correlation_func1_loop)
      FDK_vld1_2d_ia(32, D2, D3, r0)            // Q1: X
      FDK_vld1_2d_ia(32, D4, D5, r1)            // Q2: Y
      FDK_vshl_s32_q(Q1, Q1, Q8)                // Q1: X << shift
      FDK_vshl_s32_q(Q2, Q2, Q8)                // Q2: Y << shift
      FDK_vqdmulh_s32_qq(Q10, Q1, Q2)           // Q10: temp = fMult(X << shift, Y << shift)
      FDK_vshl_s32_q(Q10, Q10, Q9)              // Q10: temp >> loop_shift
      FDK_vadd_s32_q(Q0, Q0, Q10)               // Q0: acc += temp
      FDK_subs_imm(r4, r4, 4)
      FDK_branch(NE, Same_Exponent_Correlation_func1_loop)

FDK_label(Same_Exponent_Correlation_func1_loop_end)

    FDK_vhadd_s32_d(D0,D0,D1)                  // D0: accu3+accu1  accu2+accu0
    FDK_vpadd_s32(D0, D0, D0)                  // S0: accu
    FDK_vmov_reg(r0, s0)                       // r0: return value: accu
    FDK_pop(r4)
    FDK_return()
FDK_ASM_ROUTINE_RETURN(FIXP_DBL)

#endif /* FUNCTION_Same_Exponent_Correlation_func1 */


#ifdef FUNCTION_iisIGF_TNFfilter_func1
FDK_ASM_ROUTINE_START(void, iisIGF_TNFfilter_func1,
   (      FIXP_DBL * p2_spectrum,    /* r0 */
          INT        OutputShift,    /* r1 */
          INT        head_shift,     /* r2 */
    const FIXP_DBL * Aarray,         /* r3 */
    const INT        numOfLines))    /* [0]*/

    /*
      register usage/contents:
        r0: p2_spectrum (read pointer, decremental)
        r1: OutputShift, const stepregister=-4
        r2: head_shift, numLines loop counter
        r3: Aarray, p2_spectrum (write pointer)

      stack contents:
        0x00:  numOfLines

      NEON register usage:
        Q15: ------------------- unused -----------------
        Q14: ------------------- unused -----------------
        Q13: new sample     ----        ----        ----
        Q12:                      acc
        Q11:                  variably used
        Q10:                  variably used
        Q9:                        OutputShift OutputShift
        Q8:  head_shift head_shift  head_shift  head_shift
        Q7:  ----------------- reserved -----------------
        Q6:  ----------------- reserved -----------------
        Q5:  ----------------- reserved -----------------
        Q4:  ----------------- reserved -----------------
        Q3:   Aarray4    Aarray5     Aarray6     Aarray7
        Q2:   Aarray0    Aarray1     Aarray2     Aarray3
        Q1:  p2sp[-1]   p2sp[-2]    p2sp[-3]    p2sp[-4] <- multiply with Q2
        Q0:  p2sp[-5]   p2sp[-6]    p2sp[-7]    p2sp[-8] <- multiply with Q3
    */

#ifndef __ARM_NEON__
    FIXP_DBL * r0 = p2_spectrum;
    INT        r1 = OutputShift;         // later: -4
    INT        r2 = head_shift;          // later: numOfLines -1 -7
    FIXP_DBL * r3 = (FIXP_DBL *) Aarray; // later: p2_spectrum (write pointer, decremental)
#endif

    FDK_vdup_q_reg(32, Q8, r2)                   // Q8 : head_shift  head_shift  head_shift  head_shift
    FDK_vdup_d_reg(32, D18, r1)                  // D18:                        OutputShift  OutputShift

    // Load Aarray[0..7] in reverse order
    FDK_vld1_4d(32, D4, D5, D6, D7, r3)          // Q3: a7   a6    a5    a4
                                                 // Q2: a3   a2    a1    a0
    FDK_vrev64_q(32, Q2, Q2)                     // Q2: a2   a3    a0    a1
    FDK_vrev64_q(32, Q3, Q3)                     // Q3: a6   a7    a4    a5
    FDK_vswp(64, D4, D5)                         // Q2: a0   a1    a2    a3 <-- done
    FDK_vswp(64, D6, D7)                         // Q3: a4   a5    a6    a7 <-- done

    FDK_mov_imm(r1, -4)                          // r1: -4                                (Step)
    FDK_ldr(r2, sp, 0x00, numOfLines)
    FDK_sub_imm(r2, r2, 1, 0)                    // r2: numOfLines - 1
    FDK_add_op_lsl(r3, r0, r2, 2, 2)             // r3: p2_spectrum += numOfLines-1      (Storepointer)
    FDK_sub_imm(r2, r2, 7, 0)                    // r2: numOfLines -1 - 7                (Counter)
    FDK_add_op_lsl(r0, r0, r2, 2, 2)             // r0: p2_spectrum += numOfLines-1 - 7  (Readpointer)

    // Load last 8 samples from p2_spectrum, and scale them
    FDK_vld1_4d_pu(32, D0, D1, D2, D3, r0, r1)   // Q1: j-1   j-2   j-3   j-4
                                                 // Q0: j-5   j-6   j-7   j-8
    FDK_vshl_s32_q(Q0, Q0, Q8)
    FDK_vshl_s32_q(Q1, Q1, Q8)                   // Q1 :(j-1   j-2   j-3   j-4) << head_shift
                                                 // Q0 :(j-5   j-6   j-7   j-8) << head_shift
FDK_label(iisIGF_TNFfilter_func1_loop)
      FDK_vqdmulh_s32_qq(Q10, Q1, Q2)
      FDK_vqdmulh_s32_qq(Q11, Q0, Q3)            // tmp = fMult(p2_spectrum[j-i] << head_shift, Aarray[i]);

      FDK_subs_imm(r2, r2, 1)
      FDK_vld1_pu(32, S55, r0, r1)               // Q13:  nextsample(S55)   dontcare                   dontcare    dontcare
      FDK_vshl_s32_d(D27, D27, D16)              // Q13: (nextsample(S55)   dontcare) << head_shift    dontcare    dontcare
      FDK_vext_q(32, Q1, Q0, Q1, 3)              // move input registers content for one sample
      FDK_vext_q(32, Q0, Q13, Q0, 3)

      FDK_vhadd_s32_q(Q10, Q10, Q11)
      FDK_vhadd_s32_d(D20, D20, D21)
      FDK_vpadd_s32(D20, D20, D20)               // D20:  --------    --------       acc         acc

      FDK_vshl_s32_d(D20, D20, D18)              // D20  --------    --------    acc<<OShift   acc<<OShift
      FDK_vst1_pu(32, S40, r3, r1)               // S40: p2_spectrum[j]=scaleValue(acc,OutputShift)
      FDK_branch(GT, iisIGF_TNFfilter_func1_loop)

    FDK_add_imm(r2, r2, 4, 0)
    FDK_vmov_i32(64, D27, 0)                     // Q13:  0x00000000 0x00000000  dontcare    dontcare

FDK_label(iisIGF_TNFfilter_func1_loop_2)
      FDK_vqdmulh_s32_qq(Q10, Q1, Q2)
      FDK_vqdmulh_s32_qq(Q11, Q0, Q3)            // tmp = fMult(p2_spectrum[j-i] << head_shift, Aarray[i]);
      FDK_subs_imm(r2, r2, 1)
      FDK_vext_q(32, Q1, Q0, Q1, 3)              // move input registers content for one sample
      FDK_vext_q(32, Q0, Q13, Q0, 3)
      FDK_vhadd_s32_q(Q10, Q10, Q11)
      FDK_vhadd_s32_d(D20, D20, D21)
      FDK_vpadd_s32(D20, D20, D20)               // D20:  --------    --------       acc         acc
      FDK_vshl_s32_d(D20, D20, D18)
      FDK_vst1_pu(32, S40, r3, r1)               // S40: p2_spectrum[j]=scaleValue(acc,OutputShift)
      FDK_branch(GT, iisIGF_TNFfilter_func1_loop_2)

    FDK_add_imm(r2, r2, 3 << 0, 0)
    FDK_vmov_i32(64, D0 ,1)
    FDK_vsub_s32_d(D18, D18, D0)

FDK_label(iisIGF_TNFfilter_func1_loop_3)
      FDK_vqdmulh_s32_qq(Q10, Q1, Q2)            // tmp = fMult(p2_spectrum[j-i] << head_shift, Aarray[i]);
      FDK_subs_imm(r2, r2, 1)
      FDK_vext_q(32, Q1, Q13, Q1, 3)             // move input registers content for one sample
      FDK_vhadd_s32_d(D20, D20, D21)
      FDK_vpadd_s32(D20, D20, D20)               // D20:  --------    --------       acc         acc
      FDK_vshl_s32_d(D20, D20, D18)
      FDK_vst1_pu(32, S40, r3, r1)               // S40: p2_spectrum[j]=scaleValue(acc,OutputShift)
      FDK_branch(GT, iisIGF_TNFfilter_func1_loop_3)

  FDK_return()
FDK_ASM_ROUTINE_END()

#endif /* #ifdef FUNCTION_iisIGF_TNFfilter_func1 */



#ifdef FUNCTION_iisIGFDecoderApplyGainsMonoNew_func1
FDK_ASM_ROUTINE_START(void, iisIGFDecoderApplyGainsMonoNew_func1,
   (FIXP_DBL *p2_pSpectralDataReshuffle,    /* r0  */
    FIXP_DBL *p2_virtualSpec_tb,            /* r1  */
    INT       shift1,                       /* r2  */
    INT       shift,                        /* r3 */
    FIXP_SGL  hMap_fSfbGainTab_sfb,
    INT       width))

  /* stack contents:
       0x04:  width
       0x00:  hMap_fSfbGainTab_sfb

     register usage/contents:
       r0:  p2_pSpectralDataReshuffle    FIXP_DBL *
       r1:  p2_virtualSpec_tb            FIXP_DBL *
       r2:  shift1,hMap_fSfbGainTab_sfb  INT, FIXP_SGL
       r3:  shift, width                 INT

     NEON register usage:
       Q15: ------------------- unused -----------------
       Q14: ------------------- unused -----------------
       Q13: ------------------- unused -----------------
       Q12: ------------------- unused -----------------
       Q11: ------------------- unused -----------------
       Q10: ------------------- unused -----------------
       Q9:  ------------------- unused -----------------
       Q8:  ------------------- unused -----------------
       Q7:  ------------------- unused -----------------
       Q6:  ------------------- unused -----------------
       Q5:  ------------------- unused -----------------
       Q4:  ------------------- unused -----------------
       Q3:  hMap_fSfbGainTab_sfb (duplicated) in Q1.31
       Q2:  -shift     -shift        -shift     -shift
       Q1:  shift1     shift1        shift1     shift1
       Q0:  temp3      temp2         temp1      temp0
  */

#ifndef __ARM_NEON__
#include "arm/FDK_neon_regs.h"
    FIXP_DBL *r0 = p2_pSpectralDataReshuffle;
    FIXP_DBL *r1 = p2_virtualSpec_tb;
    INT       r2 = shift1;
    INT       r3 = shift;
#endif
    FDK_vdup_q_reg(32, Q1, r2)                             // Q1: shift1     shift1        shift1     shift1
    FDK_rsb_imm(r3, r3, 0)                                 // r3: -shift
    FDK_vdup_q_reg(32, Q2, r3)                             // Q2: -shift     -shift        -shift     -shift
    FDK_ldrd(r2, r3, sp, 0x00,hMap_fSfbGainTab_sfb,width)  // r2: hMap_fSfbGainTab_sfb     r3: width
    FDK_vdup_d_reg(16, D6, r2)                             // D6: hMap_fSfbGainTab_sfb (duplicated 4x) in Q1.15
    FDK_vshll_s16_imm(Q3, D6, 16)                          // Q3: hMap_fSfbGainTab_sfb (duplicated 4x) in Q1.31

FDK_label(iisIGFDecoderApplyGainsMonoNew_func1_loop)
      FDK_vld1_2d_ia(32, D0, D1, r0)                       // Q0: temp=*p2_pSpectralDataReshuffle++  <-- get input data
      FDK_vshl_s32_q(Q0, Q0, Q1)                           // Q0: temp=*p2_pSpectralDataReshuffle++ << shift1
      FDK_vqdmulh_s32_qq(Q0, Q0, Q3)                       // Q0: temp=fMult(hMap_fSfbGainTab_sfb,temp)
      FDK_subs_imm(r3, r3, 4)
      FDK_vshl_s32_q(Q0, Q0, Q2)                           // Q0: temp=*p2_pSpectralDataReshuffle++ >> shift
      FDK_vst1_2d_ia(32, D0, D1, r1)                       // Q0: *p2_virtualSpec_tb++ = temp        <-- store output data
      FDK_branch(NE, iisIGFDecoderApplyGainsMonoNew_func1_loop)

    FDK_return()
FDK_ASM_ROUTINE_END()


#endif  /* #ifdef FUNCTION_iisIGFDecoderApplyGainsMonoNew_func1 */

#ifdef FUNCTION_iisIGFDecLibInjectSourceSpectrumTCX_func2
FDK_ASM_ROUTINE_START(void, iisIGFDecLibInjectSourceSpectrumTCX_func2_ARMv7NEON,
    ( ULONG    * randomSeed,
      FIXP_DBL **Tile_pointer_Array,
      INT        IGFNumTile,
      FIXP_DBL   noise_level,
      INT        NumSb,
const ULONG    * seed_constants))

  /* stack contents:
       0x14:  seed_constants
       0x10:  NumSb
       0x0C:  r7
       0x08:  r6
       0x04:  r5
       0x00:  r4

     register usage/contents:
       r0:  randomSeed
       r1:  Tile_pointer_Array
       r2:  IGFNumTile
       r3:  noise_level, seed_constants, NumSb
       r4:  Tile_pointer_Array[0]
       r5:  Tile_pointer_Array[1]
       r6:  Tile_pointer_Array[2]
       r7:  Tile_pointer_Array[3]

     NEON register usage:
       Q15:
       Q14:
       Q13:
       Q12: 00010000    00010000    00010000    00010000 <-- mask to select randomSign result
       Q11:-noiselev   -noiselev   -noiselev   -noiselev
       Q10: noiselev    noiselev    noiselev    noiselev
       Q9:  ca_seed3    ca_seed2    ca_seed1    ca_seed0 <-- Seed constants for addition
       Q8:  cm_seed3    cm_seed2    cm_seed1    cm_seed0 <-- Seed constants for multiplication
       Q7:  reserved    reserved    reserved    reserved
       Q6:  reserved    reserved    reserved    reserved
       Q5:  reserved    reserved    reserved    reserved
       Q4:  reserved    reserved    reserved    reserved
       Q3:    seed4       seed3       seed2       seed1
       Q2:    seed4       seed3       seed2       seed1
       Q1:    seed4       seed3       seed2       seed1
       Q0:    seed4       seed3       seed2       seed1
  */
#ifndef __ARM_NEON__
 ULONG     * r0 = randomSeed;
 FIXP_DBL **r1 = Tile_pointer_Array;
 INT        r2 = IGFNumTile;
 INT        r3 = (INT) noise_level;
 FIXP_DBL *r4, *r5, *r6, *r7;
#endif

  FDK_mpush(r4, r7)

  FDK_vdup_q_reg(32, Q10, r3)                    // Q10: noiselev    noiselev    noiselev    noiselev
  FDK_vmov_i32(128, Q12, 0x00010000)             // Q12: 00010000    00010000    00010000    00010000
  FDK_vneg_q(32, Q11, Q10)                       // Q11:-noiselev   -noiselev   -noiselev   -noiselev
  FDK_ldr(r4, sp, 0x14, (FIXP_DBL *)seed_constants)
  FDK_vld1_4d(32, D16, D17, D18, D19, r4)
  FDK_ldr(r3, sp, 0x10, NumSb)

  FDK_cmp_imm(r2, 4)
  FDK_branch(EQ, iisIGFDecLibInjectSourceSpectrumTCX_func2_numTile4)

  FDK_cmp_imm(r2, 3)
  FDK_branch(EQ, iisIGFDecLibInjectSourceSpectrumTCX_func2_numTile3)

  FDK_cmp_imm(r2, 2)
  FDK_branch(EQ, iisIGFDecLibInjectSourceSpectrumTCX_func2_numTile2)

//FDK_label(iisIGFDecLibInjectSourceSpectrumTCX_func2_numTile1)
    FDK_vld1(32, S3, r0)
    FDK_ldr(r4, r1, 0x00, Tile_pointer_Array[0])
FDK_label(iisIGFDecLibInjectSourceSpectrumTCX_func2_loop1)
      FDK_vmul_s32_q_scalar(Q0, Q8, S3)          // Q0: seed0*cm3   seed0*cm2   seed0*cm1   seed0*cm0 <- iteration 1..4
      FDK_subs_imm(r3, r3, 4)
      FDK_vmov_q(Q2, Q10)                        // Q2:  noiselev    noiselev    noiselev    noiselev
      FDK_vadd_s32_q(Q0, Q0, Q9)                 // Q0:  seed1_i4    seed1_i3    seed1_i2    seed1_i1
      FDK_vtst_q(32, Q1, Q0, Q12)                // Q1:  00000000    FFFFFFFF    00000000    FFFFFFFF <- -1, if bit#16 is set, 0 otherwise
      FDK_vbit_q(Q2, Q11, Q1)                    // Q2:  noiselev   -noiselev    noiselev   -noiselev
      FDK_vst1_2d_ia(32, D4, D5, r4)
      FDK_branch(NE, iisIGFDecLibInjectSourceSpectrumTCX_func2_loop1)

    FDK_str(r4, r1, 0x00, Tile_pointer_Array[0])
    FDK_vst1(32, S3, r0)
    FDK_branch(AL, iisIGFDecLibInjectSourceSpectrumTCX_func2_end)

FDK_label(iisIGFDecLibInjectSourceSpectrumTCX_func2_numTile2)
    FDK_vld1(32, S3, r0)
    FDK_ldrd(r4, r5, r1, 0x00, Tile_pointer_Array[0], Tile_pointer_Array[1])
FDK_label(iisIGFDecLibInjectSourceSpectrumTCX_func2_loop2)
      FDK_vmul_s32_q_scalar(Q1, Q8, S3)          // Q1: seed0*cm3   seed0*cm2   seed0*cm1   seed0*cm0 <- iteration 1,2
      FDK_vadd_s32_q(Q1, Q1, Q9)                 // Q1:  seed2_i2    seed1_i2    seed2_i1    seed1_i1

      FDK_vmul_s32_q_scalar(Q0, Q8, S7)          // Q0: seed0*cm3   seed0*cm2   seed0*cm1   seed0*cm0 <- iteration 3,4
      FDK_vadd_s32_q(Q0, Q0, Q9)                 // Q1:  seed2_i4    seed1_i4    seed2_i3    seed1_i3

      FDK_vuzp_q(32, Q1, Q0)                     // Q1:  seed1_i4    seed1_i3    seed1_i2    seed1_i1 <-- store via r4
                                                 // Q0:  seed2_i4    seed2_i3    seed2_i2    seed2_i1 <-- store via r5

      FDK_vtst_q(32, Q2, Q1, Q12)                // Q1:  00000000    FFFFFFFF    00000000    FFFFFFFF <- -1, if bit#16 is set, 0 otherwise
      FDK_vmov_q(Q3, Q10)                        // Q3:  noiselev    noiselev    noiselev    noiselev
      FDK_vbit_q(Q3, Q11, Q2)                    // Q3:  noiselev   -noiselev    noiselev   -noiselev

      FDK_vst1_2d_ia(32, D6, D7, r4)
      FDK_vtst_q(32, Q2, Q0, Q12)                // Q1:  00000000    FFFFFFFF    00000000    FFFFFFFF <- -1, if bit#16 is set, 0 otherwise
      FDK_vmov_q(Q3, Q10)                        // Q2:  noiselev    noiselev    noiselev    noiselev
      FDK_vbit_q(Q3, Q11, Q2)                    // Q3:  noiselev   -noiselev    noiselev   -noiselev
      FDK_vst1_2d_ia(32, D6, D7, r5)
      FDK_subs_imm(r3, r3, 4)
      FDK_branch(NE, iisIGFDecLibInjectSourceSpectrumTCX_func2_loop2)

    FDK_strd(r4, r5, r1, 0x00, Tile_pointer_Array[0], Tile_pointer_Array[1])
    FDK_vst1(32, S3, r0)
    FDK_branch(AL, iisIGFDecLibInjectSourceSpectrumTCX_func2_end)

FDK_label(iisIGFDecLibInjectSourceSpectrumTCX_func2_numTile3)
    FDK_vld1(32, S11, r0)
    FDK_ldrd(r4, r5, r1, 0x00, Tile_pointer_Array[0], Tile_pointer_Array[1])
    FDK_ldr(r6, r1, 0x08, Tile_pointer_Array[2])
FDK_label(iisIGFDecLibInjectSourceSpectrumTCX_func2_loop3)
      FDK_vmul_s32_q_scalar(Q3, Q8, S11)         // Q3: --------    seed0*cm2   seed0*cm1   seed0*cm0 <- 1st iteraion
      FDK_vadd_s32_q(Q3, Q3, Q9)                 // Q3: --------    seed3_i1    seed2_i1    seed1_i1
      FDK_vmul_s32_q_scalar(Q2, Q8, S14)         // Q2: --------    seed0*cm2   seed0*cm1   seed0*cm0 <- 2nd iteration
      FDK_vadd_s32_q(Q2, Q2, Q9)                 // Q2: --------    seed3_i2    seed2_i2    seed1_i2
      FDK_vmul_s32_q_scalar(Q1, Q8, S10)         // Q1: --------    seed0*cm2   seed0*cm1   seed0*cm0 <- 3rd iteration
      FDK_vadd_s32_q(Q1, Q1, Q9)                 // Q1: --------    seed3_i3    seed2_i3    seed1_i3
      FDK_vmul_s32_q_scalar(Q0, Q8, S6)          // Q0: --------    seed0*cm2   seed0*cm1   seed0*cm0 <- 4th iteration
      FDK_vadd_s32_q(Q0, Q0, Q9)                 // Q0: --------    seed3_i4    seed2_i4    seed1_i4
      FDK_vzip_q(32, Q3, Q2)                     // Q3: seed2_i2    seed2_i1    seed1_i2    seed1_i1
                                                 // Q2: --------    --------    seed3_i2    seed3_i1
      FDK_vzip_q(32, Q1, Q0)                     // Q1: seed2_i4    seed2_i3    seed1_i4    seed1_i3
                                                 // Q0: --------    --------    seed3_i4    seed3_i3
      FDK_vswp(64, D7, D2)                       // Q3: seed1_i4    seed1_i3    seed1_i2    seed1_i1 <-- store via r4
      FDK_vswp(64, D5, D0)                       // Q2: seed3_i4    seed3_i3    seed3_i2    seed3_i1 <-- store via r6
                                                 // Q1: seed2_i4    seed2_i3    seed2_i2    seed2_i1 <-- store via r5

      /* Convert seed bit #16 into +/- noiselevel, Q3,Q1,Q2 => Q3,Q1,Q0 (keep Q2 unchanged) */
      FDK_vtst_q(32, Q15, Q3, Q12)               // Q15: check bit #16
      FDK_vtst_q(32, Q14, Q2, Q12)               // Q14: check bit #16
      FDK_vtst_q(32, Q13, Q1, Q12)               // Q13: check bit #16
      FDK_vmov_q(Q3, Q10)                        // Q3:  preset noiselevel to all lanes
      FDK_vmov_q(Q0, Q10)                        // Q0:  preset noiselevel to all lanes
      FDK_vmov_q(Q1, Q10)                        // Q1:  preset noiselevel to all lanes
      FDK_vbit_q(Q3, Q11, Q15)                   // Q3:  convert bit#16 into noiselevel, if set
      FDK_vbit_q(Q0, Q11, Q14)                   // Q0:  convert bit#16 into noiselevel, if set
      FDK_vbit_q(Q1, Q11, Q13)                   // Q1:  convert bit#16 into noiselevel, if set

      FDK_vst1_2d_ia(32, D6, D7, r4)
      FDK_vst1_2d_ia(32, D2, D3, r5)
      FDK_vst1_2d_ia(32, D0, D1, r6)
      FDK_subs_imm(r3, r3, 4)
      FDK_branch(NE, iisIGFDecLibInjectSourceSpectrumTCX_func2_loop3)

    FDK_strd(r4, r5, r1, 0x00, Tile_pointer_Array[0], Tile_pointer_Array[1])
    FDK_str(r6, r1, 0x08, Tile_pointer_Array[2])
    FDK_vst1(32, S11, r0)
    FDK_branch(AL, iisIGFDecLibInjectSourceSpectrumTCX_func2_end)

FDK_label(iisIGFDecLibInjectSourceSpectrumTCX_func2_numTile4)
    FDK_vld1(32, S3, r0)                          // S3: *randomSeed
    FDK_ldrd(r4, r5, r1, 0x00, Tile_pointer_Array[0], Tile_pointer_Array[1])
    FDK_ldrd(r6, r7, r1, 0x08, Tile_pointer_Array[2], Tile_pointer_Array[3])

FDK_label(iisIGFDecLibInjectSourceSpectrumTCX_func2_loop4)
      FDK_vmul_s32_q_scalar(Q3, Q8, S3)          // Q3: seed0*cm3   seed0*cm2   seed0*cm1   seed0*cm0 <- 1st iteraion
      FDK_vadd_s32_q(Q3, Q3, Q9)                 // Q3: seed4_i1    seed3_i1    seed2_i1    seed1_i1
      FDK_vmul_s32_q_scalar(Q2, Q8, S15)         // Q2: seed0*cm3   seed0*cm2   seed0*cm1   seed0*cm0 <- 2nd iteration
      FDK_vadd_s32_q(Q2, Q2, Q9)                 // Q2: seed4_i2    seed3_i2    seed2_i2    seed1_i2
      FDK_vmul_s32_q_scalar(Q1, Q8, S11)         // Q1: seed0*cm3   seed0*cm2   seed0*cm1   seed0*cm0 <- 3rd iteration
      FDK_vadd_s32_q(Q1, Q1, Q9)                 // Q1: seed4_i3    seed3_i3    seed2_i3    seed1_i3
      FDK_vmul_s32_q_scalar(Q0, Q8, S7)          // Q0: seed0*cm3   seed0*cm2   seed0*cm1   seed0*cm0 <- 4th iteration
      FDK_vadd_s32_q(Q0, Q0, Q9)                 // Q0: seed4_i4    seed3_i4    seed2_i4    seed1_4
      FDK_vzip_q(32, Q3, Q2)                     // Q3: seed2_i2    seed2_i1    seed1_i2    seed1_i1
                                                 // Q2: seed4_i2    seed4_i1    seed3_i2    seed3_i1
      FDK_vzip_q(32, Q1, Q0)                     // Q1: seed2_i4    seed2_i3    seed1_i4    seed1_i3
                                                 // Q0: seed4_i4    seed4_i3    seed3_i4    seed3_i3
      FDK_vswp(64, D7, D2)                       // Q3: seed1_i4    seed1_i3    seed1_i2    seed1_i1 <-- store via r4
      FDK_vswp(64, D5, D0)                       // Q2: seed3_i4    seed3_i3    seed3_i2    seed3_i1 <-- store via r6
                                                 // Q1: seed2_i4    seed2_i3    seed2_i2    seed2_i1 <-- store via r5
                                                 // Q0: seed4_i4    seed4_i3    seed4_i2    seed4_i1 <-- store via r7

      /* Convert seed bit #16 into +/- noiselevel, Q3,Q1,Q2,Q0 => Q3,Q1,Q2,Q14 (keep Q0 unchanged) */
      FDK_vtst_q(32, Q15, Q3, Q12)               // Q15: check bit #16
      FDK_vtst_q(32, Q14, Q2, Q12)               // Q14: check bit #16
      FDK_vtst_q(32, Q13, Q1, Q12)               // Q13: check bit #16
      FDK_vmov_q(Q3, Q10)                        // Q3:  preset noiselevel to all lanes
      FDK_vmov_q(Q2, Q10)                        // Q2:  preset noiselevel to all lanes
      FDK_vmov_q(Q1, Q10)                        // Q1:  preset noiselevel to all lanes
      FDK_vbit_q(Q3, Q11, Q15)                   // Q3:  convert bit#16 into noiselevel, if set
      FDK_vtst_q(32, Q15, Q0, Q12)               // Q15: check bit #16
      FDK_vbit_q(Q2, Q11, Q14)                   // Q2:  convert bit#16 into noiselevel, if set
      FDK_vmov_q(Q14, Q10)                       // Q14: preset noiselevel to all lanes
      FDK_vbit_q( Q1, Q11, Q13)                  // Q1:  convert bit#16 into noiselevel, if set
      FDK_vbit_q(Q14, Q11, Q15)                  // Q14: convert bit#16 into noiselevel, if set

      FDK_vst1_2d_ia(32, D6, D7, r4)
      FDK_vst1_2d_ia(32, D2, D3, r5)
      FDK_vst1_2d_ia(32, D4, D5, r6)
      FDK_vst1_2d_ia(32, D28, D29, r7)
      FDK_subs_imm(r3, r3, 4)
      FDK_branch(NE, iisIGFDecLibInjectSourceSpectrumTCX_func2_loop4)

    FDK_strd(r4, r5, r1, 0x00, Tile_pointer_Array[0], Tile_pointer_Array[1])
    FDK_strd(r6, r7, r1, 0x08, Tile_pointer_Array[2], Tile_pointer_Array[3])
    FDK_vst1(32, S3, r0)
//FDK_branch(AL, iisIGFDecLibInjectSourceSpectrumTCX_func2_end)

FDK_label(iisIGFDecLibInjectSourceSpectrumTCX_func2_end)
    FDK_mpop(r4,r7)
    FDK_return()
FDK_ASM_ROUTINE_END()

static const ULONG iisIGFDecLibInjectSourceSpectrumTCX_func2_constants[8] =
{
  0x00010DCD, /* 69069, */
  0x1C587629, /* 69069*69069, */
  0xA6FFB3D5, /* 69069*69069*69069, */
  0x6AB9D291, /* 69069*69069*69069*69069, */
  0x00000005, /*                                   5 */
  0x00054506, /*                        5 *69069 + 5 */
  0x8DBF93D3, /*             (5*69069 + 5)*69069 + 5 */
  0xD0BE16FC  /* ((5*69069 + 5)*69069 + 5)*69069 + 5 */
};

void iisIGFDecLibInjectSourceSpectrumTCX_func2(
      ULONG    * randomSeed,
      FIXP_DBL * Tile_pointer_Array[],
      INT        IGFNumTile,
      FIXP_DBL   noise_level,
      INT        NumSb)
{
    iisIGFDecLibInjectSourceSpectrumTCX_func2_ARMv7NEON(randomSeed, Tile_pointer_Array, IGFNumTile, noise_level, NumSb, iisIGFDecLibInjectSourceSpectrumTCX_func2_constants);
}

#endif /* FUNCTION_iisIGFDecLibInjectSourceSpectrumTCX_func2 */

#ifdef FUNCTION_DotProduct_func1
FDK_ASM_ROUTINE_START(FIXP_DBL, DotProduct_func1,
 (  const FIXP_DBL *Input,   /* r0: Input   */
    INT shift,               /* r1: shift=getScalefactor(Input,length)   */
    INT loop_shift,          /* r1: loop_shift */
    INT length))             /* r2: length  */

     /* stack contents:
        none

     register usage:
        r0:  Input[0..]
        r1:  shift
        r2:  loop_shift
        r3:  length


      NEON register usage:
        Q15:
        Q14:
        Q13:
        Q12:
        Q11:  loop_shift loop_shift loop_shift loop_shift
        Q10:  shift    shift    shift    shift
        Q9:   Input[3] Input[2] Input[1] Input[0]
        Q8:
        Q7:  reserved    reserved    reserved    reserved
        Q6:  reserved    reserved    reserved    reserved
        Q5:  reserved    reserved    reserved    reserved
        Q4:  reserved    reserved    reserved    reserved
        Q3:
        Q2:
        Q1:
        Q0:  acc[3] acc[2] acc[1] acc[0]
    */

#ifndef __ARM_NEON__
    FIXP_DBL *r0 = (FIXP_DBL*)Input;
    INT  r1 = (INT)shift;
    INT  r2 = (INT)loop_shift;
    INT  r3 = (INT)length;
#endif

    FDK_vdup_q_reg(32, Q10, r1)                  // Q10:  shift    shift    shift    shift
    FDK_add_imm(r2, r2, 1, 0)                    // r2: increase value by 1 due to fPow2Div2
    FDK_rsb_imm(r2, r2, 0)                       // r2: invert loop shift value
    FDK_vdup_q_reg(32, Q11, r2)                  // Q11: -(loop_shift+1)  -(loop_shift+1)  -(loop_shift+1)  -(loop_shift+1)
    FDK_vmov_i32(128, Q0, 0)                     //
    FDK_vmov_i32(128, Q9, 0)                     //

    FDK_tst_imm(r3, 1)
    FDK_branch(EQ, DotProduct_func1_tst2)
      FDK_vld1_ia(32, S38, r0)                    // Q9: 0     Input[0]     0      0

FDK_label(DotProduct_func1_tst2)
    FDK_tst_imm(r3, 2)
    FDK_branch(EQ, DotProduct_func1_tst3)
      FDK_vld1_1d_ia(32, D18, r0)                 // Q9: 0     Input[0]  Input[2]   Input[1]

FDK_label(DotProduct_func1_tst3)
    FDK_tst_imm(r3, 3)
    FDK_branch(EQ, DotProduct_func1_tst_n)
    FDK_vshl_s32_q(Q9, Q9, Q10)                // Q9:  Input[i]<< shift
    FDK_vqdmulh_s32_qq(Q9, Q9, Q9)             // Q9:  Input[i] = fMult(Input[i]<< shift, Input[i]<< shift)
    FDK_vshl_s32_q(Q9, Q9, Q11)                // Q9:  acc = acc << -(loop_shift+1)
    FDK_vadd_s32_q(Q0, Q0, Q9)                 //

FDK_label(DotProduct_func1_tst_n)
    FDK_asrs_imm(r3, r3, 2)                    //  right shift by 2
    FDK_branch(EQ, DotProduct_func1_end)

FDK_label(DotProduct_func1_loop)
      FDK_vld1_2d_ia(32, D18, D19, r0)           // Q9:  Input[3] Input[2] Input[1] Input[0]
      FDK_vshl_s32_q(Q9, Q9, Q10)                // Q9:  Input[i]<< shift
      FDK_vqdmulh_s32_qq(Q9, Q9, Q9)             // Q9:  Input[i] = fMult(Input[i]<< shift, Input[i]<< shift)
      FDK_vshl_s32_q(Q9, Q9, Q11)                // Q9:  acc = acc << -(loop_shift+1)
      FDK_vadd_s32_q(Q0, Q0, Q9)                 //
      FDK_subs_imm(r3, r3, 1)                    // decrement loop counter by 1 instead of 4 due to prior right shift
      FDK_branch(NE, DotProduct_func1_loop)

FDK_label(DotProduct_func1_end)

    FDK_vadd_s32_d(D0, D0, D1)                   // Q0:  acc =  [S0 S2]+[S1 S3]
    FDK_vpadd_s32(D0, D0, D0)                    // Q0:  acc =  [S0] +  [S1]
    FDK_vmov_reg(r0, S0)                         // r0: Store return value
    FDK_return()
    FDK_ASM_ROUTINE_RETURN(FIXP_DBL)

#endif /* FUNCTION_DotProduct_func1 */

#ifdef FUNCTION_iisIGF_ScaleSurvivedLines_func1
FDK_ASM_ROUTINE_START(void, iisIGF_ScaleSurvivedLines_func1,
    (FIXP_DBL * pSpectralData,                  /* i/o     r0 */
     FIXP_DBL * fSfbDestinEnergyTab,            /* i       r1 */
     SCHAR    * fSfbDestinEnergyTab_exp,        /* i       r2 */
     SHORT    * iSfbWidthTab,                   /* i       r3 */
     SHORT      iSfbCnt))                       /* i      [0] */

     /* stack contents:
        0x08: iSfbCnt
        0x04: r5
        0x00: r4

     register usage:
        r0:  pSpectralData
        r1:  fSfbDestinEnergyTab
        r2:  fSfbDestinEnergyTab_exp
        r3:  iSfbWidthTab
        r4:  iSfbCnt
        r5:  width


      NEON register usage:
        Q8-Q15:
        Q7:  reserved    reserved    reserved    reserved
        Q6:  reserved    reserved    reserved    reserved
        Q5:  reserved    reserved    reserved    reserved
        Q4:  reserved    reserved    reserved    reserved
        Q3:
        Q2:       scf         scf         scf         scf
        Q1:     scf_e       scf_e       scf_e       scf_e
        Q0:  pSpec[3]    pSpec[2]    pSpec[1]    pSpec[0]
    */

    FDK_mpush(r4, r5)
    FDK_ldrh(r4, sp, 0x08, iSfbCnt)              // r4: iSfbCnt

FDK_label(iisIGF_ScaleSurvivedLines_loop_sfb)
      FDK_ldrh_ia(r5, r3, 2)                     // r5:  width
      FDK_vld1dup_2d_ia(32, D4, D5, r1)          // Q2:       scf         scf         scf         scf
      FDK_vld1dup_1d_ia(16, D2, r2)              // Q1:                         scf_e scf_e scf_e scf_e
      FDK_vmovl_s(16, Q1, D2)                    // Q1:     scf_e       scf_e       scf_e       scf_e
FDK_label(iisIGF_ScaleSurvivedLines_loop_width)
        FDK_vld1_2d(32, D0, D1, r0)              // Q0:  pSpec[3]    pSpec[2]    pSpec[1]    pSpec[0] <-- read input
        FDK_subs_imm(r5, r5, 4)
        FDK_vqdmulh_s32_qq(Q0, Q0, Q2)           // Q0: fMult(pSpec[i], scf)
        FDK_vshl_s32_q(Q0, Q0, Q1)               // Q0: scaleValue(fMult(pSpec[i], scf),scf_e)
        FDK_vst1_2d_ia(32, D0, D1, r0)           // Q0:  pSpec[3]    pSpec[2]    pSpec[1]    pSpec[0] --> store output
        FDK_branch(NE, iisIGF_ScaleSurvivedLines_loop_width)

      FDK_subs_imm(r4, r4, 1)                    // r4: iSfbCnt--
      FDK_branch(NE, iisIGF_ScaleSurvivedLines_loop_sfb)

    FDK_mpop(r4, r5)
    FDK_return()

FDK_ASM_ROUTINE_END()
#endif /* FUNCTION_iisIGF_ScaleSurvivedLines_func1 */

