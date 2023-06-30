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

/************************* MPEG-D DRC decoder library **************************

   Author(s):

   Description:

*******************************************************************************/

#include "drcDec_types.h"
#include "drcDec_rom.h"

const SCHAR deltaGain_codingProfile_0_1_huffman[24][2] = {
    {1, 2},    {3, 4},     {-63, -65}, {5, -66},   {-64, 6},  {-80, 7},  {8, 9},     {-68, 10},
    {11, 12},  {-56, -67}, {-61, 13},  {-62, -69}, {14, 15},  {16, -72}, {-71, 17},  {-70, -60},
    {18, -59}, {19, 20},   {21, -79},  {-57, -73}, {22, -58}, {-76, 23}, {-75, -74}, {-78, -77}};

const SCHAR deltaGain_codingProfile_2_huffman[48][2] = {
    {1, 2},    {3, 4},     {5, 6},     {7, 8},     {9, 10},    {11, 12},   {13, -65},  {14, -64},
    {15, -66}, {16, -67},  {17, 18},   {19, -68},  {20, -63},  {-69, 21},  {-59, 22},  {-61, -62},
    {-60, 23}, {24, -58},  {-70, -57}, {-56, -71}, {25, 26},   {27, -55},  {-72, 28},  {-54, 29},
    {-53, 30}, {-73, -52}, {31, -74},  {32, 33},   {-75, 34},  {-76, 35},  {-51, 36},  {-78, 37},
    {-77, 38}, {-96, 39},  {-48, 40},  {-50, -79}, {41, 42},   {-80, -81}, {-82, 43},  {44, -49},
    {45, -84}, {-83, -89}, {-86, 46},  {-90, -85}, {-91, -93}, {-92, 47},  {-88, -87}, {-95, -94}};

const FIXP_SGL slopeSteepness[] = {
    FL2FXCONST_SGL(-3.0518f / (float)(1 << 2)), FL2FXCONST_SGL(-1.2207f / (float)(1 << 2)),
    FL2FXCONST_SGL(-0.4883f / (float)(1 << 2)), FL2FXCONST_SGL(-0.1953f / (float)(1 << 2)),
    FL2FXCONST_SGL(-0.0781f / (float)(1 << 2)), FL2FXCONST_SGL(-0.0312f / (float)(1 << 2)),
    FL2FXCONST_SGL(-0.005f / (float)(1 << 2)),  FL2FXCONST_SGL(0.0f / (float)(1 << 2)),
    FL2FXCONST_SGL(0.005f / (float)(1 << 2)),   FL2FXCONST_SGL(0.0312f / (float)(1 << 2)),
    FL2FXCONST_SGL(0.0781f / (float)(1 << 2)),  FL2FXCONST_SGL(0.1953f / (float)(1 << 2)),
    FL2FXCONST_SGL(0.4883f / (float)(1 << 2)),  FL2FXCONST_SGL(1.2207f / (float)(1 << 2)),
    FL2FXCONST_SGL(3.0518f / (float)(1 << 2))};

const SCHAR slopeSteepness_huffman[14][2] = {{1, -57},  {-58, 2},   {3, 4},    {5, 6},    {7, -56},
                                             {8, -60},  {-61, -55}, {9, -59},  {10, -54}, {-64, 11},
                                             {-51, 12}, {-62, -50}, {-63, 13}, {-52, -53}};

const FIXP_DBL downmixCoeff[] = {
    FL2FXCONST_DBL(1.0000000000 / (float)(1 << 2)), FL2FXCONST_DBL(0.9440608763 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.8912509381 / (float)(1 << 2)), FL2FXCONST_DBL(0.8413951416 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.7943282347 / (float)(1 << 2)), FL2FXCONST_DBL(0.7498942093 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.7079457844 / (float)(1 << 2)), FL2FXCONST_DBL(0.6683439176 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.6309573445 / (float)(1 << 2)), FL2FXCONST_DBL(0.5956621435 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.5623413252 / (float)(1 << 2)), FL2FXCONST_DBL(0.5308844442 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.5011872336 / (float)(1 << 2)), FL2FXCONST_DBL(0.4216965034 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.3548133892 / (float)(1 << 2)), FL2FXCONST_DBL(0.0000000000 / (float)(1 << 2))};

const FIXP_DBL downmixCoeffV1[] = {
    FL2FXCONST_DBL(3.1622776602 / (float)(1 << 2)), FL2FXCONST_DBL(1.9952623150 / (float)(1 << 2)),
    FL2FXCONST_DBL(1.6788040181 / (float)(1 << 2)), FL2FXCONST_DBL(1.4125375446 / (float)(1 << 2)),
    FL2FXCONST_DBL(1.1885022274 / (float)(1 << 2)), FL2FXCONST_DBL(1.0000000000 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.9440608763 / (float)(1 << 2)), FL2FXCONST_DBL(0.8912509381 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.8413951416 / (float)(1 << 2)), FL2FXCONST_DBL(0.7943282347 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.7498942093 / (float)(1 << 2)), FL2FXCONST_DBL(0.7079457844 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.6683439176 / (float)(1 << 2)), FL2FXCONST_DBL(0.6309573445 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.5956621435 / (float)(1 << 2)), FL2FXCONST_DBL(0.5623413252 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.5308844442 / (float)(1 << 2)), FL2FXCONST_DBL(0.5011872336 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.4731512590 / (float)(1 << 2)), FL2FXCONST_DBL(0.4466835922 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.4216965034 / (float)(1 << 2)), FL2FXCONST_DBL(0.3981071706 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.3548133892 / (float)(1 << 2)), FL2FXCONST_DBL(0.3162277660 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.2818382931 / (float)(1 << 2)), FL2FXCONST_DBL(0.2511886432 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.1778279410 / (float)(1 << 2)), FL2FXCONST_DBL(0.1000000000 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.0562341325 / (float)(1 << 2)), FL2FXCONST_DBL(0.0316227766 / (float)(1 << 2)),
    FL2FXCONST_DBL(0.0100000000 / (float)(1 << 2)), FL2FXCONST_DBL(0.0000000000 / (float)(1 << 2))};

const FILTER_BANK_PARAMETERS filterBankParameters[] = {
    {FL2FXCONST_DBL(2.0f / 1024.0f), FL2FXCONST_DBL(0.0000373252f), FL2FXCONST_DBL(0.9913600345f)},
    {FL2FXCONST_DBL(3.0f / 1024.0f), FL2FXCONST_DBL(0.0000836207f), FL2FXCONST_DBL(0.9870680830f)},
    {FL2FXCONST_DBL(4.0f / 1024.0f), FL2FXCONST_DBL(0.0001480220f), FL2FXCONST_DBL(0.9827947083f)},
    {FL2FXCONST_DBL(5.0f / 1024.0f), FL2FXCONST_DBL(0.0002302960f), FL2FXCONST_DBL(0.9785398263f)},
    {FL2FXCONST_DBL(6.0f / 1024.0f), FL2FXCONST_DBL(0.0003302134f), FL2FXCONST_DBL(0.9743033527f)},
    {FL2FXCONST_DBL(2.0f / 256.0f), FL2FXCONST_DBL(0.0005820761f), FL2FXCONST_DBL(0.9658852897f)},
    {FL2FXCONST_DBL(3.0f / 256.0f), FL2FXCONST_DBL(0.0012877837f), FL2FXCONST_DBL(0.9492662926f)},
    {FL2FXCONST_DBL(2.0f / 128.0f), FL2FXCONST_DBL(0.0022515827f), FL2FXCONST_DBL(0.9329321561f)},
    {FL2FXCONST_DBL(3.0f / 128.0f), FL2FXCONST_DBL(0.0049030350f), FL2FXCONST_DBL(0.9010958535f)},
    {FL2FXCONST_DBL(2.0f / 64.0f), FL2FXCONST_DBL(0.0084426929f), FL2FXCONST_DBL(0.8703307793f)},
    {FL2FXCONST_DBL(3.0f / 64.0f), FL2FXCONST_DBL(0.0178631928f), FL2FXCONST_DBL(0.8118317459f)},
    {FL2FXCONST_DBL(2.0f / 32.0f), FL2FXCONST_DBL(0.0299545822f), FL2FXCONST_DBL(0.7570763753f)},
    {FL2FXCONST_DBL(3.0f / 32.0f), FL2FXCONST_DBL(0.0604985076f), FL2FXCONST_DBL(0.6574551915f)},
    {FL2FXCONST_DBL(2.0f / 16.0f), FL2FXCONST_DBL(0.0976310729f), FL2FXCONST_DBL(0.5690355937f)},
    {FL2FXCONST_DBL(3.0f / 16.0f), FL2FXCONST_DBL(0.1866943331f), FL2FXCONST_DBL(0.4181633458f)},
    {FL2FXCONST_DBL(2.0f / 8.0f), FL2FXCONST_DBL(0.2928932188f), FL2FXCONST_DBL(0.2928932188f)}};
