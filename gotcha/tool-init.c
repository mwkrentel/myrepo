/*
 *  gotcha_tool_init() -- call this explicitly right after main() from
 *  within the application.
 *
 *  Normally, this is a separate library and compiled with GNU.
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

typedef int (* pthread_create_fptr) (void *, void *, void *, void *);
typedef int (* sigprocmask_fptr) (int, void *, void *);
typedef void * (* dlopen_fptr) (char *, int);

static gotcha_wrappee_handle_t pthread_create_handle;
static gotcha_wrappee_handle_t sigprocmask_handle;
static gotcha_wrappee_handle_t dlopen_handle;

static pthread_create_fptr real_pthread_create;
static sigprocmask_fptr real_sigprocmask;
static dlopen_fptr real_dlopen;

//----------------------------------------------------------------------

static int
pthread_create_wrap(void * thread, void * attr, void * start, void * arg)
{
    printf("---> gotcha: override: pthread_create\n");

    int ret = real_pthread_create(thread, attr, start, arg);

    return ret;
}

static int
sigprocmask_wrap(int how, void * set, void * old_set)
{
    printf("---> gotcha: override: sigprocmask\n");

    int ret = real_sigprocmask(how, set, old_set);

    return ret;
}

static void *
dlopen_wrap(char * name, int flags)
{
    printf("---> gotcha: override: dlopen(%s)\n", name);

    void * ret = real_dlopen(name, flags);

    return ret;
}

static gotcha_binding_t bindings [] = {
    { "pthread_create", pthread_create_wrap, &pthread_create_handle },
    { "sigprocmask", sigprocmask_wrap, &sigprocmask_handle },
    { "dlopen", dlopen_wrap, &dlopen_handle },
};

void
gotcha_tool_init(void)
{
    int i, n = sizeof(bindings)/sizeof(bindings[0]);

    printf("---> gotcha: init ctor\n");

    gotcha_wrap(bindings, n, "monitor");

    real_pthread_create = (pthread_create_fptr) gotcha_get_wrappee(pthread_create_handle);
    real_sigprocmask = (sigprocmask_fptr) gotcha_get_wrappee(sigprocmask_handle);
    real_dlopen = (dlopen_fptr) gotcha_get_wrappee(dlopen_handle);

    for (i = 0; i < n; i++) {
	printf("---> gotcha: wrap:  %18s  -->  %p\n", bindings[i].name,
	       (void *) gotcha_get_wrappee(*(bindings[i].function_handle)));
    }
}
