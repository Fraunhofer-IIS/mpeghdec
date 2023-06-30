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

   Author(s):   Stefan Gewinner

   Description: fixed point multiplication

*******************************************************************************/

#if !defined(FIXMUL_H)
#define FIXMUL_H

#include "FDK_archdef.h"
#include "machine_type.h"

#if defined(__arm__)
#include "arm/fixmul_arm.h"

#elif defined(__x86__)
#include "x86/fixmul_x86.h"

#endif /* all cores */

/*************************************************************************
 *************************************************************************
    Software fallbacks for missing functions
**************************************************************************
**************************************************************************/

#if !defined(FUNCTION_fixmuldiv2_QQ)
#define FUNCTION_fixmuldiv2_QQ
FDK_INLINE INT64 fixmuldiv2_QQ(const INT64 a, const INT64 b) {
  INT64 a_hi = a >> DFRACT_BITS;
  INT64 a_lo = (ULONG)a;
  INT64 b_hi = b >> DFRACT_BITS;
  INT64 b_lo = (ULONG)b;

  INT64 a_x_b_hi = a_hi * b_hi;
  INT64 a_x_b_mid = a_hi * b_lo;
  INT64 b_x_a_mid = b_hi * a_lo;

  INT64 res = a_x_b_hi + (a_x_b_mid >> DFRACT_BITS) + (b_x_a_mid >> DFRACT_BITS);

  return res;
}
#endif

#if !defined(FUNCTION_fixmul_QQ)
#define FUNCTION_fixmul_QQ
FDK_INLINE INT64 fixmul_QQ(const INT64 a, const INT64 b) {
  { return fixmuldiv2_QQ(a, b) << 1; }
}
#endif

#if !defined(FUNCTION_fixmuldiv2_QD)
#define FUNCTION_fixmuldiv2_QD
inline INT64 fixmuldiv2_QD(const INT64 a, const LONG b) {
  INT64 a_hi = a >> DFRACT_BITS;
  INT64 a_lo = (ULONG)a;
  INT64 b_hi = (INT64)b;

  INT64 a_x_b_hi = a_hi * b_hi;
  INT64 b_x_a_mid = b_hi * a_lo;

  INT64 res = a_x_b_hi + (b_x_a_mid >> DFRACT_BITS);

  return res;
}
#endif

#if !defined(FUNCTION_fixmul_QD)
#define FUNCTION_fixmul_QD
inline INT64 fixmul_QD(const INT64 a, const LONG b) {
  { return fixmuldiv2_QD(a, b) << 1; }
}
#endif

#if !defined(FUNCTION_fixmuldiv2_DD)
#define FUNCTION_fixmuldiv2_DD
inline LONG fixmuldiv2_DD(const LONG a, const LONG b) {
  return (LONG)((((INT64)a) * b) >> 32);
}
#endif

#if !defined(FUNCTION_fixmuldiv2BitExact_DD)
#define FUNCTION_fixmuldiv2BitExact_DD
inline LONG fixmuldiv2BitExact_DD(const LONG a, const LONG b) {
  return (LONG)((((INT64)a) * b) >> 32);
}
#endif

#if !defined(FUNCTION_fixmul_DD)
#define FUNCTION_fixmul_DD
inline LONG fixmul_DD(const LONG a, const LONG b) {
  return fixmuldiv2_DD(a, b) << 1;
}
#endif

#if !defined(FUNCTION_fixmulBitExact_DD)
#define FUNCTION_fixmulBitExact_DD
inline LONG fixmulBitExact_DD(const LONG a, const LONG b) {
  return ((LONG)((((INT64)a) * b) >> 32)) << 1;
}
#endif

#if !defined(FUNCTION_fixmuldiv2_SS)
#define FUNCTION_fixmuldiv2_SS
inline LONG fixmuldiv2_SS(const SHORT a, const SHORT b) {
  return ((LONG)a * b);
}
#endif

#if !defined(FUNCTION_fixmul_SS)
#define FUNCTION_fixmul_SS
inline LONG fixmul_SS(const SHORT a, const SHORT b) {
  return (a * b) << 1;
}
#endif

#if !defined(FUNCTION_fixmuldiv2_SD)
#define FUNCTION_fixmuldiv2_SD
inline LONG fixmuldiv2_SD(const SHORT a, const LONG b)
#ifdef FUNCTION_fixmuldiv2_DS
{
  return fixmuldiv2_DS(b, a);
}
#else
{
  return fixmuldiv2_DD(FX_SGL2FX_DBL(a), b);
}
#endif
#endif

#if !defined(FUNCTION_fixmuldiv2_DS)
#define FUNCTION_fixmuldiv2_DS
inline LONG fixmuldiv2_DS(const LONG a, const SHORT b)
#ifdef FUNCTION_fixmuldiv2_SD
{
  return fixmuldiv2_SD(b, a);
}
#else
{
  return fixmuldiv2_DD(a, FX_SGL2FX_DBL(b));
}
#endif
#endif

#if !defined(FUNCTION_fixmuldiv2BitExact_SD)
#define FUNCTION_fixmuldiv2BitExact_SD
inline LONG fixmuldiv2BitExact_SD(const SHORT a, const LONG b)
#ifdef FUNCTION_fixmuldiv2BitExact_DS
{
  return fixmuldiv2BitExact_DS(b, a);
}
#else
{
  return (LONG)((((INT64)a) * b) >> 16);
}
#endif
#endif

#if !defined(FUNCTION_fixmuldiv2BitExact_DS)
#define FUNCTION_fixmuldiv2BitExact_DS
inline LONG fixmuldiv2BitExact_DS(const LONG a, const SHORT b)
#ifdef FUNCTION_fixmuldiv2BitExact_SD
{
  return fixmuldiv2BitExact_SD(b, a);
}
#else
{
  return (LONG)((((INT64)a) * b) >> 16);
}
#endif
#endif

#if !defined(FUNCTION_fixmul_SD)
#define FUNCTION_fixmul_SD
inline LONG fixmul_SD(const SHORT a, const LONG b) {
#ifdef FUNCTION_fixmul_DS
  return fixmul_DS(b, a);
#else
  return fixmuldiv2_SD(a, b) << 1;
#endif
}
#endif

#if !defined(FUNCTION_fixmul_DS)
#define FUNCTION_fixmul_DS
inline LONG fixmul_DS(const LONG a, const SHORT b) {
#ifdef FUNCTION_fixmul_SD
  return fixmul_SD(b, a);
#else
  return fixmuldiv2_DS(a, b) << 1;
#endif
}
#endif

#if !defined(FUNCTION_fixmulBitExact_SD)
#define FUNCTION_fixmulBitExact_SD
inline LONG fixmulBitExact_SD(const SHORT a, const LONG b)
#ifdef FUNCTION_fixmulBitExact_DS
{
  return fixmulBitExact_DS(b, a);
}
#else
{
  return (LONG)(((((INT64)a) * b) >> 16) << 1);
}
#endif
#endif

#if !defined(FUNCTION_fixmulBitExact_DS)
#define FUNCTION_fixmulBitExact_DS
inline LONG fixmulBitExact_DS(const LONG a, const SHORT b)
#ifdef FUNCTION_fixmulBitExact_SD
{
  return fixmulBitExact_SD(b, a);
}
#else
{
  return (LONG)(((((INT64)a) * b) >> 16) << 1);
}
#endif
#endif

#if !defined(FUNCTION_fixpow2div2_D)
#define FUNCTION_fixpow2div2_D
inline LONG fixpow2div2_D(const LONG a) {
  return fixmuldiv2_DD(a, a);
}
#endif

#if !defined(FUNCTION_fixpow2_D)
#define FUNCTION_fixpow2_D
inline LONG fixpow2_D(const LONG a) {
  return fixpow2div2_D(a) << 1;
}
#endif

#if !defined(FUNCTION_fixpow2div2_S)
#define FUNCTION_fixpow2div2_S
inline LONG fixpow2div2_S(const SHORT a) {
  return fixmuldiv2_SS(a, a);
}
#endif

#if !defined(FUNCTION_fixpow2_S)
#define FUNCTION_fixpow2_S
inline LONG fixpow2_S(const SHORT a) {
  LONG result = fixpow2div2_S(a) << 1;
  return result ^ (result >> 31);
}
#endif

#endif /* FIXMUL_H */
