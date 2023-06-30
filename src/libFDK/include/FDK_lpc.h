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

   Author(s):   Manuel Jander

   Description: LPC related functions

*******************************************************************************/

#ifndef FDK_LPC_H
#define FDK_LPC_H

#include "common_fix.h"

#define LPC_MAX_ORDER 24

/*
 * Experimental solution for lattice filter substitution.
 * LPC_SYNTHESIS_IIR macro must be activated in aacdec_tns.cpp.
 * When LPC_SYNTHESIS_IIR enabled, there will be a substitution of the default lpc synthesis lattice
 * filter by an IIR synthesis filter (with a conversionof the filter coefs). LPC_TNS related macros
 * are intended to implement the data types used by the CLpc_Synthesis variant which is used for
 * this solution.
 * */

typedef FIXP_DBL FIXP_LPC_TNS;
#define FX_DBL2FX_LPC_TNS(x) (x)
#define FX_DBL2FXCONST_LPC_TNS(x) (x)
#define FX_LPC_TNS2FX_DBL(x) (x)
#define FL2FXCONST_LPC_TNS(val) FL2FXCONST_DBL(val)
#define MAXVAL_LPC_TNS MAXVAL_DBL

typedef FIXP_SGL FIXP_LPC;
#define FX_DBL2FX_LPC(x) FX_DBL2FX_SGL((FIXP_DBL)(x))
#define FX_DBL2FXCONST_LPC(x) FX_DBL2FXCONST_SGL(x)
#define FX_LPC2FX_DBL(x) FX_SGL2FX_DBL(x)
#define FL2FXCONST_LPC(val) FL2FXCONST_SGL(val)
#define MAXVAL_LPC MAXVAL_SGL

/**
 * \brief Synthesize signal fom residual through LPC synthesis, using LP coefficients.
 * \param signal pointer to buffer holding the residual signal. The synthesis is returned there (in
 * place)
 * \param signal_size the size of the input data in pData
 * \param inc buffer traversal increment for signal
 * \param coeff the LPC filter coefficients
 * \param coeff_e exponent of coeff
 * \param order the LPC filter order (size of coeff)
 * \param state state buffer of size LPC_MAX_ORDER
 * \param pStateIndex pointer to state index storage
 */
void CLpc_Synthesis(FIXP_DBL* signal, const int signal_size, const int inc,
                    const FIXP_LPC_TNS* lpcCoeff_m, const int lpcCoeff_e, const int order,
                    FIXP_DBL* state, int* pStateIndex);
void CLpc_Synthesis(FIXP_DBL* signal, const int signal_size, const int inc, const FIXP_LPC* coeff_m,
                    const int coeff_e, const int order, FIXP_DBL* filtState, int* pStateIndex);

/**
 * \brief Synthesize signal fom residual through LPC synthesis, using ParCor coefficients. The
 * algorithm assumes a filter gain of max 1.0. If the filter gain is higher, this must be accounted
 * into the values of signal_e and/or signal_e_out to avoid overflows.
 * \param signal pointer to buffer holding the residual signal. The synthesis is returned there (in
 * place)
 * \param signal_size the size of the input data in pData
 * \param inc buffer traversal increment for signal
 * \param coeff the LPC filter coefficients
 * \param coeff_e exponent of coeff
 * \param order the LPC filter order (size of coeff)
 * \param state state buffer of size LPC_MAX_ORDER
 */

void CLpc_SynthesisLattice(FIXP_DBL* signal, const int signal_size, const int signal_e,
                           const int signal_e_out, const int inc, const FIXP_DBL* coeff,
                           const int order, FIXP_DBL* state);

/**
 * \brief Convert parcor coefficients to lpc coefficients.
 *        Data type of parcor and lpc coefficients is FIXP_DBL.
 */
INT CLpc_ParcorToLpc(const FIXP_LPC_TNS reflCoeff[], FIXP_LPC_TNS LpcCoeff[], const int numOfCoeff,
                     FIXP_QDL workBuffer[]);
/**
 * \brief Convert parcor coefficients to lpc coefficients.
 *        Data type of parcor and lpc coefficients is FIXP_SGL.
 */
INT CLpc_ParcorToLpc(const FIXP_LPC reflCoeff[], FIXP_LPC LpcCoeff[], const int numOfCoeff,
                     FIXP_QDL workBuffer[]);

/**
 * \brief Calculate ParCor (Partial autoCorrelation, reflection) coefficients from autocorrelation
 *        coefficients using the Schur algorithm (instead of Levinson Durbin).
 * \param acorr order+1 autocorrelation coefficients
 * \param reflCoeff output reflection /ParCor coefficients. The first coefficient which is
 * always 1.0 is ommitted.
 * \param order number of acorr / reflCoeff coefficients.
 * \param pPredictionGain_m prediction gain mantissa
 * \param pPredictionGain_e prediction gain exponent
 */
void CLpc_AutoToParcor(FIXP_DBL acorr[], const int acorr_e, FIXP_LPC reflCoeff[], const int order,
                       FIXP_DBL* pPredictionGain_m, INT* pPredictionGain_e);

/**
 * \brief Calculate the autocorrelation of any order and size
 * \param signal input signal
 * \param acorr pointer to output buffer of size "order" or bigger.
 * \param signal_size the size of the input signal "signal".
 * \param order the order of the autocorrelation to be calculated.
 * \param scale a right shift to be applied to the autocorrelation output.
 * \return energy ( = acorr[0] without normalization).
 */
FIXP_DBL CLpc_AutoCorr(const FIXP_DBL* signal, const int signal_e, FIXP_DBL* acorr, INT* pAcorr_e,
                       const int signal_size, const int order);

/**
 * \brief Levinson-Durbin algorithm to compute
 *        the LPC parameters from given autocorrelation value.
 * \param lpcCoeff_m pointer to where the mantissas of output LPC coeffcients will be stored into.
 * \param lpcCoeff_e Pointer to where the exponent of lpcCoeff_m will be stored into.
 * \param R pointer to m+1 input autocorrelation coefficients.
 * \param m LPC order
 * \param rc pointer to where reflection will be stored into (currently not implemened).
 */
void CLpc_AutoToLpc(FIXP_LPC* lpcCoeff_m, INT* lpcCoeff_e, const FIXP_DBL* R, const INT R_e,
                    const int m, FIXP_DBL* rc, const int is_ltpf);

void CLpc_AutoToLpcIGF(FIXP_DBL* lpcCoeff_m, INT* lpcCoeff_e, const FIXP_DBL* R, const INT R_e,
                       const int m, FIXP_DBL* rc);

#endif /* FDK_LPC_H */
