/*
 *  The 'preinit' version.  Override all functions with gotcha and run
 *  gotcha_wrap() from a preinit constructor.  This file must be
 *  linked into the application (for "always on" profiling).
 *
 *  Copyright (c) 2019, Rice University.
 *  See the file LICENSE for details.
 *
 *  Mark W. Krentel
 *  August 2019
 */

#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "gotcha/gotcha.h"

typedef int (* start_main_fptr) (void *, int, char **, void *,
				 void *, void *, void *);
typedef int (* pthread_create_fptr) (void *, void *, void *, void *);
typedef int (* sigprocmask_fptr) (int, void *, void *);
typedef void * (* dlopen_fptr) (char *, int);

static gotcha_wrappee_handle_t start_main_handle;
static gotcha_wrappee_handle_t pthread_create_handle;
static gotcha_wrappee_handle_t sigprocmask_handle;
static gotcha_wrappee_handle_t dlopen_handle;

static start_main_fptr real_start_main;
static pthread_create_fptr real_pthread_create;
static sigprocmask_fptr real_sigprocmask;
static dlopen_fptr real_dlopen;

//----------------------------------------------------------------------

static int
start_main_wrap(void * main, int argc, char ** argv, void * init,
		void * fini, void * rtld, void * stack_end)
{
    printf("---> preinit: override: libc_start_main\n");

    int ret = real_start_main(main, argc, argv, init, fini, rtld, stack_end);

    return ret;
}

static int
pthread_create_wrap(void * thread, void * attr, void * start, void * arg)
{
    printf("---> preinit: override: pthread_create\n");

    int ret = real_pthread_create(thread, attr, start, arg);

    return ret;
}

static int
sigprocmask_wrap(int how, void * set, void * old_set)
{
    printf("---> preinit: override: sigprocmask\n");

    int ret = real_sigprocmask(how, set, old_set);

    return ret;
}

static void *
dlopen_wrap(char * name, int flags)
{
    printf("---> preinit: override: dlopen(%s)\n", name);

    void * ret = real_dlopen(name, flags);

    return ret;
}

static gotcha_binding_t bindings [] = {
    { "__libc_start_main", start_main_wrap, &start_main_handle },
    { "pthread_create", pthread_create_wrap, &pthread_create_handle },
    { "sigprocmask", sigprocmask_wrap, &sigprocmask_handle },
    { "dlopen", dlopen_wrap, &dlopen_handle },
};

void
monitor_preinit_ctor(void)
{
    int i, n = sizeof(bindings)/sizeof(bindings[0]);

    printf("---> preinit: init ctor\n");

    gotcha_wrap(bindings, n, "preinit-monitor");

    real_start_main = (start_main_fptr) gotcha_get_wrappee(start_main_handle);
    real_pthread_create = (pthread_create_fptr) gotcha_get_wrappee(pthread_create_handle);
    real_sigprocmask = (sigprocmask_fptr) gotcha_get_wrappee(sigprocmask_handle);
    real_dlopen = (dlopen_fptr) gotcha_get_wrappee(dlopen_handle);

    for (i = 0; i < n; i++) {
	printf("---> preinit: wrap:  %18s  -->  %p\n", bindings[i].name,
	       (void *) gotcha_get_wrappee(*(bindings[i].function_handle)));
    }
}

__attribute__ ((section(".preinit_array")))
typeof(monitor_preinit_ctor) * __preinit = monitor_preinit_ctor;
