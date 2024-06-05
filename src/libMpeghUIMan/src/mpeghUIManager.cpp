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

/**************************************  ***************************************
   Author(s): Matthias Neusinger
   Description: MPEG-H Mpegh UI Manager

   This software and/or program is protected by copyright law and international
   treaties. Any reproduction or distribution of this software and/or program,
   or any portion of it, may result in severe civil and criminal penalties, and
   will be prosecuted to the maximum extent possible under law.

*******************************************************************************/

#include "mpeghUIManager.h"
#include "uiManager.h"
#include "tpdec_lib.h"

struct MPEGH_UI_MANAGER {
  HANDLE_UI_MANAGER hUiManager;
  UCHAR isActive;
  UCHAR configFound;
  UCHAR secondaryConfigFound[MAX_NUMBER_SECONDARY_STREAMS];
  UINT mainStreamLabel;
  UINT secondaryStreamLabel[MAX_NUMBER_SECONDARY_STREAMS];
  UCHAR numberfOfSecondaryStreams;
  UCHAR numberOfSecondaryASI;
  UCHAR numberOfMPEGH3DA;
  UINT insertOffset;
};

static UINT nextPow2(UINT x) {
  UINT y = 1;
  while (y < x) y <<= 1;
  return y;
}

/* open */
LINKSPEC_H HANDLE_MPEGH_UI_MANAGER mpegh_UI_Manager_Open(void) {
  HANDLE_MPEGH_UI_MANAGER self;

  self = (HANDLE_MPEGH_UI_MANAGER)FDKcalloc(1, sizeof(MPEGH_UI_MANAGER));

  if (self) {
    if (UI_Manager_Create(&self->hUiManager, 1) != UI_MANAGER_OK) {
      mpegh_UI_Manager_Close(self);
      return NULL;
    }

    self->isActive = 1;
    self->insertOffset = -1;
  }

  return self;
}

/* close */
LINKSPEC_H void mpegh_UI_Manager_Close(HANDLE_MPEGH_UI_MANAGER self) {
  if (self) {
    UI_Manager_Delete(&self->hUiManager);
  }

  FDKfree(self);
}

/* get XML state */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_GetXmlSceneState(HANDLE_MPEGH_UI_MANAGER self, char* xmlOut,
                                                    UINT xmlOutSize, UINT flagsIn, UINT* flagsOut) {
  return (MPEGH_UI_ERROR)UI_Manager_GetXmlSceneState(self->hUiManager, xmlOut, xmlOutSize, flagsIn,
                                                     flagsOut);
}

/* apply XML action */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_ApplyXmlAction(HANDLE_MPEGH_UI_MANAGER self, const char* xmlIn,
                                                  UINT xmlInSize, UINT* flagsOut) {
  if (!self->isActive) return MPEGH_UI_NOT_ALLOWED;

  return (MPEGH_UI_ERROR)UI_Manager_ApplyXmlAction(self->hUiManager, xmlIn, xmlInSize, flagsOut);
}

/* feed MHAS */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_FeedMHAS(HANDLE_MPEGH_UI_MANAGER self, UCHAR* mhasBuffer,
                                            UINT mhasLength) {
  FDK_BITSTREAM bs;
  HANDLE_FDK_BITSTREAM hBs = &bs;
  INT nBitsIns = 0;
  UCHAR frameFound = 0, uiPacketFound = 0;

  FDKinitBitStream(hBs, mhasBuffer, nextPow2(mhasLength), mhasLength * 8);

  nBitsIns = FDKgetValidBits(hBs);

  self->configFound = 0;
  for (int i = 0; i < MAX_NUMBER_SECONDARY_STREAMS; i++) {
    self->secondaryConfigFound[i] = 0;
  }
  self->numberOfSecondaryASI = 0;
  self->numberOfMPEGH3DA = 0;
  self->insertOffset = -1;

  while (FDKgetValidBits(hBs)) {
    mha_pactyp_t packetType = MHA_PACTYP_NONE;
    UINT packetLabel, packetLength;
    INT nBits;
    UCHAR isMainStream = 0;

    /* parse MHAS packet header */
    packetType = (mha_pactyp_t)escapedValue(hBs, 3, 8, 8);
    packetLabel = escapedValue(hBs, 2, 8, 32);
    packetLength = escapedValue(hBs, 11, 24, 24);

    nBits = (INT)FDKgetValidBits(hBs);
    if (nBits < 8 * (INT)packetLength) return MPEGH_UI_PARSE_ERROR;

    /* check if main stream packet */
    if (packetLabel <= 0x10) isMainStream = 1;

    switch (packetType) {
      case MHA_PACTYP_MPEGH3DACFG:
        if (packetLabel == 0) return MPEGH_UI_PARSE_ERROR;

        if (isMainStream) {
          if (self->configFound == 1) return MPEGH_UI_PARSE_ERROR;

          self->configFound = 1;

          /* check for config change */
          if (packetLabel != self->mainStreamLabel) {
            AUDIO_SCENE_INFO* pASI = UI_Manager_GetAsiPointer(self->hUiManager);

            /* clear ASI */
            asiReset(pASI);

            /* save new packet label */
            self->mainStreamLabel = packetLabel;
          }
          self->numberfOfSecondaryStreams = 0;
        } else {
          if (self->secondaryConfigFound[self->numberfOfSecondaryStreams] == 1)
            return MPEGH_UI_PARSE_ERROR;

          self->secondaryConfigFound[self->numberfOfSecondaryStreams] = 1;

          /* check for config change */
          if (packetLabel != self->secondaryStreamLabel[self->numberfOfSecondaryStreams]) {
            /* save new packet label */
            self->secondaryStreamLabel[self->numberfOfSecondaryStreams] = packetLabel;
            self->numberfOfSecondaryStreams += 1;
          }
        }

        break;

      case MHA_PACTYP_AUDIOSCENEINFO:
        if (packetLabel == 0) return MPEGH_UI_PARSE_ERROR;

        if (isMainStream) {
          AUDIO_SCENE_INFO* pASI = UI_Manager_GetAsiPointer(self->hUiManager);

          /* check packet label */
          if (packetLabel != self->mainStreamLabel) return MPEGH_UI_PARSE_ERROR;

          /* parse ASI */
          if (mae_AudioSceneInfo(pASI, hBs, (2 * 28), 0) != TRANSPORTDEC_OK) {
            return MPEGH_UI_PARSE_ERROR;
          }

        } else {
          AUDIO_SCENE_INFO* pASI = UI_Manager_GetAsiPointer(self->hUiManager);

          if (self->numberOfSecondaryASI > self->numberfOfSecondaryStreams)
            return MPEGH_UI_PARSE_ERROR;

          /* check packet label */
          if (packetLabel != self->secondaryStreamLabel[self->numberOfSecondaryASI])
            return MPEGH_UI_PARSE_ERROR;

          self->numberOfSecondaryASI += 1;

          /* parse ASI */
          if (mae_AudioSceneInfo(pASI, hBs, (2 * 28), self->numberOfSecondaryASI) !=
              TRANSPORTDEC_OK) {
            return MPEGH_UI_PARSE_ERROR;
          }
        }

        break;

      case MHA_PACTYP_MARKER:
        if ((packetLength > 2) && (FDKreadBits(hBs, 8) == 0xE0) && (FDKreadBits(hBs, 8) == 0x0) &&
            (isMainStream)) {
          UINT i;
          UINT idBytes = packetLength - 2;
          UCHAR uuid[16];

          if (idBytes > 16) idBytes = 16;

          for (i = 0; i < 16; i++) {
            if (i >= (16 - idBytes)) {
              uuid[i] = FDKreadBits(hBs, 8);
            } else {
              uuid[i] = 0;
            }
          }

          if (self->isActive) {
            UI_Manager_SetUUID(self->hUiManager, uuid, 1);
          }
        }

        break;

      case MHA_PACTYP_USERINTERACTION:
      case MHA_PACTYP_LOUDNESS_DRC:
        if (packetLabel == 0) return MPEGH_UI_PARSE_ERROR;

        if (isMainStream) uiPacketFound = 1;

        break;

      case MHA_PACTYP_MPEGH3DAFRAME:
        if (packetLabel == 0) return MPEGH_UI_PARSE_ERROR;

        if (isMainStream) {
          /* check packet label */
          if (packetLabel != self->mainStreamLabel) return MPEGH_UI_PARSE_ERROR;
        } else {
          if (self->numberOfMPEGH3DA > self->numberfOfSecondaryStreams) return MPEGH_UI_PARSE_ERROR;

          /* check packet label */
          if (packetLabel != self->secondaryStreamLabel[self->numberOfMPEGH3DA])
            return MPEGH_UI_PARSE_ERROR;

          self->numberOfMPEGH3DA += 1;
        }

        if (self->numberOfMPEGH3DA >= self->numberfOfSecondaryStreams) { /* frame found */
          frameFound = 1;
          break;
        }
        break;

      default:
        break;
    }

    /* skip remaining bits */
    nBits = 8 * packetLength - (nBits - FDKgetValidBits(hBs));
    if (nBits >= 0) {
      FDKpushFor(hBs, nBits);
    } else {
      return MPEGH_UI_PARSE_ERROR;
    }

    if (frameFound) break;

    /* update position to insert UI/DRC packets */
    if ((packetType != MHA_PACTYP_MPEGH3DAFRAME) && (packetType != MHA_PACTYP_AUDIOTRUNCATION) &&
        (packetType != MHA_PACTYP_CRC16) && (packetType != MHA_PACTYP_CRC32) &&
        (packetType != MHA_PACTYPE_EARCON) && (packetType != MHA_PACTYPE_PCMCONFIG) &&
        (packetType != MHA_PACTYPE_PCMDATA)) {
      nBitsIns = FDKgetValidBits(hBs);
    }
  }

  /* check if a frame was found */
  if (!frameFound) return MPEGH_UI_PARSE_ERROR;

  /* check if UI packet found and set in/active */
  if (self->isActive && uiPacketFound) {
    self->isActive = 0;
    UI_Manager_SetIsActive(self->hUiManager, 0);
  } else if (!self->isActive && !uiPacketFound && self->configFound) {
    self->isActive = 1;
    UI_Manager_SetIsActive(self->hUiManager, 1);
  }

  /* compute byte offset to insert UI/DRC packets */
  self->insertOffset = mhasLength - (nBitsIns >> 3);

  return MPEGH_UI_OK;
}

/* write MHAS packet header */
static UINT writePacketHeader(HANDLE_FDK_BITSTREAM hBs, UINT packetType, UINT packetLabel,
                              UINT packetLength) {
  UINT nBits = 0;

  nBits += FDKwriteEscapedValue(hBs, packetType, 3, 8, 8);
  nBits += FDKwriteEscapedValue(hBs, packetLabel, 2, 8, 32);
  nBits += FDKwriteEscapedValue(hBs, packetLength, 11, 24, 24);

  return nBits >> 3;
}

/* write UI packet */
static UINT writeUiPacket(HANDLE_FDK_BITSTREAM hBs, const USER_INTERACTIVITY_STATUS* uiStatus) {
  UINT nBits = 0, grp;

  /* mpegh3daElementInteraction */
  nBits += FDKwriteBits(hBs, 0, 8); /* ei_InteractionSignatureDataLength */

  nBits += FDKwriteBits(hBs, uiStatus->interactionMode, 1);
  nBits += FDKwriteBits(hBs, uiStatus->numGroups, 7);
  if (uiStatus->interactionMode != 0) {
    nBits += FDKwriteBits(hBs, uiStatus->groupPresetID, 5);
  }

  for (grp = 0; grp < uiStatus->numGroups; grp++) {
    nBits += FDKwriteBits(hBs, uiStatus->groupData[grp].groupID, 7);
    nBits += FDKwriteBits(hBs, uiStatus->groupData[grp].onOff, 1);
    nBits += FDKwriteBits(hBs, uiStatus->groupData[grp].routeToWIRE, 1);
    if (uiStatus->groupData[grp].routeToWIRE) {
      nBits += FDKwriteBits(hBs, uiStatus->groupData[grp].routeToWireID, 16);
    }

    if (uiStatus->groupData[grp].onOff) {
      UCHAR changePosition = uiStatus->groupData[grp].changePosition;
      UCHAR changeGain = uiStatus->groupData[grp].changeGain;

      /* always set change flags to avoid problems with downmix presets */
      changePosition = 1;
      changeGain = 1;

      nBits += FDKwriteBits(hBs, changePosition, 1);
      if (changePosition) {
        nBits += FDKwriteBits(hBs, uiStatus->groupData[grp].azOffset, 8);
        nBits += FDKwriteBits(hBs, uiStatus->groupData[grp].elOffset, 6);
        nBits += FDKwriteBits(hBs, uiStatus->groupData[grp].distFact, 4);
      }

      nBits += FDKwriteBits(hBs, changeGain, 1);
      if (changeGain) {
        nBits += FDKwriteBits(hBs, uiStatus->groupData[grp].gain, 7);
      }
    }
  }

  nBits += FDKwriteBits(hBs, 0, 1); /* hasLocalZoomAreaSize */

  if (nBits & 7) {
    nBits += FDKwriteBits(hBs, 0, 8 - (nBits & 7));
  }

  return nBits >> 3;
}

/* write DRC/loudness packet */
static UINT writeDrcPacket(HANDLE_FDK_BITSTREAM hBs, const UI_DRC_LOUDNESS_STATUS* drcStatus) {
  UINT nBits = 0;
  int i, n, tmp;

  nBits += FDKwriteBits(hBs, 0, 1); /* uniDrcInterfaceSignaturePresent */

  nBits += FDKwriteBits(hBs, 0, 1); /* systemInterfacePresent */

  if (drcStatus->targetLoudness == UI_MANAGER_USE_DEFAULT_TARGET_LOUDNESS) {
    nBits += FDKwriteBits(hBs, 0, 1); /* loudnessNormalizationControlInterfacePresent */
  } else {
    nBits += FDKwriteBits(hBs, 1, 1); /* loudnessNormalizationControlInterfacePresent */
    nBits += FDKwriteBits(hBs, 1, 1); /* loudnessNormalizationOn */
    nBits += FDKwriteBits(hBs, -(INT)drcStatus->targetLoudness << 5, 12); /* targetLoudness */
  }

  if (drcStatus->albumMode < 0) {
    nBits += FDKwriteBits(hBs, 0, 1); /* loudnessNormalizationParameterInterfacePresent */
  } else {
    nBits += FDKwriteBits(hBs, 1, 1); /* loudnessNormalizationParameterInterfacePresent */
    nBits += FDKwriteBits(hBs, drcStatus->albumMode, 1); /* albumMode */
    nBits += FDKwriteBits(hBs, 1,
                          1); /* peakLimiterPresent = 1, as peak limiter is mandatory in MPEG-H */
    nBits += FDKwriteBits(hBs, 0, 1); /* changeLoudnessDeviationMax */
    nBits += FDKwriteBits(hBs, 0, 1); /* changeLoudnessMeasurementMethod  */
    nBits += FDKwriteBits(hBs, 0, 1); /* changeLoudnessMeasurementSystem */
    nBits += FDKwriteBits(hBs, 0, 1); /* changeLoudnessMeasurementPreProc */
    nBits += FDKwriteBits(hBs, 0, 1); /* changeDeviceCutOffFrequency */
    nBits += FDKwriteBits(hBs, 0, 1); /* changeLoudnessNormalizationGainDbMax */
    nBits += FDKwriteBits(hBs, 0, 1); /* changeLoudnessNormalizationGainModificationDb */
    nBits += FDKwriteBits(hBs, 0, 1); /* changeOutputPeakLevelMax */
  }

  nBits += FDKwriteBits(hBs, 1, 1); /* dynamicRangeControlInterfacePresent */

  if (drcStatus->drcSelected == UI_MANAGER_DRC_OFF) {
    nBits += FDKwriteBits(hBs, 0, 1); /* dynamicRangeControlOn */
  } else if (drcStatus->drcSelected == UI_MANAGER_USE_DEFAULT_DRC_SELECTED) {
    nBits += FDKwriteBits(hBs, 1, 1); /* dynamicRangeControlOn */
    nBits += FDKwriteBits(hBs, 1, 3); /* numDrcFeatureRequests */
    nBits += FDKwriteBits(hBs, 0, 2); /* drcFeatureRequestType */

    nBits += FDKwriteBits(hBs, 0, 4); /* numDrcEffectTypeRequests */
    nBits += FDKwriteBits(hBs, 1, 4);
    /* numDrcEffectTypeRequestsDesired */ /* value 1 to align to legacy bitstreams. It will be
                                             ignored. */
  } else {
    nBits += FDKwriteBits(hBs, 1, 1); /* dynamicRangeControlOn */

    n = 0;
    tmp = drcStatus->drcSelected;
    for (i = 0; i < 8; i++) {
      if ((i == 0) || (tmp & 15)) n++;
      tmp >>= 4;
    }

    nBits += FDKwriteBits(hBs, 1, 3); /* numDrcFeatureRequests */
    nBits += FDKwriteBits(hBs, 0, 2); /* drcFeatureRequestType */

    nBits += FDKwriteBits(hBs, n, 4); /* numDrcEffectTypeRequests */
    nBits += FDKwriteBits(hBs, 1, 4); /* numDrcEffectTypeRequestsDesired */

    tmp = drcStatus->drcSelected;
    for (i = 0; i < 8; i++) {
      if ((i == 0) || (tmp & 15)) nBits += FDKwriteBits(hBs, tmp & 15, 4);
      tmp >>= 4;
    }
  }

  if (drcStatus->boost < 0 && drcStatus->compress < 0) {
    nBits += FDKwriteBits(hBs, 0, 1); /* dynamicRangeControlParameterInterfacePresent */
  } else {
    int changeCompress = drcStatus->compress < 0 ? 0 : 1;
    int changeBoost = drcStatus->boost < 0 ? 0 : 1;

    nBits += FDKwriteBits(hBs, 1, 1); /* dynamicRangeControlParameterInterfacePresent */

    nBits += FDKwriteBits(hBs, changeCompress, 1); /* changeCompress */
    nBits += FDKwriteBits(hBs, changeBoost, 1);    /* changeBoost */

    if (changeCompress) {
      SHORT compress = 0x100 - (drcStatus->compress >> 6);
      if (compress > 0xFF) compress = 0xFF;
      nBits += FDKwriteBits(hBs, compress, 8); /* compress */
    }
    if (changeBoost) {
      SHORT boost = 0x100 - (drcStatus->boost >> 6);
      if (boost > 0xFF) boost = 0xFF;
      nBits += FDKwriteBits(hBs, boost, 8); /* boost */
    }

    nBits += FDKwriteBits(hBs, 0, 1); /* changeDrcCharacteristicTarget  */
  }

  nBits += FDKwriteBits(hBs, 0, 1); /* uniDrcInterfaceExtensionPresent */

  if (nBits & 7) {
    nBits += FDKwriteBits(hBs, 0, 8 - (nBits & 7));
  }

  return nBits >> 3;
}

/* update MHAS */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_UpdateMHAS(HANDLE_MPEGH_UI_MANAGER self, UCHAR* mhasBuffer,
                                              UINT mhasBufferLength, UINT* mhasLength) {
  FDK_BITSTREAM bs;
  HANDLE_FDK_BITSTREAM hBs = &bs;
  USER_INTERACTIVITY_STATUS uiStatus;
  UI_DRC_LOUDNESS_STATUS drcStatus;
  UI_MANAGER_ERROR err;
  UINT uiPacketLength = 0, drcPacketLength = 0, nBytes = 0;
  UCHAR uiStatusChanged, drcStatusChanged;
  UCHAR insertUiPacket = 0, insertDrcPacket = 0;

  if (!self->isActive) return MPEGH_UI_NOT_ALLOWED;

  if (self->insertOffset == (UINT)-1) return MPEGH_UI_INVALID_STATE;

  /* check for status change */
  UI_Manager_GetStatusChanged(self->hUiManager, &uiStatusChanged, &drcStatusChanged);

  insertUiPacket = self->configFound || uiStatusChanged;
  insertDrcPacket = self->configFound || drcStatusChanged;

  /* without ASI do not insert UI packets */
  if (UI_Manager_GetAsiPointer(self->hUiManager)->numGroups == 0) insertUiPacket = 0;

  if (insertUiPacket) {
    /* get UI status */
    err = UI_Manager_GetInteractivityStatus(self->hUiManager, &uiStatus, NULL);
    if (err != UI_MANAGER_OK) return (MPEGH_UI_ERROR)err;

    /* get UI packet length */
    uiPacketLength = writeUiPacket(NULL, &uiStatus);
    nBytes += uiPacketLength;

    /* get header length*/
    nBytes += writePacketHeader(NULL, (UINT)MHA_PACTYP_USERINTERACTION, self->mainStreamLabel,
                                uiPacketLength);
  }

  if (insertDrcPacket) {
    /* get DRC/loudness status */
    err = UI_Manager_GetDrcLoudnessStatus(self->hUiManager, &drcStatus);
    if (err != UI_MANAGER_OK) return (MPEGH_UI_ERROR)err;

    /* get DRC packet length */
    drcPacketLength = writeDrcPacket(NULL, &drcStatus);
    nBytes += drcPacketLength;

    /* get header length */
    nBytes += writePacketHeader(NULL, (UINT)MHA_PACTYP_LOUDNESS_DRC, self->mainStreamLabel,
                                drcPacketLength);
  }

  if (!nBytes) return MPEGH_UI_OK;

  /* check buffer size */
  if (*mhasLength + nBytes > mhasBufferLength) return MPEGH_UI_BUFFER_TOO_SMALL;

  /* prepare writing of packets */
  FDKmemmove(mhasBuffer + self->insertOffset + nBytes, mhasBuffer + self->insertOffset,
             *mhasLength - self->insertOffset);
  FDKinitBitStream(hBs, mhasBuffer + self->insertOffset, nextPow2(nBytes), 0, BS_WRITER);

  if (insertUiPacket) {
    /* insert UI packet */
    writePacketHeader(hBs, (UINT)MHA_PACTYP_USERINTERACTION, self->mainStreamLabel, uiPacketLength);
    writeUiPacket(hBs, &uiStatus);
  }

  if (insertDrcPacket) {
    /* insert DRC packet */
    writePacketHeader(hBs, (UINT)MHA_PACTYP_LOUDNESS_DRC, self->mainStreamLabel, drcPacketLength);
    writeDrcPacket(hBs, &drcStatus);
  }

  FDKsyncCache(hBs);

  *mhasLength += nBytes;

  return MPEGH_UI_OK;
}

/* set persistence memory */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_SetPersistenceMemory(HANDLE_MPEGH_UI_MANAGER self,
                                                        void* persistenceMemoryBlock,
                                                        USHORT persistenceMemorySize) {
  UI_MANAGER_ERROR err;

  err = UI_Manager_SetPersistenceMemory(self->hUiManager, persistenceMemoryBlock,
                                        persistenceMemorySize);

  if (err == UI_MANAGER_OK) return MPEGH_UI_OK;
  if (err == UI_MANAGER_BUFFER_TOO_SMALL) return MPEGH_UI_BUFFER_TOO_SMALL;
  if (err == UI_MANAGER_OK_BUT_NO_VALID_DATA) return MPEGH_UI_OK_BUT_NO_VALID_DATA;
  return MPEGH_UI_INVALID_PARAM;
}

/* get persistence memory */
LINKSPEC_H MPEGH_UI_ERROR mpegh_UI_GetPersistenceMemory(HANDLE_MPEGH_UI_MANAGER self,
                                                        void** persistenceMemoryBlock,
                                                        USHORT* persistenceMemorySize) {
  UI_Manager_GetPersistenceMemory(self->hUiManager, persistenceMemoryBlock, persistenceMemorySize);

  return MPEGH_UI_OK;
}
