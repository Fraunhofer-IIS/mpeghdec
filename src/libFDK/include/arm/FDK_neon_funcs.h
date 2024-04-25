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

   Description: Functions replacing NEON operations

*******************************************************************************/

/* clang-format off */
#ifndef FDK_NEON_FUNCS_H
#define FDK_NEON_FUNCS_H

#if !defined(FLOAT)
typedef float FLOAT;
#endif

#ifdef __ARM_NEON__
  #include <arm_neon.h>
  #define HASHSIGN #
  #define PLUSSIGN +
  #define FDK_TEXT(x) x
  #define FIXP_Q int32x4_t

  #define S0        D0[0]
  #define S1        D0[1]
  #define S2        D1[0]
  #define S3        D1[1]

  #define S4        D2[0]
  #define S5        D2[1]
  #define S6        D3[0]
  #define S7        D3[1]

  #define S8        D4[0]
  #define S9        D4[1]
  #define S10       D5[0]
  #define S11       D5[1]

  #define S12       D6[0]
  #define S13       D6[1]
  #define S14       D7[0]
  #define S15       D7[1]

  #define S16       D8[0]
  #define S17       D8[1]
  #define S18       D9[0]
  #define S19       D9[1]

  #define S20       D10[0]
  #define S21       D10[1]
  #define S22       D11[0]
  #define S23       D11[1]

  #define S24       D12[0]
  #define S25       D12[1]
  #define S26       D13[0]
  #define S27       D13[1]

  #define S28       D14[0]
  #define S29       D14[1]
  #define S30       D15[0]
  #define S31       D15[1]

  /* Note: These 32-Bit registers S32..S63 cannot be referred to for encoding */
  /*       They are availbale only for debug purpose (e.g. "watch S36")       */
  #define S32       D16[0]
  #define S33       D16[1]
  #define S34       D17[0]
  #define S35       D17[1]

  #define S36       D18[0]
  #define S37       D18[1]
  #define S38       D19[0]
  #define S39       D19[1]

  #define S40       D20[0]
  #define S41       D20[1]
  #define S42       D21[0]
  #define S43       D21[1]

  #define S44       D22[0]
  #define S45       D22[1]
  #define S46       D23[0]
  #define S47       D23[1]

  #define S48       D24[0]
  #define S49       D24[1]
  #define S50       D25[0]
  #define S51       D25[1]

  #define S52       D26[0]
  #define S53       D26[1]
  #define S54       D27[0]
  #define S55       D27[1]

  #define S56       D28[0]
  #define S57       D28[1]
  #define S58       D29[0]
  #define S59       D29[1]

  #define S60       D30[0]
  #define S61       D30[1]
  #define S62       D31[0]
  #define S63       D31[1]

  #define D0_0      D0[0]
  #define D0_1      D0[1]
  #define D0_2      D0[2]
  #define D0_3      D0[3]

  #define D1_0      D1[0]
  #define D1_1      D1[1]
  #define D1_2      D1[2]
  #define D1_3      D1[3]

  #define D2_0      D2[0]
  #define D2_1      D2[1]
  #define D2_2      D2[2]
  #define D2_3      D2[3]

  #define D3_0      D3[0]
  #define D3_1      D3[1]
  #define D3_2      D3[2]
  #define D3_3      D3[3]

  #define D4_0      D4[0]
  #define D4_1      D4[1]
  #define D4_2      D4[2]
  #define D4_3      D4[3]

  #define D5_0      D5[0]
  #define D5_1      D5[1]
  #define D5_2      D5[2]
  #define D5_3      D5[3]

  #define D6_0      D6[0]
  #define D6_1      D6[1]
  #define D6_2      D6[2]
  #define D6_3      D6[3]

  #define D7_0      D7[0]
  #define D7_1      D7[1]
  #define D7_2      D7[2]
  #define D7_3      D7[3]

  #define D8_0      D8[0]
  #define D8_1      D8[1]
  #define D8_2      D8[2]
  #define D8_3      D8[3]

  #define D9_0      D9[0]
  #define D9_1      D9[1]
  #define D9_2      D9[2]
  #define D9_3      D9[3]

  #define D10_0     D10[0]
  #define D10_1     D10[1]
  #define D10_2     D10[2]
  #define D10_3     D10[3]

  #define D11_0     D11[0]
  #define D11_1     D11[1]
  #define D11_2     D11[2]
  #define D11_3     D11[3]

  #define D12_0     D12[0]
  #define D12_1     D12[1]
  #define D12_2     D12[2]
  #define D12_3     D12[3]

  #define D13_0     D13[0]
  #define D13_1     D13[1]
  #define D13_2     D13[2]
  #define D13_3     D13[3]

  #define D14_0     D14[0]
  #define D14_1     D14[1]
  #define D14_2     D14[2]
  #define D14_3     D14[3]

  #define D15_0     D15[0]
  #define D15_1     D15[1]
  #define D15_2     D15[2]
  #define D15_3     D15[3]

  #define D30_0     D30[0]
  #define D31_0     D31[0]

#else
  #define FIXP_Q  FIXP_DBL *    /*  32x4 bit */
  #define FIXP_H  FIXP_DBL *    /*  32x2 bit */
  #define FIXP_D  FIXP_DBL *    /*    32 bit */
  #define FIXP_S  FIXP_SGL *    /*    16 bit */
  /* help pointer definitions */
  #define FIXP_C  SCHAR *       /*     8 bit */
  #define FIXP_F  float *       /*    32 bit */
  #define FIXP_W  INT64 *       /*    64 bit */
  #define fQAbs32(a) (a == MINVAL_DBL) ? MAXVAL_DBL : fAbs(a);
  #define fQAbs16(a) (a == MINVAL_SGL) ? MAXVAL_SGL : fAbs(a);
  #define fQNeg32(a) (a == MINVAL_DBL) ? MAXVAL_DBL : -a;
  #define fQNeg16(a) (a == MINVAL_SGL) ? MAXVAL_SGL : -a;
  #define MAXVAL_INT64        ((INT64)0x7FFFFFFFFFFFFFFF)
  #define MINVAL_INT64        ((INT64)0x8000000000000000)

/* ARM core flag conditions, set by operations like CMP, SUBS, ADCS, etc.    */
/* Reference: Thumb 16-bit Instruction Set, Quick Refrence Card, www.arm.com */

int __FDK_coreflags_ = 1;       /* No condition, brnach always          */
int __FDK_coreflags_AL = 1;     /* No condition, brnach always          */
int __FDK_coreflags_EQ;         /* equal [to zero]                      */
int __FDK_coreflags_NE;         /* not equal [to zero]                  */
int __FDK_coreflags_CS;         /* carry set / unsigned higher or same  */
int __FDK_coreflags_CC;         /* carry cleared / unsigned lower       */
int __FDK_coreflags_MI;         /* negative                             */
int __FDK_coreflags_PL;         /* positive or zero                     */

/* int __FDK_coreflags_VS; */   /* overflow set                         */
/* int __FDK_coreflags_VC; */   /* overflow cleared                     */
/* int __FDK_coreflags_HI; */   /* unsigned higher                      */
/* int __FDK_coreflags_LS; */   /* unsigned lower or same               */

int __FDK_coreflags_GE;         /* signed greater than or equal         */
int __FDK_coreflags_LT;         /* signed less than                     */
int __FDK_coreflags_GT;         /* signed greater than                  */
int __FDK_coreflags_LE;         /* signed less than or equal            */
#endif

#ifdef __x86__
#undef __GNUC__
#endif

/*#################################################################################*/
/*
   Description: Load a single 16/32-bit lane of a D register from memory (src)
   Parameter size: 16 or 32
   Parameter dst:  Dx[0,1] 64-bit NEON register
   Parameter src:  r core register used as a pointer to 16/32-Bit data
   Parameter step: r core register indicating number of bytes for post-incrementing the dst pointer
*/

/* reference: NEON and VFP programming, chapter 4.8.3: VLDn and VSTn(single n-element structure to one lane)             */
/* reference: NEON and VFP programming, chapter 4.8.4: VLDn(single n-element structure to all lanes) -- not implemented  */
/* reference: NEON and VFP programming, chapter 4.8.5: VLDn and VSTn(multiple n-element structures)                      */
#ifdef __ARM_NEON__

#define __FDK_vld1dup_1d_ia(   size, dst1,                   src)            "VLD1." #size " {" #dst1 "[]}, [" #src "]!  \n\t"
#define __FDK_vld1dup_2d_ia(   size, dst1, dst2,             src)            "VLD1." #size " {" #dst1 "[], " #dst2 "[]}, [" #src "]!  \n\t"

#define __FDK_vld1(            size, dst1,                   src)            "VLD1." #size " {" #dst1 "}, [" #src "] \n\t"
#define __FDK_vld1_ia(         size, dst1,                   src)            "VLD1." #size " {" #dst1 "}, [" #src "]! \n\t"
#define __FDK_vld1_pu(         size, dst1,                   src, Rm)        "VLD1." #size " {" #dst1 "}, [" #src "], " #Rm " \n\t"
#define __FDK_vld1_align(      size, dst1,                   src, align)     "VLD1." #size " {" #dst1 "}, [" #src ",:" #align "] \n\t"
#define __FDK_vld1_ia_align(   size, dst1,                   src, align)     "VLD1." #size " {" #dst1 "}, [" #src ",:" #align "]! \n\t"
#define __FDK_vld1_pu_align(   size, dst1,                   src, align, Rm) "VLD1." #size " {" #dst1 "}, [" #src ",:" #align "], " #Rm " \n\t"

#define __FDK_vld1_1d(         size, dst1,                   src)            "VLD1." #size " {" #dst1 "}, [" #src "] \n\t"
#define __FDK_vld1_1d_ia(      size, dst1,                   src)            "VLD1." #size " {" #dst1 "}, [" #src "]! \n\t"
#define __FDK_vld1_1d_pu(      size, dst1,                   src, Rm)        "VLD1." #size " {" #dst1 "}, [" #src "], " #Rm " \n\t"
#define __FDK_vld1_1d_align(   size, dst1,                   src, align)     "VLD1." #size " {" #dst1 "}, [" #src ",:" #align "] \n\t"
#define __FDK_vld1_1d_ia_align(size, dst1,                   src, align)     "VLD1." #size " {" #dst1 "}, [" #src ",:" #align "]! \n\t"
#define __FDK_vld1_1d_pu_align(size, dst1,                   src, align, Rm) "VLD1." #size " {" #dst1 "}, [" #src ",:" #align "], " #Rm " \n\t"

#define __FDK_vld1_2d(         size, dst1, dst2,             src)            "VLD1." #size " {" #dst1 ", " #dst2 "}, [" #src "] \n\t"
#define __FDK_vld1_2d_ia(      size, dst1, dst2,             src)            "VLD1." #size " {" #dst1 ", " #dst2 "}, [" #src "]! \n\t"
#define __FDK_vld1_2d_pu(      size, dst1, dst2,             src, Rm)        "VLD1." #size " {" #dst1 ", " #dst2 "}, [" #src "], " #Rm " \n\t"
#define __FDK_vld1_2d_align(   size, dst1, dst2,             src, align)     "VLD1." #size " {" #dst1 ", " #dst2 "}, [" #src ",:" #align "] \n\t"
#define __FDK_vld1_2d_ia_align(size, dst1, dst2,             src, align)     "VLD1." #size " {" #dst1 ", " #dst2 "}, [" #src ",:" #align "]! \n\t"
#define __FDK_vld1_2d_pu_align(size, dst1, dst2,             src, align, Rm) "VLD1." #size " {" #dst1 ", " #dst2 "}, [" #src ",:" #align "], " #Rm " \n\t"

#define __FDK_vld1_3d(         size, dst1, dst2, dst3,       src)            "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 "}, [" #src "] \n\t"
#define __FDK_vld1_3d_ia(      size, dst1, dst2, dst3,       src)            "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 "}, [" #src "]! \n\t"
#define __FDK_vld1_3d_pu(      size, dst1, dst2, dst3,       src, Rm)        "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 "}, [" #src "], " #Rm " \n\t"
#define __FDK_vld1_3d_align(   size, dst1, dst2, dst3,       src, align)     "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 "}, [" #src ",:" #align "] \n\t"
#define __FDK_vld1_3d_ia_align(size, dst1, dst2, dst3,       src, align)     "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 "}, [" #src ",:" #align "]! \n\t"
#define __FDK_vld1_3d_pu_align(size, dst1, dst2, dst3,       src, align, Rm) "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 "}, [" #src ",:" #align "], " #Rm " \n\t"

#define __FDK_vld1_4d(         size, dst1, dst2, dst3, dst4, src)            "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 ", " #dst4 "}, [" #src "] \n\t"
#define __FDK_vld1_4d_ia(      size, dst1, dst2, dst3, dst4, src)            "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 ", " #dst4 "}, [" #src "]! \n\t"
#define __FDK_vld1_4d_pu(      size, dst1, dst2, dst3, dst4, src, Rm)        "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 ", " #dst4 "}, [" #src "], " #Rm " \n\t"
#define __FDK_vld1_4d_align(   size, dst1, dst2, dst3, dst4, src, align)     "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 ", " #dst4 "}, [" #src ",:" #align "] \n\t"
#define __FDK_vld1_4d_ia_align(size, dst1, dst2, dst3, dst4, src, align)     "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 ", " #dst4 "}, [" #src ",:" #align "]! \n\t"
#define __FDK_vld1_4d_pu_align(size, dst1, dst2, dst3, dst4, src, align, Rm) "VLD1." #size " {" #dst1 ", " #dst2 ", " #dst3 ", " #dst4 "}, [" #src ",:" #align "], " #Rm " \n\t"

#define FDK_vld1dup_1d_ia(     size, dst1,                   src)             __FDK_vld1dup_1d_ia(   size, dst1, src)
#define FDK_vld1dup_2d_ia(     size, dst1, dst2,             src)             __FDK_vld1dup_2d_ia(   size, dst1, dst2, src)

#define FDK_vld1(              size, dst1,                   src)             __FDK_vld1(            size, dst1, src)
#define FDK_vld1_ia(           size, dst1,                   src)             __FDK_vld1_ia(         size, dst1, src)
#define FDK_vld1_pu(           size, dst1,                   src, Rm)         __FDK_vld1_pu(         size, dst1, src, Rm)
#define FDK_vld1_align(        size, dst1,                   src, align)      __FDK_vld1_align(      size, dst1, src, align)
#define FDK_vld1_ia_align(     size, dst1,                   src, align)      __FDK_vld1_ia_align(   size, dst1, src, align)
#define FDK_vld1_pu_align(     size, dst1,                   src, align, Rm)  __FDK_vld1_pu_align(   size, dst1, src, align, Rm)

#define FDK_vld1_1d(           size, dst1,                   src)             __FDK_vld1_1d(         size, dst1, src)
#define FDK_vld1_1d_ia(        size, dst1,                   src)             __FDK_vld1_1d_ia(      size, dst1, src)
#define FDK_vld1_1d_pu(        size, dst1,                   src, Rm)         __FDK_vld1_1d_pu(      size, dst1, src, Rm)
#define FDK_vld1_1d_align(     size, dst1,                   src, align)      __FDK_vld1_1d_align(   size, dst1, src, align)
#define FDK_vld1_1d_ia_align(  size, dst1,                   src, align)      __FDK_vld1_1d_ia_align(size, dst1, src, align)
#define FDK_vld1_1d_pu_align(  size, dst1,                   src, align, Rm)  __FDK_vld1_1d_pu_align(size, dst1, src, align, Rm)

#define FDK_vld1_2d(           size, dst1, dst2,             src)             __FDK_vld1_2d(         size, dst1, dst2, src)
#define FDK_vld1_2d_ia(        size, dst1, dst2,             src)             __FDK_vld1_2d_ia(      size, dst1, dst2, src)
#define FDK_vld1_2d_pu(        size, dst1, dst2,             src, Rm)         __FDK_vld1_2d_pu(      size, dst1, dst2, src, Rm)
#define FDK_vld1_2d_align(     size, dst1, dst2,             src, align)      __FDK_vld1_2d_align(   size, dst1, dst2, src, align)
#define FDK_vld1_2d_ia_align(  size, dst1, dst2,             src, align)      __FDK_vld1_2d_ia_align(size, dst1, dst2, src, align)
#define FDK_vld1_2d_pu_align(  size, dst1, dst2,             src, align, Rm)  __FDK_vld1_2d_pu_align(size, dst1, dst2, src, align, Rm)

#define FDK_vld1_3d(           size, dst1, dst2, dst3,       src)             __FDK_vld1_3d(         size, dst1, dst2, dst3, src)
#define FDK_vld1_3d_ia(        size, dst1, dst2, dst3,       src)             __FDK_vld1_3d_ia(      size, dst1, dst2, dst3, src)
#define FDK_vld1_3d_pu(        size, dst1, dst2, dst3,       src, Rm)         __FDK_vld1_3d_pu(      size, dst1, dst2, dst3, src, Rm)
#define FDK_vld1_3d_align(     size, dst1, dst2, dst3,       src, align)      __FDK_vld1_3d_align(   size, dst1, dst2, dst3, src, align)
#define FDK_vld1_3d_ia_align(  size, dst1, dst2, dst3,       src, align)      __FDK_vld1_3d_ia_align(size, dst1, dst2, dst3, src, align)
#define FDK_vld1_3d_pu_align(  size, dst1, dst2, dst3,       src, align, Rm)  __FDK_vld1_3d_pu_align(size, dst1, dst2, dst3, src, align, Rm)

#define FDK_vld1_4d(           size, dst1, dst2, dst3, dst4, src)             __FDK_vld1_4d(         size, dst1, dst2, dst3, dst4, src)
#define FDK_vld1_4d_ia(        size, dst1, dst2, dst3, dst4, src)             __FDK_vld1_4d_ia(      size, dst1, dst2, dst3, dst4, src)
#define FDK_vld1_4d_pu(        size, dst1, dst2, dst3, dst4, src, Rm)         __FDK_vld1_4d_pu(      size, dst1, dst2, dst3, dst4, src, Rm)
#define FDK_vld1_4d_align(     size, dst1, dst2, dst3, dst4, src, align)      __FDK_vld1_4d_align(   size, dst1, dst2, dst3, dst4, src, align)
#define FDK_vld1_4d_ia_align(  size, dst1, dst2, dst3, dst4, src, align)      __FDK_vld1_4d_ia_align(size, dst1, dst2, dst3, dst4, src, align)
#define FDK_vld1_4d_pu_align(  size, dst1, dst2, dst3, dst4, src, align, Rm)  __FDK_vld1_4d_pu_align(size, dst1, dst2, dst3, dst4, src, align, Rm)
#else
/* reference: NEON and VFP programming, chapter 4.8.3 */
#define FDK_vld1(   size, dst1, src)          { if (size == 32) { FDK_vld1_32((FIXP_D)dst1,(FIXP_D) src); }   else if (size == 16) { FDK_vld1_16((FIXP_S) dst1,(FIXP_S) src); } }
#define FDK_vld1_ia(size, dst1, src)          { if (size == 32) { FDK_vld1_32((FIXP_D)dst1,(FIXP_D) src);   src += (size>>3)/(INT)sizeof(*((FIXP_D)src)); }   \
                                           else if (size == 16) { FDK_vld1_16((FIXP_S)dst1,(FIXP_S) src);   src += (size>>3)/(INT)sizeof(*((FIXP_S)src)); } }
#define FDK_vld1_pu(size, dst1, src, step)    { if (size == 32) { FDK_vld1_32((FIXP_D)dst1,(FIXP_D) src); }   else if (size == 16) { FDK_vld1_16((FIXP_S) dst1,(FIXP_S) src); }   src += step/(INT)sizeof(*src); }
#define FDK_vld1_align(   size, dst1, src, align)       FDK_vld1(   size, dst1, src)
#define FDK_vld1_ia_align(size, dst1, src, align)       FDK_vld1_ia(size, dst1, src)
#define FDK_vld1_pu_align(size, dst1, src, align, step) FDK_vld1_pu(size, dst1, src, step)

/* reference: NEON and VFP programming, chapter 4.8.4 */
/* --- not yet supported ---                          */

/* reference: NEON and VFP programming, chapter 4.8.5 */
#define FDK_vld1_1d(   size, dst1, src)                         { if (size == 32) { FDK_vld1_1d_32((FIXP_D)dst1,(FIXP_D) src); }                                        else if (size == 16) { FDK_vld1_1d_16((FIXP_S)dst1,(FIXP_S) src); } }
#define FDK_vld1_1d_ia(size, dst1, src)                         { if (size == 32) { FDK_vld1_1d_32((FIXP_D)dst1,(FIXP_D) src); }                                        else if (size == 16) { FDK_vld1_1d_16((FIXP_S)dst1,(FIXP_S) src); } src += (INT)8/(INT)sizeof(*src);  }
#define FDK_vld1_1d_pu(size, dst1, src, step)                   { if (size == 32) { FDK_vld1_1d_32((FIXP_D)dst1,(FIXP_D) src); }                                        else if (size == 16) { FDK_vld1_1d_16((FIXP_S)dst1,(FIXP_S) src); } src += step/(INT)sizeof(*src); }
#define FDK_vld1_1d_align(   size, dst1, src, align)       FDK_vld1_1d(   size, dst1, src)
#define FDK_vld1_1d_ia_align(size, dst1, src, align)       FDK_vld1_1d_ia(size, dst1, src)
#define FDK_vld1_1d_pu_align(size, dst1, src, align, step) FDK_vld1_1d_pu(size, dst1, src, step)

#define FDK_vld1_2d(   size, dst1, dst2, src)                   { if (size == 64) { FDK_vld1_2d_64((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_H) src); }                           else if (size == 32) { FDK_vld1_2d_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D) src); }                           else if (size == 16) { FDK_vld1_2d_16((FIXP_S)dst1,(FIXP_S)dst2,(FIXP_S) src); } }
#define FDK_vld1_2d_ia(size, dst1, dst2, src)                   { if (size == 64) { FDK_vld1_2d_64((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_H) src); }                           else if (size == 32) { FDK_vld1_2d_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D) src); }                           else if (size == 16) { FDK_vld1_2d_16((FIXP_S)dst1,(FIXP_S)dst2,(FIXP_S) src); } src += (INT)16/(INT)sizeof(*src);  }
#define FDK_vld1_2d_pu(size, dst1, dst2, src, step)             { if (size == 64) { FDK_vld1_2d_64((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_H) src); }                           else if (size == 32) { FDK_vld1_2d_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D) src); }                           else if (size == 16) { FDK_vld1_2d_16((FIXP_S)dst1,(FIXP_S)dst2,(FIXP_S) src); } src += step/(INT)sizeof(*src); }
#define FDK_vld1_2d_align(   size, dst1, dst2, src, align)       FDK_vld1_2d(   size, dst1, dst2, src)
#define FDK_vld1_2d_ia_align(size, dst1, dst2, src, align)       FDK_vld1_2d_ia(size, dst1, dst2, src)
#define FDK_vld1_2d_pu_align(size, dst1, dst2, src, align, step) FDK_vld1_2d_pu(size, dst1, dst2, src, step)

#define FDK_vld1_3d(   size, dst1, dst2, dst3, src)             { if (size == 64) { FDK_vld1_3d_64((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_H)dst3,(FIXP_H) src); }              else if (size == 32) { FDK_vld1_3d_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D) src); }              else if (size == 16) { FDK_vld1_3d_16((FIXP_S)dst1,(FIXP_S)dst2,(FIXP_S)dst3,(FIXP_S) src); } }
#define FDK_vld1_3d_ia(size, dst1, dst2, dst3, src)             { if (size == 64) { FDK_vld1_3d_64((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_H)dst3,(FIXP_H) src); }              else if (size == 32) { FDK_vld1_3d_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D) src); }         else if (size == 16) { FDK_vld1_3d_16((FIXP_S)dst1,(FIXP_S)dst2,(FIXP_S)dst3,(FIXP_S) src); } src += (INT)24/(INT)sizeof(*src);  }
#define FDK_vld1_3d_pu(size, dst1, dst2, dst3, src, step)       { if (size == 64) { FDK_vld1_3d_64((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_H)dst3,(FIXP_H) src); }              else if (size == 32) { FDK_vld1_3d_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D) src); }         else if (size == 16) { FDK_vld1_3d_16((FIXP_S)dst1,(FIXP_S)dst2,(FIXP_S)dst3,(FIXP_S) src); } src += step/(INT)sizeof(*src); }
#define FDK_vld1_3d_align(   size, dst1, dst2, dst3, src, align)       FDK_vld1_3d(   size, dst1, dst2, dst3, src)
#define FDK_vld1_3d_ia_align(size, dst1, dst2, dst3, src, align)       FDK_vld1_3d_ia(size, dst1, dst2, dst3, src)
#define FDK_vld1_3d_pu_align(size, dst1, dst2, dst3, src, align, step) FDK_vld1_3d_pu(size, dst1, dst2, dst3, src, step)

#define FDK_vld1_4d(   size, dst1, dst2, dst3, dst4, src)       { if (size == 32) { FDK_vld1_4d_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D)dst4,(FIXP_D) src); } else if (size == 16) { FDK_vld1_4d_16((FIXP_S)dst1,(FIXP_S)dst2,(FIXP_S)dst3,(FIXP_S)dst4,(FIXP_S) src); } }
#define FDK_vld1_4d_ia(size, dst1, dst2, dst3, dst4, src)       { if (size == 32) { FDK_vld1_4d_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D)dst4,(FIXP_D) src); } else if (size == 16) { FDK_vld1_4d_16((FIXP_S)dst1,(FIXP_S)dst2,(FIXP_S)dst3,(FIXP_S)dst4,(FIXP_S) src); } src += (INT)32/(INT)sizeof(*src);  }
#define FDK_vld1_4d_pu(size, dst1, dst2, dst3, dst4, src, step) { if (size == 32) { FDK_vld1_4d_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D)dst4,(FIXP_D) src); } else if (size == 16) { FDK_vld1_4d_16((FIXP_S)dst1,(FIXP_S)dst2,(FIXP_S)dst3,(FIXP_S)dst4,(FIXP_S) src); } src += step/(INT)sizeof(*src); }
#define FDK_vld1_4d_align(   size, dst1, dst2, dst3, dst4, src, align)       FDK_vld1_4d(   size, dst1, dst2, dst3, dst4, src)
#define FDK_vld1_4d_ia_align(size, dst1, dst2, dst3, dst4, src, align)       FDK_vld1_4d_ia(size, dst1, dst2, dst3, dst4, src)
#define FDK_vld1_4d_pu_align(size, dst1, dst2, dst3, dst4, src, align, step) FDK_vld1_4d_pu(size, dst1, dst2, dst3, dst4, src, step)

#define FDK_vld1dup_1d_ia(size, dst, src) {FDK_vld1dup_1d_ia_32((FIXP_D) dst, (FIXP_D) src); src = src + 1;}
#define FDK_vld1dup_2d_ia(size, dst1, dst2, src) {FDK_vld1dup_2d_ia_32((FIXP_D) dst1, (FIXP_D) dst2, (FIXP_D) src); src = src + 1;}

static void inline FDK_vld1dup_1d_ia_32(FIXP_D dst1, FIXP_D src)
{
  FIXP_DBL imm = (FIXP_DBL) src[0];
  for (int i = 0; i < 2; i++)
  {
    dst1[i] = imm;
  }
}

static void inline FDK_vld1dup_2d_ia_32(FIXP_D dst1, FIXP_D dst2, FIXP_D src)
{
  FIXP_DBL imm = (FIXP_DBL) src[0];
  for (int i = 0; i < 2; i++)
  {
    dst1[i] = imm;
    dst2[i] = imm;
  }
}

/* Load all lanes: refer to NEON and VFP Programming chapter 4.8.5 */
static void inline FDK_vld1_1d_16(FIXP_S dst1, FIXP_S src)
{
  for (int i = 0; i < 4; i++)
  {
    dst1[i] = src[i];
  }
}
static void inline FDK_vld1_2d_16(FIXP_S dst1, FIXP_S dst2, FIXP_S src)
{
  FDK_ASSERT(dst2 == &dst1[4]);
  for (int i = 0; i < 4; i++)
  {
    dst1[i] = src[i];
  }
  for (int i = 0; i < 4; i++)
  {
    dst2[i] = src[i+4];
  }
}
static void inline FDK_vld1_3d_16(FIXP_S dst1, FIXP_S dst2, FIXP_S dst3, FIXP_S src)
{
  FDK_ASSERT(dst2 == &dst1[4]);
  FDK_ASSERT(dst3 == &dst1[8]);
  for (int i = 0; i < 4; i++)
  {
    dst1[i] = src[i];
  }
  for (int i = 0; i < 4; i++)
  {
    dst2[i] = src[i+4];
  }
  for (int i = 0; i < 4; i++)
  {
    dst3[i] = src[i+8];
  }
}
static void inline FDK_vld1_4d_16(FIXP_S dst1, FIXP_S dst2, FIXP_S dst3, FIXP_S dst4, FIXP_S src)
{
  FDK_ASSERT(dst2 == &dst1[4]);
  FDK_ASSERT(dst3 == &dst1[8]);
  FDK_ASSERT(dst4 == &dst1[12]);
  for (int i = 0; i < 4; i++)
  {
    dst1[i] = src[i];
  }
  for (int i = 0; i < 4; i++)
  {
    dst2[i] = src[i+4];
  }
  for (int i = 0; i < 4; i++)
  {
    dst3[i] = src[i+8];
  }
  for (int i = 0; i < 4; i++)
  {
    dst4[i] = src[i+12];
  }
}
static void inline FDK_vld1_1d_32(FIXP_D dst1, FIXP_D src)
{
  for (int i = 0; i < 2; i++)
  {
    dst1[i] = src[i];
  }
}
static void inline FDK_vld1_2d_32(FIXP_D dst1, FIXP_D dst2, FIXP_D src)
{
  FDK_ASSERT(dst2 == &dst1[2]);
  for (int i = 0; i < 2; i++)
  {
    dst1[i] = src[i];
  }
  for (int i = 0; i < 2; i++)
  {
    dst2[i] = src[i+2];
  }
}
static void inline FDK_vld1_3d_32(FIXP_D dst1, FIXP_D dst2, FIXP_D dst3, FIXP_D src)
{
  FDK_ASSERT(dst2 == &dst1[2]);
  FDK_ASSERT(dst3 == &dst1[4]);
  for (int i = 0; i < 2; i++)
  {
    dst1[i] = src[i];
  }
  for (int i = 0; i < 2; i++)
  {
    dst2[i] = src[i+2];
  }
  for (int i = 0; i < 2; i++)
  {
    dst3[i] = src[i+4];
  }
}
static void inline FDK_vld1_4d_32(FIXP_D dst1, FIXP_D dst2, FIXP_D dst3, FIXP_D dst4, FIXP_D src)
{
  FDK_ASSERT(dst2 == &dst1[2]);
  FDK_ASSERT(dst3 == &dst1[4]);
  FDK_ASSERT(dst4 == &dst1[6]);
  for (int i = 0; i < 2; i++)
  {
    dst1[i] = src[i];
  }
  for (int i = 0; i < 2; i++)
  {
    dst2[i] = src[i+2];
  }
  for (int i = 0; i < 2; i++)
  {
    dst3[i] = src[i+4];
  }
  for (int i = 0; i < 2; i++)
  {
    dst4[i] = src[i+6];
  }
}
static void inline FDK_vld1_2d_64(FIXP_H dst1, FIXP_H dst2, FIXP_H src)
{
  FDK_ASSERT(dst2 == &dst1[2]);
  for (int i = 0; i < 2; i++)
  {
    dst1[i] = src[i];
  }
  for (int i = 0; i < 2; i++)
  {
    dst2[i] = src[i+2];
  }
}

static void inline FDK_vld1_3d_64(FIXP_H dst1, FIXP_H dst2, FIXP_H dst3, FIXP_H src)
{
  FDK_ASSERT(dst2 == &dst1[2]);
  FDK_ASSERT(dst3 == &dst1[4]);
  for (int i = 0; i < 2; i++)
  {
    dst1[i] = src[i];
  }
  for (int i = 0; i < 2; i++)
  {
    dst2[i] = src[i+2];
  }
  for (int i = 0; i < 2; i++)
  {
    dst3[i] = src[i+4];
  }
}


/* Load single lanes: refer to NEON and VFP Programming chapter 4.8.3 */
static void inline FDK_vld1_32(FIXP_D dst, FIXP_D src)
{
  dst[0] = src[0];
}

static void inline FDK_vld1_16(FIXP_S dst, FIXP_S src)
{
  dst[0] = src[0];
}
#endif

/*
   Description: Store a single 16/32-bit lane of a D register into memory (dst)
   Parameter size: 16 or 32
   Parameter dst:  r core register used as a pointer to 16/32-Bit data
   Parameter src:  Dx[0,1] 64-bit NEON register
   Parameter step: r core register indicating number of bytes for post-incrementing the dst pointer
*/

#ifdef __ARM_NEON__
#define __FDK_vst1(            size, src1, dst)                              "VST1." #size " {" #src1 "}, [" #dst "] \n\t"
#define __FDK_vst1_ia(         size, src1, dst)                              "VST1." #size " {" #src1 "}, [" #dst "]! \n\t"
#define __FDK_vst1_pu(         size, src1, dst, Rm)                          "VST1." #size " {" #src1 "}, [" #dst "], " #Rm " \n\t"
#define __FDK_vst1_align(      size, src1, dst, align)                       "VST1." #size " {" #src1 "}, [" #dst ",:" #align "] \n\t"
#define __FDK_vst1_ia_align(   size, src1, dst, align)                       "VST1." #size " {" #src1 "}, [" #dst ",:" #align "]! \n\t"
#define __FDK_vst1_pu_align(   size, src1, dst, align, Rm)                   "VST1." #size " {" #src1 "}, [" #dst ",:" #align "], " #Rm " \n\t"
#define __FDK_vst1_1d(         size, src1, dst)                              "VST1." #size " {" #src1 "}, [" #dst "] \n\t"
#define __FDK_vst1_1d_ia(      size, src1, dst)                              "VST1." #size " {" #src1 "}, [" #dst "]! \n\t"
#define __FDK_vst1_1d_pu(      size, src1, dst, Rm)                          "VST1." #size " {" #src1 "}, [" #dst "], " #Rm " \n\t"
#define __FDK_vst1_1d_align(   size, src1, dst, align)                       "VST1." #size " {" #src1 "}, [" #dst ",:" #align "] \n\t"
#define __FDK_vst1_1d_ia_align(size, src1, dst, align)                       "VST1." #size " {" #src1 "}, [" #dst ",:" #align "]! \n\t"
#define __FDK_vst1_1d_pu_align(size, src1, dst, align, Rm)                   "VST1." #size " {" #src1 "}, [" #dst ",:" #align "], " #Rm " \n\t"
#define __FDK_vst1_2d(         size, src1, src2, dst)                        "VST1." #size " {" #src1 "," #src2 "}, [" #dst "] \n\t"
#define __FDK_vst1_2d_ia(      size, src1, src2, dst)                        "VST1." #size " {" #src1 "," #src2 "}, [" #dst "]! \n\t"
#define __FDK_vst1_2d_pu(      size, src1, src2, dst, Rm)                    "VST1." #size " {" #src1 "," #src2 "}, [" #dst "], " #Rm " \n\t"
#define __FDK_vst1_2d_align(   size, src1, src2, dst, align)                 "VST1." #size " {" #src1 "," #src2 "}, [" #dst ",:" #align "] \n\t"
#define __FDK_vst1_2d_ia_align(size, src1, src2, dst, align)                 "VST1." #size " {" #src1 "," #src2 "}, [" #dst ",:" #align "]! \n\t"
#define __FDK_vst1_2d_pu_align(size, src1, src2, dst, align, Rm)             "VST1." #size " {" #src1 "," #src2 "}, [" #dst ",:" #align "], " #Rm " \n\t"
#define __FDK_vst1_3d(         size, src1, src2, src3, dst)                  "VST1." #size " {" #src1 "," #src2 "," #src3 "}, [" #dst "] \n\t"
#define __FDK_vst1_3d_ia(      size, src1, src2, src3, dst)                  "VST1." #size " {" #src1 "," #src2 "," #src3 "}, [" #dst "]! \n\t"
#define __FDK_vst1_3d_pu(      size, src1, src2, src3, dst, Rm)              "VST1." #size " {" #src1 "," #src2 "," #src3 "}, [" #dst "], " #Rm " \n\t"
#define __FDK_vst1_3d_align(   size, src1, src2, src3, dst, align)           "VST1." #size " {" #src1 "," #src2 "," #src3 "}, [" #dst ",:" #align "] \n\t"
#define __FDK_vst1_3d_ia_align(size, src1, src2, src3, dst, align)           "VST1." #size " {" #src1 "," #src2 "," #src3 "}, [" #dst ",:" #align "]! \n\t"
#define __FDK_vst1_3d_pu_align(size, src1, src2, src3, dst, align, Rm)       "VST1." #size " {" #src1 "," #src2 "," #src3 "}, [" #dst ",:" #align "], " #Rm " \n\t"
#define __FDK_vst1_4d(         size, src1, src2, src3, src4, dst)            "VST1." #size " {" #src1 "," #src2 "," #src3 "," #src4 "}, [" #dst "] \n\t"
#define __FDK_vst1_4d_ia(      size, src1, src2, src3, src4, dst)            "VST1." #size " {" #src1 "," #src2 "," #src3 "," #src4 "}, [" #dst "]! \n\t"
#define __FDK_vst1_4d_pu(      size, src1, src2, src3, src4, dst, Rm)        "VST1." #size " {" #src1 "," #src2 "," #src3 "," #src4 "}, [" #dst "], " #Rm " \n\t"
#define __FDK_vst1_4d_align(   size, src1, src2, src3, src4, dst, align)     "VST1." #size " {" #src1 "," #src2 "," #src3 "," #src4 "}, [" #dst ",:" #align "] \n\t"
#define __FDK_vst1_4d_ia_align(size, src1, src2, src3, src4, dst, align)     "VST1." #size " {" #src1 "," #src2 "," #src3 "," #src4 "}, [" #dst ",:" #align "]! \n\t"
#define __FDK_vst1_4d_pu_align(size, src1, src2, src3, src4, dst, align, Rm) "VST1." #size " {" #src1 "," #src2 "," #src3 "," #src4 "}, [" #dst ",:" #align "], " #Rm " \n\t"

#define FDK_vst1(            size, src1, dst)                               __FDK_vst1(            size, src1, dst)
#define FDK_vst1_ia(         size, src1, dst)                               __FDK_vst1_ia(         size, src1, dst)
#define FDK_vst1_pu(         size, src1, dst, Rm)                           __FDK_vst1_pu(         size, src1, dst, Rm)
#define FDK_vst1_align(      size, src1, dst, align)                        __FDK_vst1_align(      size, src1, dst, align)
#define FDK_vst1_ia_align(   size, src1, dst, align)                        __FDK_vst1_ia_align(   size, src1, dst, align)
#define FDK_vst1_pu_align(   size, src1, dst, align, Rm)                    __FDK_vst1_pu_align(   size, src1, dst, align, Rm)
#define FDK_vst1_1d(         size, src1, dst)                               __FDK_vst1_1d(         size, src1, dst)
#define FDK_vst1_1d_ia(      size, src1, dst)                               __FDK_vst1_1d_ia(      size, src1, dst)
#define FDK_vst1_1d_pu(      size, src1, dst, Rm)                           __FDK_vst1_1d_pu(      size, src1, dst, Rm)
#define FDK_vst1_1d_align(   size, src1, dst, align)                        __FDK_vst1_1d_align    size, src1, dst, align)
#define FDK_vst1_1d_ia_align(size, src1, dst, align)                        __FDK_vst1_1d_ia_align(size, src1, dst, align)
#define FDK_vst1_1d_pu_align(size, src1, dst, align, Rm)                    __FDK_vst1_1d_pu_align(size, src1, dst, align, Rm)
#define FDK_vst1_2d(         size, src1, src2, dst)                         __FDK_vst1_2d(         size, src1, src2, dst)
#define FDK_vst1_2d_ia(      size, src1, src2, dst)                         __FDK_vst1_2d_ia(      size, src1, src2, dst)
#define FDK_vst1_2d_pu(      size, src1, src2, dst, Rm)                     __FDK_vst1_2d_pu(      size, src1, src2, dst, Rm)
#define FDK_vst1_2d_align(   size, src1, src2, dst, align)                  __FDK_vst1_2d_align(   size, src1, src2, dst, align)
#define FDK_vst1_2d_ia_align(size, src1, src2, dst, align)                  __FDK_vst1_2d_ia_align(size, src1, src2, dst, align)
#define FDK_vst1_2d_pu_align(size, src1, src2, dst, align, Rm)              __FDK_vst1_2d_pu_align(size, src1, src2, dst, align, Rm)
#define FDK_vst1_3d(         size, src1, src2, src3, dst)                   __FDK_vst1_3d(         size, src1, src2, src3, dst)
#define FDK_vst1_3d_ia(      size, src1, src2, src3, dst)                   __FDK_vst1_3d_ia(      size, src1, src2, src3, dst)
#define FDK_vst1_3d_pu(      size, src1, src2, src3, dst, Rm)               __FDK_vst1_3d_pu(      size, src1, src2, src3, dst, Rm)
#define FDK_vst1_3d_align(   size, src1, src2, src3, dst, align)            __FDK_vst1_3d_align(   size, src1, src2, src3, dst, align)
#define FDK_vst1_3d_ia_align(size, src1, src2, src3, dst, align)            __FDK_vst1_3d_ia_align(size, src1, src2, src3, dst, align)
#define FDK_vst1_3d_pu_align(size, src1, src2, src3, dst, align, Rm)        __FDK_vst1_3d_pu_align(size, src1, src2, src3, dst, align, Rm)
#define FDK_vst1_4d(         size, src1, src2, src3, src4, dst)             __FDK_vst1_4d(         size, src1, src2, src3, src4, dst)
#define FDK_vst1_4d_ia(      size, src1, src2, src3, src4, dst)             __FDK_vst1_4d_ia(      size, src1, src2, src3, src4, dst)
#define FDK_vst1_4d_pu(      size, src1, src2, src3, src4, dst, Rm)         __FDK_vst1_4d_pu(      size, src1, src2, src3, src4, dst, Rm)
#define FDK_vst1_4d_align(   size, src1, src2, src3, src4, dst, align)      __FDK_vst1_4d_align(   size, src1, src2, src3, src4, dst, align)
#define FDK_vst1_4d_ia_align(size, src1, src2, src3, src4, dst, align)      __FDK_vst1_4d_ia_align(size, src1, src2, src3, src4, dst, align)
#define FDK_vst1_4d_pu_align(size, src1, src2, src3, src4, dst, align, Rm)  __FDK_vst1_4d_pu_align(size, src1, src2, src3, src4, dst, align, Rm)
#else
/* reference: NEON and VFP programming, chapter 4.8.3 */
#define FDK_vst1(   size, src, dst)         { if (size == 32) FDK_vst1_32((FIXP_D)src,(FIXP_D)dst); else if (size == 16) FDK_vst1_16((FIXP_S)src,(FIXP_S)dst); }
#define FDK_vst1_ia(size, src, dst)         { if (size == 32) FDK_vst1_32((FIXP_D)src,(FIXP_D)dst); else if (size == 16) FDK_vst1_16((FIXP_S)src,(FIXP_S)dst);   dst += (size>>3)/(INT)sizeof(*dst); }
#define FDK_vst1_pu(size, src, dst, step)   { if (size == 32) FDK_vst1_32((FIXP_D)src,(FIXP_D)dst); else if (size == 16) FDK_vst1_16((FIXP_S) src,(FIXP_S) dst); dst += step/(INT)sizeof(*dst); }
#define FDK_vst1_align(   size, src, dst, align)         FDK_vst1(   size, src, dst)
#define FDK_vst1_ia_align(size, src, dst, align)         FDK_vst1_ia(size, src, dst)
#define FDK_vst1_pu_align(size, src, dst, align, step)   FDK_vst1_pu(size, src, dst, step)

/* reference: NEON and VFP programming, chapter 4.8.4 */
/* --- not yet supported ---                          */

/* reference: NEON and VFP programming, chapter 4.8.5 */
#define FDK_vst1_1d(   size, src1, dst)                         { if (size == 64) { FDK_vst1_1d_64((FIXP_D)src1,(FIXP_D) dst); }                                        else if (size == 32) { FDK_vst1_1d_64((FIXP_D)src1,(FIXP_D) dst); }                                        else if (size == 16) { FDK_vst1_1d_16((FIXP_S)src1,(FIXP_S) dst); } }
#define FDK_vst1_1d_ia(size, src1, dst)                         { if (size == 64) { FDK_vst1_1d_64((FIXP_D)src1,(FIXP_D) dst); }                                        else if (size == 32) { FDK_vst1_1d_64((FIXP_D)src1,(FIXP_D) dst); }                                        else if (size == 16) { FDK_vst1_1d_16((FIXP_S)src1,(FIXP_S) dst); } dst += (INT)8/(INT)sizeof(*dst);  }
#define FDK_vst1_1d_pu(size, src1, dst, step)                   { if (size == 64) { FDK_vst1_1d_64((FIXP_D)src1,(FIXP_D) dst); }                                        else if (size == 32) { FDK_vst1_1d_32((FIXP_D)src1,(FIXP_D) dst); }                                        else if (size == 16) { FDK_vst1_1d_16((FIXP_S)src1,(FIXP_S) dst); } dst += step/(INT)sizeof(*dst); }
#define FDK_vst1_2d(   size, src1, src2, dst)                   { if (size == 64) { FDK_vst1_2d_64((FIXP_D)src1,(FIXP_D)src2,(FIXP_D) dst); }                           else if (size == 32) { FDK_vst1_2d_32((FIXP_D)src1,(FIXP_D)src2,(FIXP_D) dst); }                           else if (size == 16) { FDK_vst1_2d_16((FIXP_S)src1,(FIXP_S)src2,(FIXP_S) dst); } }
#define FDK_vst1_2d_ia(size, src1, src2, dst)                   { if (size == 64) { FDK_vst1_2d_64((FIXP_D)src1,(FIXP_D)src2,(FIXP_D) dst); }                           else if (size == 32) { FDK_vst1_2d_32((FIXP_D)src1,(FIXP_D)src2,(FIXP_D) dst); }                           else if (size == 16) { FDK_vst1_2d_16((FIXP_S)src1,(FIXP_S)src2,(FIXP_S) dst); } dst += (INT)16/(INT)sizeof(*dst);  }
#define FDK_vst1_2d_pu(size, src1, src2, dst, step)             { if (size == 64) { FDK_vst1_2d_64((FIXP_D)src1,(FIXP_D)src2,(FIXP_D) dst); }                           else if (size == 32) { FDK_vst1_2d_32((FIXP_D)src1,(FIXP_D)src2,(FIXP_D) dst); }                           else if (size == 16) { FDK_vst1_2d_16((FIXP_S)src1,(FIXP_S)src2,(FIXP_S) dst); } dst += step/(INT)sizeof(*dst); }
#define FDK_vst1_3d(   size, src1, src2, src3, dst)             { if (size == 64) { FDK_vst1_3d_64((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D) dst); }              else if (size == 32) { FDK_vst1_3d_32((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D) dst); }              else if (size == 16) { FDK_vst1_3d_16((FIXP_S)src1,(FIXP_S)src2,(FIXP_S)src3,(FIXP_S) dst); } }
#define FDK_vst1_3d_ia(size, src1, src2, src3, dst)             { if (size == 64) { FDK_vst1_3d_64((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D) dst); }              else if (size == 32) { FDK_vst1_3d_32((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D) dst); }              else if (size == 16) { FDK_vst1_3d_16((FIXP_S)src1,(FIXP_S)src2,(FIXP_S)src3,(FIXP_S) dst); } dst += (INT)24/(INT)sizeof(*dst);  }
#define FDK_vst1_3d_pu(size, src1, src2, src3, dst, step)       { if (size == 64) { FDK_vst1_3d_64((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D) dst); }              else if (size == 32) { FDK_vst1_3d_32((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D) dst); }              else if (size == 16) { FDK_vst1_3d_16((FIXP_S)src1,(FIXP_S)src2,(FIXP_S)src3,(FIXP_S) dst); } dst += step/(INT)sizeof(*dst); }
#define FDK_vst1_4d(   size, src1, src2, src3, src4, dst)       { if (size == 64) { FDK_vst1_4d_64((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D)src4,(FIXP_D) dst); } else if (size == 32) { FDK_vst1_4d_32((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D)src4,(FIXP_D) dst); } else if (size == 16) { FDK_vst1_4d_16((FIXP_S)src1,(FIXP_S)src2,(FIXP_S)src3,(FIXP_S)src4,(FIXP_S) dst); } }
#define FDK_vst1_4d_ia(size, src1, src2, src3, src4, dst)       { if (size == 64) { FDK_vst1_4d_64((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D)src4,(FIXP_D) dst); } else if (size == 32) { FDK_vst1_4d_32((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D)src4,(FIXP_D) dst); } else if (size == 16) { FDK_vst1_4d_16((FIXP_S)src1,(FIXP_S)src2,(FIXP_S)src3,(FIXP_S)src4,(FIXP_S) dst); } dst += (INT)32/(INT)sizeof(*dst);  }
#define FDK_vst1_4d_pu(size, src1, src2, src3, src4, dst, step) { if (size == 64) { FDK_vst1_4d_64((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D)src4,(FIXP_D) dst); } else if (size == 32) { FDK_vst1_4d_32((FIXP_D)src1,(FIXP_D)src2,(FIXP_D)src3,(FIXP_D)src4,(FIXP_D) dst); } else if (size == 16) { FDK_vst1_4d_16((FIXP_S)src1,(FIXP_S)src2,(FIXP_S)src3,(FIXP_S)src4,(FIXP_S) dst); } dst += step/(INT)sizeof(*dst); }

#define FDK_vst1_1d_align(   size, src1, dst, align)                         FDK_vst1_1d(   size, src1, dst)
#define FDK_vst1_1d_ia_align(size, src1, dst, align)                         FDK_vst1_1d_ia(size, src1, dst)
#define FDK_vst1_1d_pu_align(size, src1, dst, align, step)                   FDK_vst1_1d_pu(size, src1, dst, step)
#define FDK_vst1_2d_align(   size, src1, src2, dst, align)                   FDK_vst1_2d(   size, src1, src2, dst)
#define FDK_vst1_2d_ia_align(size, src1, src2, dst, align)                   FDK_vst1_2d_ia(size, src1, src2, dst)
#define FDK_vst1_2d_pu_align(size, src1, src2, dst, align, step)             FDK_vst1_2d_pu(size, src1, src2, dst, step)
#define FDK_vst1_3d_align(   size, src1, src2, src3, dst, align)             FDK_vst1_3d(   size, src1, src2, src3, dst)
#define FDK_vst1_3d_ia_align(size, src1, src2, src3, dst, align)             FDK_vst1_3d_ia(size, src1, src2, src3, dst)
#define FDK_vst1_3d_pu_align(size, src1, src2, src3, dst, align, step)       FDK_vst1_3d_pu(size, src1, src2, src3, dst, step)
#define FDK_vst1_4d_align(   size, src1, src2, src3, src4, dst, align)       FDK_vst1_4d(   size, src1, src2, src3, src4, dst)
#define FDK_vst1_4d_ia_align(size, src1, src2, src3, src4, dst, align)       FDK_vst1_4d_ia(size, src1, src2, src3, src4, dst)
#define FDK_vst1_4d_pu_align(size, src1, src2, src3, src4, dst, align, step) FDK_vst1_4d_pu(size, src1, src2, src3, src4, dst, step)

static void inline FDK_vst1_32(FIXP_D src, FIXP_D dst)
{
  FIXP_S Src = (FIXP_S) src;
  FIXP_S Dst = (FIXP_S) dst;
  Dst[0] = Src[0];
  Dst[1] = Src[1];
}

static void inline FDK_vst1_16(FIXP_S src, FIXP_S dst)
{
  dst[0] = src[0];
}

void FDK_vst1_1d_16(FIXP_S src1, FIXP_S dst)
{
  for (int i = 0; i < 4; i++)
  {
    dst[i] = src1[i];
  }
}
static void inline FDK_vst1_2d_16(FIXP_S src1, FIXP_S src2, FIXP_S dst)
{
  FDK_ASSERT(src2 == &src1[4]);
  for (int i = 0; i < 4; i++)
  {
    dst[i] = src1[i];
  }
  for (int i = 0; i < 4; i++)
  {
    dst[i+4] = src2[i];
  }
}
static void inline FDK_vst1_3d_16(FIXP_S src1, FIXP_S src2, FIXP_S src3, FIXP_S dst)
{
  FDK_ASSERT(src2 == &src1[4]);
  FDK_ASSERT(src3 == &src1[8]);
  for (int i = 0; i < 4; i++)
  {
    dst[i] = src1[i];
  }
  for (int i = 0; i < 4; i++)
  {
    dst[i+4] = src2[i];
  }
  for (int i = 0; i < 4; i++)
  {
    dst[i+8] = src3[i];
  }
}
static void inline FDK_vst1_4d_16(FIXP_S src1, FIXP_S src2, FIXP_S src3, FIXP_S src4, FIXP_S dst)
{
  FDK_ASSERT(src2 == &src1[4]);
  FDK_ASSERT(src3 == &src1[8]);
  FDK_ASSERT(src4 == &src1[12]);
  for (int i = 0; i < 4; i++)
  {
    dst[i] = src1[i];
  }
  for (int i = 0; i < 4; i++)
  {
    dst[i+4] = src2[i];
  }
  for (int i = 0; i < 4; i++)
  {
    dst[i+8] = src3[i];
  }
  for (int i = 0; i < 4; i++)
  {
    dst[i+12] = src4[i];
  }
}
static void inline FDK_vst1_1d_32(FIXP_D src1, FIXP_D dst)
{
  /* for unknown reasons, this copy must be done upon 16-bit on Linux 64-bit machine */
  FDK_vst1_1d_16((FIXP_S) src1, (FIXP_S) dst);
}
static void inline FDK_vst1_2d_32(FIXP_D src1, FIXP_D src2, FIXP_D dst)
{
  FDK_ASSERT(src2 == &src1[2]);
  FDK_vst1_2d_16((FIXP_S) src1, (FIXP_S) src2, (FIXP_S) dst);
}
static void inline FDK_vst1_3d_32(FIXP_D src1, FIXP_D src2, FIXP_D src3, FIXP_D dst)
{
  FDK_ASSERT(src2 == &src1[2]);
  FDK_ASSERT(src3 == &src1[4]);
  FDK_vst1_3d_16((FIXP_S) src1, (FIXP_S) src2, (FIXP_S) src3, (FIXP_S) dst);
}
static void inline FDK_vst1_4d_32(FIXP_D src1, FIXP_D src2, FIXP_D src3, FIXP_D src4, FIXP_D dst)
{
  FDK_ASSERT(src2 == &src1[2]);
  FDK_ASSERT(src3 == &src1[4]);
  FDK_ASSERT(src4 == &src1[6]);
  FDK_vst1_4d_16((FIXP_S) src1, (FIXP_S) src2, (FIXP_S) src3, (FIXP_S) src4, (FIXP_S) dst);
}

static void inline FDK_vst1_1d_64(FIXP_H src1, FIXP_D dst)
{
  FDK_vst1_1d_16((FIXP_S) src1, (FIXP_S) dst);
}
static void inline FDK_vst1_2d_64(FIXP_H src1, FIXP_H src2, FIXP_D dst)
{
  FDK_ASSERT(src2 == &src1[2]);
  FDK_vst1_2d_16((FIXP_S) src1, (FIXP_S) src2, (FIXP_S) dst);
}
static void inline FDK_vst1_3d_64(FIXP_H src1, FIXP_H src2, FIXP_H src3, FIXP_D dst)
{
  FDK_ASSERT(src2 == &src1[2]);
  FDK_ASSERT(src3 == &src1[4]);
  FDK_vst1_3d_16((FIXP_S) src1, (FIXP_S) src2, (FIXP_S) src3, (FIXP_S) dst);
}
static void inline FDK_vst1_4d_64(FIXP_H src1, FIXP_H src2, FIXP_H src3, FIXP_H src4, FIXP_D dst)
{
  FDK_ASSERT(src2 == &src1[2]);
  FDK_ASSERT(src3 == &src1[4]);
  FDK_ASSERT(src4 == &src1[6]);
  FDK_vst1_4d_16((FIXP_S) src1, (FIXP_S) src2, (FIXP_S) src3, (FIXP_S) src4, (FIXP_S) dst);
}

#endif


/*
   Description: Load two 16/32-bit lanes of 2 D registers from memory (src)
   Parameter size: 16 or 32
   Parameter dst1: Dx[i] 64-bit NEON register
   Parameter dst1: Dy[i] 64-bit NEON register (y must be x+1 or x+2, i in [0,1]))
   Parameter src:  r core register used as a pointer to 16/32-Bit data
   Parameter step: r core register indicating number of bytes for post-incrementing the dst pointer
*/

#ifdef __ARM_NEON__
#define __FDK_vld2(   size, dst1, dst2, src)                  "VLD2." #size " {" #dst1 "," #dst2 "}, [" #src "] \n\t"
#define __FDK_vld2_ia(size, dst1, dst2, src)                  "VLD2." #size " {" #dst1 "," #dst2 "}, [" #src "]! \n\t"
#define __FDK_vld2_pu(size, dst1, dst2, src, Rm)              "VLD2." #size " {" #dst1 "," #dst2 "}, [" #src "], " #Rm " \n\t"
#define __FDK_vld2_align(   size, dst1, dst2, src, align)     "VLD2." #size " {" #dst1 "," #dst2 "}, [" #src ",:" #align "] \n\t"
#define __FDK_vld2_ia_align(size, dst1, dst2, src, align)     "VLD2." #size " {" #dst1 "," #dst2 "}, [" #src ",:" #align "]! \n\t"
#define __FDK_vld2_pu_align(size, dst1, dst2, src, align, Rm) "VLD2." #size " {" #dst1 "," #dst2 "}, [" #src ",:" #align "], " #Rm " \n\t"
#define FDK_vld2(   size, dst1, dst2, src)                      __FDK_vld2(         size, dst1, dst2, src)
#define FDK_vld2_ia(size, dst1, dst2, src)                      __FDK_vld2_ia(      size, dst1, dst2, src)
#define FDK_vld2_pu(size, dst1, dst2, src, Rm)                  __FDK_vld2_pu(      size, dst1, dst2, src, Rm)
#define FDK_vld2_align(   size, dst1, dst2, src, align)         __FDK_vld2_align(   size, dst1, dst2, src, align)
#define FDK_vld2_ia_align(size, dst1, dst2, src, align)         __FDK_vld2_ia_align(size, dst1, dst2, src, align)
#define FDK_vld2_pu_align(size, dst1, dst2, src, align, Rm)     __FDK_vld2_pu_align(size, dst1, dst2, src, align, Rm)
#define FDK_vld2_2d(   size, dst1, dst2, src)                   __FDK_vld2(         size, dst1, dst2, src)
#define FDK_vld2_2d_ia(size, dst1, dst2, src)                   __FDK_vld2_ia(      size, dst1, dst2, src)
#define FDK_vld2_2d_pu(size, dst1, dst2, src, Rm)               __FDK_vld2_pu(      size, dst1, dst2, src, Rm)
#define FDK_vld2_2d_align(   size, dst1, dst2, src, align)      __FDK_vld2_align(   size, dst1, dst2, src, align)
#define FDK_vld2_2d_ia_align(size, dst1, dst2, src, align)      __FDK_vld2_ia_align(size, dst1, dst2, src, align)
#define FDK_vld2_2d_pu_align(size, dst1, dst2, src, align, Rm)  __FDK_vld2_pu_align(size, dst1, dst2, src, align, Rm)
#define __FDK_vld2_4d(         size, dst1, dst2, dst3, dst4, src)             "VLD2." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src "] \n\t"
#define __FDK_vld2_4d_ia(      size, dst1, dst2, dst3, dst4, src)             "VLD2." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src "]! \n\t"
#define __FDK_vld2_4d_pu(      size, dst1, dst2, dst3, dst4, src, Rm)         "VLD2." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src "], " #Rm " \n\t"
#define __FDK_vld2_4d_align(   size, dst1, dst2, dst3, dst4, src, align)      "VLD2." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src ",:" #align "] \n\t"
#define __FDK_vld2_4d_ia_align(size, dst1, dst2, dst3, dst4, src, align)      "VLD2." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src ",:" #align "]! \n\t"
#define __FDK_vld2_4d_pu_align(size, dst1, dst2, dst3, dst4, src, align, Rm)  "VLD2." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src ",:" #align "], " #Rm " \n\t"
#define FDK_vld2_4d(           size, dst1, dst2, dst3, dst4, src)             __FDK_vld2_4d(         size, dst1, dst2, dst3, dst4, src)
#define FDK_vld2_4d_ia(        size, dst1, dst2, dst3, dst4, src)             __FDK_vld2_4d_ia(      size, dst1, dst2, dst3, dst4, src)
#define FDK_vld2_4d_pu(        size, dst1, dst2, dst3, dst4, src, Rm)         __FDK_vld2_4d_pu(      size, dst1, dst2, dst3, dst4, src, Rm)
#define FDK_vld2_4d_align(     size, dst1, dst2, dst3, dst4, src, align)      __FDK_vld2_4d_align(   size, dst1, dst2, dst3, dst4, src, align)
#define FDK_vld2_4d_ia_align(  size, dst1, dst2, dst3, dst4, src, align)      __FDK_vld2_4d_ia_align(size, dst1, dst2, dst3, dst4, src, align)
#define FDK_vld2_4d_pu_align(  size, dst1, dst2, dst3, dst4, src, align, Rm)  __FDK_vld2_4d_pu_align(size, dst1, dst2, dst3, dst4, src, align, Rm)
#else
/* reference: NEON and VFP programming, chapter 4.8.3 */
#define FDK_vld2(   size, dst1, dst2, src)         { if (size == 32) { FDK_vld2_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D) src); } else if (size == 16) { FDK_vld2_16((FIXP_S) dst1,(FIXP_S) dst2,(FIXP_S) src); } }
#define FDK_vld2_ia(size, dst1, dst2, src)         { if (size == 32) { FDK_vld2_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D) src); } else if (size == 16) { FDK_vld2_16((FIXP_S) dst1,(FIXP_S) dst2,(FIXP_S) src); } src += 2*(size>>3)/(INT)sizeof(*src);  }
#define FDK_vld2_pu(size, dst1, dst2, src, step)   { if (size == 32) { FDK_vld2_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D) src); } else if (size == 16) { FDK_vld2_16((FIXP_S) dst1,(FIXP_S) dst2,(FIXP_S) src); } src += step/(INT)sizeof(*src); }
#define FDK_vld2_align(   size, dst1, dst2, src, align)       FDK_vld2(   size, dst1, dst2, src)
#define FDK_vld2_ia_align(size, dst1, dst2, src, align)       FDK_vld2_ia(size, dst1, dst2, src)
#define FDK_vld2_pu_align(size, dst1, dst2, src, align, step) FDK_vld2_pu(size, dst1, dst2, src, step)

static void inline FDK_vld2_32(FIXP_Q dst1, FIXP_Q dst2, FIXP_DBL *src)
{
  FDK_ASSERT((dst2==dst1+1*2) || (dst2==dst1+2*2));
  dst1[0] = src[0];
  dst2[0] = src[1];
}

static void inline FDK_vld2_16(FIXP_S dst1, FIXP_S dst2, FIXP_SGL *src)
{
  FDK_ASSERT((dst2==dst1+1*4) || (dst2==dst1+2*4));
  dst1[0] = src[0];
  dst2[0] = src[1];
}

/* reference: NEON and VFP programming, chapter 4.8.5 */
#define FDK_vld2_2d(         size, dst1, dst2, src)                          { if (size == 32) { FDK_vld2_2d_32((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_D) src); } else if (size == 16) { FDK_vld2_2d_16((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_S) src); } }
#define FDK_vld2_2d_ia(      size, dst1, dst2, src)                          { if (size == 32) { FDK_vld2_2d_32((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_D) src); } else if (size == 16) { FDK_vld2_2d_16((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_S) src); } src += 16/(INT)sizeof(*src);  }
#define FDK_vld2_2d_pu(      size, dst1, dst2, src, step)                    { if (size == 32) { FDK_vld2_2d_32((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_D) src); } else if (size == 16) { FDK_vld2_2d_16((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_S) src); } src += step/(INT)sizeof(*src); }
#define FDK_vld2_2d_align(   size, dst1, dst2, src, align)                   FDK_vld2_2d(   size, dst1, dst2, src)
#define FDK_vld2_2d_ia_align(size, dst1, dst2, src, align)                   FDK_vld2_2d_ia(size, dst1, dst2, src)
#define FDK_vld2_2d_pu_align(size, dst1, dst2, src, align, step)             FDK_vld2_2d_pu(size, dst1, dst2, src, step)
#define FDK_vld2_4d(         size, dst1, dst2, dst3, dst4, src)              { if (size == 32) { FDK_vld2_4d_32((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_H)dst3,(FIXP_H)dst4,(FIXP_D) src); } else if (size == 16) { FDK_vld2_4d_16((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_H)dst3,(FIXP_H)dst4,(FIXP_S) src); } }
#define FDK_vld2_4d_ia(      size, dst1, dst2, dst3, dst4, src)              { if (size == 32) { FDK_vld2_4d_32((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_H)dst3,(FIXP_H)dst4,(FIXP_D) src); } else if (size == 16) { FDK_vld2_4d_16((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_H)dst3,(FIXP_H)dst4,(FIXP_S) src); } src += 32/(INT)sizeof(*src);  }
#define FDK_vld2_4d_pu(      size, dst1, dst2, dst3, dst4, src, step)        { if (size == 32) { FDK_vld2_4d_32((FIXP_H)dst1,(FIXP_H)dst2,(FIXP_H)dst3,(FIXP_H)dst4,(FIXP_D) src); } else if (size == 16) { FDK_vld2_4d_16((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_H)dst3,(FIXP_H)dst4,(FIXP_S) src); } src += step/(INT)sizeof(*src); }
#define FDK_vld2_4d_align(   size, dst1, dst2, dst3, dst4, src, align)       FDK_vld2_4d(   size, dst1, dst2, dst3, dst4, src)
#define FDK_vld2_4d_ia_align(size, dst1, dst2, dst3, dst4, src, align)       FDK_vld2_4d_ia(size, dst1, dst2, dst3, dst4, src)
#define FDK_vld2_4d_pu_align(size, dst1, dst2, dst3, dst4, src, align, step) FDK_vld2_4d_pu(size, dst1, dst2, dst3, dst4, src, step)

static void inline FDK_vld2_2d_32(FIXP_H dst1, FIXP_H dst2, FIXP_D src)
{
  FIXP_D Dst1 = (FIXP_D) dst1;
  FIXP_D Dst2 = (FIXP_D) dst2;
  FDK_ASSERT((Dst2==Dst1+1*2) || (Dst2==Dst1+2*2));
  for (int i = 0; i < 2; i++)
  {
    Dst1[i] = src[2*i+0];
    Dst2[i] = src[2*i+1];
  }
}

static void inline FDK_vld2_2d_16(FIXP_H dst1, FIXP_H dst2, FIXP_S src)
{
  FIXP_S Dst1 = (FIXP_S) dst1;
  FIXP_S Dst2 = (FIXP_S) dst2;
  FDK_ASSERT((Dst2==Dst1+1*4) || (Dst2==Dst1+2*4));
  for (int i = 0; i < 4; i++)
  {
    Dst1[i] = src[2*i+0];
    Dst2[i] = src[2*i+1];
  }
}

static void inline FDK_vld2_4d_32(FIXP_H dst1, FIXP_H dst2, FIXP_H dst3, FIXP_H dst4, FIXP_D src)
{
  FIXP_D Dst1 = (FIXP_D) dst1;
  FIXP_D Dst2 = (FIXP_D) dst2;
  FIXP_D Dst3 = (FIXP_D) dst3;
  FIXP_D Dst4 = (FIXP_D) dst4;
  (void)Dst2;
  (void)Dst4;
  FDK_ASSERT((Dst2==Dst1+1*2) && (Dst3==Dst1+2*2) && (Dst4==Dst1+3*2));
  /* load even elements into lower part, odd elements into upper part */
  for (int i = 0; i < 4; i++)
  {
    Dst1[i] = src[2*i+0];
    Dst3[i] = src[2*i+1];
  }
}

static void inline FDK_vld2_4d_16(FIXP_H dst1, FIXP_H dst2, FIXP_H dst3, FIXP_H dst4, FIXP_S src)
{
  FIXP_S Dst1 = (FIXP_S) dst1;
  FIXP_S Dst2 = (FIXP_S) dst2;
  FIXP_S Dst3 = (FIXP_S) dst3;
  FIXP_S Dst4 = (FIXP_S) dst4;
  (void)Dst2;
  (void)Dst4;
  FDK_ASSERT((Dst2==Dst1+1*4) && (Dst3==Dst1+2*4) && (Dst4==Dst1+3*4));
  /* load even elements into lower part, odd elements into upper part */
  for (int i = 0; i < 8; i++)
  {
    Dst1[i] = src[2*i+0];
    Dst3[i] = src[2*i+1];
  }
}

#endif

/*
   Description: Store two 16/32-bit lanes of two D registers into memory (dst)
   Parameter size: 16 or 32
   Parameter src1: Dx[i] 64-bit NEON register
   Parameter src2: Dy[i] 64-bit NEON register (y must be x+1 or x+2, i in [0,1]))
   Parameter dst:  r core register used as a pointer to 16/32-bit data
   Parameter step: r core register indicating number of bytes for post-incrementing the dst pointer
*/

#ifdef __ARM_NEON__
#define __FDK_vst2(   size, src1, src2, dst)                                  "VST2." #size " {" #src1 ", " #src2 "}, [" #dst "] \n\t"
#define __FDK_vst2_ia(size, src1, src2, dst)                                  "VST2." #size " {" #src1 ", " #src2 "}, [" #dst "]! \n\t"
#define __FDK_vst2_pu(size, src1, src2, dst, step)                            "VST2." #size " {" #src1 ", " #src2 "}, [" #dst "], " #step " \n\t"
#define __FDK_vst2_align(   size, src1, src2, dst, align)                     "VST2." #size " {" #src1 ", " #src2 "}, [" #dst ",:" #align "] \n\t"
#define __FDK_vst2_ia_align(size, src1, src2, dst, align)                     "VST2." #size " {" #src1 ", " #src2 "}, [" #dst ",:" #align "]! \n\t"
#define __FDK_vst2_pu_align(size, src1, src2, dst, align, step)               "VST2." #size " {" #src1 ", " #src2 "}, [" #dst ",:" #align "], " #step " \n\t"
#define FDK_vst2(            size, src1, src2, dst)                           __FDK_vst2(   size, src1, src2, dst)
#define FDK_vst2_ia(         size, src1, src2, dst)                           __FDK_vst2_ia(size, src1, src2, dst)
#define FDK_vst2_pu(         size, src1, src2, dst, step)                     __FDK_vst2_pu(size, src1, src2, dst, step)
#define FDK_vst2_align(      size, src1, src2, dst, align)                    __FDK_vst2_align(   size, src1, src2, dst, align)
#define FDK_vst2_ia_align(   size, src1, src2, dst, align)                    __FDK_vst2_ia_align(size, src1, src2, dst, align)
#define FDK_vst2_pu_align(   size, src1, src2, dst, align, step)              __FDK_vst2_pu_align(size, src1, src2, dst, align, step)
#define FDK_vst2_2d(         size, src1, src2, dst)                           __FDK_vst2(   size, src1, src2, dst)
#define FDK_vst2_2d_ia(      size, src1, src2, dst)                           __FDK_vst2_ia(size, src1, src2, dst)
#define FDK_vst2_2d_pu(      size, src1, src2, dst, step)                     __FDK_vst2_pu(size, src1, src2, dst, step)
#define FDK_vst2_2d_align(   size, src1, src2, dst, align)                    __FDK_vst2_align(   size, src1, src2, dst, align)
#define FDK_vst2_2d_ia_align(size, src1, src2, dst, align)                    __FDK_vst2_ia_align(size, src1, src2, dst, align)
#define FDK_vst2_2d_pu_align(size, src1, src2, dst, align, step)              __FDK_vst2_pu_align(size, src1, src2, dst, align, step)
#define FDK_vst2_4d(         size, src1, src2, src3, src4, dst)               "VST2." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst "] \n\t"
#define FDK_vst2_4d_ia(      size, src1, src2, src3, src4, dst)               "VST2." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst "]! \n\t"
#define FDK_vst2_4d_pu(      size, src1, src2, src3, src4, dst, step)         "VST2." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst "], " #step " \n\t"
#define FDK_vst2_4d_align(   size, src1, src2, src3, src4, dst, align)        "VST2." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst ",:" #align "] \n\t"
#define FDK_vst2_4d_ia_align(size, src1, src2, src3, src4, dst, align)        "VST2." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst ",:" #align "]! \n\t"
#define FDK_vst2_4d_pu_align(size, src1, src2, src3, src4, dst, align, step)  "VST2." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst ",:" #align "], " #step " \n\t"
#else
/* reference: NEON and VFP programming, chapter 4.8.3 */
#define FDK_vst2(   size, src1, src2, dst)         { if (size == 32) { FDK_vst2_32(src1, src2, dst); } else if (size == 16) { FDK_vst2_16((FIXP_S) src1,(FIXP_S) src2,(FIXP_S) dst); } }
#define FDK_vst2_ia(size, src1, src2, dst)         { if (size == 32) { FDK_vst2_32(src1, src2, dst); } else if (size == 16) { FDK_vst2_16((FIXP_S) src1,(FIXP_S) src2,(FIXP_S) dst); } dst += 2*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vst2_pu(size, src1, src2, dst, step)   { if (size == 32) { FDK_vst2_32(src1, src2, dst); } else if (size == 16) { FDK_vst2_16((FIXP_S) src1,(FIXP_S) src2,(FIXP_S) dst); } dst += step/(INT)sizeof(*dst); }
#define FDK_vst2_align(   size, src1, src2, dst, align)         FDK_vst2(   size, src1, src2, dst)
#define FDK_vst2_ia_align(size, src1, src2, dst, align)         FDK_vst2_ia(size, src1, src2, dst)
#define FDK_vst2_pu_align(size, src1, src2, dst, align, step)   FDK_vst2_pu(size, src1, src2, dst, step)

static void inline FDK_vst2_32(FIXP_Q src1, FIXP_Q src2, FIXP_DBL *dst)
{
  FDK_ASSERT((src2==src1+1*2) || (src2==src1+2*2));
  dst[0] = src1[0];
  dst[1] = src2[0];
}

static void inline FDK_vst2_16(FIXP_S src1, FIXP_S src2, FIXP_SGL *dst)
{
  FDK_ASSERT((src2==src1+1*4) || (src2==src1+2*4));
  dst[0] = src1[0];
  dst[1] = src2[0];
}
/* reference: NEON and VFP programming, chapter 4.8.5 */
#define FDK_vst2_2d(         size, src1, src2, dst)                          { if (size == 32) { FDK_vst2_2d_32((FIXP_H)src1,(FIXP_H)src2,(FIXP_D) dst); } else if (size == 16) { FDK_vst2_2d_16((FIXP_H) src1,(FIXP_H) src2,(FIXP_S) dst); } }
#define FDK_vst2_2d_ia(      size, src1, src2, dst)                          { if (size == 32) { FDK_vst2_2d_32((FIXP_H)src1,(FIXP_H)src2,(FIXP_D) dst); } else if (size == 16) { FDK_vst2_2d_16((FIXP_H) src1,(FIXP_H) src2,(FIXP_S) dst); } dst += 16/(INT)sizeof(*dst);  }
#define FDK_vst2_2d_pu(      size, src1, src2, dst, step)                    { if (size == 32) { FDK_vst2_2d_32((FIXP_H)src1,(FIXP_H)src2,(FIXP_D) dst); } else if (size == 16) { FDK_vst2_2d_16((FIXP_H) src1,(FIXP_H) src2,(FIXP_S) dst); } dst += step/(INT)sizeof(*dst); }
#define FDK_vst2_2d_align(   size, src1, src2, dst, align)                   FDK_vst2_2d(   size, src1, src2, dst)
#define FDK_vst2_2d_ia_align(size, src1, src2, dst, align)                   FDK_vst2_2d_ia(size, src1, src2, dst)
#define FDK_vst2_2d_pu_align(size, src1, src2, dst, align, step)             FDK_vst2_2d_pu(size, src1, src2, dst, step)
#define FDK_vst2_4d(         size, src1, src2, src3, src4, dst)              { if (size == 32) { FDK_vst2_4d_32((FIXP_H)src1,(FIXP_H)src2,(FIXP_H)src3,(FIXP_H)src4,(FIXP_D) dst); } else if (size == 16) { FDK_vst2_4d_16((FIXP_H) src1,(FIXP_H) src2,(FIXP_H)src3,(FIXP_H)src4,(FIXP_S) dst); } }
#define FDK_vst2_4d_ia(      size, src1, src2, src3, src4, dst)              { if (size == 32) { FDK_vst2_4d_32((FIXP_H)src1,(FIXP_H)src2,(FIXP_H)src3,(FIXP_H)src4,(FIXP_D) dst); } else if (size == 16) { FDK_vst2_4d_16((FIXP_H) src1,(FIXP_H) src2,(FIXP_H)src3,(FIXP_H)src4,(FIXP_S) dst); } dst += 32/(INT)sizeof(*dst);  }
#define FDK_vst2_4d_pu(      size, src1, src2, src3, src4, dst, step)        { if (size == 32) { FDK_vst2_4d_32((FIXP_H)src1,(FIXP_H)src2,(FIXP_H)src3,(FIXP_H)src4,(FIXP_D) dst); } else if (size == 16) { FDK_vst2_4d_16((FIXP_H) src1,(FIXP_H) src2,(FIXP_H)src3,(FIXP_H)src4,(FIXP_S) dst); } dst += step/(INT)sizeof(*dst); }
#define FDK_vst2_4d_align(   size, src1, src2, src3, src4, dst, align)       FDK_vst2_4d(   size, src1, src2, src3, src4, dst)
#define FDK_vst2_4d_ia_align(size, src1, src2, src3, src4, dst, align)       FDK_vst2_4d_ia(size, src1, src2, src3, src4, dst)
#define FDK_vst2_4d_pu_align(size, src1, src2, src3, src4, dst, align, step) FDK_vst2_4d_pu(size, src1, src2, src3, src4, dst, step)


static void inline FDK_vst2_2d_32(FIXP_H src1, FIXP_H src2, FIXP_DBL *dst)
{
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  FDK_ASSERT((Src2==Src1+1*2) ||(Src2==Src1+2*2));
  /* store even elements from lower part, odd elements from upper part */
  for (int i = 0; i < 2; i++)
  {
    dst[2*i+0] = Src1[i];
    dst[2*i+1] = Src2[i];
  }
}

static void inline FDK_vst2_2d_16(FIXP_H src1, FIXP_H src2, FIXP_SGL *dst)
{
  FIXP_S Src1 = (FIXP_S) src1;
  FIXP_S Src2 = (FIXP_S) src2;
  FDK_ASSERT((Src2==Src1+1*4) || (Src2==Src1+2*4));
  /* store even elements from lower part, odd elements from upper part */
  for (int i = 0; i < 4; i++)
  {
    dst[2*i+0] = Src1[i];
    dst[2*i+1] = Src2[i];
  }
}


static void inline FDK_vst2_4d_32(FIXP_H src1, FIXP_H src2, FIXP_H src3, FIXP_H src4, FIXP_DBL *dst)
{
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  FIXP_D Src3 = (FIXP_D) src3;
  FIXP_D Src4 = (FIXP_D) src4;
  (void)Src2;
  (void)Src4;
  FDK_ASSERT((Src2==Src1+1*2) && (Src3==Src1+2*2) && (Src4==Src1+3*2));
  /* store even elements from lower part, odd elements from upper part */
  for (int i = 0; i < 4; i++)
  {
    dst[2*i+0] = Src1[i];
    dst[2*i+1] = Src3[i];
  }
}

static void inline FDK_vst2_4d_16(FIXP_H src1, FIXP_H src2, FIXP_H src3, FIXP_H src4, FIXP_SGL *dst)
{
  FIXP_S Src1 = (FIXP_S) src1;
  FIXP_S Src2 = (FIXP_S) src2;
  FIXP_S Src3 = (FIXP_S) src3;
  FIXP_S Src4 = (FIXP_S) src4;
  (void)Src2;
  (void)Src4;
  FDK_ASSERT((Src2==Src1+1*4) && (Src3==Src1+2*4) && (Src4==Src1+3*4));
  /* store even elements from lower part, odd elements from upper part */
  for (int i = 0; i < 8; i++)
  {
    dst[2*i+0] = Src1[i];
    dst[2*i+1] = Src3[i];
  }
}


#endif


/*
   Description: Load 3 16/32-bit lanes of 3 D registers from memory (src)
   Parameter size: 16 or 32
   Parameter dst1: Dx[i] 64-bit NEON register
   Parameter dst1: Dy[i] 64-bit NEON register (y,z must be x+1,x+2 or x+2,x+4 i in [0,1]))
   Parameter dst2: Dz[i] 64-bit NEON register (y,z must be x+1,x+2 or x+2,x+4 i in [0,1]))
   Parameter src:  r core register used as a pointer to 16/32-bit data
   Parameter step: r core register indicating number of bytes for post-incrementing the dst pointer
*/

#ifdef __ARM_NEON__
#define __FDK_vld3(   size, dst1, dst2, dst3, src)                  "VLD3." #size " {" #dst1 "," #dst2 "," #dst3 "}, [" #src "] \n\t"
#define __FDK_vld3_ia(size, dst1, dst2, dst3, src)                  "VLD3." #size " {" #dst1 "," #dst2 "," #dst3 "}, [" #src "]! \n\t"
#define __FDK_vld3_pu(size, dst1, dst2, dst3, src, Rm)              "VLD3." #size " {" #dst1 "," #dst2 "," #dst3 "}, [" #src "], " #Rm " \n\t"
#define __FDK_vld3_align(   size, dst1, dst2, dst3, src, align)     "VLD3." #size " {" #dst1 "," #dst2 "," #dst3 "}, [" #src ",:" #align "] \n\t"
#define __FDK_vld3_ia_align(size, dst1, dst2, dst3, src, align)     "VLD3." #size " {" #dst1 "," #dst2 "," #dst3 "}, [" #src ",:" #align "]! \n\t"
#define __FDK_vld3_pu_align(size, dst1, dst2, dst3, src, align, Rm) "VLD3." #size " {" #dst1 "," #dst2 "," #dst3 "}, [" #src ",:" #align "], " #Rm " \n\t"
#define FDK_vld3(   size, dst1, dst2, dst3, src)      __FDK_vld3(   size, dst1, dst2, dst3, src)
#define FDK_vld3_ia(size, dst1, dst2, dst3, src)      __FDK_vld3_ia(size, dst1, dst2, dst3, src)
#define FDK_vld3_pu(size, dst1, dst2, dst3, src, Rm)  __FDK_vld3_pu(size, dst1, dst2, dst3, src, Rm)
#define FDK_vld3_align(   size, dst1, dst2, dst3, src, align)      __FDK_vld3_align(   size, dst1, dst2, dst3, src, align)
#define FDK_vld3_ia_align(size, dst1, dst2, dst3, src, align)      __FDK_vld3_ia_align(size, dst1, dst2, dst3, src, align)
#define FDK_vld3_pu_align(size, dst1, dst2, dst3, src, align, Rm)  __FDK_vld3_pu_align(size, dst1, dst2, dst3, src, align, Rm)
#else
#define FDK_vld3(   size, dst1, dst2, dst3, src)         { if (size == 32) { FDK_vld3_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D) src); } else if (size == 16) { FDK_vld3_16((FIXP_S) dst1,(FIXP_S) dst2,(FIXP_S) dst3,(FIXP_S) src); } }
#define FDK_vld3_ia(size, dst1, dst2, dst3, src)         { if (size == 32) { FDK_vld3_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D) src); } else if (size == 16) { FDK_vld3_16((FIXP_S) dst1,(FIXP_S) dst2,(FIXP_S) dst3,(FIXP_S) src); } src += 3*(size>>3)/(INT)sizeof(*src);  }
#define FDK_vld3_pu(size, dst1, dst2, dst3, src, step)   { if (size == 32) { FDK_vld3_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D) src); } else if (size == 16) { FDK_vld3_16((FIXP_S) dst1,(FIXP_S) dst2,(FIXP_S) dst3,(FIXP_S) src); } src += step/(INT)sizeof(*src); }
#define FDK_vld3_align(   size, dst1, dst2, dst3, src, align)         FDK_vld3(   size, dst1, dst2, dst3, src)
#define FDK_vld3_ia_align(size, dst1, dst2, dst3, src, align)         FDK_vld3_ia(size, dst1, dst2, dst3, src)
#define FDK_vld3_pu_align(size, dst1, dst2, dst3, src, align, step)   FDK_vld3_pu(size, dst1, dst2, dst3, src, step)

static void inline FDK_vld3_32(FIXP_Q dst1, FIXP_Q dst2, FIXP_Q dst3, FIXP_DBL *src)
{
  dst1[0] = src[0];
  dst2[0] = src[1];
  dst3[0] = src[2];
}

static void inline FDK_vld3_16(FIXP_S dst1, FIXP_S dst2, FIXP_S dst3, FIXP_SGL *src)
{
  dst1[0] = src[0];
  dst2[0] = src[1];
  dst3[0] = src[2];
}
#endif

/*
   Description: Store 3 16/32-bit lanes of 3 D registers into memory (dst)
   Parameter size: 16 or 32
   Parameter src1: Dx[i] 64-bit NEON register (y,z must be x+1,x+2 or x+2,x+4 i in [0,1]))
   Parameter src2: Dy[i] 64-bit NEON register (y,z must be x+1,x+2 or x+2,x+4 i in [0,1]))
   Parameter src3: Dz[i] 64-bit NEON register (y,z must be x+1,x+2 or x+2,x+4 i in [0,1]))
   Parameter dst:  r core register used as a pointer to 16/32-bit data
   Parameter step: r core register indicating number of bytes for post-incrementing the dst pointer
*/

#ifdef __ARM_NEON__
#define __FDK_vst3(   size, src1, src2, src3, dst)                     "VST3." #size " {" #src1 ", " #src2 ", " #src3 "}, [" #dst "] \n\t"
#define __FDK_vst3_ia(size, src1, src2, src3, dst)                     "VST3." #size " {" #src1 ", " #src2 ", " #src3 "}, [" #dst "]! \n\t"
#define __FDK_vst3_pu(size, src1, src2, src3, dst, step)               "VST3." #size " {" #src1 ", " #src2 ", " #src3 "}, [" #dst "], " #step " \n\t"
#define __FDK_vst3_align(   size, src1, src2, src3, dst, align)        "VST3." #size " {" #src1 ", " #src2 ", " #src3 "}, [" #dst ",:" #align "] \n\t"
#define __FDK_vst3_ia_align(size, src1, src2, src3, dst, align)        "VST3." #size " {" #src1 ", " #src2 ", " #src3 "}, [" #dst ",:" #align "]! \n\t"
#define __FDK_vst3_pu_align(size, src1, src2, src3, dst, align, step)  "VST3." #size " {" #src1 ", " #src2 ", " #src3 "}, [" #dst ",:" #align "], " #step " \n\t"
#define FDK_vst3(   size, src1, src2, src3, dst)                     __FDK_vst3(   size, src1, src2, src3, dst)
#define FDK_vst3_ia(size, src1, src2, src3, dst)                     __FDK_vst3_ia(size, src1, src2, src3, dst)
#define FDK_vst3_pu(size, src1, src2, src3, dst, step)               __FDK_vst3_pu(size, src1, src2, src3, dst, step)
#define FDK_vst3_align(   size, src1, src2, src3, dst, align)        __FDK_vst3_align(   size, src1, src2, src3, dst, align)
#define FDK_vst3_ia_align(size, src1, src2, src3, dst, align)        __FDK_vst3_ia_align(size, src1, src2, src3, dst, align)
#define FDK_vst3_pu_align(size, src1, src2, src3, dst, align, step)  __FDK_vst3_pu_align(size, src1, src2, src3, dst, align, step)
#else
#define FDK_vst3(   size, src1, src2, src3, dst)                      { if (size == 32) { FDK_vst3_32(src1, src2, src3, dst); } else if (size == 16) { FDK_vst3_16((FIXP_S) src1,(FIXP_S) src2,(FIXP_S) src3,(FIXP_S) dst); } }
#define FDK_vst3_ia(size, src1, src2, src3, dst)                      { if (size == 32) { FDK_vst3_32(src1, src2, src3, dst); } else if (size == 16) { FDK_vst3_16((FIXP_S) src1,(FIXP_S) src2,(FIXP_S) src3,(FIXP_S) dst); } dst += 3*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vst3_pu(size, src1, src2, src3, dst, step)                { if (size == 32) { FDK_vst3_32(src1, src2, src3, dst); } else if (size == 16) { FDK_vst3_16((FIXP_S) src1,(FIXP_S) src2,(FIXP_S) src3,(FIXP_S) dst); } dst += step/(INT)sizeof(*dst); }
#define FDK_vst3_align(   size, src1, src2, src3, dst, align)         FDK_vst3_align(   size, src1, src2, src3, dst)
#define FDK_vst3_ia_align(size, src1, src2, src3, dst, align)         FDK_vst3_ia_align(size, src1, src2, src3, dst)
#define FDK_vst3_pu_align(size, src1, src2, src3, dst, align, step)   FDK_vst3_pu_align(size, src1, src2, src3, dst, step)

static void inline FDK_vst3_32(FIXP_Q src1, FIXP_Q src2, FIXP_Q src3, FIXP_DBL *dst)
{
  FDK_ASSERT(((src2==src1+1*2) && (src3==src1+2*2)) || ((src2==src1+2*2) && (src3==src1+4*2)));
  dst[0] = src1[0];
  dst[1] = src2[0];
  dst[2] = src3[0];
}

static void inline FDK_vst3_16(FIXP_S src1, FIXP_S src2, FIXP_S src3, FIXP_SGL *dst)
{
  FDK_ASSERT(((src2==src1+1*4) && (src3==src1+2*4)) || ((src2==src1+2*4) && (src3==src1+4*4)));
  dst[0] = src1[0];
  dst[1] = src2[0];
  dst[2] = src3[0];
}

#endif




/*
   Description: Load 4 16/32-bit lanes of 4 D registers from memory (src)
   Parameter size: 16 or 32
   Parameter dst1: Dw[i] 64-bit NEON register
   Parameter dst2: Dx[i] 64-bit NEON register
   Parameter dst3: Dy[i] 64-bit NEON register
   Parameter dst4: Dz[i] 64-bit NEON register (x,y,z must be w+1,w+2,w+3 or  w+2,w+4,w+6 i in [0,1]))
   Parameter src:  r core register used as a pointer to 16/32-bit data
   Parameter step: r core register indicating number of bytes for post-incrementing the dst pointer
*/

#ifdef __ARM_NEON__
#define __FDK_vld4(   size, dst1, dst2, dst3, dst4, src)                   "VLD4." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src "] \n\t"
#define __FDK_vld4_ia(size, dst1, dst2, dst3, dst4, src)                   "VLD4." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src "]! \n\t"
#define __FDK_vld4_pu(size, dst1, dst2, dst3, dst4, src, Rm)               "VLD4." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src "], " #Rm " \n\t"
#define __FDK_vld4_align(   size, dst1, dst2, dst3, dst4, src, align)      "VLD4." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src ",:" #align "] \n\t"
#define __FDK_vld4_ia_align(size, dst1, dst2, dst3, dst4, src, align)      "VLD4." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src ",:" #align "]! \n\t"
#define __FDK_vld4_pu_align(size, dst1, dst2, dst3, dst4, src, align, Rm)  "VLD4." #size " {" #dst1 "," #dst2 "," #dst3 "," #dst4 "}, [" #src ",:" #align "], " #Rm " \n\t"
#define FDK_vld4(   size, dst1, dst2, dst3, dst4, src)      __FDK_vld4(   size, dst1, dst2, dst3, dst4, src)
#define FDK_vld4_ia(size, dst1, dst2, dst3, dst4, src)      __FDK_vld4_ia(size, dst1, dst2, dst3, dst4, src)
#define FDK_vld4_pu(size, dst1, dst2, dst3, dst4, src, Rm)  __FDK_vld4_pu(size, dst1, dst2, dst3, dst4, src, Rm)
#define FDK_vld4_align(   size, dst1, dst2, dst3, dst4, src, align)      __FDK_vld4_align(   size, dst1, dst2, dst3, dst4, src, align)
#define FDK_vld4_ia_align(size, dst1, dst2, dst3, dst4, src, align)      __FDK_vld4_ia_align(size, dst1, dst2, dst3, dst4, src, align)
#define FDK_vld4_pu_align(size, dst1, dst2, dst3, dst4, src, align, Rm)  __FDK_vld4_pu_align(size, dst1, dst2, dst3, dst4, src, align, Rm)
#else
#define FDK_vld4(   size, dst1, dst2, dst3, dst4, src)         { if (size == 32) { FDK_vld4_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D)dst4,(FIXP_D) src); } else if (size == 16) { FDK_vld4_16((FIXP_S) dst1,(FIXP_S) dst2,(FIXP_S) dst3,(FIXP_S) dst4,(FIXP_S) src); } }
#define FDK_vld4_ia(size, dst1, dst2, dst3, dst4, src)         { if (size == 32) { FDK_vld4_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D)dst4,(FIXP_D) src); } else if (size == 16) { FDK_vld4_16((FIXP_S) dst1,(FIXP_S) dst2,(FIXP_S) dst3,(FIXP_S) dst4,(FIXP_S) src); } src += 4*(size>>3)/(INT)sizeof(*src);  }
#define FDK_vld4_pu(size, dst1, dst2, dst3, dst4, src, step)   { if (size == 32) { FDK_vld4_32((FIXP_D)dst1,(FIXP_D)dst2,(FIXP_D)dst3,(FIXP_D)dst4,(FIXP_D) src); } else if (size == 16) { FDK_vld4_16((FIXP_S) dst1,(FIXP_S) dst2,(FIXP_S) dst3,(FIXP_S) dst4,(FIXP_S) src); } src += step/(INT)sizeof(*src); }
#define FDK_vld4_align(   size, dst1, dst2, dst3, dst4, src, align)         FDK_vld4(   size, dst1, dst2, dst3, dst4, src)
#define FDK_vld4_ia_align(size, dst1, dst2, dst3, dst4, src, align)         FDK_vld4_ia(size, dst1, dst2, dst3, dst4, src)
#define FDK_vld4_pu_align(size, dst1, dst2, dst3, dst4, src, align, step)   FDK_vld4_pu(size, dst1, dst2, dst3, dst4, src, step)

static void inline FDK_vld4_32(FIXP_Q dst1, FIXP_Q dst2, FIXP_Q dst3, FIXP_Q dst4, FIXP_DBL *src)
{
  FDK_ASSERT(((dst2==dst1+1*2) && (dst3==dst1+2*2) && (dst4==dst1+3*2)) || ((dst2==dst1+2*2) && (dst3==dst1+4*2) && (dst4==dst1+6*2)));
  dst1[0] = src[0];
  dst2[0] = src[1];
  dst3[0] = src[2];
  dst4[0] = src[3];
}

static void inline FDK_vld4_16(FIXP_S dst1, FIXP_S dst2, FIXP_S dst3, FIXP_S dst4, FIXP_SGL *src)
{
  FDK_ASSERT(((dst2==dst1+1*4) && (dst3==dst1+2*4) && (dst4==dst1+3*4)) || ((dst2==dst1+2*4) && (dst3==dst1+4*4) && (dst4==dst1+6*4)));
  dst1[0] = src[0];
  dst2[0] = src[1];
  dst3[0] = src[2];
  dst4[0] = src[3];
}
#endif

/*
   Description: Store 4 16/32-bit lanes of 4 D registers into memory (dst)
   Parameter size: 16 or 32
   Parameter src1: Dw[i] 64-bit NEON register (x,y,z must be w+1,w+2,w+3 or  w+2,w+4,w+6 i in [0,1]))
   Parameter src2: Dx[i] 64-bit NEON register (x,y,z must be w+1,w+2,w+3 or  w+2,w+4,w+6 i in [0,1]))
   Parameter src3: Dy[i] 64-bit NEON register (x,y,z must be w+1,w+2,w+3 or  w+2,w+4,w+6 i in [0,1]))
   Parameter src4: Dz[i] 64-bit NEON register (x,y,z must be w+1,w+2,w+3 or  w+2,w+4,w+6 i in [0,1]))
   Parameter dst:  r core register used as a pointer to 16/32-bit data
   Parameter step: r core register indicating number of bytes for post-incrementing the dst pointer
*/

#ifdef __ARM_NEON__
#define __FDK_vst4(   size, src1, src2, src3, src4, dst)                   "VST4." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst "] \n\t"
#define __FDK_vst4_ia(size, src1, src2, src3, src4, dst)                   "VST4." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst "]! \n\t"
#define __FDK_vst4_pu(size, src1, src2, src3, src4, dst, Rm)               "VST4." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst "], " #Rm " \n\t"
#define __FDK_vst4_align(   size, src1, src2, src3, src4, dst, align)      "VST4." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst ",:" #align "] \n\t"
#define __FDK_vst4_ia_align(size, src1, src2, src3, src4, dst, align)      "VST4." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst ",:" #align "]! \n\t"
#define __FDK_vst4_pu_align(size, src1, src2, src3, src4, dst, align, Rm)  "VST4." #size " {" #src1 ", " #src2 ", " #src3 ", " #src4 "}, [" #dst ",:" #align "], " #Rm " \n\t"
#define FDK_vst4(   size, src1, src2, src3, src4, dst)        __FDK_vst4(   size, src1, src2, src3, src4, dst)
#define FDK_vst4_ia(size, src1, src2, src3, src4, dst)        __FDK_vst4_ia(size, src1, src2, src3, src4, dst)
#define FDK_vst4_pu(size, src1, src2, src3, src4, dst, Rm)    __FDK_vst4_pu(size, src1, src2, src3, src4, dst, Rm)
#define FDK_vst4_align(   size, src1, src2, src3, src4, dst, align)       __FDK_vst4_align(   size, src1, src2, src3, src4, dst, align)
#define FDK_vst4_ia_align(size, src1, src2, src3, src4, dst, align)       __FDK_vst4_ia_align(size, src1, src2, src3, src4, dst, align)
#define FDK_vst4_pu_align(size, src1, src2, src3, src4, dst, align, Rm)   __FDK_vst4_pu_align(size, src1, src2, src3, src4, dst, align, Rm)
#else
#define FDK_vst4   (size, src1, src2, src3, src4, dst)         { if (size == 32) { FDK_vst4_32((FIXP_D) src1, (FIXP_D) src2, (FIXP_D) src3, (FIXP_D) src4, dst); } else if (size == 16) { FDK_vst4_16((FIXP_S) src1,(FIXP_S) src2,(FIXP_S) src3,(FIXP_S) src4,(FIXP_S) dst); } }
#define FDK_vst4_ia(size, src1, src2, src3, src4, dst)         { if (size == 32) { FDK_vst4_32((FIXP_D) src1, (FIXP_D) src2, (FIXP_D) src3, (FIXP_D) src4, dst); } else if (size == 16) { FDK_vst4_16((FIXP_S) src1,(FIXP_S) src2,(FIXP_S) src3,(FIXP_S) src4,(FIXP_S) dst); } dst += 4*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vst4_pu(size, src1, src2, src3, src4, dst, step)   { if (size == 32) { FDK_vst4_32((FIXP_D) src1, (FIXP_D) src2, (FIXP_D) src3, (FIXP_D) src4, dst); } else if (size == 16) { FDK_vst4_16((FIXP_S) src1,(FIXP_S) src2,(FIXP_S) src3,(FIXP_S) src4,(FIXP_S) dst); } dst += step/(INT)sizeof(*dst); }
#define FDK_vst4_align(   size, src1, src2, src3, src4, dst, align)       FDK_vst4(   size, src1, src2, src3, src4, dst)
#define FDK_vst4_ia_align(size, src1, src2, src3, src4, dst, align)       FDK_vst4_ia(size, src1, src2, src3, src4, dst)
#define FDK_vst4_pu_align(size, src1, src2, src3, src4, dst, align, Rm)   FDK_vst4_pu(size, src1, src2, src3, src4, dst, Rm)

static void inline FDK_vst4_32(const FIXP_Q src1, const FIXP_Q src2, const FIXP_Q src3, const FIXP_Q src4, FIXP_DBL *dst)
{
  FDK_ASSERT(((src2==src1+1*2) && (src3==src1+2*2) && (src4==src1+3*2)) || ((src2==src1+2*2) && (src3==src1+4*2) && (src4==src1+6*2)));
  dst[0] = src1[0];
  dst[1] = src2[0];
  dst[2] = src3[0];
  dst[3] = src4[0];
}

static void inline FDK_vst4_16(FIXP_S src1, FIXP_S src2, FIXP_S src3, FIXP_S src4, FIXP_SGL *dst)
{
  FDK_ASSERT(((src2==src1+1*4) && (src3==src1+2*4) && (src4==src1+3*4)) || ((src2==src1+2*4) && (src3==src1+4*4) && (src4==src1+6*4)));
  dst[0] = src1[0];
  dst[1] = src2[0];
  dst[2] = src3[0];
  dst[3] = src4[0];
}

#endif

/*
   Description: Load one D registers from memory (src)
   Parameter size:   64, (32 also for VFP)
   Parameter dst:    64-bit NEON register Di, i in [0..31], (VFP only: 32-bit NEON register Si, i in [0..63]
   Parameter src:    r core register used as a pointer to 32-bit data
   Parameter offset: immediate offset to be applied to pointer src, must be a multiply of 4 in range [-1020,+1020]
*/

#ifdef __ARM_NEON__
#define FDK_vldr(size, dst, src, offset)            "VLDR." #size " " #dst ", [" #src ", #" #offset "] \n\t"
#define FDK_vldr_sp_64(dst, offset, name1, name2)   "VLDR.64 " #dst ", [sp, #" #offset "] \n\t"
#else
#define FDK_vldr(   size, dst, src, offset)         { if (size == 64) { FDK_vldr_64((FIXP_H) dst,(FIXP_D) src, offset); } else if (size == 32) { FDK_vldr_32((FIXP_D) dst,(FIXP_D) src, offset); } }
#define FDK_vldr_sp_64(dst, offset, name1, name2)     __FDK_vldr_sp_64((FIXP_H) dst, (FIXP_D)&name1, (FIXP_D)&name2);

static void inline FDK_vldr_64(FIXP_H dst, FIXP_DBL *src, int offset)
{
  FIXP_D D = (FIXP_D) dst;
  FDK_ASSERT(((offset & 3) == 0) && (offset >= -1020) && (offset <= 1020));
  FDK_ASSERT (((UINT64) src & 3) == 0);
  D[0] = ((FIXP_D) src)[(offset>>2)];
  D[1] = ((FIXP_D) src)[(offset>>2) + 1];

}

static void inline FDK_vldr_32(FIXP_D dst, FIXP_DBL *src, int offset)
{
  FIXP_D D = (FIXP_D) dst;
  FDK_ASSERT(((offset & 3) == 0) && (offset >= -1020) && (offset <= 1020));
  FDK_ASSERT (((UINT64) src & 3) == 0);
  D[0] = ((FIXP_D) src)[offset>>2];
}

static void inline __FDK_vldr_sp_64(FIXP_H dst, const FIXP_DBL *src1, const FIXP_DBL *src2)
{
  FIXP_D D = (FIXP_D) dst;
  FDK_ASSERT (((UINT64) src1 & 3) == 0);
  FDK_ASSERT (((UINT64) src2 & 3) == 0);
  D[0] = ((const FIXP_D) src1)[0];
  D[1] = ((const FIXP_D) src2)[0];
}
#endif

/*
   Description: Store one D registers into memory (dst)
   Parameter size:   64, (32 also for VFP)
   Parameter src:    64-bit NEON register Di, i in [0..31], (VFP only: 32-bit NEON register Si, i in [0..63]
   Parameter dst:    r core register used as a pointer to 32-bit data
   Parameter offset: immediate offset to be applied to pointer src, must be a multiply of 4 in range [-1020,+1020]
*/


#ifdef __ARM_NEON__
#define FDK_vstr(size, src, dst, offset)  "VSTR." #size " " #src ", [" #dst ", #" #offset "] \n\t"
#else
#define FDK_vstr(size, src, dst, offset)   { if (size == 64) { FDK_vstr_64((FIXP_H) src, dst, offset); } else if (size == 32) { FDK_vstr_32((FIXP_D) src,(FIXP_D) dst, offset); } }

static void inline FDK_vstr_64(FIXP_H src, FIXP_DBL *dst, int offset)
{
  FIXP_D D = (FIXP_D) src;
  FDK_ASSERT(((offset & 3) == 0) && (offset >= -1020) && (offset <= 1020));
  FDK_ASSERT (((UINT64) src & 3) == 0);
  dst[(offset>>2)+0] = D[0];
  dst[(offset>>2)+1] = D[1];
}

static void inline FDK_vstr_32(const FIXP_D src, FIXP_DBL *dst, int offset)
{
  FIXP_D D = (FIXP_D) src;
  FDK_ASSERT(((offset & 3) == 0) && (offset >= -1020) && (offset <= 1020));
  FDK_ASSERT (((UINT64) src & 3) == 0);
  dst[(offset>>2)+0] = D[0];
}
#endif

#ifdef __ARM_NEON__
#define FDK_mvpush(first_src, last_src)                            "VPUSH { " #first_src "-" #last_src " } \n\t"
#define FDK_vpush(src)                                             "VPUSH { " #src " } \n\t"
#else
#define FDK_mvpush(first_src, last_src)                            ;
#define FDK_vpush(src)                                             ;
#endif

#ifdef __ARM_NEON__
#define FDK_mvpop(first_src, last_src)                             "VPOP { " #first_src "-" #last_src " } \n\t"
#define FDK_vpop(src)                                              "VPOP { " #src " } \n\t"
#else
#define FDK_mvpop(first_src, last_src)                             ;
#define FDK_vpop(src)                                              ;
#endif


#ifdef __ARM_NEON__
#define FDK_vldm1(size, dst1,                                              src)  "VLDM " #src ", { " #dst1 " } \n\t"
#define FDK_vldm2(size, dst1, dst2,                                        src)  "VLDM " #src ", { " #dst1 ", " #dst2 " } \n\t"
#define FDK_vldm3(size, dst1, dst2, dst3,                                  src)  "VLDM " #src ", { " #dst1 ", " #dst2 ", " #dst3 " } \n\t"
#define FDK_vldm4(size, dst1, dst2, dst3, dst4,                            src)  "VLDM " #src ", { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " } \n\t"
#define FDK_vldm5(size, dst1, dst2, dst3, dst4, dst5,                      src)  "VLDM " #src ", { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 " } \n\t"
#define FDK_vldm6(size, dst1, dst2, dst3, dst4, dst5, dst6,                src)  "VLDM " #src ", { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 " } \n\t"
#define FDK_vldm7(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7,          src)  "VLDM " #src ", { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 " } \n\t"
#define FDK_vldm8(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8,    src)  "VLDM " #src ", { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 " } \n\t"
#define FDK_vldm12(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, src)                              "VLDM " #src ", { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 ", " #dst9 ", " #dst10 ", " #dst11 ", " #dst12 " } \n\t"
#define FDK_vldm16(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15, dst16, src)  "VLDM " #src ", { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 ", " #dst9 ", " #dst10 ", " #dst11 ", " #dst12 ", " #dst13 ", " #dst14 ", " #dst15 ", " #dst16 " } \n\t"
#define FDK_vldm1_ia(size, dst1,                                           src)  "VLDMIA " #src "!, { " #dst1 " } \n\t"
#define FDK_vldm2_ia(size, dst1, dst2,                                     src)  "VLDMIA " #src "!, { " #dst1 ", " #dst2 " } \n\t"
#define FDK_vldm3_ia(size, dst1, dst2, dst3,                               src)  "VLDMIA " #src "!, { " #dst1 ", " #dst2 ", " #dst3 " } \n\t"
#define FDK_vldm4_ia(size, dst1, dst2, dst3, dst4,                         src)  "VLDMIA " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " } \n\t"
#define FDK_vldm5_ia(size, dst1, dst2, dst3, dst4, dst5,                   src)  "VLDMIA " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 " } \n\t"
#define FDK_vldm6_ia(size, dst1, dst2, dst3, dst4, dst5, dst6,             src)  "VLDMIA " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 " } \n\t"
#define FDK_vldm7_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7,       src)  "VLDMIA " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 " } \n\t"
#define FDK_vldm8_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, src)  "VLDMIA " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 " } \n\t"
#define FDK_vldm12_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, src)                              "VLDMIA " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 ", " #dst9 ", " #dst10 ", " #dst11 ", " #dst12 " } \n\t"
#define FDK_vldm16_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15, dst16, src)  "VLDMIA " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 ", " #dst9 ", " #dst10 ", " #dst11 ", " #dst12 ", " #dst13 ", " #dst14 ", " #dst15 ", " #dst16 " } \n\t"
#define FDK_vldm1_db(size, dst1,                                           src)  "VLDMDB " #src "!, { " #dst1 " } \n\t"
#define FDK_vldm2_db(size, dst1, dst2,                                     src)  "VLDMDB " #src "!, { " #dst1 ", " #dst2 " } \n\t"
#define FDK_vldm3_db(size, dst1, dst2, dst3,                               src)  "VLDMDB " #src "!, { " #dst1 ", " #dst2 ", " #dst3 " } \n\t"
#define FDK_vldm4_db(size, dst1, dst2, dst3, dst4,                         src)  "VLDMDB " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " } \n\t"
#define FDK_vldm5_db(size, dst1, dst2, dst3, dst4, dst5,                   src)  "VLDMDB " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 " } \n\t"
#define FDK_vldm6_db(size, dst1, dst2, dst3, dst4, dst5, dst6,             src)  "VLDMDB " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 " } \n\t"
#define FDK_vldm7_db(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7,       src)  "VLDMDB " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 " } \n\t"
#define FDK_vldm8_db(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, src)  "VLDMDB " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 " } \n\t"
#define FDK_vldm12_db(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, src)                              "VLDMDB " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 ", " #dst9 ", " #dst10 ", " #dst11 ", " #dst12 " } \n\t"
#define FDK_vldm16_db(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15, dst16, src)  "VLDMDB " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 ", " #dst9 ", " #dst10 ", " #dst11 ", " #dst12 ", " #dst13 ", " #dst14 ", " #dst15 ", " #dst16 " } \n\t"
#define FDK_vldm1_cond_ia(cond, size, dst1,                                           src)  "VLDMIA" #cond " " #src "!, { " #dst1 " } \n\t"
#define FDK_vldm2_cond_ia(cond, size, dst1, dst2,                                     src)  "VLDMIA" #cond " " #src "!, { " #dst1 ", " #dst2 " } \n\t"
#define FDK_vldm3_cond_ia(cond, size, dst1, dst2, dst3,                               src)  "VLDMIA" #cond " " #src "!, { " #dst1 ", " #dst2 ", " #dst3 " } \n\t"
#define FDK_vldm4_cond_ia(cond, size, dst1, dst2, dst3, dst4,                         src)  "VLDMIA" #cond " " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 " } \n\t"
#define FDK_vldm5_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5,                   src)  "VLDMIA" #cond " " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 " } \n\t"
#define FDK_vldm6_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5, dst6,             src)  "VLDMIA" #cond " " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 " } \n\t"
#define FDK_vldm7_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5, dst6, dst7,       src)  "VLDMIA" #cond " " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 " } \n\t"
#define FDK_vldm8_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, src)  "VLDMIA" #cond " " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 " } \n\t"
#define FDK_vldm12_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, src)  "VLDMIA" #cond " " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 ", " #dst9 ", " #dst10 ", " #dst11 ", " #dst12 " } \n\t"
#define FDK_vldm16_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15, dst16, src)  "VLDMIA" #cond " " #src "!, { " #dst1 ", " #dst2 ", " #dst3 ", " #dst4 ", " #dst5 ", " #dst6 ", " #dst7 ", " #dst8 ", " #dst9 ", " #dst10 ", " #dst11 ", " #dst12 ", " #dst13 ", " #dst14 ", " #dst15 ", " #dst16 " } \n\t"
#else
#define FDK_vldm1(size, dst1, src)    \
            { if      (size == 128) { FDK_vldm1_128((FIXP_Q) dst1,(FIXP_D) src); }   \
              else if (size ==  64) { FDK_vldm1_64 ((FIXP_H) dst1,(FIXP_D) src); }   \
              else if (size ==  32) { FDK_vldm1_32 ((FIXP_D) dst1,(FIXP_D) src); }   \
            }
#define FDK_vldm1_ia(size, dst1, src) { FDK_vldm1(size, dst1, src);  src += (size>>3)/(INT)sizeof(*src); }
#define FDK_vldm1_db(size, dst1, src) { src -= (size>>3)/(INT)sizeof(*src);  FDK_vldm1(size, dst1, src); }
#define FDK_vldm1_cond_ia(cond, size, dst1, src) { if (__FDK_coreflags_ ## cond) { FDK_vldm1_ia(size, dst1, src); } }

#define FDK_vldm2(size, dst1, dst2, src)    \
            { if      (size == 128) { FDK_vldm2_128((FIXP_Q) dst1,(FIXP_Q) dst2,(FIXP_D) src); }   \
              else if (size ==  64) { FDK_vldm2_64 ((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_D) src); }   \
              else if (size ==  32) { FDK_vldm2_32 ((FIXP_D) dst1,(FIXP_D) dst2,(FIXP_D) src); }     \
            }
#define FDK_vldm2_ia(size, dst1, dst2, src) { FDK_vldm2(size, dst1, dst2, src);  src += 2*(size>>3)/(INT)sizeof(*src); }
#define FDK_vldm2_db(size, dst1, dst2, src) { src -= 2*(size>>3)/(INT)sizeof(*src);  FDK_vldm2(size, dst1, dst2, src); }
#define FDK_vldm2_cond_ia(cond, size, dst1, dst2, src) { if (__FDK_coreflags_ ## cond) { FDK_vldm2_ia(size, dst1, dst2, src); } }

#define FDK_vldm3(size, dst1, dst2, dst3, src)    \
            { if      (size == 128) { FDK_vldm3_128((FIXP_Q) dst1,(FIXP_Q) dst2,(FIXP_Q) dst3,(FIXP_D) src); }   \
              else if (size ==  64) { FDK_vldm3_64 ((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_H) dst3,(FIXP_D) src); }   \
              else if (size ==  32) { FDK_vldm3_32 ((FIXP_D) dst1,(FIXP_D) dst2,(FIXP_D) dst3,(FIXP_D) src); }      \
            }
#define FDK_vldm3_ia(size, dst1, dst2, dst3, src) { FDK_vldm3(size, dst1, dst2, dst3, src);  src += 3*(size>>3)/(INT)sizeof(*src); }
#define FDK_vldm3_db(size, dst1, dst2, dst3, src) { src -= 3*(size>>3)/(INT)sizeof(*src);  FDK_vldm3(size, dst1, dst2, dst3, src); }
#define FDK_vldm3_cond_ia(cond, size, dst1, dst2, dst3, src) { if (__FDK_coreflags_ ## cond) { FDK_vldm3_ia(size, dst1, dst2, dst3, src); } }

#define FDK_vldm4(size, dst1, dst2, dst3, dst4, src)    \
            { if      (size == 128) { FDK_vldm4_128((FIXP_Q) dst1,(FIXP_Q) dst2,(FIXP_Q) dst3,(FIXP_Q) dst4,(FIXP_D) src); }   \
              else if (size ==  64) { FDK_vldm4_64 ((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_H) dst3,(FIXP_H) dst4,(FIXP_D) src); }   \
              else if (size ==  32) { FDK_vldm4_32 ((FIXP_D) dst1,(FIXP_D) dst2,(FIXP_D) dst3,(FIXP_Q) dst4,(FIXP_D) src); }       \
            }
#define FDK_vldm4_ia(size, dst1, dst2, dst3, dst4, src) { FDK_vldm4(size, dst1, dst2, dst3, dst4, src);  src += 4*(size>>3)/(INT)sizeof(*src); }
#define FDK_vldm4_db(size, dst1, dst2, dst3, dst4, src) { src -= 4*(size>>3)/(INT)sizeof(*src);  FDK_vldm4(size, dst1, dst2, dst3, dst4, src); }
#define FDK_vldm4_cond_ia(cond, size, dst1, dst2, dst3, dst4, src) { if (__FDK_coreflags_ ## cond) { FDK_vldm4_ia(size, dst1, dst2, dst3, dst4, src); } }

#define FDK_vldm5(size, dst1, dst2, dst3, dst4, dst5, src)    \
            { if      (size == 128) { FDK_vldm5_128((FIXP_Q) dst1,(FIXP_Q) dst2,(FIXP_Q) dst3,(FIXP_Q) dst4,(FIXP_Q) dst5,(FIXP_D) src); }   \
              else if (size ==  64) { FDK_vldm5_64 ((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_H) dst3,(FIXP_H) dst4,(FIXP_H) dst5,(FIXP_D) src); }   \
              else if (size ==  32) { FDK_vldm5_32 ((FIXP_D) dst1,(FIXP_D) dst2,(FIXP_D) dst3,(FIXP_D) dst4,(FIXP_D) dst5,(FIXP_D) src); }        \
            }
#define FDK_vldm5_ia(size, dst1, dst2, dst3, dst4, dst5, src) { FDK_vldm5(size, dst1, dst2, dst3, dst4, dst5, src);  src += 5*(size>>3)/(INT)sizeof(*src); }
#define FDK_vldm5_db(size, dst1, dst2, dst3, dst4, dst5, src) { src -= 5*(size>>3)/(INT)sizeof(*src);  FDK_vldm5(size, dst1, dst2, dst3, dst4, dst5, src); }
#define FDK_vldm5_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5, src) { if (__FDK_coreflags_ ## cond) { FDK_vldm5_ia(size, dst1, dst2, dst3, dst4, dst5, src); } }

#define FDK_vldm6(size, dst1, dst2, dst3, dst4, dst5, dst6, src)    \
            { if      (size == 128) { FDK_vldm6_128((FIXP_Q) dst1,(FIXP_Q) dst2,(FIXP_Q) dst3,(FIXP_Q) dst4,(FIXP_Q) dst5,(FIXP_Q) dst6,(FIXP_D) src); }   \
              else if (size ==  64) { FDK_vldm6_64 ((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_H) dst3,(FIXP_H) dst4,(FIXP_H) dst5,(FIXP_H) dst6,(FIXP_D) src); }   \
              else if (size ==  32) { FDK_vldm6_32 ((FIXP_D) dst1,(FIXP_D) dst2,(FIXP_D) dst3,(FIXP_D) dst4,(FIXP_D) dst5,(FIXP_D) dst6,(FIXP_D) src); }         \
            }
#define FDK_vldm6_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, src) { FDK_vldm6(size, dst1, dst2, dst3, dst4, dst5, dst6, src);  src += 6*(size>>3)/(INT)sizeof(*src); }
#define FDK_vldm6_db(size, dst1, dst2, dst3, dst4, dst5, dst6, src) { src -= 6*(size>>3)/(INT)sizeof(*src);  FDK_vldm6(size, dst1, dst2, dst3, dst4, dst5, dst6, src); }
#define FDK_vldm6_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5, dst6, src) { if (__FDK_coreflags_ ## cond) { FDK_vldm6_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, src); } }

#define FDK_vldm7(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, src)    \
            { if      (size == 128) { FDK_vldm7_128((FIXP_Q) dst1,(FIXP_Q) dst2,(FIXP_Q) dst3,(FIXP_Q) dst4,(FIXP_Q) dst5,(FIXP_Q) dst6,(FIXP_Q) dst7,(FIXP_D) src); }   \
              else if (size ==  64) { FDK_vldm7_64 ((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_H) dst3,(FIXP_H) dst4,(FIXP_H) dst5,(FIXP_H) dst6,(FIXP_H) dst7,(FIXP_D) src); }   \
              else if (size ==  32) { FDK_vldm7_32 ((FIXP_D) dst1,(FIXP_D) dst2,(FIXP_D) dst3,(FIXP_D) dst4,(FIXP_D) dst5,(FIXP_D) dst6,(FIXP_D) dst7,(FIXP_D) src); }   \
            }
#define FDK_vldm7_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, src) { FDK_vldm7(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, src);  src += 7*(size>>3)/(INT)sizeof(*src); }
#define FDK_vldm7_db(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, src) { src -= 7*(size>>3)/(INT)sizeof(*src);  FDK_vldm7(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, src); }
#define FDK_vldm7_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, src) { if (__FDK_coreflags_ ## cond) { FDK_vldm7_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, src); } }

#define FDK_vldm8(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, src)    \
            { if      (size == 128) { FDK_vldm8_128((FIXP_Q) dst1,(FIXP_Q) dst2,(FIXP_Q) dst3,(FIXP_Q) dst4,(FIXP_Q) dst5,(FIXP_Q) dst6,(FIXP_Q) dst7,(FIXP_Q) dst8,(FIXP_Q) src); }   \
              else if (size ==  64) { FDK_vldm8_64 ((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_H) dst3,(FIXP_H) dst4,(FIXP_H) dst5,(FIXP_H) dst6,(FIXP_H) dst7,(FIXP_H) dst8,(FIXP_H) src); }   \
              else if (size ==  32) { FDK_vldm8_32 ((FIXP_D) dst1,(FIXP_D) dst2,(FIXP_D) dst3,(FIXP_D) dst4,(FIXP_D) dst5,(FIXP_D) dst6,(FIXP_D) dst7,(FIXP_D) dst8,(FIXP_D) src); }   \
            }
#define FDK_vldm8_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, src) { FDK_vldm8(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, src);  src += 8*(size>>3)/(INT)sizeof(*src); }
#define FDK_vldm8_db(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, src) { src -= 8*(size>>3)/(INT)sizeof(*src);  FDK_vldm8(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, src); }
#define FDK_vldm8_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, src) { if (__FDK_coreflags_ ## cond) { FDK_vldm8_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, src); } }

#define FDK_vldm12(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, src)    \
            { if      (size ==  64) { FDK_vldm12_64 ((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_H) dst3,(FIXP_H) dst4,(FIXP_H) dst5,(FIXP_H) dst6,(FIXP_H) dst7,(FIXP_H) dst8,(FIXP_H) dst9,(FIXP_H) dst10,(FIXP_H) dst11,(FIXP_H) dst12,(FIXP_H) src); }   \
              else if (size ==  32) { FDK_vldm12_32 ((FIXP_D) dst1,(FIXP_D) dst2,(FIXP_D) dst3,(FIXP_D) dst4,(FIXP_D) dst5,(FIXP_D) dst6,(FIXP_D) dst7,(FIXP_D) dst8,(FIXP_D) dst9,(FIXP_D) dst10,(FIXP_D) dst11,(FIXP_D) dst12,(FIXP_D) src); }   \
            }
#define FDK_vldm12_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, src) { FDK_vldm12(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8,dst9, dst10, dst11, dst12, src);  src += 12*(size>>3)/(INT)sizeof(*src); }
#define FDK_vldm12_db(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, src) { src -= 12*(size>>3)/(INT)sizeof(*src);  FDK_vldm12(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8,dst9, dst10, dst11, dst12, src); }
#define FDK_vldm12_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, src) { if (__FDK_coreflags_ ## cond) { FDK_vldm12_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, src); } }

#define FDK_vldm16(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15, dst16, src)    \
            { if      (size ==  64) { FDK_vldm16_64 ((FIXP_H) dst1,(FIXP_H) dst2,(FIXP_H) dst3,(FIXP_H) dst4,(FIXP_H) dst5,(FIXP_H) dst6,(FIXP_H) dst7,(FIXP_H) dst8,(FIXP_H) dst9,(FIXP_H) dst10,(FIXP_H) dst11,(FIXP_H) dst12,(FIXP_H) dst13,(FIXP_H) dst14,(FIXP_H) dst15,(FIXP_H) dst16,(FIXP_H) src); }   \
              else if (size ==  32) { FDK_vldm16_32 ((FIXP_D) dst1,(FIXP_D) dst2,(FIXP_D) dst3,(FIXP_D) dst4,(FIXP_D) dst5,(FIXP_D) dst6,(FIXP_D) dst7,(FIXP_D) dst8,(FIXP_D) dst9,(FIXP_D) dst10,(FIXP_D) dst11,(FIXP_D) dst12,(FIXP_D) dst13,(FIXP_D) dst14,(FIXP_D) dst15,(FIXP_D) dst16,(FIXP_D) src); }   \
            }
#define FDK_vldm16_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15, dst16, src) { FDK_vldm16(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8,dst9, dst10, dst11, dst12, dst13, dst14, dst15, dst16, src);  src += 16*(size>>3)/(INT)sizeof(*src); }
#define FDK_vldm16_db(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15, dst16, src) { src -= 16*(size>>3)/(INT)sizeof(*src);  FDK_vldm16(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8,dst9, dst10, dst11, dst12, dst13, dst14, dst15, dst16, src); }
#define FDK_vldm16_cond_ia(cond, size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15, dst16, src) { if (__FDK_coreflags_ ## cond) { FDK_vldm16_ia(size, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15, dst16, src); } }

static void inline FDK_vldm1_128(FIXP_Q dst1, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 15) == 0);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];  D[2] = ((FIXP_D) src)[2];  D[3] = ((FIXP_D) src)[3];
}
static void inline FDK_vldm1_64(FIXP_H dst1, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 7) == 0);
  FIXP_D D;
  D = (FIXP_D) dst1;
  D[0] = src[0];
  D[1] = src[1];
}
static void inline FDK_vldm1_32(FIXP_D dst1, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 3) == 0);
  FIXP_D D;
  D = (FIXP_D) dst1;
  D[0] = ((FIXP_D) src)[0];
}

static void inline FDK_vldm2_128(FIXP_Q dst1, FIXP_Q dst2, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 15) == 0);
  FDK_ASSERT (((UINT64) dst2 & 15) == 0);
  FDK_ASSERT (dst2 == &dst1[4]);

  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];  D[2] = ((FIXP_D) src)[2];  D[3] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];  D[2] = ((FIXP_D) src)[6];  D[3] = ((FIXP_D) src)[7];
}
static void inline FDK_vldm2_64(FIXP_H dst1, FIXP_H dst2, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 7) == 0);
  FDK_ASSERT (((UINT64) dst2 & 7) == 0);
  FDK_ASSERT (dst2 == &dst1[2]);

  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[2];  D[1] = ((FIXP_D) src)[3];
}
static void inline FDK_vldm2_32(FIXP_D dst1, FIXP_D dst2, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 3) == 0);
  FDK_ASSERT (((UINT64) dst2 & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[1]);

  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[1];
}

static void inline FDK_vldm3_128(FIXP_Q dst1, FIXP_Q dst2, FIXP_Q dst3, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 15) == 0);
  FDK_ASSERT (((UINT64) dst2 & 15) == 0);
  FDK_ASSERT (((UINT64) dst3 & 15) == 0);
  FDK_ASSERT (dst2 == &dst1[4]);
  FDK_ASSERT (dst3 == &dst1[8]);

  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];  D[2] = ((FIXP_D) src)[2];  D[3] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];  D[2] = ((FIXP_D) src)[6];  D[3] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];  D[2] = ((FIXP_D) src)[10]; D[3] = ((FIXP_D) src)[11];
}
static void inline FDK_vldm3_64(FIXP_H dst1, FIXP_H dst2, FIXP_H dst3, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 7) == 0);
  FDK_ASSERT (((UINT64) dst2 & 7) == 0);
  FDK_ASSERT (((UINT64) dst3 & 7) == 0);
  FDK_ASSERT (dst2 == &dst1[2]);
  FDK_ASSERT (dst3 == &dst1[4]);

  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[2];  D[1] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];
}
static void inline FDK_vldm3_32(FIXP_D dst1, FIXP_D dst2, FIXP_D dst3, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 3) == 0);
  FDK_ASSERT (((UINT64) dst2 & 3) == 0);
  FDK_ASSERT (((UINT64) dst3 & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[1]);
  FDK_ASSERT (dst3 == &dst1[2]);

  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[2];
}

static void inline FDK_vldm4_128(FIXP_Q dst1, FIXP_Q dst2, FIXP_Q dst3, FIXP_Q dst4, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 15) == 0);
  FDK_ASSERT (((UINT64) dst2 & 15) == 0);
  FDK_ASSERT (((UINT64) dst3 & 15) == 0);
  FDK_ASSERT (((UINT64) dst4 & 15) == 0);
  FDK_ASSERT (dst2 == &dst1[4]);
  FDK_ASSERT (dst3 == &dst1[8]);
  FDK_ASSERT (dst4 == &dst1[12]);

  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];  D[2] = ((FIXP_D) src)[2];  D[3] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];  D[2] = ((FIXP_D) src)[6];  D[3] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];  D[2] = ((FIXP_D) src)[10]; D[3] = ((FIXP_D) src)[11];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[12]; D[1] = ((FIXP_D) src)[13]; D[2] = ((FIXP_D) src)[14]; D[3] = ((FIXP_D) src)[15];
}
static void inline FDK_vldm4_64(FIXP_H dst1, FIXP_H dst2, FIXP_H dst3, FIXP_H dst4, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 7) == 0);
  FDK_ASSERT (((UINT64) dst2 & 7) == 0);
  FDK_ASSERT (((UINT64) dst3 & 7) == 0);
  FDK_ASSERT (((UINT64) dst4 & 7) == 0);
  FDK_ASSERT (dst2 == &dst1[2]);
  FDK_ASSERT (dst3 == &dst1[4]);
  FDK_ASSERT (dst4 == &dst1[6]);

  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[2];  D[1] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[6];  D[1] = ((FIXP_D) src)[7];
}
static void inline FDK_vldm4_32(FIXP_D dst1, FIXP_D dst2, FIXP_D dst3, FIXP_D dst4, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (((UINT64) dst1 & 3) == 0);
  FDK_ASSERT (((UINT64) dst2 & 3) == 0);
  FDK_ASSERT (((UINT64) dst3 & 3) == 0);
  FDK_ASSERT (((UINT64) dst4 & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[1]);
  FDK_ASSERT (dst3 == &dst1[2]);
  FDK_ASSERT (dst4 == &dst1[3]);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[2];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[3];
}

static void inline FDK_vldm5_128(FIXP_Q dst1, FIXP_Q dst2, FIXP_Q dst3, FIXP_Q dst4, FIXP_Q dst5, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];  D[2] = ((FIXP_D) src)[2];  D[3] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];  D[2] = ((FIXP_D) src)[6];  D[3] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];  D[2] = ((FIXP_D) src)[10]; D[3] = ((FIXP_D) src)[11];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[12]; D[1] = ((FIXP_D) src)[13]; D[2] = ((FIXP_D) src)[14]; D[3] = ((FIXP_D) src)[15];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[16]; D[1] = ((FIXP_D) src)[17]; D[2] = ((FIXP_D) src)[18]; D[3] = ((FIXP_D) src)[19];
}
static void inline FDK_vldm5_64(FIXP_H dst1, FIXP_H dst2, FIXP_H dst3, FIXP_H dst4, FIXP_H dst5, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[2]);
  FDK_ASSERT (dst3 == &dst1[4]);
  FDK_ASSERT (dst4 == &dst1[6]);
  FDK_ASSERT (dst5 == &dst1[8]);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[2];  D[1] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[6];  D[1] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];
}
static void inline FDK_vldm5_32(FIXP_D dst1, FIXP_D dst2, FIXP_D dst3, FIXP_D dst4, FIXP_D dst5, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[1]);
  FDK_ASSERT (dst3 == &dst1[2]);
  FDK_ASSERT (dst4 == &dst1[3]);
  FDK_ASSERT (dst5 == &dst1[4]);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[2];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[4];
}

static void inline FDK_vldm6_128(FIXP_Q dst1, FIXP_Q dst2, FIXP_Q dst3, FIXP_Q dst4, FIXP_Q dst5, FIXP_Q dst6, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];  D[2] = ((FIXP_D) src)[2];  D[3] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];  D[2] = ((FIXP_D) src)[6];  D[3] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];  D[2] = ((FIXP_D) src)[10]; D[3] = ((FIXP_D) src)[11];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[12]; D[1] = ((FIXP_D) src)[13]; D[2] = ((FIXP_D) src)[14]; D[3] = ((FIXP_D) src)[15];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[16]; D[1] = ((FIXP_D) src)[17]; D[2] = ((FIXP_D) src)[18]; D[3] = ((FIXP_D) src)[19];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[20]; D[1] = ((FIXP_D) src)[21]; D[2] = ((FIXP_D) src)[22]; D[3] = ((FIXP_D) src)[23];
}
static void inline FDK_vldm6_64(FIXP_H dst1, FIXP_H dst2, FIXP_H dst3, FIXP_H dst4, FIXP_H dst5, FIXP_H dst6, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[2]);
  FDK_ASSERT (dst3 == &dst1[4]);
  FDK_ASSERT (dst4 == &dst1[6]);
  FDK_ASSERT (dst5 == &dst1[8]);
  FDK_ASSERT (dst6 == &dst1[10]);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[2];  D[1] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[6];  D[1] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[10]; D[1] = ((FIXP_D) src)[11];
}
static void inline FDK_vldm6_32(FIXP_D dst1, FIXP_D dst2, FIXP_D dst3, FIXP_D dst4, FIXP_D dst5, FIXP_D dst6, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[1]);
  FDK_ASSERT (dst3 == &dst1[2]);
  FDK_ASSERT (dst4 == &dst1[3]);
  FDK_ASSERT (dst5 == &dst1[4]);
  FDK_ASSERT (dst6 == &dst1[5]);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[2];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[4];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[5];
}

static void inline FDK_vldm7_128(FIXP_Q dst1, FIXP_Q dst2, FIXP_Q dst3, FIXP_Q dst4, FIXP_Q dst5, FIXP_Q dst6, FIXP_Q dst7, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];  D[2] = ((FIXP_D) src)[2];  D[3] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];  D[2] = ((FIXP_D) src)[6];  D[3] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];  D[2] = ((FIXP_D) src)[10]; D[3] = ((FIXP_D) src)[11];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[12]; D[1] = ((FIXP_D) src)[13]; D[2] = ((FIXP_D) src)[14]; D[3] = ((FIXP_D) src)[15];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[16]; D[1] = ((FIXP_D) src)[17]; D[2] = ((FIXP_D) src)[18]; D[3] = ((FIXP_D) src)[19];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[20]; D[1] = ((FIXP_D) src)[21]; D[2] = ((FIXP_D) src)[22]; D[3] = ((FIXP_D) src)[23];
  D = (FIXP_D) dst7;  D[0] = ((FIXP_D) src)[24]; D[1] = ((FIXP_D) src)[25]; D[2] = ((FIXP_D) src)[26]; D[3] = ((FIXP_D) src)[27];
}
static void inline FDK_vldm7_64(FIXP_H dst1, FIXP_H dst2, FIXP_H dst3, FIXP_H dst4, FIXP_H dst5, FIXP_H dst6, FIXP_H dst7, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[2]);
  FDK_ASSERT (dst3 == &dst1[4]);
  FDK_ASSERT (dst4 == &dst1[6]);
  FDK_ASSERT (dst5 == &dst1[8]);
  FDK_ASSERT (dst6 == &dst1[10]);
  FDK_ASSERT (dst7 == &dst1[12]);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[2];  D[1] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[6];  D[1] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[10]; D[1] = ((FIXP_D) src)[11];
  D = (FIXP_D) dst7;  D[0] = ((FIXP_D) src)[12]; D[1] = ((FIXP_D) src)[13];
}
static void inline FDK_vldm7_32(FIXP_D dst1, FIXP_D dst2, FIXP_D dst3, FIXP_D dst4, FIXP_D dst5, FIXP_D dst6, FIXP_D dst7, FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[1]);
  FDK_ASSERT (dst3 == &dst1[2]);
  FDK_ASSERT (dst4 == &dst1[3]);
  FDK_ASSERT (dst5 == &dst1[4]);
  FDK_ASSERT (dst6 == &dst1[5]);
  FDK_ASSERT (dst7 == &dst1[6]);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[2];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[4];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[5];
  D = (FIXP_D) dst7;  D[0] = ((FIXP_D) src)[6];
}

static void inline FDK_vldm8_128(
        FIXP_Q dst1, FIXP_Q dst2, FIXP_Q dst3, FIXP_Q dst4,
        FIXP_Q dst5, FIXP_Q dst6, FIXP_Q dst7, FIXP_Q dst8,
        FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];  D[2] = ((FIXP_D) src)[2];  D[3] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];  D[2] = ((FIXP_D) src)[6];  D[3] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];  D[2] = ((FIXP_D) src)[10]; D[3] = ((FIXP_D) src)[11];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[12]; D[1] = ((FIXP_D) src)[13]; D[2] = ((FIXP_D) src)[14]; D[3] = ((FIXP_D) src)[15];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[16]; D[1] = ((FIXP_D) src)[17]; D[2] = ((FIXP_D) src)[18]; D[3] = ((FIXP_D) src)[19];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[20]; D[1] = ((FIXP_D) src)[21]; D[2] = ((FIXP_D) src)[22]; D[3] = ((FIXP_D) src)[23];
  D = (FIXP_D) dst7;  D[0] = ((FIXP_D) src)[24]; D[1] = ((FIXP_D) src)[25]; D[2] = ((FIXP_D) src)[26]; D[3] = ((FIXP_D) src)[27];
  D = (FIXP_D) dst8;  D[0] = ((FIXP_D) src)[28]; D[1] = ((FIXP_D) src)[29]; D[2] = ((FIXP_D) src)[30]; D[3] = ((FIXP_D) src)[31];
}
static void inline FDK_vldm8_64(
        FIXP_H dst1, FIXP_H dst2, FIXP_H dst3, FIXP_H dst4,
        FIXP_H dst5, FIXP_H dst6, FIXP_H dst7, FIXP_H dst8,
        FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[2]);
  FDK_ASSERT (dst3 == &dst1[4]);
  FDK_ASSERT (dst4 == &dst1[6]);
  FDK_ASSERT (dst5 == &dst1[8]);
  FDK_ASSERT (dst6 == &dst1[10]);
  FDK_ASSERT (dst7 == &dst1[12]);
  FDK_ASSERT (dst8 == &dst1[14]);  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[2];  D[1] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[6];  D[1] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[10]; D[1] = ((FIXP_D) src)[11];
  D = (FIXP_D) dst7;  D[0] = ((FIXP_D) src)[12]; D[1] = ((FIXP_D) src)[13];
  D = (FIXP_D) dst8;  D[0] = ((FIXP_D) src)[14]; D[1] = ((FIXP_D) src)[15];
}
static void inline FDK_vldm8_32(
        FIXP_D dst1, FIXP_D dst2, FIXP_D dst3, FIXP_D dst4,
        FIXP_D dst5, FIXP_D dst6, FIXP_D dst7, FIXP_D dst8,
        FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[1]);
  FDK_ASSERT (dst3 == &dst1[2]);
  FDK_ASSERT (dst4 == &dst1[3]);
  FDK_ASSERT (dst5 == &dst1[4]);
  FDK_ASSERT (dst6 == &dst1[5]);
  FDK_ASSERT (dst7 == &dst1[6]);
  FDK_ASSERT (dst8 == &dst1[7]);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[2];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[4];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[5];
  D = (FIXP_D) dst7;  D[0] = ((FIXP_D) src)[6];
  D = (FIXP_D) dst8;  D[0] = ((FIXP_D) src)[7];
}

static void inline FDK_vldm12_64(
        FIXP_H dst1,  FIXP_H dst2,  FIXP_H dst3, FIXP_H dst4,
        FIXP_H dst5,  FIXP_H dst6,  FIXP_H dst7, FIXP_H dst8,
        FIXP_H dst9,  FIXP_H dst10, FIXP_H dst11, FIXP_H dst12,
        FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[2]);
  FDK_ASSERT (dst3 == &dst1[4]);
  FDK_ASSERT (dst4 == &dst1[6]);
  FDK_ASSERT (dst5 == &dst1[8]);
  FDK_ASSERT (dst6 == &dst1[10]);
  FDK_ASSERT (dst7 == &dst1[12]);
  FDK_ASSERT (dst8 == &dst1[14]);
  FDK_ASSERT (dst9 == &dst1[16]);
  FDK_ASSERT (dst10 == &dst1[18]);
  FDK_ASSERT (dst11 == &dst1[20]);
  FDK_ASSERT (dst12 == &dst1[22]);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[2];  D[1] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[6];  D[1] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[10]; D[1] = ((FIXP_D) src)[11];
  D = (FIXP_D) dst7;  D[0] = ((FIXP_D) src)[12]; D[1] = ((FIXP_D) src)[13];
  D = (FIXP_D) dst8;  D[0] = ((FIXP_D) src)[14]; D[1] = ((FIXP_D) src)[15];
  D = (FIXP_D) dst9;  D[0] = ((FIXP_D) src)[16]; D[1] = ((FIXP_D) src)[17];
  D = (FIXP_D) dst10; D[0] = ((FIXP_D) src)[18]; D[1] = ((FIXP_D) src)[19];
  D = (FIXP_D) dst11; D[0] = ((FIXP_D) src)[20]; D[1] = ((FIXP_D) src)[21];
  D = (FIXP_D) dst12; D[0] = ((FIXP_D) src)[22]; D[1] = ((FIXP_D) src)[23];
}
static void inline FDK_vldm12_32(
        FIXP_D dst1,  FIXP_D dst2,  FIXP_D dst3,  FIXP_D dst4,
        FIXP_D dst5,  FIXP_D dst6,  FIXP_D dst7,  FIXP_D dst8,
        FIXP_D dst9,  FIXP_D dst10, FIXP_D dst11, FIXP_D dst12,
        FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[1]);
  FDK_ASSERT (dst3 == &dst1[2]);
  FDK_ASSERT (dst4 == &dst1[3]);
  FDK_ASSERT (dst5 == &dst1[4]);
  FDK_ASSERT (dst6 == &dst1[5]);
  FDK_ASSERT (dst7 == &dst1[6]);
  FDK_ASSERT (dst8 == &dst1[7]);
  FDK_ASSERT (dst9 == &dst1[8]);
  FDK_ASSERT (dst10 == &dst1[9]);
  FDK_ASSERT (dst11 == &dst1[10]);
  FDK_ASSERT (dst12 == &dst1[11]);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[2];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[4];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[5];
  D = (FIXP_D) dst7;  D[0] = ((FIXP_D) src)[6];
  D = (FIXP_D) dst8;  D[0] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst9;  D[0] = ((FIXP_D) src)[8];
  D = (FIXP_D) dst10; D[0] = ((FIXP_D) src)[9];
  D = (FIXP_D) dst11; D[0] = ((FIXP_D) src)[10];
  D = (FIXP_D) dst12; D[0] = ((FIXP_D) src)[11];
}



static void inline FDK_vldm16_64(
        FIXP_H dst1,  FIXP_H dst2,  FIXP_H dst3, FIXP_H dst4,
        FIXP_H dst5,  FIXP_H dst6,  FIXP_H dst7, FIXP_H dst8,
        FIXP_H dst9,  FIXP_H dst10, FIXP_H dst11, FIXP_H dst12,
        FIXP_H dst13, FIXP_H dst14, FIXP_H dst15, FIXP_H dst16,
        FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[2]);
  FDK_ASSERT (dst3 == &dst1[4]);
  FDK_ASSERT (dst4 == &dst1[6]);
  FDK_ASSERT (dst5 == &dst1[8]);
  FDK_ASSERT (dst6 == &dst1[10]);
  FDK_ASSERT (dst7 == &dst1[12]);
  FDK_ASSERT (dst8 == &dst1[14]);
  FDK_ASSERT (dst9 == &dst1[16]);
  FDK_ASSERT (dst10 == &dst1[18]);
  FDK_ASSERT (dst11 == &dst1[20]);
  FDK_ASSERT (dst12 == &dst1[22]);
  FDK_ASSERT (dst13 == &dst1[24]);
  FDK_ASSERT (dst14 == &dst1[26]);
  FDK_ASSERT (dst15 == &dst1[28]);
  FDK_ASSERT (dst16 == &dst1[30]);
  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];  D[1] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[2];  D[1] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[4];  D[1] = ((FIXP_D) src)[5];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[6];  D[1] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[8];  D[1] = ((FIXP_D) src)[9];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[10]; D[1] = ((FIXP_D) src)[11];
  D = (FIXP_D) dst7;  D[0] = ((FIXP_D) src)[12]; D[1] = ((FIXP_D) src)[13];
  D = (FIXP_D) dst8;  D[0] = ((FIXP_D) src)[14]; D[1] = ((FIXP_D) src)[15];
  D = (FIXP_D) dst9;  D[0] = ((FIXP_D) src)[16]; D[1] = ((FIXP_D) src)[17];
  D = (FIXP_D) dst10; D[0] = ((FIXP_D) src)[18]; D[1] = ((FIXP_D) src)[19];
  D = (FIXP_D) dst11; D[0] = ((FIXP_D) src)[20]; D[1] = ((FIXP_D) src)[21];
  D = (FIXP_D) dst12; D[0] = ((FIXP_D) src)[22]; D[1] = ((FIXP_D) src)[23];
  D = (FIXP_D) dst13; D[0] = ((FIXP_D) src)[24]; D[1] = ((FIXP_D) src)[25];
  D = (FIXP_D) dst14; D[0] = ((FIXP_D) src)[26]; D[1] = ((FIXP_D) src)[27];
  D = (FIXP_D) dst15; D[0] = ((FIXP_D) src)[28]; D[1] = ((FIXP_D) src)[29];
  D = (FIXP_D) dst16; D[0] = ((FIXP_D) src)[30]; D[1] = ((FIXP_D) src)[31];
}
static void inline FDK_vldm16_32(
      FIXP_D dst1,  FIXP_D dst2,  FIXP_D dst3,  FIXP_D dst4,
      FIXP_D dst5,  FIXP_D dst6,  FIXP_D dst7,  FIXP_D dst8,
      FIXP_D dst9,  FIXP_D dst10, FIXP_D dst11, FIXP_D dst12,
      FIXP_D dst13, FIXP_D dst14, FIXP_D dst15, FIXP_D dst16,
      FIXP_DBL *src)
{
  FDK_ASSERT (((UINT64) src & 3) == 0);
  FDK_ASSERT (dst2 == &dst1[1]);
  FDK_ASSERT (dst3 == &dst1[2]);
  FDK_ASSERT (dst4 == &dst1[3]);
  FDK_ASSERT (dst5 == &dst1[4]);
  FDK_ASSERT (dst6 == &dst1[5]);
  FDK_ASSERT (dst7 == &dst1[6]);
  FDK_ASSERT (dst8 == &dst1[7]);
  FDK_ASSERT (dst9 == &dst1[8]);
  FDK_ASSERT (dst10 == &dst1[9]);
  FDK_ASSERT (dst11 == &dst1[10]);
  FDK_ASSERT (dst12 == &dst1[11]);
  FDK_ASSERT (dst13 == &dst1[12]);
  FDK_ASSERT (dst14 == &dst1[13]);
  FDK_ASSERT (dst15 == &dst1[14]);
  FDK_ASSERT (dst16 == &dst1[15]);

  FIXP_D D;
  D = (FIXP_D) dst1;  D[0] = ((FIXP_D) src)[0];
  D = (FIXP_D) dst2;  D[0] = ((FIXP_D) src)[1];
  D = (FIXP_D) dst3;  D[0] = ((FIXP_D) src)[2];
  D = (FIXP_D) dst4;  D[0] = ((FIXP_D) src)[3];
  D = (FIXP_D) dst5;  D[0] = ((FIXP_D) src)[4];
  D = (FIXP_D) dst6;  D[0] = ((FIXP_D) src)[5];
  D = (FIXP_D) dst7;  D[0] = ((FIXP_D) src)[6];
  D = (FIXP_D) dst8;  D[0] = ((FIXP_D) src)[7];
  D = (FIXP_D) dst9;  D[0] = ((FIXP_D) src)[8];
  D = (FIXP_D) dst10; D[0] = ((FIXP_D) src)[9];
  D = (FIXP_D) dst11; D[0] = ((FIXP_D) src)[10];
  D = (FIXP_D) dst12; D[0] = ((FIXP_D) src)[11];
  D = (FIXP_D) dst13; D[0] = ((FIXP_D) src)[12];
  D = (FIXP_D) dst14; D[0] = ((FIXP_D) src)[13];
  D = (FIXP_D) dst15; D[0] = ((FIXP_D) src)[14];
  D = (FIXP_D) dst16; D[0] = ((FIXP_D) src)[15];
}


#endif


#ifdef __ARM_NEON__
#define FDK_vstm1(size, src1,                                              dst)  "VSTM " #dst ", { " #src1 " } \n\t"
#define FDK_vstm2(size, src1, src2,                                        dst)  "VSTM " #dst ", { " #src1 ", " #src2 " } \n\t"
#define FDK_vstm3(size, src1, src2, src3,                                  dst)  "VSTM " #dst ", { " #src1 ", " #src2 ", " #src3 " } \n\t"
#define FDK_vstm4(size, src1, src2, src3, src4,                            dst)  "VSTM " #dst ", { " #src1 ", " #src2 ", " #src3 ", " #src4 " } \n\t"
#define FDK_vstm5(size, src1, src2, src3, src4, src5,                      dst)  "VSTM " #dst ", { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 " } \n\t"
#define FDK_vstm6(size, src1, src2, src3, src4, src5, src6,                dst)  "VSTM " #dst ", { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 " } \n\t"
#define FDK_vstm7(size, src1, src2, src3, src4, src5, src6, src7,          dst)  "VSTM " #dst ", { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 " } \n\t"
#define FDK_vstm8(size, src1, src2, src3, src4, src5, src6, src7, src8,    dst)  "VSTM " #dst ", { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 ", " #src8 " } \n\t"
#define FDK_vstm12(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, dst)                              "VSTM " #dst ", { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 ", " #src8 ", " #src9 ", " #src10 ", " #src11 ", " #src12 " } \n\t"
#define FDK_vstm16(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15, src16, dst)  "VSTM " #dst ", { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 ", " #src8 ", " #src9 ", " #src10 ", " #src11 ", " #src12 ", " #src13 ", " #src14 ", " #src15 ", " #src16 " } \n\t"

#define FDK_vstm1_ia(size, src1,                                           dst)  "VSTMIA " #dst "!, { " #src1 " } \n\t"
#define FDK_vstm2_ia(size, src1, src2,                                     dst)  "VSTMIA " #dst "!, { " #src1 ", " #src2 " } \n\t"
#define FDK_vstm3_ia(size, src1, src2, src3,                               dst)  "VSTMIA " #dst "!, { " #src1 ", " #src2 ", " #src3 " } \n\t"
#define FDK_vstm4_ia(size, src1, src2, src3, src4,                         dst)  "VSTMIA " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 " } \n\t"
#define FDK_vstm5_ia(size, src1, src2, src3, src4, src5,                   dst)  "VSTMIA " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 " } \n\t"
#define FDK_vstm6_ia(size, src1, src2, src3, src4, src5, src6,             dst)  "VSTMIA " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 " } \n\t"
#define FDK_vstm7_ia(size, src1, src2, src3, src4, src5, src6, src7,       dst)  "VSTMIA " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 " } \n\t"
#define FDK_vstm8_ia(size, src1, src2, src3, src4, src5, src6, src7, src8, dst)  "VSTMIA " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 ", " #src8 " } \n\t"
#define FDK_vstm12_ia(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, dst)                              "VSTMIA " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 ", " #src8 ", " #src9 ", " #src10 ", " #src11 ", " #src12 " } \n\t"
#define FDK_vstm16_ia(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15, src16, dst)  "VSTMIA " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 ", " #src8 ", " #src9 ", " #src10 ", " #src11 ", " #src12 ", " #src13 ", " #src14 ", " #src15 ", " #src16 " } \n\t"

#define FDK_vstm1_db(size, src1,                                           dst)  "VSTMDB " #dst "!, { " #src1 " } \n\t"
#define FDK_vstm2_db(size, src1, src2,                                     dst)  "VSTMDB " #dst "!, { " #src1 ", " #src2 " } \n\t"
#define FDK_vstm3_db(size, src1, src2, src3,                               dst)  "VSTMDB " #dst "!, { " #src1 ", " #src2 ", " #src3 " } \n\t"
#define FDK_vstm4_db(size, src1, src2, src3, src4,                         dst)  "VSTMDB " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 " } \n\t"
#define FDK_vstm5_db(size, src1, src2, src3, src4, src5,                   dst)  "VSTMDB " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 " } \n\t"
#define FDK_vstm6_db(size, src1, src2, src3, src4, src5, src6,             dst)  "VSTMDB " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 " } \n\t"
#define FDK_vstm7_db(size, src1, src2, src3, src4, src5, src6, src7,       dst)  "VSTMDB " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 " } \n\t"
#define FDK_vstm8_db(size, src1, src2, src3, src4, src5, src6, src7, src8, dst)  "VSTMDB " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 ", " #src8 " } \n\t"
#define FDK_vstm12_db(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, dst)                              "VSTMDB " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 ", " #src8 ", " #src9 ", " #src10 ", " #src11 ", " #src12 " } \n\t"
#define FDK_vstm16_db(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15, src16, dst)  "VSTMDB " #dst "!, { " #src1 ", " #src2 ", " #src3 ", " #src4 ", " #src5 ", " #src6 ", " #src7 ", " #src8 ", " #src9 ", " #src10 ", " #src11 ", " #src12 ", " #src13 ", " #src14 ", " #src15 ", " #src16 " } \n\t"
#else
#define FDK_vstm1(size, src1, dst)          \
            { if      (size == 128) { FDK_vstm1_128((FIXP_Q) src1,(FIXP_D) dst); }  \
              else if (size ==  64) { FDK_vstm1_64 ((FIXP_H) src1,(FIXP_D) dst); }  \
              else if (size ==  32) { FDK_vstm1_32 ((FIXP_D) src1,(FIXP_D) dst); }  \
            }
#define FDK_vstm1_ia(size, src1, dst) { FDK_vstm1(size, src1, dst); dst += 1*(size>>3)/(INT)sizeof(*dst);  }
#define FDK_vstm1_db(size, src1, dst) { dst -= 1*(size>>3)/(INT)sizeof(*dst);  FDK_vstm1(size, src1, dst); }

#define FDK_vstm2(size, src1, src2, dst)    \
            { if      (size == 128) { FDK_vstm2_128((FIXP_Q) src1,(FIXP_Q) src2,(FIXP_D) dst); }   \
              else if (size ==  64) { FDK_vstm2_64 ((FIXP_H) src1,(FIXP_H) src2,(FIXP_D) dst); }   \
              else if (size ==  32) { FDK_vstm2_32 ((FIXP_D) src1,(FIXP_D) src2,(FIXP_D) dst); }   \
            }
#define FDK_vstm2_ia(size, src1, src2, dst) { FDK_vstm2(size, src1, src2, dst);  dst += 2*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vstm2_db(size, src1, src2, dst) { dst -= 2*(size>>3)/(INT)sizeof(*dst);  FDK_vstm2(size, src1, src2, dst); }

#define FDK_vstm3(size, src1, src2, src3, dst)    \
            { if      (size == 128) { FDK_vstm3_128((FIXP_Q) src1,(FIXP_Q) src2,(FIXP_Q) src3,(FIXP_D) dst); }   \
              else if (size ==  64) { FDK_vstm3_64 ((FIXP_H) src1,(FIXP_H) src2,(FIXP_H) src3,(FIXP_D) dst); }   \
              else if (size ==  32) { FDK_vstm3_32 ((FIXP_D) src1,(FIXP_D) src2,(FIXP_D) src3,(FIXP_D) dst); }   \
            }
#define FDK_vstm3_ia(size, src1, src2, src3, dst) { FDK_vstm3(size, src1, src2, src3, dst);  dst += 3*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vstm3_db(size, src1, src2, src3, dst) { dst -= 3*(size>>3)/(INT)sizeof(*dst);  FDK_vstm3(size, src1, src2, src3, dst); }

#define FDK_vstm4(size, src1, src2, src3, src4, dst)    \
            { if      (size == 128) { FDK_vstm4_128((FIXP_Q) src1,(FIXP_Q) src2,(FIXP_Q) src3,(FIXP_Q) src4,(FIXP_D) dst); }   \
              else if (size ==  64) { FDK_vstm4_64 ((FIXP_H) src1,(FIXP_H) src2,(FIXP_H) src3,(FIXP_H) src4,(FIXP_D) dst); }   \
              else if (size ==  32) { FDK_vstm4_32 ((FIXP_D) src1,(FIXP_D) src2,(FIXP_D) src3,(FIXP_Q) src4,(FIXP_D) dst); }   \
            }
#define FDK_vstm4_ia(size, src1, src2, src3, src4, dst) { FDK_vstm4(size, src1, src2, src3, src4, dst);  dst += 4*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vstm4_db(size, src1, src2, src3, src4, dst) { dst -= 4*(size>>3)/(INT)sizeof(*dst);  FDK_vstm4(size, src1, src2, src3, src4, dst); }

#define FDK_vstm5(size, src1, src2, src3, src4, src5, dst)    \
            { if      (size == 128) { FDK_vstm5_128((FIXP_Q) src1,(FIXP_Q) src2,(FIXP_Q) src3,(FIXP_Q) src4,(FIXP_Q) src5,(FIXP_D) dst); }   \
              else if (size ==  64) { FDK_vstm5_64 ((FIXP_H) src1,(FIXP_H) src2,(FIXP_H) src3,(FIXP_H) src4,(FIXP_H) src5,(FIXP_D) dst); }   \
              else if (size ==  32) { FDK_vstm5_32 ((FIXP_D) src1,(FIXP_D) src2,(FIXP_D) src3,(FIXP_D) src4,(FIXP_D) src5,(FIXP_D) dst); }   \
            }
#define FDK_vstm5_ia(size, src1, src2, src3, src4, src5, dst) { FDK_vstm5(size, src1, src2, src3, src4, src5, dst);  dst += 5*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vstm5_db(size, src1, src2, src3, src4, src5, dst) { dst -= 5*(size>>3)/(INT)sizeof(*dst);  FDK_vstm5(size, src1, src2, src3, src4, src5, dst); }

#define FDK_vstm6(size, src1, src2, src3, src4, src5, src6, dst)    \
            { if      (size == 128) { FDK_vstm6_128((FIXP_Q) src1,(FIXP_Q) src2,(FIXP_Q) src3,(FIXP_Q) src4,(FIXP_Q) src5,(FIXP_Q) src6,(FIXP_D) dst); }   \
              else if (size ==  64) { FDK_vstm6_64 ((FIXP_H) src1,(FIXP_H) src2,(FIXP_H) src3,(FIXP_H) src4,(FIXP_H) src5,(FIXP_H) src6,(FIXP_D) dst); }   \
              else if (size ==  32) { FDK_vstm6_32 ((FIXP_D) src1,(FIXP_D) src2,(FIXP_D) src3,(FIXP_D) src4,(FIXP_D) src5,(FIXP_D) src6,(FIXP_D) dst); }   \
            }
#define FDK_vstm6_ia(size, src1, src2, src3, src4, src5, src6, dst) { FDK_vstm6(size, src1, src2, src3, src4, src5, src6, dst);  dst += 6*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vstm6_db(size, src1, src2, src3, src4, src5, src6, dst) { dst -= 6*(size>>3)/(INT)sizeof(*dst);  FDK_vstm6(size, src1, src2, src3, src4, src5, src6, dst); }

#define FDK_vstm7(size, src1, src2, src3, src4, src5, src6, src7, dst)    \
            { if      (size == 128) { FDK_vstm7_128((FIXP_Q) src1,(FIXP_Q) src2,(FIXP_Q) src3,(FIXP_Q) src4,(FIXP_Q) src5,(FIXP_Q) src6,(FIXP_Q) src7,(FIXP_D) dst); }   \
              else if (size ==  64) { FDK_vstm7_64 ((FIXP_H) src1,(FIXP_H) src2,(FIXP_H) src3,(FIXP_H) src4,(FIXP_H) src5,(FIXP_H) src6,(FIXP_H) src7,(FIXP_D) dst); }   \
              else if (size ==  32) { FDK_vstm7_32 ((FIXP_D) src1,(FIXP_D) src2,(FIXP_D) src3,(FIXP_D) src4,(FIXP_D) src5,(FIXP_D) src6,(FIXP_D) src7,(FIXP_D) dst); }   \
            }
#define FDK_vstm7_ia(size, src1, src2, src3, src4, src5, src6, src7, dst) { FDK_vstm7(size, src1, src2, src3, src4, src5, src6, src7, dst);  dst += 7*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vstm7_db(size, src1, src2, src3, src4, src5, src6, src7, dst) { dst -= 7*(size>>3)/(INT)sizeof(*dst);  FDK_vstm7(size, src1, src2, src3, src4, src5, src6, src7, dst); }

#define FDK_vstm8(size, src1, src2, src3, src4, src5, src6, src7, src8, dst)    \
            { if      (size == 128) { FDK_vstm8_128((FIXP_Q) src1,(FIXP_Q) src2,(FIXP_Q) src3,(FIXP_Q) src4,(FIXP_Q) src5,(FIXP_Q) src6,(FIXP_Q) src7,(FIXP_Q) src8,(FIXP_D) dst); }   \
              else if (size ==  64) { FDK_vstm8_64 ((FIXP_H) src1,(FIXP_H) src2,(FIXP_H) src3,(FIXP_H) src4,(FIXP_H) src5,(FIXP_H) src6,(FIXP_H) src7,(FIXP_H) src8,(FIXP_D) dst); }   \
              else if (size ==  32) { FDK_vstm8_32 ((FIXP_D) src1,(FIXP_D) src2,(FIXP_D) src3,(FIXP_D) src4,(FIXP_D) src5,(FIXP_D) src6,(FIXP_D) src7,(FIXP_D) src8,(FIXP_D) dst); }   \
            }
#define FDK_vstm8_ia(size, src1, src2, src3, src4, src5, src6, src7, src8, dst) { FDK_vstm8(size, src1, src2, src3, src4, src5, src6, src7, src8, dst);  dst += 8*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vstm8_db(size, src1, src2, src3, src4, src5, src6, src7, src8, dst) { dst -= 8*(size>>3)/(INT)sizeof(*dst);  FDK_vstm8(size, src1, src2, src3, src4, src5, src6, src7, src8, dst); }

#define FDK_vstm12(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, dst)    \
            { if      (size ==  64) { FDK_vstm12_64 ((FIXP_H) src1,(FIXP_H) src2,(FIXP_H) src3,(FIXP_H) src4,(FIXP_H) src5,(FIXP_H) src6,(FIXP_H) src7,(FIXP_H) src8,(FIXP_H) src9,(FIXP_H) src10,(FIXP_H) src11,(FIXP_H) src12,(FIXP_D) dst); }   \
              else if (size ==  32) { FDK_vstm12_32 ((FIXP_D) src1,(FIXP_D) src2,(FIXP_D) src3,(FIXP_D) src4,(FIXP_D) src5,(FIXP_D) src6,(FIXP_D) src7,(FIXP_D) src8,(FIXP_D) src9,(FIXP_D) src10,(FIXP_D) src11,(FIXP_D) src12,(FIXP_D) dst); }   \
            }
#define FDK_vstm12_ia(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, dst) { FDK_vstm12(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12,  dst);  dst += 12*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vstm12_db(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, dst) { dst -= 12*(size>>3)/(INT)sizeof(*dst);  FDK_vstm12(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12,  dst); }

#define FDK_vstm16(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15, src16, dst)    \
            { if      (size ==  64) { FDK_vstm16_64 ((FIXP_H) src1,(FIXP_H) src2,(FIXP_H) src3,(FIXP_H) src4,(FIXP_H) src5,(FIXP_H) src6,(FIXP_H) src7,(FIXP_H) src8,(FIXP_H) src9,(FIXP_H) src10,(FIXP_H) src11,(FIXP_H) src12,(FIXP_H) src13,(FIXP_H) src14,(FIXP_H) src15,(FIXP_H) src16,(FIXP_D) dst); }   \
              else if (size ==  32) { FDK_vstm16_32 ((FIXP_D) src1,(FIXP_D) src2,(FIXP_D) src3,(FIXP_D) src4,(FIXP_D) src5,(FIXP_D) src6,(FIXP_D) src7,(FIXP_D) src8,(FIXP_D) src9,(FIXP_D) src10,(FIXP_D) src11,(FIXP_D) src12,(FIXP_D) src13,(FIXP_D) src14,(FIXP_D) src15,(FIXP_D) src16,(FIXP_D) dst); }   \
            }
#define FDK_vstm16_ia(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15, src16, dst) { FDK_vstm16(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15, src16, dst);  dst += 16*(size>>3)/(INT)sizeof(*dst); }
#define FDK_vstm16_db(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15, src16, dst) { dst -= 16*(size>>3)/(INT)sizeof(*dst);  FDK_vstm16(size, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15, src16, dst); }


static void inline FDK_vstm1_128(FIXP_Q src1, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];  dst[2]  = D[2];  dst[3]  = D[3];
}
static void inline FDK_vstm1_64(FIXP_H src1, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];
}
static void inline FDK_vstm1_32(FIXP_D src1, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  dst[0] = src1[0];
}

static void inline FDK_vstm2_128(FIXP_Q src1, FIXP_Q src2, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[4]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];  dst[2]  = D[2];  dst[3]  = D[3];
  D = (FIXP_D) src2;  dst[4]  = D[0];  dst[5]  = D[1];  dst[6]  = D[2];  dst[7]  = D[3];
}
static void inline FDK_vstm2_64(FIXP_H src1, FIXP_H src2, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[2]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];
  D = (FIXP_D) src2;  dst[2]  = D[0];  dst[3]  = D[1];
}
static void inline FDK_vstm2_32(FIXP_D src1, FIXP_D src2, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[1]);
  dst[0] = src1[0];
  dst[1] = src2[0];
}

static void inline FDK_vstm3_128(FIXP_Q src1, FIXP_Q src2, FIXP_Q src3, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[4]);
  FDK_ASSERT (src3 == &src1[8]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];  dst[2]  = D[2];  dst[3]  = D[3];
  D = (FIXP_D) src2;  dst[4]  = D[0];  dst[5]  = D[1];  dst[6]  = D[2];  dst[7]  = D[3];
  D = (FIXP_D) src3;  dst[8]  = D[0];  dst[9]  = D[1];  dst[10] = D[2];  dst[11] = D[3];
}
static void inline FDK_vstm3_64(FIXP_H src1, FIXP_H src2, FIXP_H src3, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[2]);
  FDK_ASSERT (src3 == &src1[4]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];
  D = (FIXP_D) src2;  dst[2]  = D[0];  dst[3]  = D[1];
  D = (FIXP_D) src3;  dst[4]  = D[0];  dst[5]  = D[1];
}
static void inline FDK_vstm3_32(FIXP_D src1, FIXP_D src2, FIXP_D src3, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[1]);
  FDK_ASSERT (src3 == &src1[2]);
  dst[0] = src1[0];
  dst[1] = src2[0];
  dst[2] = src3[0];
}

static void inline FDK_vstm4_128(FIXP_Q src1, FIXP_Q src2, FIXP_Q src3, FIXP_Q src4, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[4]);
  FDK_ASSERT (src3 == &src1[8]);
  FDK_ASSERT (src4 == &src1[12]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];  dst[2]  = D[2];  dst[3]  = D[3];
  D = (FIXP_D) src2;  dst[4]  = D[0];  dst[5]  = D[1];  dst[6]  = D[2];  dst[7]  = D[3];
  D = (FIXP_D) src3;  dst[8]  = D[0];  dst[9]  = D[1];  dst[10] = D[2];  dst[11] = D[3];
  D = (FIXP_D) src4;  dst[12] = D[0];  dst[13] = D[1];  dst[14] = D[2];  dst[15] = D[3];
}
static void inline FDK_vstm4_64(FIXP_H src1, FIXP_H src2, FIXP_H src3, FIXP_H src4, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[2]);
  FDK_ASSERT (src3 == &src1[4]);
  FDK_ASSERT (src4 == &src1[6]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];
  D = (FIXP_D) src2;  dst[2]  = D[0];  dst[3]  = D[1];
  D = (FIXP_D) src3;  dst[4]  = D[0];  dst[5]  = D[1];
  D = (FIXP_D) src4;  dst[6]  = D[0];  dst[7]  = D[1];
}
static void inline FDK_vstm4_32(FIXP_D src1, FIXP_D src2, FIXP_D src3, FIXP_D src4, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[1]);
  FDK_ASSERT (src3 == &src1[2]);
  FDK_ASSERT (src4 == &src1[3]);
  dst[0] = src1[0];
  dst[1] = src2[0];
  dst[2] = src3[0];
  dst[3] = src4[0];
}

static void inline FDK_vstm5_128(FIXP_Q src1, FIXP_Q src2, FIXP_Q src3, FIXP_Q src4, FIXP_Q src5, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[4]);
  FDK_ASSERT (src3 == &src1[8]);
  FDK_ASSERT (src4 == &src1[12]);
  FDK_ASSERT (src5 == &src1[16]);

  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];  dst[2]  = D[2];  dst[3]  = D[3];
  D = (FIXP_D) src2;  dst[4]  = D[0];  dst[5]  = D[1];  dst[6]  = D[2];  dst[7]  = D[3];
  D = (FIXP_D) src3;  dst[8]  = D[0];  dst[9]  = D[1];  dst[10] = D[2];  dst[11] = D[3];
  D = (FIXP_D) src4;  dst[12] = D[0];  dst[13] = D[1];  dst[14] = D[2];  dst[15] = D[3];
  D = (FIXP_D) src5;  dst[16] = D[0];  dst[17] = D[1];  dst[18] = D[2];  dst[19] = D[3];
}
static void inline FDK_vstm5_64(FIXP_H src1, FIXP_H src2, FIXP_H src3, FIXP_H src4, FIXP_H src5, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[2]);
  FDK_ASSERT (src3 == &src1[4]);
  FDK_ASSERT (src4 == &src1[6]);
  FDK_ASSERT (src5 == &src1[8]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];
  D = (FIXP_D) src2;  dst[2]  = D[0];  dst[3]  = D[1];
  D = (FIXP_D) src3;  dst[4]  = D[0];  dst[5]  = D[1];
  D = (FIXP_D) src4;  dst[6]  = D[0];  dst[7]  = D[1];
  D = (FIXP_D) src5;  dst[8]  = D[0];  dst[9]  = D[1];
}
static void inline FDK_vstm5_32(FIXP_D src1, FIXP_D src2, FIXP_D src3, FIXP_D src4, FIXP_D src5, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2  == &src1[1]);
  FDK_ASSERT (src3  == &src1[2]);
  FDK_ASSERT (src4  == &src1[3]);
  FDK_ASSERT (src5  == &src1[4]);
  dst[0] = src1[0];
  dst[1] = src2[0];
  dst[2] = src3[0];
  dst[3] = src4[0];
  dst[4] = src5[0];
}

static void inline FDK_vstm6_128(FIXP_Q src1, FIXP_Q src2, FIXP_Q src3, FIXP_Q src4, FIXP_Q src5, FIXP_Q src6, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[4]);
  FDK_ASSERT (src3 == &src1[8]);
  FDK_ASSERT (src4 == &src1[12]);
  FDK_ASSERT (src5 == &src1[16]);
  FDK_ASSERT (src6 == &src1[20]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];  dst[2]  = D[2];  dst[3]  = D[3];
  D = (FIXP_D) src2;  dst[4]  = D[0];  dst[5]  = D[1];  dst[6]  = D[2];  dst[7]  = D[3];
  D = (FIXP_D) src3;  dst[8]  = D[0];  dst[9]  = D[1];  dst[10] = D[2];  dst[11] = D[3];
  D = (FIXP_D) src4;  dst[12] = D[0];  dst[13] = D[1];  dst[14] = D[2];  dst[15] = D[3];
  D = (FIXP_D) src5;  dst[16] = D[0];  dst[17] = D[1];  dst[18] = D[2];  dst[19] = D[3];
  D = (FIXP_D) src6;  dst[20] = D[0];  dst[21] = D[1];  dst[22] = D[2];  dst[23] = D[3];
}
static void inline FDK_vstm6_64(FIXP_H src1, FIXP_H src2, FIXP_H src3, FIXP_H src4, FIXP_H src5, FIXP_H src6, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2  == &src1[2]);
  FDK_ASSERT (src3  == &src1[4]);
  FDK_ASSERT (src4  == &src1[6]);
  FDK_ASSERT (src5  == &src1[8]);
  FDK_ASSERT (src6  == &src1[10]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];
  D = (FIXP_D) src2;  dst[2]  = D[0];  dst[3]  = D[1];
  D = (FIXP_D) src3;  dst[4]  = D[0];  dst[5]  = D[1];
  D = (FIXP_D) src4;  dst[6]  = D[0];  dst[7]  = D[1];
  D = (FIXP_D) src5;  dst[8]  = D[0];  dst[9]  = D[1];
  D = (FIXP_D) src6;  dst[10] = D[0];  dst[11] = D[1];
}
static void inline FDK_vstm6_32(FIXP_D src1, FIXP_D src2, FIXP_D src3, FIXP_D src4, FIXP_D src5, FIXP_D src6, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2  == &src1[1]);
  FDK_ASSERT (src3  == &src1[2]);
  FDK_ASSERT (src4  == &src1[3]);
  FDK_ASSERT (src5  == &src1[4]);
  FDK_ASSERT (src6  == &src1[5]);
  dst[0] = src1[0];
  dst[1] = src2[0];
  dst[2] = src3[0];
  dst[3] = src4[0];
  dst[4] = src5[0];
  dst[5] = src6[0];
}

static void inline FDK_vstm7_128(FIXP_Q src1, FIXP_Q src2, FIXP_Q src3, FIXP_Q src4, FIXP_Q src5, FIXP_Q src6, FIXP_Q src7, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[4]);
  FDK_ASSERT (src3 == &src1[8]);
  FDK_ASSERT (src4 == &src1[12]);
  FDK_ASSERT (src5 == &src1[16]);
  FDK_ASSERT (src6 == &src1[20]);
  FDK_ASSERT (src7 == &src1[24]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];  dst[2]  = D[2];  dst[3]  = D[3];
  D = (FIXP_D) src2;  dst[4]  = D[0];  dst[5]  = D[1];  dst[6]  = D[2];  dst[7]  = D[3];
  D = (FIXP_D) src3;  dst[8]  = D[0];  dst[9]  = D[1];  dst[10] = D[2];  dst[11] = D[3];
  D = (FIXP_D) src4;  dst[12] = D[0];  dst[13] = D[1];  dst[14] = D[2];  dst[15] = D[3];
  D = (FIXP_D) src5;  dst[16] = D[0];  dst[17] = D[1];  dst[18] = D[2];  dst[19] = D[3];
  D = (FIXP_D) src6;  dst[20] = D[0];  dst[21] = D[1];  dst[22] = D[2];  dst[23] = D[3];
  D = (FIXP_D) src7;  dst[24] = D[0];  dst[25] = D[1];  dst[26] = D[2];  dst[27] = D[3];
}
static void inline FDK_vstm7_64(FIXP_H src1, FIXP_H src2, FIXP_H src3, FIXP_H src4, FIXP_H src5, FIXP_H src6, FIXP_H src7, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2  == &src1[2]);
  FDK_ASSERT (src3  == &src1[4]);
  FDK_ASSERT (src4  == &src1[6]);
  FDK_ASSERT (src5  == &src1[8]);
  FDK_ASSERT (src6  == &src1[10]);
  FDK_ASSERT (src7  == &src1[12]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];
  D = (FIXP_D) src2;  dst[2]  = D[0];  dst[3]  = D[1];
  D = (FIXP_D) src3;  dst[4]  = D[0];  dst[5]  = D[1];
  D = (FIXP_D) src4;  dst[6]  = D[0];  dst[7]  = D[1];
  D = (FIXP_D) src5;  dst[8]  = D[0];  dst[9]  = D[1];
  D = (FIXP_D) src6;  dst[10] = D[0];  dst[11] = D[1];
  D = (FIXP_D) src7;  dst[12] = D[0];  dst[13] = D[1];
}
static void inline FDK_vstm7_32(FIXP_D src1, FIXP_D src2, FIXP_D src3, FIXP_D src4, FIXP_D src5, FIXP_D src6, FIXP_D src7, FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2  == &src1[1]);
  FDK_ASSERT (src3  == &src1[2]);
  FDK_ASSERT (src4  == &src1[3]);
  FDK_ASSERT (src5  == &src1[4]);
  FDK_ASSERT (src6  == &src1[5]);
  FDK_ASSERT (src7  == &src1[6]);
  dst[0] = src1[0];
  dst[1] = src2[0];
  dst[2] = src3[0];
  dst[3] = src4[0];
  dst[4] = src5[0];
  dst[5] = src6[0];
  dst[6] = src7[0];
}

static void inline FDK_vstm8_128(
        FIXP_Q src1, FIXP_Q src2, FIXP_Q src3, FIXP_Q src4,
        FIXP_Q src5, FIXP_Q src6, FIXP_Q src7, FIXP_Q src8,
        FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2 == &src1[4]);
  FDK_ASSERT (src3 == &src1[8]);
  FDK_ASSERT (src4 == &src1[12]);
  FDK_ASSERT (src5 == &src1[16]);
  FDK_ASSERT (src6 == &src1[20]);
  FDK_ASSERT (src7 == &src1[24]);
  FDK_ASSERT (src8 == &src1[28]);
  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];  dst[2]  = D[2];  dst[3]  = D[3];
  D = (FIXP_D) src2;  dst[4]  = D[0];  dst[5]  = D[1];  dst[6]  = D[2];  dst[7]  = D[3];
  D = (FIXP_D) src3;  dst[8]  = D[0];  dst[9]  = D[1];  dst[10] = D[2];  dst[11] = D[3];
  D = (FIXP_D) src4;  dst[12] = D[0];  dst[13] = D[1];  dst[14] = D[2];  dst[15] = D[3];
  D = (FIXP_D) src5;  dst[16] = D[0];  dst[17] = D[1];  dst[18] = D[2];  dst[19] = D[3];
  D = (FIXP_D) src6;  dst[20] = D[0];  dst[21] = D[1];  dst[22] = D[2];  dst[23] = D[3];
  D = (FIXP_D) src7;  dst[24] = D[0];  dst[25] = D[1];  dst[26] = D[2];  dst[27] = D[3];
  D = (FIXP_D) src8;  dst[28] = D[0];  dst[29] = D[1];  dst[30] = D[2];  dst[31] = D[3];
}
static void inline FDK_vstm8_64(
        FIXP_H src1, FIXP_H src2, FIXP_H src3, FIXP_H src4,
        FIXP_H src5, FIXP_H src6, FIXP_H src7, FIXP_H src8,
        FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2  == &src1[2]);
  FDK_ASSERT (src3  == &src1[4]);
  FDK_ASSERT (src4  == &src1[6]);
  FDK_ASSERT (src5  == &src1[8]);
  FDK_ASSERT (src6  == &src1[10]);
  FDK_ASSERT (src7  == &src1[12]);
  FDK_ASSERT (src8  == &src1[14]);

  FIXP_D D;
  D = (FIXP_D) src1;  dst[0]  = D[0];  dst[1]  = D[1];
  D = (FIXP_D) src2;  dst[2]  = D[0];  dst[3]  = D[1];
  D = (FIXP_D) src3;  dst[4]  = D[0];  dst[5]  = D[1];
  D = (FIXP_D) src4;  dst[6]  = D[0];  dst[7]  = D[1];
  D = (FIXP_D) src5;  dst[8]  = D[0];  dst[9]  = D[1];
  D = (FIXP_D) src6;  dst[10] = D[0];  dst[11] = D[1];
  D = (FIXP_D) src7;  dst[12] = D[0];  dst[13] = D[1];
  D = (FIXP_D) src8;  dst[14] = D[0];  dst[15] = D[1];
}
static void inline FDK_vstm8_32(
        FIXP_D src1, FIXP_D src2, FIXP_D src3, FIXP_D src4,
        FIXP_D src5, FIXP_D src6, FIXP_D src7, FIXP_D src8,
        FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2  == &src1[1]);
  FDK_ASSERT (src3  == &src1[2]);
  FDK_ASSERT (src4  == &src1[3]);
  FDK_ASSERT (src5  == &src1[4]);
  FDK_ASSERT (src6  == &src1[5]);
  FDK_ASSERT (src7  == &src1[6]);
  FDK_ASSERT (src8  == &src1[7]);
  dst[0] = src1[0];
  dst[1] = src2[0];
  dst[2] = src3[0];
  dst[3] = src4[0];
  dst[4] = src5[0];
  dst[5] = src6[0];
  dst[6] = src7[0];
  dst[7] = src8[0];
}

static void inline FDK_vstm12_64(
        FIXP_H src1,  FIXP_H src2,  FIXP_H src3,  FIXP_H src4,
        FIXP_H src5,  FIXP_H src6,  FIXP_H src7,  FIXP_H src8,
        FIXP_H src9,  FIXP_H src10, FIXP_H src11, FIXP_H src12,
        FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2  == &src1[2]);
  FDK_ASSERT (src3  == &src1[4]);
  FDK_ASSERT (src4  == &src1[6]);
  FDK_ASSERT (src5  == &src1[8]);
  FDK_ASSERT (src6  == &src1[10]);
  FDK_ASSERT (src7  == &src1[12]);
  FDK_ASSERT (src8  == &src1[14]);
  FDK_ASSERT (src9  == &src1[16]);
  FDK_ASSERT (src10 == &src1[18]);
  FDK_ASSERT (src11 == &src1[20]);
  FDK_ASSERT (src12 == &src1[22]);

  FIXP_D D;
  D = (FIXP_D) src1;   dst[0]  = D[0];  dst[1]  = D[1];
  D = (FIXP_D) src2;   dst[2]  = D[0];  dst[3]  = D[1];
  D = (FIXP_D) src3;   dst[4]  = D[0];  dst[5]  = D[1];
  D = (FIXP_D) src4;   dst[6]  = D[0];  dst[7]  = D[1];
  D = (FIXP_D) src5;   dst[8]  = D[0];  dst[9]  = D[1];
  D = (FIXP_D) src6;   dst[10] = D[0];  dst[11] = D[1];
  D = (FIXP_D) src7;   dst[12] = D[0];  dst[13] = D[1];
  D = (FIXP_D) src8;   dst[14] = D[0];  dst[15] = D[1];
  D = (FIXP_D) src9;   dst[16] = D[0];  dst[17] = D[1];
  D = (FIXP_D) src10;  dst[18] = D[0];  dst[19] = D[1];
  D = (FIXP_D) src11;  dst[20] = D[0];  dst[21] = D[1];
  D = (FIXP_D) src12;  dst[22] = D[0];  dst[23] = D[1];
}
static void inline FDK_vstm12_32(
        FIXP_D src1,  FIXP_D src2,  FIXP_D src3,  FIXP_D src4,
        FIXP_D src5,  FIXP_D src6,  FIXP_D src7,  FIXP_D src8,
        FIXP_D src9,  FIXP_D src10, FIXP_D src11, FIXP_D src12,
        FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2  == &src1[1]);
  FDK_ASSERT (src3  == &src1[2]);
  FDK_ASSERT (src4  == &src1[3]);
  FDK_ASSERT (src5  == &src1[4]);
  FDK_ASSERT (src6  == &src1[5]);
  FDK_ASSERT (src7  == &src1[6]);
  FDK_ASSERT (src8  == &src1[7]);
  FDK_ASSERT (src9  == &src1[8]);
  FDK_ASSERT (src10 == &src1[9]);
  FDK_ASSERT (src11 == &src1[10]);
  FDK_ASSERT (src12 == &src1[11]);
  dst[0]  = src1[0];
  dst[1]  = src2[0];
  dst[2]  = src3[0];
  dst[3]  = src4[0];
  dst[4]  = src5[0];
  dst[5]  = src6[0];
  dst[6]  = src7[0];
  dst[7]  = src8[0];
  dst[8]  = src9[0];
  dst[9]  = src10[0];
  dst[10] = src11[0];
  dst[11] = src12[0];
}

static void inline FDK_vstm16_64(
        FIXP_H src1,  FIXP_H src2,  FIXP_H src3,  FIXP_H src4,
        FIXP_H src5,  FIXP_H src6,  FIXP_H src7,  FIXP_H src8,
        FIXP_H src9,  FIXP_H src10, FIXP_H src11, FIXP_H src12,
        FIXP_H src13, FIXP_H src14, FIXP_H src15, FIXP_H src16,
        FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2  == &src1[2]);
  FDK_ASSERT (src3  == &src1[4]);
  FDK_ASSERT (src4  == &src1[6]);
  FDK_ASSERT (src5  == &src1[8]);
  FDK_ASSERT (src6  == &src1[10]);
  FDK_ASSERT (src7  == &src1[12]);
  FDK_ASSERT (src8  == &src1[14]);
  FDK_ASSERT (src9  == &src1[16]);
  FDK_ASSERT (src10 == &src1[18]);
  FDK_ASSERT (src11 == &src1[20]);
  FDK_ASSERT (src12 == &src1[22]);
  FDK_ASSERT (src13 == &src1[24]);
  FDK_ASSERT (src14 == &src1[26]);
  FDK_ASSERT (src15 == &src1[28]);
  FDK_ASSERT (src16 == &src1[30]);

  FIXP_D D;
  D = (FIXP_D) src1;   dst[0]  = D[0];  dst[1]  = D[1];
  D = (FIXP_D) src2;   dst[2]  = D[0];  dst[3]  = D[1];
  D = (FIXP_D) src3;   dst[4]  = D[0];  dst[5]  = D[1];
  D = (FIXP_D) src4;   dst[6]  = D[0];  dst[7]  = D[1];
  D = (FIXP_D) src5;   dst[8]  = D[0];  dst[9]  = D[1];
  D = (FIXP_D) src6;   dst[10] = D[0];  dst[11] = D[1];
  D = (FIXP_D) src7;   dst[12] = D[0];  dst[13] = D[1];
  D = (FIXP_D) src8;   dst[14] = D[0];  dst[15] = D[1];
  D = (FIXP_D) src9;   dst[16] = D[0];  dst[17] = D[1];
  D = (FIXP_D) src10;  dst[18] = D[0];  dst[19] = D[1];
  D = (FIXP_D) src11;  dst[20] = D[0];  dst[21] = D[1];
  D = (FIXP_D) src12;  dst[22] = D[0];  dst[23] = D[1];
  D = (FIXP_D) src13;  dst[24] = D[0];  dst[25] = D[1];
  D = (FIXP_D) src14;  dst[26] = D[0];  dst[27] = D[1];
  D = (FIXP_D) src15;  dst[28] = D[0];  dst[29] = D[1];
  D = (FIXP_D) src16;  dst[30] = D[0];  dst[31] = D[1];
}
static void inline FDK_vstm16_32(
        FIXP_D src1,  FIXP_D src2,  FIXP_D src3,  FIXP_D src4,
        FIXP_D src5,  FIXP_D src6,  FIXP_D src7,  FIXP_D src8,
        FIXP_D src9,  FIXP_D src10, FIXP_D src11, FIXP_D src12,
        FIXP_D src13, FIXP_D src14, FIXP_D src15, FIXP_D src16,
        FIXP_DBL *dst)
{
  FDK_ASSERT (((UINT64) dst & 3) == 0);
  FDK_ASSERT (src2  == &src1[1]);
  FDK_ASSERT (src3  == &src1[2]);
  FDK_ASSERT (src4  == &src1[3]);
  FDK_ASSERT (src5  == &src1[4]);
  FDK_ASSERT (src6  == &src1[5]);
  FDK_ASSERT (src7  == &src1[6]);
  FDK_ASSERT (src8  == &src1[7]);
  FDK_ASSERT (src9  == &src1[8]);
  FDK_ASSERT (src10 == &src1[9]);
  FDK_ASSERT (src11 == &src1[10]);
  FDK_ASSERT (src12 == &src1[11]);
  FDK_ASSERT (src13 == &src1[12]);
  FDK_ASSERT (src14 == &src1[13]);
  FDK_ASSERT (src15 == &src1[14]);
  FDK_ASSERT (src16 == &src1[15]);

  dst[0]  = src1[0];
  dst[1]  = src2[0];
  dst[2]  = src3[0];
  dst[3]  = src4[0];
  dst[4]  = src5[0];
  dst[5]  = src6[0];
  dst[6]  = src7[0];
  dst[7]  = src8[0];
  dst[8]  = src9[0];
  dst[9]  = src10[0];
  dst[10] = src11[0];
  dst[11] = src12[0];
  dst[12] = src13[0];
  dst[13] = src14[0];
  dst[14] = src15[0];
  dst[15] = src16[0];
}


#endif

#ifndef __ARM_NEON__

void FDK_check_s16_overflow(FIXP_DBL value)
{
  if (value > (FIXP_DBL) 0x7FFF)
  {
    FDKprintf("Error: 16-bit overflow: 0x%016X\n", value);
  }
  else if (value < (FIXP_DBL) 0xFFFF8000)
  {
    FDKprintf("Error: 16-bit underflow: 0x%016X\n", value);
  }
}


void FDK_check_s32_overflow(INT64 value)
{
  if (value > (INT64) 0x7FFFFFFF)
  {
    FDKprintf("Error: 32-bit overflow: 0x%016X\n", value);
  }
  else if (-value > (INT64) 2147483648 /* 0x8000.000 */)
  {
    FDKprintf("Error: 32-bit underflow: 0x%016X\n", value);
  }
}

void FDK_check_move_float_immediate(FLOAT immediate)
{
  /* CONDITION - According to ARM DUI 0489C ("ARM Compiler toolchain Assembler Reference") 4.4.5 footnote b:
  Any number that can be expressed as +/-n * 2^(-r), where n and r are integers, 16 <= n <= 31, 0 <= r <= 7. */

  INT n[32]   = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 , -16, -17, -18, -19, -20, -21, -22, -23, -24, -25, -26, -27, -28, -29, -30, -31 };
  INT r[8]    = {0, 1, 2, 3, 4, 5, 6, 7};
  FLOAT testFloat;
  INT index_r, index_n;

  for (index_r = 0; index_r < 8; index_r++)
    for (index_n = 0; index_n < 32; index_n++)
    {
      testFloat = n[index_n] * (128 >> (INT)(r[index_r])) / 128.f;
      if (testFloat == immediate) return;
    }

  FDKprintf("Error: %f can not expressed as  +/- n * 2^(-r), where n and r are integers, 16 <= n <= 31, 0 <= r <= 7 \n", immediate);
}

void FDK_check_move_immediate(int size, int datatype, int sign, INT64 immediate)
{
  FDK_ASSERT(size >= datatype);
  FDK_ASSERT (sign == 1 || sign == -1);

  if (immediate)
  {
    UCHAR byte0 = (UCHAR) (immediate>> 0);
    UCHAR byte1 = (UCHAR) (immediate>> 8);
    UCHAR byte2 = (UCHAR) (immediate>>16);
    UCHAR byte3 = (UCHAR) (immediate>>24);
    UCHAR byte4 = (UCHAR) (immediate>>32);
    UCHAR byte5 = (UCHAR) (immediate>>40);
    UCHAR byte6 = (UCHAR) (immediate>>48);
    UCHAR byte7 = (UCHAR) (immediate>>56);

    if (datatype == 8)
    {
      if (sign == -1)
        FDKprintf ("Error in 8-bit MVN immediate: 0x%16X - Use VMOV instead of VMVN for 8-bit constants\n", ~immediate);
      if (byte1 | byte2 | byte3 | byte4 | byte5 | byte6 | byte7)
      {
        FDKprintf ("Error in 8-bit immediate: 0x%016X - upper bytes must be 0x00\n", immediate);
      }
    }
    else if (datatype == 16)
    {
      if (byte2 | byte3 | byte4 | byte5 | byte6 | byte7)
      {
        if (sign == 1)
          FDKprintf ("Error in 16-bit MOV immediate: 0x%016X - upper bits (>16) must be 0x00\n", immediate);
        else
          FDKprintf ("Error in 16-bit MVN immediate: 0x%016X - upper bits (>16) must be 0xFF\n", ~immediate);
      }
      else
      {
        /* VMOV: (sign == 1)  0x00XY, 0xXY00 */
        /* VMVN: (sign == -1) 0xFFXY, 0xXYFF */
        if (!((((INT64) byte0 <<  0) == immediate) ||
              (((INT64) byte1 <<  8) == immediate) ))
        {
           if (sign == 1)
             FDKprintf ("Error in 16-bit MOV immediate: 0x%016X - must be in range: 0x00XY, 0xXY00\n", immediate);
           else
             FDKprintf ("Error in 16-bit MVN immediate: 0x%016X - must be in range: 0xFFXY, 0xXYFF\n", ~immediate);
        }
      }
    }
    else if (datatype == 32)
    {
        /* VMOV: (sign == 1)  0x000000XY, 0x0000XY00, 0x00XY0000, 0xXY000000, 0x0000XYFF, 0x00XYFFFF */
        /* VMVN: (sign == -1) 0xFFFFFFXY, 0xFFFFXYFF, 0xFFXYFFFF, 0xXYFFFFFF, 0xFFFFXY00, 0xFFXY0000 */
        if (byte4 | byte5 | byte6 | byte7)
        {
           if (sign == 1)
             FDKprintf ("Error in 32-bit MOV immediate: 0x%016X - upper fillbytes must be 0x00\n", immediate);
           else
             FDKprintf ("Error in 32-bit MVN immediate: 0x%016X - upper fillbytes must be 0xFF\n", ~immediate);
        }
        if (!((((INT64) byte0 <<  0) == immediate) ||
              (((INT64) byte1 <<  8) == immediate) ||
              (((INT64) byte2 << 16) == immediate) ||
              (((INT64) byte3 << 24) == immediate) ||
              ((((INT64) byte1 <<  8) | 0x00FF) == immediate) ||
              ((((INT64) byte2 << 16) | 0xFFFF) == immediate) ))
        {
           if (sign == 1)
             FDKprintf ("Error in 32-bit MOV immediate: 0x%016X - must be in range: 0x000000XY, 0x0000XY00, 0x00XY0000, 0xXY000000, 0x0000XYFF, 0x00XYFFFF \n", immediate);
           else
             FDKprintf ("Error in 32-bit MVN immediate: 0x%016X - must be in range: 0xFFFFFFXY, 0xFFFFXYFF, 0xFFXYFFFF, 0xXYFFFFFF, 0xFFFFXY00, 0xFFXY0000 \n",~immediate);
        }
    }
    else if (datatype == 64)
    {
      if (sign == -1)
        FDKprintf ("Error in 64-bit MVN immediate: 0x%16X - Use VMOV instead of VMVN for 64-bit constants\n", ~immediate);

      /* all bytes must match either 0x00 or 0xFF */
      if (!(( (byte0 == 0x00) || (byte0 == 0xFF) ) || ( (byte1 == 0x00) || (byte1 == 0xFF) ) ||
            ( (byte2 == 0x00) || (byte2 == 0xFF) ) || ( (byte3 == 0x00) || (byte3 == 0xFF) ) ||
            ( (byte4 == 0x00) || (byte4 == 0xFF) ) || ( (byte5 == 0x00) || (byte5 == 0xFF) ) ||
            ( (byte6 == 0x00) || (byte6 == 0xFF) ) || ( (byte7 == 0x00) || (byte7 == 0xFF) ) ))
      {
        FDKprintf ("Error in 64-bit MOV immediate: 0x%16X - any byte must be either 0x00/0xFF\n", immediate);
      }
    }
  }
}
#endif

#ifdef __ARM_NEON__
#define FDK_vmov_i64(size, dst, immediate)  "VMOV.I64 " #dst ", # " #immediate "\n\t"
#define FDK_vmvn_i64(size, dst, immediate)  "VMVN.I64 " #dst ", # " #immediate "\n\t"
#define __FDK_vmov_i32(size, dst, immediate)  "VMOV.I32 " #dst ", # " #immediate "\n\t"
#define FDK_vmvn_i32(size, dst, immediate)  "VMVN.I32 " #dst ", # " #immediate "\n\t"
#define __FDK_vmov_f32(size, dst, immediate)  "VMOV.F32 " #dst ", # " #immediate "\n\t"
#define FDK_vmov_q(dst, src)                "VMOV " #dst ", " #src "\n\t"
#define FDK_vmvn_q(dst, src)                "VMVN " #dst ", " #src "\n\t"
#define FDK_vmov_d(dst, src)                "VMOV " #dst ", " #src "\n\t"
#define FDK_vmvn_d(dst, src)                "VMVN " #dst ", " #src "\n\t"
#define __FDK_vmov_reg(Rd, Sn)              "VMOV.32 " #Rd ", " #Sn "\n\t"
#define __FDK_vmov_sn(Sn, Rd)               "VMOV.32 " #Sn ", " #Rd "\n\t"
#define FDK_vmovl_s(size, Qd, Dm)           "VMOVL.S" #size " " #Qd ", " #Dm "\n\t"
#define FDK_vmovl_u(size, Qd, Dm)           "VMOVL.U" #size " " #Qd ", " #Dm "\n\t"
#define FDK_vmov_dreg(Rd1, Rd2, Sm1, Sm2)   "VMOV " #Rd1 ", " #Rd2 ", " #Sm1 ", " #Sm2 "\n\t"
#define FDK_vmov_dsn(Sn1, Sn2, Rd1, Rd2)    "VMOV " #Sn1 ", " #Sn2 ", " #Rd1 ", " #Rd2 "\n\t"
#define FDK_vmov_reg(Rd, Sn)                __FDK_vmov_reg(Rd, Sn)
#define FDK_vmov_sn( Sn, Rd)                __FDK_vmov_sn( Sn, Rd)
#define FDK_vmov_i32(size, dst, immediate)  __FDK_vmov_i32(size, dst, immediate)
#define FDK_vmov_f32(size, dst, immediate)  __FDK_vmov_f32(size, dst, immediate)
#else
#define FDK_vmov_i64(size, dst, immediate)   { if (size == 128) { FDK_vmov_q_i64((FIXP_Q) dst, (INT64) immediate); } else if (size == 64) { FDK_vmov_d_i64((FIXP_D) dst, (INT64) immediate); } }
#define FDK_vmvn_i64(size, dst, immediate)   { if (size == 128) { FDK_vmvn_q_i64((FIXP_Q) dst, (INT64) immediate); } else if (size == 64) { FDK_vmvn_d_i64((FIXP_D) dst, (INT64) immediate); } }
#define FDK_vmov_i32(size, dst, immediate)   { if (size == 128) { FDK_vmov_q_i32((FIXP_Q) dst, (INT64) immediate); } else if (size == 64) { FDK_vmov_d_i32((FIXP_D) dst, (INT64) immediate); } }
#define FDK_vmvn_i32(size, dst, immediate)   { if (size == 128) { FDK_vmvn_q_i32((FIXP_Q) dst, (INT64) immediate); } else if (size == 64) { FDK_vmvn_d_i32((FIXP_D) dst, (INT64) immediate); } }
#define FDK_vmov_f32(size, dst, immediate)   { if (size == 128) { FDK_vmov_q_f32((FIXP_Q) dst, (FLOAT) immediate); } else if (size == 64) { FDK_vmov_d_f32((FIXP_D) dst, (FLOAT) immediate); } }
#define FDK_vmov_q(dst, src)  __FDK_vmov_q(dst, src);
#define FDK_vmvn_q(dst, src)  __FDK_vmvn_q(dst, src);
#define FDK_vmov_d(dst, src)  __FDK_vmov_d(dst, src);
#define FDK_vmvn_d(dst, src)  __FDK_vmvn_d(dst, src);
#define FDK_vmov_reg(Rd, Sn)  __FDK_vmov_reg((LONG &)Rd, (FIXP_DBL *) (void *)Sn);  /* by default, datatype is 32 */
#define FDK_vmov_sn(Sn, Rd)   __FDK_vmov_sn(Sn, (INT)Rd);   /* by default, datatype is 32 */
#define FDK_vmovl_s(size, Qd, Dm)          __FDK_vmovl_s(size, Qd, Dm);
#define FDK_vmovl_u(size, Qd, Dm)          __FDK_vmovl_u(size, Qd, Dm);
#define FDK_vmov_dreg(Rd1, Rd2, Sm1, Sm2)  __FDK_vmov_dreg(Rd1, Rd2, Sm1, Sm2);
#define FDK_vmov_dsn(Sn1, Sn2, Rd1, Rd2)   __FDK_vmov_dsn(Sn1, Sn2, (LONG)Rd1, (LONG)Rd2);

static void inline __FDK_vmovl_s(INT size, FIXP_Q Qd, FIXP_H Dm)
{
  FIXP_DBL tmp[2];
  FIXP_D dm = (FIXP_D) Dm;
  tmp[0] = dm[0];
  tmp[1] = dm[1];
  switch (size)
  {
    case 32:
    {
      FIXP_D Dm32 = (FIXP_D) tmp;
      INT64 *Qd64 = (INT64 *) Qd;
      for (int i = 0; i < 2; i++)
        Qd64[i] = (INT64) Dm32[i];
      break;
    }
    case 16:
    {
      FIXP_S Dm16 = (FIXP_S) tmp;
      FIXP_D Qd32 = (FIXP_D) Qd;
      for (int i = 0; i < 4; i++)
        Qd32[i] = (FIXP_DBL) Dm16[i];
      break;
    }
    case 8:
    {
      FIXP_C Dm8 = (FIXP_C) tmp;
      FIXP_S Qd16 = (FIXP_S) Qd;
      for (int i = 0; i < 8; i++)
        Qd16[i] = (FIXP_SGL) Dm8[i];
      break;
    }
  }
}

static void inline __FDK_vmovl_u(INT size, FIXP_Q Qd, FIXP_H Dm)
{
  FIXP_DBL tmp[2];
  FIXP_D dm = (FIXP_D) Dm;
  tmp[0] = dm[0];
  tmp[1] = dm[1];
  switch (size)
  {
    case 32:
    {
      FIXP_D Dm32 = (FIXP_D) tmp;
      UINT64 *Qd64 = (UINT64 *) Qd;
      for (int i = 0; i < 2; i++)
        Qd64[i] = (UINT64) Dm32[i];
      break;
    }
    case 16:
    {
      FIXP_S Dm16 = (FIXP_S) tmp;
      UINT *Qd32 = (UINT *) Qd;
      for (int i = 0; i < 4; i++)
        Qd32[i] = (UINT) Dm16[i];
      break;
    }
    case 8:
    {
      FIXP_C Dm8 = (FIXP_C) tmp;
      USHORT *Qd16 = (USHORT *) Qd;
      for (int i = 0; i < 8; i++)
        Qd16[i] = (USHORT) Dm8[i];
      break;
    }
  }
}

static void inline __FDK_vmov_q (FIXP_Q dst, FIXP_Q src)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  Dst[0] = Src[0];
  Dst[1] = Src[1];
  Dst[2] = Src[2];
  Dst[3] = Src[3];
}
static void inline __FDK_vmvn_q (FIXP_Q dst, FIXP_Q src)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  Dst[0] = ~Src[0];
  Dst[1] = ~Src[1];
  Dst[2] = ~Src[2];
  Dst[3] = ~Src[3];
}
static void inline __FDK_vmov_d (FIXP_D dst, FIXP_D src)
{
  dst[0] = src[0];
  dst[1] = src[1];
}
static void inline __FDK_vmvn_d (FIXP_D dst, FIXP_D src)
{
  dst[0] = ~src[0];
  dst[1] = ~src[1];
}

static void inline FDK_vmov_q_i64(FIXP_Q dst, INT64 immediate)
{
  INT64 *D = (INT64 *) dst;
  FDK_check_move_immediate(128, 64, 1, immediate);
  D[0] = immediate;
  D[1] = immediate;
}

static void inline FDK_vmov_d_i64(FIXP_H dst, INT64 immediate)
{
  INT64 *D = (INT64 *) dst;
  FDK_check_move_immediate(64, 64, 1, immediate);
  D[0] = immediate;
}

static void inline FDK_vmvn_q_i64(FIXP_Q dst, INT64 immediate)
{
  INT64 *D = (INT64 *) dst;
  FDK_check_move_immediate(128, 64, -1, immediate);
  D[0] = ~immediate;
  D[1] = ~immediate;
}
static void inline FDK_vmvn_d_i64(FIXP_H dst, INT64 immediate)
{
  INT64 *D = (INT64 *) dst;
  FDK_check_move_immediate(64, 64, -1, immediate);
  D[0] = ~immediate;
}

static void inline FDK_vmov_q_i32(FIXP_Q dst, INT64 immediate)
{
  FIXP_D D = (FIXP_D) dst;
  FDK_check_move_immediate(128, 32, 1, immediate);
  D[0] = (FIXP_DBL) immediate;
  D[1] = (FIXP_DBL) immediate;
  D[2] = (FIXP_DBL) immediate;
  D[3] = (FIXP_DBL) immediate;
}

static void inline FDK_vmov_d_i32(FIXP_H dst, INT64 immediate)
{
  FIXP_D D = (FIXP_D) dst;
  FDK_check_move_immediate(64, 32, 1, immediate);
  D[0] = (FIXP_DBL) immediate;
  D[1] = (FIXP_DBL) immediate;
}

static void inline FDK_vmvn_q_i32(FIXP_Q dst, INT64 immediate)
{
  FIXP_D D = (FIXP_D) dst;
  FDK_check_move_immediate(128, 32, -1, immediate);
  D[0] = (FIXP_DBL) ~immediate;
  D[1] = (FIXP_DBL) ~immediate;
  D[2] = (FIXP_DBL) ~immediate;
  D[3] = (FIXP_DBL) ~immediate;
}
static void inline FDK_vmvn_d_i32(FIXP_H dst, INT64 immediate)
{
  FIXP_D D = (FIXP_D) dst;
  FDK_check_move_immediate(64, 32, -1, immediate);
  D[0] = (FIXP_DBL) ~immediate;
  D[1] = (FIXP_DBL) ~immediate;
}
static void inline FDK_vmov_q_f32(FIXP_Q dst, FLOAT immediate)
{
  FLOAT* D = (FLOAT*)dst;
  FDK_check_move_float_immediate(immediate);
  D[0] = (FLOAT)immediate;
  D[1] = (FLOAT)immediate;
  D[2] = (FLOAT)immediate;
  D[3] = (FLOAT)immediate;
}

static void inline FDK_vmov_d_f32(FIXP_H dst, FLOAT immediate)
{
  FLOAT* D = (FLOAT*)dst;
  FDK_check_move_float_immediate(immediate);
  D[0] = (FLOAT)immediate;
  D[1] = (FLOAT)immediate;
}
static void inline __FDK_vmov_reg(LONG &Rd, FIXP_D Sn)
{
  Rd = (LONG)Sn[0];
}
static void inline __FDK_vmov_sn(FIXP_D Sn, LONG *Rd)
{
  Sn[0] = *Rd;
}
static void inline __FDK_vmov_sn(FIXP_D Sn, LONG Rd)
{
    Sn[0] = Rd;
}
static void inline __FDK_vmov_dreg(LONG *Rd1, LONG *Rd2, FIXP_D Sm1, FIXP_D Sm2)
{
    FDK_ASSERT(Sm2 == &Sm1[1]);
    *Rd1 = (LONG)Sm1[0];
    *Rd2 = (LONG)Sm2[0];
}
static void inline __FDK_vmov_dreg(LONG &Rd1, LONG &Rd2, FIXP_D Sm1, FIXP_D Sm2)
{
    FDK_ASSERT(Sm2 == &Sm1[1]);
    Rd1 = (LONG)Sm1[0];
    Rd2 = (LONG)Sm2[0];
}
static void inline __FDK_vmov_dsn(FIXP_D Sn1, FIXP_D Sn2, LONG Rd1, LONG Rd2)
{
    FDK_ASSERT(Sn2 == &Sn1[1]);
    Sn1[0] = Rd1;
    Sn2[0] = Rd2;
}
#endif


#ifdef __ARM_NEON__
#define FDK_vcgt_s32_q(dst, src1, src2)  "VCGT.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vcgt_s32_d(dst, src1, src2)  "VCGT.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vcge_s32_q(dst, src1, src2)  "VCGE.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vcge_s32_d(dst, src1, src2)  "VCGE.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define VC_EQ   0
#define VC_GE   1
#define VC_GT   2
#define VC_LE   3
#define VC_LT   4

#define DT_SIGNED    0
#define DT_UNSIGNED  1
#define DT_FLOAT     2

static void FDK_vc (INT comparison, INT dt, INT width, INT size, FIXP_Q dst, FIXP_Q src1, FIXP_Q src2);

#define FDK_vcgt_s32_q(dst, src1, src2)  {    FDK_vc(VC_GT, DT_SIGNED, 32, 128, (FIXP_Q) dst, (FIXP_Q) src1, (FIXP_Q) src2);   }
#define FDK_vcgt_s32_d(dst, src1, src2)  {    FDK_vc(VC_GT, DT_SIGNED, 32,  64, (FIXP_Q) dst, (FIXP_Q) src1, (FIXP_Q) src2);   }
#define FDK_vcge_s32_q(dst, src1, src2)  {    FDK_vc(VC_GE, DT_SIGNED, 32, 128, (FIXP_Q) dst, (FIXP_Q) src1, (FIXP_Q) src2);   }
#define FDK_vcge_s32_d(dst, src1, src2)  {    FDK_vc(VC_GE, DT_SIGNED, 32,  64, (FIXP_Q) dst, (FIXP_Q) src1, (FIXP_Q) src2);   }

static inline void FDK_vc (INT comparison, INT dt, INT width, INT size, FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  INT num = size / width;

  if (comparison == VC_GT)
  {
    if (dt == DT_SIGNED)
    {
      /* signed data types */
      FDK_ASSERT((width == 8) || (width == 16) || (width == 32));
      switch (width)
      {
        case 32:
        {
            INT *Dst  = (INT *) dst;
            INT *Src1 = (INT *) src1;
            INT *Src2 = (INT *) src2;
            for (INT i = 0; i < num; i++)
                Dst[i] = (Src1[i] > Src2[i]) ? (INT) -1 : (INT) 0;
        }
        break;
        case 16:
        {
            SHORT *Dst  = (SHORT *) dst;
            SHORT *Src1 = (SHORT *) src1;
            SHORT *Src2 = (SHORT *) src2;
            for (INT i = 0; i < num; i++)
                Dst[i] = (Src1[i] > Src2[i]) ? (SHORT) -1 : (SHORT) 0;
        break;
        }
        case 8:
        {
            SCHAR *Dst  = (SCHAR *) dst;
            SCHAR *Src1 = (SCHAR *) src1;
            SCHAR *Src2 = (SCHAR *) src2;
            for (INT i = 0; i < num; i++)
                Dst[i] = (Src1[i] > Src2[i]) ? (SCHAR) -1 : (SCHAR) 0;
        }
        break;
      }
    }
  }
  else if (comparison == VC_GE)
  {
    if (dt == DT_SIGNED)
    {
      /* signed data types */
      FDK_ASSERT((width == 8) || (width == 16) || (width == 32));
      switch (width)
      {
        case 32:
        {
            INT *Dst  = (INT *) dst;
            INT *Src1 = (INT *) src1;
            INT *Src2 = (INT *) src2;
            for (INT i = 0; i < num; i++)
                Dst[i] = (Src1[i] >= Src2[i]) ? (INT) -1 : (INT) 0;
        }
        break;
        case 16:
        {
            SHORT *Dst  = (SHORT *) dst;
            SHORT *Src1 = (SHORT *) src1;
            SHORT *Src2 = (SHORT *) src2;
            for (INT i = 0; i < num; i++)
                Dst[i] = (Src1[i] >= Src2[i]) ? (SHORT) -1 : (SHORT) 0;
        break;
        }
        case 8:
        {
            SCHAR *Dst  = (SCHAR *) dst;
            SCHAR *Src1 = (SCHAR *) src1;
            SCHAR *Src2 = (SCHAR *) src2;
            for (INT i = 0; i < num; i++)
                Dst[i] = (Src1[i] >= Src2[i]) ? (SCHAR) -1 : (SCHAR) 0;
        }
        break;
      }
    }
  }
}


#endif

#ifdef __ARM_NEON__
#define FDK_vneg_q( size, dst, src)  "VNEG.S" #size " " #dst ", " #src " \n\t"
#define FDK_vneg_d( size, dst, src)  "VNEG.S" #size " " #dst ", " #src " \n\t"
#define FDK_vneg_f32_q(dst, src)     "VNEG.F32 " #dst ", " #src " \n\t"
#define FDK_vneg_f32_d(dst, src)     "VNEG.F32 " #dst ", " #src " \n\t"
#define FDK_vqneg_q(size, dst, src)  "VQNEG.S" #size " " #dst ", " #src " \n\t"
#define FDK_vqneg_d(size, dst, src)  "VQNEG.S" #size " " #dst ", " #src " \n\t"
#else
#define FDK_vneg_q(size, dst, src)  { if (size == 32) FDK_vneg_s32_q(dst, src);  else if (size == 16) FDK_vneg_s16_q(dst, src); }
#define FDK_vneg_d(size, dst, src)  { if (size == 32) FDK_vneg_s32_d(dst, src);  else if (size == 16) FDK_vneg_s16_d(dst, src); }
#define FDK_vneg_f32_q(dst, src)    __FDK_vneg_f32(128, dst, src);  
#define FDK_vneg_f32_d(dst, src)    __FDK_vneg_f32( 64, (FLOAT*) dst, (FLOAT *)src);  
#define FDK_vqneg_q(size, dst, src) { if (size == 32) FDK_vqneg_s32_q(dst, src); else if (size == 16) FDK_vqneg_s16_q(dst, src); }
#define FDK_vqneg_d(size, dst, src) { if (size == 32) FDK_vqneg_s32_d(dst, src); else if (size == 16) FDK_vqneg_s16_d(dst, src); }

static void inline __FDK_vneg_f32(INT size, FLOAT *dst, FLOAT *src)
{
    INT num = size / 32;
    for (INT i = 0; i < num; i++)
    {
        dst[i] = -src[i];
    }
}

static void inline FDK_vneg_s32_q (FIXP_Q dst, FIXP_Q src)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  for(int i = 0; i < 4; i++)
  {
    FDK_check_s32_overflow((INT64) -Src[i]);
    Dst[i] = -Src[i];
  }
}
static void inline FDK_vneg_s32_d (FIXP_H dst, FIXP_H src)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  for(int i = 0; i < 2; i++)
  {
    FDK_check_s32_overflow((INT64) -Src[i]);
    Dst[i] = -Src[i];
  }
}
static void inline FDK_vneg_s16_q (FIXP_Q dst, FIXP_Q src)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  for(int i = 0; i < 8; i++)
  {
    FDK_check_s16_overflow((FIXP_DBL) -Src[i]);
    Dst[i] = -Src[i];
  }
}
static void inline FDK_vneg_s16_d (FIXP_H dst, FIXP_H src)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  for(int i = 0; i < 4; i++)
  {
    FDK_check_s16_overflow((FIXP_DBL) -Src[i]);
    Dst[i] = -Src[i];
  }
}

static void inline FDK_vqneg_s32_q (FIXP_Q dst, FIXP_Q src)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fQNeg32(Src[i]);
}
static void inline FDK_vqneg_s32_d (FIXP_H dst, FIXP_H src)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  for(int i = 0; i < 2; i++)
    Dst[i] = fQNeg32(Src[i]);
}
static void inline FDK_vqneg_s16_q (FIXP_Q dst, FIXP_Q src)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  for(int i = 0; i < 8; i++)
    Dst[i] = fQNeg16(Src[i]);
}
static void inline FDK_vqneg_s16_d (FIXP_H dst, FIXP_H src)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fQNeg16(Src[i]);
}
#endif

#ifdef __ARM_NEON__
#define FDK_vabs_f32_d(dst, src)     "VABS.F32 " #dst ", " #src "\n\t"
#define FDK_vabs_f32_q(dst, src)     "VABS.F32 " #dst ", " #src "\n\t"
#define FDK_vabs_q( size, dst, src)  "VABS.S" #size " " #dst ", " #src " \n\t"
#define FDK_vabs_d( size, dst, src)  "VABS.S" #size " " #dst ", " #src " \n\t"
#define FDK_vqabs_q(size, dst, src)  "VQABS.S" #size " " #dst ", " #src " \n\t"
#define FDK_vqabs_d(size, dst, src)  "VQABS.S" #size " " #dst ", " #src " \n\t"
#else
#define FDK_vabs_f32_d(dst, src)   { FDK_vabs_f32((INT) 64,  (FLOAT *) dst, (FLOAT *)src); }
#define FDK_vabs_f32_q(dst, src)   { FDK_vabs_f32((INT) 128, (FLOAT *) dst, (FLOAT *)src); }
#define FDK_vabs_q(size, dst, src) { if (size == 32)  FDK_vabs_s32_q(dst, src);   else if (size == 16) FDK_vabs_s16_q(dst, src); }
#define FDK_vabs_d(size, dst, src) { if (size == 32)  FDK_vabs_s32_d(dst, src);   else if (size == 16) FDK_vabs_s16_d(dst, src); }
#define FDK_vqabs_q(size, dst, src){ if (size == 32)  FDK_vqabs_s32_q(dst, src);  else if (size == 16) FDK_vqabs_s16_q(dst, src); }
#define FDK_vqabs_d(size, dst, src){ if (size == 32)  FDK_vqabs_s32_d(dst, src);  else if (size == 16) FDK_vqabs_s16_d(dst, src); }

static void  FDK_vabs_f32(INT size, FLOAT *dst, FLOAT *src)
{
  int i;
  INT num = size >> 5;
  for (i = 0; i < num; i++)
  {
      dst[i] = src[i];
  }
}

static void inline FDK_vabs_s32_q(FIXP_Q dst, FIXP_Q src)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fAbs(Src[i]);
}
static void inline FDK_vabs_s32_d(FIXP_H dst, FIXP_H src)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  for(int i = 0; i < 2; i++)
    Dst[i] = fAbs(Src[i]);
}
static void inline FDK_vabs_s16_q(FIXP_Q dst, FIXP_Q src)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  for(int i = 0; i < 8; i++)
    Dst[i] = fAbs(Src[i]);
}
static void inline FDK_vabs_s16_d(FIXP_H dst, FIXP_H src)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fAbs(Src[i]);
}
static void inline FDK_vqabs_s32_q(FIXP_Q dst, FIXP_Q src)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fQAbs32(Src[i]);
}
static void inline FDK_vqabs_s32_d(FIXP_H dst, FIXP_H src)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  for(int i = 0; i < 2; i++)
    Dst[i] = fQAbs32(Src[i]);
}
static void inline FDK_vqabs_s16_q(FIXP_Q dst, FIXP_Q src)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  for(int i = 0; i < 8; i++)
    Dst[i] = fQAbs16(Src[i]);
}
static void inline FDK_vqabs_s16_d(FIXP_H dst, FIXP_H src)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  for(int i = 0; i < 4; i++)
    Dst[i] = fQAbs16(Src[i]);
}
#endif


#ifndef __ARM_NEON__
static INT64 FDK_sat_shl_s64(INT64 val, INT immediate)
{
  FDK_ASSERT(immediate > -64);
  FDK_ASSERT(immediate < 64);
  if (val && immediate)
  {
     INT64 valPos = (val < (INT64) 0) ? ~val : val;
     FIXP_DBL valH = (FIXP_DBL) (valPos >> 32);
     FIXP_DBL valL = (FIXP_DBL) (valPos);


     INT lbits = valH ? CountLeadingBits(valH) : 32;
     if (lbits == 32)
     {   /* check also lower part */
       if (valL >= 0)
       {
         lbits += valL ? CountLeadingBits(valL) : 31;
       }
       else
       {
         /* upper part shows 0x00000000, lower part starts with binary '1', shift is limited to 31 then */
         lbits--;
       }
     }
     if (immediate > lbits)
     {
       if (val > 0)  val = MAXVAL_INT64;
       else          val = MINVAL_INT64;
     }
     else
     {
       if (immediate >= 0)
         val <<= immediate;
       else
         val >>= -immediate;
     }
  }
  return val;
}
static FIXP_DBL FDK_sat_shl_s32(FIXP_DBL val, INT immediate)
{
  FDK_ASSERT(immediate > -32);
  FDK_ASSERT(immediate < 32);
  if (val && immediate)
  {
     INT lbits = CountLeadingBits(val);
     if (immediate > lbits)
     {
       if (val > 0)  val = (FIXP_DBL) 0x7FFFFFFF;
       else          val = (FIXP_DBL) 0x80000000;
     }
     else
     {
       if (immediate >= 0)
         val <<= immediate;
       else
         val >>= -immediate;
     }
  }
  return val;
}
static FIXP_SGL FDK_sat_shl_s16(FIXP_SGL val, INT immediate)
{
  FDK_ASSERT(immediate > -16);
  FDK_ASSERT(immediate < 16);
  if (val && immediate)
  {
     INT lbits = CountLeadingBits(val);
     if (immediate > lbits)
     {
       if (val > 0)  val = FIXP_SGL(0x7FFF);
       else          val = FIXP_SGL(-0x8000);
     }
     else
     {
       if (immediate >= 0)
         val <<= immediate;
       else
         val >>= -immediate;
     }
  }
  return val;
}

static FIXP_DBL FDK_sat_shl_rnd_s32(FIXP_DBL val, INT immediate)
{
  FDK_ASSERT(immediate > -32);
  FDK_ASSERT(immediate < 32);
  if (val && immediate)
  {
     INT lbits = CountLeadingBits(val);
     if (immediate > lbits)
     {
       if (val > 0)  val = (FIXP_DBL) 0x7FFFFFFF;
       else          val = (FIXP_DBL) 0x80000000;
     }
     else
     {
       if (immediate >= 0)
         val <<= immediate;
       else
       {
         /* round result, when shifting right */
         val >>= (-immediate - 1);
         val = (val + 1) >> 1;
       }
     }
  }
  return val;
}



static FIXP_DBL FDK_sat_add_s32(FIXP_DBL val1, FIXP_DBL val2)
{
  FIXP_DBL retval = val1 + val2;
  if ((val1 ^ val2) >= 0)
  {
    /* both values have the same sign, check, if sign has changed */
    if ((retval ^ val1) < 0)
    {
      /* sign has changed */
      if (val1 > 0)
        retval = MAXVAL_DBL;
      else
        retval = MINVAL_DBL;
    }
  }
  return retval;
}

static FIXP_DBL FDK_sat_sub_s32(FIXP_DBL val1, FIXP_DBL val2)
{
  FIXP_DBL retval = val1 - val2;
  if ((val1 ^ val2) < 0)
  {
    /* values have different sign, check, if sign has changed */
    if ((retval ^ val1) < 0)
    {
      /* sign has changed */
      if (val1 > 0)
        retval = MAXVAL_DBL;
      else
        retval = MINVAL_DBL;
    }
  }
  return retval;
}
static INT64 FDK_sat_add_s64(INT64 val1, INT64 val2)
{
  INT64 retval = val1 + val2;
  if ((val1 ^ val2) >= 0)
  {
    /* both values have the same sign, check, if sign has changed compared to val1 */
    /* example: 0x3000 + 0x5000 = 0x8000 -> 0x7FFF */
    /* example: 0xD000 + 0xAFFF = 0x7FFF -> 0x8000 */
    if ((retval ^ val1) < 0)
    {
      /* sign has changed */
      if (val1 > 0) retval = MAXVAL_INT64;
      else          retval = MINVAL_INT64;
    }
  }
  return retval;
}
static INT64 FDK_sat_sub_s64(INT64 val1, INT64 val2)
{
  INT64 retval = val1 - val2;
  if ((val1 ^ val2) < 0)
  {
    /* values have different sign, check, if sign has changed compared to val1 */
    /* example: 0x4000 - 0xC000 = 0x8000 -> 0x7FFF */
    /* example: 0xC000 - 0x4001 = 0x7FFF -> 0x8000 */
    if ((retval ^ val1) < 0)
    {
      /* sign has changed */
      if (val1 > 0) retval = MAXVAL_INT64;
      else          retval = MINVAL_INT64;
    }
  }
  return retval;
}
#endif

/* reference: NEON and VFP programming, chapter 4.4.3: VDUP */
#ifdef __ARM_NEON__
#define __FDK_vdup_q_8( Qd, Dm_x)        "VDUP.8  " #Qd ", " #Dm_x " \n\t"
#define __FDK_vdup_d_8( Dd, Dm_x)        "VDUP.8  " #Dd ", " #Dm_x " \n\t"
#define __FDK_vdup_q_16(Qd, Dm_x)        "VDUP.16 " #Qd ", " #Dm_x " \n\t"
#define __FDK_vdup_d_16(Dd, Dm_x)        "VDUP.16 " #Dd ", " #Dm_x " \n\t"
#define __FDK_vdup_q_32(Qd, Dm_x)        "VDUP.32 " #Qd ", " #Dm_x " \n\t"
#define __FDK_vdup_d_32(Dd, Dm_x)        "VDUP.32 " #Dd ", " #Dm_x " \n\t"
#define FDK_vdup_q_8( Qd, Dm_x)        __FDK_vdup_q_8( Qd, Dm_x)
#define FDK_vdup_d_8( Dd, Dm_x)        __FDK_vdup_d_8( Dd, Dm_x)
#define FDK_vdup_q_16(Qd, Dm_x)        __FDK_vdup_q_16(Qd, Dm_x)
#define FDK_vdup_d_16(Dd, Dm_x)        __FDK_vdup_d_16(Dd, Dm_x)
#define FDK_vdup_q_32(Qd, Dm_x)        __FDK_vdup_q_32(Qd, Dm_x)
#define FDK_vdup_d_32(Dd, Dm_x)        __FDK_vdup_d_32(Dd, Dm_x)
#define FDK_vdup_q_reg(size, Qd, Rm)   "VDUP." #size " " #Qd ", " #Rm " \n\t"
#define FDK_vdup_d_reg(size, Dd, Rm)   "VDUP." #size " " #Dd ", " #Rm " \n\t"
#else
#define FDK_vdup_q_8(Qd,Dm_x)         __FDK_vdup_q_8( Qd,Dm_x);
#define FDK_vdup_d_8(Dd,Dm_x)         __FDK_vdup_d_8( Dd,Dm_x);
#define FDK_vdup_q_16(Qd,Dm_x)        __FDK_vdup_q_16(Qd,Dm_x);
#define FDK_vdup_d_16(Dd,Dm_x)        __FDK_vdup_d_16(Dd,Dm_x);
#define FDK_vdup_q_32(Qd,Dm_x)        __FDK_vdup_q_32(Qd,Dm_x);
#define FDK_vdup_d_32(Dd,Dm_x)        __FDK_vdup_d_32(Dd,Dm_x);
#define FDK_vdup_q_reg(size,Qd,Rm)    __FDK_vdup_q_reg(size, Qd, (INT) Rm);
#define FDK_vdup_d_reg(size,Dd,Rm)    __FDK_vdup_d_reg(size, Dd, (INT) Rm);

static void inline __FDK_vdup_q_8(FIXP_Q Qd, FIXP_C Dm_x)
{
  FIXP_C Qdst = (FIXP_C) Qd;
  for (int i = 0; i < 16; i++)
  {
    Qdst[i] = Dm_x[0];
  }
}
static void  inline __FDK_vdup_d_8(FIXP_H Dd, FIXP_C Dm_x)
{
  FIXP_C Ddst = (FIXP_C) Dd;
  for (int i = 0; i < 8; i++)
  {
    Ddst[i] = Dm_x[0];
  }
}
static void  inline __FDK_vdup_q_16(FIXP_Q Qd, FIXP_S Dm_x)
{
  FIXP_S Qdst = (FIXP_S) Qd;
  for (int i = 0; i < 8; i++)
  {
    Qdst[i] = Dm_x[0];
  }
}
static void  inline __FDK_vdup_d_16(FIXP_H Dd, FIXP_S Dm_x)
{
  FIXP_S Ddst = (FIXP_S) Dd;
  for (int i = 0; i < 4; i++)
  {
    Ddst[i] = Dm_x[0];
  }
}
static void  inline __FDK_vdup_q_32(FIXP_Q Qd, FIXP_D Dm_x)
{
  FIXP_D Qdst = (FIXP_D) Qd;
  for (int i = 0; i < 4; i++)
  {
    Qdst[i] = Dm_x[0];
  }
}
static void  inline __FDK_vdup_d_32(FIXP_H Dd, FIXP_D Dm_x)
{
  FIXP_D Ddst = (FIXP_D) Dd;
  for (int i = 0; i < 2; i++)
  {
    Ddst[i] = Dm_x[0];
  }
}
static void  inline __FDK_vdup_q_reg(INT size, FIXP_Q Qd, LONG Rm)
{
  switch (size)
  {
    case 8:
    {
      FIXP_C Qdst = (FIXP_C) Qd;
      UCHAR imm = (UCHAR) Rm;
      for (int i = 0; i < 16; i++)
      {
        Qdst[i] = imm;
      }
      break;
    }
    case 16:
    {
      FIXP_S Qdst = (FIXP_S) Qd;
      FIXP_SGL imm = (FIXP_SGL) Rm;
      for (int i = 0; i < 8; i++)
      {
        Qdst[i] = imm;
      }
      break;
    }
    case 32:
    {
      FIXP_D Qdst = (FIXP_D) Qd;
      FIXP_DBL imm = (FIXP_DBL) Rm;
      for (int i = 0; i < 4; i++)
      {
        Qdst[i] = imm;
      }
      break;
    }
  }
}
static void  inline __FDK_vdup_d_reg(INT size, FIXP_H Dd, LONG Rm)
{
  switch (size)
  {
    case 8:
    {
      FIXP_C Ddst = (FIXP_C) Dd;
      UCHAR imm = (UCHAR) Rm;
      for (int i = 0; i < 8; i++)
      {
        Ddst[i] = imm;
      }
      break;
    }
    case 16:
    {
      FIXP_S Ddst = (FIXP_S) Dd;
      FIXP_SGL imm = (FIXP_SGL) Rm;
      for (int i = 0; i < 4; i++)
      {
        Ddst[i] = imm;
      }
      break;
    }
    case 32:
    {
      FIXP_D Ddst = (FIXP_D) Dd;
      FIXP_DBL imm = (FIXP_DBL) Rm;
      for (int i = 0; i < 2; i++)
      {
        Ddst[i] = imm;
      }
      break;
    }
  }
}

#endif

#ifdef __ARM_NEON__
/*#define FDK_vqshl_s8_imm( size, dst, src, immediate)   "VQSHL.s8  " #dst ", " #src ", # " #immediate " \n\t" */
  #define __FDK_vqshl_s16_imm(size, dst, src, immediate) "VQSHL.s16 " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vqshl_s32_imm(size, dst, src, immediate)   "VQSHL.s32 " #dst ", " #src ", # " #immediate " \n\t"
/*#define FDK_vqshl_s64_imm(size, dst, src, immediate)   "VQSHL.s64 " #dst ", " #src ", # " #immediate " \n\t" */
/*#define FDK_vqshl_s8( size, dst, src1, src2)           "VQSHL.s8  " #dst ", " #src1 ", " #src2 " \n\t" */
  #define FDK_vqshl_s16(size, dst, src1, src2)           "VQSHL.s16 " #dst ", " #src1 ", " #src2 " \n\t"
  #define FDK_vqshl_s32(size, dst, src1, src2)           "VQSHL.s32 " #dst ", " #src1 ", " #src2 " \n\t"
  #define FDK_vqrshl_s32(size, dst, src1, src2)          "VQRSHL.s32 " #dst ", " #src1 ", " #src2 " \n\t"
  #define FDK_vqshl_s64(size, dst, src1, src2)           "VQSHL.s64 " #dst ", " #src1 ", " #src2 " \n\t"
  #define FDK_vrshl_s16(size, dst, src1, src2)           "VRSHL.s16 " #dst ", " #src1 ", " #src2 " \n\t"
  #define FDK_vrshl_s32(size, dst, src1, src2)           "VRSHL.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqshl_s16_imm(size, dst, src, immediate)    __FDK_vqshl_s16_imm(size, dst, src, immediate)
#define FDK_vqshl_s32_imm(size, dst, src, immediate)    __FDK_vqshl_s32_imm(size, dst, src, immediate)


#else
#define FDK_vqshl_s32(size, dst, src1, src2)           { if (size == 128) { FDK_vqshl_q_s32((FIXP_Q) dst, (FIXP_Q) src1, (FIXP_Q) src2); }   else if (size == 64) { FDK_vqshl_d_s32((FIXP_H) dst, (FIXP_H) src1, (FIXP_H) src2); } }
#define FDK_vqshl_s64(size, dst, src1, src2)           { if (size == 128) { FDK_vqshl_q_s64((FIXP_Q) dst, (FIXP_Q) src1, (FIXP_Q) src2); }   else if (size == 64) { FDK_vqshl_d_s64((FIXP_H) dst, (FIXP_H) src1, (FIXP_H) src2); } }
#define FDK_vqrshl_s32(size, dst, src1, src2)          { if (size == 128) { FDK_vqrshl_q_s32((FIXP_Q) dst, (FIXP_Q) src1, (FIXP_Q) src2); }  else if (size == 64) { FDK_vqrshl_d_s32((FIXP_H) dst, (FIXP_H) src1, (FIXP_H) src2); } }
#define FDK_vqshl_s32_imm(size, dst, src, immediate)   { if (size == 128) { FDK_vqshl_q_s32_imm((FIXP_Q) dst, (FIXP_Q) src, immediate); }    else if (size == 64) { FDK_vqshl_d_s32_imm((FIXP_H) dst, (FIXP_H) src, immediate); } }
#define FDK_vqshl_s16(size, dst, src1, src2)           { if (size == 128) { FDK_vqshl_q_s16((FIXP_Q) dst, (FIXP_Q) src1, (FIXP_Q) src2); }   else if (size == 64) { FDK_vqshl_d_s16((FIXP_H) dst, (FIXP_H) src1, (FIXP_H) src2); } }
#define FDK_vqshl_s16_imm(size, dst, src, immediate)   { if (size == 128) { FDK_vqshl_q_s16_imm((FIXP_Q) dst, (FIXP_Q) src, immediate); }    else if (size == 64) { FDK_vqshl_d_s16_imm((FIXP_H) dst, (FIXP_H) src, immediate); } }
#define FDK_vrshl_s16(size, dst, src1, src2)           { if (size == 128) { FDK_vrshl_q_s16((FIXP_Q) dst, (FIXP_Q) src1, (FIXP_Q) src2); }   else if (size == 64) { FDK_vrshl_d_s16((FIXP_H) dst, (FIXP_H) src1, (FIXP_H) src2); } }
#define FDK_vrshl_s32(size, dst, src1, src2)           { if (size == 128) { FDK_vrshl_q_s32((FIXP_Q) dst, (FIXP_Q) src1, (FIXP_Q) src2); }   else if (size == 64) { FDK_vrshl_d_s32((FIXP_H) dst, (FIXP_H) src1, (FIXP_H) src2); } }

static void inline FDK_vqshl_q_s32(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 4; i++)
  {
    Dst[i] = FDK_sat_shl_s32(Src1[i],(INT) Src2[i]);
  }
}

static void inline FDK_vqshl_q_s64(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  FIXP_W Dst  = (FIXP_W)dst;
  FIXP_W Src1 = (FIXP_W)src1;
  FIXP_W Src2 = (FIXP_W)src2;
  for (int i = 0; i < 2; i++)
  {
    Dst[i] = FDK_sat_shl_s64(Src1[i], (INT)(Src2[i]));
  }
}

static void inline FDK_vqshl_d_s32(FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 2; i++)
  {
    Dst[i] = FDK_sat_shl_s32(Src1[i],(INT) Src2[i]);
  }
}

static void inline FDK_vqshl_d_s64(FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
  FIXP_W Dst  = (FIXP_W)dst;
  FIXP_W Src1 = (FIXP_W)src1;
  FIXP_W Src2 = (FIXP_W)src2;
  Dst[0] = FDK_sat_shl_s64(Src1[0], (INT)(Src2[0]));
}

static void inline FDK_vqrshl_q_s32(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 4; i++)
  {
    Dst[i] = FDK_sat_shl_rnd_s32(Src1[i],(INT) Src2[i]);
  }
}
static void inline FDK_vqrshl_d_s32(FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 2; i++)
  {
    Dst[i] = FDK_sat_shl_rnd_s32(Src1[i],(INT) Src2[i]);
  }
}



static void inline FDK_vqshl_q_s32_imm (FIXP_Q dst, FIXP_Q src, INT immediate)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  for (int i = 0; i < 4; i++)
  {
    Dst[i] = FDK_sat_shl_s32(Src[i],immediate);
  }
}
static void inline FDK_vqshl_d_s32_imm (FIXP_H dst, FIXP_H src, INT immediate)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  for (int i = 0; i < 2; i++)
  {
    Dst[i] = FDK_sat_shl_s32(Src[i],immediate);
  }
}

static void inline FDK_vqshl_q_s16(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  FIXP_S Dst  = (FIXP_S) dst;
  FIXP_S Src1 = (FIXP_S) src1;
  FIXP_S Src2 = (FIXP_S) src2;
  for (int i = 0; i < 8; i++)
  {
    Dst[i] = FDK_sat_shl_s16(Src1[i],(INT) Src2[i]);
  }
}
static void inline FDK_vqshl_d_s16(FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
  FIXP_S Dst  = (FIXP_S) dst;
  FIXP_S Src1 = (FIXP_S) src1;
  FIXP_S Src2 = (FIXP_S) src2;
  for (int i = 0; i < 4; i++)
  {
    Dst[i] = FDK_sat_shl_s16(Src1[i],(INT) Src2[i]);
  }
}
static void inline FDK_vqshl_q_s16_imm (FIXP_Q dst, FIXP_Q src, INT immediate)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  for (int i = 0; i < 4; i++)
  {
    Dst[i] = FDK_sat_shl_s16(Src[i],immediate);
  }
}
static void inline FDK_vqshl_d_s16_imm (FIXP_H dst, FIXP_H src, INT immediate)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  for (int i = 0; i < 2; i++)
  {
    Dst[i] = FDK_sat_shl_s16(Src[i],immediate);
  }
}

static FIXP_DBL FDK_round_shr_narrow(INT64 val, INT immediate)
{
    UINT64 rounding_bit = (UINT64) 1 << (immediate - 1);
    val += rounding_bit;
    val >>= immediate;
    return (FIXP_DBL)val;
}
static void inline FDK_vrshl_q_s32(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
    FIXP_D Dst = (FIXP_D)dst;
    FIXP_D Src1 = (FIXP_D)src1;
    FIXP_D Src2 = (FIXP_D)src2;
    for (int i = 0; i < 4; i++)
    {
        INT shift = (INT)Src2[i];
        if (shift >= 0)
            Dst[i] = Src1[i] << shift;
        else
            Dst[i] = (FIXP_DBL)FDK_round_shr_narrow((INT64)Src1[i], -shift);
    }
}
static void inline FDK_vrshl_d_s32(FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
    FIXP_D Dst = (FIXP_D)dst;
    FIXP_D Src1 = (FIXP_D)src1;
    FIXP_D Src2 = (FIXP_D)src2;
    for (int i = 0; i < 2; i++)
    {
        INT shift = (INT)Src2[i];
        if (shift >= 0)
            Dst[i] = Src1[i] << shift;
        else
            Dst[i] = (FIXP_DBL)FDK_round_shr_narrow((INT64)Src1[i], -shift);
    }
}
static void inline FDK_vrshl_q_s16(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
    FIXP_S Dst = (FIXP_S)dst;
    FIXP_S Src1 = (FIXP_S)src1;
    FIXP_S Src2 = (FIXP_S)src2;
    for (int i = 0; i < 8; i++)
    {
        INT shift = (INT)Src2[i];
        if (shift >= 0)
            Dst[i] = Src1[i] << shift;
        else
            Dst[i] = (FIXP_SGL)FDK_round_shr_narrow((INT64)Src1[i], -shift);
    }
}
static void inline FDK_vrshl_d_s16(FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
    FIXP_S Dst = (FIXP_S)dst;
    FIXP_S Src1 = (FIXP_S)src1;
    FIXP_S Src2 = (FIXP_S)src2;
    for (int i = 0; i < 4; i++)
    {
        INT shift = (INT)Src2[i];
        if (shift >= 0)
            Dst[i] = Src1[i] << shift;
        else
            Dst[i] = (FIXP_SGL)FDK_round_shr_narrow((INT64)Src1[i], -shift);
    }
}

#endif

/* reference: NEON and VFP programming, chapter 4.5.5: VLSI and VRSI */
#ifdef __ARM_NEON__
#define FDK_vsri_8( size, dst, src, immediate)    "VSRI.8  " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsri_16(size, dst, src, immediate)    "VSRI.16 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsri_32(size, dst, src, immediate)    "VSRI.32 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsri_64(size, dst, src, immediate)    "VSRI.64 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsli_8( size, dst, src, immediate)    "VSLI.8  " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsli_16(size, dst, src, immediate)    "VSLI.16 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsli_32(size, dst, src, immediate)    "VSLI.32 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsli_64(size, dst, src, immediate)    "VSLI.64 " #dst ", " #src ", # " #immediate " \n\t"
#else
#define FDK_vsri_8( size, dst, src, immediate)   { if (size == 128) { FDK_vsri_q(8, (FIXP_Q) dst, (FIXP_Q) src, immediate); }  else if (size == 64) { FDK_vsri_d(8, (FIXP_H) dst, (FIXP_H) src, immediate); } }
#define FDK_vsri_16(size, dst, src, immediate)   { if (size == 128) { FDK_vsri_q(16,(FIXP_Q) dst, (FIXP_Q) src, immediate); }  else if (size == 64) { FDK_vsri_d(16,(FIXP_H) dst, (FIXP_H) src, immediate); } }
#define FDK_vsri_32(size, dst, src, immediate)   { if (size == 128) { FDK_vsri_q(32,(FIXP_Q) dst, (FIXP_Q) src, immediate); }  else if (size == 64) { FDK_vsri_d(32,(FIXP_H) dst, (FIXP_H) src, immediate); } }
#define FDK_vsri_64(size, dst, src, immediate)   { if (size == 128) { FDK_vsri_q(64,(FIXP_Q) dst, (FIXP_Q) src, immediate); }  else if (size == 64) { FDK_vsri_d(64,(FIXP_H) dst, (FIXP_H) src, immediate); } }
#define FDK_vsli_8( size, dst, src, immediate)   { if (size == 128) { FDK_vsli_q(8, (FIXP_Q) dst, (FIXP_Q) src, immediate); }  else if (size == 64) { FDK_vsli_d(8, (FIXP_H) dst, (FIXP_H) src, immediate); } }
#define FDK_vsli_16(size, dst, src, immediate)   { if (size == 128) { FDK_vsli_q(16,(FIXP_Q) dst, (FIXP_Q) src, immediate); }  else if (size == 64) { FDK_vsli_d(16,(FIXP_H) dst, (FIXP_H) src, immediate); } }
#define FDK_vsli_32(size, dst, src, immediate)   { if (size == 128) { FDK_vsli_q(32,(FIXP_Q) dst, (FIXP_Q) src, immediate); }  else if (size == 64) { FDK_vsli_d(32,(FIXP_H) dst, (FIXP_H) src, immediate); } }
#define FDK_vsli_64(size, dst, src, immediate)   { if (size == 128) { FDK_vsli_q(64,(FIXP_Q) dst, (FIXP_Q) src, immediate); }  else if (size == 64) { FDK_vsli_d(64,(FIXP_H) dst, (FIXP_H) src, immediate); } }

static inline void FDK_vsri_q(INT size, FIXP_Q dst, FIXP_Q src, int immediate)
{
  FDK_ASSERT(immediate >= 1);
  FDK_ASSERT(immediate <= size);
  switch (size)
  {
    case 8:
    {
      FIXP_C Dst = (FIXP_C) dst;
      FIXP_C Src = (FIXP_C) src;
      UCHAR mask = (UCHAR) (0x7F >> (immediate-1));
      for (int i = 0; i < 16; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 16:
    {
      FIXP_S Dst = (FIXP_S) dst;
      FIXP_S Src = (FIXP_S) src;
      FIXP_SGL mask = (FIXP_SGL) (0x7FFF >> (immediate-1));
      for (int i = 0; i < 8; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 32:
    {
      FIXP_D Dst = (FIXP_D) dst;
      FIXP_D Src = (FIXP_D) src;
      FIXP_DBL mask = (FIXP_DBL) (0x7FFFFFFF >> (immediate-1));
      for (int i = 0; i < 4; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 64:
    {
      FIXP_W Dst = (FIXP_W) dst;
      FIXP_W Src = (FIXP_W) src;
      INT64 mask = (INT64) (0x7FFFFFFFFFFFFFFF >> (immediate-1));
      for (int i = 0; i < 2; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
    }
    break;
  }
}

static inline void FDK_vsri_d(INT size, FIXP_H dst, FIXP_H src, int immediate)
{
  FDK_ASSERT(immediate >= 1);
  FDK_ASSERT(immediate <= size);
  switch (size)
  {
    case 8:
    {
      FIXP_C Dst = (FIXP_C) dst;
      FIXP_C Src = (FIXP_C) src;
      UCHAR mask = (UCHAR) (0x7F >> (immediate-1));
      for (int i = 0; i < 8; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 16:
    {
      FIXP_S Dst = (FIXP_S) dst;
      FIXP_S Src = (FIXP_S) src;
      FIXP_SGL mask = (FIXP_SGL) (0x7FFF >> (immediate-1));
      for (int i = 0; i < 4; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 32:
    {
      FIXP_D Dst = (FIXP_D) dst;
      FIXP_D Src = (FIXP_D) src;
      FIXP_DBL mask = (FIXP_DBL) (0x7FFFFFFF >> (immediate-1));
      for (int i = 0; i < 2; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
      break;
    }
    case 64:
    {
      FIXP_W Dst = (FIXP_W) dst;
      FIXP_W Src = (FIXP_W) src;
      INT64 mask = (INT64) (0x7FFFFFFFFFFFFFFF >> (immediate-1));
      for (int i = 0; i < 1; i++)
      {
        Dst[i] = (Dst[i] & ~mask) | ((Src[i] >> immediate) & mask);
      }
    }
    break;
  }
}
#endif

/* reference: NEON and VFP programming, chapter 4.4.8: VSWP */
#ifdef __ARM_NEON__
#define FDK_vswp(size, v0, v1)    "VSWP " #v0 ", " #v1 " \n\t"
#else
#define FDK_vswp(size, v0, v1)   { if (size == 128) { FDK_vswp_q(v0, v1); } else if (size == 64) { FDK_vswp_d(v0,v1); } }

static void inline FDK_vswp_q (FIXP_Q q0, FIXP_Q q1)
{
  FIXP_DBL tmp;
  tmp = q1[0]; q1[0] = q0[0]; q0[0] = tmp;
  tmp = q1[1]; q1[1] = q0[1]; q0[1] = tmp;
  tmp = q1[2]; q1[2] = q0[2]; q0[2] = tmp;
  tmp = q1[3]; q1[3] = q0[3]; q0[3] = tmp;
}
static void inline FDK_vswp_d (FIXP_D d0, FIXP_D d1)
{
  FIXP_DBL tmp;
  tmp = d1[0]; d1[0] = d0[0]; d0[0] = tmp;
  tmp = d1[1]; d1[1] = d0[1]; d0[1] = tmp;
}
#endif

/* reference: NEON and VFP programming, chapter 4.3.1: VAND, VBIC, VEOR, VORN, VORR (register) */
#ifdef __ARM_NEON__
#define FDK_vand(size, dst, src1, src2)   "VAND " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vbic(size, dst, src1, src2)   "VBIC " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_veor(size, dst, src1, src2)   "VEOR " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vorr(size, dst, src1, src2)   "VORR " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vorn(size, dst, src1, src2)   "VORN " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vand(size, dst,  src1, src2)   __FDK_vand((INT)size, (FIXP_D) dst, (FIXP_D) src1, (FIXP_D) src2);
#define FDK_vbic(size, dst,  src1, src2)   __FDK_vbic((INT)size, (FIXP_D) dst, (FIXP_D) src1, (FIXP_D) src2);
#define FDK_veor(size, dst,  src1, src2)   __FDK_veor((INT)size, (FIXP_D) dst, (FIXP_D) src1, (FIXP_D) src2);
#define FDK_vorr(size, dst,  src1, src2)   __FDK_vorr((INT)size, (FIXP_D) dst, (FIXP_D) src1, (FIXP_D) src2);
#define FDK_vorn(size, dst,  src1, src2)   __FDK_vorn((INT)size, (FIXP_D) dst, (FIXP_D) src1, (FIXP_D) src2);

static inline void __FDK_vand(INT size, FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = src1[i] & src2[i];
  }
}
static inline void __FDK_vbic(INT size, FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = src1[i] & (~src2[i]);
  }
}
static inline void __FDK_veor(INT size, FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = src1[i] ^ src2[i];
  }
}
static inline void __FDK_vorr(INT size, FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = src1[i] | src2[i];
  }
}
static inline void __FDK_vorn(INT size, FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = src1[i] | (~src2[i]);
  }
}
#endif /*__ARM_NEON__ */


#ifdef __ARM_NEON__
#define FDK_vmax_s32(size, dst, src1, src2)     "VMAX.S32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vmax_f32(size, dst, src1, src2)     "VMAX.F32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vmax_s16(size, dst, src1, src2)     "VMAX.S16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vmin_s32(size, dst, src1, src2)     "VMIN.S32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vmin_s16(size, dst, src1, src2)     "VMIN.S16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpmax_s32(size, dst, src1, src2)    "VPMAX.S32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpmax_s16(size, dst, src1, src2)    "VPMAX.S16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpmin_s32(size, dst, src1, src2)    "VPMIN.S32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpmin_s16(size, dst, src1, src2)    "VPMIN.S16 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vmax_s32(size, dst,  src1, src2)   __FDK_vmax_s32(size, (FIXP_D) dst, (FIXP_D) src1, (FIXP_D) src2);
#define FDK_vmax_f32(size, dst,  src1, src2)   __FDK_vmax_f32(size, (FLOAT*) dst, (FLOAT*) src1, (FLOAT*) src2);
#define FDK_vmax_s16(size, dst,  src1, src2)   __FDK_vmax_s16(size, (FIXP_S) dst, (FIXP_S) src1, (FIXP_S) src2);
#define FDK_vmin_s32(size, dst,  src1, src2)   __FDK_vmin_s32(size, (FIXP_D) dst, (FIXP_D) src1, (FIXP_D) src2);
#define FDK_vmin_s16(size, dst,  src1, src2)   __FDK_vmin_s16(size, (FIXP_S) dst, (FIXP_S) src1, (FIXP_S) src2);
#define FDK_vpmax_s32(size, dst,  src1, src2)  __FDK_vpmax_s32(size, (FIXP_D) dst, (FIXP_D) src1, (FIXP_D) src2);
#define FDK_vpmax_s16(size, dst,  src1, src2)  __FDK_vpmax_s16(size, (FIXP_S) dst, (FIXP_S) src1, (FIXP_S) src2);
#define FDK_vpmin_s32(size, dst,  src1, src2)  __FDK_vpmin_s32(size, (FIXP_D) dst, (FIXP_D) src1, (FIXP_D) src2);
#define FDK_vpmin_s16(size, dst,  src1, src2)  __FDK_vpmin_s16(size, (FIXP_S) dst, (FIXP_S) src1, (FIXP_S) src2);

static inline void __FDK_vmax_s32(INT size, FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = fMax(src1[i], src2[i]);
  }
}
static inline void __FDK_vmax_f32(INT size, FLOAT* dst, FLOAT* src1, FLOAT* src2)
{
  for (int i = 0; i < (size >> 5); i++)
  {
    dst[i] = src1[i] > src2[i] ? src1[i] : src2[i];
  }
}
static inline void __FDK_vmax_s16(INT size, FIXP_S dst, FIXP_S src1, FIXP_S src2)
{
  for (int i = 0; i < (size>>4); i++)
  {
    dst[i] = fMax(src1[i], src2[i]);
  }
}
static inline void __FDK_vmin_s32(INT size, FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  for (int i = 0; i < (size>>5); i++)
  {
    dst[i] = fMin(src1[i], src2[i]);
  }
}
static inline void __FDK_vmin_s16(INT size, FIXP_S dst, FIXP_S src1, FIXP_S src2)
{
  for (int i = 0; i < (size>>4); i++)
  {
  dst[i] = fMin(src1[i], src2[i]);
  }
}

static inline void __FDK_vpmax_s32(INT size, FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  FIXP_DBL tmp[4];
  int i, i0;
  for (i = 0, i0 = 0; i0 < (size >> 6); i0++, i++)
  {
    tmp[i] = fMax(src1[2 * i0], src1[2 * i0 + 1]);
  }
  for (i0 = 0; i0 < (size >> 6); i0++, i++)
  {
    tmp[i] = fMax(src2[2 * i0], src2[2 * i0 + 1]);
  }
  for (i = 0; i < (size >> 5); i++)
  {
    dst[i] = tmp[i];
  }
}

static inline void __FDK_vpmin_s32(INT size, FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  FIXP_DBL tmp[4];
  int i, i0;
  for (i = 0, i0 = 0; i0 < (size >> 6); i0++, i++)
  {
    tmp[i] = fMin(src1[2 * i0], src1[2 * i0 + 1]);
  }
  for (i0 = 0; i0 < (size >> 6); i0++, i++)
  {
    tmp[i] = fMin(src2[2 * i0], src2[2 * i0 + 1]);
  }
  for (i = 0; i < (size >> 5); i++)
  {
    dst[i] = tmp[i];
  }
}

static inline void __FDK_vpmax_s16(INT size, FIXP_S dst, FIXP_S src1, FIXP_S src2)
{
  FIXP_SGL tmp[8];
  int i, i0;
  for (i = 0, i0 = 0; i0 < (size >> 5); i0++, i++)
  {
    tmp[i] = fMax(src1[2 * i0], src1[2 * i0 + 1]);
  }
  for (i0 = 0; i0 < (size >> 5); i0++, i++)
  {
    tmp[i] = fMax(src2[2 * i0], src2[2 * i0 + 1]);
  }
  for (i = 0; i < (size >> 4); i++)
  {
    dst[i] = tmp[i];
  }
}

static inline void __FDK_vpmin_s16(INT size, FIXP_S dst, FIXP_S src1, FIXP_S src2)
{
  FIXP_SGL tmp[8];
  int i, i0;
  for (i = 0, i0 = 0; i0 < (size >> 5); i0++, i++)
  {
    tmp[i] = fMin(src1[2 * i0], src1[2 * i0 + 1]);
  }
  for (i0 = 0; i0 < (size >> 5); i0++, i++)
  {
    tmp[i] = fMin(src2[2 * i0], src2[2 * i0 + 1]);
  }
  for (i = 0; i < (size >> 4); i++)
  {
    dst[i] = tmp[i];
  }
}
#endif /* __ARM_NEON__ */


#ifdef __ARM_NEON__
#define FDK_vadd_s64_q(dst, src1, src2)   "VADD.I64 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vadd_s64_d(dst, src1, src2)   "VADD.I64 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vadd_s32_q(dst, src1, src2)   "VADD.I32 " #dst ", " #src1 ", " #src2 " \n\t"
/* #define FDK_vadd_s32_q_cond(cond, dst, src1, src2) "VADD" #cond ".I32 " #dst ", " #src1 ", " #src2 " \n\t" */
#define FDK_vadd_s32_d(dst, src1, src2)   "VADD.I32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vraddhn_i32(dst, src1, src2)  "VRADDHN.I32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vadd_s16_q(dst, src1, src2)   "VADD.I16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vadd_s16_d(dst, src1, src2)   "VADD.I16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqadd_s64_q(dst, src1, src2)  "VQADD.s64 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqadd_s64_d(dst, src1, src2)  "VQADD.s64 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqadd_s32_q(dst, src1, src2)  "VQADD.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqadd_s32_d(dst, src1, src2)  "VQADD.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vaddl_s32(dst, src1, src2)    "VADDL.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqadd_s16_q(dst, src1, src2)  "VQADD.s16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqadd_s16_d(dst, src1, src2)  "VQADD.s16 " #dst ", " #src1 ", " #src2 " \n\t"

#define FDK_vadd_f32_q(dst, src1, src2)   "VADD.F32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vadd_f32_d(dst, src1, src2)   "VADD.F32 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vadd_s64_q(dst, src1, src2)    __FDK_vadd_s64_q(dst, src1, src2);
#define FDK_vadd_s64_d(dst, src1, src2)    __FDK_vadd_s64_d(dst, src1, src2);
#define FDK_vadd_s32_q(dst, src1, src2)    __FDK_vadd_s32_q(dst, src1, src2);
/* #define FDK_vadd_s32_q_cond(cond, dst, src1, src2)   { if (__FDK_coreflags_ ## cond) FDK_vadd_s32_q(dst, src1, src2); } */
#define FDK_vadd_s32_d(dst, src1, src2)    __FDK_vadd_s32_d(dst, src1, src2);
#define FDK_vraddhn_i32(dst, src1, src2)   __FDK_vraddhn_i32(dst, src1, src2);
#define FDK_vadd_s16_q(dst, src1, src2)    __FDK_vadd_s16_q(dst, src1, src2);
#define FDK_vadd_s16_d(dst, src1, src2)    __FDK_vadd_s16_d(dst, src1, src2);
#define FDK_vqadd_s64_q(dst, src1, src2)   __FDK_vqadd_s64_q(dst, src1, src2);
#define FDK_vqadd_s64_d(dst, src1, src2)   __FDK_vqadd_s64_d(dst, src1, src2);
#define FDK_vqadd_s32_q(dst, src1, src2)   __FDK_vqadd_s32_q(dst, src1, src2);
#define FDK_vqadd_s32_d(dst, src1, src2)   __FDK_vqadd_s32_d(dst, src1, src2);
#define FDK_vaddl_s32(dst, src1, src2)     __FDK_vaddl_s32(dst, src1, src2);
#define FDK_vqadd_s16_q(dst, src1, src2)   __FDK_vqadd_s16_q(dst, src1, src2);
#define FDK_vqadd_s16_d(dst, src1, src2)   __FDK_vqadd_s16_d(dst, src1, src2);

#define FDK_vadd_f32_q(dst, src1, src2)    __FDK_vadd_f32(128, dst, src1, src2);
#define FDK_vadd_f32_d(dst, src1, src2)    __FDK_vadd_f32(64,  dst, src1, src2);

static inline void __FDK_vadd_f32(INT size, FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  INT num = size / 32;
  float *Dst  = (float *) dst;
  float *Src1 = (float *) src1;
  float *Src2 = (float *) src2;
  for (INT i = 0; i < num; i++)
  {
      Dst[i] = Src1[i] + Src2[i];
  }
}

static inline void __FDK_vadd_s64_q(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  INT64 *Dst  = (INT64 *) dst;
  INT64 *Src1 = (INT64 *) src1;
  INT64 *Src2 = (INT64 *) src2;
  Dst[0] = Src1[0] + Src2[0];
  Dst[1] = Src1[1] + Src2[1];
}
static inline void __FDK_vadd_s64_d(FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
  INT64 *Dst  = (INT64 *) dst;
  INT64 *Src1 = (INT64 *) src1;
  INT64 *Src2 = (INT64 *) src2;
  Dst[0] = Src1[0] + Src2[0];
}
static inline void __FDK_vadd_s32_q(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 4; i++)
  {
    FDK_check_s32_overflow((INT64) Src1[i] + (INT64) Src2[i]);
    Dst[i] = Src1[i] + Src2[i];
  }
}
static inline void __FDK_vadd_s32_d(FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 2; i++)
  {
    FDK_check_s32_overflow((INT64) Src1[i] + (INT64) Src2[i]);
    Dst[i] = Src1[i] + Src2[i];
  }
}

static inline void __FDK_vraddhn_i32(FIXP_H dst, FIXP_Q src1, FIXP_Q src2)
{
  FIXP_S Dst  = (FIXP_S) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 4; i++)
  {
    Dst[i] = (FIXP_SGL)( ( (FIXP_DBL) Src1[i] + (FIXP_DBL) Src2[i] + (FIXP_DBL) 0x000080000) >> 16 );
  }
}

static inline void __FDK_vaddl_s32(FIXP_Q dst, FIXP_H src1, FIXP_H src2)
{
  FIXP_W Dst = (FIXP_W)dst;
  INT64 tmp[2];
  FIXP_D Src1 = (FIXP_D)src1;
  FIXP_D Src2 = (FIXP_D)src2;
  for (int i = 0; i < 2; i++)
  {
    tmp[i] = (INT64)Src1[i] + (INT64)Src2[i];
  }
  for (int i = 0; i < 2; i++)
  {
    Dst[i] = tmp[i];
  }
}

static inline void __FDK_vqadd_s32_d(FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;

  for (int i = 0; i < 2; i++)
  {
    INT64 value = (INT64) Src1[i] + (INT64) Src2[i];

    if (value > (INT64) 0x7FFFFFFF) {
      Dst[i] = 0x7FFFFFFF;
    }
    else if (-value > (INT64) 0x80000000) {
      Dst[i] = 0x80000000;
    }
    else {
      Dst[i] = Src1[i] + Src2[i];
    }
  }
}
static inline void __FDK_vqadd_s32_q(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;

  for (int i = 0; i < 4; i++)
  {
    INT64 value = (INT64) Src1[i] + (INT64) Src2[i];

    if (value > (INT64) 0x7FFFFFFF) {
      Dst[i] = 0x7FFFFFFF;
    }
    else if (-value > (INT64) 0x80000000) {
      Dst[i] = 0x80000000;
    }
    else {
      Dst[i] = Src1[i] + Src2[i];
    }
  }
}

#endif /* __ARM_NEON__ */

#ifdef __ARM_NEON__
#define FDK_vsub_s32_q(dst, src1, src2)   "VSUB.I32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vsub_s32_d(dst, src1, src2)   "VSUB.I32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vsub_f32_q(dst, src1, src2)   "VSUB.F32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vsub_f32_d(dst, src1, src2)   "VSUB.F32 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vsub_s32_q(dst, src1, src2)   __FDK_vsub_s32_q(dst, src1, src2);
#define FDK_vsub_s32_d(dst, src1, src2)   __FDK_vsub_s32_d(dst, src1, src2);
#define FDK_vsub_f32_q(dst, src1, src2)   __FDK_vsub_f32(128, dst, src1, src2);
#define FDK_vsub_f32_d(dst, src1, src2)   __FDK_vsub_f32( 64, dst, src1, src2);

static inline void __FDK_vsub_f32(INT size, FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  INT num = size / 32;
  float *Dst  = (float *) dst;
  float *Src1 = (float *) src1;
  float *Src2 = (float *) src2;
  for (INT i = 0; i < num; i++)
  {
      Dst[i] = Src1[i] - Src2[i];
  }
}


static inline void __FDK_vsub_s32_q(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  for (int i = 0; i < 4; i++)
  {
    FDK_check_s32_overflow((INT64) src1[i] - (INT64) src2[i]);
    dst[i] = src1[i] - src2[i];
  }
}
static inline void __FDK_vsub_s32_d(FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  for (int i = 0; i < 2; i++)
  {
    FDK_check_s32_overflow((INT64) src1[i] - (INT64) src2[i]);
    dst[i] = src1[i] - src2[i];
  }
}
#endif /* __ARM_NEON__ */

#ifdef __ARM_NEON__
#define FDK_vsub_s64_q(dst, src1, src2)   "VSUB.I64 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vsub_s64_d(dst, src1, src2)   "VSUB.I64 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vsub_s64_q(dst, src1, src2)   __FDK_vsub_s64_q(dst, src1, src2);
#define FDK_vsub_s64_d(dst, src1, src2)   __FDK_vsub_s64_d(dst, src1, src2);

static inline void __FDK_vsub_s64_q(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  INT64 *Dst  = (INT64 *) dst;
  INT64 *Src1 = (INT64 *) src1;
  INT64 *Src2 = (INT64 *) src2;
  Dst[0] = Src1[0] - Src2[0];
  Dst[1] = Src1[1] - Src2[1];
}
static inline void __FDK_vsub_s64_d(FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
  INT64 *Dst  = (INT64 *) dst;
  INT64 *Src1 = (INT64 *) src1;
  INT64 *Src2 = (INT64 *) src2;
  Dst[0] = Src1[0] - Src2[0];
}
#endif /* __ARM_NEON__ */


#ifdef __ARM_NEON__
#define FDK_vhadd_s32_q(dst, src1, src2)   "VHADD.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vhadd_s32_d(dst, src1, src2)   "VHADD.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vrhadd_s32_q(dst, src1, src2)  "VRHADD.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vrhadd_s32_d(dst, src1, src2)  "VRHADD.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vhadd_s32_q(dst, src1, src2)   __FDK_vhadd_s32_q(dst, src1, src2);
#define FDK_vhadd_s32_d(dst, src1, src2)   __FDK_vhadd_s32_d(dst, src1, src2);
#define FDK_vrhadd_s32_q(dst, src1, src2)  __FDK_vrhadd_s32_q(dst, src1, src2);
#define FDK_vrhadd_s32_d(dst, src1, src2)  __FDK_vrhadd_s32_d(dst, src1, src2);

static inline void __FDK_vhadd_s32_q(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  dst[0] = (FIXP_DBL)(((INT64) src1[0] + (INT64) src2[0])>>1);
  dst[1] = (FIXP_DBL)(((INT64) src1[1] + (INT64) src2[1])>>1);
  dst[2] = (FIXP_DBL)(((INT64) src1[2] + (INT64) src2[2])>>1);
  dst[3] = (FIXP_DBL)(((INT64) src1[3] + (INT64) src2[3])>>1);
}
static inline void __FDK_vhadd_s32_d(FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  dst[0] = (FIXP_DBL)(((INT64) src1[0] + (INT64) src2[0])>>1);
  dst[1] = (FIXP_DBL)(((INT64) src1[1] + (INT64) src2[1])>>1);
}

static inline void __FDK_vrhadd_s32_q(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  dst[0] = (FIXP_DBL)((1 + (INT64) src1[0] + (INT64) src2[0])>>1);
  dst[1] = (FIXP_DBL)((1 + (INT64) src1[1] + (INT64) src2[1])>>1);
  dst[2] = (FIXP_DBL)((1 + (INT64) src1[2] + (INT64) src2[2])>>1);
  dst[3] = (FIXP_DBL)((1 + (INT64) src1[3] + (INT64) src2[3])>>1);
}
static inline void __FDK_vrhadd_s32_d(FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  dst[0] = (FIXP_DBL)((1 + (INT64) src1[0] + (INT64) src2[0])>>1);
  dst[1] = (FIXP_DBL)((1 + (INT64) src1[1] + (INT64) src2[1])>>1);
}

#endif /* __ARM_NEON__ */

#ifdef __ARM_NEON__
#define FDK_vhsub_s32_q(dst, src1, src2)   "VHSUB.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vhsub_s32_d(dst, src1, src2)   "VHSUB.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vhsub_s32_q(dst, src1, src2)   __FDK_vhsub_s32_q(dst, src1, src2);
#define FDK_vhsub_s32_d(dst, src1, src2)   __FDK_vhsub_s32_d(dst, src1, src2);
static inline void __FDK_vhsub_s32_q(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  dst[0] = (FIXP_DBL)(((INT64) src1[0] - (INT64) src2[0])>>1);
  dst[1] = (FIXP_DBL)(((INT64) src1[1] - (INT64) src2[1])>>1);
  dst[2] = (FIXP_DBL)(((INT64) src1[2] - (INT64) src2[2])>>1);
  dst[3] = (FIXP_DBL)(((INT64) src1[3] - (INT64) src2[3])>>1);
}
static inline void __FDK_vhsub_s32_d(FIXP_D dst, FIXP_D src1, FIXP_D src2)
{
  dst[0] = (FIXP_DBL)(((INT64) src1[0] - (INT64) src2[0])>>1);
  dst[1] = (FIXP_DBL)(((INT64) src1[1] - (INT64) src2[1])>>1);
}
#endif /* __ARM_NEON__ */

#ifdef __ARM_NEON__
#define FDK_vpadd_s32(dst, src1, src2)   "VPADD.s32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpadd_s16(dst, src1, src2)   "VPADD.s16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpadd_s8( dst, src1, src2)   "VPADD.s8  " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vpadd_f32(dst, src1, src2)   "VPADD.f32 " #dst ", " #src1 ", " #src2 " \n\t"
#else
#define FDK_vpadd_s32(dst, src1, src2)   __FDK_vpadd(32, dst, src1, src2);
#define FDK_vpadd_s16(dst, src1, src2)   __FDK_vpadd(16, dst, src1, src2);
#define FDK_vpadd_s8( dst, src1, src2)   __FDK_vpadd(8,  dst, src1, src2);
#define FDK_vpadd_f32(dst, src1, src2)   __FDK_vpadd((float *) dst, (float *) src1, (float *) src2);

static inline void __FDK_vpadd_f32(float * dst, float * src1, float *src2)
{
  float result0, result1;
  result0 = src1[0] + src1[1];
  result1 = src2[0] + src2[1];
  dst[0]  = result0;
  dst[1]  = result1;
}

static inline void __FDK_vpadd(INT size, FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
  switch (size)
  {
    case 8:
    {
      FIXP_C Dst  = (FIXP_C) dst;
      FIXP_C Src1 = (FIXP_C) src1;
      FIXP_C Src2 = (FIXP_C) src2;
      char tmp[8];
      tmp[0] = Src1[0] + Src1[1];
      tmp[1] = Src1[2] + Src1[3];
      tmp[2] = Src1[4] + Src1[5];
      tmp[3] = Src1[6] + Src1[7];
      tmp[4] = Src2[0] + Src2[1];
      tmp[5] = Src2[2] + Src2[3];
      tmp[6] = Src2[4] + Src2[5];
      tmp[7] = Src2[6] + Src2[7];
      for (int i = 0; i < 8; i++)
      {
        Dst[i] = tmp[i];
      }
      break;
    }
    case 16:
    {
      FIXP_S Dst  = (FIXP_S) dst;
      FIXP_S Src1 = (FIXP_S) src1;
      FIXP_S Src2 = (FIXP_S) src2;
      FIXP_SGL tmp[4];
      tmp[0] = Src1[0] + Src1[1];
      tmp[1] = Src1[2] + Src1[3];
      tmp[2] = Src2[0] + Src2[1];
      tmp[3] = Src2[2] + Src2[3];
      for (int i = 0; i < 4; i++)
      {
        Dst[i] = tmp[i];
      }
      break;
    }
    case 32:
    {
      FIXP_D Dst  = (FIXP_D) dst;
      FIXP_D Src1 = (FIXP_D) src1;
      FIXP_D Src2 = (FIXP_D) src2;
      FIXP_DBL tmp[2];
      tmp[0] = Src1[0] + Src1[1];
      tmp[1] = Src2[0] + Src2[1];
      Dst[0] = tmp[0];
      Dst[1] = tmp[1];
      break;
    }
  }
}
#endif /* __ARM_NEON__ */


/* Vector shift right by immediate */
#ifdef __ARM_NEON__
#define __FDK_vshr_s32_q_imm(dst, src, immediate)     "VSHR.s32 " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vshr_s32_d_imm(dst, src, immediate)     "VSHR.s32 " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vshr_u32_d_imm(dst, src, immediate)     "VSHR.u32 " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vshr_u32_q_imm(dst, src, immediate)     "VSHR.u32 " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vshr_s16_q_imm(dst, src, immediate)     "VSHR.s16 " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vshr_s16_d_imm(dst, src, immediate)     "VSHR.s16 " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vqshrn_s64_imm(dst, src, immediate)     "VQSHRN.s64 " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vqshrn_s32_imm(dst, src, immediate)     "VQSHRN.s32 " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vqshrn_s16_imm(dst, src, immediate)     "VQSHRN.s16 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vshr_s32_q_imm(dst, src, immediate)     __FDK_vshr_s32_q_imm(dst, src, immediate)
#define FDK_vshr_s32_d_imm(dst, src, immediate)     __FDK_vshr_s32_d_imm(dst, src, immediate)
#define FDK_vshr_u32_d_imm(dst, src, immediate)     __FDK_vshr_u32_d_imm(dst, src, immediate)
#define FDK_vshr_u32_q_imm(dst, src, immediate)     __FDK_vshr_u32_q_imm(dst, src, immediate)
#define FDK_vshr_s16_q_imm(dst, src, immediate)     __FDK_vshr_s16_q_imm(dst, src, immediate)
#define FDK_vshr_s16_d_imm(dst, src, immediate)     __FDK_vshr_s16_d_imm(dst, src, immediate)
#define FDK_vqshrn_s64_imm(dst, src, immediate)     __FDK_vqshrn_s64_imm(dst, src, immediate)
#define FDK_vqshrn_s32_imm(dst, src, immediate)     __FDK_vqshrn_s32_imm(dst, src, immediate)
#define FDK_vqshrn_s16_imm(dst, src, immediate)     __FDK_vqshrn_s16_imm(dst, src, immediate)
#else
#define FDK_vshr_s32_q_imm(dst, src, immediate)     __FDK_vshr_s32_q_imm((FIXP_Q)dst,(FIXP_Q)src, immediate);
#define FDK_vshr_s32_d_imm(dst, src, immediate)     __FDK_vshr_s32_d_imm((FIXP_H)dst,(FIXP_H)src, immediate);
#define FDK_vshr_u32_q_imm(dst, src, immediate)     __FDK_vshr_u32_q_imm((FIXP_Q)dst,(FIXP_Q)src, immediate);
#define FDK_vshr_u32_d_imm(dst, src, immediate)     __FDK_vshr_u32_d_imm((FIXP_H)dst,(FIXP_H)src, immediate);
#define FDK_vshr_s16_q_imm(dst, src, immediate)     __FDK_vshr_s16_q_imm((FIXP_Q)dst,(FIXP_Q)src, immediate);
#define FDK_vshr_s16_d_imm(dst, src, immediate)     __FDK_vshr_s16_d_imm((FIXP_H)dst,(FIXP_H)src, immediate);
#define FDK_vqshrn_s64_imm(dst, src, immediate)     __FDK_vqshrn_imm(64,(FIXP_H)dst,(FIXP_Q)src, immediate);
#define FDK_vqshrn_s32_imm(dst, src, immediate)     __FDK_vqshrn_imm(32,(FIXP_H)dst,(FIXP_Q)src, immediate);
#define FDK_vqshrn_s16_imm(dst, src, immediate)     __FDK_vqshrn_imm(16,(FIXP_H)dst,(FIXP_Q)src, immediate);

static inline void __FDK_vshr_s32_d_imm(FIXP_H dst, FIXP_H src, INT immediate)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 32);
  for (int i = 0; i < 2; i++)
    Dst[i] = Src[i]>>immediate;
}
static inline void __FDK_vshr_u32_d_imm(FIXP_H dst, FIXP_H src, INT immediate)
{
  FIXP_D Dst = (FIXP_D) dst;
  UINT* Src = (UINT*) src;
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 32);
  for (int i = 0; i < 2; i++)
    Dst[i] = Src[i]>>immediate;
}
static inline void __FDK_vshr_s32_q_imm(FIXP_Q dst, FIXP_Q src, INT immediate)
{
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_D Src = (FIXP_D) src;
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 32);
  for (int i = 0; i < 4; i++)
    Dst[i] = Src[i]>>immediate;
}
static inline void __FDK_vshr_u32_q_imm(FIXP_Q dst, FIXP_Q src, INT immediate)
{
  FIXP_D Dst = (FIXP_D) dst;
  UINT* Src = (UINT*) src;
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 32);
  for (int i = 0; i < 4; i++)
    Dst[i] = Src[i]>>immediate;
}
static inline void __FDK_vshr_s16_d_imm(FIXP_H dst, FIXP_H src, INT immediate)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 16);
  for (int i = 0; i < 4; i++)
    Dst[i] = Src[i]>>immediate;
}
static inline void __FDK_vshr_s16_q_imm(FIXP_Q dst, FIXP_Q src, INT immediate)
{
  FIXP_S Dst = (FIXP_S) dst;
  FIXP_S Src = (FIXP_S) src;
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 16);
  for (int i = 0; i < 8; i++)
    Dst[i] = Src[i]>>immediate;
}

static FIXP_DBL FDK_saturate_shr_narrow_s64(INT64 val, INT immediate)
{
  val >>= immediate;
  if (val)
  {
    INT64 valPos = (val < (INT64) 0) ? ~val : val;
    FIXP_DBL valH = (FIXP_DBL) (valPos >> 32);
    FIXP_DBL valL = (FIXP_DBL) (valPos);
    INT lbits = valH ? CountLeadingBits(valH) : 32;
    if (lbits == 32)
    {   /* check also lower part */
      if (valL >= 0)
      {
        lbits += valL ? CountLeadingBits(valL) : 31;
      }
      else
      {
        /* upper part shows 0x00000000, lower part starts with binary '1', shift is limited to 31 then */
        lbits--;
      }
    }
    if (lbits < 32)
    {
      if (val > 0)  val = (INT64) MAXVAL_DBL;
      else          val = (INT64) MINVAL_DBL;
    }
  }
  return (FIXP_DBL) val;
}
static FIXP_SGL FDK_saturate_shr_narrow_s32(FIXP_DBL val, INT immediate)
{
  val >>= immediate;
  if (val)
  {
    INT lbits = CountLeadingBits(val);
    if (lbits < 16)
    {
      if (val > 0)  val = (FIXP_DBL) MAXVAL_SGL;
      else          val = (FIXP_DBL) MINVAL_SGL;
    }
  }
  return (FIXP_SGL) val;
}
static SCHAR FDK_saturate_shr_narrow_s16(FIXP_SGL val, INT immediate)
{
  val >>= immediate;
  if (val)
  {
    INT lbits = CountLeadingBits(val);
    if (lbits < 8)
    {
      if (val > 0)  val = (FIXP_SGL)  127;
      else          val = (FIXP_SGL) -128;
    }
  }
  return (SCHAR) val;
}

static inline void __FDK_vqshrn_imm(INT size, FIXP_H dst, FIXP_Q src, INT immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < size);
  switch (size)
  {
    case 64:
    {
      INT64 *Src = (INT64 *) src;
      FIXP_D Dst = (FIXP_D) dst;
      FIXP_DBL tmp[2];
      for (int i = 0; i < 2; i++)
        tmp[i] = FDK_saturate_shr_narrow_s64(Src[i], immediate);
      for (int i = 0; i < 2; i++)
        Dst[i] = tmp[i];
      break;
    }
    case 32:
    {
      FIXP_D Src = (FIXP_D) src;
      FIXP_S Dst = (FIXP_S) dst;
      FIXP_SGL tmp[4];
      for (int i = 0; i < 4; i++)
        tmp[i] = FDK_saturate_shr_narrow_s32(Src[i], immediate);
      for (int i = 0; i < 4; i++)
        Dst[i] = tmp[i];
      break;
    }
    case 16:
    {
      FIXP_S Src = (FIXP_S) src;
      FIXP_C Dst = (FIXP_C) dst;
      SCHAR tmp[8];
      for (int i = 0; i < 8; i++)
        tmp[i] = FDK_saturate_shr_narrow_s16(Src[i], immediate);
      for (int i = 0; i < 8; i++)
        Dst[i] = tmp[i];
      break;
    }
  }
}
#endif /* __ARM_NEON__ */


/* Vector shift right by immediate and accumulate */
#ifdef __ARM_NEON__
#define __FDK_vsra_s32_q_imm(dst, src, immediate)   "VSRA.s32 " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vsra_s32_d_imm(dst, src, immediate)   "VSRA.s32 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vsra_s32_q_imm(dst, src, immediate)   __FDK_vsra_s32_q_imm(dst, src, immediate)
#define FDK_vsra_s32_d_imm(dst, src, immediate)   __FDK_vsra_s32_d_imm(dst, src, immediate)
#else
#define FDK_vsra_s32_q_imm(dst, src, immediate)   __FDK_vsra_s32_q_imm(dst, src, immediate);
#define FDK_vsra_s32_d_imm(dst, src, immediate)   __FDK_vsra_s32_d_imm(dst, src, immediate);

static inline void __FDK_vsra_s32_d_imm(FIXP_D dst, FIXP_D src, int immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 32);
  dst[0] += src[0]>>immediate;
  dst[1] += src[1]>>immediate;
}
static inline void __FDK_vsra_s32_q_imm(FIXP_Q dst, FIXP_Q src, int immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < 32);
  dst[0] += src[0]>>immediate;
  dst[1] += src[1]>>immediate;
  dst[2] += src[2]>>immediate;
  dst[3] += src[3]>>immediate;
}
#endif /* __ARM_NEON__ */

/* Vector shift right by immediate with narrowing */
#ifdef __ARM_NEON__
#define FDK_vshrn_imm(size, Dd, Qm, immediate)    "VSHRN.I" #size " " #Dd ", " #Qm ", # " #immediate " \n\t"
#else
#define FDK_vshrn_imm(size, Dd, Qm, immediate)   __FDK_vshrn_imm(size, Dd, Qm, immediate);

static inline void __FDK_vshrn_imm(INT size, FIXP_H Dd, FIXP_Q Qm, INT immediate)
{
  FDK_ASSERT(immediate >= 0);
  FDK_ASSERT(immediate < size);

  FIXP_DBL tmp[4];
  FIXP_D qm = (FIXP_D) Qm;
  tmp[0] = qm[0];
  tmp[1] = qm[1];
  tmp[2] = qm[2];
  tmp[3] = qm[3];
  switch(size)
  {
    case 16:
    {
      FIXP_S Tmp = (FIXP_S) tmp;
      FIXP_C dd  = (FIXP_C) Dd;
      for (int i = 0; i < 8; i++)
        dd[i] = (SCHAR) (Tmp[i]>>immediate);
      break;
    }
    case 32:
    {
      FIXP_D Tmp = (FIXP_D) tmp;
      FIXP_S dd  = (FIXP_S) Dd;
      for (int i = 0; i < 4; i++)
        dd[i] = (FIXP_SGL) (Tmp[i]>>immediate);
      break;
    }
    case 64:
    {
      INT64 *Tmp = (INT64 *) tmp;
      FIXP_D dd  = (FIXP_D) Dd;
      for (int i = 0; i < 2; i++)
        dd[i] = (FIXP_DBL) (Tmp[i]>>immediate);
      break;
    }
  }
}
#endif /* __ARM_NEON__ */

/* Vector shift left by immediate */
#ifdef __ARM_NEON__
#define __FDK_vshl_s16_q_imm(dst, src, immediate)   "VSHL.S16  " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vshl_s16_d_imm(dst, src, immediate)   "VSHL.S16  " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vshl_s32_q_imm(dst, src, immediate)   "VSHL.S32  " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vshl_s32_d_imm(dst, src, immediate)   "VSHL.S32  " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vshll_u8_imm( dst, src, immediate)    "VSHLL.U8  " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vshll_s16_imm(dst, src, immediate)    "VSHLL.S16 " #dst ", " #src ", # " #immediate " \n\t"
#define __FDK_vshll_s32_imm(dst, src, immediate)    "VSHLL.S32 " #dst ", " #src ", # " #immediate " \n\t"
#define FDK_vshl_s16_q(dst, src1, src2)   "VSHL.S16  " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vshl_s16_d(dst, src1, src2)   "VSHL.S16  " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vshl_s32_q(dst, src1, src2)   "VSHL.S32  " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vshl_s32_d(dst, src1, src2)   "VSHL.S32  " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vshl_s16_q_imm(dst, src, immediate)   __FDK_vshl_s16_q_imm(dst, src, immediate)
#define FDK_vshl_s16_d_imm(dst, src, immediate)   __FDK_vshl_s16_d_imm(dst, src, immediate)
#define FDK_vshl_s32_q_imm(dst, src, immediate)   __FDK_vshl_s32_q_imm(dst, src, immediate)
#define FDK_vshl_s32_d_imm(dst, src, immediate)   __FDK_vshl_s32_d_imm(dst, src, immediate)
#define FDK_vshll_u8_imm(  dst, src, immediate)   __FDK_vshll_u8_imm(  dst, src, immediate)
#define FDK_vshll_s16_imm( dst, src, immediate)   __FDK_vshll_s16_imm( dst, src, immediate)
#define FDK_vshll_s32_imm( dst, src, immediate)   __FDK_vshll_s32_imm( dst, src, immediate)


#else
#define FDK_vshl_s16_q_imm(dst, src, immediate)   __FDK_vshl_s16_q_imm(dst, src, immediate);
#define FDK_vshl_s16_d_imm(dst, src, immediate)   __FDK_vshl_s16_d_imm(dst, src, immediate);
#define FDK_vshl_s32_q_imm(dst, src, immediate)   __FDK_vshl_s32_q_imm(dst, src, immediate);
#define FDK_vshl_s32_d_imm(dst, src, immediate)   __FDK_vshl_s32_d_imm(dst, src, immediate);
#define FDK_vshll_u8_imm(  dst, src, immediate)   __FDK_vshll_u8_imm(  dst, src, immediate);
#define FDK_vshll_s16_imm( dst, src, immediate)   __FDK_vshll_s16_imm( dst, src, immediate);
#define FDK_vshll_s32_imm( dst, src, immediate)   __FDK_vshll_s32_imm( dst, src, immediate);
#define FDK_vshl_s16_q(dst, src1, src2)   __FDK_vshl_s16_q(dst, src1, src2);
#define FDK_vshl_s16_d(dst, src1, src2)   __FDK_vshl_s16_d(dst, src1, src2);
#define FDK_vshl_s32_q(dst, src1, src2)   __FDK_vshl_s32_q(dst, src1, src2);
#define FDK_vshl_s32_d(dst, src1, src2)   __FDK_vshl_s32_d(dst, src1, src2);

static inline void __FDK_vshl_s32_d_imm(FIXP_D dst, FIXP_D src, INT immediate)
{
  for (int i = 0; i < 2; i++)
  {
    dst[i] = src[i]<<immediate;
  }
}
static inline void __FDK_vshl_s32_q_imm(FIXP_D dst, FIXP_D src, INT immediate)
{
  for (int i = 0; i < 4; i++)
  {
    dst[i] = src[i]<<immediate;
  }
}
static inline void __FDK_vshl_s16_d_imm(FIXP_S dst, FIXP_S src, INT immediate)
{
  for (int i = 0; i < 4; i++)
  {
    dst[i] = src[i]<<immediate;
  }
}
static inline void __FDK_vshl_s16_q_imm(FIXP_Q dst, FIXP_Q src, INT immediate)
{
  for (int i = 0; i < 8; i++)
  {
    dst[i] = src[i]<<immediate;
  }
}

static inline void __FDK_vshll_s32_imm(FIXP_Q dst, FIXP_H src, INT immediate)
{
  INT64 tmp[2];
  FIXP_D Src = (FIXP_D) src;
  INT64 *Dst = (INT64 *) dst;
  tmp[0] = INT64 (Src[0]) << immediate;
  tmp[1] = INT64 (Src[1]) << immediate;
  Dst[0] = tmp[0];
  Dst[1] = tmp[1];
}
static inline void __FDK_vshll_s16_imm(FIXP_Q dst, FIXP_H src, INT immediate)
{
  FIXP_DBL tmp[4];
  FIXP_D Dst = (FIXP_D) dst;
  FIXP_S Src = (FIXP_S) src;
  for (int i = 0; i < 4; i++)
  {
    tmp[i] = FIXP_DBL (Src[i]) << immediate;
  }
  for (int i = 0; i < 4; i++)
  {
    Dst[i] = tmp[i];
  }
}

static inline void __FDK_vshll_u8_imm(FIXP_Q dst, FIXP_H src, INT immediate)
{
  FIXP_SGL tmp[8];
  FIXP_S Dst = (FIXP_S) dst;
  UCHAR *Src = (UCHAR *) src;
  for (int i = 0; i < 8; i++)
  {
    tmp[i] = (USHORT) (Src[i]) << immediate;
  }
  for (int i = 0; i < 8; i++)
  {
    Dst[i] = tmp[i];
  }
}

static inline void __FDK_vshl_s32_d(FIXP_H dst, FIXP_H src1, FIXP_H src2 )
{
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 2; i++)
  {
    FDK_ASSERT (Src2[i] >= -32);
    FDK_ASSERT (Src2[i] < 32);
    if (Src2[i] < 0)
    {
      Dst[i] = Src1[i] >> -Src2[i];
    }
    else
    {
      Dst[i] = Src1[i] << Src2[i];
    }
  }
}
static inline void __FDK_vshl_s32_q(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 4; i++)
  {
    FDK_ASSERT (Src2[i] >= -32);
    FDK_ASSERT (Src2[i] < 32);
    if (Src2[i] < 0)
    {
      Dst[i] = Src1[i] >> -Src2[i];
    }
    else
    {
      Dst[i] = Src1[i] << Src2[i];
    }
  }
}
static inline void __FDK_vshl_s16_d(FIXP_H dst, FIXP_H src1, FIXP_H src2)
{
  FIXP_S Dst  = (FIXP_S) dst;
  FIXP_S Src1 = (FIXP_S) src1;
  FIXP_S Src2 = (FIXP_S) src2;
  for (int i = 0; i < 4; i++)
  {
    FDK_ASSERT (Src2[i] >= -16);
    FDK_ASSERT (Src2[i] < 16);
    if (Src2[i] < 0)
    {
      Dst[i] = Src1[i] >> -Src2[i];
    }
    else
    {
      Dst[i] = Src1[i] << Src2[i];
    }
  }
}
static inline void __FDK_vshl_s16_q(FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  FIXP_S Dst  = (FIXP_S) dst;
  FIXP_S Src1 = (FIXP_S) src1;
  FIXP_S Src2 = (FIXP_S) src2;
  for (int i = 0; i < 8; i++)
  {
    FDK_ASSERT (Src2[i] >= -16);
    FDK_ASSERT (Src2[i] < 16);
    if (Src2[i] < 0)
    {
      Dst[i] = Src1[i] >> -Src2[i];
    }
    else
    {
      Dst[i] = Src1[i] << Src2[i];
    }
  }
}

#endif /* __ARM_NEON__ */

/* Vector reverse elements */
#ifdef __ARM_NEON__
#define FDK_vrev64_q(size, dst, src)   "VREV64.I" #size " " #dst ", " #src " \n\t"
#define FDK_vrev64_d(size, dst, src)   "VREV64.I" #size " " #dst ", " #src " \n\t"
#define FDK_vrev32_q(size, dst, src)   "VREV32.I" #size " " #dst ", " #src " \n\t"
#define FDK_vrev32_d(size, dst, src)   "VREV32.I" #size " " #dst ", " #src " \n\t"
#else
#define FDK_vrev64_d(size, dst, src)   __FDK_vrev64_d(size, (FIXP_W) dst, (FIXP_W) src);
#define FDK_vrev64_q(size, dst, src)   __FDK_vrev64_q(size, (FIXP_Q) dst, (FIXP_Q) src);
#define FDK_vrev32_d(size, dst, src)   __FDK_vrev32_d(size, (FIXP_W) dst, (FIXP_W) src);
#define FDK_vrev32_q(size, dst, src)   __FDK_vrev32_q(size, (FIXP_Q) dst, (FIXP_Q) src);

static inline void __FDK_vrev64_d(INT size, FIXP_W dst, FIXP_W src)
{
  switch (size)
  {
    case 8:
    {
      SCHAR tmp;
      FIXP_C Dst = (FIXP_C) dst;
      FIXP_C Src = (FIXP_C) src;
      for (int i = 0; i < 4; i++)
      {
        tmp = Src[i];  Dst[i] = Src[8-1-i];  Dst[8-1-i] = tmp;
      }
      break;
    }
    case 16:
    {
      FIXP_SGL tmp;
      FIXP_S Dst = (FIXP_S) dst;
      FIXP_S Src = (FIXP_S) src;
      for (int i = 0; i < 2; i++)
      {
        tmp = Src[i];  Dst[i] = Src[4-1-i];  Dst[4-1-i] = tmp;
      }
      break;
    }
    case 32:
    {
      FIXP_DBL tmp;
      FIXP_D Dst = (FIXP_D) dst;
      FIXP_D Src = (FIXP_D) src;
      tmp = Src[0];   Dst[0] = Src[1];  Dst[1] = tmp;
      break;
    }
  }
}

static inline void __FDK_vrev64_q(INT size, FIXP_Q dst, FIXP_Q src)
{
  __FDK_vrev64_d(size, (FIXP_W) dst+0, (FIXP_W) src+0);
  __FDK_vrev64_d(size, (FIXP_W) dst+1, (FIXP_W) src+1);
}

static inline void __FDK_vrev32_d(INT size, FIXP_W dst, FIXP_W src)
{
  switch (size)
  {
    case 8:
    {
      SCHAR tmp;
      FIXP_C Src = (FIXP_C) src;
      FIXP_C Dst = (FIXP_C) dst;
      tmp = Src[0]; Dst[0] = Src[3]; Dst[3] = tmp;
      tmp = Src[1]; Dst[1] = Src[2]; Dst[2] = tmp;
      tmp = Src[4]; Dst[4] = Src[7]; Dst[7] = tmp;
      tmp = Src[5]; Dst[5] = Src[6]; Dst[6] = tmp;
      break;
    }
    case 16:
    {
      FIXP_SGL tmp;
      FIXP_S Dst = (FIXP_S) dst;
      FIXP_S Src = (FIXP_S) src;
      tmp = Src[0];  Dst[0] = Src[1];  Dst[1] = tmp;
      tmp = Src[2];  Dst[2] = Src[3];  Dst[3] = tmp;
      break;
    }
  }
}

static inline void __FDK_vrev32_q(INT size, FIXP_Q dst, FIXP_Q src)
{
  __FDK_vrev32_d(size, (FIXP_W) dst+0, (FIXP_W) src+0);
  __FDK_vrev32_d(size, (FIXP_W) dst+1, (FIXP_W) src+1);
}



#endif /* __ARM_NEON__ */

#ifdef __ARM_NEON__
#define FDK_vmul_i16_d(Dd, Dn, Dm)        "VMUL.s16  " #Dd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmul_i16_q(Qd, Qn, Qm)        "VMUL.s16  " #Qd ", " #Qn ", " #Qm " \n\t"
#define FDK_vmull_s16(Qd, Dn, Dm)         "VMULL.s16 " #Qd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmlal_s16(Qd, Dn, Dm)         "VMLAL.s16 " #Qd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmlsl_s16(Qd, Dn, Dm)         "VMLSL.s16 " #Qd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmull_s16_scalar(Qd, Dn, Dm)  "VMULL.s16 " #Qd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmlal_s16_scalar(Qd, Dn, Dm)  "VMLAL.s16 " #Qd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmlsl_s16_scalar(Qd, Dn, Dm)  "VMLSL.s16 " #Qd ", " #Dn ", " #Dm " \n\t"
#else
#define FDK_vmul_i16_d(Dd, Dn, Dm)         __FDK_vmul_i16(64,  Dd, Dn, Dm);
#define FDK_vmul_i16_q(Qd, Qn, Qm)         __FDK_vmul_i16(128, Qd, Qn, Qm);
#define FDK_vmull_s16(Qd, Dn, Dm)         __FDK_vmull_s16(Qd, Dn, Dm);
#define FDK_vmlal_s16(Qd, Dn, Dm)         __FDK_vmlal_s16(Qd, Dn, Dm);
#define FDK_vmlsl_s16(Qd, Dn, Dm)         __FDK_vmlsl_s16(Qd, Dn, Dm);
#define FDK_vmull_s16_scalar(Qd, Dn, Sm)  __FDK_vmull_s16_scalar(Qd, Dn, Sm);
#define FDK_vmlal_s16_scalar(Qd, Dn, Sm)  __FDK_vmlal_s16_scalar(Qd, Dn, Sm);
#define FDK_vmlsl_s16_scalar(Qd, Dn, Sm)  __FDK_vmlsl_s16_scalar(Qd, Dn, Sm);

static inline void __FDK_vmul_i16(INT size, FIXP_H Dd, FIXP_H Dn, FIXP_H Dm)
{
  INT num = size / 16;
  FIXP_S Sm = (FIXP_S) Dm;
  FIXP_S Sn = (FIXP_S) Dn;
  FIXP_S Sd = (FIXP_S) Dd;
  for (INT i = 0; i < num; i++)
  {
      Sd[i] = (FIXP_SGL) Sn[i]*Sm[i];
  }
}

static inline void __FDK_vmull_s16(FIXP_Q Qd, FIXP_H Dn, FIXP_H Dm)
{
  FIXP_S Sn = (FIXP_S) Dn;
  FIXP_S Sm = (FIXP_S) Dm;
  FIXP_D Dd = (FIXP_D) Qd;
  FIXP_DBL tmp[4];
  tmp[0] = (LONG) Sn[0] * Sm[0];
  tmp[1] = (LONG) Sn[1] * Sm[1];
  tmp[2] = (LONG) Sn[2] * Sm[2];
  tmp[3] = (LONG) Sn[3] * Sm[3];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
  Dd[2] = tmp[2];
  Dd[3] = tmp[3];
}
static inline void __FDK_vmlal_s16(FIXP_Q Qd, FIXP_H Dn, FIXP_H Dm)
{
  FIXP_S Sn = (FIXP_S) Dn;
  FIXP_S Sm = (FIXP_S) Dm;
  FIXP_D Dd = (FIXP_D) Qd;
  FIXP_DBL tmp[4];
  tmp[0] = Dd[0];
  tmp[1] = Dd[1];
  tmp[2] = Dd[2];
  tmp[3] = Dd[3];
  tmp[0] += (LONG) Sn[0] * Sm[0];
  tmp[1] += (LONG) Sn[1] * Sm[1];
  tmp[2] += (LONG) Sn[2] * Sm[2];
  tmp[3] += (LONG) Sn[3] * Sm[3];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
  Dd[2] = tmp[2];
  Dd[3] = tmp[3];
}
static inline void __FDK_vmlsl_s16(FIXP_Q Qd, FIXP_H Dn, FIXP_H Dm)
{
  FIXP_S Sn = (FIXP_S) Dn;
  FIXP_S Sm = (FIXP_S) Dm;
  FIXP_D Dd = (FIXP_D) Qd;
  FIXP_DBL tmp[4];
  tmp[0] = Dd[0];
  tmp[1] = Dd[1];
  tmp[2] = Dd[2];
  tmp[3] = Dd[3];
  tmp[0] -= (LONG) Sn[0] * Sm[0];
  tmp[1] -= (LONG) Sn[1] * Sm[1];
  tmp[2] -= (LONG) Sn[2] * Sm[2];
  tmp[3] -= (LONG) Sn[3] * Sm[3];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
  Dd[2] = tmp[2];
  Dd[3] = tmp[3];
}
static inline void __FDK_vmull_s16_scalar(FIXP_Q Qd, FIXP_H Dn, FIXP_S Sm)
{
  FIXP_S Sn = (FIXP_S) Dn;
  FIXP_D Dd = (FIXP_D) Qd;
  FIXP_DBL tmp[4];
  tmp[0] = (LONG) Sn[0] * Sm[0];
  tmp[1] = (LONG) Sn[1] * Sm[0];
  tmp[2] = (LONG) Sn[2] * Sm[0];
  tmp[3] = (LONG) Sn[3] * Sm[0];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
  Dd[2] = tmp[2];
  Dd[3] = tmp[3];
}
static inline void __FDK_vmlal_s16_scalar(FIXP_Q Qd, FIXP_H Dn, FIXP_S Sm)
{
  FIXP_S Sn = (FIXP_S) Dn;
  FIXP_D Dd = (FIXP_D) Qd;
  FIXP_DBL tmp[4];
  tmp[0] = Dd[0];
  tmp[1] = Dd[1];
  tmp[2] = Dd[2];
  tmp[3] = Dd[3];
  tmp[0] += (LONG) Sn[0] * Sm[0];
  tmp[1] += (LONG) Sn[1] * Sm[0];
  tmp[2] += (LONG) Sn[2] * Sm[0];
  tmp[3] += (LONG) Sn[3] * Sm[0];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
  Dd[2] = tmp[2];
  Dd[3] = tmp[3];
}
static inline void __FDK_vmlsl_s16_scalar(FIXP_Q Qd, FIXP_H Dn, FIXP_S Sm)
{
  FIXP_S Sn = (FIXP_S) Dn;
  FIXP_D Dd = (FIXP_D) Qd;
  FIXP_DBL tmp[4];
  tmp[0] = Dd[0];
  tmp[1] = Dd[1];
  tmp[2] = Dd[2];
  tmp[3] = Dd[3];
  tmp[0] -= (LONG) Sn[0] * Sm[0];
  tmp[1] -= (LONG) Sn[1] * Sm[0];
  tmp[2] -= (LONG) Sn[2] * Sm[0];
  tmp[3] -= (LONG) Sn[3] * Sm[0];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
  Dd[2] = tmp[2];
  Dd[3] = tmp[3];
}
#endif

/* reference: NEON and VFP programming, chapter 4.7.1 */
#ifdef __ARM_NEON__
#define FDK_vmull_s32(Qd, Dn, Dm)           "VMULL.s32 " #Qd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmlal_s32(Qd, Dn, Dm)           "VMLAL.s32 " #Qd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmlsl_s32(Qd, Dn, Dm)           "VMLSL.s32 " #Qd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmull_s32_scalar(Qd, Dn, Sm)    "VMULL.s32 " #Qd ", " #Dn ", " #Sm " \n\t"
#define FDK_vmlal_s32_scalar(Qd, Dn, Sm)    "VMLAL.s32 " #Qd ", " #Dn ", " #Sm " \n\t"
#define FDK_vmlsl_s32_scalar(Qd, Dn, Sm)    "VMLSL.s32 " #Qd ", " #Dn ", " #Sm " \n\t"

#define __FDK_vmul_s32_q_scalar(Qd, Qn, Sm) "VMUL.s32 " #Qd ", " #Qn ", " #Sm " \n\t"
#define __FDK_vmla_s32_q_scalar(Qd, Qn, Sm) "VMLA.s32 " #Qd ", " #Qn ", " #Sm " \n\t"
#define __FDK_vmls_s32_q_scalar(Qd, Qn, Sm) "VMLS.s32 " #Qd ", " #Qn ", " #Sm " \n\t"
#define __FDK_vmul_s32_d_scalar(Dd, Dn, Sm) "VMUL.s32 " #Dd ", " #Dn ", " #Sm " \n\t"
#define __FDK_vmla_s32_d_scalar(Dd, Dn, Sm) "VMLA.s32 " #Dd ", " #Dn ", " #Sm " \n\t"
#define __FDK_vmls_s32_d_scalar(Dd, Dn, Sm) "VMLS.s32 " #Dd ", " #Dn ", " #Sm " \n\t"

#define __FDK_vmul_f32_q_scalar(Qd, Qn, Sm) "VMUL.f32 " #Qd ", " #Qn ", " #Sm " \n\t"
#define __FDK_vmla_f32_q_scalar(Qd, Qn, Sm) "VMLA.f32 " #Qd ", " #Qn ", " #Sm " \n\t"
#define __FDK_vmls_f32_q_scalar(Qd, Qn, Sm) "VMLS.f32 " #Qd ", " #Qn ", " #Sm " \n\t"
#define __FDK_vmul_f32_d_scalar(Dd, Dn, Sm) "VMUL.f32 " #Dd ", " #Dn ", " #Sm " \n\t"
#define __FDK_vmla_f32_d_scalar(Dd, Dn, Sm) "VMLA.f32 " #Dd ", " #Dn ", " #Sm " \n\t"
#define __FDK_vmls_f32_d_scalar(Dd, Dn, Sm) "VMLS.f32 " #Dd ", " #Dn ", " #Sm " \n\t"

#define FDK_vmul_s32_q(Qd, Qn, Qm)          "VMUL.s32 " #Qd ", " #Qn ", " #Qm " \n\t"
#define FDK_vmla_s32_q(Qd, Qn, Qm)          "VMLA.s32 " #Qd ", " #Qn ", " #Qm " \n\t"
#define FDK_vmls_s32_q(Qd, Qn, Qm)          "VMLS.s32 " #Qd ", " #Qn ", " #Qm " \n\t"
#define FDK_vmul_s32_d(Dd, Dn, Dm)          "VMUL.s32 " #Dd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmla_s32_d(Dd, Dn, Dm)          "VMLA.s32 " #Dd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmls_s32_d(Dd, Dn, Dm)          "VMLS.s32 " #Dd ", " #Dn ", " #Dm " \n\t"

#define FDK_vmul_f32_q(Qd, Qn, Qm)          "VMUL.f32 " #Qd ", " #Qn ", " #Qm " \n\t"
#define FDK_vmla_f32_q(Qd, Qn, Qm)          "VMLA.f32 " #Qd ", " #Qn ", " #Qm " \n\t"
#define FDK_vmls_f32_q(Qd, Qn, Qm)          "VMLS.f32 " #Qd ", " #Qn ", " #Qm " \n\t"
#define FDK_vmul_f32_d(Dd, Dn, Dm)          "VMUL.f32 " #Dd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmla_f32_d(Dd, Dn, Dm)          "VMLA.f32 " #Dd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmls_f32_d(Dd, Dn, Dm)          "VMLS.f32 " #Dd ", " #Dn ", " #Dm " \n\t"
#define FDK_vmul_s32_q_scalar(Qd, Qn, Sm)  __FDK_vmul_s32_q_scalar(Qd, Qn, Sm)
#define FDK_vmla_s32_q_scalar(Qd, Qn, Sm)  __FDK_vmla_s32_q_scalar(Qd, Qn, Sm)
#define FDK_vmls_s32_q_scalar(Qd, Qn, Sm)  __FDK_vmls_s32_q_scalar(Qd, Qn, Sm)
#define FDK_vmul_s32_d_scalar(Dd, Dn, Sm)  __FDK_vmul_s32_q_scalar(Dd, Dn, Sm)
#define FDK_vmla_s32_d_scalar(Dd, Dn, Sm)  __FDK_vmla_s32_q_scalar(Dd, Dn, Sm)
#define FDK_vmls_s32_d_scalar(Dd, Dn, Sm)  __FDK_vmls_s32_q_scalar(Dd, Dn, Sm)

#define FDK_vmul_f32_q_scalar(Qd, Qn, Sm)  __FDK_vmul_f32_q_scalar(Qd, Qn, Sm)
#define FDK_vmla_f32_q_scalar(Qd, Qn, Sm)  __FDK_vmla_f32_q_scalar(Qd, Qn, Sm)
#define FDK_vmls_f32_q_scalar(Qd, Qn, Sm)  __FDK_vmls_f32_q_scalar(Qd, Qn, Sm)
#define FDK_vmul_f32_d_scalar(Dd, Dn, Sm)  __FDK_vmul_f32_q_scalar(Dd, Dn, Sm)
#define FDK_vmla_f32_d_scalar(Dd, Dn, Sm)  __FDK_vmla_f32_q_scalar(Dd, Dn, Sm)
#define FDK_vmls_f32_d_scalar(Dd, Dn, Sm)  __FDK_vmls_f32_q_scalar(Dd, Dn, Sm)
#else
#define FDK_vmull_s32(Qd, Dn, Dm)         __FDK_vmull_s32(Qd, Dn, Dm);
#define FDK_vmlal_s32(Qd, Dn, Dm)         __FDK_vmlal_s32(Qd, Dn, Dm);
#define FDK_vmlsl_s32(Qd, Dn, Dm)         __FDK_vmlsl_s32(Qd, Dn, Dm);
#define FDK_vmull_s32_scalar(Qd, Dn, Sm)  __FDK_vmull_s32_scalar(Qd, Dn, Sm);
#define FDK_vmlal_s32_scalar(Qd, Dn, Sm)  __FDK_vmlal_s32_scalar(Qd, Dn, Sm);
#define FDK_vmlsl_s32_scalar(Qd, Dn, Sm)  __FDK_vmlsl_s32_scalar(Qd, Dn, Sm);
#define FDK_vmul_s32_q_scalar(Qd, Qn, Sm) __FDK_vmul_s32_q_scalar(Qd, Qn, Sm);
#define FDK_vmla_s32_q_scalar(Qd, Qn, Sm) __FDK_vmla_s32_q_scalar(Qd, Qn, Sm);
#define FDK_vmls_s32_q_scalar(Qd, Qn, Sm) __FDK_vmls_s32_q_scalar(Qd, Qn, Sm);
#define FDK_vmul_s32_d_scalar(Dd, Dn, Sm) __FDK_vmul_s32_d_scalar(Dd, Dn, Sm);
#define FDK_vmla_s32_d_scalar(Dd, Dn, Sm) __FDK_vmla_s32_d_scalar(Dd, Dn, Sm);
#define FDK_vmls_s32_d_scalar(Dd, Dn, Sm) __FDK_vmls_s32_d_scalar(Dd, Dn, Sm);
#define FDK_vmul_s32_q(Qd, Qn, Qm)        __FDK_vmul_s32_q(Qd, Qn, Qm);
#define FDK_vmla_s32_q(Qd, Qn, Qm)        __FDK_vmla_s32_q(Qd, Qn, Qm);
#define FDK_vmls_s32_q(Qd, Qn, Qm)        __FDK_vmls_s32_q(Qd, Qn, Qm);
#define FDK_vmul_s32_d(Dd, Dn, Dm)        __FDK_vmul_s32_d(Dd, Dn, Dm);
#define FDK_vmla_s32_d(Dd, Dn, Dm)        __FDK_vmla_s32_d(Dd, Dn, Dm);
#define FDK_vmls_s32_d(Dd, Dn, Dm)        __FDK_vmls_s32_d(Dd, Dn, Dm);
#define FDK_vmul_f32_q(Qd, Qn, Qm)        __FDK_vmul_f32(128, Qd, Qn, Qm);
#define FDK_vmla_f32_q(Qd, Qn, Qm)        __FDK_vmla_f32(128, Qd, Qn, Qm);
#define FDK_vmls_f32_q(Qd, Qn, Qm)        __FDK_vmls_f32(128, Qd, Qn, Qm);
#define FDK_vmul_f32_d(Dd, Dn, Dm)        __FDK_vmul_f32( 64, Dd, Dn, Dm);
#define FDK_vmla_f32_d(Dd, Dn, Dm)        __FDK_vmla_f32( 64, Dd, Dn, Dm);
#define FDK_vmls_f32_d(Dd, Dn, Dm)        __FDK_vmls_f32( 64, Dd, Dn, Dm);
#define FDK_vmul_f32_q_scalar(Qd, Qn, Sm) __FDK_vmul_f32_scalar(128, Qd, Qn, Sm);
#define FDK_vmla_f32_q_scalar(Qd, Qn, Sm) __FDK_vmla_f32_scalar(128, Qd, Qn, Sm);
#define FDK_vmls_f32_q_scalar(Qd, Qn, Sm) __FDK_vmls_f32_scalar(128, Qd, Qn, Sm);
#define FDK_vmul_f32_d_scalar(Dd, Dn, Sm) __FDK_vmul_f32_scalar( 64, Dd, Dn, Sm);
#define FDK_vmla_f32_d_scalar(Dd, Dn, Sm) __FDK_vmla_f32_scalar( 64, Dd, Dn, Sm);
#define FDK_vmls_f32_d_scalar(Dd, Dn, Sm) __FDK_vmls_f32_scalar( 64, Dd, Dn, Sm);

static inline void __FDK_vmul_f32(INT size, FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  INT num = size / 32;
  float *Dst  = (float *) dst;
  float *Src1 = (float *) src1;
  float *Src2 = (float *) src2;
  for (INT i = 0; i < num; i++)
  {
      Dst[i] = Src1[i] * Src2[i];
  }
}

static inline void __FDK_vmla_f32(INT size, FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  INT num = size / 32;
  float *Dst  = (float *) dst;
  float *Src1 = (float *) src1;
  float *Src2 = (float *) src2;
  for (INT i = 0; i < num; i++)
  {
      Dst[i] += Src1[i] * Src2[i];
  }
}

static inline void __FDK_vmls_f32(INT size, FIXP_Q dst, FIXP_Q src1, FIXP_Q src2)
{
  INT num = size / 32;
  float *Dst  = (float *) dst;
  float *Src1 = (float *) src1;
  float *Src2 = (float *) src2;
  for (INT i = 0; i < num; i++)
  {
      Dst[i] -= Src1[i] * Src2[i];
  }
}


static inline void __FDK_vmul_f32_scalar(INT size, FIXP_Q dst, FIXP_Q src1, FIXP_D src2)
{
  INT num = size / 32;
  float *Dst  = (float *) dst;
  float *Src1 = (float *) src1;
  float Scalar = ((float *)src2)[0];
  for (INT i = 0; i < num; i++)
  {
      Dst[i] = Src1[i] * Scalar;
  }
}

static inline void __FDK_vmla_f32_scalar(INT size, FIXP_Q dst, FIXP_Q src1, FIXP_D src2)
{
  INT num = size / 32;
  float *Dst  = (float *) dst;
  float *Src1 = (float *) src1;
  float Scalar = ((float *)src2)[0];
  for (INT i = 0; i < num; i++)
  {
      Dst[i] += Src1[i] * Scalar;
  }
}

static inline void __FDK_vmls_f32_scalar(INT size, FIXP_Q dst, FIXP_Q src1, FIXP_D src2)
{
  INT num = size / 32;
  float *Dst  = (float *) dst;
  float *Src1 = (float *) src1;
  float Scalar = ((float *)src2)[0];
  for (INT i = 0; i < num; i++)
  {
      Dst[i] -= Src1[i] * Scalar;
  }
}


static inline void __FDK_vmull_s32(FIXP_Q Qd, FIXP_H Dn, FIXP_H Dm)
{
  FIXP_D Sn = (FIXP_D) Dn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT64 *Dd = (INT64 *) Qd;
  INT64 tmp[2];
  tmp[0] = (INT64) Sn[0] * Sm[0];
  tmp[1] = (INT64) Sn[1] * Sm[1];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
}
static inline void __FDK_vmlal_s32(FIXP_Q Qd, FIXP_H Dn, FIXP_H Dm)
{
  FIXP_D Sn = (FIXP_D) Dn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT64 *Dd = (INT64 *) Qd;
  INT64 tmp[2];
  tmp[0] = Dd[0];
  tmp[1] = Dd[1];
  tmp[0] += (INT64) Sn[0] * Sm[0];
  tmp[1] += (INT64) Sn[1] * Sm[1];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
}
static inline void __FDK_vmlsl_s32(FIXP_Q Qd, FIXP_H Dn, FIXP_H Dm)
{
  FIXP_D Sn = (FIXP_D) Dn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT64 *Dd = (INT64 *) Qd;
  INT64 tmp[2];
  tmp[0] = Dd[0];
  tmp[1] = Dd[1];
  tmp[0] -= (INT64) Sn[0] * Sm[0];
  tmp[1] -= (INT64) Sn[1] * Sm[1];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
}
static inline void __FDK_vmull_s32_scalar(FIXP_Q Qd, FIXP_H Dn, FIXP_D Dm)
{
  FIXP_D Sn = (FIXP_D) Dn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT64 *Dd = (INT64 *) Qd;
  INT64 tmp[2];
  tmp[0] = (INT64) Sn[0] * Sm[0];
  tmp[1] = (INT64) Sn[1] * Sm[0];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
}
static inline void __FDK_vmlal_s32_scalar(FIXP_Q Qd, FIXP_H Dn, FIXP_D Dm)
{
  FIXP_D Sn = (FIXP_D) Dn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT64 *Dd = (INT64 *) Qd;
  INT64 tmp[2];
  tmp[0] = Dd[0];
  tmp[1] = Dd[1];
  tmp[0] += (INT64) Sn[0] * Sm[0];
  tmp[1] += (INT64) Sn[1] * Sm[0];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
}
static inline void __FDK_vmlsl_s32_scalar(FIXP_Q Qd, FIXP_H Dn, FIXP_D Dm)
{
  FIXP_D Sn = (FIXP_D) Dn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT64 *Dd = (INT64 *) Qd;
  INT64 tmp[2];
  tmp[0] = Dd[0];
  tmp[1] = Dd[1];
  tmp[0] -= (INT64) Sn[0] * Sm[0];
  tmp[1] -= (INT64) Sn[1] * Sm[0];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
}

static inline void __FDK_vmul_s32_d_scalar(FIXP_H Dd, FIXP_H Dn, FIXP_D Dm)
{
  FIXP_D Sn = (FIXP_D) Dn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT tmp[2];
  tmp[0] = (INT) Sn[0] * Sm[0];
  tmp[1] = (INT) Sn[1] * Sm[0];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
}
static inline void __FDK_vmla_s32_d_scalar(FIXP_H Dd, FIXP_H Dn, FIXP_D Dm)
{
  FIXP_D Sn = (FIXP_D) Dn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT tmp[2];
  tmp[0] = Dd[0];
  tmp[1] = Dd[1];
  tmp[0] += (INT) Sn[0] * Sm[0];
  tmp[1] += (INT) Sn[1] * Sm[0];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
}
static inline void __FDK_vmls_s32_d_scalar(FIXP_H Dd, FIXP_H Dn, FIXP_D Dm)
{
  FIXP_D Sn = (FIXP_D) Dn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT tmp[2];
  tmp[0] = Dd[0];
  tmp[1] = Dd[1];
  tmp[0] += (INT) Sn[0] * Sm[0];
  tmp[1] += (INT) Sn[1] * Sm[0];
  Dd[0] = tmp[0];
  Dd[1] = tmp[1];
}
static inline void __FDK_vmul_s32_q_scalar(FIXP_Q Qd, FIXP_Q Qn, FIXP_D Dm)
{
  FIXP_D Sn = (FIXP_D) Qn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT tmp[4];
  tmp[0] = (INT) Sn[0] * Sm[0];
  tmp[1] = (INT) Sn[1] * Sm[0];
  tmp[2] = (INT) Sn[2] * Sm[0];
  tmp[3] = (INT) Sn[3] * Sm[0];
  Qd[0] = tmp[0];
  Qd[1] = tmp[1];
  Qd[2] = tmp[2];
  Qd[3] = tmp[3];
}
static inline void __FDK_vmla_s32_q_scalar(FIXP_Q Qd, FIXP_Q Qn, FIXP_D Dm)
{
  FIXP_D Sn = (FIXP_D) Qn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT tmp[4];
  tmp[0] = Qd[0] + (INT) Sn[0] * Sm[0];
  tmp[1] = Qd[1] + (INT) Sn[1] * Sm[0];
  tmp[2] = Qd[2] + (INT) Sn[2] * Sm[0];
  tmp[3] = Qd[3] + (INT) Sn[3] * Sm[0];
  Qd[0] = tmp[0];
  Qd[1] = tmp[1];
  Qd[2] = tmp[2];
  Qd[3] = tmp[3];
}
static inline void __FDK_vmls_s32_q_scalar(FIXP_Q Qd, FIXP_Q Qn, FIXP_D Dm)
{
  FIXP_D Sn = (FIXP_D) Qn;
  FIXP_D Sm = (FIXP_D) Dm;
  INT tmp[4];
  tmp[0] = Qd[0] - (INT) Sn[0] * Sm[0];
  tmp[1] = Qd[1] - (INT) Sn[1] * Sm[0];
  tmp[2] = Qd[2] - (INT) Sn[2] * Sm[0];
  tmp[3] = Qd[3] - (INT) Sn[3] * Sm[0];
  Qd[0] = tmp[0];
  Qd[1] = tmp[1];
  Qd[2] = tmp[2];
  Qd[3] = tmp[3];
}

static inline void __FDK_vmul_s32_q(FIXP_Q Qd, FIXP_Q Qn, FIXP_Q Qm)
{
  Qd[0] = (INT) Qn[0] * Qm[0];
  Qd[1] = (INT) Qn[1] * Qm[1];
  Qd[2] = (INT) Qn[2] * Qm[2];
  Qd[3] = (INT) Qn[3] * Qm[3];
}

static inline void __FDK_vmla_s32_q(FIXP_Q Qd, FIXP_Q Qn, FIXP_Q Qm)
{
  Qd[0] += (INT) Qn[0] * Qm[0];
  Qd[1] += (INT) Qn[1] * Qm[1];
  Qd[2] += (INT) Qn[2] * Qm[2];
  Qd[3] += (INT) Qn[3] * Qm[3];
}

static inline void __FDK_vmls_s32_q(FIXP_Q Qd, FIXP_Q Qn, FIXP_Q Qm)
{
  Qd[0] -= (INT) Qn[0] * Qm[0];
  Qd[1] -= (INT) Qn[1] * Qm[1];
  Qd[2] -= (INT) Qn[2] * Qm[2];
  Qd[3] -= (INT) Qn[3] * Qm[3];
}

static inline void __FDK_vmul_s32_d(FIXP_D Dd, FIXP_D Dn, FIXP_D Dm)
{
  Dd[0] = (INT) Dn[0] * Dm[0];
  Dd[1] = (INT) Dn[1] * Dm[1];
}

static inline void __FDK_vmla_s32_d(FIXP_D Dd, FIXP_D Dn, FIXP_D Dm)
{
  Dd[0] += (INT) Dn[0] * Dm[0];
  Dd[1] += (INT) Dn[1] * Dm[1];
}

static inline void __FDK_vmls_s32_d(FIXP_D Dd, FIXP_D Dn, FIXP_D Dm)
{
  Dd[0] -= (INT) Dn[0] * Dm[0];
  Dd[1] -= (INT) Dn[1] * Dm[1];
}

#endif

/* reference: NEON and VFP programming, chapter 4.7.4 */
#ifdef __ARM_NEON__
#define FDK_vqdmull_s32(       dst, src1, src2)    "VQDMULL.S32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqdmull_s32_scalar(dst, src1, scalar)  "VQDMULL.S32 " #dst ", " #src1 ", " #scalar " \n\t"
#define FDK_vqdmlal_s32(       dst, src1, src2)    "VQDMLAL.S32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqdmlal_s32_scalar(dst, src1, scalar)  "VQDMLAL.S32 " #dst ", " #src1 ", " #scalar " \n\t"
#define FDK_vqdmlsl_s32(       dst, src1, src2)    "VQDMLSL.S32 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqdmlsl_s32_scalar(dst, src1, scalar)  "VQDMLSL.S32 " #dst ", " #src1 ", " #scalar " \n\t"
#define FDK_vqdmull_s16(       dst, src1, src2)    "VQDMULL.S16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqdmull_s16_scalar(dst, src1, scalar)  "VQDMULL.S16 " #dst ", " #src1 ", " #scalar " \n\t"
#define FDK_vqdmlal_s16(       dst, src1, src2)    "VQDMLAL.S16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqdmlal_s16_scalar(dst, src1, scalar)  "VQDMLAL.S16 " #dst ", " #src1 ", " #scalar " \n\t"
#define FDK_vqdmlsl_s16(       dst, src1, src2)    "VQDMLSL.S16 " #dst ", " #src1 ", " #src2 " \n\t"
#define FDK_vqdmlsl_s16_scalar(dst, src1, scalar)  "VQDMLSL.S16 " #dst ", " #src1 ", " #scalar " \n\t"
#else
#define FDK_vqdmull_s32(       dst, src1, src2)     __FDK_vqdmull_s32(dst, src1, src2);
#define FDK_vqdmull_s32_scalar(dst, src1, scalar)   __FDK_vqdmull_s32_scalar(dst, src1, scalar);
#define FDK_vqdmlal_s32(       dst, src1, src2)     __FDK_vqdmlal_s32(dst, src1, src2);
#define FDK_vqdmlal_s32_scalar(dst, src1, scalar)   __FDK_vqdmlal_s32_scalar(dst, src1, scalar);
#define FDK_vqdmlsl_s32(       dst, src1, src2)     __FDK_vqdmlsl_s32(dst, src1, src2);
#define FDK_vqdmlsl_s32_scalar(dst, src1, scalar)   __FDK_vqdmlsl_s32_scalar(dst, src1, scalar);
#define FDK_vqdmull_s16(       dst, src1, src2)     __FDK_vqdmull_s16(dst, src1, src2);
#define FDK_vqdmull_s16_scalar(dst, src1, scalar)   __FDK_vqdmull_s16_scalar(dst, src1, scalar);
#define FDK_vqdmlal_s16(       dst, src1, src2)     __FDK_vqdmlal_s16(dst, src1, src2);
#define FDK_vqdmlal_s16_scalar(dst, src1, scalar)   __FDK_vqdmlal_s16_scalar(dst, src1, scalar);
#define FDK_vqdmlsl_s16(       dst, src1, src2)     __FDK_vqdmlsl_s16(dst, src1, src2);
#define FDK_vqdmlsl_s16_scalar(dst, src1, scalar)   __FDK_vqdmlsl_s16_scalar(dst, src1, scalar);

static inline void __FDK_vqdmull_s32(FIXP_Q dst, FIXP_H src1, FIXP_H src2)
{
  INT64 tmp[2];
  INT64 *Dst  = (INT64 *) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 2; i++)
    tmp[i] = FDK_sat_shl_s64((INT64)Src1[i] * Src2[i], 1);
  for (int i = 0; i < 2; i++)
    Dst[i] = tmp[i];
}
static inline void __FDK_vqdmull_s32_scalar(FIXP_Q dst, FIXP_H src1, FIXP_D scalar)
{
  INT64 tmp[2];
  INT64 *Dst  = (INT64 *) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  for (int i = 0; i < 2; i++)
    tmp[i] = FDK_sat_shl_s64((INT64)Src1[i] * scalar[0], 1);
  for (int i = 0; i < 2; i++)
    Dst[i] = tmp[i];
}
static inline void __FDK_vqdmull_s16(FIXP_Q dst, FIXP_H src1, FIXP_H src2)
{
  FIXP_DBL tmp[4];
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_S Src1 = (FIXP_S) src1;
  FIXP_S Src2 = (FIXP_S) src2;
  for (int i = 0; i < 4; i++)
    tmp[i] = FDK_sat_shl_s32((FIXP_DBL)Src1[i] * Src2[i], 1);
  for (int i = 0; i < 4; i++)
    Dst[i] = tmp[i];
}
static inline void __FDK_vqdmull_s16_scalar(FIXP_Q dst, FIXP_H src1, FIXP_S scalar)
{
  FIXP_DBL tmp[4];
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_S Src1 = (FIXP_S) src1;
  for (int i = 0; i < 4; i++)
    tmp[i] = FDK_sat_shl_s32((FIXP_DBL)Src1[i] * scalar[0], 1);
  for (int i = 0; i < 4; i++)
    Dst[i] = tmp[i];
}

static inline void __FDK_vqdmlal_s32(FIXP_Q dst, FIXP_H src1, FIXP_H src2)
{
  INT64 tmp[2];
  INT64 *Dst  = (INT64 *) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 2; i++)
    tmp[i] = FDK_sat_add_s64(Dst[i], FDK_sat_shl_s64((INT64)Src1[i] * Src2[i], 1));
  for (int i = 0; i < 2; i++)
    Dst[i] = tmp[i];
}
static inline void __FDK_vqdmlal_s32_scalar(FIXP_Q dst, FIXP_H src1, FIXP_D scalar)
{
  INT64 tmp[2];
  INT64 *Dst  = (INT64 *) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  for (int i = 0; i < 2; i++)
    tmp[i] = FDK_sat_add_s64(Dst[i], FDK_sat_shl_s64((INT64)Src1[i] * scalar[0], 1));
  for (int i = 0; i < 2; i++)
    Dst[i] = tmp[i];
}
static inline void __FDK_vqdmlal_s16(FIXP_Q dst, FIXP_H src1, FIXP_H src2)
{
  FIXP_DBL tmp[4];
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_S Src1 = (FIXP_S) src1;
  FIXP_S Src2 = (FIXP_S) src2;
  for (int i = 0; i < 4; i++)
    tmp[i] = FDK_sat_add_s32(Dst[i], FDK_sat_shl_s32((FIXP_DBL)Src1[i] * Src2[i], 1));
  for (int i = 0; i < 4; i++)
    Dst[i] = tmp[i];
}
static inline void __FDK_vqdmlal_s16_scalar(FIXP_Q dst, FIXP_H src1, FIXP_S scalar)
{
  FIXP_DBL tmp[4];
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_S Src1 = (FIXP_S) src1;
  for (int i = 0; i < 4; i++)
    tmp[i] = FDK_sat_add_s32(Dst[i], FDK_sat_shl_s32((FIXP_DBL)Src1[i] * scalar[0], 1));
  for (int i = 0; i < 4; i++)
    Dst[i] = tmp[i];
}
static inline void __FDK_vqdmlsl_s32(FIXP_Q dst, FIXP_H src1, FIXP_H src2)
{
  INT64 tmp[2];
  INT64 *Dst  = (INT64 *) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  FIXP_D Src2 = (FIXP_D) src2;
  for (int i = 0; i < 2; i++)
    tmp[i] = FDK_sat_sub_s64(Dst[i], FDK_sat_shl_s64((INT64)Src1[i] * Src2[i], 1));
  for (int i = 0; i < 2; i++)
    Dst[i] = tmp[i];
}
static inline void __FDK_vqdmlsl_s32_scalar(FIXP_Q dst, FIXP_H src1, FIXP_D scalar)
{
  INT64 tmp[2];
  INT64 *Dst  = (INT64 *) dst;
  FIXP_D Src1 = (FIXP_D) src1;
  for (int i = 0; i < 2; i++)
    tmp[i] = FDK_sat_sub_s64(Dst[i], FDK_sat_shl_s64((INT64)Src1[i] * scalar[0], 1));
  for (int i = 0; i < 2; i++)
    Dst[i] = tmp[i];
}
static inline void __FDK_vqdmlsl_s16(FIXP_Q dst, FIXP_H src1, FIXP_H src2)
{
  FIXP_DBL tmp[4];
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_S Src1 = (FIXP_S) src1;
  FIXP_S Src2 = (FIXP_S) src2;
  for (int i = 0; i < 4; i++)
    tmp[i] = FDK_sat_sub_s32(Dst[i], FDK_sat_shl_s32((FIXP_DBL)Src1[i] * Src2[i], 1));
  for (int i = 0; i < 4; i++)
    Dst[i] = tmp[i];
}
static inline void __FDK_vqdmlsl_s16_scalar(FIXP_Q dst, FIXP_H src1, FIXP_S scalar)
{
  FIXP_DBL tmp[4];
  FIXP_D Dst  = (FIXP_D) dst;
  FIXP_S Src1 = (FIXP_S) src1;
  for (int i = 0; i < 4; i++)
    tmp[i] = FDK_sat_sub_s32(Dst[i], FDK_sat_shl_s32((FIXP_DBL)Src1[i] * scalar[0], 1));
  for (int i = 0; i < 4; i++)
    Dst[i] = tmp[i];
}
#endif /* __ARM_NEON__ */


/* reference: NEON and VFP programming, chapter 4.7.5 */
#ifdef __ARM_NEON__
#define FDK_vqdmulh_s32_dd(dst, src, factor)  "VQDMULH.s32 " #dst ", " #src ", " #factor " \n\t"
#define FDK_vqdmulh_s32_qq(dst, src, factor)  "VQDMULH.s32 " #dst ", " #src ", " #factor " \n\t"
#define FDK_vqrdmulh_s32_qq(dst, src, factor)  "VQRDMULH.s32 " #dst ", " #src ", " #factor " \n\t"
#define __FDK_vqdmulh_s32_ds(dst, src, factor)  "VQDMULH.s32 " #dst ", " #src ", " #factor " \n\t"
#define __FDK_vqdmulh_s32_qs(dst, src, factor)  "VQDMULH.s32 " #dst ", " #src ", " #factor " \n\t"
#define FDK_vqdmulh_s32_qs(dst, src, factor)  __FDK_vqdmulh_s32_qs(dst, src, factor)
#define FDK_vqdmulh_s32_ds(dst, src, factor)  __FDK_vqdmulh_s32_ds(dst, src, factor)
#else
#define FDK_vqdmulh_s32_qs(dst, src, scalar)  __FDK_vqdmulh_s32_qs(dst, src, scalar);
#define FDK_vqdmulh_s32_qq(dst, src, vector)  __FDK_vqdmulh_s32_qq(dst, src, vector);
#define FDK_vqrdmulh_s32_qq(dst, src, vector)  __FDK_vqrdmulh_s32_qq(dst, src, vector);
#define FDK_vqdmulh_s32_ds(dst, src, scalar)  __FDK_vqdmulh_s32_ds(dst, src, scalar);
#define FDK_vqdmulh_s32_dd(dst, src, vector)  __FDK_vqdmulh_s32_dd(dst, src, vector);

static inline FIXP_DBL ARM_vqdmulh(FIXP_DBL src1, FIXP_DBL src2)
{
   /* Perform 32x32 MPY with doubling, high part result */
   INT64 result = ((INT64) src1 * (INT64) src2) >> 31;
   /* Apply saturation upon max. 64-bit result: (0x80000000 * 0x800000000) >> 31 = 0x40000000.00000000 >> 31 = 0x00000000.80000000 */
   if (result == (INT64) 0x0000000080000000)
       return (FIXP_DBL) 0x7FFFFFFF;
   else
      return (FIXP_DBL) (result);
}

static inline FIXP_DBL ARM_vqrdmulh(FIXP_DBL src1, FIXP_DBL src2)
{
   /* Perform 32x32 MPY with doubling, high part result, saturation and rounding */
   INT64 result = (((INT64)src1 * (INT64)src2) + (INT64)0x40000000) >> 31;
   /* Apply saturation upon max. 64-bit result: (0x80000000 * 0x800000000) >> 31 = 0x40000000.00000000 >> 31 = 0x00000000.80000000 */
   if (result == (INT64) 0x0000000080000000)
       return (FIXP_DBL) 0x7FFFFFFF;
   else
      return (FIXP_DBL) (result);
}


static inline void __FDK_vqdmulh_s32_qq(FIXP_Q dst, FIXP_Q src, FIXP_Q vector)
{
  dst[0] = ARM_vqdmulh(src[0],vector[0]);
  dst[1] = ARM_vqdmulh(src[1],vector[1]);
  dst[2] = ARM_vqdmulh(src[2],vector[2]);
  dst[3] = ARM_vqdmulh(src[3],vector[3]);
}
static inline void __FDK_vqrdmulh_s32_qq(FIXP_Q dst, FIXP_Q src, FIXP_Q vector)
{
  dst[0] = ARM_vqrdmulh(src[0],vector[0]);
  dst[1] = ARM_vqrdmulh(src[1],vector[1]);
  dst[2] = ARM_vqrdmulh(src[2],vector[2]);
  dst[3] = ARM_vqrdmulh(src[3],vector[3]);
}
static inline void __FDK_vqdmulh_s32_dd(FIXP_H dst, FIXP_H src, FIXP_H vector)
{
  dst[0] = ARM_vqdmulh(src[0],vector[0]);
  dst[1] = ARM_vqdmulh(src[1],vector[1]);
}
static inline void __FDK_vqdmulh_s32_qs(FIXP_Q dst, FIXP_Q src, FIXP_D scalar)
{
  FIXP_DBL factor = scalar[0];
  dst[0] = ARM_vqdmulh(src[0],factor);
  dst[1] = ARM_vqdmulh(src[1],factor);
  dst[2] = ARM_vqdmulh(src[2],factor);
  dst[3] = ARM_vqdmulh(src[3],factor);
}
static inline void __FDK_vqdmulh_s32_ds(FIXP_H dst, FIXP_H src, FIXP_D scalar)
{
  FIXP_DBL factor = scalar[0];
  dst[0] = ARM_vqdmulh(src[0],factor);
  dst[1] = ARM_vqdmulh(src[1],factor);
}
#endif /* __ARM_NEON__ */

/* Interleave elements */
#ifdef __ARM_NEON__
#define FDK_vzip_q(size,Qd,Qm)  "VZIP.I" #size " " #Qd ", " #Qm " \n\t"
#define FDK_vzip_d(size,Dd,Dm)  "VZIP.I" #size " " #Dd ", " #Dm " \n\t"
#else
#define FDK_vzip_q(size,Qd,Qm)  __FDK_vzip_q(size,Qd,Qm);
#define FDK_vzip_d(size,Dd,Dm)  __FDK_vzip_d(size,Dd,Dm);

static inline void __FDK_vzip_q(int size, FIXP_Q Qd, FIXP_Q Qm)
{
  switch (size)
  {
    case 8:
      break;
    case 16:
    {
      FIXP_SGL tmp[4];
      FIXP_S Sd = (FIXP_S) Qd;
      FIXP_S Sm = (FIXP_S) Qm;

      tmp[0] = Sd[4];  /* A8 */
      tmp[1] = Sd[5];  /* A7 */
      tmp[2] = Sd[6];  /* A6 */
      tmp[3] = Sd[7];  /* A7 */

      Sd[7]  = Sm[3];
      Sd[6]  = Sd[3];
      Sd[5]  = Sm[2];
      Sd[4]  = Sd[2];
      Sd[3]  = Sm[1];
      Sd[2]  = Sd[1];
      Sd[1]  = Sm[0];

      Sm[0]  = tmp[0];
      Sm[1]  = Sm[4];
      Sm[2]  = tmp[1];
      Sm[3]  = Sm[5];
      Sm[4]  = tmp[2];
      Sm[5]  = Sm[6];
      Sm[6]  = tmp[3];
      break;
    }
    case 32:
    {
      FIXP_DBL tmp[2];
      FIXP_D Dd = (FIXP_D) Qd;
      FIXP_D Dm = (FIXP_D) Qm;

      tmp[0] = Dd[2];  /* A2 */
      tmp[1] = Dd[3];  /* A3 */
      Dd[3]  = Dm[1];
      Dd[2]  = Dd[1];
      Dd[1]  = Dm[0];
      Dm[0]  = tmp[0];
      Dm[1]  = Dm[2];
      Dm[2]  = tmp[1];
      break;
    }
  }
}
static inline void __FDK_vzip_d(int size, FIXP_H Dd, FIXP_H Dm)
{
  switch (size)
  {
    case 8:
      break;
    case 16:
    {
      FIXP_S Sd = (FIXP_S) Dd;
      FIXP_S Sm = (FIXP_S) Dm;
      FIXP_SGL tmp[2];

      tmp[0] = Sd[2];  /* A2 */
      tmp[1] = Sd[3];  /* A3 */
      Sd[3]  = Sm[1];
      Sd[2]  = Sd[1];
      Sd[1]  = Sm[0];
      Sm[0]  = tmp[0];
      Sm[1]  = Sm[2];
      Sm[2]  = tmp[1];
      break;
    }
    case 32:
    {
      FIXP_D Sd = (FIXP_D) Dd;
      FIXP_D Sm = (FIXP_D) Dm;
      FIXP_DBL tmp = Sd[1];

      Sd[1]  = Sm[0];
      Sm[0]  = tmp;
      break;
    }
  }
}
#endif /* __ARM_NEON__ */

/* Deinterleave elements */
#ifdef __ARM_NEON__
#define FDK_vuzp_q(size,Qd,Qm)        "VUZP." #size " " #Qd ", " #Qm " \n\t"
#define FDK_vuzp_d(size,Dd,Dm)        "VUZP." #size " " #Dd ", " #Dm " \n\t"
#define FDK_vext_q(size,Qd,Qn,Qm,idx) "VEXT." #size " " #Qd ", " #Qn ", " #Qm ", #" #idx " \n\t"
#define FDK_vext_d(size,Dd,Dn,Dm,idx) "VEXT." #size " " #Dd ", " #Dn ", " #Dm ", #" #idx " \n\t"
#else
#define FDK_vuzp_q(size,Qd,Qm)  __FDK_vuzp_q(size,Qd,Qm);
#define FDK_vuzp_d(size,Dd,Dm)  __FDK_vuzp_d(size,Dd,Dm);
#define FDK_vext_q(size,Qd,Qn,Qm,idx) __FDK_vext_q(size,Qd,Qn,Qm,idx);
#define FDK_vext_d(size,Dd,Dn,Dm,idx) __FDK_vext_d(size,Dd,Dn,Dm,idx);

static inline void __FDK_vuzp_q(int size, FIXP_Q Qd, FIXP_Q Qm)
{
  switch (size)
  {
    case 8:
      break;
    case 16:
    {
      FIXP_SGL tmp[4];
      FIXP_S Sd = (FIXP_S) Qd;
      FIXP_S Sm = (FIXP_S) Qm;

      tmp[0] = Sm[0];  /* B0 */
      tmp[1] = Sm[1];  /* B1 */
      tmp[2] = Sm[2];  /* B2 */
      tmp[3] = Sm[3];  /* B3 */

      Sm[0] = Sd[1];   /* A1 */
      Sm[1] = Sd[3];   /* A3 */
      Sm[2] = Sd[5];   /* A5 */
      Sm[3] = Sd[7];   /* A7 */
/*    Sd[0] = Sd[0];*/ /* A0 */
      Sd[1] = Sd[2];   /* A2 */
      Sd[2] = Sd[4];   /* A4 */
      Sd[3] = Sd[6];   /* A6 */
      Sd[4] = tmp[0];  /* B0 */
      Sd[5] = tmp[2];  /* B2 */
      Sd[6] = Sm[4];   /* B4 */
      Sd[7] = Sm[6];   /* B6 */
/*    Sm[7] = Sm[7];*/ /* B7 */
      Sm[6] = Sm[5];   /* B5 */
      Sm[5] = tmp[3];  /* B3 */
      Sm[4] = tmp[1];  /* B1 */
      break;
    }
    case 32:
    {
      FIXP_DBL tmp[2];
      FIXP_D Dd = (FIXP_D) Qd;
      FIXP_D Dm = (FIXP_D) Qm;

      tmp[0] = Dm[0];  /* B0 */
      tmp[1] = Dm[1];  /* B1 */
      Dm[0]  = Dd[1];  /* A1 */
      Dd[1]  = Dd[2];  /* A2 */
      Dd[2]  = tmp[0]; /* B0 */
      Dm[1]  = Dd[3];  /* A3 */
      Dd[3]  = Dm[2];  /* B2 */
      Dm[2]  = tmp[1]; /* B1 */
      break;
    }
  }
}
static inline void __FDK_vuzp_d(int size, FIXP_H Dd, FIXP_H Dm)
{
  switch (size)
  {
    case 8:
      break;
    case 16:
    {
      FIXP_SGL tmp[2];
      FIXP_S Sd = (FIXP_S) Dd;
      FIXP_S Sm = (FIXP_S) Dm;

      tmp[0] = Sm[0];  /* B0 */
      tmp[1] = Sm[1];  /* B1 */
      Sm[0]  = Sd[1];  /* A1 */
      Sd[1]  = Sd[2];  /* A2 */
      Sd[2]  = tmp[0]; /* B0 */
      Sm[1]  = Sd[3];  /* A3 */
      Sd[3]  = Sm[2];  /* B2 */
      Sm[2]  = tmp[1]; /* B1 */
      break;
    }
    case 32:
      FIXP_DBL tmp;
      tmp = Dd[1];
      Dd[1]  = Dm[0];
      Dm[0]  = tmp;
      break;
  }
}
static inline void __FDK_vext_q(INT width, FIXP_Q Qd, FIXP_Q Qn, FIXP_Q Qm, INT idx)
{
  FDK_ASSERT (width == 8 || width == 16 || width == 32);
  INT i, num = 128 / width;
  FDK_ASSERT ((idx >=  0) && (idx < num));
  INT64 tmp[2];

  switch(width)
  {
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) &tmp;
      SCHAR *Src1 = (SCHAR *) Qn;
      SCHAR *Src2 = (SCHAR *) Qm;

      for(i = 0; i < num - idx; i++){
        Dst[i] = Src1[i + idx];
      }
      for(i = 0; i < idx; i++){
        Dst[i + num - idx] = Src2[i];
      }
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) &tmp;
      SHORT *Src1 = (SHORT *) Qn;
      SHORT *Src2 = (SHORT *) Qm;

      for(i = 0; i < num - idx; i++){
        Dst[i] = Src1[i + idx];
      }
      for(i = 0; i < idx; i++){
        Dst[i + num - idx] = Src2[i];
      }
    }
    break;
    case 32:
    {
      INT *Dst  = (INT *) &tmp;
      INT *Src1 = (INT *) Qn;
      INT *Src2 = (INT *) Qm;

      for(i = 0; i < num - idx; i++){
        Dst[i] = Src1[i + idx];
      }
      for(i = 0; i < idx; i++){
        Dst[i + num - idx] = Src2[i];
      }
    }
    break;
  }
  ((INT64 *)Qd)[0] = tmp[0];
  ((INT64 *)Qd)[1] = tmp[1];
}

static inline void __FDK_vext_d(INT width, FIXP_H Dd, FIXP_H Dn, FIXP_D Dm, INT idx)
{
  FDK_ASSERT (width == 8 || width == 16 || width == 32);
  INT i, num = 64 / width;
  FDK_ASSERT ((idx >=  0) && (idx < num));
  INT64 tmp;

  switch(width)
  {
    case 8:
    {
      SCHAR *Dst  = (SCHAR *) &tmp;
      SCHAR *Src1 = (SCHAR *) Dn;
      SCHAR *Src2 = (SCHAR *) Dm;

      for(i = 0; i < num - idx; i++){
        Dst[i] = Src1[i + idx];
      }
      for(i = 0; i < idx; i++){
        Dst[i + num - idx] = Src2[i];
      }
    }
    break;
    case 16:
    {
      SHORT *Dst  = (SHORT *) &tmp;
      SHORT *Src1 = (SHORT *) Dn;
      SHORT *Src2 = (SHORT *) Dm;

      for(i = 0; i < num - idx; i++){
        Dst[i] = Src1[i + idx];
      }
      for(i = 0; i < idx; i++){
        Dst[i + num - idx] = Src2[i];
      }
    }
    break;
    case 32:
    {
      INT *Dst  = (INT *) &tmp;
      INT *Src1 = (INT *) Dn;
      INT *Src2 = (INT *) Dm;

      for(i = 0; i < num - idx; i++){
        Dst[i] = Src1[i + idx];
      }
      for(i = 0; i < idx; i++){
        Dst[i + num - idx] = Src2[i];
      }
    }
    break;
  }
  ((INT64 *)Dd)[0] = tmp;
}

#endif


/* Vector Transpose */
#ifdef __ARM_NEON__
#define FDK_vtrn_q(size,Qd,Qm)  "VTRN." #size " " #Qd ", " #Qm " \n\t"
#define FDK_vtrn_d(size,Dd,Dm)  "VTRN." #size " " #Dd ", " #Dm " \n\t"
#else
#define FDK_vtrn_q(size,Qd,Qm)  __FDK_vtrn_q(size,Qd,Qm);
#define FDK_vtrn_d(size,Dd,Dm)  __FDK_vtrn_d(size,Dd,Dm);

static inline void __FDK_vtrn_q(int size, FIXP_Q Qd, FIXP_Q Qm)
{
  FDK_ASSERT((Qd - Qm) != 0);
  FDK_ASSERT((size == 16) || (size == 32));  /* size=8 not implemented yet */

  switch (size)
  {
    case 8:
      break;
    case 16:
    {
      FIXP_SGL tmp[4];
      FIXP_S Sd = (FIXP_S) Qd;
      FIXP_S Sm = (FIXP_S) Qm;

      tmp[0] = Sd[1];  /* A8 */
      tmp[1] = Sd[3];  /* A7 */
      tmp[2] = Sd[5];  /* A6 */
      tmp[3] = Sd[7];  /* A7 */

      Sd[1]  = Sm[0];
      Sd[3]  = Sm[2];
      Sd[5]  = Sm[4];
      Sd[7]  = Sm[6];

      Sm[0]  = tmp[0];
      Sm[2]  = tmp[1];
      Sm[4]  = tmp[2];
      Sm[6]  = tmp[3];
      break;
    }
    case 32:
    {
      FIXP_DBL tmp[2];
      FIXP_D Dd = (FIXP_D) Qd;
      FIXP_D Dm = (FIXP_D) Qm;

      tmp[0] = Dd[1];
      tmp[1] = Dd[3];
      Dd[1]  = Dm[0];
      Dd[3]  = Dm[2];
      Dm[0]  = tmp[0];
      Dm[2]  = tmp[1];
      break;
    }
  }
}
static inline void __FDK_vtrn_d(int size, FIXP_H Dd, FIXP_H Dm)
{
  FDK_ASSERT((Dd - Dm) != 0);
  FDK_ASSERT((size == 16) || (size == 32));  /* size=8 not implemented yet */
  switch (size)
  {
    case 8:
      break;
    case 16:
    {
      FIXP_S Sd = (FIXP_S) Dd;
      FIXP_S Sm = (FIXP_S) Dm;
      FIXP_SGL tmp[2];

      tmp[0] = Sd[1];
      tmp[1] = Sd[3];

      Sd[1]  = Sm[0];
      Sd[3]  = Sm[2];
      Sm[0]  = tmp[0];
      Sm[2]  = tmp[1];
      break;
    }
    case 32:
    {
      FIXP_D Sd = (FIXP_D) Dd;
      FIXP_D Sm = (FIXP_D) Dm;
      FIXP_DBL tmp = Sd[1];

      Sd[1]  = Sm[0];
      Sm[0]  = tmp;
      break;
    }
  }
}
#endif /* __ARM_NEON__ */


/* Conversion from float to fixedpoint and vice versa */
#ifdef __ARM_NEON__
#define FDK_vcvt_f32_s32_q(Qd,Qm,fbits)  "VCVT.f32.s32 " #Qd ", " #Qm ", # " #fbits " \n\t"
#define FDK_vcvt_f32_s32_d(Dd,Dm,fbits)  "VCVT.f32.s32 " #Dd ", " #Dm ", # " #fbits " \n\t"
#define FDK_vcvt_s32_f32_q(Qd,Qm,fbits)  "VCVT.s32.f32 " #Qd ", " #Qm ", # " #fbits " \n\t"
#define FDK_vcvt_s32_f32_d(Dd,Dm,fbits)  "VCVT.s32.f32 " #Dd ", " #Dm ", # " #fbits " \n\t"
#else
#define FDK_vcvt_f32_s32_q(Qd,Qm,fbits)  __FDK_vcvt_f32_s32_q(Qd,Qm,fbits);
#define FDK_vcvt_f32_s32_d(Dd,Dm,fbits)  __FDK_vcvt_f32_s32_d(Dd,Dm,fbits);
#define FDK_vcvt_s32_f32_q(Qd,Qm,fbits)  __FDK_vcvt_s32_f32_q(Qd,Qm,fbits);
#define FDK_vcvt_s32_f32_d(Dd,Dm,fbits)  __FDK_vcvt_s32_f32_d(Dd,Dm,fbits);

/* Convert from fixedpoint to float */
static inline void __FDK_vcvt_f32_s32_q(FIXP_Q Qd, FIXP_Q Qm, int fbits)
{
  float *fQd = (float *) Qd;
  fQd[0] = ((float)((double)(Qm[0])/(double)((INT64) 1 << fbits)));
  fQd[1] = ((float)((double)(Qm[1])/(double)((INT64) 1 << fbits)));
  fQd[2] = ((float)((double)(Qm[2])/(double)((INT64) 1 << fbits)));
  fQd[3] = ((float)((double)(Qm[3])/(double)((INT64) 1 << fbits)));
}
static inline void __FDK_vcvt_f32_s32_d(FIXP_D Dd, FIXP_D Dm, int fbits)
{
  float *fDd = (float *) Dd;
  fDd[0] = ((float)((double)(Dm[0])/(double)((INT64) 1 << fbits)));
  fDd[1] = ((float)((double)(Dm[1])/(double)((INT64) 1 << fbits)));
}

/* Convert from float to fixedpoint */
static inline void __FDK_vcvt_s32_f32_q(FIXP_Q Qd, FIXP_Q Qm, int fbits)
{
  float *fQm = (float *) Qm;
  Qd[0] =  FL2FX_DBL((fQm[0] / ((INT64) 1 << (DFRACT_BITS-1-fbits))));
  Qd[1] =  FL2FX_DBL((fQm[1] / ((INT64) 1 << (DFRACT_BITS-1-fbits))));
  Qd[2] =  FL2FX_DBL((fQm[2] / ((INT64) 1 << (DFRACT_BITS-1-fbits))));
  Qd[3] =  FL2FX_DBL((fQm[3] / ((INT64) 1 << (DFRACT_BITS-1-fbits))));
}
static inline void __FDK_vcvt_s32_f32_d(FIXP_D Dd, FIXP_D Dm, int fbits)
{
  float *fDm = (float *) Dm;
  Dd[0] =  FL2FX_DBL((fDm[0] / ((INT64) 1 << (DFRACT_BITS-1-fbits))));
  Dd[1] =  FL2FX_DBL((fDm[1] / ((INT64) 1 << (DFRACT_BITS-1-fbits))));
}
#endif

#ifdef __ARM_NEON__
#define FDK_vsqrt_f32(Sd, Sm)  "VSQRT.F32 " #Sd ", " #Sm " \n\t"
#else
#include <math.h>
#define FDK_vsqrt_f32(Sd, Sm)  __FDK_vsqrt_f32(Sd, Sm);
static inline void __FDK_vsqrt_f32(FIXP_D Sd, FIXP_D Sm)
{
  FIXP_F fSd = (FIXP_F) Sd;
  FIXP_F fSm = (FIXP_F) Sm;
  fSd[0] = (float) sqrt((double) fSm[0]);
}
#endif

/* reference: NEON and VFP programming, chapter 4.3.6 */
#ifdef __ARM_NEON__
#define FDK_vcge_s32_q_zero(Qd, Qn)   "VCGE.s32 " #Qd ", " #Qn ", # 0  \n\t"
#define FDK_vcge_s32_d_zero(Dd, Dn)   "VCGE.s32 " #Dd ", " #Dn ", # 0  \n\t"
#define FDK_vceq_s32_q_zero(Qd, Qn)   "VCEQ.s32 " #Qd ", " #Qn ", # 0  \n\t"
#define FDK_vceq_s32_q(Qd, Qn, Qm)    "VCEQ.s32 " #Qd ", " #Qn ", " #Qm "\n\t"
#define FDK_vceq_s32_d(Dd, Dn, Dm)    "VCEQ.s32 " #Dd ", " #Dn ", " #Dm "\n\t"
#else
#define FDK_vcge_s32_q_zero(Qd, Qn)  __FDK_vcge_s32_q_zero(Qd, Qn);
#define FDK_vcge_s32_d_zero(Dd, Dn)  __FDK_vcge_s32_d_zero(Dd, Dn);
#define FDK_vceq_s32_q_zero(Qd, Qn)  __FDK_vceq_s32_q_zero(Qd, Qn);
#define FDK_vceq_s32_q(Qd, Qn, Qm)   __FDK_vceq_s32_q(Qd, Qn, Qm);
#define FDK_vceq_s32_d(Dd, Dn, Dm)   __FDK_vceq_s32_d(Dd, Dn, Dm);

static inline void __FDK_vcge_s32_q_zero(FIXP_Q Qd, FIXP_Q Qn)
{
   FIXP_D Dd = (FIXP_D) Qd;
   FIXP_D Dn = (FIXP_D) Qn;

   Dd[0] = (Dn[0] >= 0) ? (FIXP_DBL) 0xFFFFFFFF : (FIXP_DBL) 0x00000000;
   Dd[1] = (Dn[1] >= 0) ? (FIXP_DBL) 0xFFFFFFFF : (FIXP_DBL) 0x00000000;
   Dd[2] = (Dn[2] >= 0) ? (FIXP_DBL) 0xFFFFFFFF : (FIXP_DBL) 0x00000000;
   Dd[3] = (Dn[3] >= 0) ? (FIXP_DBL) 0xFFFFFFFF : (FIXP_DBL) 0x00000000;

}

static inline void __FDK_vceq_s32_q_zero(FIXP_Q Qd, FIXP_Q Qn)
{
  FIXP_D Dd = (FIXP_D) Qd;
  FIXP_D Dn = (FIXP_D) Qn;

   Dd[0] = (Dn[0] == 0) ? (FIXP_DBL) 0xFFFFFFFF : (FIXP_DBL) 0x00000000;
   Dd[1] = (Dn[1] == 0) ? (FIXP_DBL) 0xFFFFFFFF : (FIXP_DBL) 0x00000000;
   Dd[2] = (Dn[2] == 0) ? (FIXP_DBL) 0xFFFFFFFF : (FIXP_DBL) 0x00000000;
   Dd[3] = (Dn[3] == 0) ? (FIXP_DBL) 0xFFFFFFFF : (FIXP_DBL) 0x00000000;

}


static inline void __FDK_vcge_s32_d_zero(FIXP_H Hd, FIXP_H Hn)
{
  FIXP_D Dd = (FIXP_D)Hd;
  FIXP_D Dn = (FIXP_D)Hn;

  Dd[0] = (Dn[0] >= 0) ? (FIXP_DBL)0xFFFFFFFF : (FIXP_DBL)0x00000000;
  Dd[1] = (Dn[1] >= 0) ? (FIXP_DBL)0xFFFFFFFF : (FIXP_DBL)0x00000000;

}

static inline void __FDK_vceq_s32_q(FIXP_Q Qd, FIXP_Q Qn, FIXP_Q Qm)
{
  FIXP_D Dd = (FIXP_D)Qd;
  FIXP_D Dn = (FIXP_D)Qn;
  FIXP_D Dm = (FIXP_D)Qm;

  for (int i = 0; i < 4; i++)
  {
    Dd[i] = (Dn[i] == Dm[i]) ? (FIXP_DBL)0xFFFFFFFF : (FIXP_DBL)0x00000000;
  }
}

static inline void __FDK_vceq_s32_d(FIXP_Q Hd, FIXP_Q Hn, FIXP_Q Hm)
{
  FIXP_D Dd = (FIXP_D)Hd;
  FIXP_D Dn = (FIXP_D)Hn;
  FIXP_D Dm = (FIXP_D)Hm;

  for (int i = 0; i < 2; i++)
  {
    Dd[i] = (Dn[i] == Dm[i]) ? (FIXP_DBL)0xFFFFFFFF : (FIXP_DBL)0x00000000;
  }
}
#endif

/* reference: NEON and VFP programming, chapter 4.3.7 */
#ifdef __ARM_NEON__
#define FDK_vtst_q(size, Qd, Qn, Qm)      "VTST." #size " " #Qd ", " #Qn "," #Qm "\n\t"
#define FDK_vtst_d(size, Dd, Dn, Dm)      "VTST." #size " " #Dd ", " #Dn "," #Dm "\n\t"
#define FDK_vbit_q(Qd, Qn, Qm)            "VBIT " #Qd ", " #Qn "," #Qm "\n\t"
#define FDK_vbit_d(Dd, Dn, Dm)            "VBIT " #Dd ", " #Dn "," #Dm "\n\t"
#else
#define FDK_vtst_q(size, Qd, Qn, Qm)      __FDK_vtst_q(size, Qd, Qn, Qm);
#define FDK_vtst_d(size, Dd, Dn, Dm)      __FDK_vtst_d(size, Dd, Dn, Dm);
#define FDK_vbit_q(Qd, Qn, Qm)            __FDK_vbit_q(Qd, Qn, Qm);
#define FDK_vbit_d(Dd, Dn, Dm)            __FDK_vbit_d(Dd, Dn, Dm);

static inline void __FDK_vtst_q(INT size, FIXP_Q Qd, FIXP_Q Qn, FIXP_Q Qm)
{
    switch(size)
    {
      case 8:
      {
        FIXP_C Src1 = (FIXP_C) Qn;
        FIXP_C Src2 = (FIXP_C) Qm;
        FIXP_C Dst = (FIXP_C)  Qd;
        for (int i = 0; i < 16; i++)
        {
          Dst[i] = (Src1[i] & Src2[i]) == (UCHAR) 0 ? (UCHAR) 0x00 : (UCHAR) 0xFF;
        }
        break;
      }
      case 16:
      {
        FIXP_S Src1 = (FIXP_S) Qn;
        FIXP_S Src2 = (FIXP_S) Qm;
        FIXP_S Dst = (FIXP_S)  Qd;
        for (int i = 0; i < 8; i++)
        {
          Dst[i] = (Src1[i] & Src2[i]) == (FIXP_SGL) 0 ? (FIXP_SGL) 0x0000 : (FIXP_SGL) 0xFFFF;
        }
        break;
      }
      case 32:
      {
        FIXP_D Src1 = (FIXP_D) Qn;
        FIXP_D Src2 = (FIXP_D) Qm;
        FIXP_D Dst = (FIXP_D)  Qd;
        for (int i = 0; i < 4; i++)
        {
          Dst[i] = (Src1[i] & Src2[i]) == (FIXP_DBL) 0 ? (FIXP_DBL) 0x00000000 : (FIXP_DBL) 0xFFFFFFFF;
        }
        break;
      }
    }
}
static inline void __FDK_vtst_d(INT size, FIXP_H Dd, FIXP_H Dn, FIXP_H Dm)
{
    switch(size)
    {
      case 8:
      {
        FIXP_C Src1 = (FIXP_C) Dn;
        FIXP_C Src2 = (FIXP_C) Dm;
        FIXP_C Dst = (FIXP_C)  Dd;
        for (int i = 0; i < 8; i++)
        {
          Dst[i] = (Src1[i] & Src2[i]) == (UCHAR) 0 ? (UCHAR) 0x00 : (UCHAR) 0xFF;
        }
        break;
      }
      case 16:
      {
        FIXP_S Src1 = (FIXP_S) Dn;
        FIXP_S Src2 = (FIXP_S) Dm;
        FIXP_S Dst = (FIXP_S)  Dd;
        for (int i = 0; i < 4; i++)
        {
          Dst[i] = (Src1[i] & Src2[i]) == (FIXP_SGL) 0 ? (FIXP_SGL) 0x0000 : (FIXP_SGL) 0xFFFF;
        }
        break;
      }
      case 32:
      {
        FIXP_D Src1 = (FIXP_D) Dn;
        FIXP_D Src2 = (FIXP_D) Dm;
        FIXP_D Dst = (FIXP_D)  Dd;
        for (int i = 0; i < 2; i++)
        {
          Dst[i] = (Src1[i] & Src2[i]) == (FIXP_DBL) 0 ? (FIXP_DBL) 0x00000000 : (FIXP_DBL) 0xFFFFFFFF;
        }
        break;
      }
    }
}

static inline void __FDK_vbit_q(FIXP_Q Qd, FIXP_Q Qn, FIXP_Q Qm)
{
    FIXP_D Src1 = (FIXP_D) Qn;
    FIXP_D Src2 = (FIXP_D) Qm;
    FIXP_D Dst = (FIXP_D)  Qd;
    for (int i = 0; i < 4; i++)
    {
      Dst[i] = (Dst[i] & (~Src2[i])) | (Src1[i] & Src2[i]);
    }
}

static inline void __FDK_vbit_d(FIXP_H Dd, FIXP_H Dn, FIXP_H Dm)
{
    FIXP_D Src1 = (FIXP_D) Dn;
    FIXP_D Src2 = (FIXP_D) Dm;
    FIXP_D Dst = (FIXP_D)  Dd;
    for (int i = 0; i < 2; i++)
    {
      Dst[i] = (Dst[i] & (~Src2[i])) | (Src1[i] & Src2[i]);
    }
}



#endif




#ifdef __ARM_NEON__
#define FDK_vclz_q(size, dst, src)    "VCLZ.S" #size " " #dst ", " #src " \n\t"
#define FDK_vclz_d(size, dst, src)    "VCLZ.S" #size " " #dst ", " #src " \n\t"
#define FDK_vcls_q(size, dst, src)    "VCLS.S" #size " " #dst ", " #src " \n\t"
#define FDK_vcls_d(size, dst, src)    "VCLS.S" #size " " #dst ", " #src " \n\t"
#else /* __ARM_NEON__ */
#define FDK_vclz_q(size, dst, src)    { if (size == 32) { __FDK_vclz_32_q((FIXP_Q)dst, (FIXP_Q)src); } else if (size == 16)  { __FDK_vclz_16_q((FIXP_Q)dst, (FIXP_Q)src); } else if (size == 8) { __FDK_vclz_8_q((FIXP_Q)dst, (FIXP_Q)src); } }
#define FDK_vclz_d(size, dst, src)    { if (size == 32) { __FDK_vclz_32_d((FIXP_H)dst, (FIXP_H)src); } else if (size == 16)  { __FDK_vclz_16_d((FIXP_H)dst, (FIXP_H)src); } else if (size == 8) { __FDK_vclz_8_d((FIXP_H)dst, (FIXP_H)src); } }
#define FDK_vcls_q(size, dst, src)    { if (size == 32) { __FDK_vcls_32_q((FIXP_Q)dst, (FIXP_Q)src); } else if (size == 16)  { __FDK_vcls_16_q((FIXP_Q)dst, (FIXP_Q)src); } else if (size == 8) { __FDK_vcls_8_q((FIXP_Q)dst, (FIXP_Q)src); } }
#define FDK_vcls_d(size, dst, src)    { if (size == 32) { __FDK_vcls_32_d((FIXP_H)dst, (FIXP_H)src); } else if (size == 16)  { __FDK_vcls_16_d((FIXP_H)dst, (FIXP_H)src); } else if (size == 8) { __FDK_vcls_8_d((FIXP_H)dst, (FIXP_H)src); } }

static void inline __FDK_vclz_32_q(FIXP_Q dst, FIXP_Q src)
{
  FIXP_D Src = (FIXP_D)src;
  FIXP_D Dst = (FIXP_D)dst;

  for (int i = 0; i < 4; i++)
    Dst[i] = CntLeadingZeros(Src[i]);
}
static void inline __FDK_vclz_32_d(FIXP_H dst, FIXP_H src)
{
  FIXP_D Src = (FIXP_D)src;
  FIXP_D Dst = (FIXP_D)dst;

  for (int i = 0; i < 2; i++)
    Dst[i] = CntLeadingZeros(Src[i]);
}
static void inline __FDK_vclz_16_q(FIXP_Q dst, FIXP_Q src)
{
  FIXP_S Src = (FIXP_S)src;
  FIXP_S Dst = (FIXP_S)dst;

  for (int i = 0; i < 8; i++)
    Dst[i] = CntLeadingZeros(Src[i]);
}
static void inline __FDK_vclz_16_d(FIXP_H dst, FIXP_H src)
{
  FIXP_S Src = (FIXP_S)src;
  FIXP_S Dst = (FIXP_S)dst;

  for (int i = 0; i < 4; i++)
    Dst[i] = CntLeadingZeros(Src[i]);
}
static void inline __FDK_vclz_8_q(FIXP_Q dst, FIXP_Q src)
{
  FIXP_C Src = (FIXP_C)src;
  FIXP_C Dst = (FIXP_C)dst;

  for (int i = 0; i < 16; i++)
    Dst[i] = CntLeadingZeros(Src[i]);
}
static void inline __FDK_vclz_8_d(FIXP_H dst, FIXP_H src)
{
  FIXP_C Src = (FIXP_C)src;
  FIXP_C Dst = (FIXP_C)dst;

  for (int i = 0; i < 8; i++)
    Dst[i] = CntLeadingZeros(Src[i]);
}

static void inline __FDK_vcls_32_q(FIXP_Q dst, FIXP_Q src)
{
  FIXP_D Src = (FIXP_D)src;
  FIXP_D Dst = (FIXP_D)dst;

  for (int i = 0; i < 4; i++)
    Dst[i] = (Src[i] != (FIXP_DBL) 0) ? CountLeadingBits(Src[i]) : 31;
}
static void inline __FDK_vcls_32_d(FIXP_H dst, FIXP_H src)
{
  FIXP_D Src = (FIXP_D)src;
  FIXP_D Dst = (FIXP_D)dst;

  for (int i = 0; i < 2; i++)
    Dst[i] = (Src[i] != (FIXP_DBL) 0) ? CountLeadingBits(Src[i]) : 31;
}
static void inline __FDK_vcls_16_q(FIXP_Q dst, FIXP_Q src)
{
  FIXP_S Src = (FIXP_S)src;
  FIXP_S Dst = (FIXP_S)dst;

  for (int i = 0; i < 8; i++)
    Dst[i] = (Src[i] != (FIXP_DBL) 0) ? CountLeadingBits((FIXP_DBL) (Src[i]<<16)) : 15;
}
static void inline __FDK_vcls_16_d(FIXP_H dst, FIXP_H src)
{
  FIXP_S Src = (FIXP_S)src;
  FIXP_S Dst = (FIXP_S)dst;

  for (int i = 0; i < 4; i++)
    Dst[i] = (Src[i] != (FIXP_DBL) 0) ? CountLeadingBits((FIXP_DBL) (Src[i]<<16)) : 15;
}
static void inline __FDK_vcls_8_q(FIXP_Q dst, FIXP_Q src)
{
  FIXP_C Src = (FIXP_C)src;
  FIXP_C Dst = (FIXP_C)dst;

  for (int i = 0; i < 16; i++)
    Dst[i] = (Src[i] != (FIXP_DBL) 0) ? CountLeadingBits((FIXP_DBL) (Src[i]<<24)) : 7;
}
static void inline __FDK_vcls_8_d(FIXP_H dst, FIXP_H src)
{
  FIXP_C Src = (FIXP_C)src;
  FIXP_C Dst = (FIXP_C)dst;

  for (int i = 0; i < 8; i++)
    Dst[i] = (Src[i] != (FIXP_DBL) 0) ? CountLeadingBits((FIXP_DBL) (Src[i]<<24)) : 7;
}

#endif /* __ARM_NEON__ */

/* reference: NEON and VFP programming, chapter 4.10.2 */
#ifdef __ARM_NEON__
#define FDK_vdiv_f32(dst, src1, src2)     "VDIV.F32 " #dst ", " #src1 ", " #src2 " \n\t"
#else /* __ARM_NEON__ */
#define FDK_vdiv_f32(dst, src1, src2)  __FDK_vdiv_f32((FLOAT*)dst, (FLOAT*)src1, (FLOAT*)src2);


static void inline __FDK_vdiv_f32(FLOAT* dst, FLOAT* src1, FLOAT* src2 )
{
  *dst = *src1 / *src2;
}

#endif /* __ARM_NEON__*/

#endif  /* FDK_NEON_FUNCS_H */
