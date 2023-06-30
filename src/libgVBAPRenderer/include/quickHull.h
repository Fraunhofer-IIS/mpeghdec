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

/******************** MPEG-H 3DA object rendering library **********************

   Author(s):   Thomas Blender

   Description: Quickhull algorithm to calculate triangulation meshes
                of the convex hull around the given loudspeaker setup

*******************************************************************************/

#ifndef QUICKHULL_H
#define QUICKHULL_H

#include "cartesianMath.h"
#include "vbap_core.h"
#include "FDK_core.h"
#include "FDK_matrixCalloc.h"

#define FX_DBL_DEGREE2INT                                                                  \
  11930464 /* 11930464,7 corresponds to 1 degree in FIXP_DBL domain. Bring down because of \
              rounding reasons in init_vertex*/

#define GVBAPRENDERER_VERTEX_LIST_MAX_SIZE 28 + 2   /* 2 is a value of safety */
#define GVBAPRENDERER_TRIANGLE_LIST_MAX_SIZE 49 + 2 /* 2 is a value of safety */

#define D2F(val) FL2FXCONST_DBL(val / 180.0) /* convert a degree value to fixpoint */

/* quadrilateral front and rear subsets */
const FIXP_DBL rangeOfSubsetA[16] = {
    D2F(+23.0), D2F(+37.0), D2F(-9.0),  D2F(+20.0),  /* CH_M_L030 */
    D2F(-37.0), D2F(-23.0), D2F(-9.0),  D2F(+20.0),  /* CH_M_R030 */
    D2F(-37.0), D2F(-11.0), D2F(+21.0), D2F(+60.0),  /* CH_U_R030 */
    D2F(+11.0), D2F(+37.0), D2F(+21.0), D2F(+60.0)}; /* CH_U_L030 */

const FIXP_DBL rangeOfSubsetB[16] = {
    D2F(+38.0), D2F(+52.0), D2F(-9.0),  D2F(+20.0),  /* CH_M_L045 */
    D2F(-52.0), D2F(-38.0), D2F(-9.0),  D2F(+20.0),  /* CH_M_R045 */
    D2F(-66.0), D2F(-38.0), D2F(+21.0), D2F(+60.0),  /* CH_U_R045 */
    D2F(+38.0), D2F(+66.0), D2F(+21.0), D2F(+60.0)}; /* CH_U_L045 */

const FIXP_DBL rangeOfSubsetC[16] = {
    D2F(+101.0), D2F(+124.0), D2F(-45.0), D2F(+20.0),  /* CH_M_L110 */
    D2F(+101.0), D2F(+124.0), D2F(+21.0), D2F(+60.0),  /* CH_U_L110 */
    D2F(-124.0), D2F(-101.0), D2F(+21.0), D2F(+60.0),  /* CH_U_R110 */
    D2F(-124.0), D2F(-101.0), D2F(-45.0), D2F(+20.0)}; /* CH_M_R110 */

const FIXP_DBL rangeOfSubsetD[16] = {
    D2F(+125.0), D2F(+142.0), D2F(-45.0), D2F(+20.0),  /* CH_M_L135 */
    D2F(+125.0), D2F(+157.0), D2F(+21.0), D2F(+60.0),  /* CH_U_L135 */
    D2F(-157.0), D2F(-125.0), D2F(+21.0), D2F(+60.0),  /* CH_U_R135 */
    D2F(-142.0), D2F(-125.0), D2F(-45.0), D2F(+20.0)}; /* CH_M_R135 */

/*
 * @brief Vertex struct defined by its three Cartesian coordinates
 * @param xyz       Cartesian coordinate
 * @param index     sorting Index
 * @param sph       Azimuth
 */
typedef struct _vertex {
  PointCartesian xyz;
  int index;
  PointSpherical sph;

} vertex;

/*
 * @brief Vertex List
 * @param size      size of List
 * @param element   vertex containing:
 *                    xyz       Cartesian coordinates
 *                    index     sorting Index
 *                    sph       Azimuth
 */
typedef struct _vertexList {
  int size;
  int limit;
  vertex element[GVBAPRENDERER_VERTEX_LIST_MAX_SIZE];

} vertexList;

/*
 * @brief Triangle struct defined by a set of three Vertex indices
 * @param three corner vertices
 */
typedef struct _triangle {
  int index[3];
} triangle;

/*
 * @brief Triangle List
 * @param size      size of List
 * @param element   three corner vertices
 */
typedef struct _triangleList {
  int size;
  int limit;
  triangle element[GVBAPRENDERER_TRIANGLE_LIST_MAX_SIZE];

} triangleList;

/*
 * @brief Stores surround speaker below 45 degrees elevation
 * @param azi       Azimuth
 * @param index     Speaker index in original vertexList
 */
typedef struct _surroundList {
  FIXP_DBL azi;
  int index;

} surroundList;

/*! \brief imagSpeakerTable struct */
/*!
 Structure that contains pre-calculated data for imaginary loudspeakers.
 */
typedef struct imagSpeakerTableStruct {
  int numGhost;                /*!< number of imaginary/ghost loudspeakers */
  const int* aziGhost;         /*!< array with azimuth angles of the imaginary loudspeakers */
  const int* eleGhost;         /*!< array with elevation angles of the imaginary loudspeakers */
  const int* initialPoly;      /*!< array with vertex indices of the initial polyhedron*/
  int numRow;                  /*!< number of rows of the downmix matrix */
  int numCol;                  /*!< number of colons of the downmix matrix */
  const FIXP_DBL* downMixData; /*!< sequential list of the coefficients of the downmix matrix */
} imagSpeakerTable;

typedef struct speakerSuperSetStruct {
  int supersetCicpIndex;       /*!< Index of loudspeaker superset */
  int nSpeakersSuperSet;       /*!< Number of speakers in the superset (not counting in LFEs) */
  int nSpeakersSubSet;         /*!< Number of speakers in the subset (not counting in LFEs) */
  const FIXP_DBL* downMixData; /*!< Sequential list of the coefficients of the superset-to-subset
                                  downmix matrix */
} speakerSuperSetTable;

/******************************************
 *          API Functions                 *
 ******************************************/

/*
 * @brief Fills a vertexList vL with given a number of speakers and speaker positions (degree)
 * @param vL Output Vertex list with spherical and Cartesian speaker positions and a sorting index
 * @param numSpk Number of speakers
 * @param az Azimuth   fixpoint value of speaker position. Value is mapped to -1 ... 1 for -180 ...
 * 180 degrees or -pi .. pi
 * @param el Elevation fixpoint value of speaker position. Value is mapped to -1 ... 1 for -180 ...
 * 180 degrees or -pi .. pi
 */
void qh_gen_VertexList(int numSpk, FIXP_DBL* az, FIXP_DBL* el, vertexList* vL);

/*
 * @brief Fills a triangleList tL with speaker triangles by given vertexList vL
 *        and generates downmix matrix if ghost speakers where added
 * @param vL Input Vertex list with spherical and Cartesian speaker positions and a sorting index
 * @param tL Output triangleList
 * @return error code
 */
int qh_sphere_triangulation(int cicpIndex, vertexList* vL, triangleList* tL,
                            FIXP_DBL*** downmixMatrix, int* downmixMatrixNumRows,
                            int* downmixMatrixNumCols);

/******************************************
 *        list functions                  *
 ******************************************/

/*
 * @brief reset initial values of vertexList vL
 */
void resetVertexList(vertexList* vL);

/*
 * @brief reset initial values of triangleList tL
 */
void resetTriangleList(triangleList* tL);

/*
 * @brief adds element v to a given vertexList vL
 */
void addVertexToList(vertexList* vL, vertex v);

/*
 * @brief adds element t to a given triangleList tL
 */
void addTriangleToList(triangleList* tL, triangle t);

/*
 * @brief Sort indices of an edge in an ascending fashion
 * @param edgeList is a pointer to the position of list where the next 2 elements will be sorted
 */
void sortEdgeList(int* edgeList);

/*
 * @brief Add new vertex vtx to a convex hull. The convex hull is given by the triangles tL. The
 * plane orientation has to be counter clockwise and the result is guaranteed to be counter
 * clockwise as well. The given convex hull has to cover at least a hemisphere.
 * @param vL Vertex list with coordinates.
 * @param tL List of indices of plane vertices.
 * @param vtx New vertex.
 */
void add_vertex(vertexList* vL, triangleList* tL, int vtx);

/*
 * @brief Initialize vertex including the sorting index.
 * @param azimuth Azimuth angle in degree
 * @param elevation Elevation angle in degree
 */
vertex init_vertex(FIXP_DBL azimuth, FIXP_DBL elevation);

/*
 *  @brief Add Speakers by the 5 rules in the definition
 *  @param vertexList vL of real speaker positions
 *  @return modified vertexList with added Ghosts
 *  @return indices of initial pentagon (length have to be 5)
 */
int add_imaginary_speakers(vertexList* vL, int* pentagon);

/*
 * \brief Tests whether a subset exists and if it is empty or not.
 *        If an empty subset was found a ghost speaker will be added
 *
 * \param vL vertexList of all speakers
 * \param subsetRange is one of the defined subsetrange in quickHull.h
 * \param frontSide indicates whether the subset is a front or back side subset
 * Returns 0 if no ghost speaker was added
 * Returns 1 if a ghost speaker was added
 */
int addGhostToSubset(vertexList* vL, const FIXP_DBL* subsetRange, bool frontSide);

/*
 * @brief calculates the distance between a point p and a plane spanned by the points x1, x2, x3
 * @param x1, x2, x3, p Cartesian points. All exponents have to be the same.
 *
 * @return scalar value of distance with sign as orientation. Result has a exponent of 1.
 */
FIXP_DBL distPointPlane(PointCartesian x1, PointCartesian x2, PointCartesian x3, PointCartesian p);

#endif
