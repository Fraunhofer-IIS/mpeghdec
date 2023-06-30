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

   Author(s):   Achim Kuntz, Alfonso Pino, Thomas Blender

   Description: format converter library

*******************************************************************************/

#ifndef FDK_FORMATCONVERTERLIB_H
#define FDK_FORMATCONVERTERLIB_H

#include "FDK_cicp2geometry.h"
#include "common_fix.h"
#include "FDK_bitstream.h"
#include "FDK_drcDecLib.h"

#define AFC_MAX_IN_CHANNELS 32
#define AFC_MAX_OUT_CHANNELS 32
#define MAX_NUM_DMX_RULES 200

typedef enum {
  IIS_FORMATCONVERTER_MODE_INVALID = 0,
  IIS_FORMATCONVERTER_MODE_PASSIVE_TIME_DOMAIN = 1,
  IIS_FORMATCONVERTER_MODE_PASSIVE_FREQ_DOMAIN = 2,
  IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN = 3,
  IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN = 4,
  IIS_FORMATCONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT = 5,
  IIS_FORMATCONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT = 6
} IIS_FORMATCONVERTER_MODE;

typedef struct {
  UINT numLocalSpeaker;
  INT pas;
  INT aes;
  void* member;
  UINT numSignalsTotal;
  IIS_FORMATCONVERTER_MODE fCMode;
} IIS_FORMATCONVERTER, *IIS_FORMATCONVERTER_HANDLE;

/** */
int IIS_FormatConverter_Create(IIS_FORMATCONVERTER_HANDLE* self, IIS_FORMATCONVERTER_MODE mode,
                               CICP2GEOMETRY_CHANNEL_GEOMETRY* outGeo, UINT numOutChannels,
                               UINT sampleRate, UINT frameSize);

/**********************************************************************/ /**
 IIS_FormatConverter_Config_SetDeviationAngles() will set the horizontal
 and vertical deviation of the output speakers. the local setup has to be known
 before this function is called.

 \return returns 0 for no error.
 **************************************************************************/

/**********************************************************************/ /**
 IIS_FormatConverter_Config_AddInputSetup() will add the geometric information
 of another channel signal group with given channels.


 \return returns 0 for no error, 1 in case of an error.
 **************************************************************************/
int IIS_FormatConverter_Config_AddInputSetup(
    IIS_FORMATCONVERTER_HANDLE self, CICP2GEOMETRY_CHANNEL_GEOMETRY* geo, UINT numChannels,
    UINT channelOffset, /**< in: offset of the first channel of the setup in the buffer handed over
                           to process() */
    int* channelId /**< in: Array of ids - will be used to identify signals with the correpsonding
                      downmixmatrices (if given) In MPEG-H context this id is conform with the
                      metaDataElementId. May be NULL if no downmixmatrices are handed over.*/
);

void IIS_FormatConverter_Config_SetAES(IIS_FORMATCONVERTER_HANDLE self, UINT aes);

void IIS_FormatConverter_Config_SetPAS(IIS_FORMATCONVERTER_HANDLE self, UINT pas);

int IIS_FormatConverter_Config_SetImmersiveDownmixFlag(IIS_FORMATCONVERTER_HANDLE self,
                                                       UINT immersiveFlag);

int IIS_FormatConverter_Config_SetRendering3DTypeFlag(IIS_FORMATCONVERTER_HANDLE self,
                                                      UINT rendering3DtypeFlag);

int IIS_FormatConverter_GetDelay(IIS_FORMATCONVERTER_HANDLE self, UINT* delay);
/**  Call this open after all _Config_ methods have been called. The FormatConverter will then
   initialize itself acording to those settings.
*/
int IIS_FormatConverter_Open(IIS_FORMATCONVERTER_HANDLE self, INT* p_buffer, UINT buf_size);

INT IIS_FormatConverter_Process(IIS_FORMATCONVERTER_HANDLE self, HANDLE_DRC_DECODER hDrcDec,
                                FIXP_DBL* deinBuffer, FIXP_DBL* deoutBuffer,
                                const int timeBufferChannelOffset);

/**********************************************************************/ /**

 \return returns 0 for no error, 1 in case of an error.
 **************************************************************************/

int IIS_FormatConverter_Close(IIS_FORMATCONVERTER_HANDLE* self);

INT FormatConverterFrame(IIS_FORMATCONVERTER_HANDLE FormatConverter, HANDLE_FDK_BITSTREAM hBitStr);

#endif /* FDK_FORMATCONVERTERLIB_H */
