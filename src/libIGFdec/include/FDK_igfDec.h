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

/********************** Intelligent gap filling library ************************

   Author(s):   Youliy Ninov

   Description: Intelligent gap filling library

*******************************************************************************/

#ifndef FDK_IGFDEC_H
#define FDK_IGFDEC_H

#include "FDK_bitstream.h"
#include "FDK_igfSCFDecoder.h"

#define IGF_CLEAN_THRESHOLD 16

#define IGF_MAX_WIN 8
#define IGF_MAX_TILES 4
#define IGF_MAX_GRIDS 2
#define N_IGF_FRAME_DIVISION 2
#define IGF_MAX_SFB_BANDS 51
#define IGF_MAX_SFB_SHORT 16
#define IGF_MAX_TILE_WIDTH 1024

typedef enum {
  IGF_FRAME_DIVISION_AAC_OR_TCX_LONG =
      0, /* AAC short blocks are interleaved and AAC and TCX long can share memory */
  IGF_FRAME_DIVISION_TCX_SHORT_1 = 1,
  IGF_FRAME_DIVISION_TCX_SHORT_2 = 2
} IGF_FRAME_DIVISION;

typedef enum { WHITENING_MID, WHITENING_OFF, WHITENING_STRONG, WHITENING_UNUSED } WHITENING_LEVEL;

typedef enum {
  IGF_GRID_LONG_WINDOW,
  IGF_GRID_SHORT_WINDOW,
  IGF_GRID_TCX10_WINDOW,
  IGF_GRID_TCX20_WINDOW,
  IGF_GRID_UNDEF
} DEC_IGF_GRID;

typedef struct igf_map_info_struct {
  FIXP_DBL* fSpectrumTab[1]; /**< holds the information of the spectrum after dequantization and
                                noisefilling from iSrcStart to (iSrcStart+iWidth), is used as source
                                spectrum for IGF **/

  WHITENING_LEVEL iWhiteningLevel; /**< whitening level of the tile **/

  SHORT iSfbWidthTab[IGF_MAX_SFB_BANDS]; /**< holds information for the sfb granulation in a tile */
  SHORT iSfbCnt;                         /**< count of sfbs in a tile */
  SHORT iDesStart; /**< defines the first line in destination spectrum of the tile **/
  SHORT iWidth;    /**< width (or lengt) of the tile **/
  UCHAR iIdx;      /**< tile index, transmitted in the bitstream **/
  UCHAR iSfbSplit; /**< When a paired SFB must be split between two tiles */

} IGF_MAP_INFO, *IGF_MAP_INFO_HANDLE;

typedef struct igf_work_memory_struct {
  FIXP_DBL fSfbGainTab[IGF_MAX_SFB_BANDS];         /**< gain to apply on IGF data */
  FIXP_DBL fSfbDestinEnergyTab[IGF_MAX_SFB_BANDS]; /**< energy levels transmitted and decoded */
  FIXP_DBL fSfbTileEnergyTab[IGF_MAX_WIN *
                             IGF_MAX_SFB_SHORT]; /**< energy levels calculated from source region */
  FIXP_DBL fSfbSurvivedEnergyTab[IGF_MAX_WIN * IGF_MAX_SFB_SHORT]; /**< energy levels calculated
                                                                      from source region */
  FIXP_DBL sE[IGF_MAX_SFB_BANDS];
  FIXP_DBL tE[IGF_MAX_SFB_BANDS];

  SCHAR fSfbGainTab_exp[IGF_MAX_SFB_BANDS];
  SCHAR fSfbDestinEnergyTab_exp[IGF_MAX_SFB_BANDS]; /**< energy levels transmitted and decoded */
  SCHAR fSfbTileEnergyTab_exp[IGF_MAX_WIN * IGF_MAX_SFB_SHORT];
  SCHAR fSfbSurvivedEnergyTab_exp[IGF_MAX_WIN * IGF_MAX_SFB_SHORT];
  SCHAR sE_exp[IGF_MAX_SFB_BANDS];
  SCHAR tE_exp[IGF_MAX_SFB_BANDS];

  SCHAR intermediate_shift[IGF_MAX_WIN * IGF_MAX_SFB_SHORT];
  SCHAR Initial_exp[IGF_MAX_WIN * IGF_MAX_SFB_SHORT];

} IGF_WORK_MEMORY, *IGF_WORK_MEMORY_HANDLE;

typedef struct igf_grid_info_struct {
  IGF_MAP_INFO sIGFMapInfoTab[IGF_MAX_TILES];
  SHORT iIGFStartLine;                 /**< start line in IGF range */
  SHORT iIGFStopLine;                  /**< stop line in IGF range */
  SHORT iIGFMinLine;                   /**< minimal line where IGF start to copy from */
  UCHAR iIGFTileIdxTab[IGF_MAX_TILES]; /**< tile index table used with initialization of
                                          sIGFMapInfoTab */
  UCHAR iIGFNumSfb;                    /**< IGF number of SCF bands in total */
  UCHAR iIGFStartSfb;                  /**< start SBF in IGF range */
  UCHAR iIGFStopSfb;                   /**< stop SFB in IGF range */
  UCHAR iIGFNumTile;                   /**< number of tiles used */

} IGF_GRID_INFO, *IGF_GRID_INFO_HANDLE;

typedef struct igf_private_data_static_struct {
  IGFSCFDEC_PRIVATE_DATA hIGFSCFDecoder;

  IGF_GRID_INFO sGridInfoTab[IGF_MAX_GRIDS];

  SHORT fSpectrumScale[IGF_MAX_TILES][IGF_MAX_WIN]; /**< window scale values **/
  SHORT fSpectrumTab_sfb_exp[IGF_MAX_TILES]
                            [IGF_MAX_WIN * IGF_MAX_SFB_SHORT]; /**< sfb scale values **/

  WHITENING_LEVEL igfWhiteningLevelPrev[IGF_MAX_TILES];

  INT aacFrameLength;
  INT igfMinSubbandLB;
  INT igfMinSubbandSB;
  INT igfMinSubbandTCX10;
  INT igfMinSubbandTCX20;
  INT igfTilesLB[IGF_MAX_TILES];
  INT igfTilesSB[IGF_MAX_TILES];
  INT igfTilesTCX10[IGF_MAX_TILES];
  INT igfTilesTCX20[IGF_MAX_TILES];
  UCHAR prevPatchNum[IGF_MAX_TILES]; /**< indices of the best matching patch */
  SHORT shortBlockSize;
  SHORT longBlockSize;
  UCHAR numSfbLB;
  const SHORT* sfbOffsetLB;
  UCHAR numSfbSB;
  const SHORT* sfbOffsetSB;
  UCHAR enhancedNoiseFilling;
  UCHAR igfUseWhitening;
  UCHAR igfAfterTnsSynth;
  UCHAR igfIndependentTiling;
  UCHAR useHighRes;
  UCHAR igfStartSfbLB;
  UCHAR igfStopSfbLB;
  UCHAR igfStartSfbSB;
  UCHAR igfStopSfbSB;
  UCHAR igfNTilesLB;
  UCHAR igfNTilesSB;
  UCHAR igfNTilesTCX10;
  UCHAR igfNTilesTCX20;

} IGF_PRIVATE_STATIC_DATA, *IGF_PRIVATE_STATIC_DATA_HANDLE;

typedef struct {
  SCHAR sfe[256]; /**< array which will contain the decoded quantized coefficients */
  UCHAR iCurrTileIdxTab[IGF_MAX_TILES];
  WHITENING_LEVEL igfWhiteningLevel[IGF_MAX_TILES];
  UCHAR igf_allZero;
  UCHAR igfUseEnfFlat;
} IGF_BITSTREAM_DATA;

typedef struct igf_private_data_common_struct {
  FIXP_DBL Spectrum_tab_array[IGF_MAX_TILES * 1024];
  FIXP_DBL virtualSpec[1024];
  IGF_WORK_MEMORY IGF_WorkingMem[IGF_MAX_TILES];

} IGF_PRIVATE_DATA_COMMON, *IGF_PRIVATE_DATA_COMMON_HANDLE;

typedef struct igf_private_data_struct {
  IGF_PRIVATE_DATA_COMMON_HANDLE IGF_Common_channel_data_handle;
  IGF_BITSTREAM_DATA bitstreamData[N_IGF_FRAME_DIVISION];
  UCHAR tileNum[IGF_MAX_TILES]; /**< indices of the best matching patch */

} IGF_PRIVATE_DATA, *IGF_PRIVATE_DATA_HANDLE;

void iisIGFDecLibInit(
    IGF_PRIVATE_STATIC_DATA* hPrivateStaticData, /**< inout: instance handle of MPEG-H */
    IGF_PRIVATE_DATA* hPrivateData,              /**< inout: instance handle of MPEG-H */
    IGF_PRIVATE_DATA_COMMON* hPrivateCommonData, const UCHAR igfStartIndex,
    const UCHAR igfStopIndex, const UCHAR igfUseHighRes, const UCHAR igfUseWhitening,
    const UINT aacSampleRate, const INT aacFrameLength, const SHORT* sfb_offset_LB,
    const INT len_LB, const SHORT* sfb_offset_SB, const INT len_SB,
    const UCHAR enhancedNoiseFilling, const UCHAR igfAfterTnsSynth,
    const UCHAR igfIndependentTiling);

/**********************************************************************/ /**
 implements the decoding of SCF
 **************************************************************************/
void iisIGFDecLibReadSCF(
    IGF_PRIVATE_STATIC_DATA* hPrivateStaticData, IGF_PRIVATE_DATA* hPrivateData,
    HANDLE_FDK_BITSTREAM hBStr,
    const INT indepFlag,       /**< in: if  1 on input the encoder will be forced to reset
                           if  0 on input the encodder will be forced to encode without a reset */
    const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq */
    const INT winCount,        /**< in: number of windows in the group */
    const INT frameType);

/**********************************************************************/ /**
 implements the decoding of IGF
 **************************************************************************/
int iisIGFDecLibReadIGF(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData,
                        IGF_PRIVATE_DATA_HANDLE hPrivateData, HANDLE_FDK_BITSTREAM hBStr,
                        const INT window_sequence,       /**< in: 1==SHORT 0==LONG window seq */
                        const INT bUsacIndependencyFlag, /**< in: USAC independency flag */
                        const INT igfUseEnf, const INT frameType);

void iisIGF_Sync_Data(IGF_PRIVATE_DATA_HANDLE hPrivateDataL, IGF_PRIVATE_DATA_HANDLE hPrivateDataR);

void CIgf_apply(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData,
                IGF_PRIVATE_DATA_HANDLE hPrivateData, FIXP_DBL* p2_spectrum, SHORT* specScale,
                const INT window_sequence, const INT numOfGroups, const INT NumberOfSpectra,
                const UCHAR* groupLength, ULONG* randomSeed, UCHAR* TNF_mask,
                const UCHAR flag_INF_active, const INT frameType);

void CIgf_apply_stereo(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticDataL,
                       IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticDataR,
                       IGF_PRIVATE_DATA_HANDLE hPrivateDataL, IGF_PRIVATE_DATA_HANDLE hPrivateDataR,
                       FIXP_DBL* p2_spectrumL, FIXP_DBL* p2_spectrumR, SHORT* specScaleL,
                       SHORT* specScaleR, const INT window_sequence, const INT numOfGroups,
                       const INT NumberOfSpectra, const UCHAR* groupLength, ULONG* randomSeedL,
                       ULONG* randomSeedR, const UCHAR* iUseMSTab, UCHAR* TNF_maskL,
                       UCHAR* TNF_maskR, const UCHAR flag_INF_active, const INT frameType);

void iisIGFDecLibInjectSourceSpectrumNew(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    FIXP_DBL* pSpectralData,                             /**< ptr to spectral window data       */
    SHORT* pSpectralData_exp, const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq    */
    const INT frameType);

FIXP_DBL* iisIGFDecLibAccessSourceSpectrum(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    const INT tileIdx, const INT window_sequence, /**< in: 1==SHORT 0==LONG window seq */
    const INT win);

SHORT* iisIGFDecLibAccessSourceSpectrum_exponent(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, const INT tileIdx,
    const INT window_sequence /**< in: 1==SHORT 0==LONG window seq */
);

SHORT* iisIGFDecLibAccessSourceSpectrum_Scale(
    IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData, IGF_PRIVATE_DATA_HANDLE hPrivateData,
    const INT tileIdx, const INT window_sequence /**< in: 1==SHORT 0==LONG window seq */
);

INT iisIGFDecLibGetNumberOfTiles(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData,
                                 const INT window_sequence /**< in: 1==SHORT 0==LONG window seq */
);

/********************************************************
    TNF functions
*********************************************************/
void CIgf_TNF_apply(IGF_PRIVATE_STATIC_DATA_HANDLE hPrivateStaticData,
                    IGF_PRIVATE_DATA_HANDLE hPrivateData, FIXP_DBL* p2_spectrum, SHORT* specScale,
                    const UCHAR* TNF_mask, const INT window_sequence, const INT frameType);

#endif /* FDK_IGFDEC_H */
