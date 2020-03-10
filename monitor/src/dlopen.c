/*
 *  Libmonitor dlopen functions.
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

#define _GNU_SOURCE

#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>

#include <dlfcn.h>

#if defined(MONITOR_GOTCHA_PRELOAD) || defined(MONITOR_GOTCHA_LINK)
#include <gotcha/gotcha.h>
#endif

#include "monitor-config.h"
#include "monitor-common.h"
#include "monitor.h"

typedef void * dlopen_fcn_t (const char *, int);
typedef int dlclose_fcn_t (void *);

//----------------------------------------------------------------------

/*
 * Initialization for the pure preload case.  We enter this code from
 * a preload override, regardless of thread.
 *
 * At the first override, use dlsym(RTLD_NEXT) to get the real
 * versions.
 */
#if defined(MONITOR_PURE_PRELOAD)

static volatile int dlopen_init_done = 0;

static dlopen_fcn_t  * real_dlopen = NULL;
static dlclose_fcn_t * real_dlclose = NULL;

static void
monitor_init_dlopen(void)
{
    void *func = NULL;

    if (dlopen_init_done) {
	return;
    }

    if (real_dlopen == NULL) {
	GET_DLSYM_FUNC(func, "dlopen");
	__sync_val_compare_and_swap(&real_dlopen, NULL, func);
    }

    func = NULL;
    if (real_dlclose == NULL) {
	GET_DLSYM_FUNC(func, "dlclose");
	__sync_val_compare_and_swap(&real_dlclose, NULL, func);
    }

    dlopen_init_done = 1;
}
#endif

//----------------------------------------------------------------------

void *
dlopen(const char * name, int flags)
{
#if defined(MONITOR_PURE_PRELOAD)
    monitor_init_dlopen();
#endif

    if (real_dlopen == NULL) {
	errx(1, "unable to get real version of dlopen");
    }

    void * data = monitor_pre_dlopen_cb(name, flags);

    void * handle = (* real_dlopen) (name, flags);

    monitor_post_dlopen_cb(data, handle);

    return handle;
}

//----------------------------------------------------------------------

int
dlclose(void * handle)
{
#if defined(MONITOR_PURE_PRELOAD)
    monitor_init_dlopen();
#endif

    if (real_dlclose == NULL) {
	errx(1, "unable to get real version of dlclose");
    }

    void * data = monitor_pre_dlclose_cb(handle);

    int ret = (* real_dlclose) (handle);

    monitor_post_dlclose_cb(data, handle, ret);

    return ret;
}
