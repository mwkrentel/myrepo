/*
 *  The 'hybrid' version.  Override __libc_start_main() and
 *  pthread_create() with preload and use gotcha for the rest.
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

#include "gotcha/gotcha.h"

#ifndef RTLD_NEXT
#define RTLD_NEXT  ((void *) -1l)
#endif

typedef int start_main_fcn_t (void *, int, char **, void *,
                              void *, void *, void *);
typedef int pthread_create_fcn_t(void *, void *, void *, void *);

typedef int (* sigprocmask_fptr) (int, void *, void *);
typedef void * (* dlopen_fptr) (char *, int);

static start_main_fcn_t  *real_start_main = NULL;
static pthread_create_fcn_t  *real_pthread_create = NULL;

static gotcha_wrappee_handle_t sigprocmask_handle;
static gotcha_wrappee_handle_t dlopen_handle;

static sigprocmask_fptr real_sigprocmask;
static dlopen_fptr real_dlopen;

#define GET_REAL_FUNC(var, name)                \
    if ( var == NULL ) {                        \
        var = dlsym(RTLD_NEXT, name);           \
        if ( var == NULL ) {                    \
            errx(1, "dlsym(" name ")failed");   \
        }                                       \
    }

//----------------------------------------------------------------------

static int
sigprocmask_wrap(int how, void * set, void * oldset)
{
    printf("---> hybrid: override: sigprocmask\n");

    int ret = real_sigprocmask(how, set, oldset);

    return ret;
}

static void *
dlopen_wrap(char * name, int flags)
{
    printf("---> hybrid: override: dlopen(%s)\n", name);

    void * ret = real_dlopen(name, flags);

    return ret;
}

static gotcha_binding_t bindings [] = {
    { "sigprocmask", sigprocmask_wrap, &sigprocmask_handle },
    { "dlopen", dlopen_wrap, &dlopen_handle },
};

static void
wrap_functions(void)
{
    static int wrap_done = 0;

    if (wrap_done) {
	return;
    }
    wrap_done = 1;

    int i, n = sizeof(bindings)/sizeof(bindings[0]);

    gotcha_wrap(bindings, n, "hybrid");

    real_sigprocmask = (sigprocmask_fptr) gotcha_get_wrappee(sigprocmask_handle);
    real_dlopen = (dlopen_fptr) gotcha_get_wrappee(dlopen_handle);

    for (i = 0; i < n; i++) {
        printf("---> hybrid: wrap:  %12s  -->  %p\n", bindings[i].name,
               (void *) gotcha_get_wrappee(*(bindings[i].function_handle)));
    }
}

int
__libc_start_main(void * main, int argc, char ** argv, void * init,
                  void * fini, void * rtld, void * stack_end)
{
    printf("---> hybrid: override: libc_start_main\n");

    GET_REAL_FUNC(real_start_main, "__libc_start_main");

    wrap_functions();

    int ret = (* real_start_main) (main, argc, argv, init, fini, rtld, stack_end);

    return ret;
}

int
pthread_create(void *tid, void *attr, void *st_routine, void *arg)
{
    printf("---> hybrid: override: pthread_create\n");

    GET_REAL_FUNC(real_pthread_create, "pthread_create");

    wrap_functions();

    int ret = real_pthread_create(tid, attr, st_routine, arg);

    return ret;
}

void __attribute__ ((constructor))
monitor_init_ctor(void)
{
    printf("---> hybrid: init ctor\n");

    wrap_functions();
}
