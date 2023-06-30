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

   Author(s):

   Description: Scaling operations

*******************************************************************************/

#include "common_fix.h"

#include "genericStds.h"

/**************************************************
 * Inline definitions
 **************************************************/

#include "scale.h"

#if defined(__arm__)
#include "arm/scale_arm.cpp"

#endif

#ifndef FUNCTION_scaleValues_SGL
/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *  \param len    must be larger than 4
 *  \return void
 *
 */
#define FUNCTION_scaleValues_SGL
SCALE_INLINE
void scaleValues(FIXP_SGL* vector, /*!< Vector */
                 INT len,          /*!< Length */
                 INT scalefactor   /*!< Scalefactor */
) {
  INT i;

  /* Return if scalefactor is Zero */
  if (scalefactor == 0) return;

  if (scalefactor > 0) {
    scalefactor = fixmin_I(scalefactor, (INT)(FRACT_BITS - 1));
    for (i = len & 3; i--;) {
      *(vector++) <<= scalefactor;
    }
    for (i = len >> 2; i--;) {
      *(vector++) <<= scalefactor;
      *(vector++) <<= scalefactor;
      *(vector++) <<= scalefactor;
      *(vector++) <<= scalefactor;
    }
  } else {
    INT negScalefactor = fixmin_I(-scalefactor, (INT)FRACT_BITS - 1);
    for (i = len & 3; i--;) {
      *(vector++) >>= negScalefactor;
    }
    for (i = len >> 2; i--;) {
      *(vector++) >>= negScalefactor;
      *(vector++) >>= negScalefactor;
      *(vector++) >>= negScalefactor;
      *(vector++) >>= negScalefactor;
    }
  }
}
#endif

#ifndef FUNCTION_scaleValues_DBL
/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *  \param len must be larger than 4
 *  \return void
 *
 */
#define FUNCTION_scaleValues_DBL
SCALE_INLINE
void scaleValues(FIXP_DBL* vector, /*!< Vector */
                 INT len,          /*!< Length */
                 INT scalefactor   /*!< Scalefactor */
) {
  INT i;

  /* Return if scalefactor is Zero */
  if (scalefactor == 0) return;

  if (scalefactor > 0) {
    scalefactor = fixmin_I(scalefactor, (INT)DFRACT_BITS - 1);
    for (i = len & 3; i--;) {
      *(vector++) <<= scalefactor;
    }
    for (i = len >> 2; i--;) {
      *(vector++) <<= scalefactor;
      *(vector++) <<= scalefactor;
      *(vector++) <<= scalefactor;
      *(vector++) <<= scalefactor;
    }
  } else {
    INT negScalefactor = fixmin_I(-scalefactor, (INT)DFRACT_BITS - 1);
    for (i = len & 3; i--;) {
      *(vector++) >>= negScalefactor;
    }
    for (i = len >> 2; i--;) {
      *(vector++) >>= negScalefactor;
      *(vector++) >>= negScalefactor;
      *(vector++) >>= negScalefactor;
      *(vector++) >>= negScalefactor;
    }
  }
}
#endif

#ifndef FUNCTION_scaleValuesSaturate_DBL
/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *  \param vector      source/destination buffer
 *  \param len         length of vector
 *  \param scalefactor amount of shifts to be applied
 *  \return void
 *
 */
#define FUNCTION_scaleValuesSaturate_DBL
SCALE_INLINE
void scaleValuesSaturate(FIXP_DBL* vector, /*!< Vector */
                         INT len,          /*!< Length */
                         INT scalefactor   /*!< Scalefactor */
) {
  INT i;

  /* Return if scalefactor is Zero */
  if (scalefactor == 0) return;

  scalefactor = fixmax_I(fixmin_I(scalefactor, (INT)DFRACT_BITS - 1), (INT) - (DFRACT_BITS - 1));

  for (i = 0; i < len; i++) {
    vector[i] = scaleValueSaturate(vector[i], scalefactor);
  }
}
#endif /* FUNCTION_scaleValuesSaturate_DBL */

#ifndef FUNCTION_scaleValuesSaturate_DBL_DBL
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
#define FUNCTION_scaleValuesSaturate_DBL_DBL
SCALE_INLINE
void scaleValuesSaturate(FIXP_DBL* dst,       /*!< Output */
                         const FIXP_DBL* src, /*!< Input   */
                         INT len,             /*!< Length */
                         INT scalefactor      /*!< Scalefactor */
) {
  INT i;

  /* Return if scalefactor is Zero */
  if (scalefactor == 0) {
    if (dst != src) FDKmemmove(dst, src, len * sizeof(FIXP_DBL));
    return;
  }

  scalefactor = fixmax_I(fixmin_I(scalefactor, (INT)DFRACT_BITS - 1), (INT) - (DFRACT_BITS - 1));

  for (i = 0; i < len; i++) {
    dst[i] = scaleValueSaturate(src[i], scalefactor);
  }
}
#endif /* FUNCTION_scaleValuesSaturate_DBL_DBL */

#ifndef FUNCTION_scaleValuesSaturate_SGL_DBL
/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *  \param dst         destination buffer (FIXP_SGL)
 *  \param src         source buffer (FIXP_DBL)
 *  \param len         length of vector
 *  \param scalefactor amount of shifts to be applied
 *  \return void
 *
 */
#define FUNCTION_scaleValuesSaturate_SGL_DBL
SCALE_INLINE
void scaleValuesSaturate(FIXP_SGL* dst,       /*!< Output */
                         const FIXP_DBL* src, /*!< Input   */
                         INT len,             /*!< Length */
                         INT scalefactor)     /*!< Scalefactor */
{
  INT i;
  scalefactor = fixmax_I(fixmin_I(scalefactor, (INT)DFRACT_BITS - 1), (INT) - (DFRACT_BITS - 1));

  for (i = 0; i < len; i++) {
    dst[i] = FX_DBL2FX_SGL(fAddSaturate(scaleValueSaturate(src[i], scalefactor), (FIXP_DBL)0x8000));
  }
}
#endif /* FUNCTION_scaleValuesSaturate_SGL_DBL */

#ifndef FUNCTION_scaleValuesSaturate_SGL
/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *  \param vector      source/destination buffer
 *  \param len         length of vector
 *  \param scalefactor amount of shifts to be applied
 *  \return void
 *
 */
#define FUNCTION_scaleValuesSaturate_SGL
SCALE_INLINE
void scaleValuesSaturate(FIXP_SGL* vector, /*!< Vector */
                         INT len,          /*!< Length */
                         INT scalefactor   /*!< Scalefactor */
) {
  INT i;

  /* Return if scalefactor is Zero */
  if (scalefactor == 0) return;

  scalefactor = fixmax_I(fixmin_I(scalefactor, (INT)DFRACT_BITS - 1), (INT) - (DFRACT_BITS - 1));

  for (i = 0; i < len; i++) {
    vector[i] = FX_DBL2FX_SGL(scaleValueSaturate(FX_SGL2FX_DBL(vector[i]), scalefactor));
  }
}
#endif /* FUNCTION_scaleValuesSaturate_SGL */

#ifndef FUNCTION_scaleValuesSaturate_SGL_SGL
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
#define FUNCTION_scaleValuesSaturate_SGL_SGL
SCALE_INLINE
void scaleValuesSaturate(FIXP_SGL* dst,       /*!< Output */
                         const FIXP_SGL* src, /*!< Input */
                         INT len,             /*!< Length */
                         INT scalefactor      /*!< Scalefactor */
) {
  INT i;

  /* Return if scalefactor is Zero */
  if (scalefactor == 0) {
    if (dst != src) FDKmemmove(dst, src, len * sizeof(FIXP_SGL));
    return;
  }

  scalefactor = fixmax_I(fixmin_I(scalefactor, (INT)DFRACT_BITS - 1), (INT) - (DFRACT_BITS - 1));

  for (i = 0; i < len; i++) {
    dst[i] = FX_DBL2FX_SGL(scaleValueSaturate(FX_SGL2FX_DBL(src[i]), scalefactor));
  }
}
#endif /* FUNCTION_scaleValuesSaturate_SGL_SGL */

#ifndef FUNCTION_scaleValues_DBLDBL
/*!
 *
 *  \brief  Multiply input vector src by \f$ 2^{scalefactor} \f$
 *          and place result into dst
 *  \param dst detination buffer
 *  \param src source buffer
 *  \param len must be larger than 4
 *  \param scalefactor amount of left shifts to be applied
 *  \return void
 *
 */
#define FUNCTION_scaleValues_DBLDBL
SCALE_INLINE
void scaleValues(FIXP_DBL* dst,       /*!< dst Vector */
                 const FIXP_DBL* src, /*!< src Vector */
                 INT len,             /*!< Length */
                 INT scalefactor      /*!< Scalefactor */
) {
  INT i;

  /* Return if scalefactor is Zero */
  if (scalefactor == 0) {
    if (dst != src) FDKmemmove(dst, src, len * sizeof(FIXP_DBL));
  } else {
    if (scalefactor > 0) {
      scalefactor = fixmin_I(scalefactor, (INT)DFRACT_BITS - 1);
      for (i = len & 3; i--;) {
        *(dst++) = *(src++) << scalefactor;
      }
      for (i = len >> 2; i--;) {
        *(dst++) = *(src++) << scalefactor;
        *(dst++) = *(src++) << scalefactor;
        *(dst++) = *(src++) << scalefactor;
        *(dst++) = *(src++) << scalefactor;
      }
    } else {
      INT negScalefactor = fixmin_I(-scalefactor, (INT)DFRACT_BITS - 1);
      for (i = len & 3; i--;) {
        *(dst++) = *(src++) >> negScalefactor;
      }
      for (i = len >> 2; i--;) {
        *(dst++) = *(src++) >> negScalefactor;
        *(dst++) = *(src++) >> negScalefactor;
        *(dst++) = *(src++) >> negScalefactor;
        *(dst++) = *(src++) >> negScalefactor;
      }
    }
  }
}
#endif

#if (SAMPLE_BITS == 16)
#ifndef FUNCTION_scaleValues_PCMDBL
/*!
 *
 *  \brief  Multiply input vector src by \f$ 2^{scalefactor} \f$
 *          and place result into dst
 *  \param dst detination buffer
 *  \param src source buffer
 *  \param len must be larger than 4
 *  \param scalefactor amount of left shifts to be applied
 *  \return void
 *
 */
#define FUNCTION_scaleValues_PCMDBL
SCALE_INLINE
void scaleValues(FIXP_PCM* dst,       /*!< dst Vector */
                 const FIXP_DBL* src, /*!< src Vector */
                 INT len,             /*!< Length */
                 INT scalefactor      /*!< Scalefactor */
) {
  INT i;

  scalefactor -= DFRACT_BITS - SAMPLE_BITS;

  /* Return if scalefactor is Zero */
  {
    if (scalefactor > 0) {
      scalefactor = fixmin_I(scalefactor, (INT)DFRACT_BITS - 1);
      for (i = len & 3; i--;) {
        *(dst++) = (FIXP_PCM)(*(src++) << scalefactor);
      }
      for (i = len >> 2; i--;) {
        *(dst++) = (FIXP_PCM)(*(src++) << scalefactor);
        *(dst++) = (FIXP_PCM)(*(src++) << scalefactor);
        *(dst++) = (FIXP_PCM)(*(src++) << scalefactor);
        *(dst++) = (FIXP_PCM)(*(src++) << scalefactor);
      }
    } else {
      INT negScalefactor = fixmin_I(-scalefactor, (INT)DFRACT_BITS - 1);
      for (i = len & 3; i--;) {
        *(dst++) = (FIXP_PCM)(*(src++) >> negScalefactor);
      }
      for (i = len >> 2; i--;) {
        *(dst++) = (FIXP_PCM)(*(src++) >> negScalefactor);
        *(dst++) = (FIXP_PCM)(*(src++) >> negScalefactor);
        *(dst++) = (FIXP_PCM)(*(src++) >> negScalefactor);
        *(dst++) = (FIXP_PCM)(*(src++) >> negScalefactor);
      }
    }
  }
}
#endif
#endif /* (SAMPLE_BITS == 16) */

#ifndef FUNCTION_scaleValues_SGLSGL
/*!
 *
 *  \brief  Multiply input vector src by \f$ 2^{scalefactor} \f$
 *          and place result into dst
 *  \param dst detination buffer
 *  \param src source buffer
 *  \param len must be larger than 4
 *  \param scalefactor amount of left shifts to be applied
 *  \return void
 *
 */
#define FUNCTION_scaleValues_SGLSGL
SCALE_INLINE
void scaleValues(FIXP_SGL* dst,       /*!< dst Vector */
                 const FIXP_SGL* src, /*!< src Vector */
                 INT len,             /*!< Length */
                 INT scalefactor      /*!< Scalefactor */
) {
  INT i;

  /* Return if scalefactor is Zero */
  if (scalefactor == 0) {
    if (dst != src) FDKmemmove(dst, src, len * sizeof(FIXP_DBL));
  } else {
    if (scalefactor > 0) {
      scalefactor = fixmin_I(scalefactor, (INT)DFRACT_BITS - 1);
      for (i = len & 3; i--;) {
        *(dst++) = *(src++) << scalefactor;
      }
      for (i = len >> 2; i--;) {
        *(dst++) = *(src++) << scalefactor;
        *(dst++) = *(src++) << scalefactor;
        *(dst++) = *(src++) << scalefactor;
        *(dst++) = *(src++) << scalefactor;
      }
    } else {
      INT negScalefactor = fixmin_I(-scalefactor, (INT)DFRACT_BITS - 1);
      for (i = len & 3; i--;) {
        *(dst++) = *(src++) >> negScalefactor;
      }
      for (i = len >> 2; i--;) {
        *(dst++) = *(src++) >> negScalefactor;
        *(dst++) = *(src++) >> negScalefactor;
        *(dst++) = *(src++) >> negScalefactor;
        *(dst++) = *(src++) >> negScalefactor;
      }
    }
  }
}
#endif

#ifndef FUNCTION_scaleValuesWithFactor_DBL
/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *  \param len must be larger than 4
 *  \return void
 *
 */
#define FUNCTION_scaleValuesWithFactor_DBL
SCALE_INLINE
void scaleValuesWithFactor(FIXP_DBL* vector, FIXP_DBL factor, INT len, INT scalefactor) {
  INT i;

  /* Compensate fMultDiv2 */
  scalefactor++;

  if (scalefactor > 0) {
    scalefactor = fixmin_I(scalefactor, (INT)DFRACT_BITS - 1);
    for (i = len & 3; i--;) {
      *vector = fMultDiv2(*vector, factor) << scalefactor;
      vector++;
    }
    for (i = len >> 2; i--;) {
      *vector = fMultDiv2(*vector, factor) << scalefactor;
      vector++;
      *vector = fMultDiv2(*vector, factor) << scalefactor;
      vector++;
      *vector = fMultDiv2(*vector, factor) << scalefactor;
      vector++;
      *vector = fMultDiv2(*vector, factor) << scalefactor;
      vector++;
    }
  } else {
    INT negScalefactor = fixmin_I(-scalefactor, (INT)DFRACT_BITS - 1);
    for (i = len & 3; i--;) {
      *vector = fMultDiv2(*vector, factor) >> negScalefactor;
      vector++;
    }
    for (i = len >> 2; i--;) {
      *vector = fMultDiv2(*vector, factor) >> negScalefactor;
      vector++;
      *vector = fMultDiv2(*vector, factor) >> negScalefactor;
      vector++;
      *vector = fMultDiv2(*vector, factor) >> negScalefactor;
      vector++;
      *vector = fMultDiv2(*vector, factor) >> negScalefactor;
      vector++;
    }
  }
}
#endif /* FUNCTION_scaleValuesWithFactor_DBL */

/*******************************************

IMPORTANT NOTE for usage of getScalefactor()

If the input array contains negative values too, then these functions may sometimes return
the actual maximum value minus 1, due to the nature of the applied algorithm.
So be careful with possible fractional -1 values that may lead to overflows when being fPow2()'ed.

********************************************/

#ifndef FUNCTION_getScalefactorShort
/*!
 *
 *  \brief Calculate max possible scale factor for input vector of shorts
 *
 *  \return Maximum scale factor / possible left shift
 *
 */
#define FUNCTION_getScalefactorShort
SCALE_INLINE
INT getScalefactorShort(const SHORT* vector, /*!< Pointer to input vector */
                        INT len              /*!< Length of input vector */
) {
  INT i;
  SHORT temp, maxVal = 0;

  for (i = len; i != 0; i--) {
    temp = (SHORT)(*vector++);
    maxVal |= (temp ^ (temp >> (SHORT_BITS - 1)));
  }

  return fixmax_I((INT)0,
                  (INT)(fixnormz_D((INT)maxVal) - (INT)1 - (INT)(DFRACT_BITS - SHORT_BITS)));
}
#endif

#ifndef FUNCTION_getScalefactorPCM
/*!
 *
 *  \brief Calculate max possible scale factor for input vector of shorts
 *
 *  \return Maximum scale factor
 *
 */
#define FUNCTION_getScalefactorPCM
SCALE_INLINE
INT getScalefactorPCM(const INT_PCM* vector, /*!< Pointer to input vector */
                      INT len,               /*!< Length of input vector */
                      INT stride) {
  INT i;
  INT_PCM temp, maxVal = 0;

  for (i = len; i != 0; i--) {
    temp = (INT_PCM)(*vector);
    vector += stride;
    maxVal |= (temp ^ (temp >> ((sizeof(INT_PCM) * 8) - 1)));
  }
  return fixmax_I((INT)0,
                  (INT)(fixnormz_D((INT)maxVal) - (INT)1 - (INT)(DFRACT_BITS - SAMPLE_BITS)));
}
#endif

#ifndef FUNCTION_getScalefactorShort
/*!
 *
 *  \brief Calculate max possible scale factor for input vector of shorts
 *  \param stride, item increment between vector members.
 *  \return Maximum scale factor
 *
 */
#define FUNCTION_getScalefactorShort
SCALE_INLINE
INT getScalefactorShort(const SHORT* vector, /*!< Pointer to input vector */
                        INT len,             /*!< Length of input vector */
                        INT stride) {
  INT i;
  SHORT temp, maxVal = 0;

  for (i = len; i != 0; i--) {
    temp = (SHORT)(*vector);
    vector += stride;
    maxVal |= (temp ^ (temp >> (SHORT_BITS - 1)));
  }

  return fixmax_I((INT)0,
                  (INT)(fixnormz_D((INT)maxVal) - (INT)1 - (INT)(DFRACT_BITS - SHORT_BITS)));
}
#endif

#ifndef FUNCTION_getScalefactor_DBL
/*!
 *
 *  \brief Calculate max possible scale factor for input vector
 *
 *  \return Maximum scale factor
 *
 *  This function can constitute a significant amount of computational complexity - very much
 * depending on the bitrate. Since it is a rather small function, effective assembler optimization
 * might be possible.
 *
 *  If all data is 0xFFFF.FFFF or 0x0000.0000 function returns 31
 *  Note: You can skip data normalization only if return value is 0
 *
 */
#define FUNCTION_getScalefactor_DBL
SCALE_INLINE
INT getScalefactor(const FIXP_DBL* vector, /*!< Pointer to input vector */
                   INT len)                /*!< Length of input vector */
{
  INT i;
  FIXP_DBL temp, maxVal = (FIXP_DBL)0;

  for (i = len; i != 0; i--) {
    temp = (LONG)(*vector++);
    maxVal |= (FIXP_DBL)((LONG)temp ^ (LONG)(temp >> (DFRACT_BITS - 1)));
  }

  return fixmax_I((INT)0, (INT)(fixnormz_D(maxVal) - 1));
}
#endif

#ifndef FUNCTION_getScalefactor_SGL
#define FUNCTION_getScalefactor_SGL
SCALE_INLINE
INT getScalefactor(const FIXP_SGL* vector, /*!< Pointer to input vector */
                   INT len)                /*!< Length of input vector */
{
  INT i;
  SHORT temp, maxVal = (FIXP_SGL)0;

  for (i = len; i != 0; i--) {
    temp = (SHORT)(*vector++);
    maxVal |= (temp ^ (temp >> (FRACT_BITS - 1)));
  }

  return fixmax_I((INT)0, (INT)(fixnormz_S((FIXP_SGL)maxVal)) - 1);
}
#endif
