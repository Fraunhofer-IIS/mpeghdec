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

/******************* MPEG transport format decoder library *********************

   Author(s):   Manuel Jander

   Description: MPEG Transport decoder

*******************************************************************************/

#include "tpdec_lib.h"

/* library version */
#include "tp_version.h"

#include "tp_data.h"

#include "FDK_crc.h"

#define MODULE_NAME "transportDec"

typedef union {
  struct {
    INT bufferFullness;
    UINT endOfFrame;
    UINT mhasLabels[TPDEC_MAX_TRACKS];
    UINT flags[TPDEC_MAX_TRACKS];
  } mhas;
} transportdec_parser_t;

#define MHAS_CONFIG_PRESENT 0x001
#define MHAS_UI_PRESENT 0x002

struct TRANSPORTDEC {
  TRANSPORT_TYPE transportFmt; /*!< MPEG4 transportDec type. */

  CSTpCallBacks callbacks; /*!< Struct holding callback and its data */

  FDK_BITSTREAM bitStream[TPDEC_MAX_LAYERS]; /* Bitstream reader */
  UCHAR* bsBuffer;                           /* Internal bitstreamd data buffer */

  transportdec_parser_t parser; /* Format specific parser structs. */

  CSAudioSpecificConfig
      asc[CSAUDIOSPECIFICCONFIG_SIZE]; /* Audio specific config from the last config found.
                                       One additional CSAudioSpecificConfig is used temporarily
                                       for parsing. */
  CCtrlCFGChange ctrlCFGChange[TPDEC_MAX_TRACKS]; /* Controls config change */

  AUDIO_SCENE_INFO* pASI;

  UINT globalFramePos;                     /* Global transport frame reference bit position. */
  UINT accessUnitAnchor[TPDEC_MAX_LAYERS]; /* Current access unit start bit position. */
  INT auLength[TPDEC_MAX_LAYERS];          /* Length of current access unit. */
  INT numberOfRawDataBlocks;    /* Current number of raw data blocks contained remaining from the
                                   current transport frame. */
  UINT avgBitRate;              /* Average bit rate used for frame loss estimation. */
  UINT lastValidBufferFullness; /* Last valid buffer fullness value for frame loss estimation */
  INT remainder;                /* Reminder in division during lost access unit estimation. */
  INT missingAccessUnits;       /* Estimated missing access units. */
  UINT burstPeriod;             /* Data burst period in mili seconds. */
  UINT holdOffFrames; /* Amount of frames that were already hold off due to buffer fullness
                         condition not being met. */
  UINT flags;         /* Flags. */
  INT targetLayout;   /* CICP target layout. */
  INT* pLoudnessInfoSetPosition; /* Reference and start position (bits) and length (bytes) of
                                    loudnessInfoSet within mpegh3daConfig.  */
};

/* Flag bitmasks for "flags" member of struct TRANSPORTDEC */
#define TPDEC_SYNCOK 1
#define TPDEC_MINIMIZE_DELAY 2
#define TPDEC_IGNORE_BUFFERFULLNESS 4
#define TPDEC_EARLY_CONFIG 8
#define TPDEC_LOST_FRAMES_PENDING 16
#define TPDEC_CONFIG_FOUND 32
#define TPDEC_USE_ELEM_SKIPPING 64

/* force config/content change */
#define TPDEC_FORCE_CONFIG_CHANGE 1
#define TPDEC_FORCE_CONTENT_CHANGE 2

/* skip packet */
#define TPDEC_SKIP_PACKET 1

C_ALLOC_MEM(Ram_TransportDecoder, struct TRANSPORTDEC, 1)
C_ALLOC_MEM(Ram_TransportDecoderBuffer, UCHAR, (65536 * 1))

static void disregardLoudnessChange(HANDLE_TRANSPORTDEC hTp, HANDLE_FDK_BITSTREAM hBs,
                                    int MHASPacketLength);

HANDLE_TRANSPORTDEC transportDec_Open(const TRANSPORT_TYPE transportFmt, const UINT flags,
                                      const UINT nrOfLayers) {
  HANDLE_TRANSPORTDEC hInput;

  hInput = GetRam_TransportDecoder(0);
  if (hInput == NULL) {
    return NULL;
  }

  /* Init transportDec struct. */
  hInput->transportFmt = transportFmt;

  switch (transportFmt) {
    case TT_MP4_RAW:
    case TT_MHA_RAW:
    case TT_MHAS:
    case TT_MHAS_PACKETIZED:
      break;

    default:
      FreeRam_TransportDecoder(&hInput);
      hInput = NULL;
      break;
  }

  if (hInput != NULL) {
    /* Create bitstream */
    {
      hInput->bsBuffer = GetRam_TransportDecoderBuffer(0);
      if (hInput->bsBuffer == NULL) {
        transportDec_Close(&hInput);
        return NULL;
      }
      if (nrOfLayers > TPDEC_MAX_LAYERS) {
        transportDec_Close(&hInput);
        return NULL;
      }
      for (UINT i = 0; i < nrOfLayers; i++) {
        FDKinitBitStream(&hInput->bitStream[i], hInput->bsBuffer, (65536 * 1), 0, BS_READER);
      }
    }
    hInput->burstPeriod = 0;
  }

  return hInput;
}

TRANSPORTDEC_ERROR transportDec_OutOfBandConfig(HANDLE_TRANSPORTDEC hTp, UCHAR* conf,
                                                const UINT length, UINT layer) {
  int i;

  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;

  FDK_BITSTREAM bs;
  HANDLE_FDK_BITSTREAM hBs = &bs;

  int fConfigFound = 0;

  UCHAR configChanged = 0;
  UCHAR configMode = AC_CM_DET_CFG_CHANGE;

  UCHAR tmpConf[1024] = {0};
  if (length > 1024) {
    return TRANSPORTDEC_UNSUPPORTED_FORMAT;
  }
  FDKmemcpy(tmpConf, conf, length);
  FDKinitBitStream(hBs, tmpConf, 1024, length << 3, BS_READER);

  for (i = 0; i < 2; i++) {
    if (i > 0) {
      FDKpushBack(hBs, (INT)length * 8 - FDKgetValidBits(hBs));
      configMode = AC_CM_ALLOC_MEM;
    }

    /* config transport decoder */
    switch (hTp->transportFmt) {
      case TT_MHA_RAW:
      case TT_MHAS:
      case TT_MHAS_PACKETIZED:
        fConfigFound = 1;
        err = Mpegh3daConfig_Parse(&hTp->asc[TPDEC_MAX_TRACKS], hTp->pASI, hBs, 1, &hTp->callbacks,
                                   configMode, configChanged, hTp->targetLayout, 0,
                                   hTp->pLoudnessInfoSetPosition);
        if (err == TRANSPORTDEC_OK) {
          hTp->asc[layer] = hTp->asc[TPDEC_MAX_TRACKS];
          int errC = hTp->callbacks.cbUpdateConfig(hTp->callbacks.cbUpdateConfigData,
                                                   &hTp->asc[layer], hTp->asc[layer].configMode,
                                                   &hTp->asc[layer].AacConfigChanged);
          if (errC != 0) {
            err = TRANSPORTDEC_PARSE_ERROR;
          }
        }

        if (configMode & AC_CM_DET_CFG_CHANGE) {
          if (FDKmemcmp(hTp->ctrlCFGChange[layer].Mpegh3daConfig, conf, length) != 0) {
            hTp->asc[layer].AacConfigChanged |= 1;
          }
        }

        hTp->ctrlCFGChange[layer].Mpegh3daConfigLen = length;
        FDKmemcpy(hTp->ctrlCFGChange[layer].Mpegh3daConfig, conf, length);
        break;
      default:
        err = TRANSPORTDEC_UNSUPPORTED_FORMAT;
        break;
    }

    if (err == TRANSPORTDEC_OK) {
      if ((i == 0) && (hTp->asc[layer].AacConfigChanged || hTp->asc[layer].SbrConfigChanged ||
                       hTp->asc[layer].SacConfigChanged)) {
        int errC;

        configChanged = 1;
        errC = hTp->callbacks.cbFreeMem(hTp->callbacks.cbFreeMemData, &hTp->asc[layer]);
        if (errC != 0) {
          err = TRANSPORTDEC_PARSE_ERROR;
        }
      }
    }

    /* if an error is detected terminate config parsing to avoid that an invalid config is accepted
     * in the second pass */
    if (err != TRANSPORTDEC_OK) {
      break;
    }
  }

  if (err == TRANSPORTDEC_OK && fConfigFound) {
    hTp->flags |= TPDEC_CONFIG_FOUND;
  }

  return err;
}

TRANSPORTDEC_ERROR transportDec_InBandConfig(HANDLE_TRANSPORTDEC hTp, UCHAR* newConfig,
                                             const UINT newConfigLength, const UCHAR buildUpStatus,
                                             UCHAR* configChanged, UINT layer) {
  int errC;
  FDK_BITSTREAM bs;
  HANDLE_FDK_BITSTREAM hBs = &bs;
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;
  int fConfigFound = 0;
  UCHAR configMode = AC_CM_ALLOC_MEM;

  FDK_ASSERT((hTp->asc->m_aot == AOT_MPEGH3DA) || (hTp->asc->m_aot == AOT_USAC));

  FDKinitBitStream(hBs, newConfig, TP_USAC_MAX_CONFIG_LEN, newConfigLength << 3, BS_READER);

  if ((hTp->ctrlCFGChange[layer].flushStatus == TPDEC_FLUSH_OFF) &&
      (hTp->ctrlCFGChange[layer].buildUpStatus != TPDEC_MPEGH_BUILD_UP_IDLE_IN_BAND)) {
    {
      if (hTp->asc->m_aot == AOT_MPEGH3DA) {
        if (layer == 0) {
          disregardLoudnessChange(hTp, hBs, hTp->ctrlCFGChange[layer].Mpegh3daConfigLen);
        }
        if (hTp->ctrlCFGChange[layer].Mpegh3daConfigLen == newConfigLength) {
          if (0 ==
              FDKmemcmp(newConfig, hTp->ctrlCFGChange[layer].Mpegh3daConfig, newConfigLength)) {
            *configChanged = 0;
            return err;
          }
        }
      }
    }
  }

  {
    if ((hTp->ctrlCFGChange[layer].flushStatus == TPDEC_FLUSH_OFF) &&
        (hTp->ctrlCFGChange[layer].buildUpStatus != TPDEC_MPEGH_BUILD_UP_IDLE_IN_BAND)) {
      hTp->ctrlCFGChange[layer].flushCnt = 0;
      hTp->ctrlCFGChange[layer].buildUpCnt = 0;
      hTp->ctrlCFGChange[layer].buildUpStatus = TPDEC_BUILD_UP_OFF;
      if (hTp->asc->m_aot == AOT_MPEGH3DA) {
        hTp->ctrlCFGChange[layer].flushStatus = TPDEC_MPEGH_DASH_IPF_ATSC_FLUSH_ON;

        if (hTp->numberOfRawDataBlocks != 0) {
          err = TRANSPORTDEC_PARSE_ERROR;
        }
        hTp->numberOfRawDataBlocks = 1;

        /* Skip truncation callback if explicit and implicit config are different. */
        if (buildUpStatus != TPDEC_MPEGH_BUILD_UP_ON) {
          if (!hTp->ctrlCFGChange[layer].truncationPresent) {
            if (hTp->callbacks.cbTruncationMsg) {
              hTp->callbacks.cbTruncationMsg(hTp->callbacks.cbTruncationMsgData, 0, 2);
            }
            hTp->ctrlCFGChange[layer].truncationPresent = 1;
          }
        } else {
          /* Skip flushing if explicit and implicit config are different. Continue with idle status
           */
          hTp->ctrlCFGChange[layer].flushCnt = TPDEC_MPEGH_NUM_CONFIG_CHANGE_FRAMES;
        }
      }
    }

    if ((hTp->ctrlCFGChange[layer].flushStatus == TPDEC_MPEGH_DASH_IPF_ATSC_FLUSH_ON) ||
        (hTp->ctrlCFGChange[layer].flushStatus == TPDEC_USAC_DASH_IPF_FLUSH_ON)) {
      SCHAR counter = 0;
      if (hTp->asc->m_aot == AOT_MPEGH3DA) {
        counter = TPDEC_MPEGH_NUM_CONFIG_CHANGE_FRAMES;
      }
      if (hTp->ctrlCFGChange[layer].flushCnt >= counter) {
        hTp->ctrlCFGChange[layer].flushCnt = 0;
        hTp->ctrlCFGChange[layer].flushStatus = TPDEC_FLUSH_OFF;
        hTp->ctrlCFGChange[layer].forceCfgChange = 0;
        if (hTp->asc->m_aot == AOT_MPEGH3DA) {
          hTp->ctrlCFGChange[layer].buildUpCnt = TPDEC_MPEGH_NUM_CONFIG_CHANGE_FRAMES;
          hTp->ctrlCFGChange[layer].buildUpStatus = TPDEC_MPEGH_BUILD_UP_IDLE_IN_BAND;
          hTp->ctrlCFGChange[layer].truncationPresent = 0;
          if (hTp->numberOfRawDataBlocks != 1) {
            err = TRANSPORTDEC_PARSE_ERROR;
          }
        }
      }

      /* Activate flush mode. After that continue with build up mode in core */
      if (hTp->callbacks.cbCtrlCFGChange &&
          (hTp->callbacks.cbCtrlCFGChange(hTp->callbacks.cbCtrlCFGChangeData,
                                          &hTp->ctrlCFGChange[layer]) != 0)) {
        err = TRANSPORTDEC_PARSE_ERROR;
      }

      if ((hTp->ctrlCFGChange[layer].flushStatus == TPDEC_MPEGH_DASH_IPF_ATSC_FLUSH_ON) ||
          (hTp->ctrlCFGChange[layer].flushStatus == TPDEC_USAC_DASH_IPF_FLUSH_ON)) {
        hTp->ctrlCFGChange[layer].flushCnt++;
        return err;
      }
    } else if (hTp->ctrlCFGChange[layer].buildUpStatus == TPDEC_MPEGH_BUILD_UP_IDLE_IN_BAND) {
      hTp->ctrlCFGChange[layer].buildUpStatus = TPDEC_MPEGH_BUILD_UP_ON_IN_BAND;
      if (hTp->callbacks.cbCtrlCFGChange &&
          (hTp->callbacks.cbCtrlCFGChange(hTp->callbacks.cbCtrlCFGChangeData,
                                          &hTp->ctrlCFGChange[layer]) != 0)) {
        err = TRANSPORTDEC_PARSE_ERROR;
      }

      hTp->numberOfRawDataBlocks = 0;
      *configChanged = 1;
      return err;
    }

    if (hTp->asc->m_aot == AOT_MPEGH3DA) {
      *configChanged = 1;
      /* free memory for config change */
      errC = hTp->callbacks.cbFreeMem(hTp->callbacks.cbFreeMemData, &hTp->asc[layer]);
      if (errC != 0) {
        err = TRANSPORTDEC_PARSE_ERROR;
      }

      /* config transport decoder */
      if (err == TRANSPORTDEC_OK) {
        switch (hTp->transportFmt) {
          case TT_MHA_RAW:
          case TT_MHAS:
          case TT_MHAS_PACKETIZED:
            fConfigFound = 1;
            err = Mpegh3daConfig_Parse(&hTp->asc[TPDEC_MAX_TRACKS], hTp->pASI, hBs, 1,
                                       &hTp->callbacks, configMode, *configChanged,
                                       hTp->targetLayout, 0, hTp->pLoudnessInfoSetPosition);
            if (err == TRANSPORTDEC_OK) {
              hTp->asc[layer] = hTp->asc[TPDEC_MAX_TRACKS];
              errC = hTp->callbacks.cbUpdateConfig(hTp->callbacks.cbUpdateConfigData,
                                                   &hTp->asc[layer], hTp->asc[layer].configMode,
                                                   &hTp->asc[layer].AacConfigChanged);
              if (errC != 0) {
                err = TRANSPORTDEC_PARSE_ERROR;
              }
            }
            break;
          default:
            break;
        }
      }
    }

    /* save new config */
    if (err == TRANSPORTDEC_OK) {
      if (hTp->asc->m_aot == AOT_MPEGH3DA) {
        hTp->ctrlCFGChange[layer].Mpegh3daConfigLen = newConfigLength;
        FDKmemcpy(hTp->ctrlCFGChange[layer].Mpegh3daConfig, newConfig, newConfigLength);
      }
    } else {
      hTp->numberOfRawDataBlocks = 0;

      /* If parsing error while config found, clear ctrlCFGChange-struct */
      hTp->ctrlCFGChange[layer].Mpegh3daConfigLen = 1;
      hTp->ctrlCFGChange[layer].flushCnt = 0;
      hTp->ctrlCFGChange[layer].flushStatus = TPDEC_FLUSH_OFF;
      hTp->ctrlCFGChange[layer].buildUpCnt = 0;
      hTp->ctrlCFGChange[layer].buildUpStatus = TPDEC_BUILD_UP_OFF;
      hTp->ctrlCFGChange[layer].cfgChanged = 0;
      hTp->ctrlCFGChange[layer].contentChanged = 0;
      hTp->ctrlCFGChange[layer].forceCfgChange = 0;
      hTp->ctrlCFGChange[layer].truncationPresent = 0;

      if (hTp->callbacks.cbCtrlCFGChange)
        hTp->callbacks.cbCtrlCFGChange(hTp->callbacks.cbCtrlCFGChangeData,
                                       &hTp->ctrlCFGChange[layer]);
    }
  }

  if (err == TRANSPORTDEC_OK && fConfigFound) {
    hTp->flags |= TPDEC_CONFIG_FOUND;
  }

  return err;
}

int transportDec_RegisterAscCallback(HANDLE_TRANSPORTDEC hTpDec,
                                     const cbUpdateConfig_t cbUpdateConfig, void* user_data) {
  if (hTpDec == NULL) {
    return -1;
  }
  hTpDec->callbacks.cbUpdateConfig = cbUpdateConfig;
  hTpDec->callbacks.cbUpdateConfigData = user_data;
  return 0;
}

int transportDec_RegisterDecodeFrameCallback(HANDLE_TRANSPORTDEC hTpDec,
                                             const cbDecodeFrame_t cbDecodeFrame, void* user_data) {
  if (hTpDec == NULL) {
    return -1;
  }
  hTpDec->callbacks.cbDecodeFrame = cbDecodeFrame;
  hTpDec->callbacks.cbDecodeFrameData = user_data;
  return 0;
}

int transportDec_RegisterFreeMemCallback(HANDLE_TRANSPORTDEC hTpDec, const cbFreeMem_t cbFreeMem,
                                         void* user_data) {
  if (hTpDec == NULL) {
    return -1;
  }
  hTpDec->callbacks.cbFreeMem = cbFreeMem;
  hTpDec->callbacks.cbFreeMemData = user_data;
  return 0;
}

int transportDec_RegisterCtrlCFGChangeCallback(HANDLE_TRANSPORTDEC hTpDec,
                                               const cbCtrlCFGChange_t cbCtrlCFGChange,
                                               void* user_data) {
  if (hTpDec == NULL) {
    return -1;
  }
  hTpDec->callbacks.cbCtrlCFGChange = cbCtrlCFGChange;
  hTpDec->callbacks.cbCtrlCFGChangeData = user_data;
  return 0;
}

int transportDec_RegisterTruncationMsgCallback(HANDLE_TRANSPORTDEC hTpDec,
                                               const cbTruncationMsg_t cbTruncationMsg,
                                               void* user_data) {
  if (hTpDec == NULL) {
    return -1;
  }
  hTpDec->callbacks.cbTruncationMsg = cbTruncationMsg;
  hTpDec->callbacks.cbTruncationMsgData = user_data;
  return 0;
}

int transportDec_RegisterUniDrcConfigCallback(HANDLE_TRANSPORTDEC hTpDec, const cbUniDrc_t cbUniDrc,
                                              void* user_data, INT* pLoudnessInfoSetPosition) {
  if (hTpDec == NULL) {
    return -1;
  }

  hTpDec->callbacks.cbUniDrc = cbUniDrc;
  hTpDec->callbacks.cbUniDrcData = user_data;

  hTpDec->pLoudnessInfoSetPosition = pLoudnessInfoSetPosition;
  return 0;
}

int transportDec_RegisterParseDmxMatrixCallback(HANDLE_TRANSPORTDEC hTpDec,
                                                const cbParseDmxMatrix_t cbParseDmxMatrix,
                                                void* user_data) {
  if (hTpDec == NULL) {
    return -1;
  }

  hTpDec->callbacks.cbParseDmxMatrix = cbParseDmxMatrix;
  hTpDec->callbacks.cbParseDmxMatrixData = user_data;
  return 0;
}

int transportDec_RegisterUserInteractCallback(HANDLE_TRANSPORTDEC hTpDec,
                                              const cbUserInteract_t cbUserInteract,
                                              void* user_data) {
  if (hTpDec == NULL) {
    return -1;
  }

  hTpDec->callbacks.cbUserInteract = cbUserInteract;
  hTpDec->callbacks.cbUserInteractData = user_data;
  return 0;
}

int transportDec_RegisterEarconSetBSCallBack(HANDLE_TRANSPORTDEC hTpDec,
                                             const cbEarconBS_t cbEarconBS, void* user_data) {
  hTpDec->callbacks.cbEarconBS = cbEarconBS;
  hTpDec->callbacks.cbEarconBSData = user_data;
  return 0;
}

int transportDec_RegisterEarconConfigCallBack(HANDLE_TRANSPORTDEC hTpDec,
                                              const cbEarconConfig_t cbEarconConfig,
                                              void* user_data) {
  hTpDec->callbacks.cbEarconConfig = cbEarconConfig;
  hTpDec->callbacks.cbEarconConfigData = user_data;
  return 0;
}

int transportDec_RegisterEarconInfoCallBack(HANDLE_TRANSPORTDEC hTpDec,
                                            const cbEarconInfo_t cbEarconInfo, void* user_data) {
  hTpDec->callbacks.cbEarconInfo = cbEarconInfo;
  hTpDec->callbacks.cbEarconInfoData = user_data;
  return 0;
}

int transportDec_SetAsiParsing(HANDLE_TRANSPORTDEC hTpDec, AUDIO_SCENE_INFO* pASI) {
  hTpDec->pASI = pASI;

  return 0;
}

TRANSPORTDEC_ERROR transportDec_FillData(const HANDLE_TRANSPORTDEC hTp, const UCHAR* pBuffer,
                                         const UINT bufferSize, UINT* pBytesValid,
                                         const INT layer) {
  HANDLE_FDK_BITSTREAM hBs;

  if ((hTp == NULL) || (layer >= TPDEC_MAX_LAYERS)) {
    return TRANSPORTDEC_INVALID_PARAMETER;
  }

  /* set bitbuffer shortcut */
  hBs = &hTp->bitStream[layer];

  if (TT_IS_PACKET(hTp->transportFmt)) {
    if (hTp->numberOfRawDataBlocks == 0) {
      FDKresetBitbuffer(hBs);
      FDKfeedBuffer(hBs, pBuffer, bufferSize, pBytesValid);
      if (*pBytesValid != 0) {
        return TRANSPORTDEC_TOO_MANY_BITS;
      }
    }
  } else {
    /* ... else feed bitbuffer with new stream data (append). */

    if (*pBytesValid == 0) {
      /* nothing to do */
      return TRANSPORTDEC_OK;
    } else {
      const int bytesValid = *pBytesValid;
      FDKfeedBuffer(hBs, pBuffer, bufferSize, pBytesValid);

      if (hTp->numberOfRawDataBlocks > 0) {
        hTp->globalFramePos += (bytesValid - *pBytesValid) * 8;
        if (hTp->transportFmt == TT_MHAS) {
          hTp->parser.mhas.endOfFrame += (bytesValid - *pBytesValid) * 8;
        }
        hTp->accessUnitAnchor[layer] = FDKgetValidBits(hBs);
      }
    }
  }

  return TRANSPORTDEC_OK;
}

HANDLE_FDK_BITSTREAM transportDec_GetBitstream(const HANDLE_TRANSPORTDEC hTp, const UINT layer) {
  return &hTp->bitStream[layer];
}

TRANSPORT_TYPE transportDec_GetFormat(const HANDLE_TRANSPORTDEC hTp) {
  return hTp->transportFmt;
}

/**
 * \brief adjust bit stream position and the end of an access unit.
 * \param hTp transport decoder handle.
 * \return error code.
 */
static TRANSPORTDEC_ERROR transportDec_AdjustEndOfAccessUnit(HANDLE_TRANSPORTDEC hTp) {
  HANDLE_FDK_BITSTREAM hBs = &hTp->bitStream[0];
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;

  switch (hTp->transportFmt) {
    case TT_MHAS:
    case TT_MHAS_PACKETIZED: {
      int offset;

      offset = FDKgetValidBits(hBs) - (INT)hTp->parser.mhas.endOfFrame;
      if (offset != 0) {
        FDKpushBiDirectional(hBs, offset);
        /* err = TRANSPORTDEC_PARSE_ERROR; */
      }
    } break;
    default:
      break;
  }

  return err;
}

static void disregardLoudnessChange(HANDLE_TRANSPORTDEC hTp, HANDLE_FDK_BITSTREAM hBs,
                                    int MHASPacketLength) {
  /* enable loudness change on the fly (without config change) if only the values in loudnessInfoSet
   * change */
  int loudnessStartBit = hTp->pLoudnessInfoSetPosition[0] - hTp->pLoudnessInfoSetPosition[1];
  int loudnessStartByte = loudnessStartBit >> 3;
  int loudnessPayloadLength = hTp->pLoudnessInfoSetPosition[2];
  UINT usacConfigExtType;
  int usacConfigExtLength, startPos, i;
  int mhasSubstream = 0;
  UCHAR this_byte, bitmask;

  if ((loudnessStartBit <= 0) || (loudnessPayloadLength <= 0) ||
      ((loudnessStartByte + loudnessPayloadLength) > MHASPacketLength))
    return;

  startPos = FDKgetValidBits(hBs);
  FDKpushFor(hBs, loudnessStartBit);
  usacConfigExtType = escapedValue(hBs, 4, 8, 16);
  usacConfigExtLength = (int)escapedValue(hBs, 4, 8, 16);
  if ((usacConfigExtType == ID_CONFIG_EXT_LOUDNESS_INFO) &&
      (usacConfigExtLength == (int)hTp->pLoudnessInfoSetPosition[2])) {
    /* disregard loudnessInfoSet in comparison by replacing it in the reference in advance */
    FDKpushBack(hBs, startPos - FDKgetValidBits(hBs));
    FDKpushFor(hBs, loudnessStartByte << 3);

    this_byte = FDKreadBits(hBs, 8);
    bitmask = (1 << (loudnessStartBit & 0x7)) - 1;
    hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfig[loudnessStartByte] &= ~bitmask;
    hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfig[loudnessStartByte] |= (this_byte & bitmask);
    for (i = 1; i < usacConfigExtLength + 1; i++) {
      hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfig[loudnessStartByte + i] =
          (UCHAR)FDKreadBits(hBs, 8);
    }
    this_byte = FDKreadBits(hBs, 8);
    bitmask = ~bitmask;
    hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfig[loudnessStartByte + i] &= ~bitmask;
    hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfig[loudnessStartByte + i] |=
        (this_byte & bitmask);
  }
  FDKpushBack(hBs, startPos - FDKgetValidBits(hBs));
}

static TRANSPORTDEC_ERROR transportDec_readHeader(HANDLE_TRANSPORTDEC hTp, HANDLE_FDK_BITSTREAM hBs,
                                                  int syncLength, int ignoreBufferFullness,
                                                  int* pRawDataBlockLength,
                                                  int* pfTraverseMoreFrames,
                                                  int* pSyncLayerFrameBits, int* pfConfigFound,
                                                  int* pHeaderBits) {
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;
  int rawDataBlockLength = *pRawDataBlockLength;
  int fTraverseMoreFrames = (pfTraverseMoreFrames != NULL) ? *pfTraverseMoreFrames : 0;
  int syncLayerFrameBits = (pSyncLayerFrameBits != NULL) ? *pSyncLayerFrameBits : 0;
  int fConfigFound = (pfConfigFound != NULL) ? *pfConfigFound : 0;
  int startPos;

  startPos = FDKgetValidBits(hBs);

  switch (hTp->transportFmt) {
    case TT_MHAS:
    case TT_MHAS_PACKETIZED: {
      mha_pactyp_t MHASPacketType = MHA_PACTYP_NONE, MHASPacketTypePrev;
      UINT crc = 0;
      int crcType = -1 /* -1: none, 0: 16 bit, 1: 32 bits */, nbits, mainStreamPresent = 0;
      FDK_CRCINFO crcData;
      FDK_BITSTREAM bsMainStream;
      EarconConfig earcon_config = {};
      int nbits3DAcfg = 0;
      int auBitPosBuildUp = -1;

      for (int i = 0; i < TPDEC_MAX_TRACKS; i++) {
        /* Invalidate all streams */
        hTp->auLength[i] = 0;
      }
      /* Reset secondary streams */
      for (int i = 1; i < TPDEC_MAX_TRACKS; i++) {
        FDKresetBitbuffer(&hTp->bitStream[i]);
      }

      while (1 /* && MHASPacketType != MHA_PACTYP_MPEGH3DAFRAME */) {
        UINT MHASPacketLabel;
        int MHASPacketLength, nbitsMhas, mhasSubstream = 0;

        nbitsMhas = FDKgetValidBits(hBs);
        if (nbitsMhas <= 0) {
          hTp->parser.mhas.endOfFrame = 0;
          err = TRANSPORTDEC_NOT_ENOUGH_BITS;
          break;
        }
        hTp->parser.mhas.endOfFrame = nbitsMhas;

        MHASPacketTypePrev = MHASPacketType;
        MHASPacketType = (mha_pactyp_t)escapedValue(hBs, 3, 8, 8);
        MHASPacketLabel = escapedValue(hBs, 2, 8, 32);
        MHASPacketLength = escapedValue(hBs, 11, 24, 24);

        nbits = (INT)FDKgetValidBits(hBs);

        /* sanity check if read too many bits */
        if (nbits < 0) {
          err = TRANSPORTDEC_NOT_ENOUGH_BITS;
          goto bail;
        }

        /* Packet length sanity checks */
        if (MHASPacketLength > (65536 * 1)) {
          err = TRANSPORTDEC_SYNC_ERROR;
          goto bail;
        }
        if (MHASPacketLength * 8 > nbits) {
          err = TRANSPORTDEC_NOT_ENOUGH_BITS;
          goto bail;
        }

        /* Verify CRC if present */
        if (crcType >= 0) {
          const UINT poly[2] = {0x8021, 0x04c11db7};
          const int length[2] = {16, 32};
          const UINT startv[2] = {0xffff, 0xffffffff};
          int reg;
          FDKcrcInit(&crcData, poly[crcType], startv[crcType], length[crcType]);
          reg = FDKcrcStartReg(&crcData, hBs, MHASPacketLength * 8);
          FDKpushFor(hBs, MHASPacketLength * 8);
          FDKcrcEndReg(&crcData, hBs, reg);
          if (FDKcrcGetCRC(&crcData) != crc) {
            err = TRANSPORTDEC_SYNC_ERROR;
            goto bail;
          }
          FDKpushBack(hBs, MHASPacketLength * 8);
          crcType = -1; /* Reset CRC type */
        }

        /* Redirect EARCON packets */
        if ((MHASPacketLabel == 2049) &&
            (MHASPacketType == MHA_PACTYPE_EARCON || MHASPacketType == MHA_PACTYPE_PCMCONFIG ||
             MHASPacketType == MHA_PACTYPE_PCMDATA ||
             MHASPacketType == MHA_PACTYP_AUDIOTRUNCATION)) {
          MHASPacketLabel = 0;
        }

        /* Count MHA_PACTYP_MPEGH3DACFG of secondary streams */
        if (MHASPacketLabel > 16 && MHASPacketLabel != 0) {
          /* Skip secondary streams without altering the execution path */
          MHASPacketType = MHA_PACTYP_NONE;
          MHASPacketLabel = 0;
        } else {
          mhasSubstream = 0;
        }

        /* At the end of a series of MHA_PACTYP_MPEGH3DAFRAME break to start decoding */
        if (hTp->transportFmt == TT_MHAS && MHASPacketTypePrev == MHA_PACTYP_MPEGH3DAFRAME &&
            MHASPacketLabel <= 16) {
          {
            /* skip all MHAS packets belonging to a MHA_PACTYP_MPEGH3DACFG except itself after an AU
             * that is right truncated */
            if (hTp->ctrlCFGChange[mhasSubstream].truncationPresent)
              hTp->parser.mhas.mhasLabels[mhasSubstream] = 0;
          }

          /* The following code is present twice. */
          if (hTp->ctrlCFGChange[0].buildUpStatus == TPDEC_MPEGH_BUILD_UP_ON) {
            if (auBitPosBuildUp != -1) {
              FDKpushBack(hBs, auBitPosBuildUp - FDKgetValidBits(hBs));
              goto bail;
            } else {
              /* Something went wrong during build up, reset. */
              for (int i = 0; i < TPDEC_MAX_TRACKS; i++) {
                hTp->ctrlCFGChange[i].buildUpStatus = TPDEC_MPEGH_BUILD_UP_IDLE;
              }
            }
          }
          if (mainStreamPresent) {
            /* Restore bit stream state to main stream */
            *hBs = bsMainStream;
            syncLayerFrameBits = rawDataBlockLength = hTp->auLength[0];
            break;
          }
          /* The preceding code is present twice. */
        }

        /* Process MHAS packet according to its type */
        switch (MHASPacketType) {
          case MHA_PACTYP_SYNC: {
            const int syncByte = (int)FDKreadBits(hBs, 8);
            if (syncByte != 0xA5 || MHASPacketLabel != 0) {
              err = TRANSPORTDEC_SYNC_ERROR;
              goto bail;
            }
          } break;
          case MHA_PACTYP_MPEGH3DACFG:
            if (MHASPacketLabel != 0) {
              nbits3DAcfg = nbitsMhas;
              if (hTp->ctrlCFGChange[mhasSubstream].flushStatus == TPDEC_FLUSH_OFF) {
                UCHAR startUp;
                UCHAR Mpeg3daCfgChanged;

                startUp = (hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfigLen == 0) ? 1 : 0;

                /* Check if config buffer is large enough */
                if (MHASPacketLength > TP_MPEGH3DA_MAX_CONFIG_LEN) {
                  err = TRANSPORTDEC_SYNC_ERROR;
                  goto bail;
                }

                /* Check if config has changed. Save new config. */
                if (MHASPacketLength != (int)hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfigLen) {
                  int i;
                  Mpeg3daCfgChanged = 1;
                  hTp->ctrlCFGChange[mhasSubstream].cfgChanged = 1;
                  for (i = 0; i < MHASPacketLength; i++) {
                    hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfig[i] =
                        (UCHAR)FDKreadBits(hBs, 8);
                  }
                  hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfigLen = MHASPacketLength;
                } else {
                  int i;
                  Mpeg3daCfgChanged = 0;
                  if (mhasSubstream == 0) {
                    disregardLoudnessChange(hTp, hBs, MHASPacketLength);
                  }
                  for (i = 0; i < MHASPacketLength; i++) {
                    if (hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfig[i] !=
                        ((UCHAR)FDKreadBits(hBs, 8))) {
                      Mpeg3daCfgChanged = 1;
                      hTp->ctrlCFGChange[mhasSubstream].cfgChanged = 1;
                      FDKpushBack(hBs, 8);
                      break;
                    }
                  }
                  for (; i < MHASPacketLength; i++) {
                    hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfig[i] =
                        (UCHAR)FDKreadBits(hBs, 8);
                  }
                }

                if ((hTp->parser.mhas.mhasLabels[mhasSubstream] != MHASPacketLabel) && (!startUp)) {
                  Mpeg3daCfgChanged = 1;
                  hTp->ctrlCFGChange[mhasSubstream].cfgChanged = 1;
                  hTp->parser.mhas.mhasLabels[mhasSubstream] = MHASPacketLabel;
                }

                if (Mpeg3daCfgChanged) hTp->ctrlCFGChange[mhasSubstream].contentChanged = 1;

                FDKpushBack(hBs, MHASPacketLength * 8);

                if (hTp->ctrlCFGChange[mhasSubstream].forceCfgChange) {
                  Mpeg3daCfgChanged = 1;
                  hTp->ctrlCFGChange[mhasSubstream].cfgChanged = 1;
                  if (hTp->ctrlCFGChange[mhasSubstream].forceCfgChange ==
                      TPDEC_FORCE_CONTENT_CHANGE) {
                    hTp->ctrlCFGChange[mhasSubstream].contentChanged = 1;
                  }
                  hTp->ctrlCFGChange[mhasSubstream].forceCfgChange = 0;
                }

                /* Clear ASI and downmix information in uniDrcConfig either at startUp or in case of
                   a config change after flushing. ASI and downmix information is still needed
                   during flushing. */
                if (hTp->ctrlCFGChange[mhasSubstream].contentChanged &&
                    ((Mpeg3daCfgChanged == 0) || (startUp == 1))) {
                  /* if content change clear ASI (to avoid keeping old ASI if new content does not
                   * have ASI) */
                  if (hTp->pASI) {
                    asiReset(hTp->pASI);
                  }
                  /* also clear downmix information in uniDrcConfig */
                  if (hTp->callbacks.cbUniDrc) {
                    hTp->callbacks.cbUniDrc(hTp->callbacks.cbUniDrcData, NULL, 0, 2, mhasSubstream,
                                            0, AOT_MPEGH3DA);
                  }
                }

                if ((Mpeg3daCfgChanged == 1) && (startUp == 0)) {
                  hTp->ctrlCFGChange[mhasSubstream].flushCnt = 0;
                  hTp->ctrlCFGChange[mhasSubstream].flushStatus =
                      TPDEC_MPEGH_CFG_CHANGE_ATSC_FLUSH_ON;

                  if (!hTp->ctrlCFGChange[mhasSubstream].truncationPresent) {
                    if (hTp->callbacks.cbTruncationMsg && mhasSubstream == 0) {
                      hTp->callbacks.cbTruncationMsg(hTp->callbacks.cbTruncationMsgData, 0, 2);
                    }
                    hTp->ctrlCFGChange[mhasSubstream].truncationPresent = 1;
                  }
                } else {
                  UCHAR configChanged = 0;
                  UCHAR configMode = AC_CM_DET_CFG_CHANGE;

                  err = Mpegh3daConfig_Parse(&hTp->asc[TPDEC_MAX_TRACKS], hTp->pASI, hBs, 0,
                                             &hTp->callbacks, configMode, configChanged,
                                             hTp->targetLayout, mhasSubstream,
                                             hTp->pLoudnessInfoSetPosition);
                  earcon_config.sampling_frequency = hTp->asc[TPDEC_MAX_TRACKS].m_samplingFrequency;
                  if (err == TRANSPORTDEC_OK) {
                    hTp->parser.mhas.mhasLabels[mhasSubstream] = MHASPacketLabel;
                    hTp->asc[mhasSubstream] = hTp->asc[TPDEC_MAX_TRACKS];
                    int errC = hTp->callbacks.cbUpdateConfig(hTp->callbacks.cbUpdateConfigData,
                                                             &hTp->asc[mhasSubstream], configMode,
                                                             &configChanged);
                    if (errC != 0) {
                      err = TRANSPORTDEC_SYNC_ERROR;
                      goto bail;
                    } else {
                      fConfigFound = 1;
                    }
                  } else {
                    err = TRANSPORTDEC_SYNC_ERROR;
                    goto bail;
                  }

                  if (err == TRANSPORTDEC_OK) {
                    if ((hTp->ctrlCFGChange[mhasSubstream].cfgChanged || configChanged ||
                         hTp->asc[mhasSubstream].AacConfigChanged ||
                         hTp->asc[mhasSubstream].SbrConfigChanged ||
                         hTp->asc[mhasSubstream].SacConfigChanged)) {
                      int errC;

                      hTp->ctrlCFGChange[mhasSubstream].cfgChanged = 1;
                      configChanged = 1;
                      errC = hTp->callbacks.cbFreeMem(hTp->callbacks.cbFreeMemData,
                                                      &hTp->asc[mhasSubstream]);
                      if (errC != 0) {
                        err = TRANSPORTDEC_SYNC_ERROR;
                      }
                      if (startUp != 0) {
                        hTp->ctrlCFGChange[mhasSubstream].flushStatus = TPDEC_FLUSH_OFF;
                        hTp->ctrlCFGChange[mhasSubstream].flushCnt = 0;
                        hTp->ctrlCFGChange[mhasSubstream].buildUpStatus = TPDEC_MPEGH_BUILD_UP_IDLE;
                        hTp->ctrlCFGChange[mhasSubstream].buildUpCnt =
                            TPDEC_MPEGH_NUM_CONFIG_CHANGE_FRAMES - 1;
                      }
                    } else {
                      hTp->parser.mhas.flags[mhasSubstream] |= MHAS_CONFIG_PRESENT;
                      hTp->parser.mhas.flags[mhasSubstream] &= ~MHAS_UI_PRESENT;
                      break;
                    }
                  }
                  /* FDKpushBiDirectional(hBs, 8*MHASPacketLength - (nbits2 -
                   * FDKgetValidBits(hBs))); */
                }
              }

              if (hTp->ctrlCFGChange[mhasSubstream].flushStatus ==
                  TPDEC_MPEGH_CFG_CHANGE_ATSC_FLUSH_ON) {
                FDKpushBack(hBs, nbitsMhas - FDKgetValidBits(hBs));
                /* Activate flush mode in core */
                if (hTp->callbacks.cbCtrlCFGChange(hTp->callbacks.cbCtrlCFGChangeData,
                                                   &hTp->ctrlCFGChange[mhasSubstream]) != 0) {
                  err = TRANSPORTDEC_SYNC_ERROR;
                }

                hTp->ctrlCFGChange[mhasSubstream].flushCnt++;

                if (err == TRANSPORTDEC_OK && hTp->transportFmt == TT_MHAS_PACKETIZED) {
                  /* don't reset buffer after flushing */
                  hTp->numberOfRawDataBlocks = 1;
                }

                /* Deactivate flush mode and prepare for build up mode */
                if (hTp->ctrlCFGChange[mhasSubstream].flushCnt >=
                    (TPDEC_MPEGH_NUM_CONFIG_CHANGE_FRAMES - 1)) {
                  hTp->ctrlCFGChange[mhasSubstream].flushStatus = TPDEC_FLUSH_OFF;
                  hTp->ctrlCFGChange[mhasSubstream].flushCnt = 0;
                  hTp->ctrlCFGChange[mhasSubstream].buildUpStatus = TPDEC_MPEGH_BUILD_UP_IDLE;
                  hTp->ctrlCFGChange[mhasSubstream].buildUpCnt =
                      TPDEC_MPEGH_NUM_CONFIG_CHANGE_FRAMES - 1;
                  hTp->ctrlCFGChange[mhasSubstream].forceCfgChange = 0;
                  hTp->ctrlCFGChange[mhasSubstream].truncationPresent = 0;
                }
                goto bail;
              }
              hTp->parser.mhas.flags[mhasSubstream] |= MHAS_CONFIG_PRESENT;
              hTp->parser.mhas.flags[mhasSubstream] &= ~MHAS_UI_PRESENT;
            } else {
              err = TRANSPORTDEC_SYNC_ERROR;
            }
            break;
          case MHA_PACTYP_MPEGH3DAFRAME:
            if ((MHASPacketLabel != 0) &&
                (hTp->parser.mhas.mhasLabels[mhasSubstream] == MHASPacketLabel)) {
              if (hTp->parser.mhas.flags[mhasSubstream] & MHAS_CONFIG_PRESENT) {
                hTp->asc[mhasSubstream].m_sc.m_usacConfig.uiManagerActive =
                    !(hTp->parser.mhas.flags[mhasSubstream] & MHAS_UI_PRESENT);
              }
              hTp->parser.mhas.flags[mhasSubstream] &= ~(MHAS_CONFIG_PRESENT | MHAS_UI_PRESENT);

              if (hTp->ctrlCFGChange[mhasSubstream].contentChanged) {
                hTp->ctrlCFGChange[mhasSubstream].contentChanged = 0;
              }
              if (hTp->ctrlCFGChange[mhasSubstream].cfgChanged) {
                hTp->ctrlCFGChange[mhasSubstream].cfgChanged = 0;
                UCHAR configChanged = 1;
                int errC = hTp->callbacks.cbUpdateConfig(hTp->callbacks.cbUpdateConfigData,
                                                         &hTp->asc[mhasSubstream], AC_CM_ALLOC_MEM,
                                                         &configChanged);
                if (errC != 0) {
                  err = TRANSPORTDEC_SYNC_ERROR;
                  goto bail;
                } else {
                  fConfigFound = 1;
                }
              }

              if (hTp->ctrlCFGChange[mhasSubstream].buildUpStatus == TPDEC_MPEGH_BUILD_UP_IDLE) {
                /* Activate build up mode in core */
                if (hTp->callbacks.cbCtrlCFGChange(hTp->callbacks.cbCtrlCFGChangeData,
                                                   &hTp->ctrlCFGChange[mhasSubstream]) != 0) {
                  err = TRANSPORTDEC_SYNC_ERROR;
                }
                hTp->ctrlCFGChange[mhasSubstream].buildUpStatus = TPDEC_MPEGH_BUILD_UP_ON;
                // FDKpushBack(hBs, nbitsMhas - FDKgetValidBits(hBs));
                if (auBitPosBuildUp == -1) {
                  auBitPosBuildUp = nbitsMhas;
                }
                if (err == TRANSPORTDEC_OK && hTp->transportFmt == TT_MHAS_PACKETIZED) {
                  /* don't reset buffer after idle process */
                  hTp->numberOfRawDataBlocks = 1;
                }
                break;
              }

              if (hTp->ctrlCFGChange[mhasSubstream].buildUpStatus == TPDEC_MPEGH_BUILD_UP_ON) {
                /* Activate build up mode in core */
                if (hTp->callbacks.cbCtrlCFGChange(hTp->callbacks.cbCtrlCFGChangeData,
                                                   &hTp->ctrlCFGChange[mhasSubstream]) != 0) {
                  err = TRANSPORTDEC_SYNC_ERROR;
                }
                hTp->ctrlCFGChange[mhasSubstream].buildUpStatus = TPDEC_BUILD_UP_OFF;
              }

              { hTp->auLength[mhasSubstream] = MHASPacketLength * 8; }
              {
                mainStreamPresent = 1;
                bsMainStream = *hBs;
              }
            } else {
              err = TRANSPORTDEC_SYNC_ERROR;
            }

            if (hTp->ctrlCFGChange[mhasSubstream].buildUpStatus == TPDEC_BUILD_UP_OFF) {
              if (hTp->callbacks.cbEarconBSData != NULL) {
                hTp->callbacks.cbEarconBS(hTp->callbacks.cbEarconBSData, hBs);
              }
            }
            break;
          case MHA_PACTYP_AUDIOSCENEINFO:
            if ((MHASPacketLabel != 0) &&
                (hTp->parser.mhas.mhasLabels[mhasSubstream] == MHASPacketLabel)) {
              if (hTp->pASI != NULL) {
                if (mae_AudioSceneInfo(hTp->pASI, hBs, (2 * 28), mhasSubstream) != 0) {
                  err = TRANSPORTDEC_SYNC_ERROR;
                } else {
                  if (checkASI(hTp->pASI,
                               hTp->asc[mhasSubstream].m_sc.m_usacConfig.bsNumSignalGroups,
                               hTp->asc[mhasSubstream].m_sc.m_usacConfig.m_signalGroupType) != 0) {
                    asiReset(hTp->pASI);
                    err = TRANSPORTDEC_SYNC_ERROR;
                  }
                }

                if ((hTp->pASI->diffFlags & ASI_DIFF_NEEDS_RESET) &&
                    (hTp->flags & TPDEC_USE_ELEM_SKIPPING) && nbits3DAcfg &&
                    (hTp->ctrlCFGChange[mhasSubstream].buildUpStatus == TPDEC_BUILD_UP_OFF)) {
                  if (hTp->ctrlCFGChange[mhasSubstream].Mpegh3daConfigLen != 0) {
                    hTp->ctrlCFGChange[mhasSubstream].cfgChanged = 1;
                    hTp->ctrlCFGChange[mhasSubstream].forceCfgChange = 0;
                    hTp->ctrlCFGChange[mhasSubstream].forceCrossfade = 1;

                    hTp->ctrlCFGChange[mhasSubstream].flushCnt = 0;
                    hTp->ctrlCFGChange[mhasSubstream].flushStatus =
                        TPDEC_MPEGH_CFG_CHANGE_ATSC_FLUSH_ON;

                    if (!hTp->ctrlCFGChange[mhasSubstream].truncationPresent) {
                      if (hTp->callbacks.cbTruncationMsg && mhasSubstream == 0) {
                        hTp->callbacks.cbTruncationMsg(hTp->callbacks.cbTruncationMsgData, 0, 2);
                      }
                      hTp->ctrlCFGChange[mhasSubstream].truncationPresent = 1;
                    }
                    FDKpushBack(hBs, nbits3DAcfg - FDKgetValidBits(hBs));
                    /* Activate flush mode in core */
                    if (hTp->callbacks.cbCtrlCFGChange(hTp->callbacks.cbCtrlCFGChangeData,
                                                       &hTp->ctrlCFGChange[mhasSubstream]) != 0) {
                      err = TRANSPORTDEC_SYNC_ERROR;
                    }

                    hTp->ctrlCFGChange[mhasSubstream].flushCnt++;

                    if (err == TRANSPORTDEC_OK && hTp->transportFmt == TT_MHAS_PACKETIZED) {
                      /* don't reset buffer after flushing */
                      hTp->numberOfRawDataBlocks = 1;
                    }

                    /* Deactivate flush mode and prepare for build up mode */
                    if (hTp->ctrlCFGChange[mhasSubstream].flushCnt >=
                        (TPDEC_MPEGH_NUM_CONFIG_CHANGE_FRAMES - 1)) {
                      hTp->ctrlCFGChange[mhasSubstream].flushStatus = TPDEC_FLUSH_OFF;
                      hTp->ctrlCFGChange[mhasSubstream].flushCnt = 0;
                      hTp->ctrlCFGChange[mhasSubstream].buildUpStatus = TPDEC_MPEGH_BUILD_UP_IDLE;
                      hTp->ctrlCFGChange[mhasSubstream].buildUpCnt =
                          TPDEC_MPEGH_NUM_CONFIG_CHANGE_FRAMES - 1;
                      hTp->ctrlCFGChange[mhasSubstream].truncationPresent = 0;
                      hTp->ctrlCFGChange[mhasSubstream].forceCrossfade = 0;
                    }
                    goto bail;
                  }
                }
              }
            } else {
              err = TRANSPORTDEC_SYNC_ERROR;
            }
            break;
          case MHA_PACTYP_MARKER:
            if (hTp->callbacks.cbUserInteract) {
              if ((MHASPacketLength > 2) && (FDKreadBits(hBs, 8) == 0xE0) &&
                  (FDKreadBits(hBs, 8) == 0x0)) {
                /* Value 0xE0 indicates an ID_Marker. This packet contains an unique content
                 * identifier. */
                if ((hTp->parser.mhas.mhasLabels[mhasSubstream] == MHASPacketLabel) &&
                    (MHA_IS_MAIN_STREAM(MHASPacketLabel))) {
                  if (hTp->callbacks.cbUserInteract(hTp->callbacks.cbUserInteractData, hBs,
                                                    MHASPacketType, MHASPacketLength) != 0) {
                    err = TRANSPORTDEC_SYNC_ERROR;
                  }
                } else {
                  err = TRANSPORTDEC_SYNC_ERROR;
                }
              }
            }
            break;
          case MHA_PACTYP_LOUDNESS_DRC:
          case MHA_PACTYP_USERINTERACTION:
            if ((hTp->parser.mhas.mhasLabels[mhasSubstream] == MHASPacketLabel) &&
                (MHA_IS_MAIN_STREAM(MHASPacketLabel))) {
              if (hTp->callbacks.cbUserInteract) {
                if (hTp->callbacks.cbUserInteract(hTp->callbacks.cbUserInteractData, hBs,
                                                  MHASPacketType, MHASPacketLength) != 0) {
                  err = TRANSPORTDEC_SYNC_ERROR;
                } else {
                  hTp->parser.mhas.flags[mhasSubstream] |= MHAS_UI_PRESENT;
                }
              }
            } else {
              err = TRANSPORTDEC_SYNC_ERROR;
            }
            break;
          case MHA_PACTYP_BUFFERINFO:
            if (FDKreadBit(hBs)) {
              hTp->parser.mhas.bufferFullness = escapedValue(hBs, 15, 24, 32);
            }
            break;
            /* Audio TruncationInfo */
          case MHA_PACTYP_AUDIOTRUNCATION:
            /* audioTruncationInfo(); */
            if ((MHASPacketLabel != 0) &&
                (hTp->parser.mhas.mhasLabels[mhasSubstream] == MHASPacketLabel)) {
              int isActive, truncFromBegin, nTruncSamples;

              isActive = FDKreadBits(hBs, 1);
              FDKreadBits(hBs, 1); /* reserved */
              truncFromBegin = FDKreadBits(hBs, 1);
              nTruncSamples = FDKreadBits(hBs, 13);
              earcon_config.m_bsMHASTruncationLength = nTruncSamples;

              /* Check that truncation length does not exceed max nominal frame length */
              if (isActive) {
                if (48000 * 1024 < ((UINT)nTruncSamples * hTp->asc->m_samplingFrequency)) {
                  err = TRANSPORTDEC_SYNC_ERROR;
                }
              }

              if (isActive && hTp->callbacks.cbTruncationMsg) {
                /* truncation callback should not be executed if a left truncation without content
                 * change or without config change occurred */
                if (!(!hTp->ctrlCFGChange[mhasSubstream].contentChanged &&
                      !hTp->ctrlCFGChange[mhasSubstream].cfgChanged && truncFromBegin)) {
                  if (hTp->callbacks.cbTruncationMsg(hTp->callbacks.cbTruncationMsgData,
                                                     nTruncSamples, truncFromBegin) != 0) {
                    err = TRANSPORTDEC_SYNC_ERROR;
                  }
                }
              }
              if (isActive && !truncFromBegin) {
                hTp->ctrlCFGChange[mhasSubstream].truncationPresent = 1;
                if ((hTp->ctrlCFGChange[mhasSubstream].flushStatus == TPDEC_FLUSH_OFF) &&
                    (hTp->ctrlCFGChange[mhasSubstream].buildUpStatus == TPDEC_BUILD_UP_OFF)) {
                  hTp->ctrlCFGChange[mhasSubstream].forceCfgChange = TPDEC_FORCE_CONTENT_CHANGE;
                }
              }
            }
            if (MHASPacketLabel == 0) {
              int isActive = FDKreadBits(hBs, 1);
              if (!isActive) err = TRANSPORTDEC_SYNC_ERROR;
              FDKreadBits(hBs, 1); /* reserved */
              int truncFromBegin = FDKreadBits(hBs, 1);
              if (truncFromBegin) err = TRANSPORTDEC_SYNC_ERROR;
              UINT nTruncSamples = FDKreadBits(hBs, 13);
              /*Store for further processing*/
              if (hTp->callbacks.cbEarconConfigData != NULL) {
                earcon_config.TruncationFlag = 1;
                earcon_config.m_bsPcmTruncationLength = nTruncSamples;
                hTp->callbacks.cbEarconConfig(hTp->callbacks.cbEarconConfigData, &earcon_config);
              }
            }
            break;
          case MHA_PACTYP_CRC32:
            crc = FDKreadBits(hBs, 32);
            /* crcType = 1; */ /* Not yet supported */
            break;
          case MHA_PACTYP_CRC16:
            crc = FDKreadBits(hBs, 16);
            crcType = 0; /* Signal CRC for next item in MHAS */
            break;
          case MHA_PACTYPE_EARCON: {
            if (hTp->callbacks.cbEarconInfo != NULL) {
              EarconInfo earcon_info;
              if (earconInfo(hBs, &earcon_info)) {
                earcon_config.EarconParseError = 1;
                break;
              }
              hTp->callbacks.cbEarconInfo(hTp->callbacks.cbEarconInfoData, &earcon_info);
            }
          } break;
          case MHA_PACTYPE_PCMCONFIG: {
            if (hTp->callbacks.cbEarconConfig != NULL) {
              if (earcon_config.EarconParseError) {
                break;
              }
              /*Parse the Config data*/
              if (pcmDataConfig(hBs, &earcon_config)) {
                earcon_config.EarconParseError = 1;
                break;
              }
              earcon_config.EarconFlag = 1;
              hTp->callbacks.cbEarconConfig(hTp->callbacks.cbEarconConfigData, &earcon_config);
            }
          } break;
          case MHA_PACTYPE_PCMDATA: {
            if (hTp->callbacks.cbEarconBS != NULL) {
              if (earcon_config.EarconParseError) {
                break;
              }
              hTp->callbacks.cbEarconBS(hTp->callbacks.cbEarconBSData, hBs);
            }
          } break;
          case MHA_PACTYP_LOUDNESS:
            /* mpegh3daLoudnessInfoSet(); */
            if ((MHASPacketLabel != 0) &&
                (hTp->parser.mhas.mhasLabels[mhasSubstream] == MHASPacketLabel)) {
              if (hTp->callbacks.cbUniDrc != NULL) {
                int errC = (TRANSPORTDEC_ERROR)hTp->callbacks.cbUniDrc(
                    hTp->callbacks.cbUniDrcData, hBs, MHASPacketLength, 1, /* loudnessInfoSet */
                    0, 0, AOT_MPEGH3DA);
                if (errC != 0) {
                  err = TRANSPORTDEC_SYNC_ERROR;
                }
              }
            } else {
              err = TRANSPORTDEC_SYNC_ERROR;
            }
            break;
          default:
            break;
        }

        if (hTp->transportFmt == TT_MHAS_PACKETIZED &&
            (MHASPacketType == MHA_PACTYP_MPEGH3DAFRAME ||
             (MHASPacketType == MHA_PACTYP_NONE && MHASPacketLabel == 0)) &&
            (nbits <= MHASPacketLength * 8)) {
          /* The following code is present twice. */
          if (hTp->ctrlCFGChange[0].buildUpStatus == TPDEC_MPEGH_BUILD_UP_ON) {
            if (auBitPosBuildUp != -1) {
              FDKpushBack(hBs, auBitPosBuildUp - FDKgetValidBits(hBs));
              goto bail;
            } else {
              /* Something went wrong during build up, reset. */
              for (int i = 0; i < TPDEC_MAX_TRACKS; i++) {
                hTp->ctrlCFGChange[i].buildUpStatus = TPDEC_MPEGH_BUILD_UP_IDLE;
              }
            }
          } else {
            /* Restore bit stream state to main stream */
            if (mainStreamPresent) {
              *hBs = bsMainStream;
              hTp->parser.mhas.endOfFrame = nbits - MHASPacketLength * 8;
              syncLayerFrameBits = rawDataBlockLength = hTp->auLength[0];
              /* skip all MHAS packets belonging to a MHA_PACTYP_MPEGH3DACFG except itself after an
               * AU that is right truncated */
              if (hTp->ctrlCFGChange[0].truncationPresent)
                hTp->parser.mhas.mhasLabels[mhasSubstream] = 0;
              break;
            }
          }
          /* The preceding code is present twice. */
        }

        /* Check amount of parsed bits. If too many bits were read assume parse error */
        {
          nbits = 8 * MHASPacketLength - (nbits - FDKgetValidBits(hBs));
          if (nbits >= 0) {
            FDKpushFor(hBs, nbits);
          } else {
            err = TRANSPORTDEC_SYNC_ERROR;
          }
        }
        if (err != TRANSPORTDEC_OK) {
          break;
        }
      } /* while (1) over all MHAS packets */
    }
      if (hTp->transportFmt == TT_MHAS_PACKETIZED) {
        /* packet finished */
        hTp->numberOfRawDataBlocks = 0;
      }
      if (err != TRANSPORTDEC_OK) {
        for (int i = 0; i < TPDEC_MAX_TRACKS; i++) {
          hTp->parser.mhas.flags[i] &= ~MHAS_CONFIG_PRESENT;
        }
      }
      break;
    default: {
      syncLayerFrameBits = 0;
    } break;
  }

bail:

  *pRawDataBlockLength = rawDataBlockLength;

  if (pHeaderBits != NULL) {
    *pHeaderBits += startPos - (INT)FDKgetValidBits(hBs);
  }

  for (int i = 0; i < TPDEC_MAX_TRACKS; i++) {
    /* If parsing error while config found, clear ctrlCFGChange-struct */
    if (hTp->ctrlCFGChange[i].cfgChanged && err != TRANSPORTDEC_OK) {
      hTp->numberOfRawDataBlocks = 0;
      hTp->ctrlCFGChange[i].Mpegh3daConfigLen = 1;
      hTp->ctrlCFGChange[i].flushCnt = 0;
      hTp->ctrlCFGChange[i].flushStatus = TPDEC_FLUSH_OFF;
      hTp->ctrlCFGChange[i].buildUpCnt = 0;
      hTp->ctrlCFGChange[i].buildUpStatus = TPDEC_BUILD_UP_OFF;
      hTp->ctrlCFGChange[i].cfgChanged = 0;
      hTp->ctrlCFGChange[i].contentChanged = 0;
      hTp->ctrlCFGChange[i].forceCfgChange = 0;
      hTp->ctrlCFGChange[i].truncationPresent = 0;

      hTp->callbacks.cbCtrlCFGChange(hTp->callbacks.cbCtrlCFGChangeData, &hTp->ctrlCFGChange[i]);
    }
  }

  if (pfConfigFound != NULL) {
    *pfConfigFound = fConfigFound;
  }

  if (pfTraverseMoreFrames != NULL) {
    *pfTraverseMoreFrames = fTraverseMoreFrames;
  }
  if (pSyncLayerFrameBits != NULL) {
    *pSyncLayerFrameBits = syncLayerFrameBits;
  }

  if (err == TRANSPORTDEC_NOT_ENOUGH_BITS && TT_IS_PACKET(hTp->transportFmt)) {
    err = TRANSPORTDEC_SYNC_ERROR;
  }

  return err;
}

/* How many bits to advance for synchronization search. */
#define TPDEC_SYNCSKIP 8

static TRANSPORTDEC_ERROR synchronization(HANDLE_TRANSPORTDEC hTp, INT* pHeaderBits) {
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK, errFirstFrame = TRANSPORTDEC_OK;
  HANDLE_FDK_BITSTREAM hBs = &hTp->bitStream[0];

  INT syncLayerFrameBits = 0; /* Length of sync layer frame (i.e. LOAS) */
  INT rawDataBlockLength = 0, rawDataBlockLengthPrevious;
  INT totalBits;
  INT headerBits = 0, headerBitsFirstFrame = 0, headerBitsPrevious;
  INT numFramesTraversed = 0, fTraverseMoreFrames, fConfigFound = (hTp->flags & TPDEC_CONFIG_FOUND),
      startPosFirstFrame = -1;
  INT numRawDataBlocksFirstFrame = 0, numRawDataBlocksPrevious, globalFramePosFirstFrame = 0,
      rawDataBlockLengthFirstFrame = 0;
  INT ignoreBufferFullness =
      hTp->flags & (TPDEC_LOST_FRAMES_PENDING | TPDEC_IGNORE_BUFFERFULLNESS | TPDEC_SYNCOK);
  UINT endTpFrameBitsPrevious = 0;

  /* Synch parameters */
  INT syncLength; /* Length of sync word in bits */
  UINT syncWord;  /* Sync word to be found */
  UINT syncMask;  /* Mask for sync word (for adding one bit, so comprising one bit less) */
  C_ALLOC_SCRATCH_START(contextFirstFrame, transportdec_parser_t, 1);

  totalBits = (INT)FDKgetValidBits(hBs);

  if (totalBits <= 0) {
    err = TRANSPORTDEC_NOT_ENOUGH_BITS;
    goto bail;
  }

  fTraverseMoreFrames =
      (hTp->flags & (TPDEC_MINIMIZE_DELAY | TPDEC_EARLY_CONFIG)) && !(hTp->flags & TPDEC_SYNCOK);

  /* Set transport specific sync parameters */
  switch (hTp->transportFmt) {
    case TT_MHAS:
      syncWord = (6 << (2 + 11 + 8)) + (1 << (8)) + 0xa5;
      syncLength = 3 + 2 + 11 + 8;
      break;
    default:
      syncWord = 0;
      syncLength = 0;
      break;
  }

  syncMask = (1 << syncLength) - 1;

  do {
    INT bitsAvail = 0;   /* Bits available in bitstream buffer    */
    INT checkLengthBits; /* Helper to check remaining bits and buffer boundaries */
    UINT synch;          /* Current sync word read from bitstream */

    fConfigFound = (hTp->flags & TPDEC_CONFIG_FOUND);
    headerBitsPrevious = headerBits;

    bitsAvail = (INT)FDKgetValidBits(hBs);

    if (hTp->numberOfRawDataBlocks == 0
        /* In case of MHAS skip sync parsing once we are in SYNCOK state because there might be no
           MHA_PACTYP_SYNC here. */
        && (((hTp->transportFmt == TT_MHAS) && !(hTp->flags & TPDEC_SYNCOK)) ||
            !(hTp->transportFmt == TT_MHAS))) {
      /* search synchword */

      FDK_ASSERT((bitsAvail % TPDEC_SYNCSKIP) == 0);

      if ((bitsAvail - syncLength) < TPDEC_SYNCSKIP) {
        err = TRANSPORTDEC_NOT_ENOUGH_BITS;
        headerBits = 0;
      } else {
        synch = FDKreadBits(hBs, syncLength);

        if (!(hTp->flags & TPDEC_SYNCOK)) {
          for (; (bitsAvail - syncLength) >= TPDEC_SYNCSKIP; bitsAvail -= TPDEC_SYNCSKIP) {
            if (synch == syncWord) {
              break;
            }
            synch = ((synch << TPDEC_SYNCSKIP) & syncMask) | FDKreadBits(hBs, TPDEC_SYNCSKIP);
          }
        }
        if (synch != syncWord) {
          /* No correct syncword found. */
          err = TRANSPORTDEC_SYNC_ERROR;
        } else {
          err = TRANSPORTDEC_OK;
        }
        headerBits = syncLength;
      }
    } else {
      headerBits = 0;
    }

    /* Save previous raw data block data */
    rawDataBlockLengthPrevious = rawDataBlockLength;
    numRawDataBlocksPrevious = hTp->numberOfRawDataBlocks;

    /* Parse transport header (raw data block granularity) */

    if (err == TRANSPORTDEC_OK) {
      err = transportDec_readHeader(hTp, hBs, syncLength, ignoreBufferFullness, &rawDataBlockLength,
                                    &fTraverseMoreFrames, &syncLayerFrameBits, &fConfigFound,
                                    &headerBits);
      if (headerBits > bitsAvail) {
        err = (headerBits < (INT)hBs->hBitBuf.bufBits) ? TRANSPORTDEC_NOT_ENOUGH_BITS
                                                       : TRANSPORTDEC_SYNC_ERROR;
      }
      if (TPDEC_IS_FATAL_ERROR(err)) {
        /* Rewind - TPDEC_SYNCSKIP, in order to look for a synch one bit ahead next time. Ensure
         * that the bit amount lands at a multiple of TPDEC_SYNCSKIP. */
        FDKpushBiDirectional(hBs, -headerBits + TPDEC_SYNCSKIP + (bitsAvail % TPDEC_SYNCSKIP));

        goto bail;
      }
    }

    bitsAvail -= headerBits;

    checkLengthBits = syncLayerFrameBits;

    /* Check if the whole frame would fit the bitstream buffer */
    if (err == TRANSPORTDEC_OK) {
      if ((checkLengthBits + headerBits) > (((65536 * 1) << 3) - 7)) {
        /* We assume that the size of the transport bit buffer has been
           chosen to meet all system requirements, thus this condition
           is considered a synchronisation error. */
        err = TRANSPORTDEC_SYNC_ERROR;
      } else {
        if (bitsAvail < checkLengthBits) {
          err = TRANSPORTDEC_NOT_ENOUGH_BITS;
        }
      }
    }

    if (err == TRANSPORTDEC_NOT_ENOUGH_BITS) {
      break;
    }

    if (err == TRANSPORTDEC_SYNC_ERROR) {
      int bits;

      /* Enforce re-sync of transport headers. */
      hTp->numberOfRawDataBlocks = 0;

      /* Ensure that the bit amount lands at a multiple of TPDEC_SYNCSKIP */
      bits = (bitsAvail + headerBits) % TPDEC_SYNCSKIP;
      /* Rewind - TPDEC_SYNCSKIP, in order to look for a synch one bit ahead next time. */
      FDKpushBiDirectional(hBs, -(headerBits - TPDEC_SYNCSKIP) + bits);
      headerBits = 0;
    }

    /* Frame traversal */
    if (fTraverseMoreFrames) {
      /* Save parser context for early config discovery "rewind all frames" */
      if ((hTp->flags & TPDEC_EARLY_CONFIG) && !(hTp->flags & TPDEC_MINIMIZE_DELAY)) {
        /* ignore buffer fullness if just traversing additional frames for ECD */
        ignoreBufferFullness = 1;

        /* Save context in order to return later */
        if (err == TRANSPORTDEC_OK && startPosFirstFrame == -1) {
          startPosFirstFrame = FDKgetValidBits(hBs);
          numRawDataBlocksFirstFrame = hTp->numberOfRawDataBlocks;
          globalFramePosFirstFrame = hTp->globalFramePos;
          rawDataBlockLengthFirstFrame = rawDataBlockLength;
          headerBitsFirstFrame = headerBits;
          errFirstFrame = err;
          FDKmemcpy(contextFirstFrame, &hTp->parser, sizeof(transportdec_parser_t));
        }

        /* Break when config was found or it is not possible anymore to find a config */
        if (startPosFirstFrame != -1 && (fConfigFound || err != TRANSPORTDEC_OK)) {
          /* In case of ECD and sync error, do not rewind anywhere. */
          if (err == TRANSPORTDEC_SYNC_ERROR) {
            startPosFirstFrame = -1;
            fConfigFound = 0;
            numFramesTraversed = 0;
          }
          break;
        }
      }

      if (err == TRANSPORTDEC_OK) {
        FDKpushFor(hBs, rawDataBlockLength);
        numFramesTraversed++;
        endTpFrameBitsPrevious = (INT)FDKgetValidBits(hBs);
        /* Ignore error here itentionally. */
        transportDec_AdjustEndOfAccessUnit(hTp);
        endTpFrameBitsPrevious -= FDKgetValidBits(hBs);
      }
    }
  } while (fTraverseMoreFrames || (err == TRANSPORTDEC_SYNC_ERROR && !(hTp->flags & TPDEC_SYNCOK)));

  /* Restore context in case of ECD frame traversal */
  if (startPosFirstFrame != -1 && (fConfigFound || err != TRANSPORTDEC_OK)) {
    FDKpushBiDirectional(hBs, FDKgetValidBits(hBs) - startPosFirstFrame);
    FDKmemcpy(&hTp->parser, contextFirstFrame, sizeof(transportdec_parser_t));
    hTp->numberOfRawDataBlocks = numRawDataBlocksFirstFrame;
    hTp->globalFramePos = globalFramePosFirstFrame;
    rawDataBlockLength = rawDataBlockLengthFirstFrame;
    headerBits = headerBitsFirstFrame;
    err = errFirstFrame;
    numFramesTraversed = 0;
  }

  /* Rewind for retry because of not enough bits */
  if (err == TRANSPORTDEC_NOT_ENOUGH_BITS) {
    FDKpushBack(hBs, headerBits);
    hTp->numberOfRawDataBlocks = numRawDataBlocksPrevious;
    headerBits = 0;
    rawDataBlockLength = rawDataBlockLengthPrevious;
  } else {
    /* reset hold off frame counter */
    hTp->holdOffFrames = 0;
  }

  /* Return to last good frame in case of frame traversal but not ECD. */
  if (numFramesTraversed > 0) {
    FDKpushBack(hBs, rawDataBlockLengthPrevious + endTpFrameBitsPrevious);
    if (err != TRANSPORTDEC_OK) {
      hTp->numberOfRawDataBlocks = numRawDataBlocksPrevious;
      headerBits = headerBitsPrevious;
      rawDataBlockLength = rawDataBlockLengthPrevious;
    }
    err = TRANSPORTDEC_OK;
  }

bail:
  hTp->auLength[0] = rawDataBlockLength;

  /* Detect pointless TRANSPORTDEC_NOT_ENOUGH_BITS error case, where the bit buffer is already full,
     or no new burst packet fits. Recover by advancing the bit buffer. */
  if ((totalBits > 0) && (TRANSPORTDEC_NOT_ENOUGH_BITS == err) &&
      (FDKgetValidBits(hBs) >=
       (INT)(((65536 * 1) * 8 - ((hTp->avgBitRate * hTp->burstPeriod) / 1000)) - 7))) {
    FDKpushFor(hBs, TPDEC_SYNCSKIP);
    err = TRANSPORTDEC_SYNC_ERROR;
  }

  if (err == TRANSPORTDEC_OK) {
    hTp->flags |= TPDEC_SYNCOK;
  }

  if (fConfigFound) {
    hTp->flags |= TPDEC_CONFIG_FOUND;
  }

  if (pHeaderBits != NULL) {
    *pHeaderBits = headerBits;
  }

  if (err == TRANSPORTDEC_SYNC_ERROR) {
    hTp->flags &= ~TPDEC_SYNCOK;
  }

  C_ALLOC_SCRATCH_END(contextFirstFrame, transportdec_parser_t, 1);

  return err;
}

/**
 * \brief Synchronize to stream and estimate the amount of missing access units due
 *        to a current synchronization error in case of constant average bit rate.
 */
static TRANSPORTDEC_ERROR transportDec_readStream(HANDLE_TRANSPORTDEC hTp, const UINT layer) {
  TRANSPORTDEC_ERROR error = TRANSPORTDEC_OK;
  INT headerBits;
  error = synchronization(hTp, &headerBits);

  hTp->missingAccessUnits = -1;

  return error;
}

/* returns error code */
TRANSPORTDEC_ERROR transportDec_ReadAccessUnit(const HANDLE_TRANSPORTDEC hTp, const UINT layer) {
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;
  HANDLE_FDK_BITSTREAM hBs;

  if (!hTp) {
    return TRANSPORTDEC_INVALID_PARAMETER;
  }

  hBs = &hTp->bitStream[layer];

  if ((INT)FDKgetValidBits(hBs) <= 0) {
    /* This is only relevant for RAW and ADIF cases.
     * For streaming formats err will get overwritten. */
    err = TRANSPORTDEC_NOT_ENOUGH_BITS;
    hTp->numberOfRawDataBlocks = 0;
  }

  switch (hTp->transportFmt) {
    case TT_MP4_RAW:
    case TT_MHA_RAW:
      /* One Access Unit was filled into buffer.
         So get the length out of the buffer. */
      hTp->auLength[layer] = FDKgetValidBits(hBs);
      hTp->flags |= TPDEC_SYNCOK;
      break;

    case TT_MHAS_PACKETIZED:
      if (err == TRANSPORTDEC_OK) {
        int fConfigFound = hTp->flags & TPDEC_CONFIG_FOUND;
        err = transportDec_readHeader(hTp, hBs, 0, 1, &hTp->auLength[layer], NULL, NULL,
                                      &fConfigFound, NULL);
        if (fConfigFound) {
          hTp->flags |= TPDEC_CONFIG_FOUND;
        }
      }
      break;

    case TT_MHAS:
      err = transportDec_readStream(hTp, layer);
      break;

    default:
      err = TRANSPORTDEC_UNSUPPORTED_FORMAT;
      break;
  }

  if (err == TRANSPORTDEC_OK) {
    hTp->accessUnitAnchor[layer] = FDKgetValidBits(hBs);
  } else {
    hTp->accessUnitAnchor[layer] = 0;
  }

  return err;
}

INT transportDec_GetAuBitsRemaining(const HANDLE_TRANSPORTDEC hTp, const UINT layer) {
  INT bits;

  if (hTp->accessUnitAnchor[layer] > 0 && hTp->auLength[layer] > 0) {
    bits = (INT)FDKgetValidBits(&hTp->bitStream[layer]);
    if (bits >= 0) {
      bits = hTp->auLength[layer] - ((INT)hTp->accessUnitAnchor[layer] - bits);
    }
  } else {
    bits = FDKgetValidBits(&hTp->bitStream[layer]);
  }

  return bits;
}

INT transportDec_GetAuBitsTotal(const HANDLE_TRANSPORTDEC hTp, const UINT layer) {
  return hTp->auLength[layer];
}

TRANSPORTDEC_ERROR transportDec_GetMissingAccessUnitCount(INT* pNAccessUnits,
                                                          HANDLE_TRANSPORTDEC hTp) {
  *pNAccessUnits = hTp->missingAccessUnits;

  return TRANSPORTDEC_OK;
}

/* Inform the transportDec layer that reading of access unit has finished. */
TRANSPORTDEC_ERROR transportDec_EndAccessUnit(HANDLE_TRANSPORTDEC hTp) {
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;

  err = transportDec_AdjustEndOfAccessUnit(hTp);

  switch (hTp->transportFmt) {
    default:
      break;
  }

  return err;
}

TRANSPORTDEC_ERROR transportDec_SetParam(const HANDLE_TRANSPORTDEC hTp, const TPDEC_PARAM param,
                                         const INT value) {
  TRANSPORTDEC_ERROR error = TRANSPORTDEC_OK;

  if (hTp == NULL) {
    return TRANSPORTDEC_INVALID_PARAMETER;
  }

  switch (param) {
    case TPDEC_PARAM_MINIMIZE_DELAY:
      if (value) {
        hTp->flags |= TPDEC_MINIMIZE_DELAY;
      } else {
        hTp->flags &= ~TPDEC_MINIMIZE_DELAY;
      }
      break;
    case TPDEC_PARAM_EARLY_CONFIG:
      if (value) {
        hTp->flags |= TPDEC_EARLY_CONFIG;
      } else {
        hTp->flags &= ~TPDEC_EARLY_CONFIG;
      }
      break;
    case TPDEC_PARAM_IGNORE_BUFFERFULLNESS:
      if (value) {
        hTp->flags |= TPDEC_IGNORE_BUFFERFULLNESS;
      } else {
        hTp->flags &= ~TPDEC_IGNORE_BUFFERFULLNESS;
      }
      break;
    case TPDEC_PARAM_SET_BITRATE:
      hTp->avgBitRate = value;
      break;
    case TPDEC_PARAM_BURST_PERIOD:
      hTp->burstPeriod = value;
      break;
    case TPDEC_PARAM_RESET: {
      int i;

      for (i = 0; i < TPDEC_MAX_TRACKS; i++) {
        FDKresetBitbuffer(&hTp->bitStream[i]);
        hTp->auLength[i] = 0;
        hTp->accessUnitAnchor[i] = 0;
        hTp->ctrlCFGChange[i].Mpegh3daConfigLen = 0;
      }
      hTp->flags &= ~(TPDEC_SYNCOK | TPDEC_CONFIG_FOUND | TPDEC_LOST_FRAMES_PENDING);
      hTp->remainder = 0;
      hTp->avgBitRate = 0;
      hTp->missingAccessUnits = 0;
      hTp->numberOfRawDataBlocks = 0;
      hTp->globalFramePos = 0;
      hTp->holdOffFrames = 0;
    } break;
    case TPDEC_PARAM_TARGETLAYOUT:
      hTp->targetLayout = value;
      break;
    case TPDEC_PARAM_FORCE_CONFIG_CHANGE:
      hTp->ctrlCFGChange[value].forceCfgChange = TPDEC_FORCE_CONFIG_CHANGE;
      break;
    case TPDEC_PARAM_USE_ELEM_SKIPPING:
      if (value) {
        hTp->flags |= TPDEC_USE_ELEM_SKIPPING;
      } else {
        hTp->flags &= ~TPDEC_USE_ELEM_SKIPPING;
      }
      break;
  }

  return error;
}

void transportDec_Close(HANDLE_TRANSPORTDEC* phTp) {
  if (phTp != NULL) {
    if (*phTp != NULL) {
      FreeRam_TransportDecoderBuffer(&(*phTp)->bsBuffer);
      FreeRam_TransportDecoder(phTp);
    }
  }
}

int transportDec_CrcStartReg(HANDLE_TRANSPORTDEC pTp, INT mBits) {
  switch (pTp->transportFmt) {
    default:
      return -1;
  }
}

void transportDec_CrcEndReg(HANDLE_TRANSPORTDEC pTp, INT reg) {
  switch (pTp->transportFmt) {
    default:
      break;
  }
}

TRANSPORTDEC_ERROR transportDec_CrcCheck(HANDLE_TRANSPORTDEC pTp) {
  switch (pTp->transportFmt) {
    default:
      return TRANSPORTDEC_OK;
  }
}
