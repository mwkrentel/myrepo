/*
 *  Override pthread_create().
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
#include <stdlib.h>

#include <pthread.h>

#include "monitor-config.h"
#include "common.h"

typedef void * (pthread_start_fcn_t) (void *);
typedef int (pthread_create_fcn_t)
    (pthread_t *, const pthread_attr_t *, pthread_start_fcn_t *, void *);

struct monitor_thread_node {
    pthread_start_fcn_t * tn_start_routine;
    void * tn_arg;
};

extern pthread_create_fcn_t __real_pthread_create;

//----------------------------------------------------------------------

/*
 *  This is the start routine that we pass to the real
 *  pthread_create(), where the newly created thread begins.
 */
static void *
monitor_begin_thread(void *arg)
{
    struct monitor_thread_node *tn = (struct monitor_thread_node *) arg;
    void *ret;

    printf("---> monitor: begin thread\n");

    ret = (tn->tn_start_routine) (tn->tn_arg);

    printf("---> monitor: end thread\n");

    return ret;
}

//----------------------------------------------------------------------

/*
 *  Override pthread_create().
 */
#ifdef MONITOR_PRELOAD
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

#ifdef MONITOR_PRELOAD
    pthread_create_fcn_t *real_pthread_create = NULL;

    GET_DLSYM_FUNC(real_pthread_create, "pthread_create");

    ret = (* real_pthread_create) (thread, attr, &monitor_begin_thread, tn);

#else
    ret = __real_pthread_create (thread, attr, &monitor_begin_thread, tn);
#endif

    return ret;
}