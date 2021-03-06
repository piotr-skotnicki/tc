/**
 * This version is stamped on Apr. 14, 2015
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* 3mm.c: this file is part of PolyBench/C */

#include <omp.h>
#include <math.h>
#define ceild(n,d)  ceil(((double)(n))/((double)(d)))
#define floord(n,d) floor(((double)(n))/((double)(d)))
#define max(x,y)    ((x) > (y)? (x) : (y))
#define min(x,y)    ((x) < (y)? (x) : (y))

#define S3_I(i,j) F[i][j] = SCALAR_VAL(0.0);
#define S3(i,j) S3_I((i),(j))
#define S2_I(i,j,k) E[i][j] += A[i][k] * B[k][j];
#define S2(i,j,k) S2_I((i),(j),(k))
#define S1_I(i,j) E[i][j] = SCALAR_VAL(0.0);
#define S1(i,j) S1_I((i),(j))
#define S6_I(i,j,k) G[i][j] += E[i][k] * F[k][j];
#define S6(i,j,k) S6_I((i),(j),(k))
#define S5_I(i,j) G[i][j] = SCALAR_VAL(0.0);
#define S5(i,j) S5_I((i),(j))
#define S4_I(i,j,k) F[i][j] += C[i][k] * D[k][j];
#define S4(i,j,k) S4_I((i),(j),(k))

#include <isl/ctx.h>
#include <isl/space.h>
#include <isl/point.h>
#include <isl/val.h>
#include <isl/set.h>
#include <isl/map.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
#include "3mm.h"

/* Variable declaration/allocation. */
POLYBENCH_2D_ARRAY_DECL(E, DATA_TYPE, NI, NJ, ni, nj);
POLYBENCH_2D_ARRAY_DECL(A, DATA_TYPE, NI, NK, ni, nk);
POLYBENCH_2D_ARRAY_DECL(B, DATA_TYPE, NK, NJ, nk, nj);
POLYBENCH_2D_ARRAY_DECL(F, DATA_TYPE, NJ, NL, nj, nl);
POLYBENCH_2D_ARRAY_DECL(C, DATA_TYPE, NJ, NM, nj, nm);
POLYBENCH_2D_ARRAY_DECL(D, DATA_TYPE, NM, NL, nm, nl);
POLYBENCH_2D_ARRAY_DECL(G, DATA_TYPE, NI, NL, ni, nl);

/* Array initialization. */
static
void init_array(int ni, int nj, int nk, int nl, int nm,
		DATA_TYPE POLYBENCH_2D(A,NI,NK,ni,nk),
		DATA_TYPE POLYBENCH_2D(B,NK,NJ,nk,nj),
		DATA_TYPE POLYBENCH_2D(C,NJ,NM,nj,nm),
		DATA_TYPE POLYBENCH_2D(D,NM,NL,nm,nl))
{
  int i, j;

  for (i = 0; i < ni; i++)
    for (j = 0; j < nk; j++)
      A[i][j] = (DATA_TYPE) (i*j % ni) / (5*ni);
  for (i = 0; i < nk; i++)
    for (j = 0; j < nj; j++)
      B[i][j] = (DATA_TYPE) (i*(j+1) % nj) / (5*nj);
  for (i = 0; i < nj; i++)
    for (j = 0; j < nm; j++)
      C[i][j] = (DATA_TYPE) (i*(j+3) % nl) / (5*nl);
  for (i = 0; i < nm; i++)
    for (j = 0; j < nl; j++)
      D[i][j] = (DATA_TYPE) (i*(j+2) % nk) / (5*nk);
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int ni, int nl,
		 DATA_TYPE POLYBENCH_2D(G,NI,NL,ni,nl))
{
  int i, j;

  POLYBENCH_DUMP_START;
  POLYBENCH_DUMP_BEGIN("G");
  for (i = 0; i < ni; i++)
    for (j = 0; j < nl; j++) {
	if ((i * ni + j) % 20 == 0) fprintf (POLYBENCH_DUMP_TARGET, "\n");
	fprintf (POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, G[i][j]);
    }
  POLYBENCH_DUMP_END("G");
  POLYBENCH_DUMP_FINISH;
}

int create_task(isl_point* point, void* user) {
  isl_val* p0 = isl_point_get_coordinate_val(point, isl_dim_set, 0);
  int ii0 = isl_val_get_num_si(p0);
  isl_val_free(p0);
  isl_val* p1 = isl_point_get_coordinate_val(point, isl_dim_set, 1);
  int ii1 = isl_val_get_num_si(p1);
  isl_val_free(p1);
  isl_val* p2 = isl_point_get_coordinate_val(point, isl_dim_set, 2);
  int ii2 = isl_val_get_num_si(p2);
  isl_val_free(p2);
  isl_val* p3 = isl_point_get_coordinate_val(point, isl_dim_set, 3);
  int ii3 = isl_val_get_num_si(p3);
  isl_val_free(p3);
  isl_val* p4 = isl_point_get_coordinate_val(point, isl_dim_set, 4);
  int ii4 = isl_val_get_num_si(p4);
  isl_val_free(p4);
  isl_point_free(point);
  #pragma omp task
  {
    if (ii0 == 2 && ii1 >= 0 && ii2 >= 0 && ii3 == 1 && ii4 >= 0) {
      for (int c1 = 256 * ii1; c1 <= min(NI - 1, 256 * ii1 + 255); c1 += 1)
        for (int c2 = 256 * ii2; c2 <= min(NL - 1, 256 * ii2 + 255); c2 += 1)
          for (int c4 = 256 * ii4; c4 <= min(NJ - 1, 256 * ii4 + 255); c4 += 1)
            S6(c1, c2, c4);
    } else if (ii0 == 1 && ii1 >= 0 && ii2 >= 0 && ii3 == 1 && ii4 >= 0) {
      for (int c1 = 256 * ii1; c1 <= min(NJ - 1, 256 * ii1 + 255); c1 += 1)
        for (int c2 = 256 * ii2; c2 <= min(NL - 1, 256 * ii2 + 255); c2 += 1)
          for (int c4 = 256 * ii4; c4 <= min(NM - 1, 256 * ii4 + 255); c4 += 1)
            S4(c1, c2, c4);
    } else if (ii0 == 0 && ii1 >= 0 && ii2 >= 0 && ii3 == 1 && ii4 >= 0) {
      for (int c1 = 256 * ii1; c1 <= min(NI - 1, 256 * ii1 + 255); c1 += 1)
        for (int c2 = 256 * ii2; c2 <= min(NJ - 1, 256 * ii2 + 255); c2 += 1)
          for (int c4 = 256 * ii4; c4 <= min(NK - 1, 256 * ii4 + 255); c4 += 1)
            S2(c1, c2, c4);
    } else if (ii0 == 2 && ii1 >= 0 && ii2 >= 0 && ii3 == 0 && ii4 == 0) {
      for (int c1 = 256 * ii1; c1 <= min(NI - 1, 256 * ii1 + 255); c1 += 1)
        for (int c2 = 256 * ii2; c2 <= min(NL - 1, 256 * ii2 + 255); c2 += 1)
          S5(c1, c2);
    } else if (ii0 == 1 && ii1 >= 0 && ii2 >= 0 && ii3 == 0 && ii4 == 0) {
      for (int c1 = 256 * ii1; c1 <= min(NJ - 1, 256 * ii1 + 255); c1 += 1)
        for (int c2 = 256 * ii2; c2 <= min(NL - 1, 256 * ii2 + 255); c2 += 1)
          S3(c1, c2);
    } else if (ii0 == 0 && ii1 >= 0 && ii2 >= 0 && ii3 == 0 && ii4 == 0)
      for (int c1 = 256 * ii1; c1 <= min(NI - 1, 256 * ii1 + 255); c1 += 1)
        for (int c2 = 256 * ii2; c2 <= min(NJ - 1, 256 * ii2 + 255); c2 += 1)
          S1(c1, c2);
  }
  return 0;
}

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_3mm()
{
#pragma scop
  isl_ctx* ctx = isl_ctx_alloc();
  isl_map* rtile = isl_map_read_from_str(ctx, "[NI, NL, NJ, NM, NK] -> { [2, i1, i2, 1, i4] -> [2, i1, i2, 1, o4] : i1 >= 0 and 256i1 <= -1 + NI and i2 >= 0 and 256i2 <= -1 + NL and i4 >= 0 and o4 >= 1 + i4 and 256o4 <= -1 + NJ; [1, i1, i2, 1, i4] -> [1, i1, i2, 1, o4] : i1 >= 0 and 256i1 <= -1 + NJ and i2 >= 0 and 256i2 <= -1 + NL and i4 >= 0 and o4 >= 1 + i4 and 256o4 <= -1 + NM; [0, i1, i2, 1, i4] -> [0, i1, i2, 1, o4] : i1 >= 0 and 256i1 <= -1 + NI and i2 >= 0 and 256i2 <= -1 + NJ and i4 >= 0 and o4 >= 1 + i4 and 256o4 <= -1 + NK; [1, i1, i2, 1, i4] -> [2, o1, i2, 1, i1] : i1 >= 0 and 256i1 <= -1 + NJ and i2 >= 0 and 256i2 <= -1 + NL and i4 >= 0 and 256i4 <= -1 + NM and o1 >= 0 and 256o1 <= -1 + NI; [0, i1, i2, 1, i4] -> [2, i1, o2, 1, i2] : i1 >= 0 and 256i1 <= -1 + NI and i2 >= 0 and 256i2 <= -1 + NJ and i4 >= 0 and 256i4 <= -1 + NK and o2 >= 0 and 256o2 <= -1 + NL; [2, i1, i2, 0, 0] -> [2, i1, i2, 1, o4] : i1 >= 0 and 256i1 <= -1 + NI and i2 >= 0 and 256i2 <= -1 + NL and o4 >= 0 and 256o4 <= -1 + NJ; [1, i1, i2, 0, 0] -> [1, i1, i2, 1, o4] : i1 >= 0 and 256i1 <= -1 + NJ and i2 >= 0 and 256i2 <= -1 + NL and o4 >= 0 and 256o4 <= -1 + NM; [0, i1, i2, 0, 0] -> [0, i1, i2, 1, o4] : i1 >= 0 and 256i1 <= -1 + NI and i2 >= 0 and 256i2 <= -1 + NJ and o4 >= 0 and 256o4 <= -1 + NK; [1, i1, i2, 0, 0] -> [2, o1, i2, 1, i1] : i1 >= 0 and 256i1 <= -1 + NJ and i2 >= 0 and 256i2 <= -1 + NL and o1 >= 0 and 256o1 <= -1 + NI; [0, i1, i2, 0, 0] -> [2, i1, o2, 1, i2] : i1 >= 0 and 256i1 <= -1 + NI and i2 >= 0 and 256i2 <= -1 + NJ and o2 >= 0 and 256o2 <= -1 + NL }");
  isl_set* ii_set = isl_set_read_from_str(ctx, "[NL, NI, NJ, NM, NK] -> { [2, i1, i2, 1, i4] : i1 >= 0 and 256i1 <= -1 + NI and i2 >= 0 and 256i2 <= -1 + NL and i4 >= 0 and 256i4 <= -1 + NJ; [1, i1, i2, 1, i4] : i1 >= 0 and 256i1 <= -1 + NJ and i2 >= 0 and 256i2 <= -1 + NL and i4 >= 0 and 256i4 <= -1 + NM; [0, i1, i2, 1, i4] : i1 >= 0 and 256i1 <= -1 + NI and i2 >= 0 and 256i2 <= -1 + NJ and i4 >= 0 and 256i4 <= -1 + NK; [2, i1, i2, 0, 0] : i1 >= 0 and 256i1 <= -1 + NI and i2 >= 0 and 256i2 <= -1 + NL; [1, i1, i2, 0, 0] : i1 >= 0 and 256i1 <= -1 + NJ and i2 >= 0 and 256i2 <= -1 + NL; [0, i1, i2, 0, 0] : i1 >= 0 and 256i1 <= -1 + NI and i2 >= 0 and 256i2 <= -1 + NJ }");
  rtile = isl_map_fix_si(rtile, isl_dim_param, 0, NI);
  ii_set = isl_set_fix_si(ii_set, isl_dim_param, 0, NL);
  rtile = isl_map_fix_si(rtile, isl_dim_param, 1, NL);
  ii_set = isl_set_fix_si(ii_set, isl_dim_param, 1, NI);
  rtile = isl_map_fix_si(rtile, isl_dim_param, 2, NJ);
  ii_set = isl_set_fix_si(ii_set, isl_dim_param, 2, NJ);
  rtile = isl_map_fix_si(rtile, isl_dim_param, 3, NM);
  ii_set = isl_set_fix_si(ii_set, isl_dim_param, 3, NM);
  rtile = isl_map_fix_si(rtile, isl_dim_param, 4, NK);
  ii_set = isl_set_fix_si(ii_set, isl_dim_param, 4, NK);

  isl_set* uds = isl_set_subtract(isl_map_domain(isl_map_copy(rtile)), isl_map_range(isl_map_copy(rtile)));
  
  #pragma omp parallel
  #pragma omp single
  {
    while (!isl_set_is_empty(uds)) {
      isl_set_foreach_point(uds, &create_task, NULL);
      #pragma omp taskwait

      ii_set = isl_set_subtract(ii_set, uds);
      rtile = isl_map_intersect_domain(rtile, isl_set_copy(ii_set));
      uds = isl_set_subtract(isl_map_domain(isl_map_copy(rtile)), isl_map_range(isl_map_copy(rtile)));
    }
    
    isl_set_foreach_point(ii_set, &create_task, NULL);
    #pragma omp taskwait
  }
  
  isl_set_free(uds);
  isl_set_free(ii_set);
  isl_map_free(rtile);
  isl_ctx_free(ctx);
#pragma endscop
}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int ni = NI;
  int nj = NJ;
  int nk = NK;
  int nl = NL;
  int nm = NM;

  /* Initialize array(s). */
  init_array (ni, nj, nk, nl, nm,
	      POLYBENCH_ARRAY(A),
	      POLYBENCH_ARRAY(B),
	      POLYBENCH_ARRAY(C),
	      POLYBENCH_ARRAY(D));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_3mm ();

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(ni, nl,  POLYBENCH_ARRAY(G)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(E);
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(B);
  POLYBENCH_FREE_ARRAY(F);
  POLYBENCH_FREE_ARRAY(C);
  POLYBENCH_FREE_ARRAY(D);
  POLYBENCH_FREE_ARRAY(G);

  return 0;
}
