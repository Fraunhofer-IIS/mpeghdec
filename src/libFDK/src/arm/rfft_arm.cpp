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

   Description: RFFT subroutines optimized for ARMv7 NEON

*******************************************************************************/

/* clang-format off */
#if defined(__ARM_NEON__)
#include "arm/FDK_arm_funcs.h"
#include "arm/FDK_neon_funcs.h"

#define FUNCTION_fft_postsort_func1
#define FUNCTION_fft_presort_func1
#endif


#ifdef FUNCTION_fft_postsort_func1
FDK_ASM_ROUTINE_START(void, fft_postsort_func1,
    (FIXP_DBL *      buf,       /* r0  */
     const INT       N,         /* r1  */
     const FIXP_STP *w,         /* r2  */
     INT             step))     /* r3  */

#ifndef __ARM_NEON__
      FIXP_DBL *r0 = buf;
      INT       r1 = N;
      FIXP_STP *r2 = (FIXP_STP *) w;
      INT       r3 = step;
      FIXP_DBL *r4;
      FIXP_DBL *r5;
      FIXP_DBL *r6;
      INT       lr;
#endif

    FDK_mpush_lr(r4, r6)

    // FIXP_DBL *pBuf_0 = &buf[0];          /* r0: moves ++4   */
    // FIXP_DBL *pBuf_1 = &buf[N-4];        /* r5: moves --4   */
    // FIXP_DBL *pBuf_2 = &buf[N/2];        /* r6: moves --4   */
    // FIXP_DBL *pBuf_3 = &buf[N/2+2];      /* r7: moves ++4   */

    FDK_add_op_lsl(r4, r0, r1, 2, 2)        /* r4:          &buf[N]     */
    FDK_sub_imm(r4, r4, 16, 2)              /* r4: pBuf_1 = &buf[N-4]   */
    FDK_asr_imm(r1, r1, 1)                  /* r1: N -> N/2             */
    FDK_add_op_lsl(r2, r2, r3, 2, 2)        /* r2: &w[step]             */
    FDK_lsl_imm(r3, r3, 2)                  /* r3: step -> 4*step       */
    FDK_add_op_lsl(r5, r0, r1, 2, 2)        /* r5: pBuf_2 = &buf[N/2]   */
    FDK_add_imm(r6, r5, 8, 2)               /* r6: pBuf_3 = &buf[N/2+2] */

    FDK_vld1_1d(32, D0, r0)                 /* Q0:                           pBuf_0[1]   pBuf_0[0]   */
    FDK_vld1_1d(32, D1, r5)                 /* Q0:  pBuf_2[1]   pBuf_2[0]    pBuf_0[1]   pBuf_0[0]   */

    FDK_mov_imm(lr, -16)

    /* Create Q15 with integer +/- 1 */
    FDK_vmov_i32(64, D30, 1)                /* Q15:                          0000.0001   0000.0001   */
    FDK_vneg_d(32, D31, D30)                /* Q15: FFFF.FFFF   FFFF.FFFF    0000.0001   0000.0001   */
    FDK_vzip_d(32, D30, D31)                /* Q15: FFFF.FFFF   0000.0001    FFFF.FFFF   0000.0001   */

    FDK_vmul_s32_q(Q1, Q0, Q15)             /* Q1: -pBuf_2[1]   pBuf_2[0]   -pBuf_0[1]   pBuf_0[0]   */
    FDK_vmov_i32(64, D1, 0)                 /* Q0:  0000.0000   0000.0000    pBuf_0[1]   pBuf_0[0]   */
    FDK_vzip_d(32, D1, D3)                  /* Q1: -pBuf_2[1]   0000.0000   -pBuf_0[1]   pBuf_0[0]   */
                                            /* Q0:  pBuf_2[0]   0000.0000    pBuf_0[1]   pBuf_0[0]   */
    FDK_vshr_s32_q_imm(Q0, Q0, 1)
    FDK_vshr_s32_q_imm(Q1, Q1, 1)

    FDK_vpadd_s32(D0, D0, D2)               /* D0:                           (t1-t2)/2   (t1+t2)/2   */
    FDK_vpadd_s32(D1, D1, D3)               /* D1:  -p2[1]/2    p2[0]/2      (t1-t2)/2   (t1+t2)/2   */

    // pBuf_0[0] = ((tmp1>>1) + (tmp2>>1));
    // pBuf_0[1] = ((tmp1>>1) - (tmp2>>1));
    // pBuf_0 += 2;
    FDK_vst1_1d_ia(32, D0, r0)              /* D0: store with pointer update (t1-t2)/2   (t1+t2)/2   */

    // pBuf_2[0] =  (pBuf_2[0] >> 1);
    // pBuf_2[1] = -(pBuf_2[1] >> 1);
    // pBuf_2 -= 4;
    FDK_vst1_1d_pu(32, D1, r5, lr)         /* D1: -p2[1]/2    p2[0]/2 store with pointer update: -4 */

    FDK_asr_imm(r1, r1, 3)
FDK_label(fft_postsort_func1_loop)
      FDK_vld1_2d(32, D0, D1, r0)           /* Q0:  pBuf_0[3]   pBuf_0[2]    pBuf_0[1]   pBuf_0[0]   */
      FDK_vld1_2d(32, D2, D3, r4)           /* Q1:  pBuf_1[3]   pBuf_1[2]    pBuf_1[1]   pBuf_1[0]   */
      FDK_vld1_2d(32, D4, D5, r5)           /* Q2:  pBuf_2[3]   pBuf_2[2]    pBuf_2[1]   pBuf_2[0]   */
      FDK_vswp(64, D2, D3)                  /* Q1:  pBuf_1[1]   pBuf_1[0]    pBuf_1[3]   pBuf_1[2]   */
      FDK_vld1_2d(32, D6, D7, r6)           /* Q3:  pBuf_3[3]   pBuf_3[2]    pBuf_3[1]   pBuf_3[0]   */

      FDK_vhadd_s32_q(Q8, Q0, Q1)           /* Q8:  sim1        sre1         sim0        sre0        */
      FDK_vhsub_s32_q(Q9, Q0, Q1)           /* Q9:  dim1        dre1         dim0        dre0        */

      FDK_vswp(64, D4, D5)                  /* Q2:  pBuf_2[1]   pBuf_2[0]    pBuf_2[3]   pBuf_2[1]   */
      FDK_vhadd_s32_q(Q10, Q2, Q3)          /* Q10: sim3        sre3         sim2        sre2        */
      FDK_vhsub_s32_q(Q11, Q2, Q3)          /* Q11: dim3        dre3         dim2        dre2        */

      FDK_vld1_pu(32, S8, r2, r3)           /* Q2:                                       w0          */
      FDK_vld1_pu(32, S9, r2, r3)           /* Q2:                           w1          w0          */
      FDK_vshll_s16_imm(Q2, D4, 16)         /* Q2.  w1.im       w1.re        w0.im       w0.re       */
      FDK_vzip_d(32, D4,  D5)               /* Q2.  w1.im       w0.im        w1.re       w0.re       */
      FDK_vuzp_q(32, Q8, Q10)               /* Q8:  sre3        sre2         sre1        sre0        */
                                            /* Q10: sim3        sim2         sim1        sim0        */
      FDK_vuzp_q(32, Q9, Q11)               /* Q9:  dre3        dre2         dre1        dre0        */
                                            /* Q11: dim3        dim2         dim1        dim0        */
      FDK_vswp(64, D18, D20)                /* Q10: sim3        sim2         dre1        dre0      <-- imag input for cplx MPY  */
                                            /* Q9:  dre3        dre2         sim1        sim0      <-- real input for cplx MPY  */
      FDK_vqdmulh_s32_qq(Q12, Q9, Q2)       /* Q12: dre3*im1    dre2*im0     sim1*re1    sim0*re0  <-- RE*im, RE*re */
      FDK_subs_imm(r1, r1, 1)
      FDK_vqdmulh_s32_qq(Q13, Q10,Q2)       /* Q13: sim3*im1    sim2*im0     dre1*re1    dre0*re0  <-- IM*im, IM*re */
      FDK_vswp(64, D4, D5)                  /* Q2:  w1.re       w0.re        w1.im       w0.im       */
      FDK_vqdmulh_s32_qq(Q14, Q9, Q2)       /* Q14: dre3*re1    dre2*re0     sim1*im1    sim0*im0  <-- RE*re, RE*im */
      FDK_vqdmulh_s32_qq(Q2, Q10, Q2)       /* Q2:  sim3*re1    sim2*re0     dre1*im1    dre0*im0  <-- IM*re, IM*im */

      FDK_vswp(64, D25, D29)                /* Q12: dre3*re1    dre2*re0     sim1*re1    sim0*re0  <-- RE*re, RE*re */
                                            /* Q14: dre3*im1    dre2*im0     sim1*im1    sim0*im0  <-- RE*im, RE*im */
      FDK_vswp(64, D4, D26)                 /* Q2:  sim3*re1    sim2*re0     dre1*re1    dre0*re0  <-- IM*re, IM*re */
                                            /* Q13: sim3*im1    sim2*im0     dre1*im1    dre0*im0  <-- IM*im, IM*im */
      FDK_vsub_s32_q(Q12, Q12, Q13)         /* Q12: tmp7        tmp5         tmp3        tmp1      <-- real output of cplx MPY */
      FDK_vadd_s32_q(Q13, Q14, Q2)          /* Q13: tmp8        tmp6         tmp4        tmp2      <-- imag output of cplx MPY */

      FDK_vzip_q(32, Q12, Q13)              /* Q12: tmp4        tmp3         tmp2        tmp1        */
                                            /* Q13: tmp8        tmp7         tmp6        tmp5        */
      FDK_vzip_q(32, Q8, Q11)               /* Q8:  dim1        sre1         dim0        sre0        */
                                            /* Q11: dim3        sre3         dim2        sre2        */
      FDK_vmul_s32_q(Q2, Q12, Q15)          /* Q2: -tmp4        tmp3        -tmp2        tmp1        */
      FDK_vhadd_s32_q(Q3, Q8, Q2)           /* Q3:  pBuf_0[3]   pBuf_0[2]    pBuf_0[1]   pBuf_0[0]  <-- ready to store */
      FDK_vhsub_s32_q(Q2, Q8, Q2)           /* Q2: -pBuf_1[1]   pBuf_1[0]   -pBuf_1[3]   pBuf_1[2]   */
      FDK_vmul_s32_q(Q2, Q2, Q15)           /* Q2:  pBuf_1[1]   pBuf_1[0]    pBuf_1[3]   pBuf_1[2]   */
      FDK_vst1_2d_ia(32, D6, D7, r0)        /* store pBuf_0[0..3] with pointer increment ++4         */
      FDK_vswp(64, D4, D5)                  /* Q2:  pBuf_1[3]   pBuf_1[2]    pBuf_1[1]   pBuf_1[0]  <-- ready to store */

      FDK_vhadd_s32_q(Q0, Q11, Q13)         /* Q0: -pBuf_3[3]   pBuf_3[2]   -pBuf_3[1]   pBuf_3[0]   */
      FDK_vmul_s32_q(Q0, Q0, Q15)           /* Q0:  pBuf_3[3]   pBuf_3[2]    pBuf_3[1]   pBuf_3[0]  <-- ready to store */
      FDK_vst1_2d_pu(32, D4, D5, r4, lr)    /* store pBuf_1[0..3] with pointer decrement --4         */
      FDK_vhsub_s32_d(D2, D23, D27)         /* Q1:                           pBuf_2[1]   pBuf_2[0]   */
      FDK_vst1_2d_ia(32, D0, D1, r6)        /* store pBuf_3[0..3] with pointer increment ++4         */
      FDK_vhsub_s32_d(D3, D22, D26)         /* Q1:  pBuf_2[3]   pBuf_2[2]    pBuf_2[1]   pBuf_2[0]  <-- ready to store */
      FDK_vst1_2d_pu(32, D2, D3, r5, lr)    /* store pBuf_2[0..3] with pointer decrement --4         */
      FDK_branch(NE, fft_postsort_func1_loop)

    FDK_mpop_pc(r4, r6)
    FDK_return()
FDK_ASM_ROUTINE_END()
#endif /* #ifdef FUNCTION_fft_postsort_func1 */

/* This is the reference function for the subroutine above.   */
/* It provides an 2x unrolled inner loop, no code afterwards. */
#if defined(FUNCTION_fft_postsort)
static void fft_postsort(
        FIXP_DBL * const          buf,
        const INT                 N,
        INT * const               scalefactor
        )
{
    const FIXP_STP *w;
    UINT step;

    *scalefactor += 1;

    getSineTab(N, &w, &step);


#ifdef FUNCTION_fft_postsort_func1
    fft_postsort_func1(buf, N, w, step);
#else
    FIXP_STP w0, w1;
    FIXP_DBL tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    FIXP_DBL sre0, sim0, dre0, dim0;
    FIXP_DBL sre1, sim1, dre1, dim1;
    FIXP_DBL sre2, sim2, dre2, dim2;
    FIXP_DBL sre3, sim3, dre3, dim3;

    INT n;

    FIXP_DBL *pBuf_0 = &buf[0];          /* moves ++4 */
    FIXP_DBL *pBuf_1 = &buf[N-4];        /* moves --4 */
    FIXP_DBL *pBuf_2 = &buf[N/2];        /* moves --4 */
    FIXP_DBL *pBuf_3 = &buf[N/2+2];      /* moves ++4 */

    tmp1 = pBuf_0[0];
    tmp2 = pBuf_0[1];
    pBuf_0[0] = ((tmp1>>1) + (tmp2>>1));
    pBuf_0[1] = ((tmp1>>1) - (tmp2>>1));          /* imag part of first spec value X[0] = 0   */
    pBuf_0 += 2;

    pBuf_2[0] =  (pBuf_2[0] >> 1);
    pBuf_2[1] = -(pBuf_2[1] >> 1);
    pBuf_2 -= 4;

    /* Postsorting algorithm */

    for (n=((N>>4)),w+=step; n!=0; n--)
    {
      w0 = w[0];  w += step;
      w1 = w[0];  w += step;
      sre0 = ((pBuf_0[0]>>1) + (pBuf_1[2]>>1));
      sim0 = ((pBuf_0[1]>>1) + (pBuf_1[3]>>1));
      sre1 = ((pBuf_0[2]>>1) + (pBuf_1[0]>>1));
      sim1 = ((pBuf_0[3]>>1) + (pBuf_1[1]>>1));
      dre0 = ((pBuf_0[0]>>1) - (pBuf_1[2]>>1));
      dim0 = ((pBuf_0[1]>>1) - (pBuf_1[3]>>1));
      dre1 = ((pBuf_0[2]>>1) - (pBuf_1[0]>>1));
      dim1 = ((pBuf_0[3]>>1) - (pBuf_1[1]>>1));

      sre2 = ((pBuf_2[2]>>1) + (pBuf_3[0]>>1));
      sim2 = ((pBuf_2[3]>>1) + (pBuf_3[1]>>1));
      sre3 = ((pBuf_2[0]>>1) + (pBuf_3[2]>>1));
      sim3 = ((pBuf_2[1]>>1) + (pBuf_3[3]>>1));
      dre2 = ((pBuf_2[2]>>1) - (pBuf_3[0]>>1));
      dim2 = ((pBuf_2[3]>>1) - (pBuf_3[1]>>1));
      dre3 = ((pBuf_2[0]>>1) - (pBuf_3[2]>>1));
      dim3 = ((pBuf_2[1]>>1) - (pBuf_3[3]>>1));

      cplxMultDiv2(&tmp1, &tmp2, sim0, dre0, w0);
      cplxMultDiv2(&tmp3, &tmp4, sim1, dre1, w1);
      cplxMultDiv2(&tmp5, &tmp6, dre2, sim2, w0);
      cplxMultDiv2(&tmp7, &tmp8, dre3, sim3, w1);

      pBuf_0[0] =  ((sre0>>1) + tmp1);
      pBuf_0[1] =  ((dim0>>1) - tmp2);
      if (n != 1)
      {
        pBuf_0[2] =  ((sre1>>1) + tmp3);
        pBuf_0[3] =  ((dim1>>1) - tmp4);

        pBuf_1[0] =  ((sre1>>1) - tmp3);
        pBuf_1[1] = -((dim1>>1) + tmp4);
      }
      pBuf_1[2] =  ((sre0>>1) - tmp1);
      pBuf_1[3] = -((dim0>>1) + tmp2);

      pBuf_2[0] =  ((sre3>>1) - tmp7);
      pBuf_2[1] =  ((dim3>>1) - tmp8);
      pBuf_2[2] =  ((sre2>>1) - tmp5);
      pBuf_2[3] =  ((dim2>>1) - tmp6);
      pBuf_3[0] =  ((sre2>>1) + tmp5);
      pBuf_3[1] = -((dim2>>1) + tmp6);
      pBuf_3[2] =  ((sre3>>1) + tmp7);
      pBuf_3[3] = -((dim3>>1) + tmp8);

      pBuf_0 += 4;
      pBuf_1 -= 4;
      pBuf_2 -= 4;
      pBuf_3 += 4;
    }
#endif /* FUNCTION_fft_postsort_func1 */
}
#endif



#ifdef FUNCTION_fft_presort_func1
FDK_ASM_ROUTINE_START(void, fft_presort_func1,
    (FIXP_DBL *      buf,       /* r0  */
     const INT       N,         /* r1  */
     const FIXP_STP *w,         /* r2  */
     INT             step))     /* r3  */

#ifndef __ARM_NEON__
      FIXP_DBL *r0 = buf;
      INT       r1 = N;
      FIXP_STP *r2 = (FIXP_STP *) w;
      INT       r3 = step;
      FIXP_DBL *r4;
      FIXP_DBL *r5;
      FIXP_DBL *r6;
      INT       lr;
#endif

    FDK_mpush_lr(r4, r6)

    FDK_add_op_lsl(r4, r0, r1, 2, 2)        /* r4:          &buf[N]     */
    FDK_sub_imm(r4, r4, 16, 2)              /* r4: pBuf_1 = &buf[N-4]   */
    FDK_asr_imm(r1, r1, 1)                  /* r1: N -> N/2             */
    FDK_add_op_lsl(r2, r2, r3, 2, 2)        /* r2: &w[step]             */
    FDK_lsl_imm(r3, r3, 2)                  /* r3: step -> 4*step       */
    FDK_add_op_lsl(r5, r0, r1, 2, 2)        /* r5: pBuf_2 = &buf[N/2]   */
    FDK_add_imm(r6, r5, 8, 2)               /* r6: pBuf_3 = &buf[N/2+2] */

    FDK_vld1_1d(32, D0, r0)                 /* Q0:                           pBuf_0[1]   pBuf_0[0]   */
    FDK_vld1_1d(32, D1, r5)                 /* Q0:  pBuf_2[1]   pBuf_2[0]    pBuf_0[1]   pBuf_0[0]   */

    FDK_mov_imm(lr, -16)

    /* Create Q15 with integer +/- 1 */
    FDK_vmov_i32(64, D30, 1)                /* Q15:                          0000.0001   0000.0001   */
    FDK_vneg_d(32, D31, D30)                /* Q15: FFFF.FFFF   FFFF.FFFF    0000.0001   0000.0001   */
    FDK_vzip_d(32, D30, D31)                /* Q15: FFFF.FFFF   0000.0001    FFFF.FFFF   0000.0001   */

    FDK_vmul_s32_q(Q1, Q0, Q15)             /* Q1: -pBuf_2[1]   pBuf_2[0]   -pBuf_0[1]   pBuf_0[0]   */
    FDK_vmov_i32(64, D1, 0)                 /* Q0:  0000.0000   0000.0000    pBuf_0[1]   pBuf_0[0]   */
    FDK_vzip_d(32, D1, D3)                  /* Q1: -pBuf_2[1]   0000.0000   -pBuf_0[1]   pBuf_0[0]   */
                                            /* Q0:  pBuf_2[0]   0000.0000    pBuf_0[1]   pBuf_0[0]   */
    FDK_vshr_s32_q_imm(Q0, Q0, 1)           /* Q0:  pBuf_2[0]/2 0000.0000    pBuf_0[1]/2 pBuf_0[0]/2 */
    FDK_vshr_s32_q_imm(Q1, Q1, 1)           /* Q1: -pBuf_2[1]/2 0000.0000   -pBuf_0[1]/2 pBuf_0[0]/2 */

    FDK_vpadd_s32(D0, D0, D2)               /* D0:                           (t1-t2)/2   (t1+t2)/2   */
    FDK_vshr_s32_d_imm(D0, D0, 1)           /* D0:                           (t1-t2)/4   (t1+t2)/4   */
    FDK_vpadd_s32(D1, D1, D3)               /* D1:  -p2[1]/2    p2[0]/2      (t1-t2)/2   (t1+t2)/2   */

    // pBuf_0[0] = ((tmp1>>1) + (tmp2>>1))>>1;
    // pBuf_0[1] = ((tmp1>>1) - (tmp2>>1))>>1;
    // pBuf_0 += 2;
    FDK_vst1_1d_ia(32, D0, r0)              /* D0: store with pointer update (t1-t2)/2   (t1+t2)/2   */

    // pBuf_2[0] =  (pBuf_2[0] >> 1);
    // pBuf_2[1] = -(pBuf_2[1] >> 1);
    // pBuf_2 -= 4;
    FDK_vst1_1d_pu(32, D1, r5, lr)         /* D1: -p2[1]/2    p2[0]/2 store with pointer update: -4 */

    FDK_asr_imm(r1, r1, 3)
FDK_label(fft_presort_func1_loop)
      FDK_vld1_2d(32, D0, D1, r0)           /* Q0:  pBuf_0[3]   pBuf_0[2]    pBuf_0[1]   pBuf_0[0]   */
      FDK_vld1_2d(32, D2, D3, r4)           /* Q1:  pBuf_1[3]   pBuf_1[2]    pBuf_1[1]   pBuf_1[0]   */
      FDK_vld1_2d(32, D4, D5, r5)           /* Q2:  pBuf_2[3]   pBuf_2[2]    pBuf_2[1]   pBuf_2[0]   */
      FDK_vswp(64, D2, D3)                  /* Q1:  pBuf_1[1]   pBuf_1[0]    pBuf_1[3]   pBuf_1[2]   */
      FDK_vld1_2d(32, D6, D7, r6)           /* Q3:  pBuf_3[3]   pBuf_3[2]    pBuf_3[1]   pBuf_3[0]   */
      FDK_vhadd_s32_q(Q8, Q0, Q1)           /* Q8:  sim2        sre2         sim0        sre0        */
      FDK_vhsub_s32_q(Q9, Q0, Q1)           /* Q9:  dim2        dre2         dim0        dre0        */
      FDK_vswp(64, D4, D5)                  /* Q2:  pBuf_2[1]   pBuf_2[0]    pBuf_2[3]   pBuf_2[1]   */
      FDK_vhadd_s32_q(Q10, Q2, Q3)          /* Q10: sim3        sre3         sim1        sre1        */
      FDK_vhsub_s32_q(Q11, Q2, Q3)          /* Q11: dim3        dre3         dim1        dre1        */
      FDK_vld1_pu(32, S8, r2, r3)           /* Q2:                                       w0          */
      FDK_vld1_pu(32, S9, r2, r3)           /* Q2:                           w1          w0          */
      FDK_vshll_s16_imm(Q2, D4, 16)         /* Q2.  w1.im       w1.re        w0.im       w0.re       */
      FDK_vzip_d(32, D4,  D5)               /* Q2.  w1.im       w0.im        w1.re       w0.re       */

      FDK_vuzp_q(32, Q8, Q11)               /* Q8:  dre3        dre1         sre2        sre0        */
                                            /* Q11: dim3        dim1         sim2        sim0        */
      FDK_vuzp_q(32, Q9, Q10)               /* Q9:  sre3        sre1         dre2        dre0        */
                                            /* Q10: sim3        sim1         dim2        dim0        */
      FDK_vswp(64, D19, D21)                /* Q9:  sim3        sim1         dre2        dre0      <-- real input of cplx MPY */
                                            /* Q10: sre3        sre1         dim2        dim0        */

      FDK_vswp(64, D16, D22)                /* Q8:  dre3        dre1         sim2        sim0      <-- imag input of cplx MPY */
                                            /* Q11: dim3        dim1         sre2        sre0        */

      FDK_vqdmulh_s32_qq(Q12, Q9, Q2)       /* Q12: sim3*im1    sim1*im0     dre2*re1    dre0*re0  <-- RE1*im, RE0*re */
      FDK_subs_imm(r1, r1, 1)
      FDK_vqdmulh_s32_qq(Q13, Q8,Q2)        /* Q13: dre3*im1    dre1*im0     sim2*re1    sim0*re0  <-- IM1*im, IM0*re */
      FDK_vswp(64, D4, D5)                  /* Q2:  w1.re       w0.re        w1.im       w0.im       */
      FDK_vqdmulh_s32_qq(Q14, Q9, Q2)       /* Q14: sim3*re1    sim1*re0     dre2*im1    dre0*im0  <-- RE1*re, RE0*im */
      FDK_vqdmulh_s32_qq(Q2,  Q8, Q2)       /* Q2:  dre3*re1    dre1*re0     sim2*im1    sim0*im0  <-- IM1*re, IM0*im */

      FDK_vswp(64, D25, D29)                /* Q12: sim3*re1    sim1*re0     dre2*re1    dre0*re0  <-- RE1*re, RE0*re */
                                            /* Q14: sim3*im1    sim1*im0     dre2*im1    dre0*im0  <-- RE1*im, RE0*im */
      FDK_vswp(64, D4, D26)                 /* Q2:  dre3*re1    dre1*re0     sim2*re1    sim0*re0  <-- IM1*re, IM0*re */
                                            /* Q13: dre3*im1    dre1*im0     sim2*im1    sim0*im0  <-- IM1*im, IM0*im */
      FDK_vsub_s32_q(Q13, Q12, Q13)         /* Q13: tmp8        tmp4         tmp6        tmp2      <-- real output of cplx MPY */
      FDK_vadd_s32_q(Q12, Q14, Q2)          /* Q12: tmp7        tmp3         tmp5        tmp1      <-- imag output of cplx MPY */

      FDK_vzip_q(32, Q12, Q13)              /* Q12: tmp6        tmp5         tmp2        tmp1        */
                                            /* Q13: tmp8        tmp7         tmp4        tmp3        */

      FDK_vswp(64, D21, D23)                /* Q11: sre3        sre1         sre2        sre0        */
                                            /* Q10: dim3        dim1         dim2        dim0        */

      FDK_vzip_q(32, Q11, Q10)              /* Q11: dim2        sre2         dim0        sre0       <-- use for pBuf_0,1 */
                                            /* Q10: dim3        sre3         dim1        sre1       <-- use for pBuf_2,3 */

      FDK_vhadd_s32_q(Q2, Q10, Q13)         /* Q2: -pBuf_2[1]   pBuf_2[0]   -pBuf_2[3]   pBuf_2[2]   */
      FDK_vmul_s32_q(Q2, Q2, Q15)           /* Q2:  pBuf_2[1]   pBuf_2[0]    pBuf_2[3]   pBuf_2[2]   */
      FDK_vhsub_s32_q(Q3, Q10, Q13)         /* Q3:  pBuf_3[3]   pBuf_3[2]    pBuf_3[1]   pBuf_3[0]  <-- ready to store */
      FDK_vmul_s32_q(Q12, Q12, Q15)         /* Q12:-tmp6        tmp5        -tmp2        tmp1        */
      FDK_vswp(64, D4, D5)                  /* Q2:  pBuf_2[3]   pBuf_2[2]    pBuf_2[1]   pBuf_2[0]  <-- ready to store */
      FDK_vst1_2d_ia(32, D6, D7, r6)        /* store pBuf_3[0..3] with pointer increment ++4         */
      FDK_vhadd_s32_q(Q0, Q11, Q12)         /* Q0: -pBuf_0[3]   pBuf_0[2]   -pBuf_0[1]   pBuf_0[0]   */
      FDK_vhsub_s32_q(Q1, Q11, Q12)         /* Q1:  pBuf_1[1]   pBuf_1[0]    pBuf_1[3]   pBuf_1[2]   */
      FDK_vst1_2d_pu(32, D4, D5, r5, lr)    /* store pBuf_2[0..3] with pointer decrement --4         */
      FDK_vmul_s32_q(Q0, Q0, Q15)           /* Q0:  pBuf_0[3]   pBuf_0[2]    pBuf_0[1]   pBuf_0[0]  <-- ready to store */
      FDK_vswp(64, D2, D3)                  /* Q1:  pBuf_1[3]   pBuf_1[2]    pBuf_1[1]   pBuf_1[0]  <-- ready to store */
      FDK_vst1_2d_ia(32, D0, D1, r0)        /* store pBuf_0[0..3] with pointer increment ++4         */
      FDK_vst1_2d_pu(32, D2, D3, r4, lr)    /* store pBuf_1[0..3] with pointer decrement --4         */
      FDK_branch(NE, fft_presort_func1_loop)

    FDK_mpop_pc(r4, r6)
    FDK_return()
FDK_ASM_ROUTINE_END()
#endif /* #ifdef FUNCTION_fft_presort_func1 */

/* This is the reference function for the subroutine above.   */
/* It provides an 2x unrolled inner loop, no code afterwards. */
#if defined(FUNCTION_fft_presort)
static void fft_presort(
        FIXP_DBL * const          buf,
        const INT                 N,
        INT * const               scalefactor
        )
{
    const FIXP_STP *w;
    UINT step;

    *scalefactor += 1;
    getSineTab(N, &w, &step);

#ifdef FUNCTION_fft_presort_func1
    fft_presort_func1(buf, N, w, step);
#else
    INT n;
    FIXP_DBL tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    FIXP_DBL sre0, sim0, dre0, dim0;
    FIXP_DBL sre1, sim1, dre1, dim1;
    FIXP_DBL sre2, sim2, dre2, dim2;
    FIXP_DBL sre3, sim3, dre3, dim3;
    FIXP_STP w0, w1;

    FIXP_DBL *pBuf_0 = &buf[0];
    FIXP_DBL *pBuf_1 = &buf[N-4];
    FIXP_DBL *pBuf_2 = &buf[N/2];
    FIXP_DBL *pBuf_3 = &buf[N/2+2];

    tmp1 = pBuf_0[0];
    tmp2 = pBuf_0[1];
    pBuf_0[0] = ((tmp1>>1) + (tmp2>>1)) >> 1;
    pBuf_0[1] = ((tmp1>>1) - (tmp2>>1)) >> 1;

    tmp3 = pBuf_2[0];
    tmp4 = pBuf_2[1];
    pBuf_2[0] =  tmp3 >> 1;
    pBuf_2[1] = -tmp4 >> 1;

    pBuf_0 += 2;
    pBuf_2 -= 4;

    w += step;

    /* Presorting algorithm */
    for (n=((N>>4)); n!=0; n--)
    {
      w0 = w[0];  w += step;
      w1 = w[0];  w += step;

      sre0 = ((pBuf_0[0]>>1) + (pBuf_1[2]>>1));
      sim0 = ((pBuf_0[1]>>1) + (pBuf_1[3]>>1));
      sre2 = ((pBuf_0[2]>>1) + (pBuf_1[0]>>1));
      sim2 = ((pBuf_0[3]>>1) + (pBuf_1[1]>>1));

      dre0 = ((pBuf_0[0]>>1) - (pBuf_1[2]>>1));
      dim0 = ((pBuf_0[1]>>1) - (pBuf_1[3]>>1));
      dre2 = ((pBuf_0[2]>>1) - (pBuf_1[0]>>1));
      dim2 = ((pBuf_0[3]>>1) - (pBuf_1[1]>>1));

      sre1 = ((pBuf_2[2]>>1) + (pBuf_3[0]>>1));
      sim1 = ((pBuf_2[3]>>1) + (pBuf_3[1]>>1));
      sre3 = ((pBuf_2[0]>>1) + (pBuf_3[2]>>1));
      sim3 = ((pBuf_2[1]>>1) + (pBuf_3[3]>>1));

      dre1 = ((pBuf_2[2]>>1) - (pBuf_3[0]>>1));
      dim1 = ((pBuf_2[3]>>1) - (pBuf_3[1]>>1));
      dre3 = ((pBuf_2[0]>>1) - (pBuf_3[2]>>1));
      dim3 = ((pBuf_2[1]>>1) - (pBuf_3[3]>>1));

      cplxMultDiv2(&tmp2, &tmp1, dre0, sim0, w0);
      cplxMultDiv2(&tmp6, &tmp5, dre2, sim2, w1);
      cplxMultDiv2(&tmp4, &tmp3, sim1, dre1, w0);
      cplxMultDiv2(&tmp8, &tmp7, sim3, dre3, w1);

      pBuf_2[0] =  ((sre3>>1) + tmp7);
      pBuf_2[1] = -((dim3>>1) + tmp8);
      pBuf_2[2] =  ((sre1>>1) + tmp3);
      pBuf_2[3] = -((dim1>>1) + tmp4);
      pBuf_3[0] =  ((sre1>>1) - tmp3);
      pBuf_3[1] =  ((dim1>>1) - tmp4);
      pBuf_3[2] =  ((sre3>>1) - tmp7);
      pBuf_3[3] =  ((dim3>>1) - tmp8);
      pBuf_0[0] =  ((sre0>>1) + tmp1);
      pBuf_0[1] = -((dim0>>1) - tmp2);
      pBuf_0[2] =  ((sre2>>1) + tmp5);
      pBuf_0[3] = -((dim2>>1) - tmp6);
      pBuf_1[0] =  ((sre2>>1) - tmp5);
      pBuf_1[1] =  ((dim2>>1) + tmp6);
      pBuf_1[2] =  ((sre0>>1) - tmp1);
      pBuf_1[3] =  ((dim0>>1) + tmp2);
      pBuf_0 += 4;
      pBuf_1 -= 4;
      pBuf_2 -= 4;
      pBuf_3 += 4;
    }
#endif /* FUNCTION_fft_presort_func1 */
}
#endif /* FUNCTION_fft_presort */
