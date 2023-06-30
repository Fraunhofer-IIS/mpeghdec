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

/************************* MPEG-H 3DA decoder library **************************

   Author(s):   Josef Hoepfl

   Description:

*******************************************************************************/

#include "aac_ram.h"
#include "aac_rom.h"

#define WORKBUFFER1_TAG 0
#define WORKBUFFER2_TAG 1

#define WORKBUFFER3_TAG 4
#define WORKBUFFER4_TAG 5

#define WORKBUFFER5_TAG 6

#define WORKBUFFER6_TAG 7

#define WORKBUFFER7_TAG 8

/*! The structure AAC_DECODER_INSTANCE is the top level structure holding all decoder
   configurations, handles and structs.
 */
C_ALLOC_MEM(AacDecoder, struct AAC_DECODER_INSTANCE, 1)

/*!
  \name StaticAacData

  Static memory areas, must not be overwritten in other sections of the decoder
*/
/* @{ */

/*! The structure CAacDecoderStaticChannelInfo contains the static sideinfo which is needed
    for the decoding of one aac channel. <br>
    Dimension: #AacDecoderChannels                                                      */
C_ALLOC_MEM2(AacDecoderStaticChannelInfo, CAacDecoderStaticChannelInfo, 1, (28))

/*! The structure CAacDecoderChannelInfo contains the dynamic sideinfo which is needed
    for the decoding of one aac channel. <br>
    Dimension: #AacDecoderChannels                                                      */
C_AALLOC_MEM2(AacDecoderChannelInfo, CAacDecoderChannelInfo, 1, (28))

/*! Concealment spectrum buffer */
C_AALLOC_MEM2(ConcealmentSpecBuffer, FIXP_CNCL, 1024, (28))

/*! Overlap buffer */
C_AALLOC_MEM2(OverlapBuffer, FIXP_DBL, OverlapBufferSize, (28))

/*! The structure CpePersistentData holds the persistent data shared by both channels of a CPE. <br>
    It needs to be allocated for each CPE. <br>
    Dimension: 1                                                                        */
C_ALLOC_MEM(CpePersistentData, CpePersistentData, 1)

/*! The structure CCplxPredictionData holds data for complex stereo prediction. <br>
    Dimension: 1                                                                        */
C_ALLOC_MEM(CplxPredictionData, CCplxPredictionData, 1)

/* @} */

/*!
  \name DynamicAacData

  Dynamic memory areas, might be reused in other algorithm sections,
  e.g. the sbr decoder
*/

C_ALLOC_MEM_OVERLAY(WorkBufferCore2, FIXP_DBL, ((24) * (1024 * 3)), SECT_DATA_L2, WORKBUFFER2_TAG)

C_ALLOC_MEM_OVERLAY(WorkBufferCore6, FIXP_DBL,
                    (((INT)sizeof(CAacDecoderCommonData) + (INT)sizeof(FIXP_DBL)) /
                     (INT)sizeof(FIXP_DBL)),
                    SECT_DATA_L2, WORKBUFFER6_TAG)

C_ALLOC_MEM_OVERLAY(WorkBufferCore1, CWorkBufferCore1, 1, SECT_DATA_L1, WORKBUFFER1_TAG)

C_ALLOC_MEM_OVERLAY(WorkBufferCore5, PCM_DEC, (24) * ((1024 * 3) + 256), SECT_DATA_EXTERN,
                    WORKBUFFER5_TAG)
