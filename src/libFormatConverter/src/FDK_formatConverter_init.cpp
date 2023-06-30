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

/******************** MPEG-H 3DA channel rendering library *********************

   Author(s):

   Description:

*******************************************************************************/

/***********************************************************************************

 This software module was originally developed by

 Fraunhofer IIS

 in the course of development of the ISO/IEC 23008-3 for reference purposes and its
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard
 and which satisfy any specified conformance criteria. Those intending to use this
 software module in products are advised that its use may infringe existing patents.
 ISO/IEC have no liability for use of this software module or modifications thereof.
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3
 standard.

 Fraunhofer IIS retains full right to modify and use the code for
 their own purpose, assign or donate the code to a third party and to inhibit third
 parties from using the code for products that do not conform to MPEG-related ITU
 Recommendations and/or ISO/IEC International Standards.

 This copyright notice must be included in all copies or derivative works.

 Copyright (c) ISO/IEC 2013.

 ***********************************************************************************/

#include "FDK_formatConverter_init.h"
#include "FDK_formatConverter_constants.h"

#include "FDK_trigFcts.h"

#define STFT_ERB_BANDS 58

/* prototypes */
static void compute_eq(const FIXP_DBL* bands_nrm, const INT nbands, const INT sfreq_Hz,
                       const INT eq_1_idx, const FIXP_DBL eq_1_strength, const INT eq_2_idx,
                       const FIXP_DBL eq_2_strength, FIXP_EQ_H* eq);

static int find_channel(const int ch, const AFC_FORMAT_CONVERTER_CHANNEL_ID* channels,
                        const int nchan);

/* hard coded parameters
 * ************************************************************************** */

#define TC_AEQ_MS (60.0f)         /* time constant for adaptive eq smoothing [ms] */
#define AEQ_MAX_LIMIT_DB (8.0f)   /* adaptive equatization gain upper limit [dB] */
#define AEQ_MIN_LIMIT_DB (-10.0f) /* adaptive equatization gain upper limit [dB] */
#define SOS_M_S (340.0f)          /* speed of sound [m/s] */

/* parameter range limitations */
#define SFREQ_HZ_MIN (8000)         /* minimum sampling frequency */
#define SFREQ_HZ_MAX (384000)       /* maximum sampling frequency */
#define NBANDS_MIN (10)             /* minimum number of bands */
#define NBANDS_MAX (2048)           /* maximum number of bands */
#define BLOCKLENGTH_MIN_MS (0.167f) /* minimum blocklength [ms] */
#define BLOCKLENGTH_MAX_MS (10.0f)  /* maximum blocklength [ms] */
#define RANDOMIZATION_AZI_MAX (35)  /* maximum azi deviation [deg] */
#define RANDOMIZATION_ELE_MAX (55)  /* maximum ele deviation [deg] */
#define RANDOMIZATION_SPK_ANGLE_MIN \
  FL2FXCONST_DBL(15.0f / 360.f)     \
  /* minimum angle between any loudspeaker pair [deg] */ /* mapped to -2pi .. 2pi */

/* definitions
 * ************************************************************************** */

const FIXP_DBL ch_azi_ele[CH_M_RSCR + 1][2] = {
    {FL2FXCONST_DBL(FIX_ANGLE(0.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+15.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-15.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+30.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-30.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+45.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-45.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+60.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-60.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+90.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-90.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+110.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-110.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+135.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-135.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+150.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-150.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(180.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(0.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+45.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-45.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+30.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-30.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+90.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-90.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+110.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-110.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+135.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-135.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(180.0)), FL2FXCONST_DBL(FIX_ANGLE(35.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(0.0)), FL2FXCONST_DBL(FIX_ANGLE(90.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(0.0)), FL2FXCONST_DBL(FIX_ANGLE(-15.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+45.0)), FL2FXCONST_DBL(FIX_ANGLE(-15.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-45.0)), FL2FXCONST_DBL(FIX_ANGLE(-15.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(0.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(0.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(0.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(+60.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))},
    {FL2FXCONST_DBL(FIX_ANGLE(-60.0)), FL2FXCONST_DBL(FIX_ANGLE(0.0))}};

/* downmix rules */

#define G1 \
  FL2FXCONST_DBL(0.8) /* gain (0..100 corresponds to 0.0..1.0) when mapping rear => front */
#define G2                                                                                        \
  FL2FXCONST_DBL(0.6) /* gain (0..100 corresponds to 0.0..1.0) when mapping vary far (rear center \
                         => front center) */
#define G3                                                                                        \
  FL2FXCONST_DBL(0.85) /* gain (0..100 corresponds to 0.0..1.0) when mapping height to horizontal \
                        */
#define G4                                                                                      \
  MAXVAL_DBL /* gain (0..100 corresponds to 0.0..1.0) for 60deg and 90deg sources on ITU target \
                setups */

#define ANGLE_30 FL2FXCONST_DBL(1 / 12)

const INT dmx_rules_classic[MAX_NUM_DMX_RULES][8] = {
    /* input    dest 1       dest 2      gain   processing idx/params */
    {CH_M_000, CH_M_L022, CH_M_R022, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_000, CH_M_L030, CH_M_R030, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L022, CH_M_000, CH_M_L030, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L022, CH_M_L030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R022, CH_M_000, CH_M_R030, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R022, CH_M_R030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L045, CH_M_L030, CH_M_L060, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L045, CH_M_L030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R045, CH_M_R030, CH_M_R060, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R045, CH_M_R030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L060, CH_M_L045, CH_M_L090, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L060, CH_M_L030, CH_M_L090, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L060, CH_M_L045, CH_M_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L060, CH_M_L030, CH_M_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L060, CH_M_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L060, CH_M_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R060, CH_M_R045, CH_M_R090, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R060, CH_M_R030, CH_M_R090, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R060, CH_M_R045, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R060, CH_M_R030, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R060, CH_M_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R060, CH_M_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L090, CH_M_L060, CH_M_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L090, CH_M_L045, CH_M_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L090, CH_M_L030, CH_M_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L090, CH_M_L060, CH_M_L135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L090, CH_M_L060, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L090, CH_M_L045, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L090, CH_M_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R090, CH_M_R060, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R090, CH_M_R045, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R090, CH_M_R030, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R090, CH_M_R060, CH_M_R135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R090, CH_M_R060, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R090, CH_M_R045, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R090, CH_M_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L110, CH_M_L135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L110, CH_M_L090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L110, CH_M_L060, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L110, CH_M_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L110, CH_M_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R110, CH_M_R135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R110, CH_M_R090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R110, CH_M_R060, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R110, CH_M_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R110, CH_M_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L135, CH_M_L110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L135, CH_M_L150, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L135, CH_M_L090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L135, CH_M_L060, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L135, CH_M_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L135, CH_M_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R135, CH_M_R110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R135, CH_M_R150, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R135, CH_M_R090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R135, CH_M_R060, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R135, CH_M_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R135, CH_M_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L150, CH_M_L135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L150, CH_M_L110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L150, CH_M_L090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L150, CH_M_L060, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L150, CH_M_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L150, CH_M_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R150, CH_M_R135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R150, CH_M_R110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R150, CH_M_R090, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R150, CH_M_R060, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R150, CH_M_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R150, CH_M_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_180, CH_M_L150, CH_M_R150, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_180, CH_M_L135, CH_M_R135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_180, CH_M_L110, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_180, CH_M_L090, CH_M_R090, G1, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_180, CH_M_L060, CH_M_R060, G1, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_180, CH_M_L045, CH_M_R045, G2, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_180, CH_M_L030, CH_M_R030, G2, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    /**/
    {CH_U_000, CH_U_L030, CH_U_R030, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_000, CH_U_L045, CH_U_R045, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_000, CH_M_L030, CH_M_R030, G3, RULE_PANNING, ANGLE_30, 0, RULE_EQ1},
    /**/
    {CH_U_L045, CH_U_L030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L045, CH_M_L045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    {CH_U_L045, CH_M_L030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    /**/
    {CH_U_R045, CH_U_R030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R045, CH_M_R045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    {CH_U_R045, CH_M_R030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    /**/
    {CH_U_L030, CH_U_L045, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L030, CH_M_L030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    /**/
    {CH_U_R030, CH_U_R045, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R030, CH_M_R030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    /**/
    {CH_U_L090, CH_U_L030, CH_U_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_L090, CH_U_L030, CH_U_L135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_L090, CH_U_L045, CH_U_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_L090, CH_U_L045, CH_U_L135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_L090, CH_U_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L090, CH_U_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L090, CH_M_L045, CH_M_L110, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_L090, CH_M_L030, CH_M_L110, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_L090, CH_M_L090, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L090, CH_M_L060, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L090, CH_M_L045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L090, CH_M_L030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_R090, CH_U_R030, CH_U_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_R090, CH_U_R030, CH_U_R135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_R090, CH_U_R045, CH_U_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_R090, CH_U_R045, CH_U_R135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_R090, CH_U_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R090, CH_U_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R090, CH_M_R045, CH_M_R110, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_R090, CH_M_R030, CH_M_R110, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_R090, CH_M_R090, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R090, CH_M_R060, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R090, CH_M_R045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R090, CH_M_R030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_L110, CH_U_L135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L110, CH_U_L090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L110, CH_U_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L110, CH_U_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L110, CH_M_L110, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L110, CH_M_L135, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L110, CH_M_L090, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L110, CH_M_L060, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L110, CH_M_L045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L110, CH_M_L030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_R110, CH_U_R135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R110, CH_U_R090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R110, CH_U_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R110, CH_U_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R110, CH_M_R110, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R110, CH_M_R135, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R110, CH_M_R090, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R110, CH_M_R060, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R110, CH_M_R045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R110, CH_M_R030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_L135, CH_U_L110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L135, CH_U_L090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L135, CH_U_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L135, CH_U_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L135, CH_M_L135, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L135, CH_M_L110, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L135, CH_M_L090, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L135, CH_M_L060, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L135, CH_M_L045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L135, CH_M_L030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_R135, CH_U_R110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R135, CH_U_R090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R135, CH_U_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R135, CH_U_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R135, CH_M_R135, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R135, CH_M_R110, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R135, CH_M_R090, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R135, CH_M_R060, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R135, CH_M_R045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R135, CH_M_R030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_180, CH_U_L135, CH_U_R135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_180, CH_U_L110, CH_U_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_180, CH_M_180, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_180, CH_M_L135, CH_M_R135, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_180, CH_M_L110, CH_M_R110, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_180, CH_U_L090, CH_U_R090, G1, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_180, CH_U_L045, CH_U_R045, G1, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_180, CH_U_L030, CH_U_R030, G1, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_180, CH_M_L090, CH_M_R090, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_180, CH_M_L060, CH_M_R060, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_180, CH_M_L045, CH_M_R045, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_180, CH_M_L030, CH_M_R030, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    /**/
    {CH_T_000, -1, -1, G1, RULE_TOP2ALLU, 0, 0, RULE_EQ3},
    {CH_T_000, -1, -1, G1, RULE_TOP2ALLM, 0, 0, RULE_EQ4},
    /**/
    {CH_L_000, CH_M_000, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_L_000, CH_M_L030, CH_M_R030, G4, RULE_PANNING, ANGLE_30, 0, RULE_NOPROC},
    /**/
    {CH_L_L045, CH_M_L045, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_L_L045, CH_M_L030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_L_R045, CH_M_R045, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_L_R045, CH_M_R030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_LFE1, CH_LFE2, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_LFE1, CH_M_L030, CH_M_R030, G4, RULE_PANNING, ANGLE_30, 0, RULE_NOPROC},
    /**/
    {CH_LFE2, CH_LFE1, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_LFE2, CH_M_L030, CH_M_R030, G4, RULE_PANNING, ANGLE_30, 0, RULE_NOPROC},
    /**/
    {CH_LFE3, CH_LFE2, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_LFE3, CH_M_L030, CH_M_R030, G4, RULE_PANNING, ANGLE_30, 0, RULE_NOPROC},

    /* indicate end of matrix */
    {-1, -1, -1, -1, -1, -1, -1, -1}};

/* const DMX_RULES_ARRAY  */
const INT dmx_rules_immersive[MAX_NUM_DMX_RULES][8] = {
    /* input    dest 1       dest 2      gain   processing idx/params */
    {CH_M_000, CH_M_L022, CH_M_R022, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_000, CH_M_L030, CH_M_R030, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L022, CH_M_000, CH_M_L030, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L022, CH_M_L030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R022, CH_M_000, CH_M_R030, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R022, CH_M_R030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L045, CH_M_L030, CH_M_L060, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L045, CH_M_L030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R045, CH_M_R030, CH_M_R060, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R045, CH_M_R030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L060, CH_M_L045, CH_M_L090, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L060, CH_M_L030, CH_M_L090, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L060, CH_M_L045, CH_M_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L060, CH_M_L030, CH_M_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L060, CH_M_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R060, CH_M_R045, CH_M_R090, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R060, CH_M_R030, CH_M_R090, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R060, CH_M_R045, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R060, CH_M_R030, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R060, CH_M_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L090, CH_M_L060, CH_M_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L090, CH_M_L045, CH_M_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L090, CH_M_L030, CH_M_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_L090, CH_M_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R090, CH_M_R060, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R090, CH_M_R045, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R090, CH_M_R030, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_R090, CH_M_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L110, CH_M_L135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L110, CH_M_L090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L110, CH_M_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L110, CH_M_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R110, CH_M_R135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R110, CH_M_R090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R110, CH_M_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R110, CH_M_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L135, CH_M_L110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L135, CH_M_L150, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L135, CH_M_L090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L135, CH_M_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L135, CH_M_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R135, CH_M_R110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R135, CH_M_R150, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R135, CH_M_R090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R135, CH_M_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R135, CH_M_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_L150, CH_M_L135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L150, CH_M_L110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L150, CH_M_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_L150, CH_M_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_R150, CH_M_R135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R150, CH_M_R110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R150, CH_M_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_M_R150, CH_M_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /**/
    {CH_M_180, CH_M_L150, CH_M_R150, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_180, CH_M_L135, CH_M_R135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_180, CH_M_L110, CH_M_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_180, CH_M_L090, CH_M_R090, G1, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_180, CH_M_L045, CH_M_R045, G2, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_M_180, CH_M_L030, CH_M_R030, G2, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    /**/
    {CH_U_000, CH_U_L030, CH_U_R030, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_000, TFC, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVFC},
    {CH_U_000, CH_M_L030, CH_M_R030, G3, RULE_PANNING, ANGLE_30, 0, RULE_EQ1},
    /**/
    {CH_U_L045, CH_U_L030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_L045, TFL, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVF},
    {CH_U_L045, CH_M_L045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    {CH_U_L045, CH_M_L030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    /**/
    {CH_U_R045, CH_U_R030, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_R045, TFR, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVF},
    {CH_U_R045, CH_M_R045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    {CH_U_R045, CH_M_R030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    /**/
    {CH_U_L030, CH_U_L045, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_L030, TFLA, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVF},
    {CH_U_L030, CH_M_L030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    /**/
    {CH_U_R030, CH_U_R045, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_R030, TFRA, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVF},
    {CH_U_R030, CH_M_R030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ1},
    /**/
    {CH_U_L090, CH_U_L030, CH_U_L110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_L090, CH_U_L030, CH_U_L135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_L090, CH_U_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L090, CH_U_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_L090, TSL, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVS},
    {CH_U_L090, CH_M_L045, CH_M_L110, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_L090, CH_M_L030, CH_M_L110, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_L090, CH_M_L030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_R090, CH_U_R030, CH_U_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_R090, CH_U_R030, CH_U_R135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_R090, CH_U_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R090, CH_U_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_R090, TSR, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVS},
    {CH_U_R090, CH_M_R045, CH_M_R110, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_R090, CH_M_R030, CH_M_R110, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_R090, CH_M_R030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_L110, CH_U_L135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L110, CH_U_L090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L110, CH_U_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L110, CH_U_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_L110, TBLA, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVBA},
    {CH_U_L110, CH_M_L110, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L110, CH_M_L045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L110, CH_M_L030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_R110, CH_U_R135, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R110, CH_U_R090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R110, CH_U_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R110, CH_U_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_R110, TBRA, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVBA},
    {CH_U_R110, CH_M_R110, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R110, CH_M_R045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R110, CH_M_R030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_L135, CH_U_L110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L135, CH_U_L090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L135, CH_U_L045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_L135, CH_U_L030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_L135, TBL, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVB},
    {CH_U_L135, CH_M_L110, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L135, CH_M_L045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_L135, CH_M_L030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_R135, CH_U_R110, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R135, CH_U_R090, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R135, CH_U_R045, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_U_R135, CH_U_R030, -1, G1, RULE_NOPROC, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_R135, TBR, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVB},
    {CH_U_R135, CH_M_R110, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R135, CH_M_R045, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_R135, CH_M_R030, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    /**/
    {CH_U_180, CH_U_L135, CH_U_R135, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_180, CH_U_L110, CH_U_R110, G4, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    /* Adding the spatial elevation rendering */
    {CH_U_180, TBC, -1, G4, RULE_VIRTUAL, 0, 0, RULE_EQVBC},
    {CH_U_180, CH_M_180, -1, G3, RULE_NOPROC, 0, 0, RULE_EQ2},
    {CH_U_180, CH_M_L110, CH_M_R110, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    {CH_U_180, CH_U_L030, CH_U_R030, G1, RULE_AUTOPAN, 0, 0, RULE_NOPROC},
    {CH_U_180, CH_M_L030, CH_M_R030, G3, RULE_AUTOPAN, 0, 0, RULE_EQ2},
    /**/
    {CH_T_000, -1, -1, G1, RULE_TOP2ALLU, 0, 0, RULE_EQ3},
    /* Adding the spatial elevation rendering */
    {CH_T_000, VOG, -1, G1, RULE_VIRTUAL, 0, 0, RULE_EQVOG},
    {CH_T_000, -1, -1, G1, RULE_TOP2ALLM, 0, 0, RULE_EQ4},
    /**/
    {CH_L_000, CH_M_000, -1, G4, RULE_NOPROC, 0, 0, RULE_EQBTM},
    {CH_L_000, CH_M_L030, CH_M_R030, G4, RULE_PANNING, ANGLE_30, 0, RULE_EQBTM},
    /**/
    {CH_L_L045, CH_M_L045, -1, G4, RULE_NOPROC, 0, 0, RULE_EQBTM},
    {CH_L_L045, CH_M_L030, -1, G4, RULE_NOPROC, 0, 0, RULE_EQBTM},
    {CH_L_R045, CH_M_R045, -1, G4, RULE_NOPROC, 0, 0, RULE_EQBTM},
    {CH_L_R045, CH_M_R030, -1, G4, RULE_NOPROC, 0, 0, RULE_EQBTM},
    /**/
    {CH_LFE1, CH_LFE2, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_LFE1, CH_M_L030, CH_M_R030, G4, RULE_PANNING, ANGLE_30, 0, RULE_NOPROC},
    /**/
    {CH_LFE2, CH_LFE1, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_LFE2, CH_M_L030, CH_M_R030, G4, RULE_PANNING, ANGLE_30, 0, RULE_NOPROC},
    /**/
    {CH_LFE3, CH_LFE2, -1, G4, RULE_NOPROC, 0, 0, RULE_NOPROC},
    {CH_LFE3, CH_M_L030, CH_M_R030, G4, RULE_PANNING, ANGLE_30, 0, RULE_NOPROC},
    /* indicate end of matrix */
    {-1, -1, -1, -1, -1, -1, -1, -1}};

/* Internal functions
 * ************************************************************************** */

/* functions */

static INT find_channel(const INT ch, const AFC_FORMAT_CONVERTER_CHANNEL_ID* channels,
                        const INT nchan) {
  int i;
  for (i = 0; i < nchan; i++) {
    if (channels[i] == ch) {
      return i;
    }
  }
  return -1;
}

/* This function is part of the equalizers calculation and it returns a result value and its
 * exponent in result_e. */

#define DBL_16_div_20 (0x66666666)
#define DBL_10_div_16 (0x50000000)

FIXP_DBL peak_filter(FIXP_DBL f,          /* peak frequency [Hz] */
                     INT f_e, FIXP_DBL q, /* peak Q factor */
                     INT q_e, FIXP_DBL g, /* peak gain */
                     INT g_e, FIXP_DBL G, /* gain */
                     INT G_e, FIXP_DBL b, /* band center frequency [Hz] */
                     INT b_e, INT* result_e) {
  FIXP_DBL tmp_b = 0, tmp_f = 0, tmp_fb = 0, tmp_q = 0, tmp_v = 0, tmp_V0 = 0, tmp_sum = 0,
           tmp_g_v = 0, tmp_g_1 = 0, tmp_G = 0, gain = 0, result = 0, V0 = 0;
  INT V0_e = 0, tmp_b_e = 0, tmp_f_e = 0, tmp_fb_e = 0, tmp_q_e = 0, tmp_v_e = 0, tmp_sum_e = 0,
      tmp_g_v_e = 0, tmp_g_1_e = 0, gain_e = 0;

  INT norm_b = CountLeadingBits(b);
  b <<= norm_b;
  b_e -= norm_b;

  INT norm_q = CountLeadingBits(q);
  q <<= norm_q;
  q_e -= norm_q;

  /* V0 */

  /* peak gain in linear domain */
  tmp_V0 = fMult((FIXP_DBL)DBL_16_div_20, fAbs(g));
  V0_e += g_e - 4;
  V0 = fPow((FIXP_DBL)DBL_10_div_16, 4, tmp_V0, V0_e, &V0_e);

  /* f*f*b*b */
  FIXP_DBL tmp_fPow2_f = fMult(f, f);
  FIXP_DBL tmp_fPow2_b = fMult(b, b);
  tmp_fb = fMult(tmp_fPow2_f, tmp_fPow2_b);
  tmp_fb_e = 2 * b_e + 2 * f_e;

  /* b*b*b*b */
  tmp_b = fMult(tmp_fPow2_b, tmp_fPow2_b);
  tmp_b_e = 4 * b_e;

  /* f*f*f*f */
  tmp_f = fMult(tmp_fPow2_f, tmp_fPow2_f);
  tmp_f_e = 4 * f_e;

  /* tmp_q = (1.0f/(q*q) - 2.0f) */
  /* tmp_v = (V0*V0/(q*q) - 2.0f) */
  tmp_q = fMult(q, q);
  tmp_q_e = 2 * q_e;
  norm_q = CountLeadingBits(tmp_q);
  tmp_q <<= norm_q;
  tmp_q_e -= norm_q;
  tmp_v = fMult(V0, V0);
  V0_e = 2 * V0_e;
  tmp_v = fDivNormSigned(tmp_v, tmp_q, &tmp_v_e);
  tmp_v_e = tmp_v_e + (V0_e - tmp_q_e);
  tmp_q = invFixp(tmp_q, &tmp_q_e);  // -fDivNormSigned( MINVAL_DBL, tmp_q, &tmp_q_e);
  tmp_q = fAddNorm(tmp_q, tmp_q_e, MINVAL_DBL, 1, &tmp_q_e);
  tmp_v = fAddNorm(tmp_v, tmp_v_e, MINVAL_DBL, 1, &tmp_v_e);

  /* b*b*b*b + f*f*f*f */
  tmp_sum = fAddNorm(tmp_f, tmp_f_e, tmp_b, tmp_b_e, &tmp_sum_e);

  /* b*b*b*b + (V0*V0/(q*q) - 2.0f)*f*f*b*b + f*f*f*f */
  tmp_g_v = fAddNorm(tmp_sum, tmp_sum_e, fMult(tmp_v, tmp_fb), (tmp_v_e + tmp_fb_e), &tmp_g_v_e);

  /* b*b*b*b + (1.0f/(q*q) - 2.0f)*f*f*b*b + f*f*f*f */
  tmp_g_1 = fAddNorm(tmp_sum, tmp_sum_e, fMult(tmp_q, tmp_fb), (tmp_q_e + tmp_fb_e), &tmp_g_1_e);

  /* 2nd order peak filter magnitude response */
  if (g < (FIXP_DBL)0) {
    gain = fDivNormSigned(tmp_g_1, tmp_g_v, &gain_e);
    gain_e = gain_e + tmp_g_1_e - tmp_g_v_e;
  } else {
    gain = fDivNormSigned(tmp_g_v, tmp_g_1, &gain_e);
    gain_e = gain_e - tmp_g_1_e + tmp_g_v_e;
  }

  INT tmp_G_e = 0;
  tmp_G = fMult((FIXP_DBL)DBL_16_div_20, G);
  tmp_G_e = tmp_G_e + G_e - 4;
  tmp_G = fPow((FIXP_DBL)DBL_10_div_16, 4, tmp_G, tmp_G_e, &tmp_G_e);

  if (gain_e & 0x1) {
    gain_e++;
    gain >>= 1;
  }

  result = sqrtFixp(gain);
  *result_e += (gain_e >> 1);

  result = fMult(result, tmp_G);
  *result_e += tmp_G_e;

  return result;
}

#define DBL_0_dot_8 (0x66666666)
#define DBL_0_dot_5 (0x40000000)
#define DBL_0_dot_3 (0x26666666)
#define DBL_0_dot_25 (0x20000000)

#define DBL_n2_div_4 (0xC0000000)
#define DBL_2_div_4 (0x40000000)
#define DBL_n6dot5_div_8 (0x98000000)
#define DBL_4dot5_div_8 (0x48000000)
#define DBL_n3dot5_div_4 (0x90000000)
#define DBL_n3dot1_div_4 (0x9ccccccd)
#define DBL_1dot8_div_2 (0x73333333)
#define DBL_n1dot3_div_2 (0xaccccccd)
#define DBL_0dot7_div_1 (0x5999999a)

/* compute_eq:
    bands_nrm:     constant Q31 vector.
    nbands:        Number of bands.
    sfreq_Hz:      Sampling freq.
    eq_1_idx:      Equalizer index 1.
    eq_1_strength: Q31 equalizer strength.
    eq_2_idx:      Equalizer index 2.
    eq_2_strength: Q31 equalizer strength.
    eq:            Resulting equalizer vector in Q14 format when FIXP_DMX_H = FIXP_SGL or Q30 when
   FIXP_DMX_H = FIXP_DBL.
*/

static void compute_eq(const FIXP_DBL* bands_nrm, const INT nbands, const INT sfreq_Hz,
                       const INT eq_1_idx, const FIXP_DBL eq_1_strength, const INT eq_2_idx,
                       const FIXP_DBL eq_2_strength, FIXP_EQ_H* eq) {
  INT i, eq_e;
  FIXP_DBL f;
  FIXP_DBL tmp;

  /* initialize static eqs */

  FIXP_DBL fe1_1 = 0, fe1_2 = 0, fe1_3 = 0, qe1_1 = 0, qe1_2 = 0, qe1_3 = 0, ge1_1 = 0, ge1_2 = 0,
           ge1_3 = 0, Ge1_1 = 0, Ge1_2 = 0, Ge1_3 = 0;
  FIXP_DBL fe2_1 = 0, fe2_2 = 0, fe2_3 = 0, qe2_1 = 0, qe2_2 = 0, qe2_3 = 0, ge2_1 = 0, ge2_2 = 0,
           ge2_3 = 0, Ge2_1 = 0, Ge2_2 = 0, Ge2_3 = 0;
  INT fe1_1_e = 0, fe1_2_e = 0, fe1_3_e = 0, ge1_1_e = 0, ge1_2_e = 0, ge1_3_e = 0, Ge1_1_e = 0,
      Ge1_2_e = 0, Ge1_3_e = 0;
  INT fe2_1_e = 0, fe2_2_e = 0, fe2_3_e = 0, ge2_1_e = 0, ge2_2_e = 0, ge2_3_e = 0, Ge2_1_e = 0,
      Ge2_2_e = 0, Ge2_3_e = 0;

  FIXP_DBL tmp_sfreq_Hz;

  tmp_sfreq_Hz = (FIXP_DBL)sfreq_Hz;
  tmp_sfreq_Hz <<= 15;

  /* RULE_EQ1, RULE_EQ2, RULE_EQ5 */
  if (eq_1_idx == RULE_EQ1) {
    fe1_1 = (FIXP_DBL)12000;
    fe1_1 <<= 16;
    fe1_1_e = 15;
    qe1_1 = (FIXP_DBL)DBL_0_dot_3;
    ge1_1 = fMult((FIXP_DBL)DBL_n2_div_4, eq_1_strength);
    ge1_1_e = 2;
    Ge1_1 = eq_1_strength;
    Ge1_1_e = 0;
  }
  if (eq_2_idx == RULE_EQ1) {
    fe2_1 = (FIXP_DBL)12000;
    fe2_1 <<= 16;
    fe2_1_e = 15;
    qe2_1 = (FIXP_DBL)DBL_0_dot_3;
    ge2_1 = fMult((FIXP_DBL)DBL_n2_div_4, eq_2_strength);
    ge2_1_e = 2;
    Ge2_1 = eq_2_strength;
    Ge2_1_e = 0;
  }
  if (eq_1_idx == RULE_EQ2) {
    fe1_1 = (FIXP_DBL)12000;
    fe1_1 <<= 16;
    fe1_1_e = 15;
    qe1_1 = (FIXP_DBL)DBL_0_dot_3;
    ge1_1 = fMult((FIXP_DBL)DBL_n3dot5_div_4, eq_1_strength);
    ge1_1_e = 2;
    Ge1_1 = eq_1_strength;
    Ge1_1_e = 0;
  }
  if (eq_2_idx == RULE_EQ2) {
    fe2_1 = (FIXP_DBL)12000;
    fe2_1 <<= 16;
    fe2_1_e = 15;
    qe2_1 = (FIXP_DBL)DBL_0_dot_3;
    ge2_1 = fMult((FIXP_DBL)DBL_n3dot5_div_4, eq_2_strength);
    ge2_1_e = 2;
    Ge2_1 = eq_2_strength;
    Ge2_1_e = 0;
  }
  if (eq_1_idx == RULE_EQ5) {
    fe1_1 = (FIXP_DBL)35;
    fe1_1 <<= 25;
    fe1_1_e = 6;
    qe1_1 = (FIXP_DBL)DBL_0_dot_25;
    ge1_1 = fMult((FIXP_DBL)DBL_n1dot3_div_2, eq_1_strength);
    ge1_1_e = 1;
    Ge1_1 = eq_1_strength;
    Ge1_1_e = 0;
  }
  if (eq_2_idx == RULE_EQ5) {
    fe2_1 = (FIXP_DBL)35;
    fe2_1 <<= 25;
    fe2_1_e = 6;
    qe2_1 = (FIXP_DBL)DBL_0_dot_25;
    ge2_1 = fMult((FIXP_DBL)DBL_n1dot3_div_2, eq_2_strength);
    ge2_1_e = 1;
    Ge2_1 = eq_2_strength;
    Ge2_1_e = 0;
  }

  if ((eq_1_idx == RULE_EQ1) || (eq_1_idx == RULE_EQ2) || (eq_1_idx == RULE_EQ5) ||
      (eq_2_idx == RULE_EQ1) || (eq_2_idx == RULE_EQ2) || (eq_2_idx == RULE_EQ5)) {
    for (i = 0; i < nbands; i++) {
      eq[i] = (FIXP_EQ_H)MAXVAL_EQ_H;
      eq_e = 0;
      if (eq_1_idx != RULE_NOPROC) {
        f = fMult(fAbs(bands_nrm[i]), tmp_sfreq_Hz);
        tmp =
            peak_filter(fe1_1, fe1_1_e, qe1_1, 0, ge1_1, ge1_1_e, Ge1_1, Ge1_1_e, f, 16 - 1, &eq_e);
        eq[i] = scaleValue(FX_DBL2FX_EQ_H(fMult(eq[i], tmp)), (eq_e - EQ_H_EXP));
      }
      if (eq_2_idx != RULE_NOPROC) {
        f = fMult(fAbs(bands_nrm[i]), tmp_sfreq_Hz);
        tmp =
            peak_filter(fe2_1, fe2_1_e, qe2_1, 0, ge2_1, ge2_1_e, Ge2_1, Ge2_1_e, f, 16 - 1, &eq_e);
        eq[i] = scaleValue(FX_DBL2FX_EQ_H(fMult(eq[i], tmp)), (eq_e - EQ_H_EXP));
      }
    }
  }

  /* RULE_EQ3 */
  if (eq_1_idx == RULE_EQ3) {
    fe1_1 = (FIXP_DBL)200;
    fe1_1 <<= 23;
    fe1_1_e = 8;
    qe1_1 = (FIXP_DBL)DBL_0_dot_3;
    ge1_1 = fMult((FIXP_DBL)DBL_n6dot5_div_8, eq_1_strength);
    ge1_1_e = 3;
    Ge1_1 = fMult((FIXP_DBL)DBL_0dot7_div_1, eq_1_strength);
    Ge1_1_e = 0;

    fe1_2 = (FIXP_DBL)1300;
    fe1_2 <<= 20;
    fe1_2_e = 11;
    qe1_2 = (FIXP_DBL)DBL_0_dot_5;
    ge1_2 = fMult((FIXP_DBL)DBL_1dot8_div_2, eq_1_strength);
    ge1_2_e = 1;
    Ge1_2 = (FIXP_DBL)0;
    Ge1_2_e = 0;

    fe1_3 = (FIXP_DBL)600;
    fe1_3 <<= 21;
    fe1_3_e = 10;
    qe1_3 = (FIXP_DBL)MAXVAL_DBL;
    ge1_3 = fMult((FIXP_DBL)DBL_2_div_4, eq_1_strength);
    ge1_3_e = 2;
    Ge1_3 = (FIXP_DBL)0;
    Ge1_3_e = 0;
  }
  if (eq_2_idx == RULE_EQ3) {
    fe2_1 = (FIXP_DBL)200;
    fe2_1 <<= 23;
    fe2_1_e = 8;
    qe2_1 = (FIXP_DBL)DBL_0_dot_3;
    ge2_1 = fMult((FIXP_DBL)DBL_n6dot5_div_8, eq_2_strength);
    ge2_1_e = 3;
    Ge2_1 = fMult((FIXP_DBL)DBL_0dot7_div_1, eq_2_strength);
    Ge2_1_e = 0;

    fe2_2 = (FIXP_DBL)1300;
    fe2_2 <<= 20;
    fe2_2_e = 11;
    qe2_2 = (FIXP_DBL)DBL_0_dot_5;
    ge2_2 = fMult((FIXP_DBL)DBL_1dot8_div_2, eq_2_strength);
    ge2_2_e = 1;
    Ge2_2 = (FIXP_DBL)0;
    Ge2_2_e = 0;

    fe2_3 = (FIXP_DBL)600;
    fe2_3 <<= 21;
    fe2_3_e = 10;
    qe2_3 = (FIXP_DBL)MAXVAL_DBL;
    ge2_3 = fMult((FIXP_DBL)DBL_2_div_4, eq_2_strength);
    ge2_3_e = 2;
    Ge2_3 = (FIXP_DBL)0;
    Ge2_3_e = 0;
  }

  if ((eq_1_idx == RULE_EQ3) || (eq_2_idx == RULE_EQ3)) {
    for (i = 0; i < nbands; i++) {
      eq[i] = (FIXP_EQ_H)MAXVAL_EQ_H;
      eq_e = 0;
      if (eq_1_idx == RULE_EQ3) {
        f = fMult(fAbs(bands_nrm[i]), tmp_sfreq_Hz);
        tmp =
            peak_filter(fe1_1, fe1_1_e, qe1_1, 0, ge1_1, ge1_1_e, Ge1_1, Ge1_1_e, f, 16 - 1, &eq_e);
        eq[i] = FX_DBL2FX_EQ_H(fMult(eq[i], tmp));
        tmp =
            peak_filter(fe1_2, fe1_2_e, qe1_2, 0, ge1_2, ge1_2_e, Ge1_2, Ge1_2_e, f, 16 - 1, &eq_e);
        eq[i] = FX_DBL2FX_EQ_H(fMult(eq[i], tmp));
        tmp =
            peak_filter(fe1_3, fe1_3_e, qe1_3, 0, ge1_3, ge1_3_e, Ge1_3, Ge1_3_e, f, 16 - 1, &eq_e);
        eq[i] = scaleValue(FX_DBL2FX_EQ_H(fMult(eq[i], tmp)), (eq_e - EQ_H_EXP));
      }
      if (eq_2_idx == RULE_EQ3) {
        f = fMult(fAbs(bands_nrm[i]), tmp_sfreq_Hz);
        tmp =
            peak_filter(fe2_1, fe2_1_e, qe2_1, 0, ge2_1, ge2_1_e, Ge2_1, Ge2_1_e, f, 16 - 1, &eq_e);
        eq[i] = FX_DBL2FX_EQ_H(fMult(eq[i], tmp));
        tmp =
            peak_filter(fe2_2, fe2_2_e, qe2_2, 0, ge2_2, ge2_2_e, Ge2_2, Ge2_2_e, f, 16 - 1, &eq_e);
        eq[i] = FX_DBL2FX_EQ_H(fMult(eq[i], tmp));
        tmp =
            peak_filter(fe2_3, fe2_3_e, qe2_3, 0, ge2_3, ge2_3_e, Ge2_3, Ge2_3_e, f, 16 - 1, &eq_e);
        eq[i] = scaleValue(FX_DBL2FX_EQ_H(fMult(eq[i], tmp)), (eq_e - EQ_H_EXP));
      }
    }
  }

  /* RULE_EQ4 */
  if (eq_1_idx == RULE_EQ4) {
    fe1_1 = (FIXP_DBL)5000;
    fe1_1 <<= 18;
    fe1_1_e = 13;
    qe1_1 = (FIXP_DBL)MAXVAL_DBL;
    ge1_1 = fMult((FIXP_DBL)DBL_4dot5_div_8, eq_1_strength);
    ge1_1_e = 3;
    Ge1_1 = fMult((FIXP_DBL)DBL_n3dot1_div_4, eq_1_strength);
    Ge1_1_e = 2;

    fe1_2 = (FIXP_DBL)1100;
    fe1_2 <<= 20;
    fe1_2_e = 11;
    qe1_2 = (FIXP_DBL)DBL_0_dot_8;
    ge1_2 = fMult((FIXP_DBL)DBL_1dot8_div_2, eq_1_strength);
    ge1_2_e = 1;
    Ge1_2 = (FIXP_DBL)0;
    Ge1_2_e = 0;
  }
  if (eq_2_idx == RULE_EQ4) {
    fe2_1 = (FIXP_DBL)5000;
    fe2_1 <<= 18;
    fe2_1_e = 13;
    qe2_1 = (FIXP_DBL)MAXVAL_DBL;
    ge2_1 = fMult((FIXP_DBL)DBL_4dot5_div_8, eq_2_strength);
    ge2_1_e = 3;
    Ge2_1 = fMult((FIXP_DBL)DBL_n3dot1_div_4, eq_2_strength);
    Ge2_1_e = 2;

    fe2_2 = (FIXP_DBL)1100;
    fe2_2 <<= 20;
    fe2_2_e = 11;
    qe2_2 = (FIXP_DBL)DBL_0_dot_8;
    ge2_2 = fMult((FIXP_DBL)DBL_1dot8_div_2, eq_2_strength);
    ge2_2_e = 1;
    Ge2_2 = (FIXP_DBL)0;
    Ge2_2_e = 0;
  }
  if ((eq_1_idx == RULE_EQ4) || (eq_2_idx == RULE_EQ4)) {
    for (i = 0; i < nbands; i++) {
      eq[i] = (FIXP_EQ_H)MAXVAL_EQ_H;
      eq_e = 0;
      if (eq_1_idx == RULE_EQ4) {
        f = fMult(fAbs(bands_nrm[i]), tmp_sfreq_Hz);
        tmp =
            peak_filter(fe1_1, fe1_1_e, qe1_1, 0, ge1_1, ge1_1_e, Ge1_1, Ge1_1_e, f, 16 - 1, &eq_e);
        eq[i] = FX_DBL2FX_EQ_H(fMult(eq[i], tmp));
        tmp =
            peak_filter(fe1_2, fe1_2_e, qe1_2, 0, ge1_2, ge1_2_e, Ge1_2, Ge1_2_e, f, 16 - 1, &eq_e);
        eq[i] = scaleValue(FX_DBL2FX_EQ_H(fMult(eq[i], tmp)), (eq_e - EQ_H_EXP));
      }
      if (eq_2_idx == RULE_EQ4) {
        f = fMult(fAbs(bands_nrm[i]), tmp_sfreq_Hz);
        tmp =
            peak_filter(fe2_1, fe2_1_e, qe2_1, 0, ge2_1, ge2_1_e, Ge2_1, Ge2_1_e, f, 16 - 1, &eq_e);
        eq[i] = FX_DBL2FX_EQ_H(fMult(eq[i], tmp));
        tmp =
            peak_filter(fe2_2, fe2_2_e, qe2_2, 0, ge2_2, ge2_2_e, Ge2_2, Ge2_2_e, f, 16 - 1, &eq_e);
        eq[i] = scaleValue(FX_DBL2FX_EQ_H(fMult(eq[i], tmp)), (eq_e - EQ_H_EXP));
      }
    }
  }

  const FIXP_DBL* EQ;
  INT EQ_flag = 0;
  if (nbands == STFT_ERB_BANDS) {
    switch (eq_1_idx) {
      case RULE_EQVF:
        EQ = EQVF_StftErb;
        EQ_flag = RULE_EQVF;
        break;
      case RULE_EQVB:
        EQ = EQVB_StftErb;
        EQ_flag = RULE_EQVB;
        break;
      case RULE_EQVFC:
        EQ = EQVFC_StftErb;
        EQ_flag = RULE_EQVFC;
        break;
      case RULE_EQVBC:
        EQ = EQVBC_StftErb;
        EQ_flag = RULE_EQVBC;
        break;
      case RULE_EQVOG:
        EQ = EQVOG_StftErb;
        EQ_flag = RULE_EQVOG;
        break;
      case RULE_EQVS:
        EQ = EQVS_StftErb;
        EQ_flag = RULE_EQVS;
        break;
      case RULE_EQBTM:
        EQ = EQBTM_StftErb;
        EQ_flag = RULE_EQBTM;
        break;
      case RULE_EQVBA:
        EQ = EQVBA_StftErb;
        EQ_flag = RULE_EQVBA;
        break;
      case RULE_06_03:
        EQ = COLOR_060_030_StftErb;
        EQ_flag = RULE_06_03;
        break;
      case RULE_09_03:
        EQ = COLOR_090_030_StftErb;
        EQ_flag = RULE_09_03;
        break;
      case RULE_06_11:
        EQ = COLOR_060_110_StftErb;
        EQ_flag = RULE_06_11;
        break;
      case RULE_09_11:
        EQ = COLOR_090_110_StftErb;
        EQ_flag = RULE_09_11;
        break;
      case RULE_13_11:
        EQ = COLOR_135_110_StftErb;
        EQ_flag = RULE_13_11;
        break;
      case RULE_18_11:
        EQ = COLOR_180_110_StftErb;
        EQ_flag = RULE_18_11;
        break;
      default:
        break;
    }
  }

  if (EQ_flag != 0) {
    for (i = 0; i < nbands; i++) {
      eq[i] = scaleValue(FX_DBL2FX_EQ_H(EQ[i]), (1 - EQ_H_EXP));
    }
  }
}

/* API functions
 * ************************************************************************** */

/**
 * Define generic input/output formats.
 */

void converter_set_inout_format(
    IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt,
    const INT inout,        /* in:  0: set input format, 1: set output format */
    const INT num_channels, /* in:  number of channels in setup to define */
    const AFC_FORMAT_CONVERTER_CHANNEL_ID* channel_vector /* in:  vector containing channel enums */
) {
  if (inout == 0) {
    for (INT l = 0; l < num_channels; l++) fcInt->format_in_listOfChannels[l] = channel_vector[l];
    fcInt->format_in_listOfChannels_nchan = num_channels;
  } else {
    FDK_ASSERT(inout == 1);
    for (INT l = 0; l < num_channels; l++) fcInt->format_out_listOfChannels[l] = channel_vector[l];
    fcInt->format_out_listOfChannels_nchan = num_channels;
  }
}

/**
 * Initialize audio processing.
 *
 * No memory is allocated, nothing to release.
 */
typedef INT dmx_rules_t[MAX_NUM_DMX_RULES][8];
converter_status_t converter_init(
    IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt,
    converter_pr_t* params,                       /* out: initialized parameters */
    const FDK_converter_formatid_t input_format,  /* in:  input format id */
    const FDK_converter_formatid_t output_format, /* in:  output format id */
    const INT* randomization, /* in:  randomization angles [azi,ele,azi,ele, ... in degrees] */
    const INT sfreq_Hz,       /* in:  sampling frequency [Hz] */
    const INT blocklength,    /* in:  processing block length [samples] */
    const UINT nbands,        /* in:  number of filterbank processing bands */
    const FIXP_DBL*
        bands_nrm, /* in:  filterbank processing bands center frequencies [normalized frequency] */
    INT* p_buffer  /* in:  temporary buffer */
) {
  /* Random setups are not supported. */
  FDK_ASSERT(randomization == 0);

  INT i, nchanin, nchanout, ch, ptr;  // not in use: in_out_n

  INT* in_out_src = p_buffer;
  p_buffer += IN_OUT_N;

  INT* in_out_dst = p_buffer;
  p_buffer += IN_OUT_N;

  FIXP_DMX_H* in_out_gain = (FIXP_DMX_H*)p_buffer;
  p_buffer = p_buffer + (IN_OUT_N * sizeof(FIXP_DMX_H)) / sizeof(INT);

  INT* in_out_proc = p_buffer;
  p_buffer += IN_OUT_N;

  /* Immersive variables */
  FIXP_DMX_H* in_out_gainL = (FIXP_DMX_H*)p_buffer;
  p_buffer = p_buffer + (IN_OUT_N * sizeof(FIXP_DMX_H)) / sizeof(INT);

  INT* in_out_src2 = p_buffer;
  p_buffer += IN_OUT_N;

  INT* in_out_dst2 = p_buffer;
  p_buffer += IN_OUT_N;

  FIXP_DMX_H* in_out_gain2 = (FIXP_DMX_H*)p_buffer;
  p_buffer = p_buffer + (IN_OUT_N * sizeof(FIXP_DMX_H)) / sizeof(INT);

  FIXP_DMX_H* in_out_gaini = (FIXP_DMX_H*)p_buffer;
  p_buffer = p_buffer + (IN_OUT_N * sizeof(FIXP_DMX_H)) / sizeof(INT);

  INT* in_out_proc2 = p_buffer;
  p_buffer += IN_OUT_N;

  const AFC_FORMAT_CONVERTER_CHANNEL_ID *format_in, *format_out;

  INT azi_ele[CH_LFE2 + 1][2];
  dmx_rules_t* p_dmx_rules = (dmx_rules_t*)p_buffer;
  p_buffer += MAX_NUM_DMX_RULES * 8;
#define dmx_rules (*p_dmx_rules)

  FDK_ASSERT(nbands <= MAXBANDS);

  /* init in_out vectors */
  for (i = 0; i < IN_OUT_N; i++) {
    in_out_src[i] = 0;
    in_out_dst[i] = 0;
    in_out_gain[i] = (FIXP_DBL)0;
    in_out_proc[i] = 0;
    in_out_gainL[i] = (FIXP_DMX_H)-1;

    in_out_src2[i] = 0;
    in_out_dst2[i] = 0;
    in_out_gain2[i] = (FIXP_DMX_H)0;
    in_out_proc2[i] = 0;
    in_out_gaini[i] = (FIXP_DMX_H)MAXVAL_DMX_H;
  }

  /* parameter range checking */

  if (input_format >= FDK_NFORMATS - 1) {
    return FORMAT_CONVERTER_STATUS_INFORMAT;
  }

  if ((output_format >= FDK_NFORMATS) || (output_format == FDK_NFORMATS - 2)) {
    return FORMAT_CONVERTER_STATUS_OUTFORMAT;
  }

  if ((sfreq_Hz < SFREQ_HZ_MIN) || (sfreq_Hz > SFREQ_HZ_MAX)) {
    return FORMAT_CONVERTER_STATUS_SFREQ;
  }

  if ((nbands < NBANDS_MIN) || (nbands > NBANDS_MAX)) {
    if (nbands == 0) {
    } else {
      return FORMAT_CONVERTER_STATUS_BANDS;
    }
  }

  for (i = 0; i < (INT)nbands; i++) {
    if (bands_nrm[i] < (FIXP_DBL)0)  // if ((bands_nrm[i] > 1.0f) || (bands_nrm[i] < 0.0f))
      return FORMAT_CONVERTER_STATUS_BANDS;
  }

  /* get input and output format info */
  if (input_format == FDK_FORMAT_IN_LISTOFCHANNELS) {
    nchanin = fcInt->format_in_listOfChannels_nchan;
    format_in = fcInt->format_in_listOfChannels;
    FDK_ASSERT(format_in != NULL);
  } else {
    return FORMAT_CONVERTER_STATUS_INFORMAT;
  }

  if (output_format == FDK_FORMAT_OUT_LISTOFCHANNELS) {
    nchanout = fcInt->format_out_listOfChannels_nchan;
    format_out = fcInt->format_out_listOfChannels;
    FDK_ASSERT(format_out != NULL);
  } else {
    return FORMAT_CONVERTER_STATUS_OUTFORMAT;
  }

  params->nchanin = nchanin;
  params->nchanout = nchanout;

  /* copy channel angles (they may be modified for random setups) */

  for (i = 0; i < (CH_LFE2 + 1); i++) {
    azi_ele[i][0] = ch_azi_ele[i][0];
    azi_ele[i][1] = ch_azi_ele[i][1];
  }

  if ((fcInt->immersiveDownmixFlag == 1) && ((nchanout == 5) || (nchanout == 6))) {
    /* 11.4.1.6.7.2  iar_initElevSptlParms : Initialization of elevation rendering parameters based
     * on the input channel elevation */
    fcInt->elv = FL2FXCONST_DBL(0.136718750000000); /* default elevation of the height channels */
    initElevSptlParms(fcInt, fcInt->elv, nbands, sfreq_Hz, randomization, nchanout);
    FDKmemcpy(dmx_rules, dmx_rules_immersive, sizeof(dmx_rules_immersive));
  } else {
    FDKmemcpy(dmx_rules, dmx_rules_classic, sizeof(dmx_rules_classic));
  }

  /* more parameter error checking */

  /* Define format conversion. Downmix matrices genration. */

  ch = 0;
  for (i = 0; i < nchanin; i++) {
    INT input_channel_id, output_channel_idx;

    /* map EMPTY channel as silent channel, i.e. generate a dmx mtx line containing zeros */
    input_channel_id = format_in[i];
    if (input_channel_id == CH_EMPTY) {
      in_out_src[ch] = i; /* input channel index */
      in_out_dst[ch] = 0; /* use first output channel. unimportant since gain is set to 0.0 */
      in_out_gain[ch] = (FIXP_DMX_H)0; /* gain: 0 gain, silence */
      in_out_proc[ch] = 0;             /* processing: 0 = no EQ */
      ch++;
      continue;
    }

    /* test if input channel exists in output format */
    output_channel_idx = find_channel(input_channel_id, format_out, nchanout);

    /* input channel exists in output format
       (just copy channel from input to output) */

    if (output_channel_idx >= 0) {
      in_out_src[ch] = i;                         /* input channel index */
      in_out_dst[ch] = output_channel_idx;        /* output channel index */
      in_out_gain[ch] = (FIXP_DMX_H)MAXVAL_DMX_H; /* gain */
      in_out_proc[ch] = 0;                        /* processing */
      ch++;

      /* input channel does not exist in output format
         (apply downmix rules) */

    } else {
      UINT k, ch_start;

      /* search best downmix rule */
      ch_start = ch;
      k = 0;
      while (dmx_rules[k][0] != -1) {
        INT ch_count, uch;
        /* when the downmix rule is RULE_VIRTUAL : perform the "spatial elevation rendering" */
        if (dmx_rules[k][0] == input_channel_id && dmx_rules[k][4] == RULE_VIRTUAL) {
          INT mch;
          if (nchanout == 6) {
            for (mch = 0; mch < 6; mch++) {
              if (fcInt->GVH[dmx_rules[k][1]][mch] == (FIXP_DBL)0) {
                continue;
              }

              if (ch >= IN_OUT_N) return FORMAT_CONVERTER_STATUS_MISSING_RULE;
              in_out_src[ch] = i;
              in_out_dst[ch] = mch;
              in_out_gain[ch] = scaleValue(FX_DBL2FX_DMX_H(fMult(fcInt->GVH[dmx_rules[k][1]][mch],
                                                                 ((FIXP_DBL)dmx_rules[k][3]))),
                                           fcInt->GVH_e[dmx_rules[k][1]][mch]);
              in_out_gainL[ch] = scaleValue(FX_DBL2FX_DMX_H(fMult(fcInt->GVL[dmx_rules[k][1]][mch],
                                                                  ((FIXP_DBL)dmx_rules[k][3]))),
                                            fcInt->GVL_e[dmx_rules[k][1]][mch]);

              { in_out_proc[ch] = dmx_rules[k][7]; }

              fcInt->topIn[dmx_rules[k][1]] = i;

              ch++;
            }
          } else if (nchanout == 5) {
            for (mch = 0; mch < 6; mch++) {
              INT mch5;

              if (fcInt->GVH[dmx_rules[k][1]][mch] == (FIXP_DBL)0) {
                continue;
              }

              mch5 = mch > 3 ? mch - 1 : mch;

              if (ch >= IN_OUT_N) return FORMAT_CONVERTER_STATUS_MISSING_RULE;
              in_out_src[ch] = i;
              in_out_dst[ch] = mch5;
              in_out_gain[ch] = scaleValue(FX_DBL2FX_DMX_H(fMult(fcInt->GVH[dmx_rules[k][1]][mch],
                                                                 ((FIXP_DBL)dmx_rules[k][3]))),
                                           fcInt->GVH_e[dmx_rules[k][1]][mch]);
              in_out_gainL[ch] = scaleValue(FX_DBL2FX_DMX_H(fMult(fcInt->GVL[dmx_rules[k][1]][mch],
                                                                  ((FIXP_DBL)dmx_rules[k][3]))),
                                            fcInt->GVL_e[dmx_rules[k][1]][mch]);
              { in_out_proc[ch] = dmx_rules[k][7]; }

              fcInt->topIn[dmx_rules[k][1]] = i;

              ch++;
            }
          }

          break;
        }
        /* find rule with input == input_channel_id */
        if (dmx_rules[k][0] == input_channel_id) {
          INT idx, idx2;

          /* top loudspeaker to all height loudspeakers rule */
          if (dmx_rules[k][4] == RULE_TOP2ALLU) {
            /* count number of height channels */
            ch_count = 0;
            for (uch = CH_U_000; uch <= CH_U_180; uch++) {
              if (find_channel(uch, format_out, nchanout) != -1) ch_count++;
            }
            /* if there are height channels, top is mixed to these */
            if (ch_count > 0) {
              FIXP_DMX_H gain_m;
              INT gain_e = 0, ch_count_e = 4;
              gain_m = FX_DBL2FX_DMX_H(
                  invSqrtNorm2((FIXP_DBL)(ch_count << (DFRACT_BITS - (ch_count_e + 1))), &gain_e));
              gain_e = gain_e - (ch_count_e >> 1);

              for (uch = CH_U_000; uch <= CH_U_180; uch++) {
                if (ch >= IN_OUT_N) return FORMAT_CONVERTER_STATUS_MISSING_RULE;
                in_out_src[ch] = i;                                       /* input channel index */
                in_out_dst[ch] = find_channel(uch, format_out, nchanout); /* output channel index */
                in_out_gain[ch] =
                    FX_DBL2FX_DMX_H(scaleValue(fMult((FIXP_DBL)dmx_rules[k][3], gain_m), gain_e));
                in_out_proc[ch] = dmx_rules[k][7]; /* processing */
                if (in_out_dst[ch] != -1) ch++;
              }
              break;
            }
            k++;
            continue;
          }
          /* top loudspeaker to all horizontal loudspeakers rule */
          if (dmx_rules[k][4] == RULE_TOP2ALLM) {
            /* count number of horizontal channels */
            ch_count = 0;
            for (uch = CH_M_000; uch <= CH_M_180; uch++) {
              if (find_channel(uch, format_out, nchanout) != -1) ch_count++;
            }
            /* if there are horizontal channels, top is mixed to these */
            if (ch_count > 0) {
              FIXP_DMX_H gain_m;
              INT gain_e = 0, ch_count_e = 4;
              gain_m = FX_DBL2FX_DMX_H(
                  invSqrtNorm2((FIXP_DBL)(ch_count << (DFRACT_BITS - (ch_count_e + 1))), &gain_e));
              gain_e = gain_e - (ch_count_e >> 1);

              for (uch = CH_M_000; uch <= CH_M_180; uch++) {
                if (ch >= IN_OUT_N) return FORMAT_CONVERTER_STATUS_MISSING_RULE;
                in_out_src[ch] = i;                                       /* input channel index */
                in_out_dst[ch] = find_channel(uch, format_out, nchanout); /* output channel index */
                in_out_gain[ch] =
                    FX_DBL2FX_DMX_H(scaleValue(fMult((FIXP_DBL)dmx_rules[k][3], gain_m), gain_e));
                in_out_proc[ch] = dmx_rules[k][7]; /* processing */
                if (in_out_dst[ch] != -1) ch++;
              }
              break;
            }
            k++;
            continue;
          }
          /* do target channels of rule exist? */
          idx = find_channel(dmx_rules[k][1], format_out, nchanout);
          idx2 = idx;
          if (dmx_rules[k][2] != -1) {
            /* rule with two output channels */
            idx2 = find_channel(dmx_rules[k][2], format_out, nchanout);
          }
          /* do all rule output channels exist? */
          if ((idx >= 0) && (idx2 >= 0)) {
            if ((dmx_rules[k][4] == RULE_PANNING) || (dmx_rules[k][4] == RULE_AUTOPAN)) {
              FIXP_DBL azi_1, azi_2, azi_target, mn, alpha, alpha0, center;
              /* add offest such that all angles are positive */
              azi_1 = (FIXP_DBL)azi_ele[format_out[idx]][0];
              azi_2 = (FIXP_DBL)azi_ele[format_out[idx2]][0];
              azi_target = (FIXP_DBL)azi_ele[input_channel_id][0];
              mn = fMin(fMin(azi_1, azi_2), azi_target);
              azi_1 = (FIXP_DBL)((INT)azi_1 - (INT)mn);
              azi_2 = azi_2 - mn;
              azi_target = (FIXP_DBL)((INT)azi_target - (INT)mn);
              /* manual and auto panning */
              if (dmx_rules[k][4] == RULE_PANNING) {
                /* manual definition of parameters */
                alpha0 = (FIXP_DBL)dmx_rules[k][5]; /* angle of system +-alpha0 */
                alpha = (FIXP_DBL)dmx_rules[k][6];  /* definition: negative is left */
              } else /* if (dmx_rules[k][4] == RULE_AUTOPAN) */ {
                /* amplitude panning angle alpha0 */
                alpha0 = fAbs(azi_1 - azi_2) >> 1;
                /* center angle */
                center = (azi_1 >> 1) + (azi_2 >> 1);
                /* amplitude panning angle alpha */
                alpha = (FIXP_DBL)((INT)center - (INT)azi_target);
                if (azi_1 > azi_2) {
                  alpha = -alpha;
                }
              }

              alpha0 = fMult(alpha0, FX_PI);
              INT alpha0_e = 3;
              alpha = fMult(alpha, FX_PI);
              INT alpha_e = 3;

              FIXP_DBL sin_alpha0_m = fixp_sin(alpha0, alpha0_e);
              FIXP_DBL cos_alpha0_m = fixp_cos(alpha0, alpha0_e);

              FIXP_DBL sin_alpha_m = fixp_sin(alpha, alpha_e);
              FIXP_DBL cos_alpha_m = fixp_cos(alpha, alpha_e);

              FIXP_DBL tan_alpha0_m = fDivNormSigned(sin_alpha0_m, cos_alpha0_m, &alpha0_e);
              FIXP_DBL tan_alpha_m = fDivNormSigned(sin_alpha_m, cos_alpha_m, &alpha_e);

              FIXP_DBL num_m, denom_m;
              INT num_e, denom_e;

              num_m = fAddNorm(tan_alpha0_m, alpha0_e, tan_alpha_m, alpha_e, &num_e);
              num_m = fAddNorm(num_m, num_e, ((FIXP_DBL)0x00000001), (-3), &num_e);
              denom_m = fAddNorm(tan_alpha0_m, alpha0_e, -tan_alpha_m, alpha_e, &denom_e);
              denom_m = fAddNorm(denom_m, denom_e, ((FIXP_DBL)0x00000001), (-3), &denom_e);

              INT a1_e = 0;
              FIXP_DBL a1_m = fDivNormSigned(num_m, denom_m, &a1_e);
              a1_e = a1_e + num_e - denom_e;

              INT nrm_e = 0, tmp_e = 0;
              FIXP_DBL nrm_m, tmp_m;

              tmp_m = fMult(a1_m, a1_m);
              tmp_e = a1_e << 1;
              tmp_m = fAddNorm(tmp_m, tmp_e, FL2FXCONST_DBL(0.5f), 1, &tmp_e);
              if (tmp_e & 0x1) {
                tmp_m >>= 1;
                tmp_e += 1;
              }

              nrm_m = invSqrtNorm2(tmp_m, &nrm_e);
              nrm_e = nrm_e - (tmp_e >> 1);

              a1_m = fMult(a1_m, nrm_m);
              a1_e = a1_e + nrm_e;

              /* panning */
              if (ch >= IN_OUT_N) return FORMAT_CONVERTER_STATUS_MISSING_RULE;
              in_out_src[ch] = i;   /* input channel index */
              in_out_dst[ch] = idx; /* output channel index */
              in_out_gain[ch] =
                  FX_DBL2FX_DMX_H(scaleValue(fMult((FIXP_DBL)dmx_rules[k][3], a1_m), a1_e));
              in_out_proc[ch] = dmx_rules[k][7]; /* processing */
              ch = ch + 1;
              in_out_src[ch] = i;    /* input channel index */
              in_out_dst[ch] = idx2; /* output channel index */
              in_out_gain[ch] =
                  FX_DBL2FX_DMX_H(scaleValue(fMult((FIXP_DBL)dmx_rules[k][3], nrm_m), nrm_e));
              in_out_proc[ch] = dmx_rules[k][7]; /* processing */
              ch = ch + 1;
            } else {
              /* only external processing option */
              if (ch >= IN_OUT_N) return FORMAT_CONVERTER_STATUS_MISSING_RULE;
              in_out_src[ch] = i;   /* input channel index */
              in_out_dst[ch] = idx; /* output channel index */
              in_out_gain[ch] = FX_DBL2FX_DMX_H((FIXP_DBL)dmx_rules[k][3]); /* gain */
              in_out_proc[ch] = dmx_rules[k][7];                            /* processing */
              ch = ch + 1;
              /* phantom source (and thus 2nd destination channel)? */
              if (dmx_rules[k][2] != -1) {
                idx = find_channel(dmx_rules[k][2], format_out, nchanout);
                if (idx != -1) {
                  in_out_src[ch] = i;   /* input channel index */
                  in_out_dst[ch] = idx; /* output channel index */
                  in_out_gain[ch] = FX_DBL2FX_DMX_H((FIXP_DBL)dmx_rules[k][3]); /* gain */
                  in_out_proc[ch] = dmx_rules[k][7];                            /* processing */
                  ch = ch + 1;
                }
              }
            }
            /* done (break loop over k) */
            break;
          }
        }
        k++;
      } /* = loop over k end */
      if (ch_start == (UINT)ch) {
        /* downmix rule missing */
        return FORMAT_CONVERTER_STATUS_MISSING_RULE;
      }
    }
  }
  if (ch >= IN_OUT_N) return FORMAT_CONVERTER_STATUS_MISSING_RULE;
  in_out_src[ch] = -1;
  in_out_dst[ch] = -1;
  in_out_gain[ch] = (FIXP_DMX_H)-1;
  in_out_proc[ch] = -1;
  in_out_gainL[ch] = (FIXP_DMX_H)-1;

  /* re-order in_out such that ordering is by destination channel */
  ptr = 0;
  for (i = 0; i < nchanout; i++) {
    ch = 0;
    while (in_out_src[ch] >= 0) {
      if (i == in_out_dst[ch]) {
        params->in_out_src[ptr] = in_out_src[ch];
        params->in_out_dst[ptr] = in_out_dst[ch];
        params->in_out_gain[ptr] = in_out_gain[ch];
        params->in_out_proc[ptr] = in_out_proc[ch];
        params->in_out_gainL[ptr] = in_out_gainL[ch];
        ptr++;
      }
      ch++;
    }
  }
  FDK_ASSERT(ch == ptr);
  params->in_out_src[ch] = -1;
  params->in_out_dst[ch] = -1;
  params->in_out_gain[ch] = (FIXP_DMX_H)-1;  //-1
  params->in_out_proc[ch] = -1;

  if (fcInt->immersiveDownmixFlag == 1 &&
      ((fcInt->numOutputChannels == 6) || (fcInt->numOutputChannels == 5))) {
    params->in_out_gainL[ch] = (FIXP_DMX_H)-1;

    ch = 0;
    for (i = 0; i < nchanin; i++) {
      int input_channel_id, output_channel_idx;

      /* */

      input_channel_id = format_in[i];
      if (input_channel_id == CH_EMPTY) continue;

      /* test if input channel exists in output format */

      output_channel_idx = find_channel(input_channel_id, format_out, nchanout);

      /* input channel exists in output format
         (just copy channel from input to output) */

      if (output_channel_idx >= 0) {
        in_out_src2[ch] = i;                         /* input channel index */
        in_out_dst2[ch] = output_channel_idx;        /* output channel index */
        in_out_gain2[ch] = (FIXP_DMX_H)MAXVAL_DMX_H; /* gain */
        in_out_proc2[ch] = 0;                        /* processing */
        in_out_gaini[ch] = (FIXP_DMX_H)MAXVAL_DMX_H;
        ch++;

        /* input channel does not exist in output format
           (apply downmix rules) */

      } else {
        UINT k, ch_start;

        /* search best downmix rule */
        ch_start = ch;
        k = 0;
        while (dmx_rules[k][0] != -1) {
          INT ch_count, uch = 0;
          if (dmx_rules[k][4] == RULE_VIRTUAL) {
            k++;
            continue;
          }
          if (dmx_rules[k][0] == input_channel_id) {
            INT idx, idx2;

            /* top loudspeaker to all height loudspeakers rule */
            if (dmx_rules[k][4] == RULE_TOP2ALLU) {
              /* count number of height channels */
              ch_count = 0;
              for (uch = CH_U_000; uch <= CH_U_180; uch++) {
                if (find_channel(uch, format_out, nchanout) != -1) ch_count++;
              }
              /* if there are height channels, top is mixed to these */
              if (ch_count > 0) {
                /*
                  float gain;
                  gain = 1.0f / (float)sqrt(ch_count);

                  */

                FIXP_DMX_H gain_m;
                INT gain_e = 0, ch_count_e = 4;
                gain_m = FX_DBL2FX_DMX_H(invSqrtNorm2(
                    (FIXP_DBL)(ch_count << (DFRACT_BITS - (ch_count_e + 1))), &gain_e));
                gain_e = gain_e - (ch_count_e >> 1);

                for (uch = CH_U_000; uch <= CH_U_180; uch++) {
                  in_out_src2[ch] = i; /* input channel index */
                  in_out_dst2[ch] =
                      find_channel(uch, format_out, nchanout); /* output channel index */
                  in_out_gain2[ch] = FX_DBL2FX_DMX_H(
                      scaleValue(fMult((FIXP_DBL)dmx_rules[k][3], gain_m), gain_e)); /* gain */
                  in_out_proc2[ch] = dmx_rules[k][7]; /* processing */
                  in_out_gaini[ch] = FX_DBL2FX_DMX_H((FIXP_DBL)dmx_rules[k][3]);
                  if (in_out_dst2[ch] != -1) ch++;
                }
                break;
              }
              k++;
              continue;
            }
            /* top loudspeaker to all horizontal loudspeakers rule */
            if (dmx_rules[k][4] == RULE_TOP2ALLM) {
              /* count number of height channels */
              ch_count = 0;
              for (uch = CH_M_000; uch <= CH_M_180; uch++) {
                if (find_channel(uch, format_out, nchanout) != -1) ch_count++;
              }
              /* if there are height channels, top is mixed to these */
              if (ch_count > 0) {
                FIXP_DMX_H gain_m;
                INT gain_e = 0, ch_count_e = 4;
                gain_m = FX_DBL2FX_DMX_H(invSqrtNorm2(
                    (FIXP_DBL)(ch_count << (DFRACT_BITS - (ch_count_e + 1))), &gain_e));
                gain_e = gain_e - (ch_count_e >> 1);

                /* gain = 1.0f / (float)sqrt(ch_count); */

                for (uch = CH_M_000; uch <= CH_M_180; uch++) {
                  in_out_src2[ch] = i; /* input channel index */
                  in_out_dst2[ch] =
                      find_channel(uch, format_out, nchanout); /* output channel index */
                  in_out_gain2[ch] = FX_DBL2FX_DMX_H(
                      scaleValue(fMult((FIXP_DBL)dmx_rules[k][3], gain_m), gain_e)); /* gain */
                  in_out_proc2[ch] = dmx_rules[k][7]; /* processing */
                  in_out_gaini[ch] = FX_DBL2FX_DMX_H((FIXP_DBL)dmx_rules[k][3]);
                  if (in_out_dst2[ch] != -1) ch++;
                }
                break;
              }
              k++;
              continue;
            }
            /* do target channels of rule exist? */
            idx = find_channel(dmx_rules[k][1], format_out, nchanout);
            idx2 = idx;
            if (dmx_rules[k][2] != -1) {
              /* rule with two output channels */
              idx2 = find_channel(dmx_rules[k][2], format_out, nchanout);
            }
            /* do all rule output channels exist? */
            if ((idx >= 0) && (idx2 >= 0)) {
              if ((dmx_rules[k][4] == RULE_PANNING) || (dmx_rules[k][4] == RULE_AUTOPAN)) {
                /*
                float azi_1, azi_2, azi_target, mn, alpha, alpha0, center;
                float a1, a2, nrm;
                */

                FIXP_DBL azi_1, azi_2, azi_target, mn, alpha = 0, alpha0 = 0, center;

                /* add offest such that all angles are positive */
                azi_1 = (FIXP_DBL)azi_ele[format_out[idx]][0];
                azi_2 = (FIXP_DBL)azi_ele[format_out[idx2]][0];
                azi_target = (FIXP_DBL)azi_ele[input_channel_id][0];
                mn = fMax(fMin(azi_1, azi_2), azi_target);
                azi_1 = (FIXP_DBL)((INT)azi_1 - (INT)mn);
                azi_2 = azi_2 - mn;
                azi_target = (FIXP_DBL)((INT)azi_target - (INT)mn);
                /* manual and auto panning */
                if (dmx_rules[k][4] == RULE_PANNING) {
                  /* manual definition of parameters */
                  alpha0 = (FIXP_DBL)dmx_rules[k][5]; /* angle of system +-alpha0 */
                  alpha = (FIXP_DBL)dmx_rules[k][6];  /* definition: negative is left */
                } else if (dmx_rules[k][4] == RULE_AUTOPAN) {
                  /* amplitude panning angle alpha0 */
                  alpha0 = fAbs(azi_1 - azi_2) >> 1;
                  /* center angle */
                  center = (azi_1 >> 1) + (azi_2 >> 1);
                  /* amplitude panning angle alpha */
                  alpha = (FIXP_DBL)((INT)center - (INT)azi_target);
                  if (azi_1 > azi_2) {
                    alpha = -alpha;
                  }
                }
                /*
                alpha0   = alpha0 * (float)M_PI / 180.0f;
                alpha    = alpha * (float)M_PI / 180.0f;
                a2       = 1.0f;
                a1       =
                a2*((float)tan(alpha0)+(float)tan(alpha)+1e-10f)/((float)tan(alpha0)-(float)tan(alpha)+1e-10f);
                nrm      = 1.0f / (float)sqrt(a1*a1+a2*a2);
                if  ( nrm > 1 )
                {
                  int a;
                  a=1;
                }*/

                alpha0 = fMult(alpha0, FX_PI); /* exp PI 2 and exp alpha 1 */
                INT alpha0_e = 3;
                alpha = fMult(alpha, FX_PI);
                INT alpha_e = 3;

                FIXP_DBL sin_alpha0_m = fixp_sin(alpha0, alpha0_e);
                FIXP_DBL cos_alpha0_m = fixp_cos(alpha0, alpha0_e);

                FIXP_DBL sin_alpha_m = fixp_sin(alpha, alpha_e);
                FIXP_DBL cos_alpha_m = fixp_cos(alpha, alpha_e);

                FIXP_DBL tan_alpha0_m = fDivNormSigned(sin_alpha0_m, cos_alpha0_m, &alpha0_e);
                FIXP_DBL tan_alpha_m = fDivNormSigned(sin_alpha_m, cos_alpha_m, &alpha_e);

                FIXP_DBL num_m, denom_m;
                INT num_e, denom_e;

                num_m = fAddNorm(tan_alpha0_m, alpha0_e, tan_alpha_m, alpha_e, &num_e);
                num_m = fAddNorm(num_m, num_e, ((FIXP_DBL)0x00000001), (-3), &num_e);
                denom_m = fAddNorm(tan_alpha0_m, alpha0_e, -tan_alpha_m, alpha_e, &denom_e);
                denom_m = fAddNorm(denom_m, denom_e, ((FIXP_DBL)0x00000001), (-3), &denom_e);

                INT a1_e = 0;
                FIXP_DBL a1_m = fDivNormSigned(num_m, denom_m, &a1_e);
                a1_e = a1_e + num_e - denom_e;

                INT nrm_e = 0, tmp_e = 0;
                FIXP_DBL nrm_m, tmp_m;

                tmp_m = fMult(a1_m, a1_m);
                tmp_e = a1_e << 1;
                tmp_m = fAddNorm(tmp_m, tmp_e, FL2FXCONST_DBL(0.5f), 1, &tmp_e);
                if (tmp_e & 0x1) {
                  tmp_m >>= 1;
                  tmp_e += 1;
                }

                nrm_m = invSqrtNorm2(tmp_m, &nrm_e);
                nrm_e = nrm_e - (tmp_e >> 1);

                a1_m = fMult(a1_m, nrm_m);
                a1_e = a1_e + nrm_e;

                /*
                a1       = a1 * nrm;
                a2       = a2 * nrm;
                */

                /* panning */
                in_out_src2[ch] = i;   /* input channel index */
                in_out_dst2[ch] = idx; /* output channel index */
                in_out_gain2[ch] = FX_DBL2FX_DMX_H(
                    scaleValue(fMult((FIXP_DBL)dmx_rules[k][3], a1_m), a1_e)); /* gain */
                in_out_proc2[ch] = dmx_rules[k][7];                            /* processing */
                in_out_gaini[ch] = FX_DBL2FX_DMX_H((FIXP_DBL)dmx_rules[k][3]);
                ch = ch + 1;
                in_out_src2[ch] = i;    /* input channel index */
                in_out_dst2[ch] = idx2; /* output channel index */
                in_out_gain2[ch] = FX_DBL2FX_DMX_H(
                    scaleValue(fMult((FIXP_DBL)dmx_rules[k][3], nrm_m), nrm_e)); /* gain */
                in_out_proc2[ch] = dmx_rules[k][7];                              /* processing */
                in_out_gaini[ch] = FX_DBL2FX_DMX_H((FIXP_DBL)dmx_rules[k][3]);
                ch = ch + 1;
              } else {
                /* only external processing option */
                in_out_src2[ch] = i;   /* input channel index */
                in_out_dst2[ch] = idx; /* output channel index */
                in_out_gain2[ch] = FX_DBL2FX_DMX_H((FIXP_DBL)dmx_rules[k][3]); /* gain */
                in_out_proc2[ch] = dmx_rules[k][7];                            /* processing */
                in_out_gaini[ch] = FX_DBL2FX_DMX_H((FIXP_DBL)dmx_rules[k][3]);
                ch = ch + 1;
                /* phantom source (and thus 2nd destination channel)? */
                if (dmx_rules[k][2] != -1) {
                  idx = find_channel(dmx_rules[k][2], format_out, nchanout);
                  if (idx != -1) {
                    in_out_src2[ch] = i;   /* input channel index */
                    in_out_dst2[ch] = idx; /* output channel index */
                    in_out_gain2[ch] = FX_DBL2FX_DMX_H((FIXP_DBL)dmx_rules[k][3]); /* gain */
                    in_out_proc2[ch] = dmx_rules[k][7];                            /* processing */
                    in_out_gaini[ch] = FX_DBL2FX_DMX_H((FIXP_DBL)dmx_rules[k][3]);
                    ch = ch + 1;
                  }
                }
              }
              /* done (break loop over k) */
              break;
            }
          }
          k++;
        } /* = loop over k end */
        if (ch_start == (UINT)ch) {
          /* downmix rule missing */
          return FORMAT_CONVERTER_STATUS_MISSING_RULE;
        }
      }
    }
    in_out_src2[ch] = -1;
    in_out_dst2[ch] = -1;
    in_out_gain2[ch] = (FIXP_DMX_H)-1;
    in_out_gaini[ch] = (FIXP_DMX_H)-1;
    in_out_proc2[ch] = -1;

    /* re-order in_out such that ordering is by destination channel */
    ptr = 0;
    for (i = 0; i < nchanout; i++) {
      ch = 0;
      while (in_out_src2[ch] >= 0) {
        if (i == in_out_dst2[ch]) {
          params->in_out_src2[ptr] = in_out_src2[ch];
          params->in_out_dst2[ptr] = in_out_dst2[ch];
          params->in_out_gain2[ptr] = in_out_gain2[ch];
          params->in_out_gaini[ptr] = in_out_gaini[ch];
          if (in_out_proc2[ch] == RULE_EQBTM)
            params->in_out_proc2[ptr] = RULE_NOPROC;
          else
            params->in_out_proc2[ptr] = in_out_proc2[ch];
          ptr++;
        }
        ch++;
      }
    }
    if (ch != ptr) {
      return FORMAT_CONVERTER_STATUS_FAILED;
    }
    params->in_out_src2[ch] = -1;
    params->in_out_dst2[ch] = -1;
    params->in_out_gain2[ch] = (FIXP_DMX_H)-1;
    params->in_out_gaini[ch] = (FIXP_DMX_H)-1;
    params->in_out_proc2[ch] = -1;
  }

  /* random formats: adjust gains and eqs */

  /* initialize static eqs: */

  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQ1, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC, (FIXP_DBL)0,
             params->eq[0]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQ2, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC, (FIXP_DBL)0,
             params->eq[1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQ3, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC, (FIXP_DBL)0,
             params->eq[2]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQ4, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC, (FIXP_DBL)0,
             params->eq[3]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQ5, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC, (FIXP_DBL)0,
             params->eq[4]);

  /* initialize static eqs for the immersive mode */

  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQVF, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC, (FIXP_DBL)0,
             params->eq[RULE_EQVF - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQVB, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC, (FIXP_DBL)0,
             params->eq[RULE_EQVB - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQVFC, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC,
             (FIXP_DBL)0, params->eq[RULE_EQVFC - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQVBC, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC,
             (FIXP_DBL)0, params->eq[RULE_EQVBC - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQVOG, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC,
             (FIXP_DBL)0, params->eq[RULE_EQVOG - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQVS, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC, (FIXP_DBL)0,
             params->eq[RULE_EQVS - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQBTM, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC,
             (FIXP_DBL)0, params->eq[RULE_EQBTM - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQVBA, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC,
             (FIXP_DBL)0, params->eq[RULE_EQVBA - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_06_03, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC,
             (FIXP_DBL)0, params->eq[RULE_06_03 - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_09_03, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC,
             (FIXP_DBL)0, params->eq[RULE_09_03 - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_06_11, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC,
             (FIXP_DBL)0, params->eq[RULE_06_11 - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_09_11, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC,
             (FIXP_DBL)0, params->eq[RULE_09_11 - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_13_11, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC,
             (FIXP_DBL)0, params->eq[RULE_13_11 - 1]);
  compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_18_11, (FIXP_DBL)MAXVAL_DBL, RULE_NOPROC,
             (FIXP_DBL)0, params->eq[RULE_18_11 - 1]);

  /* adaptive eq parameter init */

  return FORMAT_CONVERTER_STATUS_OK;
}

#define FREQ_MACRO(x) ((FIXP_DBL)(x << (31 - 19)))
#define FREQ_E 19

void setActiveDownmixRange_StftErb(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt, INT fs) {
  int i;

  for (i = 0; i < STFT_ERB_BANDS; i++) {
    FIXP_DBL f_tmp = fMult(FREQ_MACRO(fs), fc_StftErb[i]);
    if (f_tmp < FREQ_MACRO(2800 << 1)) {
      fcInt->is4GVH_StftErb[i] = 0;
    } else {
      if (f_tmp > FREQ_MACRO(10000 << 1)) {
        fcInt->is4GVH_StftErb[i] = 0;
      } else {
        fcInt->is4GVH_StftErb[i] = 1;
      }
    }
  }

  for (i = 0; i < STFT_ERB_BANDS; i++) {
    /* search for first non-zero */
    if (fcInt->is4GVH_StftErb[i]) {
      fcInt->erb_is4GVH_L = i;
      break;
    }
  }
  for (; i < STFT_ERB_BANDS; i++) {
    /* search for next zero */
    if (!fcInt->is4GVH_StftErb[i]) {
      fcInt->erb_is4GVH_H = i;
      break;
    }
  }
}

void normalizePG(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt) {
  int chin, chout;

  /* power normalization of GVH */
  for (chin = TFC; chin <= VOG; chin++) {
    FIXP_DBL P = 0;
    INT P_e = 0;
    for (chout = FL; chout <= SR; chout++) {
      /* fAddNorm */
      // P  +=  GVH[chin][chout] * GVH[chin][chout];
      if (fcInt->GVH[chin][chout] == (FIXP_DBL)MAXVAL_DBL) {
        P = fAddNorm(MAXVAL_DBL, 0, P, P_e, &P_e);
      } else {
        P = fAddNorm(fPow2(fcInt->GVH[chin][chout]), 0, P, P_e, &P_e);
      }
    }

    /* sqrtFixp */
    // P    = sqrt (P);

    if (P_e & 0x1) {
      P_e += 1;
      P >>= 1;
    }

    if (P == (FIXP_DBL)MAXVAL_DBL) {
      P = (FIXP_DBL)MAXVAL_DBL;
    } else {
      P = sqrtFixp(P);
    }

    P_e >>= 1;

    for (chout = FL; chout <= SR; chout++) {
      /* fDivNorm */
      // GVH[chin][chout] /= P;
      fcInt->GVH[chin][chout] =
          fDivNormSigned(fcInt->GVH[chin][chout], P, &(fcInt->GVH_e[chin][chout]));
      fcInt->GVH_e[chin][chout] -= P_e;
    }
  }

  /* power normalization of GVL */
  for (chin = TFC; chin <= VOG; chin++) {
    FIXP_DBL P = 0;
    INT P_e = 0;
    for (chout = FL; chout <= SR; chout++) {
      /* fAddNorm */
      // P  +=  GVL[chin][chout] * GVL[chin][chout];
      if (fcInt->GVL[chin][chout] == (FIXP_DBL)MAXVAL_DBL) {
        P = fAddNorm(MAXVAL_DBL, 0, P, P_e, &P_e);
      } else {
        P = fAddNorm(fPow2(fcInt->GVL[chin][chout]), 0, P, P_e, &P_e);
      }
    }

    /* P    = sqrt (P); */
    if (P_e & 0x1) {
      P_e += 1;
      P >>= 1;
    }

    if (P == (FIXP_DBL)MAXVAL_DBL) {
      P = (FIXP_DBL)MAXVAL_DBL;
    } else {
      P = sqrtFixp(P);
    }

    P_e >>= 1;

    for (chout = FL; chout <= SR; chout++) {
      /* fDivNorm */
      // GVL[chin][chout] /= P;
      fcInt->GVL[chin][chout] =
          fDivNormSigned(fcInt->GVL[chin][chout], P, &(fcInt->GVL_e[chin][chout]));
      fcInt->GVL_e[chin][chout] -= P_e;
    }
  }
}

void randElevSptlParms(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt, const INT* randomization,
                       int nchanout) {
  /*  (6) power normalization */
  normalizePG(fcInt);
}

#define FIX_ANGLE_BIN(x) FL2FXCONST_DBL(x)
#define ELV_ANGLE_E 8
#define DBL_O_dot_O5 (0x66666666) /* 0.05 exp = -4 */
#define DBL_O_dot_O5_e -4
#define DBL_O_dot_O7 (0x47ae147b) /* 0.07 exp = -3 */
#define DBL_O_dot_O7_e -3
#define DBL_1_div_20 (0x47ae147b) /* 16*(1/20), exp = -4 */
#define DBL_1_div_20_e -4
#define DBL_10 (0x50000000) /* 16*(1/20), exp = 4 */
#define DBL_10_e 4

#define DBL_log_constant (0x53abbb45) /* 20 / (log(10.0f) * log2(10)) */
#define DBL_log_constant_e 2

#define DBL_O_dot_O55 (0x8ee8d10f) /* -0.05522f */
#define DBL_O_dot_O55_e -4
#define DBL_O_dot_41 (0x6b35d24a) /* 0.41879f */
#define DBL_O_dot_41_e -1
#define DBL_O_dot_O47 (0x9eec397a) /* -0.047401f */
#define DBL_O_dot_O47_e -4
#define DBL_O_dot_14 (0x4cb923a3) /* 0.14985f */
#define DBL_O_dot_14_e -2

/*
  fcInt: Format Converter internal handle.
  elv: Azimuth in [0 ... 1] = [0 ... 180]
  num_band: number of freq bands (STFT STFT_ERB_BANDS, QMF 71 )
  fs: Sample Freq. Q12 in FIXP_DBL
  randomization: Randomization data for random set.
*/
void initElevSptlParms(IIS_FORMATCONVERTER_INTERNAL_HANDLE fcInt, FIXP_DBL elv, const UINT num_band,
                       const FIXP_DBL fs, const INT* randomization, int nchanout) {
  UINT CH;

  /* setActiveDownmixRange(fcInt, fs); 71 bands QMF domanin */
  setActiveDownmixRange_StftErb(fcInt, fs); /*  */
  /* fcInt->D2    = (int) ( fs * 0.003 / 64 + 0.5 );  Only needed in QMF domain */

  for (CH = 0; CH < 13; CH++) {
    fcInt->topIn[CH] = -1;
  }

  for (CH = 0; CH < (UINT)nchanout; CH++) {
    fcInt->midOut[CH] = CH; /* 5.1 layout */
  }
  if (nchanout == 5) {
    fcInt->midOut[3]++;
    fcInt->midOut[4]++;
  }
  /* update panning coefficients if random ( normalizePG is called anyways ) */
  randElevSptlParms(fcInt, randomization, nchanout);

  return;
}
