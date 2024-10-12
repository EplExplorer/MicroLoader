/*
 * Copyright (c) 1994 by Xerox Corporation.  All rights reserved.
 * Copyright (c) 1996 by Silicon Graphics.  All rights reserved.
 * Copyright (c) 1998 by Fergus Henderson.  All rights reserved.
 * Copyright (c) 2000-2009 by Hewlett-Packard Development Company.
 * All rights reserved.
 * Copyright (c) 2009-2018 Ivan Maidanski
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to use or copy this program
 * for any purpose,  provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 */

/* This file could be used for the following purposes:          */
/* - get the complete GC as a single link object file (module); */
/* - enable more compiler optimizations.                        */

/* Tip: to get the highest level of compiler optimizations, the typical */
/* compiler options (GCC) to use are:                                   */
/* -O3 -fno-strict-aliasing -march=native -Wall -fprofile-generate/use  */

/* Warning: GCC for Linux (for C++ clients only): Use -fexceptions both */
/* for GC and the client otherwise GC_thread_exit_proc() is not         */
/* guaranteed to be invoked (see the comments in pthread_start.c).      */

#ifndef __cplusplus
  /* static is desirable here for more efficient linkage.               */
  /* TODO: Enable this in case of the compilation as C++ code.          */
# define GC_INNER STATIC
# define GC_EXTERN GC_INNER
                /* STATIC is defined in gcconfig.h. */
#endif

/* Small files go first... */
#include "bdwgc/backgraph.c"
#include "bdwgc/blacklst.c"
#include "bdwgc/checksums.c"
#include "bdwgc/gcj_mlc.c"
#include "bdwgc/headers.c"
#include "bdwgc/new_hblk.c"
#include "bdwgc/obj_map.c"
#include "bdwgc/ptr_chck.c"

#include "gc_inline.h"
#include "bdwgc/allchblk.c"
#include "bdwgc/alloc.c"
#include "bdwgc/dbg_mlc.c"
#include "bdwgc/finalize.c"
#include "bdwgc/fnlz_mlc.c"
#include "bdwgc/malloc.c"
#include "bdwgc/mallocx.c"
#include "bdwgc/mark.c"
#include "bdwgc/mark_rts.c"
#include "bdwgc/reclaim.c"
#include "bdwgc/typd_mlc.c"

#include "bdwgc/misc.c"
#include "bdwgc/os_dep.c"
#include "bdwgc/thread_local_alloc.c"

/* Most platform-specific files go here... */
#include "bdwgc/darwin_stop_world.c"
#include "bdwgc/dyn_load.c"
#include "bdwgc/gc_dlopen.c"
#if !defined(PLATFORM_MACH_DEP)
# include "bdwgc/mach_dep.c"
#endif
#if !defined(PLATFORM_STOP_WORLD)
# include "bdwgc/pthread_stop_world.c"
#endif
#include "bdwgc/pthread_support.c"
#include "bdwgc/specific.c"
#include "bdwgc/win32_threads.c"

#ifndef GC_PTHREAD_START_STANDALONE
# include "bdwgc/pthread_start.c"
#endif

/* Restore pthread calls redirection (if altered in             */
/* pthread_stop_world.c, pthread_support.c or win32_threads.c). */
/* This is only useful if directly included from application    */
/* (instead of linking gc).                                     */
#ifndef GC_NO_THREAD_REDIRECTS
# define GC_PTHREAD_REDIRECTS_ONLY
# include "gc_pthread_redirects.h"
#endif

/* The files from "extra" folder are not included. */
