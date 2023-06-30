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

   Description: (ARM optimised) Scaling of spectral data

*******************************************************************************/

/* clang-format off */
#define FUNCTION_CBlock_ScaleSpectralData_func1

#if defined(__ARM_NEON__)
#define FUNCTION_get_gain_func1
#endif


#if defined(__GNUC__)
#include "arm/FDK_arm_funcs.h"
#if defined(__ARM_ARCH_6__)
#endif
#endif

/* Note: This loop is only separated for ARM in order to save cycles
         by loop unrolling. The ARM core provides by default a 5-cycle
         loop overhead per sample, that goes down to 1-cycle per sample
         with an optimal 4x-loop construct (do - 4x - while).
*/
static inline void CBlock_ScaleSpectralData_func1(
    FIXP_DBL *pSpectrum,
    int maxSfbs,
    const SHORT * RESTRICT BandOffsets,
    int SpecScale_window,
    const SHORT * RESTRICT pSfbScale,
    int window)
{
  int band_offset = 0;
  for (int band=0; band < maxSfbs; band++)
  {
    int runs = band_offset;
    band_offset = BandOffsets[band+1];
    runs = band_offset - runs;    /* is always a multiple of 4 */
    FDK_ASSERT((runs & 3) == 0);
    int scale = fMin(DFRACT_BITS-1, SpecScale_window - pSfbScale[window*16+band]);

    if (scale)
    {
      do
      {
        FIXP_DBL tmp0, tmp1, tmp2, tmp3;
        tmp0 = pSpectrum[0];
        tmp1 = pSpectrum[1];
        tmp2 = pSpectrum[2];
        tmp3 = pSpectrum[3];
        tmp0 >>= scale;
        tmp1 >>= scale;
        tmp2 >>= scale;
        tmp3 >>= scale;
        *pSpectrum++ = tmp0;
        *pSpectrum++ = tmp1;
        *pSpectrum++ = tmp2;
        *pSpectrum++ = tmp3;
      } while ((runs = runs-4) != 0);
    }
    else
    {
      pSpectrum+= runs;
    }
  }
}

#if defined(FUNCTION_maxabs_D)
#if defined(__GNUC__)
FDK_ASM_ROUTINE_START(FIXP_DBL,maxabs_D,(
        FIXP_DBL *pSpectralCoefficient,
        int noLines
        ))

  // save upper core registers according to call convention
  FDK_mpush(r4,r6)

  /* stack contents:
       0x10:     r6
       0x0C:     r5
       0x08:     r4
       0x04:     r3
       0x00:     r2

     register contents:
       r0: vector
       r1: len

     register usage:
       r0: pointer to input/output vector
       r1: vector length

       r2: 1st output value
       r3: 2nd output value
       r4: 3rd output value
       r5: 4th output value
       r6: holds the max. value
  */

  /* load 4 spectral values and increment pointer by 4 */
  FDK_ldrd_ia(r2, r3, r0, 8)

  /* Initialize max value register */
  FDK_mov_imm(r6, 0)

FDK_label(maxabs_D_preroll)

  /* Calc abs of 1 spectral value */
  FDK_eor_op_asr(r2, r2, r2, 31, 2) /* abs() */

  FDK_ldrd_ia(r4, r5, r0, 8)

  FDK_eor_op_asr(r3, r3, r3, 31, 2) /* abs() */
  FDK_eor_op_asr(r4, r4, r4, 31, 2) /* abs() */
  FDK_eor_op_asr(r5, r5, r5, 31, 2) /* abs() */

  FDK_subs_imm(r1, r1, 4)  /* decrement by 4 */
  FDK_branch(LE, maxabs_D_postroll)  /* repeat if loop counter>0*/

FDK_label(maxabs_D_loop_4x)
  /* find max of the 4 values */
  FDK_orr(r6, r6, r2)
  FDK_orr(r6, r6, r3)
  FDK_orr(r6, r6, r4)

  /* load 2 spectral values and increment pointer by 2 */
  FDK_ldrd_ia(r2, r3, r0, 8)

  FDK_orr(r6, r6, r5)

  /* Calc abs of 1 spectral values */
  FDK_eor_op_asr(r2, r2, r2, 31, 2) /* abs() */

  FDK_ldrd_ia(r4, r5, r0, 8)

  FDK_eor_op_asr(r3, r3, r3, 31, 2) /* abs() */
  FDK_eor_op_asr(r4, r4, r4, 31, 2) /* abs() */
  FDK_eor_op_asr(r5, r5, r5, 31, 2) /* abs() */

  FDK_subs_imm(r1, r1, 4)  /* decrement by 4 */
  FDK_branch(GT, maxabs_D_loop_4x)  /* repeat if loop counter>0*/

FDK_label(maxabs_D_postroll)
  /* find max of the 4 values */
  FDK_orr(r6, r6, r2)
  FDK_orr(r6, r6, r3)
  FDK_orr(r6, r6, r4)
  FDK_orr(r6, r6, r5)

  /* Return greatest value */
  FDK_mov_reg(r0, r6)
  // restore upper core registers according to call convention
  FDK_mpop(r4,r6)
  FDK_return()
FDK_ASM_ROUTINE_END()
#else
static inline FIXP_DBL maxabs_D (
        const FIXP_DBL *pSpectralCoefficient,
        const int noLines)
{
  /* Find max spectral line value of the current sfb */
  FIXP_DBL maxVal = (FIXP_DBL)0;
  int i;

  DWORD_ALIGNED(pSpectralCoefficient);
  FDK_PRAGMA_MUST_ITERATE(noLines,4,2048,4)

  for (i = noLines; i-- > 0; ) {
    /* Expensive memory access */
    maxVal = fMax(fixp_abs(pSpectralCoefficient[i]), maxVal);
  }

  return maxVal;
}
#endif /* defined(__CC_ARM) || defined(__GNUC__) */
#endif /* defined(FUNCTION_maxabs_D) */

#if defined(FUNCTION_scale_imdct_samples)
static inline void scale_imdct_samples(
        const FIXP_DBL *const vecIn,
        FIXP_DBL *const vecOut,
        const SHORT frameLen,
        const int stride
)
{
  int i;

  FDK_PRAGMA_MUST_ITERATE(frameLen/2,240,2048,16)
  for (i=0; i<frameLen; i++)
  {
    vecOut[i*stride] = (vecIn[i] > (MAXVAL_DBL>>MDCT_OUT_HEADROOM)) ? MAXVAL_DBL : vecIn[i]<<MDCT_OUT_HEADROOM;
  }
}

#endif /* defined(FUNCTION_scale_imdct_samples) */

#if defined(FUNCTION_InverseQuantizeBand)
FDK_ASM_ROUTINE_START(void,InverseQuantizeBand,(
        FIXP_DBL *spectrum,
        const FIXP_DBL *InverseQuantTabler,
        const FIXP_DBL *MantissaTabler,
        const SCHAR *ExponentTabler,
        INT noLines,
        INT scale
        ))

  // save upper core registers according to call convention
  FDK_mpush(r4,r12)

  /* stack contents:
       0x28:     scale
       0x24:     noLines
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
       r0: pointer to input/output spectrum
       r1: pointer to inverse quantization table
       r2: pointer to mantissa table
       r3: pointer to exponent table

       r4: number of lines, counter
       r5: scale

       r6: 1st input value
       r7: exponent table value

       r8: clz / value headroom for shift, later exponent for table access
       r9: interpolation factor for table values

       r10: invquant table value 0
       r11: invquant table value 1, retVal

       r12: asb(value), tableIndex
  */

  /* Load arguments */
  FDK_ldrd (r4, r5, sp, 0x24, noLines, scale)

  FDK_cmp_imm (r4, 0)
  FDK_branch(LE, InverseQuantizeBand_end)
  FDK_ldr(r6, r0, 0, spectrum)

  FDK_add_imm(r5, r5, 1, 0)

FDK_label(InverseQuantizeBand_loop)
    FDK_movs_reg(r12, r6)
    FDK_branch(EQ, InverseQuantizeBand_loop_end)
    FDK_rsb_cond_imm(LT, r12, r12, 0) /* abs */

    // shift value to full scale
    FDK_clz(r8, r12)
    FDK_lsl_imm(r12, r12, 1)
    FDK_lsl(r12, r12, r8)

    // use r12[31:24] as invquant table index
    // note: is shifted here by 22 only, since we need 32-bit values
    // and therefore want to address by 4-bytes values.
    FDK_lsr_imm(r9, r12, 22)
    FDK_and_imm(r9, r9, 0xfffffffc)

    // use r12[23:20] for interpolation
    FDK_lsr_imm(r12, r12, 20)
    FDK_and_imm(r12, r12, 0x0f)

    // load two values from invquant table
    FDK_ldrd_reg(r10, r11, r1, r9)

    // r8=exponent
    FDK_rsb_imm(r8, r8, 32)

    // r12=(r11-r10)*r9 + (r10<<4)
    FDK_sub(r12, r11, r10)

    FDK_ldr_reg_oplsl(r11, r2, r8, 2) // load mantissa table value

    FDK_smmul(r12, r12, r9)

    // scale value
    FDK_ldrsb_reg(r7, r3, r8, ExponentTabler+r8) // load exponent table value

    FDK_add_op_lsl(r12, r12, r10, 4, 0)

    // mantissa calculation
    FDK_smmul(r12, r12, r11)

    FDK_adds(r7, r7, r5) // add exponent to scaling

    FDK_rsb_cond_imm(LT, r7, r7, 0)  // invert shift factor sign and ...
    FDK_asr_cond(LT, r12, r12, r7)   // ... shift right or ...
    FDK_lsl_cond(GT, r12, r12, r7)   // shift left

    // restore sign
    FDK_cmp_imm(r6, 0)
    FDK_rsb_cond_imm(LT, r12, r12, 0)

    // store value
    FDK_str(r12, r0, 0, spectrum)

FDK_label(InverseQuantizeBand_loop_end)
    FDK_add_imm(r0, r0, 4, 2)
    FDK_subs_imm(r4, r4, 1)
    FDK_ldr_cond(GT, r6, r0, 0)
  FDK_branch(GT, InverseQuantizeBand_loop)

FDK_label(InverseQuantizeBand_end)
  FDK_mpop(r4,r12)
  FDK_return()
FDK_ASM_ROUTINE_END()
#endif /* defined(FUNCTION_InverseQuantizeBand) */


#ifdef FUNCTION_get_gain_func1
FDK_ASM_ROUTINE_START(void,  get_gain_func1,
    (const FIXP_DBL *x,
     const FIXP_DBL *y,
           INT       headroom_x,
           INT       headroom_y,
           INT       width_shift,
           INT       n,
           FIXP_DBL *corr,
           FIXP_DBL *ener))

     /* stack contents:
        0x0C: ener
        0x08: corr
        0x04: n
        0x00: width_shift

     register usage:
        r0:  x
        r1:  y
        r2:  headroom_x, width_shift, corr
        r3:  headroom_y, n,           ener

      NEON register usage:
        Q15: -width_shift-1
        Q14: headroom_y (duplicated)
        Q13: headroom_x (duplicated)
        Q12:
        Q11:
        Q10:
        Q9:  fMult(y[i]<<hdr_y, y[i]<<hdr_y])>>(width_shift+1) --> Q3
        Q8:  fMult(x[i]<<hdr_x, y[i]<<hdr_y])>>(width_shift+1) --> Q2
        Q7:  reserved    reserved    reserved    reserved
        Q6:  reserved    reserved    reserved    reserved
        Q5:  reserved    reserved    reserved    reserved
        Q4:  reserved    reserved    reserved    reserved
        Q3:   ener[3]     ener[2]     ener[1]     ener[0] <-- += Q9
        Q2:   corr[3]     corr[2]     corr[1]     corr[0] <-- += Q8
        Q1:      y[3]        y[2]        y[1]        y[0]
        Q0:      x[3]        x[2]        x[1]        x[0]
    */

    FDK_vdup_q_reg(32, Q13, r2)
    FDK_vdup_q_reg(32, Q14, r3)
    FDK_vmov_i32(128, Q0, 0)
    FDK_vmov_i32(128, Q1, 0)
    FDK_vmov_i32(128, Q2, 0)
    FDK_vmov_i32(128, Q3, 0)
    FDK_mov_imm(r2, 1)
    FDK_vmov_sn(S15, r2)                         // Q3:  00000001    00000000    00000000    00000000 (ener)
    FDK_ldrd(r2, r3, sp, 0x00, width_shift, n)
    FDK_add_imm(r2, r2, 1, 0)                    // r2:  width_shift + 1
    FDK_rsb_imm(r2, r2, 0)                       // r2: -width_shift - 1
    FDK_vdup_q_reg(32, Q15, r2)

//FDK_label(get_gain_func1_tst1)
    FDK_tst_imm(r3, 1)
    FDK_branch(EQ, get_gain_func1_tst2)
      FDK_vld1_ia(32, S2, r0)                    // S2: x[0]
      FDK_vld1_ia(32, S6, r1)                    // S6: y[0]

FDK_label(get_gain_func1_tst2)
    FDK_tst_imm(r3, 2)
    FDK_branch(EQ, get_gain_func1_tst3)
      FDK_vld1_1d_ia(32, D0, r0)                 // D0: x[1]     x[0]
      FDK_vld1_1d_ia(32, D2, r1)                 // D2: y[1]     y[0]

FDK_label(get_gain_func1_tst3)
    FDK_tst_imm(r3, 3)
    FDK_branch(EQ, get_gain_func1_tst_n)
      FDK_vshl_s32_q(Q1, Q1, Q14)                // Q1: y[i] << headroom_x
      FDK_vshl_s32_q(Q0, Q0, Q13)                // Q0: x[i] << headroom_x
      FDK_vqdmulh_s32_qq(Q9, Q1, Q1)             // Q9: fMult(y[i]<<hdr_y, y[i]<<hdr_y])
      FDK_vqdmulh_s32_qq(Q8, Q0, Q1)             // Q8: fMult(x[i]<<hdr_x, y[i]<<hdr_y])
      FDK_vshl_s32_q(Q9, Q9, Q15)                // Q9: fMult(y[i]<<hdr_y, y[i]<<hdr_y])>>(width_shift+1)
      FDK_vshl_s32_q(Q8, Q8, Q15)                // Q8: fMult(x[i]<<hdr_x, y[i]<<hdr_y])>>(width_shift+1)
      FDK_vadd_s32_q(Q3, Q3, Q9)                 // Q3:   ener[3]     ener[2]     ener[1]     ener[0] <-- += Q9
      FDK_vadd_s32_q(Q2, Q2, Q8)                 // Q2:   corr[3]     corr[2]     corr[1]     corr[0] <-- += Q8

FDK_label(get_gain_func1_tst_n)
    FDK_asrs_imm(r3, r3, 2)
    FDK_branch(EQ, get_gain_func1_end)

FDK_label(get_gain_func1_loop)
      FDK_vld1_2d_ia(32, D0, D1, r0)
      FDK_vld1_2d_ia(32, D2, D3, r1)
      FDK_vshl_s32_q(Q1, Q1, Q14)                // Q1: y[i] << headroom_x
      FDK_vshl_s32_q(Q0, Q0, Q13)                // Q0: x[i] << headroom_x
      FDK_vqdmulh_s32_qq(Q9, Q1, Q1)             // Q9: fMult(y[i]<<hdr_y, y[i]<<hdr_y])
      FDK_vqdmulh_s32_qq(Q8, Q0, Q1)             // Q8: fMult(x[i]<<hdr_x, y[i]<<hdr_y])
      FDK_vshl_s32_q(Q9, Q9, Q15)                // Q9: fMult(y[i]<<hdr_y, y[i]<<hdr_y])>>(width_shift+1)
      FDK_vshl_s32_q(Q8, Q8, Q15)                // Q8: fMult(x[i]<<hdr_x, y[i]<<hdr_y])>>(width_shift+1)
      FDK_vadd_s32_q(Q3, Q3, Q9)                 // Q3:   ener[3]     ener[2]     ener[1]     ener[0] <-- += Q9
      FDK_vadd_s32_q(Q2, Q2, Q8)                 // Q2:   corr[3]     corr[2]     corr[1]     corr[0] <-- += Q8
      FDK_subs_imm(r3, r3, 1)
      FDK_branch(NE, get_gain_func1_loop)

FDK_label(get_gain_func1_end)
    FDK_ldrd(r2, r3, sp, 0x08, corr, ener)
    FDK_vadd_s32_d(D0, D4, D5)                   // D0:   corr{3,2]    corr[1,0]
    FDK_vadd_s32_d(D1, D6, D7)                   // D1:   ener[3,2]    ener[1,0]
    FDK_vpadd_s32(D0, D0, D1)                    // D0:     ener         corr
    FDK_vst1(32, S0, r2)                         // r2: store corr
    FDK_vst1(32, S1, r3)                         // r3: store ener
    FDK_return()
FDK_ASM_ROUTINE_END()

#endif /* FUNCTION_get_gain_func1 */
