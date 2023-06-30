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

   Author(s):   M. Lohwasser, M. Gayer

   Description: fixed point intrinsics

*******************************************************************************/

#if !defined(FIXMADD_H)
#define FIXMADD_H

#include "FDK_archdef.h"
#include "machine_type.h"
#include "fixmul.h"

#if defined(__arm__)
#include "arm/fixmadd_arm.h"

#endif /* all cores */

/*************************************************************************
 *************************************************************************
    Software fallbacks for missing functions.
**************************************************************************
**************************************************************************/

/* Divide by two versions. */

#if !defined(FUNCTION_fixmadddiv2_DD)
inline FIXP_DBL fixmadddiv2_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return (x + fMultDiv2(a, b));
}
#endif

#if !defined(FUNCTION_fixmadddiv2_SD)
inline FIXP_DBL fixmadddiv2_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
#ifdef FUNCTION_fixmadddiv2_DS
  return fixmadddiv2_DS(x, b, a);
#else
  return fixmadddiv2_DD(x, FX_SGL2FX_DBL(a), b);
#endif
}
#endif

#if !defined(FUNCTION_fixmadddiv2_DS)
inline FIXP_DBL fixmadddiv2_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
#ifdef FUNCTION_fixmadddiv2_SD
  return fixmadddiv2_SD(x, b, a);
#else
  return fixmadddiv2_DD(x, a, FX_SGL2FX_DBL(b));
#endif
}
#endif

#if !defined(FUNCTION_fixmadddiv2_SS)
inline FIXP_DBL fixmadddiv2_SS(FIXP_DBL x, const FIXP_SGL a, const FIXP_SGL b) {
  return x + fMultDiv2(a, b);
}
#endif
#if !defined(FUNCTION_fixmadddiv2_SS_Dual)
inline FIXP_DBL fixmadddiv2_SS_Dual(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return x + fMultDiv2((FIXP_SGL)a, (FIXP_SGL)b) +
         fMultDiv2((FIXP_SGL)(a >> 16), (FIXP_SGL)(b >> 16));
}
#endif

#if !defined(FUNCTION_fixmsubdiv2_DD)
inline FIXP_DBL fixmsubdiv2_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return (x - fMultDiv2(a, b));
}
#endif

#if !defined(FUNCTION_fixmsubdiv2_SD)
inline FIXP_DBL fixmsubdiv2_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
#ifdef FUNCTION_fixmsubdiv2_DS
  return fixmsubdiv2_DS(x, b, a);
#else
  return fixmsubdiv2_DD(x, FX_SGL2FX_DBL(a), b);
#endif
}
#endif

#if !defined(FUNCTION_fixmsubdiv2_DS)
inline FIXP_DBL fixmsubdiv2_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
#ifdef FUNCTION_fixmsubdiv2_SD
  return fixmsubdiv2_SD(x, b, a);
#else
  return fixmsubdiv2_DD(x, a, FX_SGL2FX_DBL(b));
#endif
}
#endif

#if !defined(FUNCTION_fixmsubdiv2_SS)
inline FIXP_DBL fixmsubdiv2_SS(FIXP_DBL x, const FIXP_SGL a, const FIXP_SGL b) {
  return x - fMultDiv2(a, b);
}
#endif

#if !defined(FUNCTION_fixmadddiv2BitExact_DD)
#define FUNCTION_fixmadddiv2BitExact_DD
inline FIXP_DBL fixmadddiv2BitExact_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return x + fMultDiv2BitExact(a, b);
}
#endif
#if !defined(FUNCTION_fixmadddiv2BitExact_SD)
#define FUNCTION_fixmadddiv2BitExact_SD
inline FIXP_DBL fixmadddiv2BitExact_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
#ifdef FUNCTION_fixmadddiv2BitExact_DS
  return fixmadddiv2BitExact_DS(x, b, a);
#else
  return x + fMultDiv2BitExact(a, b);
#endif
}
#endif
#if !defined(FUNCTION_fixmadddiv2BitExact_DS)
#define FUNCTION_fixmadddiv2BitExact_DS
inline FIXP_DBL fixmadddiv2BitExact_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
#ifdef FUNCTION_fixmadddiv2BitExact_SD
  return fixmadddiv2BitExact_SD(x, b, a);
#else
  return x + fMultDiv2BitExact(a, b);
#endif
}
#endif

#if !defined(FUNCTION_fixmsubdiv2BitExact_DD)
#define FUNCTION_fixmsubdiv2BitExact_DD
inline FIXP_DBL fixmsubdiv2BitExact_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return x - fMultDiv2BitExact(a, b);
}
#endif
#if !defined(FUNCTION_fixmsubdiv2BitExact_SD)
#define FUNCTION_fixmsubdiv2BitExact_SD
inline FIXP_DBL fixmsubdiv2BitExact_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
#ifdef FUNCTION_fixmsubdiv2BitExact_DS
  return fixmsubdiv2BitExact_DS(x, b, a);
#else
  return x - fMultDiv2BitExact(a, b);
#endif
}
#endif
#if !defined(FUNCTION_fixmsubdiv2BitExact_DS)
#define FUNCTION_fixmsubdiv2BitExact_DS
inline FIXP_DBL fixmsubdiv2BitExact_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
#ifdef FUNCTION_fixmsubdiv2BitExact_SD
  return fixmsubdiv2BitExact_SD(x, b, a);
#else
  return x - fMultDiv2BitExact(a, b);
#endif
}
#endif

/* Normal versions */

#if !defined(FUNCTION_fixmadd_DD)
inline FIXP_DBL fixmadd_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return fixmadddiv2_DD(x, a, b) << 1;
}
#endif
#if !defined(FUNCTION_fixmadd_SD)
inline FIXP_DBL fixmadd_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
#ifdef FUNCTION_fixmadd_DS
  return fixmadd_DS(x, b, a);
#else
  return fixmadd_DD(x, FX_SGL2FX_DBL(a), b);
#endif
}
#endif
#if !defined(FUNCTION_fixmadd_DS)
inline FIXP_DBL fixmadd_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
#ifdef FUNCTION_fixmadd_SD
  return fixmadd_SD(x, b, a);
#else
  return fixmadd_DD(x, a, FX_SGL2FX_DBL(b));
#endif
}
#endif
#if !defined(FUNCTION_fixmadd_SS)
inline FIXP_DBL fixmadd_SS(FIXP_DBL x, const FIXP_SGL a, const FIXP_SGL b) {
  return (x + fMultDiv2(a, b)) << 1;
}
#endif

#if !defined(FUNCTION_fixmsub_DD)
inline FIXP_DBL fixmsub_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return fixmsubdiv2_DD(x, a, b) << 1;
}
#endif
#if !defined(FUNCTION_fixmsub_SD)
inline FIXP_DBL fixmsub_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
#ifdef FUNCTION_fixmsub_DS
  return fixmsub_DS(x, b, a);
#else
  return fixmsub_DD(x, FX_SGL2FX_DBL(a), b);
#endif
}
#endif
#if !defined(FUNCTION_fixmsub_DS)
inline FIXP_DBL fixmsub_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
#ifdef FUNCTION_fixmsub_SD
  return fixmsub_SD(x, b, a);
#else
  return fixmsub_DD(x, a, FX_SGL2FX_DBL(b));
#endif
}
#endif
#if !defined(FUNCTION_fixmsub_SS)
inline FIXP_DBL fixmsub_SS(FIXP_DBL x, const FIXP_SGL a, const FIXP_SGL b) {
  return (x - fMultDiv2(a, b)) << 1;
}
#endif

#if !defined(FUNCTION_fixpow2adddiv2_D)
#ifdef FUNCTION_fixmadddiv2_DD
#define fixpadddiv2_D(x, a) fixmadddiv2_DD(x, a, a)
#else
inline INT fixpadddiv2_D(FIXP_DBL x, const FIXP_DBL a) {
  return (x + fPow2Div2(a));
}
#endif
#endif
#if !defined(FUNCTION_fixpow2add_D)
inline INT fixpadd_D(FIXP_DBL x, const FIXP_DBL a) {
  return (x + fPow2(a));
}
#endif

#if !defined(FUNCTION_fixpow2adddiv2_S)
#ifdef FUNCTION_fixmadddiv2_SS
#define fixpadddiv2_S(x, a) fixmadddiv2_SS(x, a, a)
#else
inline INT fixpadddiv2_S(FIXP_DBL x, const FIXP_SGL a) {
  return (x + fPow2Div2(a));
}
#endif
#endif
#if !defined(FUNCTION_fixpow2add_S)
inline INT fixpadd_S(FIXP_DBL x, const FIXP_SGL a) {
  return (x + fPow2(a));
}
#endif

#endif /* FIXMADD_H */
