/*
  npdist.c  - distance between two Norming Points (NPId) using segment lengths

  Build:
    gcc -O2 -Wall -Wextra -o npdist npdist.c

  Run:
    ./npdist /path/to/vato_dev.log 2 34
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "npdist.h"

static RouteRec g_routes[MAX_ROUTES];
static NPRec    g_np[MAX_NPIDS];

static void die(const char *msg) {
  fprintf(stderr, "ERROR: %s\n", msg);
  exit(1);
}

static int build_prefix_for_route(int r) {
  if (r < 0 || r >= MAX_ROUTES) return 0;
  RouteRec *rt = &g_routes[r];
  if (!rt->present) return 0;
  if (rt->prefix_built) return 1;

  long long acc = 0;
  rt->seg_start[0] = 0;

  for (int s = 1; s <= rt->max_seg; s++) {
    rt->seg_start[s] = acc;
    int L = rt->seg_len[s];
    if (L < 0) L = 0; // defensive
    acc += (long long)L;
  }
  rt->prefix_built = 1;
  return 1;
}

static long long np_position(int npid, int *out_route) {
  if (npid < 0 || npid >= MAX_NPIDS || !g_np[npid].present) {
    errno = ENOENT;
    return -1;
  }
  NPRec *np = &g_np[npid];
  int r = np->route;
  int s = np->seg;

  if (r < 0 || r >= MAX_ROUTES || !g_routes[r].present) {
    errno = EINVAL;
    return -1;
  }
  if (s <= 0 || s > g_routes[r].max_seg || g_routes[r].seg_len[s] <= 0) {
    errno = EINVAL;
    return -1;
  }

  if (!build_prefix_for_route(r)) {
    errno = EINVAL;
    return -1;
  }

  if (out_route) *out_route = r;
  return g_routes[r].seg_start[s] + (long long)np->off;
}

static void parse_file(const char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp) {
    perror("fopen");
    exit(1);
  }

  char line[2048];

  while (fgets(line, (int)sizeof(line), fp)) {
    // SEG lines look like:
    // "...|  SEG -  1   2| 15   768|  9   257|0x01  1890| ..."
    char *pseg = strstr(line, "SEG -");
    if (pseg && strstr(line, "SEG -  R Seg") == NULL && strstr(line, "Num_Segment_Records") == NULL) {
      int r = -1, s = -1, len = -1;

      // parse from "SEG -"
      // We only need: route, seg, and the "Len" number after "0x??"
      // Use %*[^|] to skip each pipe-field.
      if (sscanf(pseg, "SEG - %d %d|%*[^|]|%*[^|]|0x%*x %d|", &r, &s, &len) == 3) {
        if (r >= 0 && r < MAX_ROUTES && s > 0 && s <= MAX_SEGS) {
          RouteRec *rt = &g_routes[r];
          rt->present = 1;
          if (s > rt->max_seg) rt->max_seg = s;
          rt->seg_len[s] = len;
          rt->prefix_built = 0; // invalidate if we see more data
        }
      }
      continue;
    }

    // NP lines look like:
    // "...|  NP  -  1   2   333|    2 13"
    char *pnp = strstr(line, "NP  -");
    if (pnp && strstr(line, "NP  -  R Seg") == NULL && strstr(line, "Number_Of_Norming_Points") == NULL) {
      int r = -1, s = -1, off = -1, npid = -1, type = -1;
      if (sscanf(pnp, "NP  - %d %d %d| %d %d", &r, &s, &off, &npid, &type) == 5) {
        if (npid >= 0 && npid < MAX_NPIDS) {
          g_np[npid].present = 1;
          g_np[npid].route = r;
          g_np[npid].seg = s;
          g_np[npid].off = off;
          g_np[npid].type = type;
        }
      }
      continue;
    }
  }

  fclose(fp);

  // build prefixes for all present routes
  for (int r = 0; r < MAX_ROUTES; r++) {
    if (g_routes[r].present) build_prefix_for_route(r);
  }
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <vato_dev.log> <NPId_A> <NPId_B>\n", argv[0]);
    return 2;
  }

  const char *path = argv[1];
  int npA = atoi(argv[2]);
  int npB = atoi(argv[3]);

  parse_file(path);

  int routeA = -1, routeB = -1;
  long long posA = np_position(npA, &routeA);
  long long posB = np_position(npB, &routeB);

  if (posA < 0) {
    fprintf(stderr, "NPId %d not found or invalid (route/segment/offset).\n", npA);
    return 1;
  }
  if (posB < 0) {
    fprintf(stderr, "NPId %d not found or invalid (route/segment/offset).\n", npB);
    return 1;
  }

  NPRec *a = &g_np[npA];
  NPRec *b = &g_np[npB];

  printf("NP %d: R=%d Seg=%d Off=%d  => pos=%lld\n", npA, a->route, a->seg, a->off, posA);
  printf("NP %d: R=%d Seg=%d Off=%d  => pos=%lld\n", npB, b->route, b->seg, b->off, posB);

  if (routeA != routeB) {
    printf("Distance: N/A (different routes: %d vs %d)\n", routeA, routeB);
    return 0;
  }

  long long dist = (posA > posB) ? (posA - posB) : (posB - posA);
  printf("Distance between NP %d and NP %d (Route %d) = %lld (same units as Len/Off)\n",
         npA, npB, routeA, dist);

  return 0;
}
