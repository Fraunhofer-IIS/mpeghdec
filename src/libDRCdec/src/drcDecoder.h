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

/************************* MPEG-D DRC decoder library **************************

   Author(s):

   Description:

*******************************************************************************/

#ifndef DRCDECODER_H
#define DRCDECODER_H

/* drcDecoder.h: definitions used in all submodules */

/* Maximum number of DRC sets applied simulateneously, see section 6.3.5 of ISO/IEC 23003-4. */
#define MAX_ACTIVE_DRCS 3
#define ACTIVE_DRC_LOCATIONS 2

typedef enum { DM_REGULAR_DELAY = 0, DM_LOW_DELAY = 1 } DELAY_MODE;

typedef enum {
  DE_OK = 0,
  DE_NOT_OK = -100,
  DE_PARAM_OUT_OF_RANGE,
  DE_PARAM_INVALID,
  DE_MEMORY_ERROR
} DRC_ERROR;

typedef enum { SDM_OFF, SDM_QMF64, SDM_QMF71, SDM_STFT256 } SUBBAND_DOMAIN_MODE;

#define DOWNMIX_ID_BASE_LAYOUT 0x0
#define DOWNMIX_ID_ANY_DOWNMIX 0x7F
#define DRC_SET_ID_NO_DRC 0x0
#define DRC_SET_ID_ANY_DRC 0x3F

#define LOCATION_MP4_INSTREAM_UNIDRC 0x1
#define LOCATION_MP4_DYN_RANGE_INFO 0x2
#define LOCATION_MP4_COMPRESSION_VALUE 0x3
#define LOCATION_SELECTED LOCATION_MP4_INSTREAM_UNIDRC /* set to location selected by system */

#define MAX_REQUESTS_DOWNMIX_ID 15
#define MAX_REQUESTS_DRC_FEATURE 7
#define MAX_REQUESTS_DRC_EFFECT_TYPE 15

#define DEFAULT_LOUDNESS_DEVIATION_MAX 63

#define DRC_INPUT_LOUDNESS_TARGET FL2FXCONST_DBL(-31.0f / (float)(1 << 7))
#define DRC_INPUT_LOUDNESS_TARGET_SGL FL2FXCONST_SGL(-31.0f / (float)(1 << 7))

#endif
