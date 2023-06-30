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

   Description: temporal noise shaping tool

*******************************************************************************/

#include "aacdec_tns.h"
#include "aac_rom.h"
#include "FDK_bitstream.h"
#include "channelinfo.h"

#include "FDK_lpc.h"

#define TNS_MAXIMUM_ORDER_AAC 12

#define LPC_SYNTHESIS_IIR

/*!
  \brief Reset tns data

  The function resets the tns data

  \return  none
*/
void CTns_Reset(CTnsData* pTnsData) {
  /* Note: the following FDKmemclear should not be required. */
  FDKmemclear(pTnsData->Filter, TNS_MAX_WINDOWS * TNS_MAXIMUM_FILTERS * sizeof(CFilter));
  FDKmemclear(pTnsData->NumberOfFilters, TNS_MAX_WINDOWS * sizeof(UCHAR));
  pTnsData->DataPresent = 0;
  pTnsData->Active = 0;
}

void CTns_ReadDataPresentFlag(HANDLE_FDK_BITSTREAM bs, /*!< pointer to bitstream */
                              CTnsData* pTnsData)      /*!< pointer to aac decoder channel info */
{
  pTnsData->DataPresent = (UCHAR)FDKreadBit(bs);
}

/*!
  \brief Read tns data from bitstream

  The function reads the elements for tns from
  the bitstream.

  \return  none
*/
AAC_DECODER_ERROR CTns_Read(HANDLE_FDK_BITSTREAM bs, CTnsData* pTnsData, const CIcsInfo* pIcsInfo,
                            const UINT flags) {
  UCHAR n_filt, order;
  UCHAR length, coef_res, coef_compress;
  UCHAR window;
  UCHAR wins_per_frame;
  UCHAR isLongFlag;
  UCHAR start_window;
  AAC_DECODER_ERROR ErrorStatus = AAC_DEC_OK;

  if (!pTnsData->DataPresent) {
    return ErrorStatus;
  }

  {
    start_window = 0;
    wins_per_frame = GetWindowsPerFrame(pIcsInfo);
    isLongFlag = IsLongBlock(pIcsInfo);
  }

  pTnsData->GainLd = 0;

  for (window = start_window; window < wins_per_frame; window++) {
    pTnsData->NumberOfFilters[window] = n_filt = (UCHAR)FDKreadBits(bs, isLongFlag ? 2 : 1);

    if (n_filt) {
      int index;
      UCHAR nextstopband;

      coef_res = (UCHAR)FDKreadBit(bs);

      nextstopband = GetScaleFactorBandsTotal(pIcsInfo);

      for (index = 0; index < n_filt; index++) {
        CFilter* filter = &pTnsData->Filter[window][index];

        length = (UCHAR)FDKreadBits(bs, isLongFlag ? 6 : 4);

        if (length > nextstopband) {
          length = nextstopband;
        }

        filter->StartBand = nextstopband - length;
        filter->StopBand = nextstopband;
        nextstopband = filter->StartBand;

        if (flags & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA)) {
          /* max(Order) = 15 (long), 7 (short) */
          filter->Order = order = (UCHAR)FDKreadBits(bs, isLongFlag ? 4 : 3);
        } else {
          filter->Order = order = (UCHAR)FDKreadBits(bs, isLongFlag ? 5 : 3);

          if (filter->Order > TNS_MAXIMUM_ORDER) {
            ErrorStatus = AAC_DEC_TNS_READ_ERROR;
            return ErrorStatus;
          }
        }

        FDK_ASSERT(order <= TNS_MAXIMUM_ORDER); /* avoid illegal memory access */
        if (order) {
          UCHAR coef, s_mask;
          UCHAR i;
          SCHAR n_mask;

          static const UCHAR sgn_mask[] = {0x2, 0x4, 0x8};
          static const SCHAR neg_mask[] = {~0x3, ~0x7, ~0xF};

          filter->Direction = FDKreadBit(bs) ? -1 : 1;

          coef_compress = (UCHAR)FDKreadBit(bs);

          filter->Resolution = coef_res + 3;

          s_mask = sgn_mask[coef_res + 1 - coef_compress];
          n_mask = neg_mask[coef_res + 1 - coef_compress];

          for (i = 0; i < order; i++) {
            coef = (UCHAR)FDKreadBits(bs, filter->Resolution - coef_compress);
            filter->Coeff[i] = (coef & s_mask) ? (coef | n_mask) : coef;
          }
          pTnsData->GainLd = 4;
        }
      }
    }
  }

  pTnsData->Active = 1;

  return ErrorStatus;
}

void CTns_ReadDataPresentUsac(HANDLE_FDK_BITSTREAM hBs, CTnsData* pTnsData0, CTnsData* pTnsData1,
                              UCHAR* ptns_on_lr, const CIcsInfo* pIcsInfo, const UINT flags,
                              const UINT elFlags, const int fCommonWindow) {
  int common_tns = 0;

  if (fCommonWindow) {
    common_tns = FDKreadBit(hBs);
  }
  /*if ( igFilling && !igFAfterTnsSynth) { */
  if (((elFlags & (AC_EL_ENHANCED_NOISE | AC_EL_IGF_AFTER_TNS)) == AC_EL_ENHANCED_NOISE)) {
    *ptns_on_lr = 1;
  } else {
    *ptns_on_lr = FDKreadBit(hBs);
  }
  if (common_tns) {
    pTnsData0->DataPresent = 1;
    CTns_Read(hBs, pTnsData0, pIcsInfo, flags);

    pTnsData0->DataPresent = 0;
    pTnsData0->Active = 1;
    *pTnsData1 = *pTnsData0;
  } else {
    int tns_present_both;

    tns_present_both = FDKreadBit(hBs);
    if (tns_present_both) {
      pTnsData0->DataPresent = 1;
      pTnsData1->DataPresent = 1;
    } else {
      pTnsData1->DataPresent = FDKreadBit(hBs);
      pTnsData0->DataPresent = !pTnsData1->DataPresent;
    }
  }
}

/*!
  \brief Apply tns to spectral lines

  The function applies the tns to the spectrum,

  \return  none
*/
void CTns_Apply(CTnsData* RESTRICT pTnsData, /*!< pointer to aac decoder info */
                const CIcsInfo* pIcsInfo, SPECTRAL_PTR pSpectralCoefficient,
                const SamplingRateInfo* pSamplingRateInfo, const INT granuleLength,
                const UCHAR nbands, const UCHAR igf_active, const UINT flags) {
  int window, index, start, stop, size, start_window, wins_per_frame;

#ifdef LPC_SYNTHESIS_IIR
  int synStateIndex = 0;
  INT lpcCoeff_e = 0;
#endif

  if (pTnsData->Active) {
    C_AALLOC_SCRATCH_START(coeff, FIXP_TCC, TNS_MAXIMUM_ORDER)
#ifdef LPC_SYNTHESIS_IIR
    C_AALLOC_SCRATCH_START(temp, FIXP_QDL, TNS_MAXIMUM_ORDER)
    C_AALLOC_SCRATCH_START(lpcCoeff_m, FIXP_TCC, TNS_MAXIMUM_ORDER)
#endif

    {
      start_window = 0;
      wins_per_frame = GetWindowsPerFrame(pIcsInfo);
    }

    for (window = start_window; window < wins_per_frame; window++) {
      FIXP_DBL* pSpectrum;

      { pSpectrum = SPEC(pSpectralCoefficient, window, granuleLength); }

      for (index = 0; index < pTnsData->NumberOfFilters[window]; index++) {
#ifdef LPC_SYNTHESIS_IIR
        synStateIndex = 0;
#endif
        CFilter* filter = &pTnsData->Filter[window][index];

        if (filter->Order > 0) {
          FIXP_TCC* pCoeff;
          UCHAR tns_max_bands;

          pCoeff = coeff;
          if (filter->Resolution == 3) {
            int i;
            for (i = 0; i < filter->Order; i++)
              *pCoeff++ = FDKaacDec_tnsCoeff3[filter->Coeff[i] + 4];
          } else {
            int i;
            for (i = 0; i < filter->Order; i++)
              *pCoeff++ = FDKaacDec_tnsCoeff4[filter->Coeff[i] + 8];
          }

          switch (granuleLength) {
            default:
              tns_max_bands = GetMaximumTnsBands(pIcsInfo, pSamplingRateInfo->samplingRateIndex);
              /* See redefinition of TNS_MAX_BANDS table */
              if ((flags & (AC_USAC | AC_RSVD50 | AC_MPEGH3DA)) &&
                  (pSamplingRateInfo->samplingRateIndex > 5)) {
                tns_max_bands += 1;
              }
              break;
          }

          start = fixMin(fixMin(filter->StartBand, tns_max_bands), nbands);

          start = GetScaleFactorBandOffsets(pIcsInfo, pSamplingRateInfo)[start];

          if ((flags & AC_MPEGH3DA) && igf_active) {
            stop = fixMin(filter->StopBand, nbands);
          } else {
            stop = fixMin(fixMin(filter->StopBand, tns_max_bands), nbands);
          }

          stop = GetScaleFactorBandOffsets(pIcsInfo, pSamplingRateInfo)[stop];

          size = stop - start;

          if (size) {
            C_ALLOC_SCRATCH_START(state, FIXP_DBL, TNS_MAXIMUM_ORDER)

            FDKmemclear(state, TNS_MAXIMUM_ORDER * sizeof(FIXP_DBL));
#ifdef LPC_SYNTHESIS_IIR
            lpcCoeff_e = CLpc_ParcorToLpc(coeff, lpcCoeff_m, filter->Order, temp);
            CLpc_Synthesis(pSpectrum + start, size, filter->Direction, lpcCoeff_m, lpcCoeff_e,
                           filter->Order, state, &synStateIndex);
#else
            CLpc_SynthesisLattice(pSpectrum + start, size, 0, 0, filter->Direction, coeff,
                                  filter->Order, state);
#endif

            C_ALLOC_SCRATCH_END(state, FIXP_DBL, TNS_MAXIMUM_ORDER)
          }
        }
      }
    }
    C_AALLOC_SCRATCH_END(coeff, FIXP_TCC, TNS_MAXIMUM_ORDER)
#ifdef LPC_SYNTHESIS_IIR
    C_AALLOC_SCRATCH_END(temp, FIXP_QDL, TNS_MAXIMUM_ORDER)
    C_AALLOC_SCRATCH_END(lpcCoeff_m, FIXP_TCC, TNS_MAXIMUM_ORDER)
#endif
  }
}
