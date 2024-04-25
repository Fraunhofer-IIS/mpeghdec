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

   Description:

*******************************************************************************/

#ifndef FDK_ARCHDEF_H
#define FDK_ARCHDEF_H

/* Unify some few toolchain specific defines to avoid having large "or" macro contraptions all over
 * the source code. */

/* Use single macro (the GCC built in macro) for architecture identification independent of the
 * particular toolchain */
#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) ||  \
    (defined(_MSC_VER) && defined(_M_IX86)) || (defined(_MSC_VER) && defined(_M_X64)) || \
    defined(__x86_64__)
#define __x86__
#endif

#if defined(_M_ARM) && !defined(__arm__) || defined(__aarch64__)
#define __arm__
#endif

#if defined(__wasm__) || defined(__wasm32__)
#define __WASM__
#endif

#if (__TARGET_ARCH_ARM == 5) || defined(__TARGET_FEATURE_DSPMUL) || (_M_ARM == 5) || \
    defined(__ARM_ARCH_5TEJ__) || defined(__ARM_ARCH_7EM__)
/* Define __ARM_ARCH_5TE__ if armv5te features are supported  */
#define __ARM_ARCH_5TE__
#endif

#if (__TARGET_ARCH_ARM == 6) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6ZK__) || \
    defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6T2__)
/* Define __ARM_ARCH_6__ if the armv6 intructions are being supported. */
#define __ARM_ARCH_5TE__
#define __ARM_ARCH_6__
#endif

#if defined(__TARGET_ARCH_7_R) || defined(__ARM_ARCH_7R__)
/* Define __ARM_ARCH_7_A__ if the armv7 intructions are being supported. */
#define __ARM_ARCH_5TE__
#define __ARM_ARCH_6__
#define __ARM_ARCH_7_R__
#endif

#if defined(__TARGET_ARCH_7_A) || defined(__ARM_ARCH_7A__) || \
    ((__ARM_ARCH == 8 || __ARM_ARCH == 9) && (__ARM_32BIT_STATE == 1))
/* Define __ARM_ARCH_7_A__ if the armv7 intructions are being supported. */
#define __ARM_ARCH_5TE__
#define __ARM_ARCH_6__
#define __ARM_ARCH_7_A__
#endif

#if defined(__TARGET_ARCH_7_M) || defined(__ARM_ARCH_7_M__)
/* Define __ARM_ARCH_7M__ if the ARMv7-M instructions are being supported, e.g. Cortex-M3. */
#define __ARM_ARCH_7M__
#endif

#if defined(__TARGET_ARCH_7E_M) || defined(__ARM_ARCH_7E_M__)
/* Define __ARM_ARCH_7EM__ if the ARMv7-ME instructions are being supported, e.g. Cortex-M4. */
#define __ARM_ARCH_7EM__
#endif

#if defined(__aarch64__)
#define __ARM_ARCH_8__
#endif

#if (defined(__TARGET_FEATURE_NEON) || defined(__ARM_NEON)) && !defined(__ARM_NEON__)
/* Detect and unify macros for neon feature. */
#define __ARM_NEON__
#endif
#if defined(__ARM_ARCH_8__)
#undef __ARM_NEON__ /* disable use of ARMv7 legacy NEON */
#define __ARM_AARCH64_NEON__
#endif

#if defined(__THUMBEL__) && defined(__ARM_ARCH_7_A__)
#undef __ARM_ARCH_6__
#undef __ARM_ARCH_7_A__
#undef __ARM_NEON__
#define __ARM_ARCH_7EM__
#endif

#if defined(__APPLE__)
#undef __ARM_NEON__         /* disable use of ARMv7 legacy NEON */
#undef __ARM_AARCH64_NEON__ /* disable use of ARMv8 NEON */
#endif

#ifdef _M_ARM
#include "armintr.h"
#endif

#if defined(__riscv)
#if defined(_LP64)
#define __riscv64__
#else
#define __riscv32__
#endif
#endif

/* Define preferred Multiplication type */

#if defined(__riscv64__) || defined(__riscv32__)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#elif defined(__arm__) && defined(__ARM_ARCH_8__)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#elif defined(__arm__) && defined(__ARM_ARCH_5TE__)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#elif defined(__arm__) && defined(__ARM_ARCH_7M__)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#elif defined(__arm__) && defined(__ARM_ARCH_7EM__)
#define ARCH_PREFER_MULT_32x32
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#elif defined(__arm__) && !defined(__ARM_ARCH_5TE__)
#define ARCH_PREFER_MULT_16x16
#undef SINETABLE_16BIT
#undef WINDOWTABLE_16BIT
#undef POW2COEFF_16BIT
#undef LDCOEFF_16BIT

#elif defined(_M_ARM64)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#elif defined(__WASM__)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define WINDOWTABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT

#elif defined(__x86__)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define WINDOWTABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT

#else
#warning>>>> Please set architecture characterization defines for your platform (FDK_HIGH_PERFORMANCE)! <<<<

#endif /* Architecture switches */

#ifdef SINETABLE_16BIT
#define FIXP_STB FIXP_SGL /* STB sinus Tab used in transformation */
#define FIXP_STP FIXP_SPK
#define STC(a) (FX_DBL2FXCONST_SGL(a))
#else
#define FIXP_STB FIXP_DBL
#define FIXP_STP FIXP_DPK
#define STC(a) ((FIXP_DBL)(LONG)(a))
#endif /* defined(SINETABLE_16BIT) */

#define STCP(cos, sin)     \
  {                        \
    { STC(cos), STC(sin) } \
  }

#ifdef WINDOWTABLE_16BIT
#define FIXP_WTB FIXP_SGL /* single FIXP_SGL values */
#define FX_DBL2FX_WTB(x) FX_DBL2FX_SGL(x)
#define FIXP_WTP FIXP_SPK /* packed FIXP_SGL values */
#define WTC(a) FX_DBL2FXCONST_SGL(a)
#else /* SINETABLE_16BIT */
#define FIXP_WTB FIXP_DBL
#define FX_DBL2FX_WTB(x) (x)
#define FIXP_WTP FIXP_DPK
#define WTC(a) (FIXP_DBL)(a)
#endif /* SINETABLE_16BIT */

#define WTCP(a, b)     \
  {                    \
    { WTC(a), WTC(b) } \
  }

#endif /* FDK_ARCHDEF_H */
