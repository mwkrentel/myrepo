/*
 *  Override pthread_create().
 *
 *  Copyright (c) 2019-2020, Rice University.
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
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(MONITOR_PURE_PRELOAD) || defined(MONITOR_GOTCHA_PRELOAD)
#include <dlfcn.h>
#endif
#if defined(MONITOR_GOTCHA_PRELOAD) || defined(MONITOR_GOTCHA_LINK)
#include <gotcha/gotcha.h>
#endif
#include <pthread.h>

#include "monitor-config.h"
#include "monitor-common.h"
#include "monitor.h"

typedef void * (pthread_start_fcn_t) (void *);
typedef int (pthread_create_fcn_t)
    (pthread_t *, const pthread_attr_t *, pthread_start_fcn_t *, void *);

struct monitor_thread_node {
    pthread_start_fcn_t * tn_start_routine;
    void * tn_arg;
};

#if defined(MONITOR_GOTCHA_PRELOAD) || defined(MONITOR_GOTCHA_LINK)
static gotcha_wrappee_handle_t  pthread_create_handle;
#endif

#if defined(MONITOR_STATIC)
extern pthread_create_fcn_t  __real_pthread_create;
#else
static pthread_create_fcn_t  * real_pthread_create;
#endif

//----------------------------------------------------------------------

/*
 *  Pthread cancel function.  When a thread is canceled, this is
 *  called and the start routine does not return.
 */
static void
monitor_thread_cleanup_routine(void *arg)
{
    monitor_end_thread_cb();
}

//----------------------------------------------------------------------

/*
 *  This is the start routine that we pass to the real
 *  pthread_create(), where the newly created thread begins.
 */
static void *
monitor_thread_start_routine(void *arg)
{
    struct monitor_thread_node *tn = (struct monitor_thread_node *) arg;
    void *ret;

    monitor_begin_thread_cb();

    pthread_cleanup_push(monitor_thread_cleanup_routine, NULL);

    ret = (tn->tn_start_routine) (tn->tn_arg);

    pthread_cleanup_pop(0);

    monitor_end_thread_cb();

    return ret;
}

//----------------------------------------------------------------------

/*
 *  Override pthread_create().
 */
#if defined(MONITOR_PURE_PRELOAD) || defined(MONITOR_GOTCHA_PRELOAD)
int pthread_create
#else
int __wrap_pthread_create
#endif
(pthread_t * thread, const pthread_attr_t * attr,
 pthread_start_fcn_t * start_routine, void * arg)
{
    int ret;

    struct monitor_thread_node *tn =
	(struct monitor_thread_node *) malloc(sizeof(struct monitor_thread_node));

    tn->tn_start_routine = start_routine;
    tn->tn_arg = arg;

#if defined(MONITOR_PURE_PRELOAD) || defined(MONITOR_GOTCHA_PRELOAD)
    GET_DLSYM_FUNC(real_pthread_create, "pthread_create");
#endif

#if defined(MONITOR_GOTCHA_PRELOAD) || defined(MONITOR_GOTCHA_LINK)
    monitor_gotcha_init();
#endif

    monitor_try_begin_process();

#if defined(MONITOR_STATIC)
    ret = __real_pthread_create (thread, attr, &monitor_thread_start_routine, tn);
#else
    ret = (* real_pthread_create) (thread, attr, &monitor_thread_start_routine, tn);
#endif

    return ret;
}

//----------------------------------------------------------------------

#ifdef MONITOR_GOTCHA_LINK

/*
 *  Init and preinit constructors to turn on gotcha wrap for the
 *  gotcha link case.
 *
 *  Preinit comes before the library init constructors.  We need to
 *  wrap pthread_create() here in order to catch a new thread from an
 *  init constructor before main().  But this is too early for all
 *  wraps or the begin_process() callback.
 */

static gotcha_binding_t thread_bindings [] = {
    { "pthread_create", __wrap_pthread_create, &pthread_create_handle },
};

/*
 *  Gotcha wrap pthread_create().  This is too early for the other
 *  functions or for the begin process callback.
 */
void
monitor_preinit_ctor(void)
{
    gotcha_wrap(thread_bindings, 1, "libmonitor");

    real_pthread_create = (pthread_create_fcn_t *) gotcha_get_wrappee(pthread_create_handle);
}

__attribute__ ((section(".preinit_array")))
typeof(monitor_preinit_ctor) * monitor_preinit = monitor_preinit_ctor;

/*
 *  Gotcha wrap the rest of the functions.
 */
void __attribute__ ((constructor))
monitor_init_ctor(void)
{
    monitor_gotcha_init();
    monitor_try_begin_process();
}

#endif
