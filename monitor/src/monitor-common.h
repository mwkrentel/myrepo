/*
 *  Internal shared declarations.
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

#ifndef _MONITOR_COMMON_H_
#define _MONITOR_COMMON_H_

#include <dlfcn.h>
#include <err.h>
#include <errno.h>

#include "monitor-config.h"

/*
 *  There are four build cases and exactly one of these must be
 *  defined:
 *    MONITOR_PURE_PRELOAD, MONITOR_GOTCHA_PRELOAD,
 *    MONITOR_GOTCHA_LINK or MONITOR_STATIC.
 */
#if (defined(MONITOR_PURE_PRELOAD) && defined(MONITOR_GOTCHA_PRELOAD))     \
    || (defined(MONITOR_PURE_PRELOAD) && defined(MONITOR_GOTCHA_LINK))     \
    || (defined(MONITOR_PURE_PRELOAD) && defined(MONITOR_STATIC))          \
    || (defined(MONITOR_GOTCHA_PRELOAD) && defined(MONITOR_GOTCHA_LINK))   \
    || (defined(MONITOR_GOTCHA_PRELOAD) && defined(MONITOR_STATIC))        \
    || (defined(MONITOR_GOTCHA_LINK) && defined(MONITOR_STATIC))
#error cannot define more than one of: MONITOR_PURE_PRELOAD, \
MONITOR_GOTCHA_PRELOAD, MONITOR_GOTCHA_LINK or MONITOR_STATIC
#endif
#if !defined(MONITOR_PURE_PRELOAD) && !defined(MONITOR_GOTCHA_PRELOAD)     \
    && !defined(MONITOR_GOTCHA_LINK) && !defined(MONITOR_STATIC)
#error must define one of: MONITOR_PURE_PRELOAD, MONITOR_GOTCHA_PRELOAD, \
MONITOR_GOTCHA_LINK or MONITOR_STATIC
#endif

//----------------------------------------------------------------------

#ifndef RTLD_NEXT
#define RTLD_NEXT  ((void *) -1l)
#endif

#define GET_DLSYM_FUNC(var, name)		\
    if ( var == NULL ) {			\
        var = dlsym(RTLD_NEXT, name);		\
	if ( var == NULL ) {			\
	    errx(1, "dlsym(" name ")failed");	\
	}					\
    }

//----------------------------------------------------------------------

void monitor_try_begin_process(void);

#endif  // _MONITOR_COMMON_H_
