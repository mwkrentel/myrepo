/*
 *  Override __libc_start_main() with ld_preload.
 *
 *  Copyright (c) 2019, Rice University.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *  * Neither the name of Rice University (RICE) nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  This software is provided by RICE and contributors "as is" and any
 *  express or implied warranties, including, but not limited to, the
 *  implied warranties of merchantability and fitness for a particular
 *  purpose are disclaimed. In no event shall RICE or contributors be
 *  liable for any direct, indirect, incidental, special, exemplary, or
 *  consequential damages (including, but not limited to, procurement of
 *  substitute goods or services; loss of use, data, or profits; or
 *  business interruption) however caused and on any theory of liability,
 *  whether in contract, strict liability, or tort (including negligence
 *  or otherwise) arising in any way out of the use of this software, even
 *  if advised of the possibility of such damage.
 */

#include <sys/types.h>
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>

#include "monitor-config.h"

#ifndef RTLD_NEXT
#define RTLD_NEXT  ((void *) -1l)
#endif

#define GET_REAL_FUNC(var, name)		\
    if ( var == NULL ) {			\
        var = dlsym(RTLD_NEXT, name);		\
	if ( var == NULL ) {			\
	    errx(1, "dlsym(" name ")failed");	\
	}					\
    }

/*
 *  Powerpc adds a fourth argument to main().
 */
#ifdef MONITOR_START_MAIN_PPC

#define AUXVEC_ARG   , auxvec
#define AUXVEC_DECL  , void * auxvec

#else  // x86 and default libc_start_main args

#define AUXVEC_ARG
#define AUXVEC_DECL

#endif

typedef int main_fcn_t (int, char **, char **  AUXVEC_DECL );

static main_fcn_t  *real_main = NULL;
extern main_fcn_t  __real_main;

//----------------------------------------------------------------------

/*
 *  The stack frame around the application's main().
 */
int
__wrap_main(int argc, char **argv, char **envp  AUXVEC_DECL )
{
    int ret;

    printf("---> monitor: begin process\n");

#if defined(MONITOR_PRELOAD)
    ret = (* real_main) (argc, argv, envp  AUXVEC_ARG );
#else
    ret = __real_main (argc, argv, envp  AUXVEC_ARG );
#endif

    printf("---> monitor: end process\n");

    return ret;
}

//----------------------------------------------------------------------

/*
 *  Override __libc_start_main() via LD_PRELOAD.  The signature is
 *  different for powerpc.
 */
#if defined(MONITOR_PRELOAD)

#ifdef MONITOR_START_MAIN_PPC

typedef int start_main_fcn_t (int, char **, char **, void *, void *, void **, void *);

static void *new_stinfo[4];

int
__libc_start_main(int argc, char **argv, char **envp, void *auxp,
		  void *rtld_fini, void **stinfo, void *stack_end)
{
    start_main_fcn_t *real_start_main = NULL;

    GET_REAL_FUNC(real_start_main, "__libc_start_main");

    real_main = (main_fcn_t *) stinfo[1];
    new_stinfo[0] = stinfo[0];
    new_stinfo[1] = & __wrap_main;
    new_stinfo[2] = stinfo[2];
    new_stinfo[3] = stinfo[3];

    int ret = (* real_start_main)
	(argc, argv, envp, auxp, rtld_fini, new_stinfo, stack_end);

    // not reached
    return ret;
}

#else  // x86 and default libc_start_main type

typedef int start_main_fcn_t (main_fcn_t *, int, char **, void *, void *, void *, void *);

int
__libc_start_main(main_fcn_t *main, int argc, char **argv, void *init,
		  void *fini, void *rtld_fini, void *stack_end)
{
    start_main_fcn_t *real_start_main = NULL;

    GET_REAL_FUNC(real_start_main, "__libc_start_main");

    real_main = main;

    int ret = (* real_start_main)
	(__wrap_main, argc, argv, init, fini, rtld_fini, stack_end);

    // not reached
    return ret;
}

#endif  // libc_start_main type
#endif  // preload case for libc_start_main
