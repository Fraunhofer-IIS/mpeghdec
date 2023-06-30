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

/**************************** PCM utility library ******************************

   Author(s):   Matthias Neusinger

   Description: Hard limiter for clipping prevention

*******************************************************************************/

#include "limiter.h"
#include "FDK_core.h"

/* library version */
#include "version.h"
/* library title */
#define TDLIMIT_LIB_TITLE "TD Limiter Lib"

#if defined(__arm__)
#include "arm/limiter_arm.cpp"
#endif

/* create limiter */
TDLimiterPtr pcmLimiter_Create(unsigned int maxAttackMs, unsigned int releaseMs, FIXP_DBL threshold,
                               unsigned int maxChannels, UINT maxSampleRate) {
  TDLimiterPtr limiter = NULL;
  unsigned int attack, release;
  FIXP_DBL attackConst, releaseConst, exponent;
  INT e_ans;

  /* calc attack and release time in samples */
  attack = (unsigned int)(maxAttackMs * maxSampleRate / 1000);
  if (attack == 0) return NULL;

  release = (unsigned int)(releaseMs * maxSampleRate / 1000);

  /* alloc limiter struct */
  limiter = (TDLimiterPtr)FDKcalloc(1, sizeof(struct TDLimiter));
  if (!limiter) return NULL;

  /* alloc max and delay buffers */
  limiter->maxBuf = (FIXP_DBL*)FDKcalloc(attack + 1, sizeof(FIXP_DBL));
  limiter->delayBuf = (FIXP_DBL*)FDKcalloc(attack * maxChannels, sizeof(FIXP_DBL));

  if (!limiter->maxBuf || !limiter->delayBuf) {
    pcmLimiter_Destroy(limiter);
    return NULL;
  }

  /* attackConst = pow(0.1, 1.0 / (attack + 1)) */
  exponent = invFixp(attack + 1);
  attackConst = fPow(FL2FXCONST_DBL(0.1f), 0, exponent, 0, &e_ans);
  attackConst = scaleValue(attackConst, e_ans);

  /* releaseConst  = (float)pow(0.1, 1.0 / (release + 1)) */
  exponent = invFixp(release + 1);
  releaseConst = fPow(FL2FXCONST_DBL(0.1f), 0, exponent, 0, &e_ans);
  releaseConst = scaleValue(releaseConst, e_ans);

  /* init parameters */
  limiter->attackMs = maxAttackMs;
  limiter->maxAttackMs = maxAttackMs;
  limiter->releaseMs = releaseMs;
  limiter->attack = attack;
  limiter->attackConst = attackConst;
  limiter->releaseConst = releaseConst;
  limiter->threshold = threshold;
  limiter->channels = maxChannels;
  limiter->maxChannels = maxChannels;
  limiter->sampleRate = maxSampleRate;
  limiter->maxSampleRate = maxSampleRate;
  limiter->release = release;
  limiter->cleanSamples = (TDLIMIT_FALLBACK_FACTOR + 1) * release;

  pcmLimiter_Reset(limiter);

  return limiter;
}

/* apply limiter */
TDLIMITER_ERROR pcmLimiter_Apply(TDLimiterPtr limiter, PCM_LIM* samplesIn, INT_PCM* samplesOut,
                                 PCM_LIM* workBuf, FIXP_DBL* pGainPerSample, const INT scaling,
                                 const UINT nSamples) {
  unsigned int i, j;
  FIXP_DBL additionalGain = 0;
  UINT additionalGainAvailable = 1;

  if (limiter == NULL) return TDLIMIT_INVALID_HANDLE;

  {
    unsigned int channels = limiter->channels;
    unsigned int attack = limiter->attack;
    unsigned int delayBufIdx = limiter->delayBufIdx;
    FIXP_DBL* delayBuf = limiter->delayBuf;
    FIXP_DBL threshold = limiter->threshold >> scaling;
    FIXP_DBL max = limiter->max;
    FIXP_DBL* maxBuf = limiter->maxBuf;
#if (SAMPLE_BITS != DFRACT_BITS)
    const FIXP_DBL roundingConst = (FIXP_DBL)0x8000 >> (scaling + 1);
    const INT roundingScaling = scaling + 1;
#endif

    if (pGainPerSample == NULL) {
      additionalGainAvailable = 0;
    }

    if (limiter->scaling != scaling) {
      scaleValuesSaturate(delayBuf, attack * channels, limiter->scaling - scaling);
      scaleValuesSaturate(maxBuf, attack + 1, limiter->scaling - scaling);
      max = scaleValueSaturate(max, limiter->scaling - scaling);
      limiter->scaling = scaling;
    }

    /* Preparatory step. Find local maxima and store them in an array. Find the global maximum. */
    PCM_LIM global_max = (PCM_LIM)0;
    PCM_LIM* loc_max_array = workBuf;
    PCM_LIM* pIn = samplesIn;

#if (PCM_LIM_BITS == 32)
    if (additionalGainAvailable) {
      /* apply additionalGain on input signal while searching for the maximum */
      for (i = 0; i < nSamples; i++) {
        PCM_LIM loc_max = (PCM_LIM)0;
        additionalGain = pGainPerSample[i];
        for (j = 0; j < channels; j++) {
          FIXP_DBL tmpIn = PCM_LIM2FIXP_DBL(*pIn);
          *pIn = FIXP_DBL2PCM_LIM(fMult(tmpIn, additionalGain));
          loc_max = fMax(loc_max, (PCM_LIM)fAbs(*pIn++));
        }
        loc_max_array[i] = loc_max;
        global_max = fMax(global_max, loc_max);
      }
    } else
#endif
    {
      for (i = 0; i < nSamples; i++) {
        PCM_LIM loc_max = (PCM_LIM)0;
        for (j = 0; j < channels; j++) {
          loc_max = fMax(loc_max, (PCM_LIM)fAbs(*pIn++));
        }
        loc_max_array[i] = loc_max;
        global_max = fMax(global_max, loc_max);
      }
    }

#if (PCM_LIM_BITS != 32)
    if (additionalGainAvailable) {
      /* force default mode, as simplified mode doesn't support additionalGain */
      limiter->cleanSamples = 0;
    } else
#endif
    {
      /* Check and control which mode (default/simplified) must be started */
      if ((FIXP_DBL)PCM_LIM2FIXP_DBL(global_max) > threshold) {
        limiter->cleanSamples = 0;
      } else {
        limiter->cleanSamples += nSamples;
      }
    }

    /* Simplified mode.The limiter is not active. Just handle Circular Buffer Input/Output. */
    if (limiter->cleanSamples > TDLIMIT_FALLBACK_FACTOR * limiter->release) {
      /* Copy delayed signal from delay line to output */
      UINT delayHeadLength = attack - delayBufIdx; /* delayHead: part of buffer after delayBufIdx */
      UINT delayTailLength =
          delayBufIdx; /* delayTail: part of buffer from beginning to delayBufIdx */

      UINT delayHeadCopyLength = fMin(delayHeadLength, nSamples);
      UINT delayTailCopyLength = fMin(delayTailLength, nSamples - delayHeadCopyLength);

      FIXP_DBL* p2Data = delayBuf + delayBufIdx * channels;
      INT_PCM* p2Output = samplesOut;
      for (i = 0; i < (delayHeadCopyLength * channels); i++) {
#if (SAMPLE_BITS == DFRACT_BITS)
        *p2Output++ =
            (INT_PCM)FX_DBL2FX_PCM((FIXP_DBL)SATURATE_LEFT_SHIFT(p2Data[i], scaling, DFRACT_BITS));
#else
        *p2Output++ = (INT_PCM)FX_DBL2FX_PCM((FIXP_DBL)SATURATE_LEFT_SHIFT(
            (p2Data[i] >> 1) + roundingConst, roundingScaling, DFRACT_BITS));
#endif
      }

      p2Data = delayBuf;
      for (i = 0; i < (delayTailCopyLength * channels); i++) {
#if (SAMPLE_BITS == DFRACT_BITS)
        *p2Output++ =
            (INT_PCM)FX_DBL2FX_PCM((FIXP_DBL)SATURATE_LEFT_SHIFT(p2Data[i], scaling, DFRACT_BITS));
#else
        *p2Output++ = (INT_PCM)FX_DBL2FX_PCM((FIXP_DBL)SATURATE_LEFT_SHIFT(
            (p2Data[i] >> 1) + roundingConst, roundingScaling, DFRACT_BITS));
#endif
      }

      /* Copy delayed signal that was not contained in delay line directly from input to output */
      INT delayDirectLength = (INT)nSamples - (INT)attack;
      UINT delayDirectCopyLength = fMax(delayDirectLength, (INT)0);

      p2Output = samplesOut + attack * channels;
      for (i = 0; i < (delayDirectCopyLength * channels); i++) {
        FIXP_DBL tmpSamplesIn = PCM_LIM2FIXP_DBL(samplesIn[i]);
#if (SAMPLE_BITS == DFRACT_BITS)
        *p2Output++ = (INT_PCM)FX_DBL2FX_PCM(
            (FIXP_DBL)SATURATE_LEFT_SHIFT(tmpSamplesIn, scaling, DFRACT_BITS));
#else
        *p2Output++ = (INT_PCM)FX_DBL2FX_PCM((FIXP_DBL)SATURATE_LEFT_SHIFT(
            (tmpSamplesIn >> 1) + roundingConst, roundingScaling, DFRACT_BITS));
#endif
      }

      /* Increment delayBufIdx to the position where delay line is to be filled */
      delayBufIdx = (delayBufIdx + delayDirectCopyLength) % attack;

      /* Fill delay line from input */
      delayHeadLength = attack - delayBufIdx;
      delayTailLength = delayBufIdx;

      delayHeadCopyLength = fMin(delayHeadLength, nSamples);
      delayTailCopyLength = fMin(delayTailLength, nSamples - delayHeadCopyLength);

#if PCM_LIM_BITS == DFRACT_BITS
      FDKmemcpy(delayBuf + delayBufIdx * channels, samplesIn + delayDirectCopyLength * channels,
                delayHeadCopyLength * channels * sizeof(PCM_LIM));
#else
      FIXP_DBL* p2Delay = delayBuf + delayBufIdx * channels;
      PCM_LIM* p2Input = samplesIn + delayDirectCopyLength * channels;
      for (i = 0; i < (delayHeadCopyLength * channels); i++) {
        *p2Delay++ = PCM_LIM2FIXP_DBL(p2Input[i]);
      }
#endif

#if PCM_LIM_BITS == DFRACT_BITS
      FDKmemcpy(delayBuf, samplesIn + (delayDirectCopyLength + delayHeadCopyLength) * channels,
                delayTailCopyLength * channels * sizeof(PCM_LIM));
#else
      p2Delay = delayBuf;
      p2Input = samplesIn + (delayDirectCopyLength + delayHeadCopyLength) * channels;
      for (i = 0; i < (delayTailCopyLength * channels); i++) {
        *p2Delay++ = PCM_LIM2FIXP_DBL(p2Input[i]);
      }
#endif

      /* Increment delayBufIdx after filling delay line */
      delayBufIdx = (delayBufIdx + delayHeadCopyLength + delayTailCopyLength) % attack;

      /* If we come from the default mode, then reset state */
      if (limiter->previous_mode == 1) {
        limiter->maxBufIdx = 0;
        limiter->max = (FIXP_DBL)0;
        limiter->cor = FL2FXCONST_DBL(1.0f / (1 << 1));
        limiter->smoothState0 = FL2FXCONST_DBL(1.0f / (1 << 1));
        limiter->minGain = FL2FXCONST_DBL(1.0f / (1 << 1));
        limiter->previous_mode = 0; /* Set to simplified mode */
        FDKmemset(limiter->maxBuf, 0, (limiter->attack + 1) * sizeof(FIXP_DBL));
      }

      /* Store the circular buffer pointer */
      limiter->delayBufIdx = delayBufIdx;

      /* Limit cleanSamples growth */
      limiter->cleanSamples = (TDLIMIT_FALLBACK_FACTOR + 1) * limiter->release;
    } else {
      unsigned int maxBufIdx = limiter->maxBufIdx;
      FIXP_DBL tmp, old;
      FIXP_DBL gain = FL2FXCONST_DBL(1.0f / (1 << 1));
      FIXP_DBL minGain = FL2FXCONST_DBL(1.0f / (1 << 1));
      FIXP_DBL attackConst = limiter->attackConst;
      FIXP_DBL releaseConst = limiter->releaseConst;
      FIXP_DBL cor = limiter->cor;
      FIXP_DBL smoothState0 = limiter->smoothState0;

      for (i = 0; i < nSamples; i++) {
        tmp = PCM_LIM2FIXP_DBL(loc_max_array[i]);

#if (PCM_LIM_BITS != 32)
        if (additionalGainAvailable) {
          additionalGain = pGainPerSample[i];
          tmp = fMult(tmp, additionalGain);
        }
#endif

        /* set threshold as lower border to save calculations in running maximum algorithm */
        tmp = fMax(tmp, threshold);

        /* running maximum */
        old = maxBuf[maxBufIdx];
        maxBuf[maxBufIdx] = tmp;

        if (tmp >= max) {
          /* new sample is greater than old maximum, so it is the new maximum */
          max = tmp;
        } else if (old < max) {
          /* maximum does not change, as the sample, which has left the window was
             not the maximum */
        } else {
          /* the old maximum has left the window, we have to search the complete
             buffer for the new max */
#if defined(FUNCTION_applyLimiter_func3) && (PCM_OUT_BITS == 32)
          max = applyLimiter_func3(maxBuf, attack + 1);
#else
          max = maxBuf[0];
          for (j = 1; j <= attack; j++) {
            max = fMax(max, maxBuf[j]);
          }
#endif
        }
        maxBufIdx++;
        if (maxBufIdx >= attack + 1) maxBufIdx = 0;

        /* calc gain */
        /* gain is downscaled by one, so that gain = 1.0 can be represented */
        if (max > threshold) {
          gain = fDivNorm(threshold, max) >> 1;
        } else {
          gain = FL2FXCONST_DBL(1.0f / (1 << 1));
        }

        /* gain smoothing, method: TDL_EXPONENTIAL */
        /* first order IIR filter with attack correction to avoid overshoots */

        /* correct the 'aiming' value of the exponential attack to avoid the remaining overshoot */
        if (gain < smoothState0) {
          cor =
              fMin(cor, fMultDiv2((gain - fMultDiv2(FL2FXCONST_SGL(0.1f * (1 << 1)), smoothState0)),
                                  FL2FXCONST_SGL(1.11111111f / (1 << 1)))
                            << 2);
        } else {
          cor = gain;
        }

        /* smoothing filter */
        if (cor < smoothState0) {
          smoothState0 = fMult(attackConst, (smoothState0 - cor)) + cor; /* attack */
          smoothState0 = fMax(smoothState0, gain); /* avoid overshooting target */
        } else {
          /* sign inversion twice to round towards +infinity,
             so that gain can converge to 1.0 again,
             for bit-identical output when limiter is not active */
          smoothState0 = -fMult(releaseConst, -(smoothState0 - cor)) + cor; /* release */
        }

        gain = smoothState0;

        FIXP_DBL* p_delayBuf = &delayBuf[delayBufIdx * channels + 0];
#if defined(FUNCTION_applyLimiter_func1)
        applyLimiter_func1(p_delayBuf, samplesIn, samplesOut,
                           (additionalGainAvailable == 0) ? 0 : additionalGain,
                           additionalGainAvailable, gain, scaling, channels);
#else
        for (j = 0; j < channels; j++) {
          /* Apply gain to delayed signal */
          tmp = fMult(p_delayBuf[j], gain);
#if (SAMPLE_BITS == DFRACT_BITS)
          samplesOut[j] =
              (INT_PCM)FX_DBL2FX_PCM((FIXP_DBL)SATURATE_LEFT_SHIFT(tmp, scaling + 1, DFRACT_BITS));
#else
          samplesOut[j] = (INT_PCM)FX_DBL2FX_PCM(
              (FIXP_DBL)SATURATE_LEFT_SHIFT(tmp + roundingConst, roundingScaling, DFRACT_BITS));
#endif
          /* feed delay line */
          p_delayBuf[j] = PCM_LIM2FIXP_DBL(samplesIn[j]);
#if (PCM_LIM_BITS != 32)
          if (additionalGainAvailable) {
            p_delayBuf[j] = fMult(p_delayBuf[j], additionalGain);
          }
#endif
        }
#endif

        delayBufIdx++;
        if (delayBufIdx >= attack) {
          delayBufIdx = 0;
        }

        /* save minimum gain factor */
        if (gain < minGain) {
          minGain = gain;
        }

        /* advance sample pointer by <channel> samples */
        samplesIn += channels;
        samplesOut += channels;
      }

      limiter->max = max;
      limiter->maxBufIdx = maxBufIdx;
      limiter->cor = cor;
      limiter->delayBufIdx = delayBufIdx;
      limiter->smoothState0 = smoothState0;
      limiter->minGain = minGain;
      limiter->previous_mode = 1; /*Set to default mode*/
    }

    return TDLIMIT_OK;
  }
}

/* set limiter threshold */
TDLIMITER_ERROR pcmLimiter_SetThreshold(TDLimiterPtr limiter, FIXP_DBL threshold) {
  if (limiter == NULL) return TDLIMIT_INVALID_HANDLE;

  limiter->threshold = threshold;

  return TDLIMIT_OK;
}

/* reset limiter */
TDLIMITER_ERROR pcmLimiter_Reset(TDLimiterPtr limiter) {
  if (limiter != NULL) {
    limiter->maxBufIdx = 0;
    limiter->delayBufIdx = 0;
    limiter->max = (FIXP_DBL)0;
    limiter->cor = FL2FXCONST_DBL(1.0f / (1 << 1));
    limiter->smoothState0 = FL2FXCONST_DBL(1.0f / (1 << 1));
    limiter->minGain = FL2FXCONST_DBL(1.0f / (1 << 1));
    limiter->scaling = 0;

    FDKmemset(limiter->maxBuf, 0, (limiter->attack + 1) * sizeof(FIXP_DBL));
    FDKmemset(limiter->delayBuf, 0, limiter->attack * limiter->channels * sizeof(FIXP_DBL));
  } else {
    return TDLIMIT_INVALID_HANDLE;
  }

  return TDLIMIT_OK;
}

/* destroy limiter */
TDLIMITER_ERROR pcmLimiter_Destroy(TDLimiterPtr limiter) {
  if (limiter != NULL) {
    FDKfree(limiter->maxBuf);
    FDKfree(limiter->delayBuf);

    FDKfree(limiter);
  } else {
    return TDLIMIT_INVALID_HANDLE;
  }
  return TDLIMIT_OK;
}

/* get delay in samples */
unsigned int pcmLimiter_GetDelay(TDLimiterPtr limiter) {
  FDK_ASSERT(limiter != NULL);
  return limiter->attack;
}

/* get maximum gain reduction of last processed block */
INT pcmLimiter_GetMaxGainReduction(TDLimiterPtr limiter) {
  /* maximum gain reduction in dB = -20 * log10(limiter->minGain)
     = -20 * log2(limiter->minGain)/log2(10) = -6.0206*log2(limiter->minGain) */
  int e_ans;
  FIXP_DBL loggain, maxGainReduction;

  FDK_ASSERT(limiter != NULL);

  loggain = fLog2(limiter->minGain, 1, &e_ans);

  maxGainReduction = fMult(loggain, FL2FXCONST_DBL(-6.0206f / (1 << 3)));

  return fixp_roundToInt(maxGainReduction, (e_ans + 3));
}

/* set number of channels */
TDLIMITER_ERROR pcmLimiter_SetNChannels(TDLimiterPtr limiter, unsigned int nChannels) {
  if (limiter == NULL) return TDLIMIT_INVALID_HANDLE;

  if (limiter->channels == nChannels) {
    return TDLIMIT_OK;
  }

  if (nChannels > limiter->maxChannels) return TDLIMIT_INVALID_PARAMETER;

  if (limiter->channels > nChannels) {
    for (int i = 1; i < (int)limiter->attack; i++) {
      for (int c = 0; c < (int)nChannels; c++) {
        limiter->delayBuf[i * nChannels + c] = limiter->delayBuf[i * limiter->channels + c];
      }
    }
  } else if (limiter->channels < nChannels) {
    for (int i = (int)limiter->attack - 1; i >= 0; i--) {
      for (int c = (int)limiter->channels - 1; c >= 0; c--) {
        limiter->delayBuf[i * nChannels + c] = limiter->delayBuf[i * limiter->channels + c];
      }
      for (int c = nChannels - 1; c >= (int)limiter->channels; c--) {
        limiter->delayBuf[i * nChannels + c] = (FIXP_DBL)0;
      }
    }
  }
  limiter->channels = nChannels;

  return TDLIMIT_OK;
}

/* set sampling rate */
TDLIMITER_ERROR pcmLimiter_SetSampleRate(TDLimiterPtr limiter, UINT sampleRate) {
  unsigned int attack, release;
  FIXP_DBL attackConst, releaseConst, exponent;
  INT e_ans;

  if (limiter == NULL) return TDLIMIT_INVALID_HANDLE;

  if (sampleRate > limiter->maxSampleRate) return TDLIMIT_INVALID_PARAMETER;

  /* update attack and release time in samples */
  attack = (unsigned int)(limiter->attackMs * sampleRate / 1000);
  if (attack == 0) return TDLIMIT_INVALID_PARAMETER;

  release = (unsigned int)(limiter->releaseMs * sampleRate / 1000);

  /* attackConst = pow(0.1, 1.0 / (attack + 1)) */
  exponent = invFixp(attack + 1);
  attackConst = fPow(FL2FXCONST_DBL(0.1f), 0, exponent, 0, &e_ans);
  attackConst = scaleValue(attackConst, e_ans);

  /* releaseConst  = (float)pow(0.1, 1.0 / (release + 1)) */
  exponent = invFixp(release + 1);
  releaseConst = fPow(FL2FXCONST_DBL(0.1f), 0, exponent, 0, &e_ans);
  releaseConst = scaleValue(releaseConst, e_ans);

  limiter->attack = attack;
  limiter->attackConst = attackConst;
  limiter->releaseConst = releaseConst;

  /* In case that the sampling rate changes, seamless switching is not supported yet */
  if (limiter->sampleRate != sampleRate) {
    pcmLimiter_Reset(limiter);
  }

  limiter->sampleRate = sampleRate;

  /* reset */
  // pcmLimiter_Reset(limiter);

  return TDLIMIT_OK;
}

/* set attack time */
TDLIMITER_ERROR pcmLimiter_SetAttack(TDLimiterPtr limiter, unsigned int attackMs) {
  unsigned int attack;
  FIXP_DBL attackConst, exponent;
  INT e_ans;

  if (limiter == NULL) return TDLIMIT_INVALID_HANDLE;

  if (attackMs > limiter->maxAttackMs) return TDLIMIT_INVALID_PARAMETER;

  /* calculate attack time in samples */
  attack = (unsigned int)(attackMs * limiter->sampleRate / 1000);
  if (attack == 0) return TDLIMIT_INVALID_PARAMETER;

  /* attackConst = pow(0.1, 1.0 / (attack + 1)) */
  exponent = invFixp(attack + 1);
  attackConst = fPow(FL2FXCONST_DBL(0.1f), 0, exponent, 0, &e_ans);
  attackConst = scaleValue(attackConst, e_ans);

  /* Reset in case that the attack time changes. Seamless switching is not supported yet */
  if (limiter->attack != attack) {
    pcmLimiter_Reset(limiter);
  }

  limiter->attack = attack;
  limiter->attackConst = attackConst;
  limiter->attackMs = attackMs;

  return TDLIMIT_OK;
}

/* set release time */
TDLIMITER_ERROR pcmLimiter_SetRelease(TDLimiterPtr limiter, unsigned int releaseMs) {
  unsigned int release;
  FIXP_DBL releaseConst, exponent;
  INT e_ans;

  if (limiter == NULL) return TDLIMIT_INVALID_HANDLE;

  /* calculate  release time in samples */
  release = (unsigned int)(releaseMs * limiter->sampleRate / 1000);

  /* releaseConst  = (float)pow(0.1, 1.0 / (release + 1)) */
  exponent = invFixp(release + 1);
  releaseConst = fPow(FL2FXCONST_DBL(0.1f), 0, exponent, 0, &e_ans);
  releaseConst = scaleValue(releaseConst, e_ans);

  limiter->releaseConst = releaseConst;
  limiter->releaseMs = releaseMs;

  return TDLIMIT_OK;
}
