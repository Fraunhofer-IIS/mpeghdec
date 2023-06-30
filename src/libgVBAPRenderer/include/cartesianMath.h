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

/******************** MPEG-H 3DA object rendering library **********************

   Author(s):   Thomas Blender, Arthur Tritthart

   Description: Math functions for Vector Base Amplitude Panning (VBAP)

*******************************************************************************/

#ifndef CARTESIANMATH_H
#define CARTESIANMATH_H

#include "FDK_core.h"
#include "fixpoint_math.h"
#include "FDK_trigFcts.h"

typedef struct _PointSpherical {
  /*
   * @param azi Azimuth in radian
   * @param ele Elevation in radian
   * @param rad Radius
   *
   * -180 ... 180 is mapped to -1 ... 1
   *  -pi ... pi  is mapped to -1 ... 1
   * radius 0.5 ... 16 with exponent 4
   */

  FIXP_DBL azi;
  FIXP_DBL ele;
  FIXP_DBL rad;

} PointSpherical;

/*
 * @brief Point (or vector) in Cartesian coordinates.
 */
typedef struct _PointCartesian {
  /*
   * @param x Cartesian x coordinate
   * @param y Cartesian y coordinate
   * @param z Cartesian z coordinate
   *
   * Exponent for object data is normally 4 (because of the required radius of maximal 16m)
   * Exponent for speaker position is 0
   */

  FIXP_DBL x;
  FIXP_DBL y;
  FIXP_DBL z;

} PointCartesian;

/************************************************
 *     Function prototypes                      *
 ************************************************/

/*
 * @brief converts a spherical point to a Cartesian point
 * @param sph     spherical point, where azimuth and elevation is mapped -pi ... pi to -1 ... 1
 *
 * @return Cartesian coordinate, where x,y, and z is between -1 ... 1
 */
PointCartesian sphericalToCartesian(PointSpherical sph);

/*
 * @brief calculates the cross product of two Cartesian vectors
 * @param a, b    Cartesian points. Both exponents have to be the same.
 *
 * @return a x b. Result has an exponent of 1. In most cases the vector length don't care.
 */
PointCartesian crossProduct(PointCartesian* a, PointCartesian* b);

/*
 * @brief calculates the scalar product of two Cartesian vectors
 * @param a, b    Cartesian points. Both exponents have to be the same.
 *
 * @return a dot b. Result has a exponent of 2.
 */
FIXP_DBL dotProduct(PointCartesian* a, PointCartesian* b);

/*
 * @brief normalizes the vector a. The entries of a normalized vector are always between -1 and 1
 *        Therefore the results exponent will be set to 0
 * @param a       is a Cartesian point or vector
 * @return 0 if succeed, 1 if error
 */

FDK_INLINE int normalize(PointCartesian* a) {
  INT e_sf = 0;
  INT e_min;
  FIXP_DBL scalefactor;
  FIXP_DBL a_x_abs = fAbs(a->x);
  FIXP_DBL a_y_abs = fAbs(a->y);
  FIXP_DBL a_z_abs = fAbs(a->z);

  /* if the entries are very small we could not calculate a normalized vector.
   * Therefore we will scale it first */
  FIXP_DBL tmp = (FIXP_DBL)((LONG)a_x_abs | (LONG)a_y_abs | (LONG)a_z_abs);
  e_min = CountLeadingBits(tmp);

  e_min -= 1;
  if (e_min >= 0) {
    a_x_abs <<= e_min;
    a_y_abs <<= e_min;
    a_z_abs <<= e_min;
  } else {
    a_x_abs >>= -e_min;
    a_y_abs >>= -e_min;
    a_z_abs >>= -e_min;
  }
  scalefactor = fPow2(a_x_abs) + fPow2(a_y_abs) + fPow2(a_z_abs);
  if (scalefactor ==
      (FIXP_DBL)0) /* it must be a zero vector. There is no way to normalize it => do nothing */
    return -1;

  scalefactor = sqrtFixp(scalefactor);

  scalefactor = fMax(scalefactor, a_x_abs);
  scalefactor = fMax(scalefactor, a_y_abs);
  scalefactor = fMax(scalefactor, a_z_abs);
  scalefactor = scalefactor + (FIXP_DBL)1;

  scalefactor = invFixp(scalefactor, &e_sf);

  a->x = scaleValueSaturate(fMult(a->x, scalefactor), e_sf + e_min);
  a->y = scaleValueSaturate(fMult(a->y, scalefactor), e_sf + e_min);
  a->z = scaleValueSaturate(fMult(a->z, scalefactor), e_sf + e_min);

  return 0;
}

/************************************************
 *     Operator overloading                     *
 ************************************************/
PointCartesian operator-(const PointCartesian& lhs, const PointCartesian& rhs);
PointCartesian operator+(const PointCartesian& lhs, const PointCartesian& rhs);
PointCartesian operator*(const FIXP_DBL& lhs, const PointCartesian& rhs);
PointCartesian operator>>=(PointCartesian& lhs, const FIXP_DBL& rhs);
PointCartesian operator<<=(PointCartesian& lhs, const FIXP_DBL& rhs);
PointCartesian operator-(PointCartesian& lhs);

#endif
