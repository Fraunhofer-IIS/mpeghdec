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

 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using
 the code for products that do not conform to MPEG-related ITU Recommendations and/or
 ISO/IEC International Standards.

 This copyright notice must be included in all copies or derivative works.

 Copyright (c) ISO/IEC 2013.

 ***********************************************************************************/
#include <string.h>

#include "common_fix.h"

/* locals includes */
#include "FDK_cicp2geometry.h"

typedef enum {
  CH_M_L030 = 0,
  CH_M_R030 = 1,
  CH_M_000 = 2,
  CH_LFE1 = 3,
  CH_M_L110 = 4,
  CH_M_R110 = 5,
  CH_M_L022 = 6,
  CH_M_R022 = 7,
  CH_M_L135 = 8,
  CH_M_R135 = 9,
  CH_M_180 = 10,
  CH_RES1 = 11,
  CH_RES2 = 12,
  CH_M_L090 = 13,
  CH_M_R090 = 14,
  CH_M_L060 = 15,
  CH_M_R060 = 16,
  CH_U_L030 = 17,
  CH_U_R030 = 18,
  CH_U_000 = 19,
  CH_U_L135 = 20,
  CH_U_R135 = 21,
  CH_U_180 = 22,
  CH_U_L090 = 23,
  CH_U_R090 = 24,
  CH_T_000 = 25,
  CH_LFE2 = 26,
  CH_L_L045 = 27,
  CH_L_R045 = 28,
  CH_L_000 = 29,
  CH_U_L110 = 30,
  CH_U_R110 = 31,
  CH_U_L045 = 32,
  CH_U_R045 = 33,
  CH_M_L045 = 34,
  CH_M_R045 = 35,
  CH_LFE3 = 36,
  CH_M_LSCR = 37,
  CH_M_RSCR = 38,
  CH_M_LSCH = 39,
  CH_M_RSCH = 40,
  CH_M_L150 = 41,
  CH_M_R150 = 42
  /* Non standard positions */
  ,
  CH_B_000 = 128,
  CH_B_L030 = 129,
  CH_B_R030 = 130,
  CH_SU_000 = 131,
  CH_SU_L030 = 132,
  CH_SU_R030 = 133,
  CH_SU_L110 = 134,
  CH_SU_R110 = 135,
  CH_SU_L078 = 136,
  CH_SU_R078 = 137,
  CH_M_L120 = 138,
  CH_M_R120 = 139,
  CH_B_L110 = 140,
  CH_B_R110 = 141
} CICP2GEOMETRAY_CICP_SPEAKER_INDEX_LABEL;

/**********************************************************************/ /**

 **************************************************************************/
#define CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE_NON_STD_GAP (128 - 42 - 1)
const CICP2GEOMETRY_CHANNEL_GEOMETRY CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE[] = {
    /* idx  az    el lfe screenRel   loudspeaker type             informative label (merely
       descriptive) */
    {0, 30, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},   /**< left front   */
    {1, -30, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},  /**< right front  */
    {2, 0, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},    /**< centre front    */
    {3, 0, -15, 1, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},  /**< Front LFE  */
    {4, 110, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},  /**< left surround  */
    {5, -110, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right surround */
    {6, 22, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},   /**< left front centre   */
    {7, -22, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},  /**< right front centre  */
    {8, 135, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},  /**< rear surround left 1  */
    {9, -135, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< rear surround right 1 */
    {10, 180, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< rear centre */
    {11, -1, -1, -1, -1, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_INVALID}, /**< reserved */
    {12, -1, -1, -1, -1, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_INVALID}, /**< reserved */
    {13, 90, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},  /**< left side surround      */
    {14, -90, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right side surround     */
    {15, 60, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},  /**< left front wide      */
    {16, -60, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front wide     */
    {17, 30, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front vertical height 1       */
    {18, -30, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front vertical height 1      */
    {19, 0, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< centre front vertical height       */
    {20, 135, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left surround vertical height rear */
    {21, -135, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right surround vertical height rear*/
    {22, 180, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< centre vertical height rear        */
    {23, 90, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left vertical height side surround */
    {24, -90, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right vertical height side surround*/
    {25, 0, 90, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},   /**< top centre surround   */
    {26, 45, -15, 1, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front LFE */
    {27, 45, -15, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front vertical bottom         */
    {28, -45, -15, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front vertical bottom        */
    {29, 0, -15, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< centre front vertical bottom       */
    {30, 110, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left surround vertical height      */
    {31, -110, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right surround vertical height     */
    {32, 45, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front vertical height  2      */
    {33, -45, 35, 0, 0, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front vertical height 2      */
    {34, 45, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},    /**< left front 2    */
    {35, -45, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},   /**< right front 2   */
    {36, -45, -15, 1, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front LFE */
    {37, 60, 0, 0, 1, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},  /**< left edge of display    */
    {38, -60, 0, 0, 1, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right edge of display   */
    {39, 30, 0, 0, 1, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< half-way btw. centre of display and left edge of display
                                        */
    {40, -30, 0, 0, 1, 0, 0, 0, 0,
     CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< half-way btw. centre of display and right edge of
                                          display */
    {41, 150, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},  /**< rear surround left 2  */
    {42, -150, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< rear surround right 2 */
    /* Non standard positions */
    {128, 0, -20, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {129, 30, -20, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {130, -30, -20, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {131, 0, 30, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {132, 30, 30, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {133, -30, 30, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {134, 110, 30, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {135, -110, 30, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {136, 78, 60, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {137, -78, 60, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {138, 120, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {139, -120, 0, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {140, 110, -20, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
    {141, -110, -20, 0, 0, 0, 0, 0, 0, CICP2GEOMETRY_LOUDSPEAKER_KNOWN},
};

typedef struct {
  SHORT index;                                     /**< CICP speaker layout index */
  UCHAR numberOfLoudspeakers;                      /**< amount of speakers */
  UCHAR speakerId[CICP2GEOMETRY_MAX_LOUDSPEAKERS]; /**< list of speaker CICP indices */
} CICP2GEOMETRY_CICP_LAYOUT;

const CICP2GEOMETRY_CICP_LAYOUT CICP_speakerLayouts[] = {
    {1, 1, {CH_M_000}},
    {2, 2, {CH_M_L030, CH_M_R030}},
    {3, 3, {CH_M_L030, CH_M_R030, CH_M_000}},
    {4, 4, {CH_M_L030, CH_M_R030, CH_M_000, CH_M_180}},
    {5, 5, {CH_M_L030, CH_M_R030, CH_M_000, CH_M_L110, CH_M_R110}},
    {6, 6, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110}},
    {7, 8, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060}},
    {9, 3, {CH_M_L030, CH_M_R030, CH_M_180}},
    {10, 4, {CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110}},
    {11, 7, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180}},
    {12, 8, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L135, CH_M_R135}},
    {13, 24, {CH_M_L060, CH_M_R060, CH_M_000, CH_LFE2,  CH_M_L135, CH_M_R135,
              CH_M_L030, CH_M_R030, CH_M_180, CH_LFE3,  CH_M_L090, CH_M_R090,
              CH_U_L045, CH_U_R045, CH_U_000, CH_T_000, CH_U_L135, CH_U_R135,
              CH_U_L090, CH_U_R090, CH_U_180, CH_L_000, CH_L_L045, CH_L_R045}},
    {14, 8, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030}},
    {15,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE2, CH_M_L135, CH_M_R135, CH_LFE3, CH_M_L090, CH_M_R090,
      CH_U_L045, CH_U_R045, CH_U_180}},
    {16,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030,
      CH_U_L110, CH_U_R110}},
    {17,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_000,
      CH_U_L110, CH_U_R110, CH_T_000}},
    {18,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L150, CH_M_R150,
      CH_U_L030, CH_U_R030, CH_U_000, CH_U_L110, CH_U_R110, CH_T_000}},
    {19,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135}},
    {20,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L045, CH_U_R045, CH_U_L135, CH_U_R135, CH_M_LSCR, CH_M_RSCR}},
    {100,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030}},
    {101, 3, {CH_M_L030, CH_M_R030, CH_LFE1}},
    {102, 4, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1}},
    {103, 5, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110}},
    {104, 6, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_U_L030, CH_U_R030}},
    {105,
     9,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030}},
    {106, 5, {CH_M_L030, CH_M_R030, CH_M_000, CH_U_L030, CH_U_R030}},
    {107, 7, {CH_M_L030, CH_M_R030, CH_M_000, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060}},
    {108, 6, {CH_M_L030, CH_M_R030, CH_M_000, CH_M_L110, CH_M_R110, CH_M_180}},
    {109, 7, {CH_M_L030, CH_M_R030, CH_M_000, CH_M_L110, CH_M_R110, CH_M_L135, CH_M_R135}},
    {110, 7, {CH_M_L030, CH_M_R030, CH_M_000, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030}},
    {111,
     9,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_L110,
      CH_U_R110}},
    {112,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_000,
      CH_T_000, CH_U_L110, CH_U_R110}},
    {113,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_000,
      CH_T_000, CH_U_L110, CH_U_R110, CH_M_L150, CH_M_R150}},
    {114,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030, CH_U_L135, CH_U_R135}},
    {115,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L135,
      CH_U_R135, CH_U_L045, CH_U_R045, CH_M_LSCR, CH_M_RSCR}},
    {116, 5, {CH_M_L030, CH_M_R030, CH_LFE1, CH_U_L030, CH_U_R030}},
    {117, 5, {CH_M_L030, CH_M_R030, CH_LFE1, CH_U_L090, CH_U_R090}},
    {118, 5, {CH_M_L030, CH_M_R030, CH_LFE1, CH_U_L110, CH_U_R110}},
    {119, 6, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_U_L090, CH_U_R090}},
    {120, 6, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_U_L110, CH_U_R110}},
    {121, 7, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030}},
    {122, 7, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L090, CH_U_R090}},
    {123, 7, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L110, CH_U_R110}},
    {124,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_L090,
      CH_U_R090}},
    {125,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_L110,
      CH_U_R110}},
    {126,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L090, CH_U_R090, CH_U_L110,
      CH_U_R110}},
    {127,
     10,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_T_000,
      CH_U_L110, CH_U_R110}},
    {128,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_L090,
      CH_U_R090, CH_U_L110, CH_U_R110}},
    {129, 8, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L090, CH_U_R090}},
    {130, 8, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L110, CH_U_R110}},
    {131,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030,
      CH_U_L090, CH_U_R090}},
    {132,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L090, CH_U_R090,
      CH_U_L110, CH_U_R110}},
    {133,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_T_000,
      CH_U_L110, CH_U_R110}},
    {134,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_000,
      CH_U_L110, CH_U_R110}},
    {135,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030,
      CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {136, 5, {CH_M_L030, CH_M_R030, CH_LFE1, CH_U_L135, CH_U_R135}},
    {137, 6, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_U_L135, CH_U_R135}},
    {138, 7, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L135, CH_U_R135}},
    {139,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_L135,
      CH_U_R135}},
    {140,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L135, CH_U_R135, CH_U_L090,
      CH_U_R090}},
    {141,
     10,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_L135,
      CH_U_R135, CH_T_000}},
    {142,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_L135,
      CH_U_R135, CH_U_L090, CH_U_R090}},
    {143, 8, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L135, CH_U_R135}},
    {144,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135}},
    {145,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L135, CH_U_R135,
      CH_U_L090, CH_U_R090}},
    {146,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135, CH_T_000}},
    {147,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_000,
      CH_U_L135, CH_U_R135}},
    {148,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_000,
      CH_U_L135, CH_U_R135, CH_T_000}},
    {149,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {150, 6, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180}},
    {151, 8, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030}},
    {152, 8, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L090, CH_U_R090}},
    {153, 8, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L110, CH_U_R110}},
    {154,
     10,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L090, CH_U_R090}},
    {155,
     10,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L110, CH_U_R110}},
    {156,
     10,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L090, CH_U_R090,
      CH_U_L110, CH_U_R110}},
    {157,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030, CH_T_000,
      CH_U_L110, CH_U_R110}},
    {158,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {159,
     9,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030,
      CH_U_R030}},
    {160,
     9,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L090,
      CH_U_R090}},
    {161,
     9,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L110,
      CH_U_R110}},
    {162,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L090, CH_U_R090}},
    {163,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L110, CH_U_R110}},
    {164,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L090, CH_U_R090,
      CH_U_L110, CH_U_R110}},
    {165,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_T_000, CH_U_L110, CH_U_R110}},
    {166,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_000, CH_U_L110, CH_U_R110}},
    {167,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_000, CH_T_000, CH_U_L110, CH_U_R110}},
    {168,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {169, 8, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L135, CH_U_R135}},
    {170,
     10,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135}},
    {171,
     10,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L135, CH_U_R135,
      CH_U_L090, CH_U_R090}},
    {172,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135, CH_T_000}},
    {173,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {174,
     9,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L135,
      CH_U_R135}},
    {175,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135}},
    {176,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L135, CH_U_R135,
      CH_U_L090, CH_U_R090}},
    {177,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135, CH_T_000}},
    {178,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_000, CH_U_L135, CH_U_R135}},
    {179,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_000, CH_U_L135, CH_U_R135, CH_T_000}},
    {180,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {181, 7, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090}},
    {182,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030}},
    {183,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L090,
      CH_U_R090}},
    {184,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L110,
      CH_U_R110}},
    {185,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030, CH_U_L090, CH_U_R090}},
    {186,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030, CH_U_L110, CH_U_R110}},
    {187,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L090,
      CH_U_R090, CH_U_L110, CH_U_R110}},
    {188,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030, CH_T_000, CH_U_L110, CH_U_R110}},
    {189,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {190, 8, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090}},
    {191,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L090, CH_U_R090}},
    {192,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L110, CH_U_R110}},
    {193,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090}},
    {194,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {195,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_U_L110, CH_U_R110}},
    {196,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_T_000, CH_U_L110, CH_U_R110}},
    {197,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_U_000, CH_U_L110, CH_U_R110}},
    {198,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_U_000, CH_T_000, CH_U_L110, CH_U_R110}},
    {199,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {200,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L135,
      CH_U_R135}},
    {201,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030, CH_U_L135, CH_U_R135}},
    {202,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L135,
      CH_U_R135, CH_U_L090, CH_U_R090}},
    {203,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030, CH_U_L135, CH_U_R135, CH_T_000}},
    {204,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {205,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L135, CH_U_R135}},
    {206,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {207,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_T_000}},
    {208,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_U_000, CH_U_L135, CH_U_R135}},
    {209,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_U_000, CH_U_L135, CH_U_R135, CH_T_000}},
    {210,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {211,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L045,
      CH_U_R045}},
    {212,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L090,
      CH_U_R090, CH_U_L045, CH_U_R045}},
    {213,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L110,
      CH_U_R110, CH_U_L045, CH_U_R045}},
    {214,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_T_000,
      CH_U_L110, CH_U_R110, CH_U_L045, CH_U_R045}},
    {215,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L090,
      CH_U_R090, CH_U_L110, CH_U_R110, CH_U_L045, CH_U_R045}},
    {216,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L090, CH_U_R090, CH_U_L045, CH_U_R045}},
    {217,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L110, CH_U_R110, CH_U_L045, CH_U_R045}},
    {218,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_T_000,
      CH_U_L110, CH_U_R110, CH_U_L045, CH_U_R045}},
    {219,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_000,
      CH_U_L110, CH_U_R110, CH_U_L045, CH_U_R045}},
    {220,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_000,
      CH_T_000, CH_U_L110, CH_U_R110, CH_U_L045, CH_U_R045}},
    {221,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110, CH_U_L045, CH_U_R045}},
    {222,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L135,
      CH_U_R135, CH_U_L045, CH_U_R045}},
    {223,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L135,
      CH_U_R135, CH_T_000, CH_U_L045, CH_U_R045}},
    {224,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L135,
      CH_U_R135, CH_U_L090, CH_U_R090, CH_U_L045, CH_U_R045}},
    {225,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L135, CH_U_R135, CH_U_L045, CH_U_R045}},
    {226,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L135, CH_U_R135, CH_T_000, CH_U_L045, CH_U_R045}},
    {227,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_000,
      CH_U_L135, CH_U_R135, CH_U_L045, CH_U_R045}},
    {228,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_000,
      CH_U_L135, CH_U_R135, CH_T_000, CH_U_L045, CH_U_R045}},
    {229,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090, CH_U_L045, CH_U_R045}},
    {230, 7, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060}},
    {231,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L030,
      CH_U_R030}},
    {232,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L090,
      CH_U_R090}},
    {233,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L110,
      CH_U_R110}},
    {234,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L030,
      CH_U_R030, CH_U_L090, CH_U_R090}},
    {235,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L030,
      CH_U_R030, CH_U_L110, CH_U_R110}},
    {236,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L090,
      CH_U_R090, CH_U_L110, CH_U_R110}},
    {237,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L030,
      CH_U_R030, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {238,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030}},
    {239,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L090, CH_U_R090}},
    {240,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L110, CH_U_R110}},
    {241,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090}},
    {242,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L110, CH_U_R110}},
    {243,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {244,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {245,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L135,
      CH_U_R135}},
    {246,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L030,
      CH_U_R030, CH_U_L135, CH_U_R135}},
    {247,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L135,
      CH_U_R135, CH_U_L090, CH_U_R090}},
    {248,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L030,
      CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {249,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L135, CH_U_R135}},
    {250,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135}},
    {251,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {252,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {253, 8, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060}},
    {254,
     10,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030}},
    {255,
     10,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L090, CH_U_R090}},
    {256,
     10,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L110, CH_U_R110}},
    {257,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090}},
    {258,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L110, CH_U_R110}},
    {259,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {260,
     14,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {261,
     9,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060,
      CH_M_R060}},
    {262,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030}},
    {263,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L090, CH_U_R090}},
    {264,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L110, CH_U_R110}},
    {265,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090}},
    {266,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L110, CH_U_R110}},
    {267,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {268,
     15,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {269,
     10,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L135, CH_U_R135}},
    {270,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135}},
    {271,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {272,
     14,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {273,
     11,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L135, CH_U_R135}},
    {274,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135}},
    {275,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {276,
     15,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {277,
     9,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060}},
    {278,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L030, CH_U_R030}},
    {279,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L090, CH_U_R090}},
    {280,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L110, CH_U_R110}},
    {281,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090}},
    {282,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L110, CH_U_R110}},
    {283,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {284,
     15,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {285,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L135, CH_U_R135}},
    {286,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135}},
    {287,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {288,
     15,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {289,
     10,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060}},
    {290,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030}},
    {291,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L090, CH_U_R090}},
    {292,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L110, CH_U_R110}},
    {293,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090}},
    {294,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L110, CH_U_R110}},
    {295,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {296,
     16,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110}},
    {297,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L135, CH_U_R135}},
    {298,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135}},
    {299,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {300,
     16,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090}},
    {301,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L045, CH_U_R045}},
    {302,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L090, CH_U_R090, CH_U_L045, CH_U_R045}},
    {303,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L110, CH_U_R110, CH_U_L045, CH_U_R045}},
    {304,
     15,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110, CH_U_L045, CH_U_R045}},
    {305,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L135, CH_U_R135, CH_U_L045, CH_U_R045}},
    {306,
     15,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090, CH_U_L045, CH_U_R045}},
    {307,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L045, CH_U_R045}},
    {308,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L090, CH_U_R090, CH_U_L045, CH_U_R045}},
    {309,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L110, CH_U_R110, CH_U_L045, CH_U_R045}},
    {310,
     16,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L090, CH_U_R090, CH_U_L110, CH_U_R110, CH_U_L045, CH_U_R045}},
    {311,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L135, CH_U_R135, CH_U_L045, CH_U_R045}},
    {312,
     16,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L135, CH_U_R135, CH_U_L090, CH_U_R090, CH_U_L045, CH_U_R045}},
    {313,
     11,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_L135,
      CH_U_R135, CH_U_L110, CH_U_R110}},
    {314,
     12,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135, CH_U_L110, CH_U_R110}},
    {315,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135, CH_U_L110, CH_U_R110}},
    {316,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_U_L030, CH_U_R030,
      CH_U_L135, CH_U_R135, CH_U_L110, CH_U_R110}},
    {317,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L110, CH_U_R110}},
    {318,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L110, CH_U_R110}},
    {319,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L030,
      CH_U_R030, CH_U_L135, CH_U_R135, CH_T_000}},
    {320,
     12,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L030,
      CH_U_R030, CH_T_000, CH_U_L110, CH_U_R110}},
    {321,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L030,
      CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L110, CH_U_R110}},
    {322,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_T_000}},
    {323,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_000, CH_U_L135, CH_U_R135}},
    {324,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_000, CH_U_L135, CH_U_R135, CH_T_000}},
    {325,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_T_000, CH_U_L110, CH_U_R110}},
    {326,
     13,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_000, CH_U_L110, CH_U_R110}},
    {327,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_000, CH_T_000, CH_U_L110, CH_U_R110}},
    {328,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L110, CH_U_R110}},
    {329,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_T_000}},
    {330,
     13,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_T_000, CH_U_L110, CH_U_R110}},
    {331,
     14,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L110, CH_U_R110}},
    {332,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_T_000}},
    {333,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_000, CH_U_L135, CH_U_R135}},
    {334,
     15,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_000, CH_U_L135, CH_U_R135, CH_T_000}},
    {335,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_T_000, CH_U_L110, CH_U_R110}},
    {336,
     14,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_000, CH_U_L110, CH_U_R110}},
    {337,
     15,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_000, CH_T_000, CH_U_L110, CH_U_R110}},
    {338,
     15,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L110, CH_M_R110, CH_M_180, CH_M_L060, CH_M_R060,
      CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L110, CH_U_R110}},
    {339,
     14,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_T_000}},
    {340,
     14,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L030, CH_U_R030, CH_T_000, CH_U_L110, CH_U_R110}},
    {341,
     15,
     {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_M_L060,
      CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L110, CH_U_R110}},
    {342,
     15,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_T_000}},
    {343,
     15,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_U_000, CH_U_L135, CH_U_R135}},
    {344,
     16,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_U_000, CH_U_L135, CH_U_R135, CH_T_000}},
    {345,
     15,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_T_000, CH_U_L110, CH_U_R110}},
    {346,
     15,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_U_000, CH_U_L110, CH_U_R110}},
    {347,
     16,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_U_000, CH_T_000, CH_U_L110, CH_U_R110}},
    {348,
     16,
     {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090,
      CH_M_L060, CH_M_R060, CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135, CH_U_L110, CH_U_R110}},
    {349, 4, {CH_M_L030, CH_M_R030, CH_LFE1, CH_M_180}},
    {350, 5, {CH_M_L030, CH_M_R030, CH_M_000, CH_LFE1, CH_M_180}},
    {351, 2, {CH_M_000, CH_LFE1}},
    {400,
     14,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_SU_000, CH_SU_L030, CH_SU_R030,
      CH_SU_L110, CH_SU_R110, CH_B_000, CH_B_L030, CH_B_R030, CH_LFE1}},
    {401,
     9,
     {CH_M_L030, CH_M_R030, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030, CH_U_R030,
      CH_LFE1}},
    {402,
     10,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L135, CH_M_R135, CH_M_L090, CH_M_R090, CH_U_L030,
      CH_U_R030, CH_LFE1}},
    {403,
     9,
     {CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_U_L045, CH_U_R045, CH_U_L110, CH_U_R110,
      CH_LFE1}},
    {404,
     10,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_U_L045, CH_U_R045, CH_U_L110,
      CH_U_R110, CH_LFE1}},
    {405,
     9,
     {CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_L135, CH_U_R135,
      CH_LFE1}},
    {406,
     10,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_U_L030, CH_U_R030, CH_U_L135,
      CH_U_R135, CH_LFE1}},
    {407,
     11,
     {CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_U_L045, CH_U_R045, CH_U_L090, CH_U_R090,
      CH_U_L110, CH_U_R110, CH_LFE1}},
    {408,
     12,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_U_L045, CH_U_R045, CH_U_L090,
      CH_U_R090, CH_U_L110, CH_U_R110, CH_LFE1}},
    {409,
     9,
     {CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L045, CH_U_R045,
      CH_LFE1}},
    {410,
     10,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L045,
      CH_U_R045, CH_LFE1}},
    {411,
     11,
     {CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L045, CH_U_R045,
      CH_U_L110, CH_U_R110, CH_LFE1}},
    {412,
     12,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_M_L060, CH_M_R060, CH_U_L045,
      CH_U_R045, CH_U_L110, CH_U_R110, CH_LFE1}},
    {413,
     13,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_SU_000, CH_SU_L030, CH_SU_R030,
      CH_SU_L110, CH_SU_R110, CH_B_000, CH_B_L030, CH_B_R030}},
    {414,
     13,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_U_000, CH_U_L030, CH_U_R030,
      CH_U_L110, CH_U_R110, CH_B_000, CH_B_L030, CH_B_R030}},
    {415,
     11,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_M_L150, CH_M_R150, CH_U_L045,
      CH_U_R045, CH_U_L135, CH_U_R135}},
    {416,
     9,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_U_L045, CH_U_R045, CH_U_L135,
      CH_U_R135}},
    {417, 7, {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L120, CH_M_R120, CH_SU_L078, CH_SU_R078}},
    {418,
     14,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_U_000, CH_U_L030, CH_U_R030,
      CH_U_L110, CH_U_R110, CH_B_000, CH_B_L030, CH_B_R030, CH_LFE1}},
    {419,
     12,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_M_L150, CH_M_R150, CH_U_L045,
      CH_U_R045, CH_U_L135, CH_U_R135, CH_LFE1}},
    {420,
     10,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_U_L045, CH_U_R045, CH_U_L135,
      CH_U_R135, CH_LFE1}},
    {421,
     8,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L120, CH_M_R120, CH_SU_L078, CH_SU_R078, CH_LFE1}},
    {422,
     10,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_U_000, CH_U_180, CH_M_L120, CH_M_R120, CH_SU_L078,
      CH_SU_R078, CH_LFE1}},
    {423,
     16,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_U_000, CH_U_L030, CH_U_R030,
      CH_U_L110, CH_U_R110, CH_B_000, CH_B_L030, CH_B_R030, CH_B_L110, CH_B_R110, CH_LFE1}},
    {424,
     16,
     {CH_M_000, CH_M_L030, CH_M_R030, CH_M_L110, CH_M_R110, CH_SU_000, CH_SU_L030, CH_SU_R030,
      CH_SU_L110, CH_SU_R110, CH_B_000, CH_B_L030, CH_B_R030, CH_B_L110, CH_B_R110, CH_LFE1}},
};

/**********************************************************************/ /**

 **************************************************************************/

static int matchLoudspeakers(const CICP2GEOMETRY_CHANNEL_GEOMETRY tmpAzElLfe,
                             const CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe) {
  int error = 0;

  error = (tmpAzElLfe.Az != AzElLfe.Az);
  error |= (tmpAzElLfe.El != AzElLfe.El);
  error |= (tmpAzElLfe.LFE != AzElLfe.LFE);
  error |= (tmpAzElLfe.screenRelative != AzElLfe.screenRelative);

  return error;
}

/**********************************************************************/ /**

 **************************************************************************/

static int matchLoudspeakerSetups(
    CICP2GEOMETRY_CHANNEL_GEOMETRY* const AzElLfe,
    const CICP2GEOMETRY_CHANNEL_GEOMETRY tmpAzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
    const int numChannels, const int tmpNumChannels, const int numLFEs, const int tmpNumLFEs,
    const int numTotalChannels) {
  int error = 0;
  int ch = 0;

  if ((numChannels != tmpNumChannels) || (numLFEs != tmpNumLFEs)) {
    error = 1;
    return error;
  }

  for (ch = 0; ch < numTotalChannels; ++ch) {
    error = matchLoudspeakers(tmpAzElLfe[ch], AzElLfe[ch]);
    if (error) {
      break;
    }
  }

  return error;
}

/**********************************************************************/ /**

 **************************************************************************/

CICP2GEOMETRY_ERROR cicp2geometry_get_cicpIndex_from_geometry(
    CICP2GEOMETRY_CHANNEL_GEOMETRY* AzElLfe, int numChannels, int numLFEs, int* cicpIndex) {
  int numTotalChannels = numChannels + numLFEs;

  *cicpIndex = CICP2GEOMETRY_CICP_INVALID;

  for (int i = 0; i < (int)(sizeof(CICP_speakerLayouts) / sizeof(CICP_speakerLayouts[0])); i++) {
    if (numTotalChannels == CICP_speakerLayouts[i].numberOfLoudspeakers) {
      CICP2GEOMETRY_CHANNEL_GEOMETRY tmpAzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
      int tmpNumChannels = 0;
      int tmpNumLFEs = 0;
      int error = 0;

      cicp2geometry_get_geometry_from_cicp(CICP_speakerLayouts[i].index, tmpAzElLfe,
                                           &tmpNumChannels, &tmpNumLFEs);
      error = matchLoudspeakerSetups(AzElLfe, tmpAzElLfe, numChannels, tmpNumChannels, numLFEs,
                                     tmpNumLFEs, numTotalChannels);
      if (error == 0) {
        *cicpIndex = CICP_speakerLayouts[i].index;
        return CICP2GEOMETRY_OK;
      }
    }
  }

  return CICP2GEOMETRY_UNSUPPORTED_CHANNEL_COUNT;
}

/**********************************************************************/ /**

 **************************************************************************/

CICP2GEOMETRY_ERROR cicp2geometry_get_geometry_from_cicp(
    int cicpIndex, CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
    int* numChannels, int* numLFEs) {
  for (int i = 0; i < (int)(sizeof(CICP_speakerLayouts) / sizeof(CICP_speakerLayouts[0])); i++) {
    if (CICP_speakerLayouts[i].index == cicpIndex) {
      int tmpLfe = 0;

      for (int ii = 0; ii < CICP_speakerLayouts[i].numberOfLoudspeakers; ii++) {
        const int speakerIndex = CICP_speakerLayouts[i].speakerId[ii];
        const int tableIndex =
            (speakerIndex < 128)
                ? speakerIndex
                : (speakerIndex - CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE_NON_STD_GAP);
        if (AzElLfe != NULL) {
          AzElLfe[ii] = CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE[tableIndex];
        }
        if (CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE[tableIndex].LFE) {
          tmpLfe++;
        }
      }
      *numChannels = CICP_speakerLayouts[i].numberOfLoudspeakers - tmpLfe;
      *numLFEs = tmpLfe;

      return CICP2GEOMETRY_OK;
    }
  }

  *numChannels = -1;
  *numLFEs = -1;

  return CICP2GEOMETRY_INVALID_CICP_INDEX;
}

CICP2GEOMETRY_ERROR cicp2geometry_get_numChannels_from_cicp(int cicpIndex, int* numTotalChannels) {
  CICP2GEOMETRY_ERROR cicpError = CICP2GEOMETRY_OK;

  int numChannels = 0, numLFEs = 0;

  cicpError = cicp2geometry_get_geometry_from_cicp(cicpIndex, NULL, &numChannels, &numLFEs);

  *numTotalChannels = numChannels + numLFEs;

  return cicpError;
}

/***************************************************************************/

CICP2GEOMETRY_ERROR cicp2geometry_get_geometry_from_cicp_loudspeaker_index(
    int cicpLoudspeakerIndex, CICP2GEOMETRY_CHANNEL_GEOMETRY* AzElLfe) {
  CICP2GEOMETRY_ERROR cicpError = CICP2GEOMETRY_OK;

  if (AzElLfe == NULL) {
    return CICP2GEOMETRY_BAD_POINTER;
  }

  if (cicpLoudspeakerIndex >= 0 &&
      cicpLoudspeakerIndex < (int)(sizeof(CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE) /
                                   sizeof(CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE[0]))) {
    *AzElLfe = CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE[cicpLoudspeakerIndex];

    if (AzElLfe->loudspeakerType == CICP2GEOMETRY_LOUDSPEAKER_INVALID) {
      cicpError = CICP2GEOMETRY_UNKNOWN_SPEAKER_INDEX;
    }

  } else {
    /* save the cicp loudspeaker even if it's not known */
    AzElLfe->cicpLoudspeakerIndex = cicpLoudspeakerIndex;

    AzElLfe->Az = -1;
    AzElLfe->El = -1;
    AzElLfe->LFE = -1;
    AzElLfe->screenRelative = -1;
    AzElLfe->hasDistance = 0;
    AzElLfe->distance = 0;
    AzElLfe->hasLoudspeakerCalibrationGain = 0;
    AzElLfe->loudspeakerCalibrationGain = 0;
    AzElLfe->loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_UNKNOWN;
    cicpError = CICP2GEOMETRY_UNKNOWN_SPEAKER_INDEX;
  }

  return cicpError;
}

/**********************************************************************/

CICP2GEOMETRY_ERROR cicp2geometry_get_number_of_lfes(CICP2GEOMETRY_CHANNEL_GEOMETRY* AzElLfe,
                                                     UINT numChannels, UINT* numLfes) {
  CICP2GEOMETRY_ERROR result = CICP2GEOMETRY_OK;
  UINT i = 0;
  UINT lfeCount = 0;

  for (i = 0; i < numChannels; i++) {
    if (AzElLfe[i].LFE) {
      FDK_ASSERT(AzElLfe[i].LFE != -1); /* Avoid counting negative marked LFE's */
      lfeCount++;
    }
  }

  *numLfes = lfeCount;
  return result;
}

#ifdef FDK_FC_MISPLACED_SPEAKER_ENABLE
CICP2GEOMETRY_ERROR cicp2geometry_get_deviation_angles(
    CICP2GEOMETRY_CHANNEL_GEOMETRY* AzElLfe, UINT numSpeaker, INT* azDev, INT* elDev,
    INT* isDeviated, CICP2GEOMETRY_CHANNEL_GEOMETRY* normalizedGeo) {
  CICP2GEOMETRY_ERROR result = CICP2GEOMETRY_OK;
  CICP2GEOMETRY_CHANNEL_GEOMETRY normedGeo;
  UINT ch;

  for (ch = 0; ch < numSpeaker; ch++) {
    result = cicp2geometry_get_geometry_from_cicp_loudspeaker_index(
        AzElLfe[ch].cicpLoudspeakerIndex, &normedGeo);
    if (result) {
      return result;
    }

    azDev[ch] = AzElLfe[ch].Az - normedGeo.Az;
    elDev[ch] = AzElLfe[ch].El - normedGeo.El;
    if (normalizedGeo != NULL) {
      if (&(normalizedGeo[ch]) != &(AzElLfe[ch]))
        memcpy(&(normalizedGeo[ch]), &(AzElLfe[ch]), sizeof(normalizedGeo[ch]));

      normalizedGeo[ch].Az = normedGeo.Az;
      normalizedGeo[ch].El = normedGeo.El;
    }

    if ((azDev[ch] != 0) || (elDev[ch] != 0)) *isDeviated = 1;
  }

  return result;
}
#endif

CICP2GEOMETRY_ERROR cicp2geometry_get_front_speakers(const INT targetLayout, const INT numSpeakers,
                                                     INT* numSignalsMix, INT* speakerPosIndices,
                                                     INT* speakerGains) {
  const INT speakersToLookFor[2][2] = {{CH_M_000, CH_M_000}, {CH_M_L030, CH_M_R030}};
  CICP2GEOMETRY_CHANNEL_GEOMETRY geoTL[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  int numTLSpeakers, numTLLFE, found = 0;
  CICP2GEOMETRY_ERROR error;

  if (numSpeakers < 0 || numSpeakers > 2) {
    return CICP2GEOMETRY_UNSUPPORTED_CHANNEL_COUNT;
  }

  error = cicp2geometry_get_geometry_from_cicp(targetLayout, geoTL, &numTLSpeakers, &numTLLFE);
  if (error != CICP2GEOMETRY_OK) {
    return error;
  }
  numTLSpeakers += numTLLFE;

  /* Assume direct match */
  *numSignalsMix = numSpeakers;
  for (int i = 0; i < numSpeakers; i++) {
    speakerGains[i] = (FIXP_DBL)MAXVAL_DBL;
  }

  for (int i = 0; i < numSpeakers; i++) {
    for (int t = 0; t < numTLSpeakers; t++) {
      if (geoTL[t].cicpLoudspeakerIndex == speakersToLookFor[numSpeakers - 1][i]) {
        speakerPosIndices[i] = t;
        found += 1;
        break;
      }
    }
  }

  /* If direct match not possible, try switching center / stereo pair */
  if (found != numSpeakers) {
    found = 0;
    int numSpeakersMatch = (numSpeakers == 1) ? 2 : 1;

    for (int i = 0; i < numSpeakersMatch; i++) {
      speakerGains[i] = FL2FXCONST_DBL(0.707);

      for (int t = 0; t < numTLSpeakers; t++) {
        if (geoTL[t].cicpLoudspeakerIndex == speakersToLookFor[numSpeakersMatch - 1][i]) {
          speakerPosIndices[i] = t;
          found += 1;
          break;
        }
      }
    }
    if (numSpeakersMatch < numSpeakers) {
      speakerPosIndices[1] = speakerPosIndices[0];
    }

    if (found == 0) {
      return CICP2GEOMETRY_CONFIG_ERROR;
    }

    *numSignalsMix = numSpeakersMatch;
  }

  return CICP2GEOMETRY_OK;
}
