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

#include "quickHull.h"
#include "tables.h"

#include "FDK_cicp2geometry.h"

void add_imaginary_speakers_cicp(int cicpIndex, vertexList* vL, int* idx);
static CICP2GEOMETRY_ERROR augment_setup_to_superset(int* cicpIdx, vertexList* vL,
                                                     FIXP_DBL*** downmix_mat);
#include <stdlib.h> /* qsort */

/******************************************
 *   Public API Functions                 *
 ******************************************/

/*
 * @brief Fills a vertexList vL with a given number of speakers and speaker positions (degree)
 * @param vL Output Vertex list with spherical and Cartesian speaker positions and a sorting index
 * @param numSpk Number of speakers
 * @param az Azimuth   fixpoint value of speaker positions. Value is mapped to -1 ... 1 for -180 ...
 * 180 degrees or -pi .. pi
 * @param el Elevation fixpoint value of speaker positions. Value is mapped to -1 ... 1 for -180 ...
 * 180 degrees or -pi .. pi
 */
void qh_gen_VertexList(int numSpk, FIXP_DBL* az, FIXP_DBL* el, vertexList* vL) {
  int i;

  for (i = 0; i < numSpk; i++) {
    addVertexToList(vL, init_vertex(az[i], el[i]));
  }
}

/*
 * @brief surroundList comparison function for sorting the floats.
 * @param a First item.
 * @param b Second item.
 * @return c Item difference.
 */
extern "C" int aziCmp(const void* a, const void* b);

/*
 * @brief Fills a triangleList tL with speaker triangles by given vertexList vL
 *        and generates a downmix matrix if ghost speakers where added
 * @param vL Input Vertex list with spherical and Cartesian speaker positions and a sorting index
 * @param tL Output triangleList
 * @return error code
 */
int qh_sphere_triangulation(int cicpIndex, vertexList* vL, triangleList* tL,
                            FIXP_DBL*** downmixMatrix, int* downmixMatrixNumRows,
                            int* downmixMatrixNumCols) {
  int realSpeaker = vL->size;      /* store number of real speakers */
  int subsetSpeaker = realSpeaker; /* number of subset speakers */
  int pentagon[5];
  int i, j, k;
  int cnt;
  int err = 0;
  triangle tri;
  FIXP_DBL max;
  FIXP_DBL** downmixMatrixSuperset = NULL;
  FIXP_DBL** mSquare = NULL;
  FIXP_DBL** adjacencyMatrix = NULL;
  FIXP_DBL inverse_cnt; /* must be DBL never change to SGL! */
  surroundList* positionIndexList =
      NULL; /* use surroundlist as double integer list for sorting vL by index */

  if (cicpIndex) {
    if (augment_setup_to_superset(&cicpIndex, vL, &downmixMatrixSuperset)) {
      /*memory allocation error*/
      err = 1;
      goto bail;
    }
    realSpeaker = vL->size;
  }

  /* Add imaginary speakers and get initial pentagon as convex hull */
  if (cicpIndex) {
    add_imaginary_speakers_cicp(cicpIndex, vL, &pentagon[0]);
  } else {
    if (add_imaginary_speakers(vL, &pentagon[0])) {
      /*memory allocation error*/
      err = 1;
      goto bail;
    }
  }

  /* Add first initial polyhedron to triangle list*/
  for (i = 0; i < 2; i++) {
    tri.index[0] = pentagon[0];
    tri.index[1] = pentagon[i + 2];
    tri.index[2] = pentagon[i + 3];
    addTriangleToList(tL, tri);
  }
  tri.index[0] = pentagon[0];
  tri.index[1] = pentagon[4];
  tri.index[2] = pentagon[2];
  addTriangleToList(tL, tri);

  for (i = 0; i < 2; i++) {
    tri.index[0] = pentagon[1];
    tri.index[1] = pentagon[i + 3];
    tri.index[2] = pentagon[i + 2];
    addTriangleToList(tL, tri);
  }
  tri.index[0] = pentagon[1];
  tri.index[1] = pentagon[2];
  tri.index[2] = pentagon[4];
  addTriangleToList(tL, tri);

  positionIndexList = (surroundList*)FDKcalloc(vL->size, sizeof(surroundList));

  if (!positionIndexList) {
    /*memory allocation error*/
    err = 1;
    goto bail;
  }

  /* Sorting the order of vertices by index and save the order in positionIndexList
   * Where index is the position in the vertexList vL and azi is the calculated sorting index */
  for (i = 0; i < vL->size; i++) {
    positionIndexList[i].index = i;
    positionIndexList[i].azi = (FIXP_DBL)vL->element[i].index;
  }

  qsort(positionIndexList, vL->size, sizeof(surroundList), aziCmp);

  for (i = 0; i < vL->size; i++) {
    cnt = positionIndexList[i].index;
    add_vertex(vL, tL, cnt);
  }

  /* if some ghost speaker was added then calculate downmix matrix */
  if (vL->size > realSpeaker) {
    if (cicpIndex) {
      const imagSpeakerTable* hTable = getImagSpeakerTable(cicpIndex);
      int row, col;

      if (hTable) {
        /* copy downmix matrix from imaginary speaker table */
        *downmixMatrix =
            (FIXP_DBL**)fdkCallocMatrix2D(hTable->numRow, hTable->numCol, sizeof(FIXP_DBL));
        if (*downmixMatrix == NULL) {
          /*memory allocation error*/
          err = 1;
          goto bail;
        }
        *downmixMatrixNumRows = hTable->numRow;
        *downmixMatrixNumCols = hTable->numCol;
        for (row = 0; row < hTable->numRow; row++) {
          for (col = 0; col < hTable->numCol; col++) {
            (*downmixMatrix)[row][col] = hTable->downMixData[col + row * hTable->numCol];
          }
        }
      }
    }

    if (!(*downmixMatrix)) {
      *downmixMatrix = (FIXP_DBL**)fdkCallocMatrix2D(
          realSpeaker, vL->size, sizeof(FIXP_DBL)); /* Dim: realSpeaker x (realSpeaker + ghost) */
      adjacencyMatrix = (FIXP_DBL**)fdkCallocMatrix2D(
          vL->size, vL->size,
          sizeof(FIXP_DBL)); /* Dim: (realSpeaker + ghost) x (realSpeaker + ghost) */
      mSquare = (FIXP_DBL**)fdkCallocMatrix2D(
          vL->size, vL->size,
          sizeof(FIXP_DBL)); /* Dim: (realSpeaker + ghost) x (realSpeaker + ghost) */
      *downmixMatrixNumRows = realSpeaker;
      *downmixMatrixNumCols = vL->size;

      if (!*downmixMatrix || !adjacencyMatrix || !mSquare) {
        /*memory allocation error*/
        err = 1;
        goto bail;
      }

      /* create adjacencyMatrix */
      for (i = 0; i < vL->size; i++) /* go through the whole adjacencyMatrix */
      {
        if (i < realSpeaker) /* for real speakers set main diagonal to 1.0 */
          adjacencyMatrix[i][i] = (FIXP_DBL)MAXVAL_DBL; /* Set to 1.0 */
        else /* for ghost speakers search neighbours and mark them in the adjacencyMatrix with 1.0
              */
        {
          /* go through all Triangles */
          for (k = 0; k < tL->size; k++) {
            if ((tL->element[k].index[0] == i) || (tL->element[k].index[1] == i) ||
                (tL->element[k].index[2] == i)) /* if a neighbour to ghostspeaker i is found */
            {
              if (tL->element[k].index[0] !=
                  i) /* indicate neighbourhood only if the element is not itself */
                adjacencyMatrix[tL->element[k].index[0]][i] = (FIXP_DBL)MAXVAL_DBL; /* Set to 1.0 */
              if (tL->element[k].index[1] !=
                  i) /* indicate neighbourhood only if the element is not itself */
                adjacencyMatrix[tL->element[k].index[1]][i] = (FIXP_DBL)MAXVAL_DBL; /* Set to 1.0 */
              if (tL->element[k].index[2] !=
                  i) /* indicate neighbourhood only if the element is not itself */
                adjacencyMatrix[tL->element[k].index[2]][i] = (FIXP_DBL)MAXVAL_DBL; /* Set to 1.0 */
            }
          }
        }

        /* normalize ghost columns */
        cnt = 0;
        for (k = 0; k < vL->size;
             k++) /* counting to how many speakers the ghost energy have to distributed */
          if (adjacencyMatrix[k][i] > (FIXP_DBL)0) cnt++;
        if (cnt > 0) {
          inverse_cnt = GetInvInt(cnt);  /* Only values for cnt below 80 are allowed! */
          for (k = 0; k < vL->size; k++) /* normalize the distributed energy */
            if (adjacencyMatrix[k][i] > (FIXP_DBL)0)
              adjacencyMatrix[k][i] = fMult(
                  adjacencyMatrix[k][i], inverse_cnt); /* we don't want to use a fixpoint division,
                                                          that's why using GetInvInt. */
        }
      }

      /* distribute the energy of ghost speakers to real speakers */
      /* the lower rows belong to the ghost speaker have to be as near as possible to 0 */
      for (cnt = 0; cnt < 20; cnt++) /* The maximum number of iterations we do is 20 */
      {
        /* Multiply adjacencyMatrix by itself and sort the result in mSquare */
        for (i = 0; i < vL->size; i++) {
          for (k = 0; k < vL->size; k++) {
            mSquare[i][k] = (FIXP_DBL)0;
            for (j = 0; j < vL->size; j++)
              mSquare[i][k] = mSquare[i][k] + fMult(adjacencyMatrix[i][j], adjacencyMatrix[j][k]);
          }
        }

        /* copy mSquare to adjacencyMatrix */
        for (i = 0; i < vL->size; i++)
          for (j = 0; j < vL->size; j++) adjacencyMatrix[i][j] = mSquare[i][j];

        /* Search the highest value in the submatrix which holds the values for the ghostspeakers.
         * It is the part which maps ghosts to ghosts. It starts at row realspeaker and at column
         * realspeaker and goes to the end */
        max = (FIXP_DBL)0; /* stores the greatest value */
        for (i = realSpeaker; i < vL->size; i++)
          for (j = realSpeaker; j < vL->size; j++)
            if (adjacencyMatrix[i][j] > max) max = adjacencyMatrix[i][j];

        /* if the highest value is lower than 0.0001f we stop iteration */
        if (max <= (FIXP_DBL)FL2FXCONST_DBL(0.0001)) break;
      }

      /* copy adjacencyMatrix to downmixMatrix but cut the ghost rows off */
      for (i = 0; i < realSpeaker; i++)
        for (j = 0; j < vL->size; j++) (*downmixMatrix)[i][j] = adjacencyMatrix[i][j];

      /* Power Normalize */
      for (i = 0; i < realSpeaker; i++)
        for (j = realSpeaker; j < vL->size;
             j++) /* start at realSpeaker position should work, because before this position we have
                     a identity submatrix */
          if (((*downmixMatrix)[i][j] != (FIXP_DBL)0) &&
              ((*downmixMatrix)[i][j] !=
               (FIXP_DBL)MAXVAL_DBL)) /* calculate square root only for non trivial values */
            (*downmixMatrix)[i][j] = sqrtFixp((*downmixMatrix)[i][j]);
    }

    if (downmixMatrixSuperset) {
      /* multiply superset downmix matrix with imaginary speaker downmix matrix */
      FIXP_DBL** downmixMatrixImaginaries = *downmixMatrix;
      *downmixMatrix = (FIXP_DBL**)fdkCallocMatrix2D(subsetSpeaker, vL->size, sizeof(FIXP_DBL));
      if (*downmixMatrix == NULL) {
        /*memory allocation error*/
        fdkFreeMatrix2D((void**)downmixMatrixImaginaries);
        err = 1;
        goto bail;
      }
      *downmixMatrixNumRows = subsetSpeaker;
      *downmixMatrixNumCols = vL->size;

      for (i = 0; i < subsetSpeaker; i++) {
        for (j = 0; j < vL->size; j++) {
          FIXP_DBL tmp = 0;
          for (k = 0; k < realSpeaker; k++) {
            tmp += fMultDiv2(downmixMatrixSuperset[i][k], downmixMatrixImaginaries[k][j]);
          }

          (*downmixMatrix)[i][j] = tmp;
        }
      }

      fdkFreeMatrix2D((void**)downmixMatrixImaginaries);
    }
  }
bail:
  fdkFreeMatrix2D((void**)downmixMatrixSuperset);
  fdkFreeMatrix2D((void**)adjacencyMatrix);
  fdkFreeMatrix2D((void**)mSquare);
  FDKfree(positionIndexList);

  return err;
}

/******************************************
 *   Private Functions                     *
 ******************************************/

/*
 * @brief all list functions
 *
 */

void resetVertexList(vertexList* vL) {
  vL->size = 0;
  vL->limit = GVBAPRENDERER_VERTEX_LIST_MAX_SIZE;
}

void addVertexToList(vertexList* vL, vertex v) {
  /* adding more elements than allocated memory is available is not allowed */
  FDK_ASSERT(vL->size < vL->limit);

  vL->element[vL->size] = v;
  vL->size = vL->size + 1;
}

void resetTriangleList(triangleList* tL) {
  tL->size = 0;
  tL->limit = GVBAPRENDERER_TRIANGLE_LIST_MAX_SIZE;
}

void addTriangleToList(triangleList* tL, triangle t) {
  /* adding more elements than allocated memory is available is not allowed */
  FDK_ASSERT(tL->size < tL->limit);

  tL->element[tL->size] = t;
  tL->size = tL->size + 1;
}

/*
 * @brief Add a new vertex vtx to a convex hull. The convex hull is given by the triangles tL. The
 * plane orientation has to be counter clockwise and the result is guaranteed to be counter
 * clockwise as well. The given convex hull has to cover at least a hemisphere.
 * @param vL Vertex list with coordinates.
 * @param tL List of indices of plane vertices.
 * @param vtx New vertex.
 */
void add_vertex(vertexList* vL, triangleList* tL, int vtx) {
  int i, k;
  int a, b, c;
  int duplicate;
  triangle tmp;
  PointCartesian origin;
  FIXP_DBL distance;

  triangleList tListNew, tListVisible;
  triangleList* tLnew = &tListNew;
  triangleList* tLvis = &tListVisible;

  resetTriangleList(tLnew);
  resetTriangleList(tLvis);

  for (i = 0; i < tL->size; i++) {
    a = tL->element[i].index[0];
    b = tL->element[i].index[1];
    c = tL->element[i].index[2];
    distance = distPointPlane(vL->element[a].xyz, vL->element[b].xyz, vL->element[c].xyz,
                              vL->element[vtx].xyz); /* exponent is 1 */
    if (distance > FL2FX_DBL(-0.0001))               /* ref code value is -0.00001 */
    {
      /* tLvis list is to short to add one more element */
      FDK_ASSERT(tLvis->size < tLvis->limit);

      tLvis->element[tLvis->size] = tL->element[i];
      tLvis->size++;
    } else {
      /* tLnew list is to short to add one more element */
      FDK_ASSERT(tLnew->size < tLnew->limit);

      tLnew->element[tLnew->size] = tL->element[i];
      tLnew->size++;
    }
  }

  /* Extracts visible edges for a given set of triangles.
   * Essentially, all edges which occurs exactly once in the set of triangles are visible. */
  int edgeList[GVBAPRENDERER_TRIANGLE_LIST_MAX_SIZE * 3 * 2];

  /* create all possible edges */
  for (i = 0; i < tLvis->size; i++) {
    edgeList[6 * i + 0] = tLvis->element[i].index[0];
    edgeList[6 * i + 1] = tLvis->element[i].index[1];
    sortEdgeList(&edgeList[6 * i + 0]); /* Sort for easier comparability */
    edgeList[6 * i + 2] = tLvis->element[i].index[0];
    edgeList[6 * i + 3] = tLvis->element[i].index[2];
    sortEdgeList(&edgeList[6 * i + 2]); /* Sort for easier comparability */
    edgeList[6 * i + 4] = tLvis->element[i].index[1];
    edgeList[6 * i + 5] = tLvis->element[i].index[2];
    sortEdgeList(&edgeList[6 * i + 4]); /* Sort for easier comparability */
  }

  /* delete those which are doubles */
  a = 0; /* counter for visible edges */
  for (i = 0; i < (tLvis->size) * 3; i++) {
    if (edgeList[2 * i + 0] == -1) /* old duplicate */
      continue;

    duplicate = 0;
    for (k = i + 1; k < (tLvis->size) * 3; k++) {
      if ((edgeList[2 * i + 0] == edgeList[2 * k + 0]) &&
          (edgeList[2 * i + 1] == edgeList[2 * k + 1])) {
        edgeList[2 * k + 0] = -1; /* do not insert break. We have to mark all duplicates pairs */
        /* edgeList[2*k + 1] = -1; */
        duplicate = 1; /* duplicated element found. => no visible edge */
      }
    }
    if (!duplicate) {
      edgeList[2 * a + 0] = edgeList[2 * i + 0]; /* copy visible edges. Store it in existing list */
      edgeList[2 * a + 1] = edgeList[2 * i + 1];
      a++; /* Count visible edges. This value is at same time the length of the new list */
    }
  }

  /* Delete entries of the old triangleList tL by setting size to new size and copy new entries */
  tL->size = tLnew->size;
  /* Copy tLnew to tL */
  for (i = 0; i < tLnew->size; i++) {
    tL->element[i] = tLnew->element[i];
  }

  /* Add new values to triangleList tL */
  k = a;
  origin.x = (FIXP_DBL)0;
  origin.y = (FIXP_DBL)0;
  origin.z = (FIXP_DBL)0;
  for (i = 0; i < k; i++) {
    /* check if flipping the orientation of the plane is necessary
     * It will ensure counter clockwise order of the vertices */
    a = edgeList[2 * i + 0];
    b = edgeList[2 * i + 1];
    c = vtx;
    distance = distPointPlane(vL->element[a].xyz, vL->element[b].xyz, vL->element[c].xyz,
                              origin); /* exponent don't care */
    if (distance > (FIXP_DBL)0) {
      tmp.index[0] = vtx;
      tmp.index[1] = edgeList[2 * i + 1];
      tmp.index[2] = edgeList[2 * i + 0];
    } else {
      tmp.index[0] = edgeList[2 * i + 0];
      tmp.index[1] = edgeList[2 * i + 1];
      tmp.index[2] = vtx;
    }

    addTriangleToList(tL, tmp);
  }
}

void sortEdgeList(int* edgeList) {
  int tmp;
  if (edgeList[0] > edgeList[1]) {
    tmp = edgeList[0];
    edgeList[0] = edgeList[1];
    edgeList[1] = tmp;
  }
}

FIXP_DBL distPointPlane(PointCartesian x1, PointCartesian x2, PointCartesian x3, PointCartesian p) {
  FIXP_DBL dist;
  PointCartesian n;

  PointCartesian op_a, op_b;
  /* Prescale to avoid overflow of subtraction
   * If direction of the vectors x1x2, x1x3 are the same then the resulting normal
   * vector will be the same as without scaling.
   * x1 isn't a point on plane anymore. But we scaled p also,
   * from this it follows that (x-p)n is now 0.5(x-p)n (scaled with an exponent of 1) */
  x1.x = x1.x >> 1;
  x1.y = x1.y >> 1;
  x1.z = x1.z >> 1;
  x2.x = x2.x >> 1;
  x2.y = x2.y >> 1;
  x2.z = x2.z >> 1;
  x3.x = x3.x >> 1;
  x3.y = x3.y >> 1;
  x3.z = x3.z >> 1;
  p.x = p.x >> 1;
  p.y = p.y >> 1;
  p.z = p.z >> 1;

  op_a = x3 - x1;
  op_b = x2 - x1;
  n = crossProduct(
      &op_a, &op_b); /* exponent doesen't really matter, because the direction is what we want */
  normalize(&n);
  op_a = x1 - p;
  dist = dotProduct(&op_a, &n); /* returns an exponent of 2. All together we have now a exponent of
                                   3 because of scaled x1 and p */

  /* Maximal possible dist in this context could be 2 (because we are in a sphere with radius 1.
   * Thus we normalize to a exponent of 1 with saturation */
  dist = SATURATE_LEFT_SHIFT(dist, 2, DFRACT_BITS);

  /* return value has a exponent of 1 (divided by 2) */
  return dist;
}

/*
 * @brief Initialize vertex including the sorting index.
 * @param azimuth     Azimuth angle in degree
 * @param elevation   Elevation angle in degree
 */
vertex init_vertex(FIXP_DBL azimuth, FIXP_DBL elevation) {
  vertex v;
  int idx_azi;
  int idx_ele;
  FIXP_DBL tmp;

  v.sph.azi = azimuth;
  v.sph.ele = fMax(D2F(-90.0), fMin(D2F(90.0), elevation));
  v.sph.rad = FL2FXCONST_DBL(1.0);

  /* Calculate Cartesian points from spherical points */
  v.xyz = sphericalToCartesian(v.sph);

  tmp = (FIXP_DBL)fAbs((FIXP_DBL)1073741824 - (FIXP_DBL)fAbs(v.sph.azi));  // D2F(90.0)
  tmp = tmp + (FIXP_DBL)5965232; /* Preparing for rounding */              // D2F(0.5)
  idx_azi = (int)(INT(tmp) / (INT)FX_DBL_DEGREE2INT); /* Calculate integer from fixpoint value
                                                         11930465 corresponds to 1 degree */

  tmp = (FIXP_DBL)fAbs(v.sph.ele) + (FIXP_DBL)5965232; /* Preparing for rounding */  // D2F(0.5)
  idx_ele = (int)(INT(tmp) / (INT)FX_DBL_DEGREE2INT); /* Calculate integer from fixpoint value
                                                         11930465 corresponds to 1 degree */
  // idx_ele = 90 - idx_ele;                       /* along cone of confusion instead of orthogonal
  // to cone of confusion */

  v.index = idx_azi + (181 * idx_ele); /* vertices on the median plane have lowest index */
  return v;
}

/*
 *  @brief Add speakers by the 5 rules in the definition
 *  @param vertexList vL of real speaker positions
 *  @param indices of initial pentagon (length have to be 5)
 *  @return error code
 */
int add_imaginary_speakers(vertexList* vL, int* pentagon) {
  int highestSpeaker = 0;
  int lowestSpeaker = 0;
  int err = 0;

  FIXP_DBL thresh = D2F(45.0);
  FIXP_DBL max_angle = D2F(160.0);

  surroundList* aziSurroundList;
  FIXP_DBL* angleDiffList;
  FIXP_DBL sector;

  int i, k;
  int cnt = 0;
  unsigned int surroundListSize = 0;

  int numberOfRealSpeaker = vL->size;

  /* Rule 1 and 2 */

  /* Search for highest and lowest speaker in vertexList */
  for (i = 0; i < vL->size; i++) {
    if (vL->element[i].sph.ele > vL->element[highestSpeaker].sph.ele) highestSpeaker = i;
    if (vL->element[i].sph.ele < vL->element[lowestSpeaker].sph.ele) lowestSpeaker = i;
  }

  /* Adding Voice of God Ghost speaker if necessary */
  if (vL->element[highestSpeaker].sph.ele < thresh) {
    highestSpeaker = vL->size;
    addVertexToList(vL, init_vertex(0, D2F(90.0)));
  }
  /* Adding Voice of Hell Ghost speaker if necessary */
  if (vL->element[lowestSpeaker].sph.ele > -thresh) {
    lowestSpeaker = vL->size;
    addVertexToList(vL, init_vertex(0, D2F(-90.0)));
  }

  /* Rule 3 */
  if (!addGhostToSubset(vL, &rangeOfSubsetA[0], true)) /* If no ghostspeaker was added */
  {
    addGhostToSubset(vL, &rangeOfSubsetB[0], true);
  }

  /* Rule 4 */
  if (!addGhostToSubset(vL, &rangeOfSubsetC[0], false)) /* If no ghostspeaker was added */
  {
    addGhostToSubset(vL, &rangeOfSubsetD[0], false);
  }

  /* Rule 5 */
  /* count surround real speaker */
  for (i = 0; i < numberOfRealSpeaker; i++) {
    if ((FIXP_DBL)fAbs(vL->element[i].sph.ele) < thresh) surroundListSize++;
  }
  aziSurroundList = (surroundList*)FDKmalloc(
      (surroundListSize + 2) *
      sizeof(surroundList)); /* a maximum number of 2 surround speaker can be added */
  angleDiffList = (FIXP_DBL*)FDKmalloc((surroundListSize) * sizeof(FIXP_DBL));

  if (!aziSurroundList || !angleDiffList) {
    err = 1;
    goto bail;
  }

  for (i = 0; i < numberOfRealSpeaker; i++) /* copy real surround speaker into surroundList */
  {
    if ((FIXP_DBL)fAbs(vL->element[i].sph.ele) < thresh) {
      aziSurroundList[cnt].azi = vL->element[i].sph.azi;
      aziSurroundList[cnt].index = i;
      cnt++;
    }
  }
  /* sort surroundList by Azimuth */
  qsort((void*)aziSurroundList, surroundListSize, sizeof(surroundList), aziCmp);

  /* calculate angle differences between real surround speakers */
  for (i = 1; i < (int)surroundListSize; i++) {
    angleDiffList[i - 1] =
        ((aziSurroundList[i].azi) >> 1) -
        ((aziSurroundList[i - 1].azi) >>
         1); /* Scale because angle between two speaker could be greater than 180 degrees */
  }

  FDK_ASSERT(surroundListSize >= 1);
  angleDiffList[surroundListSize - 1] = FIXP_DBL(
      INT((aziSurroundList[0].azi) >> 1) - INT((aziSurroundList[surroundListSize - 1].azi) >> 1) +
      INT(MAXVAL_DBL));  // + D2F(180.0)

  /* if there are gaps greater 160 degrees fill them with ghost speakers */
  for (i = 0; i < cnt; i++) {
    sector = angleDiffList[i];
    if (sector >= max_angle) /* remember angle scaling: if angle difference is greater than 2 times
                                max_angle (320 degrees) */
    {
      sector = fMult(sector, FL2FXCONST_DBL(2.0 / 3.0));
      for (k = 0; k < 2; k++) {
        aziSurroundList[surroundListSize].index = vL->size;
        aziSurroundList[surroundListSize].azi =
            FIXP_DBL(INT(aziSurroundList[i].azi) + (INT(sector) << k));
        addVertexToList(vL, init_vertex(aziSurroundList[surroundListSize].azi, 0));
        surroundListSize++;
      }
    } else if (sector >=
               (max_angle >> 1)) /* if angle difference is greater than max_angle (160 degrees)*/
    {
      aziSurroundList[surroundListSize].index = vL->size;
      aziSurroundList[surroundListSize].azi = FIXP_DBL(INT(aziSurroundList[i].azi) + INT(sector));
      addVertexToList(vL, init_vertex(aziSurroundList[surroundListSize].azi, 0));
      surroundListSize++;
    }
  }

  /* sort surroundList by Azimuth again. Because the new added values are not sorted */
  qsort((void*)aziSurroundList, surroundListSize, sizeof(surroundList), aziCmp);

  /******************************************
   *   Find indices of initial pentagon     *
   ******************************************/

  pentagon[0] = highestSpeaker;
  pentagon[1] = lowestSpeaker;

  i = 0;
  k = 0;

  for (cnt = 1; cnt < (int)surroundListSize; cnt++) {
    if ((aziSurroundList[cnt].azi >> 1) <=
        ((aziSurroundList[0].azi >> 1) +
         D2F(80.0))) /* if (aziSurroundList[i].azi <= aziSurroundList[0].azi + 160) */
    {
      i = cnt;
    }
    if ((aziSurroundList[cnt].azi >> 2) >=
        ((aziSurroundList[0].azi >> 2) +
         D2F(50.0))) /* if (aziSurroundList[i].azi >= aziSurroundList[0].azi + 200) */
    {
      k = cnt;
      break;
    }
  }

  pentagon[2] = aziSurroundList[0].index;
  pentagon[3] = aziSurroundList[i].index;
  pentagon[4] = aziSurroundList[k].index;

bail:
  FDKfree(aziSurroundList);
  FDKfree(angleDiffList);

  return err;
}

void add_imaginary_speakers_cicp(int cicpIndex, vertexList* vL, int* idx) {
  const imagSpeakerTable* hTable = getImagSpeakerTable(cicpIndex);
  int n;

  if (!hTable) {
    add_imaginary_speakers(vL, idx);
    return;
  }

  /* extend vertex list */
  for (n = 0; n < hTable->numGhost; n++) {
    addVertexToList(
        vL, init_vertex(hTable->aziGhost[n] * (INT)11930464, hTable->eleGhost[n] * (INT)11930464));
  }

  /* set initial vertex list */
  for (n = 0; n < 5; n++) {
    idx[n] = hTable->initialPoly[n];
  }
}

static CICP2GEOMETRY_ERROR augment_setup_to_superset(int* cicpIdx, vertexList* vL,
                                                     FIXP_DBL*** downmix_mat) {
  CICP2GEOMETRY_ERROR errCicp = CICP2GEOMETRY_OK;
  CICP2GEOMETRY_CHANNEL_GEOMETRY cicpGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  int numChannels;
  int numLfes;

  int i;
  int row;
  int col;
  const speakerSuperSetTable* hTable = getSpeakerSuperSetTable(*cicpIdx);

  if (hTable != NULL) {
    errCicp = cicp2geometry_get_geometry_from_cicp(hTable->supersetCicpIndex, cicpGeometry,
                                                   &numChannels, &numLfes);
    if (errCicp == CICP2GEOMETRY_OK) {
      *cicpIdx = hTable->supersetCicpIndex;
      resetVertexList(vL);
      for (i = 0; i < numChannels + numLfes; i++) {
        if (!cicpGeometry[i].LFE) {
          addVertexToList(vL, init_vertex(FIXP_DBL((INT)(cicpGeometry[i].Az * (INT)11930464)),
                                          FIXP_DBL((INT)(cicpGeometry[i].El * (INT)11930464))));
        }
      }

      *downmix_mat = (FIXP_DBL**)fdkCallocMatrix2D(hTable->nSpeakersSubSet,
                                                   hTable->nSpeakersSuperSet, sizeof(FIXP_DBL));
      if (*downmix_mat == NULL) {
        return CICP2GEOMETRY_ALLOC_ERROR;
      }
      for (row = 0; row < hTable->nSpeakersSubSet; row++) {
        for (col = 0; col < hTable->nSpeakersSuperSet; col++) {
          (*downmix_mat)[row][col] = hTable->downMixData[row * hTable->nSpeakersSuperSet + col];
        }
      }
    }
  }

  return errCicp;
}

/*
 * @brief surroundList comparison function for sorting the floats.
 * @param a   First item.
 * @param b   Second item.
 * @return c  Item difference.
 */
int aziCmp(const void* a, const void* b) {
  FIXP_DBL c;
  c = (FIXP_DBL)(((*(const surroundList*)a).azi >> 1) -
                 ((*(const surroundList*)b).azi >> 1)); /* shift one bit to prevent overflow */
  return c;
}

/*
 * \brief Tests whether a subset exists and it tests whether it is empty or not.
 *        If an empty subset was found a ghost speaker will be added
 *
 * \param vL            vertexList of all speakers
 * \param subsetRange   is one of the defined subset range in quickHull.h
 * \param frontSide     indicates whether the subset is a front or back side subset
 * Returns 0            if no ghost speaker was added
 * Returns 1            if a ghost speaker was added
 */

int addGhostToSubset(vertexList* vL, const FIXP_DBL* subsetRange, bool frontSide) {
  int subsetIndex[4];
  int foundSpeaker = 0;
  FIXP_DBL relCornerPoints[8];
  FIXP_DBL revertedCornerPoint;
  FIXP_DBL revertedTestingPoint;
  int i, k;
  int next;
  int foundPointInPolygon = 0;
  FIXP_DBL mean_azi;
  FIXP_DBL mean_ele;

  /* Search speaker subset in vertexList regarding to given subsetRange */
  for (i = 0; i < 4; i++) {
    for (k = 0; k < vL->size; k++) {
      if ((vL->element[k].sph.azi >= subsetRange[i * 4 + 0]) &&
          (vL->element[k].sph.azi <= subsetRange[i * 4 + 1]) &&
          (vL->element[k].sph.ele >= subsetRange[i * 4 + 2]) &&
          (vL->element[k].sph.ele <= subsetRange[i * 4 + 3])) {
        subsetIndex[i] = k;
        foundSpeaker++;
        if (foundSpeaker == 4) break; /* make sure to find never more than 4 speaker */
      }
    }
  }

  if (foundSpeaker == 4) /* complete subset was found */
  {
    /* check whether subset is empty or not */
    for (i = 0; i < vL->size; i++) /* go through all vertices */
    {
      if ((i == subsetIndex[0]) || (i == subsetIndex[1]) || (i == subsetIndex[2]) ||
          (i == subsetIndex[3])) /* don't check the corner points of the polygon itself */
        continue;

      /* shift the polygon by the testing point. relCornerPoints are the relative coordinates. */
      for (k = 0; k < 4; k++) {
        if (frontSide == true)
          relCornerPoints[2 * k + 0] =
              (vL->element[subsetIndex[k]].sph.azi >> 1) -
              (vL->element[i].sph.azi >> 1); /* shift one bit right to prevent overflow */
        else /* testing speaker polygon is on back side. To cope with the azimuth wrap around at
                -180/180 we revert the orientation to front side */
        {
          revertedCornerPoint =
              (FIXP_DBL)((INT)(vL->element[subsetIndex[k]].sph.azi) ^ (INT)MINVAL_DBL);
          revertedTestingPoint = (FIXP_DBL)((INT)(vL->element[i].sph.azi) ^ (INT)MINVAL_DBL);
          relCornerPoints[2 * k + 0] =
              (revertedCornerPoint >> 1) -
              (revertedTestingPoint >> 1); /* shift one bit right to prevent overflow */
        }
        relCornerPoints[2 * k + 1] =
            (vL->element[subsetIndex[k]].sph.ele >> 1) -
            (vL->element[i].sph.ele >> 1); /* shift one bit right to prevent overflow */
      }

      foundPointInPolygon = 1;
      /* indicates the 'orientation' of the line defined by a polygonal boundary. If all
       * orientations are the same, then the point is inside the polygon. */
      for (k = 0; k < 4; k++) {
        next = (k + 1) % 4;
        foundPointInPolygon =
            foundPointInPolygon &&
            ((fMultDiv2(relCornerPoints[2 * next + 0], relCornerPoints[2 * k + 1]) -
              fMultDiv2(relCornerPoints[2 * next + 1], relCornerPoints[2 * k + 0])) >=
             (FIXP_DBL)0); /* shift multiplication results one bit right to prevent overflow */
      }
      if (foundPointInPolygon == 1) break;
    }

    if (foundPointInPolygon == 0) /* Subset is empty. We have to add a Ghost speaker */
    {
      mean_azi = (FIXP_DBL)0;
      mean_ele = (FIXP_DBL)0;

      for (i = 0; i < 4; i++) {
        mean_azi += (FIXP_DBL)((vL->element[subsetIndex[i]].sph.azi) >> 2); /* Divide by 4 */
        mean_ele += (FIXP_DBL)((vL->element[subsetIndex[i]].sph.ele) >> 2); /* Divide by 4 */
      }
      if (frontSide == false) mean_azi = FIXP_DBL(INT(mean_azi) - INT(MAXVAL_DBL));  //- D2F(180.0)

      addVertexToList(vL, init_vertex((FIXP_DBL)mean_azi, (FIXP_DBL)mean_ele));

      return 1;
    }
  }

  return 0;
}
