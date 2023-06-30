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

#if !defined(FDK_DMXMATRIXPARSER_H)
#define FDK_DMXMATRIXPARSER_H

#include "FDK_formatConverter_intern.h"
#include "FDK_formatConverterLib.h"
#include "FDK_bitstream.h"

#define DMX_MATRIX_SET_COUNT 32
#define DMX_MATRIX_MAX_SPEAKER_COUNT 32
#define DMX_MATRIX_CODER_STATE_COUNT_MAX 512
#define DMX_MATRIX_GAIN_TABLE_SIZE_MAX ((22 - (-47)) * (1 << 2) + 2)
#define DMX_MATRIX_GAIN_ZERO -256
#define FDK_DMX_MATRIX_GAIN_ZERO ((FIXP_DBL)((LONG)DMX_MATRIX_GAIN_ZERO * (1 << 23))) /* Q23 */
#define ARRAYLENGTH(a) (sizeof(a) / sizeof(a[0]))

#define FDK_DOWNMIX_MATRIX_DEC_MAX_NUM_EQ \
  (39) /* Max: ReadEscapedValue(hBitStream, 3, 5, 0) + 1; (dmx_matrix_dec.c) */
#define FDK_DOWNMIX_MATRIX_DEC_MAX_NUM_PEAK_FILTER_PARAMS \
  (19) /* Max: ReadEscapedValue(hBitStream, 2, 4, 0) + 1; (dmx_matrix_dec.c) */
#define FDK_DOWNMIX_MATRIX_DEC_MAX_INPUT_CHANNELS \
  (FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNELS) /* max at LC Level 5 */

#define FDK_MPEGHAUDIO_DEC_MAX_LOUDSPEAKERS \
  (48) /* Mpeg-H actually allows up to 65536 loudspeakers in this context. */
#define FDK_MPEGHAUDIO_DEC_MAX_SIGNAL_GROUPS (FDK_FORMAT_CONVERTER_MAX_INPUT_CHANNEL_GROUPS * 2)
#define FDK_MPEGHAUDIO_DEC_MAX_OUTPUT_CHANNELS (FDK_FORMAT_CONVERTER_MAX_OUTPUT_CHANNELS)

#define FDK_DOWNMIX_CONFIG_MAX_DMX_MATRICES_PER_ID (9)
#define FDK_DOWNMIX_CONFIG_MAX_DMX_MATRIX_SIZE (256)

/* parameters struct for peak filter definition */
typedef struct {
  FIXP_DBL f; /* peak frequency [Hz]     */
  FIXP_DBL q; /* peak Q factor           */
  FIXP_DBL g; /* peak gain [dB]          */
  SCHAR f_e;  /* peak frequency [Hz] exp */
  SCHAR q_e;  /* peak Q factor exp       */
  SCHAR g_e;  /* peak gain [dB] exp      */
} pkFilterParamsStruct;

/* parameters struct for EQ definition */
typedef struct {
  pkFilterParamsStruct pkFilterParams
      [FDK_DOWNMIX_MATRIX_DEC_MAX_NUM_PEAK_FILTER_PARAMS]; /* pointer to nPkFilter-element array of
                                                              pkFilterParamsStructs */
  FIXP_DBL G;                                              /* global gain [dB]                */
  SCHAR G_e;                                               /* global gain [dB] exp            */
  UCHAR nPkFilter;                                         /* number of cascaded peak filters */
} eqParamsStruct;

/* parameters struct for the entire EQ configuration */
typedef struct {
  eqParamsStruct eqParams[FDK_DOWNMIX_MATRIX_DEC_MAX_NUM_EQ]; /* array of EQs of size numEQs */
  UINT eqMap[FDK_DOWNMIX_MATRIX_DEC_MAX_INPUT_CHANNELS]; /* index map for the EQs of size inputCount
                                                          */
  UCHAR numEQs;                                          /* number of EQs */
} eqConfigStruct;

typedef struct {
  UCHAR downmixMatrixMemory[FDK_DOWNMIX_CONFIG_MAX_DMX_MATRICES_PER_ID]
                           [FDK_DOWNMIX_CONFIG_MAX_DMX_MATRIX_SIZE];
  INT downmixMatrixSize[FDK_DOWNMIX_CONFIG_MAX_DMX_MATRICES_PER_ID];
  UCHAR downmixMatrix[FDK_MPEGHAUDIO_DEC_MAX_SIGNAL_GROUPS];
} FDK_DOWNMIX_GROUPS_MATRIX_SET;

typedef enum {
  SP_PAIR_CENTER,    /* one center speaker */
  SP_PAIR_SYMMETRIC, /* a symmetric L/R speaker pair */
  SP_PAIR_SINGLE,    /* an asymmetric single speaker */
  SP_PAIR_NONE       /* the right speaker of a symmetric L/R speaker pair */
} SP_PAIR_TYPE;

typedef struct SpeakerInformationStruct {
  SHORT elevation; /* speaker elevation in degrees, positive angles upwards */
  SHORT azimuth;   /* speaker azimuth in degrees, positive angles to the left */
  SHORT isLFE;     /* whether the speaker type is LFE */

  SHORT originalPosition; /* original speaker position in the channel list */
  SHORT isAlreadyUsed;    /* channel is already used in the compact channel list */
  struct SpeakerInformationStruct*
      symmetricPair;     /* the right speaker of a symmetric L/R speaker pair */
  SP_PAIR_TYPE pairType; /* the type of pair for compact speaker configurations */
} SpeakerInformation;

typedef struct {
  INT minGain;
  INT maxGain;
  INT precisionLevel;
  INT rawCodingNonzeros;
  INT gainLGRParam;
  FIXP_DBL history[DMX_MATRIX_CODER_STATE_COUNT_MAX];
  INT historyCount;
  FIXP_DBL gainTable[DMX_MATRIX_GAIN_TABLE_SIZE_MAX];
  INT gainTableSize;
} CoderState;

/*
  ReadRange()
*/

UINT ReadRange(HANDLE_FDK_BITSTREAM hBs, UINT alphabetSize);

/*
  EqualizerConfig() subroutine for equalizers
*/

INT EqualizerConfig(HANDLE_FDK_BITSTREAM hBs, SpeakerInformation* inputConfig, UINT inputCount,
                    eqConfigStruct* eqConfig);

/*
  DownmixMatrix() subroutine for downmix matrix.
*/

void ConvertToCompactConfig(SpeakerInformation* inputConfig, INT inputCount, INT* compactInputCount,
                            SpeakerInformation* compactInputConfig[]);

void DecodeFlatCompactMatrix(HANDLE_FDK_BITSTREAM hBs, signed char* flatCompactMatrix,
                             int totalCount);

void CoderStateGenerateGainTable(CoderState* cs);

signed char* FindCompactTemplate(int inputIndex, int outputIndex);

void CoderStateInit(CoderState* cs);

INT DecodeDownmixMatrix(INT inputIndex, INT inputCount, SpeakerInformation* inputConfig,
                        INT outputIndex, INT outputCount, SpeakerInformation* outputConfig,
                        HANDLE_FDK_BITSTREAM hBs, FIXP_SGL* downmixMatrix, eqConfigStruct* eqConfig,
                        FIXP_DBL* p_buffer);

INT DownmixMatrixSet(HANDLE_FDK_BITSTREAM hBs,
                     FDK_DOWNMIX_GROUPS_MATRIX_SET* groupsDownmixMatrixSet, const INT targetLayout,
                     const INT downmixConfigType, INT* downmixId, HANDLE_DRC_DECODER hUniDrcDec);

#endif /* !defined(FDK_DMXMATRIXPARSER_H) */
