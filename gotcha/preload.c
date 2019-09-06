/*
 *  The 'pure preload' version.  Override all functions with
 *  LD_PRELOAD.  This is what libmonitor does now.
 *
 *  Copyright (c) 2019, Rice University.
 *  See the file LICENSE for details.
 *
 *  Mark W. Krentel
 *  August 2019
 */

#include <sys/types.h>
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#ifndef RTLD_NEXT
#define RTLD_NEXT  ((void *) -1l)
#endif

typedef int start_main_fcn_t (void *, int, char **, void *,
                              void *, void *, void *);
typedef int pthread_create_fcn_t(void *, void *, void *, void *);
typedef int sigprocmask_fcn_t(int, void *, void *);
typedef void * dlopen_fcn_t(const char *, int);

static start_main_fcn_t  *real_start_main = NULL;
static pthread_create_fcn_t  *real_pthread_create = NULL;
static sigprocmask_fcn_t  *real_sigprocmask = NULL;
static dlopen_fcn_t  *real_dlopen = NULL;

#define GET_REAL_FUNC(var, name)		\
    if ( var == NULL ) {			\
        var = dlsym(RTLD_NEXT, name);		\
	if ( var == NULL ) {			\
	    errx(1, "dlsym(" name ")failed");	\
	}					\
    }

//----------------------------------------------------------------------

int
__libc_start_main(void * main, int argc, char ** argv, void * init,
                  void * fini, void * rtld, void * stack_end)
{
    printf("---> preload: override: libc_start_main\n");

    GET_REAL_FUNC(real_start_main, "__libc_start_main");

    int ret = (* real_start_main) (main, argc, argv, init, fini, rtld, stack_end);

    return ret;
}

int
pthread_create(void *tid, void *attr, void *st_routine, void *arg)
{
    printf("---> preload: override: pthread_create\n");

    GET_REAL_FUNC(real_pthread_create, "pthread_create");

    int ret = real_pthread_create(tid, attr, st_routine, arg);

    return ret;
}

int
sigprocmask(int how, void *set, void *old_set)
{
    printf("---> preload: override: sigprocmask\n");

    GET_REAL_FUNC(real_sigprocmask, "sigprocmask");

    int ret = real_sigprocmask(how, set, old_set);

    return ret;
}

void *
dlopen(const char *name, int flags)
{
    printf("---> preload: override: dlopen(%s)\n", name);

    GET_REAL_FUNC(real_dlopen, "dlopen");

    void * handle = real_dlopen(name, flags);

    return handle;
}
