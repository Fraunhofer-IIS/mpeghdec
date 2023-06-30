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

   Description: long/short-block decoding

*******************************************************************************/

#ifndef BLOCK_H
#define BLOCK_H

#include "common_fix.h"

#include "channelinfo.h"
#include "FDK_bitstream.h"

void CBlock_ApplyNoise(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                       CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                       SamplingRateInfo* pSamplingRateInfo, ULONG* nfRandomSeed,
                       UCHAR* band_is_noise, INT flag_INFactive, INT igf_StereoFilling,
                       INT max_noise_sfb);

void IGF_StereoFillingPrepare(CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                              SamplingRateInfo* pSamplingRateInfo, const INT max_noise_sfb,
                              FIXP_DBL* dmx_prev_modified, SHORT* dmx_prev_win_exp,
                              const UCHAR* band_is_noise);

void IGF_StereoFillingApply(CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                            SPECTRAL_PTR pSpectralCoefficient, SHORT* p2_tile_spectrum_exp,
                            SamplingRateInfo* pSamplingRateInfo, const INT max_noise_sfb,
                            FIXP_DBL* dmx_prev_modified, const SHORT* dmx_prev_modified_e,
                            const UCHAR* band_is_noise);

/* TNS (of block) */
/*!
  \brief Read tns data-present flag from bitstream

  The function reads the data-present flag for tns from
  the bitstream.

  \return  none
*/
void CTns_ReadDataPresentFlag(HANDLE_FDK_BITSTREAM bs, CTnsData* pTnsData);

void CTns_ReadDataPresentUsac(HANDLE_FDK_BITSTREAM hBs, CTnsData* pTnsData0, CTnsData* pTnsData1,
                              UCHAR* ptns_on_lr, const CIcsInfo* pIcsInfo, const UINT flags,
                              const UINT elFlags, const int fCommonWindow);

AAC_DECODER_ERROR CTns_Read(HANDLE_FDK_BITSTREAM bs, CTnsData* pTnsData, const CIcsInfo* pIcsInfo,
                            const UINT flags);

void CTns_Apply(CTnsData* RESTRICT pTnsData, /*!< pointer to aac decoder info */
                const CIcsInfo* pIcsInfo, SPECTRAL_PTR pSpectralCoefficient,
                const SamplingRateInfo* pSamplingRateInfo, const INT granuleLength,
                const UCHAR nbands, const UCHAR igf_active, const UINT flags);

/* Block */

/**
 * \brief Read scale factor data. See chapter 4.6.2.3.2 of ISO/IEC 14496-3.
 *        The SF_OFFSET = 100 value referenced in chapter 4.6.2.3.3 is already substracted
 *        from the scale factor values. Also includes PNS data reading.
 * \param bs bit stream handle data source
 * \param pAacDecoderChannelInfo channel context info were decoded data is stored into.
 * \param flags the decoder flags.
 */
AAC_DECODER_ERROR CBlock_ReadScaleFactorData(CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                                             HANDLE_FDK_BITSTREAM bs, const UINT flags);

/**
 * \brief Read Arithmetic encoded spectral data.
 * \param pAacDecoderChannelInfo channel context info.
 * \param pAacDecoderStaticChannelInfo static channel context info.
 * \param pSamplingRateInfo sampling rate info (sfb offsets).
 * \param frame_length spectral window length.
 * \param flags syntax flags.
 * \return error code.
 */
AAC_DECODER_ERROR CBlock_ReadAcSpectralData(
    HANDLE_FDK_BITSTREAM hBs, CAacDecoderChannelInfo* pAacDecoderChannelInfo,
    CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
    const SamplingRateInfo* pSamplingRateInfo, const UINT frame_length, const UINT flags);

/**
 * \brief find a common exponent (shift factor) for all sfb in each Spectral window, and store them
 * into CAacDecoderChannelInfo::specScale.
 * \param pAacDecoderChannelInfo channel context info.
 * \param UCHAR maxSfbs maximum number of SFBs to be processed (might differ from
 * pAacDecoderChannelInfo->icsInfo.MaxSfBands)
 * \param pSamplingRateInfo sampling rate info (sfb offsets).
 */
void CBlock_ScaleSpectralData(CAacDecoderChannelInfo* pAacDecoderChannelInfo, UCHAR maxSfbs,
                              SamplingRateInfo* pSamplingRateInfo);

void CBlock_ScaleTileData(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                          CAacDecoderChannelInfo* pAacDecoderChannelInfo, UCHAR maxSfbs,
                          SamplingRateInfo* pSamplingRateInfo);

/**
 * \brief Apply TNS and PNS tools.
 */
void ApplyTools(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo[],
                CAacDecoderChannelInfo* pAacDecoderChannelInfo[],
                const SamplingRateInfo* pSamplingRateInfo, const UINT flags, const UINT elFlags,
                const int channel, const int maybe_jstereo);

/**
 * \brief Transform MDCT spectral data into time domain
 */
void CBlock_FrequencyToTime(CAacDecoderStaticChannelInfo* pAacDecoderStaticChannelInfo,
                            CAacDecoderChannelInfo* pAacDecoderChannelInfo, PCM_DEC outSamples[],
                            const SHORT frameLen, const int frameOk, FIXP_DBL* pWorkBuffer1,
                            const INT aacOutDataHeadroom, UINT elFlags, INT elCh);

AAC_DECODER_ERROR CBlock_InverseQuantizeSpectralData(CAacDecoderChannelInfo* pAacDecoderChannelInfo,
                                                     SamplingRateInfo* pSamplingRateInfo,
                                                     UCHAR* band_is_noise,
                                                     UCHAR active_band_search);

/**
 * \brief Calculate 2^(lsb/4) * value^(4/3)
 * \param pValue pointer to quantized value. The inverse quantized result is stored back here.
 * \param lsb 2 LSBs of the scale factor (scaleFactor % 4) applied as power 2 factor to the
 *        resulting inverse quantized value.
 * \return the exponent of the result (mantissa) stored into *pValue.
 */
FDK_INLINE
int EvaluatePower43(FIXP_DBL* pValue, UINT lsb) {
  INT q = (INT)*pValue;
  INT valueExp = a2ToPow0p25Exp;

  if (q < INV_QUANT_TABLESIZE) {
    *pValue = fMult(InverseQuantTableMant[q], a2ToPow0p25Mant[lsb]);
    valueExp += InverseQuantTableExp[q];
  } else {
    INT e0, e1, idx;
    FIXP_DBL t0, t1, td;

    idx = (q >> 3);
    e0 = InverseQuantTableExp[idx];
    e1 = InverseQuantTableExp[idx + 1];

    t0 = InverseQuantTableMant[idx] >> (e1 - e0);
    t1 = InverseQuantTableMant[idx + 1];

    td = fMult(t1 - t0, InverseQuantInterpolationTable[q & 7]);

    *pValue = fMult(t0 + td, a2ToPow0p25Mant[lsb]);
    valueExp += e1 + InverseQuantInterpolationExp;
  }

  return valueExp;
}

FDK_INLINE void InverseQuantize(FIXP_DBL* pSignedValue, UINT lsb, int scale) {
  FIXP_DBL signedValue;

  signedValue = *pSignedValue;
  if (signedValue == (FIXP_DBL)0) return;

  FIXP_DBL value;
  value = (signedValue < (FIXP_DBL)0) ? -signedValue : signedValue;

  scale += EvaluatePower43(&value, lsb);

  value = scaleValue(value, scale);

  *pSignedValue = (signedValue < (FIXP_DBL)0) ? -value : value;
}

/*!
  \brief Conversion of index values to quantized coefficients

  The function converts the index values to quantized coefficients.

  \return  dimension
*/
FDK_INLINE
int CBlock_UnpackIndex(
    int idx,                        /*!< pointer to index */
    FIXP_DBL* RESTRICT qp,          /*!< pointer to quantized coefficients */
    const CodeBookDescription* hcb) /*!< pointer to sideinfo of the current codebook */
{
  int dim = hcb->Dimension;
  int offset = hcb->Offset;
  int bits = hcb->numBits;
  int mask = (1 << bits) - 1;
  int i;

  for (i = 0; i < dim; i++) {
    qp[i] = (FIXP_DBL)((idx & mask) - offset);
    idx >>= bits;
  }

  return dim;
}

/*!
  \brief Read huffman codeword

  The function reads the huffman codeword from the bitstream and
  returns the index value.

  \return  index value
*/
inline int CBlock_DecodeHuffmanWord(
    HANDLE_FDK_BITSTREAM bs,        /*!< pointer to bitstream */
    const CodeBookDescription* hcb) /*!< pointer to codebook description */
{
  UINT val;
  UINT index = 0;
  const USHORT(*CodeBook)[HuffmanEntries] = hcb->CodeBook;

  while (1) {
    val = CodeBook[index][FDKreadBits(bs, HuffmanBits)]; /* Expensive memory access */

    if ((val & 1) == 0) {
      index = val >> 2;
      continue;
    } else {
      if (val & 2) {
        FDKpushBackCache(bs, 1);
      }

      val >>= 2;
      break;
    }
  }

  return val;
}
FDK_FORCEINLINE
int CBlock_DecodeHuffmanWordCB(
    HANDLE_FDK_BITSTREAM bs,                  /*!< pointer to bitstream */
    const USHORT (*CodeBook)[HuffmanEntries]) /*!< pointer to codebook description */
{
  UINT index = 0;
  while (1) {
    index = CodeBook[index][FDKread2Bits(bs)]; /* Expensive memory access */
    if (index & 1) break;
    index >>= 2;
  }
  if (index & 2) {
    FDKpushBackCache(bs, 1);
  }
  return index >> 2;
}

#endif /* #ifndef BLOCK_H */
