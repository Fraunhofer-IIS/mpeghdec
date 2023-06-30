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

   Author(s):   M. Gayer

   Description: Fixed point specific mathematical functions

*******************************************************************************/

#ifndef FIXPOINT_MATH_H
#define FIXPOINT_MATH_H

#include "common_fix.h"
#include "scale.h"

/*
 * Data definitions
 */

#define LD_DATA_SCALING (64.0f)
#define LD_DATA_SHIFT 6 /* pow(2, LD_DATA_SHIFT) = LD_DATA_SCALING */

#define MAX_LD_PRECISION 10
#define LD_PRECISION 10

/* Taylor series coefficients for ln(1-x), centered at 0 (MacLaurin polynomial). */
#ifndef LDCOEFF_16BIT
LNK_SECTION_CONSTDATA_L1
static const FIXP_DBL ldCoeff[MAX_LD_PRECISION] = {
    FL2FXCONST_DBL(-1.0),       FL2FXCONST_DBL(-1.0 / 2.0), FL2FXCONST_DBL(-1.0 / 3.0),
    FL2FXCONST_DBL(-1.0 / 4.0), FL2FXCONST_DBL(-1.0 / 5.0), FL2FXCONST_DBL(-1.0 / 6.0),
    FL2FXCONST_DBL(-1.0 / 7.0), FL2FXCONST_DBL(-1.0 / 8.0), FL2FXCONST_DBL(-1.0 / 9.0),
    FL2FXCONST_DBL(-1.0 / 10.0)};
#else  /* LDCOEFF_16BIT */
LNK_SECTION_CONSTDATA_L1
static const FIXP_SGL ldCoeff[MAX_LD_PRECISION] = {
    FL2FXCONST_SGL(-1.0),       FL2FXCONST_SGL(-1.0 / 2.0), FL2FXCONST_SGL(-1.0 / 3.0),
    FL2FXCONST_SGL(-1.0 / 4.0), FL2FXCONST_SGL(-1.0 / 5.0), FL2FXCONST_SGL(-1.0 / 6.0),
    FL2FXCONST_SGL(-1.0 / 7.0), FL2FXCONST_SGL(-1.0 / 8.0), FL2FXCONST_SGL(-1.0 / 9.0),
    FL2FXCONST_SGL(-1.0 / 10.0)};
#endif /* LDCOEFF_16BIT */

/*****************************************************************************

    functionname: invSqrtNorm2
    description:  delivers 1/sqrt(op) normalized to .5...1 and the shift value of the OUTPUT

*****************************************************************************/
#define SQRT_BITS 7
#define SQRT_VALUES (128 + 2)
#define SQRT_BITS_MASK 0x7f
#define SQRT_FRACT_BITS_MASK 0x007FFFFF

extern const FIXP_DBL invSqrtTab[SQRT_VALUES];

/*
 * Hardware specific implementations
 */

#if defined(__arm__)
#include "arm/fixpoint_math_arm.h"
#elif defined(__x86__)
#include "x86/fixpoint_math_x86.h"
#endif /* target architecture selector */

/*
 * Fallback implementations
 */
#if !defined(FUNCTION_fIsLessThan)
/**
 * \brief Compares two fixpoint values incl. scaling.
 * \param a_m mantissa of the first input value.
 * \param a_e exponent of the first input value.
 * \param b_m mantissa of the second input value.
 * \param b_e exponent of the second input value.
 * \return non-zero if (a_m*2^a_e) < (b_m*2^b_e), 0 otherwise
 */
FDK_INLINE INT fIsLessThan(FIXP_DBL a_m, INT a_e, FIXP_DBL b_m, INT b_e) {
  INT n;

  n = fixnorm_D(a_m);
  a_m <<= n;
  a_e -= n;

  n = fixnorm_D(b_m);
  b_m <<= n;
  b_e -= n;

  if (a_m == (FIXP_DBL)0) a_e = b_e;
  if (b_m == (FIXP_DBL)0) b_e = a_e;

  if (a_e > b_e) {
    return ((b_m >> fMin(a_e - b_e, DFRACT_BITS - 1)) > a_m);
  } else {
    return ((a_m >> fMin(b_e - a_e, DFRACT_BITS - 1)) < b_m);
  }
}

FDK_INLINE INT fIsLessThan(FIXP_SGL a_m, INT a_e, FIXP_SGL b_m, INT b_e) {
  INT n;

  n = fixnorm_S(a_m);
  a_m <<= n;
  a_e -= n;

  n = fixnorm_S(b_m);
  b_m <<= n;
  b_e -= n;

  if (a_m == (FIXP_SGL)0) a_e = b_e;
  if (b_m == (FIXP_SGL)0) b_e = a_e;

  if (a_e > b_e) {
    return ((b_m >> fMin(a_e - b_e, FRACT_BITS - 1)) > a_m);
  } else {
    return ((a_m >> fMin(b_e - a_e, FRACT_BITS - 1)) < b_m);
  }
}
#endif

/**
 * \brief deprecated. Use fLog2() instead.
 */
#define CalcLdData(op) fLog2(op, 0)

void LdDataVector(FIXP_DBL* srcVector, FIXP_DBL* destVector, INT number);

extern const UINT exp2_tab_long[32];
extern const UINT exp2w_tab_long[32];
extern const UINT exp2x_tab_long[32];

LNK_SECTION_CODE_L1
FDK_INLINE FIXP_DBL CalcInvLdData(const FIXP_DBL x) {
  int set_zero = (x < FL2FXCONST_DBL(-31.0 / 64.0)) ? 0 : 1;
  int set_max = (x >= FL2FXCONST_DBL(31.0 / 64.0)) | (x == FL2FXCONST_DBL(0.0));
  FIXP_DBL retVal;

  if (set_max) {
    retVal = (FIXP_DBL)MAXVAL_DBL;
  } else {
    FIXP_SGL frac = (FIXP_SGL)((LONG)x & 0x3FF);
    UINT index3 = (UINT)(LONG)(x >> 10) & 0x1F;
    UINT index2 = (UINT)(LONG)(x >> 15) & 0x1F;
    UINT index1 = (UINT)(LONG)(x >> 20) & 0x1F;
    int exp = fMin(31, ((x > FL2FXCONST_DBL(0.0f)) ? (31 - (int)(x >> 25)) : (int)(-(x >> 25))));

    UINT lookup1 = exp2_tab_long[index1] * set_zero;
    UINT lookup2 = exp2w_tab_long[index2];
    UINT lookup3 = exp2x_tab_long[index3];
    UINT lookup3f = lookup3 + (UINT)(LONG)fMultDiv2((FIXP_DBL)(0x0016302F), (FIXP_SGL)frac);

    UINT lookup12 = (UINT)(LONG)fMult((FIXP_DBL)lookup1, (FIXP_DBL)lookup2);
    UINT lookup = (UINT)(LONG)fMult((FIXP_DBL)lookup12, (FIXP_DBL)lookup3f);

    retVal = (FIXP_DBL)((lookup << 3) >> exp);
  }

  return retVal;
}

void InitLdInt();
FIXP_DBL CalcLdInt(INT i);

extern const USHORT sqrt_tab[49];

inline FIXP_DBL sqrtFixp_lookup(FIXP_DBL x) {
  UINT y = (INT)x;
  UCHAR is_zero = (y == 0);
  UINT t = 0;
  if (!is_zero) {
    INT zeros = fixnormz_D(y) & 0x1e;
    y <<= zeros;
    UINT idx = (y >> 26) - 16;
    USHORT frac = (y >> 10) & 0xffff;
    USHORT nfrac = 0xffff ^ frac;
    t = (UINT)nfrac * sqrt_tab[idx] + (UINT)frac * sqrt_tab[idx + 1];
    t = t >> (zeros >> 1);
  }
  return t;
}

inline FIXP_DBL sqrtFixp_lookup(FIXP_DBL x, INT* x_e) {
  UINT y = (INT)x;
  INT e;

  if (x == (FIXP_DBL)0) {
    return x;
  }

  /* Normalize */
  e = fixnormz_D(y);
  y <<= e;
  e = *x_e - e + 2;

  /* Correct odd exponent. */
  if (e & 1) {
    y >>= 1;
    e++;
  }
  /* Get square root */
  UINT idx = (y >> 26) - 16;
  USHORT frac = (y >> 10) & 0xffff;
  USHORT nfrac = 0xffff ^ frac;
  UINT t = (UINT)nfrac * sqrt_tab[idx] + (UINT)frac * sqrt_tab[idx + 1];

  /* Write back exponent */
  *x_e = e >> 1;
  return (FIXP_DBL)(LONG)(t >> 1);
}

void InitInvSqrtTab();

#ifndef FUNCTION_invSqrtNorm2
/**
 * \brief calculate 1.0/sqrt(op)
 * \param op_m mantissa of input value.
 * \param result_e pointer to return the exponent of the result
 * \return mantissa of the result
 */
/*****************************************************************************
  delivers 1/sqrt(op) normalized to .5...1 and the shift value of the OUTPUT,
  i.e. the denormalized result is 1/sqrt(op) = invSqrtNorm(op) * 2^(shift)
  uses Newton-iteration for approximation
      Q(n+1) = Q(n) + Q(n) * (0.5 - 2 * V * Q(n)^2)
      with Q = 0.5* V ^-0.5; 0.5 <= V < 1.0
*****************************************************************************/
static FDK_FORCEINLINE FIXP_DBL invSqrtNorm2(FIXP_DBL op, INT* shift) {
  FIXP_DBL val = op;
  FIXP_DBL reg1, reg2;

  if (val == FL2FXCONST_DBL(0.0)) {
    *shift = 16;
    return ((LONG)MAXVAL_DBL); /* maximum positive value */
  }

#define INVSQRTNORM2_LINEAR_INTERPOLATE
#define INVSQRTNORM2_LINEAR_INTERPOLATE_HQ

  /* normalize input, calculate shift value */
  FDK_ASSERT(val > FL2FXCONST_DBL(0.0));
  *shift =
      fNormz(val) - 1; /* CountLeadingBits() is not necessary here since test value is always > 0 */
  val <<= *shift;      /* normalized input V */
  *shift += 2;         /* bias for exponent */

#if defined(INVSQRTNORM2_LINEAR_INTERPOLATE)
  INT index = (INT)(val >> (DFRACT_BITS - 1 - (SQRT_BITS + 1))) & SQRT_BITS_MASK;
  FIXP_DBL Fract = (FIXP_DBL)(((INT)val & SQRT_FRACT_BITS_MASK) << (SQRT_BITS + 1));
  FIXP_DBL diff = invSqrtTab[index + 1] - invSqrtTab[index];
  reg1 = invSqrtTab[index] + (fMultDiv2(diff, Fract) << 1);
#if defined(INVSQRTNORM2_LINEAR_INTERPOLATE_HQ)
  /* reg1 = t[i] + (t[i+1]-t[i])*fract ... already computed ...
                                       + (1-fract)fract*(t[i+2]-t[i+1])/2 */
  if (Fract != (FIXP_DBL)0) {
    /* fract = fract * (1 - fract) */
    Fract = fMultDiv2(Fract, (FIXP_DBL)((ULONG)0x80000000 - (LONG)Fract)) << 1;
    diff = diff - (invSqrtTab[index + 2] - invSqrtTab[index + 1]);
    reg1 = fMultAddDiv2(reg1, Fract, diff);
  }
#endif /* INVSQRTNORM2_LINEAR_INTERPOLATE_HQ */
#else
#error "Either define INVSQRTNORM2_NEWTON_ITERATE or INVSQRTNORM2_LINEAR_INTERPOLATE"
#endif
  /* calculate the output exponent = input exp/2 */
  if (*shift & 0x00000001) { /* odd shift values ? */
    /* Note: Do not use rounded value 0x5A82799A to avoid overflow with shift-by-2 */
    reg2 = (FIXP_DBL)0x5A827999;
    /* FL2FXCONST_DBL(0.707106781186547524400844362104849f);*/ /* 1/sqrt(2); */
    reg1 = fMultDiv2(reg1, reg2) << 2;
  }

  *shift = *shift >> 1;

  return (reg1);
}
#endif /* FUNCTION_invSqrtNorm2 */

#ifndef FUNCTION_sqrtFixp
static FDK_FORCEINLINE FIXP_DBL sqrtFixp(FIXP_DBL op) {
  INT tmp_exp = 0;
  FIXP_DBL tmp_inv = invSqrtNorm2(op, &tmp_exp);

  FDK_ASSERT(tmp_exp > 0);
  return ((FIXP_DBL)(fMultDiv2((op << (tmp_exp - 1)), tmp_inv) << 2));
}
#endif /* FUNCTION_sqrtFixp */

#ifndef FUNCTION_invFixp
/**
 * \brief calculate 1.0/op
 * \param op mantissa of the input value.
 * \return mantissa of the result with implicit exponent of 31
 * \exceptions are provided for op=0,1 setting max. positive value
 */
static inline FIXP_DBL invFixp(FIXP_DBL op) {
  if ((op == (FIXP_DBL)0x00000000) || (op == (FIXP_DBL)0x00000001)) {
    return ((LONG)MAXVAL_DBL);
  }
  INT tmp_exp;
  FIXP_DBL tmp_inv = invSqrtNorm2(op, &tmp_exp);
  FDK_ASSERT((31 - (2 * tmp_exp + 1)) >= 0);
  int shift = 31 - (2 * tmp_exp + 1);
  tmp_inv = fPow2Div2(tmp_inv);
  if (shift) {
    tmp_inv = ((tmp_inv >> (shift - 1)) + (FIXP_DBL)1) >> 1;
  }
  return tmp_inv;
}

/**
 * \brief calculate 1.0/(op_m * 2^op_e)
 * \param op_m mantissa of the input value.
 * \param op_e pointer into were the exponent of the input value is stored, and the result will be
 * stored into.
 * \return mantissa of the result
 */
static inline FIXP_DBL invFixp(FIXP_DBL op_m, int* op_e) {
  if ((op_m == (FIXP_DBL)0x00000000) || (op_m == (FIXP_DBL)0x00000001)) {
    *op_e = 31 - *op_e;
    return ((LONG)MAXVAL_DBL);
  }

  INT tmp_exp;
  FIXP_DBL tmp_inv = invSqrtNorm2(op_m, &tmp_exp);

  *op_e = (tmp_exp << 1) - *op_e + 1;
  return fPow2Div2(tmp_inv);
}
#endif /* FUNCTION_invFixp */

#ifndef FUNCTION_schur_div

/**
 * \brief Divide two FIXP_DBL values with given precision.
 * \param num dividend
 * \param denum divisor
 * \param count amount of significant bits of the result (starting to the MSB)
 * \return num/divisor
 */

FIXP_DBL schur_div(FIXP_DBL num, FIXP_DBL denum, INT count);

#endif /* FUNCTION_schur_div */

FIXP_DBL mul_dbl_sgl_rnd(const FIXP_DBL op1, const FIXP_SGL op2);

#ifndef FUNCTION_fMultNorm
/**
 * \brief multiply two values with normalization, thus max precision.
 * Author: Robert Weidner
 *
 * \param f1 first factor
 * \param f2 second factor
 * \param result_e pointer to an INT where the exponent of the result is stored into
 * \return mantissa of the product f1*f2
 */
FIXP_DBL fMultNorm(FIXP_DBL f1, FIXP_DBL f2, INT* result_e);

/**
 * \brief Multiply 2 values using maximum precision. The exponent of the result is 0.
 * \param f1_m mantissa of factor 1
 * \param f2_m mantissa of factor 2
 * \return mantissa of the result with exponent equal to 0
 */
inline FIXP_DBL fMultNorm(FIXP_DBL f1, FIXP_DBL f2) {
  FIXP_DBL m;
  INT e;

  m = fMultNorm(f1, f2, &e);

  m = scaleValueSaturate(m, e);

  return m;
}

/**
 * \brief Multiply 2 values with exponent and use given exponent for the mantissa of the result.
 * \param f1_m mantissa of factor 1
 * \param f1_e exponent of factor 1
 * \param f2_m mantissa of factor 2
 * \param f2_e exponent of factor 2
 * \param result_e exponent for the returned mantissa of the result
 * \return mantissa of the result with exponent equal to result_e
 */
inline FIXP_DBL fMultNorm(FIXP_DBL f1_m, INT f1_e, FIXP_DBL f2_m, INT f2_e, INT result_e) {
  FIXP_DBL m;
  INT e;

  m = fMultNorm(f1_m, f2_m, &e);

  m = scaleValueSaturate(m, e + f1_e + f2_e - result_e);

  return m;
}
#endif /* FUNCTION_fMultNorm */

#ifndef FUNCTION_fMultI
/**
 * \brief Multiplies a fractional value and a integer value and performs rounding to nearest
 * \param a fractional value
 * \param b integer value
 * \return integer value
 */
inline INT fMultI(FIXP_DBL a, INT b) {
  FIXP_DBL m, mi;
  INT m_e;

  m = fMultNorm(a, (FIXP_DBL)b, &m_e);

  if (m_e < (INT)0) {
    if (m_e > (INT)-DFRACT_BITS) {
      m = m >> ((-m_e) - 1);
      mi = (m + (FIXP_DBL)1) >> 1;
    } else {
      mi = (FIXP_DBL)0;
    }
  } else {
    mi = scaleValueSaturate(m, m_e);
  }

  return ((INT)mi);
}
#endif /* FUNCTION_fMultI */

#ifndef FUNCTION_fMultIfloor
/**
 * \brief Multiplies a fractional value and a integer value and performs floor rounding
 * \param a fractional value
 * \param b integer value
 * \return integer value
 */
inline INT fMultIfloor(FIXP_DBL a, INT b) {
  FIXP_DBL m, mi;
  INT m_e;

  m = fMultNorm(a, (FIXP_DBL)b, &m_e);

  if (m_e < (INT)0) {
    if (m_e > (INT)-DFRACT_BITS) {
      mi = m >> (-m_e);
    } else {
      mi = (FIXP_DBL)0;
      if (m < (FIXP_DBL)0) {
        mi = (FIXP_DBL)-1;
      }
    }
  } else {
    mi = scaleValueSaturate(m, m_e);
  }

  return ((INT)mi);
}
#endif /* FUNCTION_fMultIfloor */

#ifndef FUNCTION_fMultIceil
/**
 * \brief Multiplies a fractional value and a integer value and performs ceil rounding
 * \param a fractional value
 * \param b integer value
 * \return integer value
 */
inline INT fMultIceil(FIXP_DBL a, INT b) {
  FIXP_DBL m, mi;
  INT m_e;

  m = fMultNorm(a, (FIXP_DBL)b, &m_e);

  if (m_e < (INT)0) {
    if (m_e > (INT) - (DFRACT_BITS - 1)) {
      mi = (m >> (-m_e));
      if ((LONG)m & ((1 << (-m_e)) - 1)) {
        mi = mi + (FIXP_DBL)1;
      }
    } else {
      if (m > (FIXP_DBL)0) {
        mi = (FIXP_DBL)1;
      } else {
        if ((m_e == -(DFRACT_BITS - 1)) && (m == (FIXP_DBL)MINVAL_DBL)) {
          mi = (FIXP_DBL)-1;
        } else {
          mi = (FIXP_DBL)0;
        }
      }
    }
  } else {
    mi = scaleValueSaturate(m, m_e);
  }

  return ((INT)mi);
}
#endif /* FUNCTION_fMultIceil */

#ifndef FUNCTION_fDivNorm
/**
 * \brief Divide 2 FIXP_DBL values with normalization of input values.
 * \param num numerator
 * \param denum denominator
 * \param result_e pointer to an INT where the exponent of the result is stored into
 * \return num/denum with exponent = *result_e
 */
FIXP_DBL fDivNorm(FIXP_DBL num, FIXP_DBL denom, INT* result_e);

/**
 * \brief Divide 2 positive FIXP_DBL values with normalization of input values.
 * \param num numerator
 * \param denum denominator
 * \return num/denum with exponent = 0
 */
FIXP_DBL fDivNorm(FIXP_DBL num, FIXP_DBL denom);

/**
 * \brief Divide 2 signed FIXP_DBL values with normalization of input values.
 * \param num numerator
 * \param denum denominator
 * \param result_e pointer to an INT where the exponent of the result is stored into
 * \return num/denum with exponent = *result_e
 */
FIXP_DBL fDivNormSigned(FIXP_DBL L_num, FIXP_DBL L_denum, INT* result_e);

/**
 * \brief Divide 2 signed FIXP_DBL values with normalization of input values.
 * \param num numerator
 * \param denum denominator
 * \return num/denum with exponent = 0
 */
FIXP_DBL fDivNormSigned(FIXP_DBL num, FIXP_DBL denom);
#endif /* FUNCTION_fDivNorm */

/**
 * \brief Adjust mantissa to exponent -1
 * \param a_m mantissa of value to be adjusted
 * \param pA_e pointer to the exponen of a_m
 * \return adjusted mantissa
 */
inline FIXP_DBL fAdjust(FIXP_DBL a_m, INT* pA_e) {
  INT shift;

  shift = fNorm(a_m) - 1;
  *pA_e -= shift;

  return scaleValue(a_m, shift);
}

#ifndef FUNCTION_fAddNorm
/**
 * \brief Add two values with normalization
 * \param a_m mantissa of first summand
 * \param a_e exponent of first summand
 * \param a_m mantissa of second summand
 * \param a_e exponent of second summand
 * \param pResult_e pointer to where the exponent of the result will be stored to.
 * \return mantissa of result
 */
inline FIXP_DBL fAddNorm(FIXP_DBL a_m, INT a_e, FIXP_DBL b_m, INT b_e, INT* pResult_e) {
  INT result_e;
  FIXP_DBL result_m;

  /* If one of the summands is zero, return the other.
     This is necessary for the summation of a very small number to zero */
  if (a_m == (FIXP_DBL)0) {
    *pResult_e = b_e;
    return b_m;
  }
  if (b_m == (FIXP_DBL)0) {
    *pResult_e = a_e;
    return a_m;
  }

  a_m = fAdjust(a_m, &a_e);
  b_m = fAdjust(b_m, &b_e);

  if (a_e > b_e) {
    result_m = a_m + (b_m >> fMin(a_e - b_e, DFRACT_BITS - 1));
    result_e = a_e;
  } else {
    result_m = (a_m >> fMin(b_e - a_e, DFRACT_BITS - 1)) + b_m;
    result_e = b_e;
  }

  *pResult_e = result_e;
  return result_m;
}

inline FIXP_DBL fAddNorm(FIXP_DBL a_m, INT a_e, FIXP_DBL b_m, INT b_e, INT result_e) {
  FIXP_DBL result_m;

  a_m = scaleValue(a_m, a_e - result_e);
  b_m = scaleValue(b_m, b_e - result_e);

  result_m = a_m + b_m;

  return result_m;
}
#endif /* FUNCTION_fAddNorm */

/**
 * \brief Divide 2 FIXP_DBL values with normalization of input values.
 * \param num numerator
 * \param denum denomintator
 * \return num/denum with exponent = 0
 */
FIXP_DBL fDivNormHighPrec(FIXP_DBL L_num, FIXP_DBL L_denum, INT* result_e);

#ifndef FUNCTION_fPow
/**
 * \brief return 2 ^ (exp_m * 2^exp_e)
 * \param exp_m mantissa of the exponent to 2.0f
 * \param exp_e exponent of the exponent to 2.0f
 * \param result_e pointer to a INT where the exponent of the result will be stored into
 * \return mantissa of the result
 */
FIXP_DBL f2Pow(const FIXP_DBL exp_m, const INT exp_e, INT* result_e);

/**
 * \brief return 2 ^ (exp_m * 2^exp_e). This version returns only the mantissa with implicit
 * exponent of zero.
 * \param exp_m mantissa of the exponent to 2.0f
 * \param exp_e exponent of the exponent to 2.0f
 * \return mantissa of the result
 */
FIXP_DBL f2Pow(const FIXP_DBL exp_m, const INT exp_e);

/**
 * \brief return x ^ (exp_m * 2^exp_e), where log2(x) = baseLd_m * 2^(baseLd_e). This saves
 *        the need to compute log2() of constant values (when x is a constant).
 * \param baseLd_m mantissa of log2() of x.
 * \param baseLd_e exponent of log2() of x.
 * \param exp_m mantissa of the exponent to 2.0f
 * \param exp_e exponent of the exponent to 2.0f
 * \param result_e pointer to a INT where the exponent of the result will be stored into
 * \return mantissa of the result
 */
FIXP_DBL fLdPow(FIXP_DBL baseLd_m, INT baseLd_e, FIXP_DBL exp_m, INT exp_e, INT* result_e);

/**
 * \brief return x ^ (exp_m * 2^exp_e), where log2(x) = baseLd_m * 2^(baseLd_e). This saves
 *        the need to compute log2() of constant values (when x is a constant). This version
 *        does not return an exponent, which is implicitly 0.
 * \param baseLd_m mantissa of log2() of x.
 * \param baseLd_e exponent of log2() of x.
 * \param exp_m mantissa of the exponent to 2.0f
 * \param exp_e exponent of the exponent to 2.0f
 * \return mantissa of the result
 */
FIXP_DBL fLdPow(FIXP_DBL baseLd_m, INT baseLd_e, FIXP_DBL exp_m, INT exp_e);

/**
 * \brief return (base_m * 2^base_e) ^ (exp * 2^exp_e). Use fLdPow() instead whenever possible.
 * \param base_m mantissa of the base.
 * \param base_e exponent of the base.
 * \param exp_m mantissa of power to be calculated of the base.
 * \param exp_e exponent of power to be calculated of the base.
 * \param result_e pointer to a INT where the exponent of the result will be stored into.
 * \return mantissa of the result.
 */
FIXP_DBL fPow(FIXP_DBL base_m, INT base_e, FIXP_DBL exp_m, INT exp_e, INT* result_e);

/**
 * \brief return (base_m * 2^base_e) ^ N
 * \param base_m mantissa of the base. Must not be negative.
 * \param base_e exponent of the base
 * \param N power to be calculated of the base
 * \param result_e pointer to a INT where the exponent of the result will be stored into
 * \return mantissa of the result
 */
FIXP_DBL fPowInt(FIXP_DBL base_m, INT base_e, INT N, INT* result_e);
#endif /* #ifndef FUNCTION_fPow */

#ifndef FUNCTION_fLog2
/**
 * \brief Calculate log(argument)/log(2) (logarithm with base 2). deprecated. Use fLog2() instead.
 * \param arg mantissa of the argument
 * \param arg_e exponent of the argument
 * \param result_e pointer to an INT to store the exponent of the result
 * \return the mantissa of the result.
 * \param
 */
FIXP_DBL CalcLog2(FIXP_DBL arg, INT arg_e, INT* result_e);

/**
 * \brief calculate logarithm of base 2 of x_m * 2^(x_e)
 * \param x_m mantissa of the input value.
 * \param x_e exponent of the input value.
 * \param pointer to an INT where the exponent of the result is returned into.
 * \return mantissa of the result.
 */
FDK_INLINE FIXP_DBL fLog2(FIXP_DBL x_m, INT x_e, INT* result_e) {
  FIXP_DBL result_m;

  /* Short cut for zero and negative numbers. */
  if (x_m <= FL2FXCONST_DBL(0.0f)) {
    *result_e = DFRACT_BITS - 1;
    return FL2FXCONST_DBL(-1.0f);
  }

  /* Calculate log2() */
  {
    FIXP_DBL x2_m;

    /* Move input value x_m * 2^x_e toward 1.0, where the taylor approximation
       of the function log(1-x) centered at 0 is most accurate. */
    {
      INT b_norm;

      b_norm = fNormz(x_m) - 1;
      x2_m = x_m << b_norm;
      x_e = x_e - b_norm;
    }

    /* map x from log(x) domain to log(1-x) domain. */
    x2_m = -(x2_m + FL2FXCONST_DBL(-1.0));

    /* Taylor polynomial approximation of ln(1-x) */
#ifdef FUNCTION_flog2_func1
    result_m = flog2_func1(x2_m);
#else
    {
      FIXP_DBL px2_m;
      result_m = FL2FXCONST_DBL(0.0);
      px2_m = x2_m;
      for (int i = 0; i < LD_PRECISION; i++) {
        result_m = fMultAddDiv2(result_m, ldCoeff[i], px2_m);
        px2_m = fMult(px2_m, x2_m);
      }
    }
#endif
    /* Multiply result with 1/ln(2) = 1.0 + 0.442695040888 (get log2(x) from ln(x) result). */
    result_m =
        fMultAddDiv2(result_m, result_m, FL2FXCONST_DBL(2.0 * 0.4426950408889634073599246810019));

    /* Add exponent part. log2(x_m * 2^x_e) = log2(x_m) + x_e */
    if (x_e != 0) {
      int enorm;

      enorm = DFRACT_BITS - fNorm((FIXP_DBL)x_e);
      /* The -1 in the right shift of result_m compensates the fMultDiv2() above in the taylor
       * polynomial evaluation loop.*/
      result_m = (result_m >> (enorm - 1)) + ((FIXP_DBL)x_e << (DFRACT_BITS - 1 - enorm));

      *result_e = enorm;
    } else {
      /* 1 compensates the fMultDiv2() above in the taylor polynomial evaluation loop.*/
      *result_e = 1;
    }
  }

  return result_m;
}

/**
 * \brief calculate logarithm of base 2 of x_m * 2^(x_e)
 * \param x_m mantissa of the input value.
 * \param x_e exponent of the input value.
 * \return mantissa of the result with implicit exponent of LD_DATA_SHIFT.
 */
FDK_INLINE FIXP_DBL fLog2(FIXP_DBL x_m, INT x_e) {
  if (x_m <= FL2FXCONST_DBL(0.0f)) {
    x_m = FL2FXCONST_DBL(-1.0f);
  } else {
    INT result_e;
    x_m = fLog2(x_m, x_e, &result_e);
    x_m = scaleValue(x_m, result_e - LD_DATA_SHIFT);
  }
  return x_m;
}

#endif /* FUNCTION_fLog2 */

#ifndef FUNCTION_fAddSaturate
/**
 * \brief Add with saturation of the result.
 * \param a first summand
 * \param b second summand
 * \return saturated sum of a and b.
 */
inline FIXP_SGL fAddSaturate(const FIXP_SGL a, const FIXP_SGL b) {
  LONG sum;

  sum = (LONG)(SHORT)a + (LONG)(SHORT)b;
  sum = fMax(fMin((INT)sum, (INT)MAXVAL_SGL), (INT)MINVAL_SGL);
  return (FIXP_SGL)(SHORT)sum;
}

/**
 * \brief Add with saturation of the result.
 * \param a first summand
 * \param b second summand
 * \return saturated sum of a and b.
 */
#if defined(__clang__)
/* implementation relies on unsigned overflow */
__attribute__((no_sanitize("unsigned-integer-overflow")))
#endif
inline FIXP_DBL
fAddSaturate(const FIXP_DBL a, const FIXP_DBL b) {
  ULONG ua = (ULONG)(LONG)a;
  ULONG ub = (ULONG)(LONG)b;
  ULONG sum = ua + ub;

  ua = MAXVAL_DBL + (ua >> 31);
  if ((LONG)((ua ^ ub) | ~(ub ^ sum)) >= 0) {
    sum = ua;
  }
  return (FIXP_DBL)(LONG)sum;
}
#endif /* FUNCTION_fAddSaturate */

#ifndef FUNCTION_fNegateSaturate
/**
 * \brief Negate with saturation. Both 0x8000 and 0x8001 map to 0x7fff.
 * \param x value to be negated
 * \return negated value.
 */
FDK_INLINE FIXP_DBL fNegateSaturate(FIXP_DBL x) {
  return (-fMax((FIXP_DBL)(MINVAL_DBL + 1), x));
}

FDK_INLINE FIXP_SGL fNegateSaturate(FIXP_SGL x) {
  return (-fMax((FIXP_SGL)(MINVAL_SGL + 1), x));
}
#endif

INT fixp_floorToInt(FIXP_DBL f_inp, INT sf);
FIXP_DBL fixp_floor(FIXP_DBL f_inp, INT sf);

INT fixp_ceilToInt(FIXP_DBL f_inp, INT sf);
FIXP_DBL fixp_ceil(FIXP_DBL f_inp, INT sf);

INT fixp_truncateToInt(FIXP_DBL f_inp, INT sf);
FIXP_DBL fixp_truncate(FIXP_DBL f_inp, INT sf);

INT fixp_roundToInt(FIXP_DBL f_inp, INT sf);
FIXP_DBL fixp_round(FIXP_DBL f_inp, INT sf);

/*****************************************************************************

 array for 1/n, n=1..80

****************************************************************************/

extern const FIXP_DBL invCount[80];

LNK_SECTION_INITCODE
inline void InitInvInt(void) {}

/**
 * \brief Calculate the value of 1/i where i is a integer value. It supports
 *        input values from 1 upto (80-1).
 * \param intValue Integer input value.
 * \param FIXP_DBL representation of 1/intValue
 */
inline FIXP_DBL GetInvInt(int intValue) {
  return invCount[fMin(fMax(intValue, 0), 80 - 1)];
}

#endif /* FIXPOINT_MATH_H */
