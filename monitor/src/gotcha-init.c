/*
 *  Gotcha init functions.
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
 *  This file is only included for the two gotcha cases.
 */

#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>

#include <gotcha/gotcha.h>

#include "monitor-config.h"
#include "monitor-common.h"
#include "monitor.h"

static volatile int gotcha_init_start = 0;
static volatile int gotcha_init_done = 0;

//----------------------------------------------------------------------

/*
 *  The first thread to get here calls gotcha init for each module.
 *  The other threads wait until init finishes.
 */
void
monitor_gotcha_init(void)
{
    if (gotcha_init_done) {
	return;
    }

    if (__sync_bool_compare_and_swap(&gotcha_init_start, 0, 1))
    {
#ifdef MONITOR_USE_DLOPEN
	monitor_gotcha_init_dlopen();
#endif
	gotcha_init_done = 1;
    }
    else {
	while (! gotcha_init_done)
	    ;
    }
}
