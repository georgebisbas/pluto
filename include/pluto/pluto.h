/******************************************************************************
 *               libpluto -  A library version of Pluto                       *
 ******************************************************************************
 *                                                                            *
 * Copyright (C) 2012 Uday Bondhugula                                         *
 *                                                                            *
 * This library is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU Lesser General Public                 *
 * License version 2.1 as published by the Free Software Foundation.          *
 *                                                                            *
 * This library is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          *
 * Lesser General Public License for more details.                            *
 *                                                                            *
 * A copy of the GNU Lesser General Public Licence can be found in the file
 * `LICENSE.LGPL2' in the top-level directory of this distribution.
 *
 * This file is part of libpluto.
 *
 */
#ifndef __LIBPLUTO__
#define __LIBPLUTO__

typedef struct isl_union_set isl_union_set;
typedef struct isl_union_map isl_union_map;

#include "osl/scop.h"
#include "isl/ctx.h"

#include "pluto/matrix.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Fusion options for options->fuse */
enum fusionType {
  /* Do not fuse across SCCs */
  kNoFuse,

  /* Geared towards maximal fusion, but not really maximal fusion. */
  kMaximalFuse,

  /* Does not fuse nests with different dimensionality. */
  kSmartFuse,

  /* Fuses SCCs only if fusion does not result in loss of parallelism. */
  kTypedFuse,

  /* Typed fuse at outer levels, Max fuse at inner levels. */
  kHybridFuse
};
typedef enum fusionType FusionType;

struct plutoOptions {

  /* To tile or not? */
  int tile;

  /* Intra-tile optimization */
  int intratileopt;

  /* Diamond tiling for concurrent startup; enables concurrent startup along
   * one dimension. */
  int diamondtile;

  /* Use per connected component u and w instead of single u and w for the whole
   * program. */
  int per_cc_obj;

  /* Extract scop information from libpet*/
  int pet;

  /* Dynamic scheduling using Synthesized Runtime Interface. */
  int dynschedule;

  /* Dynamic scheduling - previous technique of building the entire task graph
   * in memory using Intel TBB Flow Graph scheduler */
  int dynschedule_graph;

  // Dynamic scheduling - previous technique of building the entire task graph
  // in memory using a custom DAG scheduler.
  // No longer maintained
  // TODO: remove this!
  int dynschedule_graph_old;

  /* consider transitive dependences between tasks */
  int dyn_trans_deps_tasks;

  /* Enables concurrent startup along dimensions  */
  int fulldiamondtile;

  /* Parallelization */
  int parallel;

  /* prefer pure inner parallelism to pipelined parallelism */
  int innerpar;

  /* Automatic unroll/unroll-jamming of loops */
  int unroll;

  /* unroll/jam factor */
  int ufactor;

  /* Enable or disable post-transformations to make code amenable to
   * vectorization (default - enabled) */
  int prevector;

  /* consider RAR dependences */
  int rar;

  /* Decides the fusion algorithm (MAXIMAL_FUSE, NO_FUSE, or SMART_FUSE) */
  FusionType fuse;

  /* For experimental purposes with dfp */
  int delayed_cut;

  /* For experimental purposes */
  int intra_tile_distribute;

  /* Tyepd fuse at outer levels, max fuse at inner levels */
  int hybridcut;

  /* for debugging - print default cloog-style total */
  int scancount;

  /* parameters will be assumed to be at least this much */
  /* This is appended to the context passed to cloog */
  int codegen_context;

  /* Loop depth (1-indexed) to force as parallel */
  int forceparallel;

  /* multiple (currently two) degrees of pipelined parallelism */
  int multipar;

  /* Tile for L2 too */
  /* By default, only L1 tiling is done; under parallel execution, every
   * processor executes a sequence of L1 tiles (OpenMP adds another blocking
   * on the parallel loop). With L2 tiling, each processor executes a
   * sequence of L2 tiles and barrier is done after a group of L2 tiles is
   * exectuted -- causes load imbalance due to pipe startup when problem
   * sizes are not huge */
  int l2tile;

  /* NOTE: --ft and --lt are to manually force tiling depths */
  /* First depth to tile (starting from 0) */
  int ft;
  /* Last depth to tile (indexed from 0)  */
  int lt;

  /* Output for debugging */
  int debug;

  /* More debugging output */
  int moredebug;

  /* Not implemented yet: Don't output anything unless something fails */
  int quiet;

  /* Identity transformation */
  int identity;

  /* Generate scheduling pragmas for Bee+Cl@k */
  int bee;

  /* Force this for cloog's -f */
  int cloogf;

  /* Force this for cloog's -l */
  int cloogl;

  /* Enable cloog's -sh (simple convex hull) */
  int cloogsh;

  /* Enable cloog's -backtrack */
  int cloogbacktrack;

  /* Use isl to compute dependences (default) */
  int isldep;

  /* Use candl to compute dependences */
  int candldep;

  /* Access-wise dependences with ISL */
  int isldepaccesswise;

  /* Coalesce ISL deps */
  int isldepcoalesce;

  /* Compute lastwriter for dependences */
  int lastwriter;

  /* DEV: Don't use cost function */
  int nodepbound;

  /* hard upper bound for transformation coefficients */
  int coeff_bound;

  /* Ask candl to privatize */
  int scalpriv;

  /* No output from Pluto if everything goes right */
  int silent;

  /* Read input from a .scop file */
  int readscop;

  /* Use PIP as the ILP solver. */
  int pipsolve;

  /* Use isl as the ILP solver. */
  int islsolve;

  /* Use glpk as the ILP solver. */
  int glpk;

  /* Use gurobi as the ILP solver. */
  int gurobi;

  /* Use lp instead of ILP. */
  int lp;

  /* Use pluto-(i)lp-dfp framework instead of pluto-ilp */
  int dfp;

  /* Use ILP with pluto-dfp instead of LP. */
  int ilp;

  /* Use LP solutions to colour SCCs */
  int lpcolour;

  /* Cluster the statements of the SCC. Currently supported with DFP based
   * approach only */
  int scc_cluster;

  /* Index set splitting */
  int iss;

  /* Output file name supplied from -o */
  char *out_file;

  /* Polyhedral compile time stats */
  int time;

  /* fast linear independence check */
  int flic;
};
typedef struct plutoOptions PlutoOptions;

PlutoOptions *pluto_options_alloc();
void pluto_options_free(PlutoOptions *);

/// Run the Pluto transformation algorithm on the provided domains and
/// dependences. Read and writes accesses can be optionally provided (NULL
/// otherwise); if they are provided, they are exploited for certain late
/// transformations (for intra-tile optimization in particular). Returns the
/// schedules as an isl_union_map, ownership of which is with the caller.
__isl_give isl_union_map *pluto_transform(__isl_take isl_union_set *domains,
                                          __isl_take isl_union_map *dependences,
                                          __isl_take isl_union_map *reads,
                                          __isl_take isl_union_map *writes,
                                          PlutoOptions *options);

/// Use the Pluto transformation algorithm on the domains cum schedules provided
/// in `schedules' and with the read and writes access relations provided in
/// `reads' and `writes' respectively. Returns the schedules as an
/// isl_union_map, ownership of which is with the caller. Note that the returned
/// schedules encode both the mapping and the set information.
__isl_give isl_union_map *pluto_schedule(__isl_take isl_union_map *schedules,
                                         __isl_take isl_union_map *reads,
                                         __isl_take isl_union_map *writes,
                                         PlutoOptions *options_l);

int pluto_schedule_osl(osl_scop_p scop, PlutoOptions *options_l);

/*
 * Structure to hold iterator remapping information ---
 * consists of number of statements, remapping matrix and divs.
 */
struct remapping {
  unsigned nstmts;
  PlutoMatrix **stmt_inv_matrices;
  int **stmt_divs;
};
typedef struct remapping Remapping;

// This function is a HACK. The reason this exists is to allow for easy FFI
// between PolyMage and Pluto. Sending isl objects between PyIsl to libpluto is
// hard (because PyIsl does not seem to have a way to access the underlying C
// object pointer). Hence, the solution is to convert everything to strings,
// and return the generated schedule as a string as well, which is then
// converted back to an ISL object.
void pluto_schedule_str(const char *domains_str, const char *dependences_str,
                        char **schedules_str_buffer_ptr, char **p_loops,
                        Remapping **remapping_ptr, PlutoOptions *options);

void pluto_get_remapping_str(const char *domains_str,
                             const char *dependences_str, PlutoOptions *options,
                             Remapping *remapping);

void pluto_remapping_free(Remapping remapping);

/// Free the string stored in schedules_str_buffer_ptr.
void pluto_schedules_strbuf_free(char *schedules_str_buffer);

#if defined(__cplusplus)
}
#endif

#endif
