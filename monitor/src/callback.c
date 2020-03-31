/*
 *  Libmonitor default callback functions.
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

#include <stdio.h>

#include "monitor.h"
#include "monitor-common.h"

#define DEBUG_PREFIX  "---> monitor: "

#define MONITOR_DEBUG1(fmt) 			\
    if (monitor_debug()) { 			\
	fprintf(stderr, DEBUG_PREFIX fmt);	\
    }

#define MONITOR_DEBUG(fmt, ...) 		\
    if (monitor_debug()) { 			\
        fprintf(stderr, DEBUG_PREFIX fmt, __VA_ARGS__);  \
    }

//----------------------------------------------------------------------

void  __attribute__ ((weak))
monitor_begin_process_cb(void)
{
    MONITOR_DEBUG1("begin process\n");
}

void  __attribute__ ((weak))
monitor_at_main_cb(void)
{
    MONITOR_DEBUG1("at main\n");
}

void  __attribute__ ((weak))
monitor_end_process_cb(void)
{
    MONITOR_DEBUG1("end process\n");
}

void  __attribute__ ((weak))
monitor_begin_thread_cb(void)
{
    MONITOR_DEBUG1("begin thread\n");
}

void  __attribute__ ((weak))
monitor_end_thread_cb(void)
{
    MONITOR_DEBUG1("end thread\n");
}

//----------------------------------------------------------------------

void * __attribute__ ((weak))
monitor_pre_dlopen_cb(const char *name, int flags)
{
    MONITOR_DEBUG("pre-dlopen (%s, %d)\n", name, flags);
    return NULL;
}

void  __attribute__ ((weak))
monitor_post_dlopen_cb(void *data, void * handle)
{
    MONITOR_DEBUG("post-dlopen: %p\n", handle);
}

void * __attribute__ ((weak))
monitor_pre_dlclose_cb(void * handle)
{
    MONITOR_DEBUG("pre-dlclose (%p)\n", handle);
    return NULL;
}

void  __attribute__ ((weak))
monitor_post_dlclose_cb(void *data, void * handle, int ret)
{
    MONITOR_DEBUG("post-dlclose: %p  ret: %d\n", handle, ret);
}
