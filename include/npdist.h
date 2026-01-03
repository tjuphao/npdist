#ifndef NPDIST_H
#define NPDIST_H

/*****************************************************************************/
/*
 * npdist.h - NP distance data structures
 *
 * (c) 2025
 *
 *//**
 * @file npdist.h
 * @ingroup NormingPointDistance
 *//*
 * Project: npdist - Norming Point Distance Calculator
 *
 * System:  none
 *
 * Revision History:
 * 07/23/2011 - tjuphao
 *      Initial revision
 *****************************************************************************/

#define MAX_ROUTES   64
#define MAX_SEGS     4096   // should be safely above your max segment index
#define MAX_NPIDS    4096   // should be safely above your max NPId

typedef struct {
  int present;
  int route;
  int seg;
  int off;
  int type;
} NPRec;

typedef struct {
  int present;
  int max_seg;
  int seg_len[MAX_SEGS + 1];     // 1-based segment index
  long long seg_start[MAX_SEGS + 1]; // prefix start position for segment
  int prefix_built;
} RouteRec;

#endif // NPDIST_H