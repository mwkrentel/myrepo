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
 *
 *  ----------------------------------------------------------------------
 *
 *  This file is not used in the static case.
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

#if defined(MONITOR_PURE_PRELOAD)

/*
 *  Initialization for the pure preload case.  We enter this code from
 *  a preload override, regardless of thread.
 *
 *  At the first override, use dlsym(RTLD_NEXT) to get the real
 *  versions.
 */

static volatile int dlopen_init_start = 0;
static volatile int dlopen_init_done = 0;

static dlopen_fcn_t  * real_dlopen = NULL;
static dlclose_fcn_t * real_dlclose = NULL;

static void
monitor_preload_init_dlopen(void)
{
    if (dlopen_init_done) {
	return;
    }

    if (__sync_bool_compare_and_swap(&dlopen_init_start, 0, 1))
    {
	GET_DLSYM_FUNC(real_dlopen, "dlopen");
	GET_DLSYM_FUNC(real_dlclose, "dlclose");

	__sync_synchronize();

	dlopen_init_done = 1;
    }
    else {
	while (! dlopen_init_done)
	    ;
    }
}
#endif

//----------------------------------------------------------------------

#if defined(MONITOR_GOTCHA_ANY)

/*
 *  Initialization for the gotcha preload and gotcha link cases.
 *  This is already serialized from gotcha-init.
 */

void * __wrap_dlopen (const char *, int);
int __wrap_dlclose (void *);

static gotcha_wrappee_handle_t dlopen_handle;
static gotcha_wrappee_handle_t dlclose_handle;

static gotcha_binding_t dlopen_bindings [] = {
    { "dlopen",  __wrap_dlopen,  &dlopen_handle },
    { "dlclose", __wrap_dlclose, &dlclose_handle },
};

void
monitor_gotcha_init_dlopen(void)
{
    gotcha_wrap(dlopen_bindings, 2, "libmonitor");
}
#endif

//----------------------------------------------------------------------

/*
 *  Override dlopen.
 */
#if defined(MONITOR_PURE_PRELOAD)
void * dlopen
#else
void * __wrap_dlopen
#endif
  (const char * name, int flags)
{
    monitor_first_entry();

#if defined(MONITOR_PURE_PRELOAD)
    monitor_preload_init_dlopen();

#elif defined(MONITOR_GOTCHA_PRELOAD) || defined(MONITOR_GOTCHA_LINK)
    dlopen_fcn_t * real_dlopen = gotcha_get_wrappee(dlopen_handle);

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

/*
 *  Override dlclose.
 */
#if defined(MONITOR_PURE_PRELOAD)
int dlclose
#else
int __wrap_dlclose
#endif
  (void * handle)
{
    monitor_first_entry();

#if defined(MONITOR_PURE_PRELOAD)
    monitor_preload_init_dlopen();

#elif defined(MONITOR_GOTCHA_PRELOAD) || defined(MONITOR_GOTCHA_LINK)
    dlclose_fcn_t * real_dlclose = gotcha_get_wrappee(dlclose_handle);

#endif

    if (real_dlclose == NULL) {
	errx(1, "unable to get real version of dlclose");
    }

    void * data = monitor_pre_dlclose_cb(handle);

    int ret = (* real_dlclose) (handle);

    monitor_post_dlclose_cb(data, handle, ret);

    return ret;
}
