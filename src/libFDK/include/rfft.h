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

/******************* Library for basic calculation routines ********************

   Author(s):   Josef Hoepfl, DSP Solutions

   Description: Fix point RFFT

*******************************************************************************/

#ifndef RFFT_H
#define RFFT_H

#include "common_fix.h"

/**
 * \brief Perform an inplace real FFT using a complex fft of half length.
 *
 * A FFT of an real input signal with N input samples results (N/2+1) complex output samples.
 * To get the (N/2+1) complex output values the input buffer has to be at least N+2 samples. If the
 * input buffer has only N samples the real part of (N/2+1) complex value is written into imaginary
 * part of first complex output value.
 *
 * \param fftLength             Length of the FFT to be calculated.
 * \param pBuffer               Pointer to Input/Output buffer. The input data must have at least 1
 * bit scale headroom.
 * \param bufferSize            Size of Input/Output buffer in bytes.
 * \param scalefactor           Pointer to an INT, which contains the current scale of the input
 * data, which is updated according to the FFT scale.
 *
 * \return                      0, on success; nonzero, on failure.
 */
INT rfft(const UINT fftLength, FIXP_DBL* const pBuffer, const UINT bufferSize,
         INT* const scalefactor);

/**
 * \brief Perform an inplace real IFFT using a complex fft of half length.
 *
 * The input buffer has to hold (N/2+1) complex spectral values. Because of symmetric issue the real
 * time signal can be calculated with the halve length of complex spectrum. The imaginary part of
 * first and (N/2+1) spectral value must be zero. Alternatively it's possible to feed the IFFT with
 * an input buffer of size N/2 complex spectral values wherein the real part of (N/2+1) spectral
 * value must be placed to the imaginary part of the first complex spectral value.
 *
 * \param fftLength             Length of the FFT to be calculated.
 * \param pBuffer               Pointer to Input/Output buffer. The input data must have at least 1
 * bit scale headroom.
 * \param bufferSize            Size of Input/Output buffer in bytes.
 * \param scalefactor           Pointer to an INT, which contains the current scale of the input
 * data, which is updated according to the FFT scale.
 *
 * \return                      0, on success; nonzero, on failure.
 */
INT irfft(const UINT fftLength, FIXP_DBL* const pBuffer, const UINT bufferSize,
          INT* const scalefactor);

void getSineTab(const UINT length, const FIXP_STP** ppSineTab, UINT* pStep);

#endif /* RFFT_H */
