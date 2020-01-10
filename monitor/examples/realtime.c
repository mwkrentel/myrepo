/*
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
 *  Mark W. Krentel, Rice University
 *  January 2020
 *
 *  ----------------------------------------------------------------------
 *
 *  Use libmonitor to attach to a program, turn on REALTIME or CPUTIME
 *  interrupts per process and thread, and report the elapsed time and
 *  number of interrupts.
 *
 *  This tests if a high rate of interrupts causes problems for an
 *  application.
 *
 *  Link with -lrt and -lpthread.
 *
 *  Usage:  export EVENT='name@period'
 *   where name is 'real' or 'cpu', and period is time in
 *   micro-seconds.
 *
 *  ----------------------------------------------------------------------
 *
 *  Todo:
 *
 *   1. add support for threads.
 *
 *   2. record ip addrs, so in the event of a crash, can see where
 *   recent interrupts happened.
 */

#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <err.h>
#include <error.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define REALTIME_NAME  "REALTIME"
#define CPUTIME_NAME   "CPUTIME"
#define REALTIME_CLOCK_TYPE  CLOCK_REALTIME
#define CPUTIME_CLOCK_TYPE   CLOCK_THREAD_CPUTIME_ID
#define NOTIFY_METHOD   SIGEV_THREAD_ID
#define PROF_SIGNAL    (SIGRTMIN + 4)

#define DEFAULT_PERIOD  4000
#define MILLION  1000000

static struct itimerspec itspec_start;
static struct itimerspec itspec_stop;

static timer_t  timerid;
static struct sigevent sigev;
static struct timeval  start;
static struct timeval  end;

static clockid_t clock_type;
static char *clock_name;
static long period;

static long count = 0;
static long before_main = 0;

//----------------------------------------------------------------------

static void
create_timer(void)
{
    memset(&sigev, 0, sizeof(sigev));

    sigev.sigev_notify = NOTIFY_METHOD;
    sigev.sigev_signo = PROF_SIGNAL;
    sigev.sigev_value.sival_ptr = &timerid;
    sigev._sigev_un._tid = syscall(SYS_gettid);

    if (timer_create(clock_type, &sigev, &timerid) != 0) {
	err(1, "timer create failed");
    }
}

static void
start_timer(void)
{
    if (timer_settime(timerid, 0, &itspec_start, NULL) != 0) {
	err(1, "timer start failed");
    }
}

static void
stop_timer(void)
{
    if (timer_settime(timerid, 0, &itspec_stop, NULL) != 0) {
	err(1, "timer stop failed");
    }
}

//----------------------------------------------------------------------

void
my_handler(int sig, siginfo_t *info, void *context)
{
    count++;
    start_timer();
}

//----------------------------------------------------------------------

static void
init_profile(void)
{
    struct sigaction act;

    clock_type = REALTIME_CLOCK_TYPE;
    clock_name = REALTIME_NAME;
    period = DEFAULT_PERIOD;

    char *str = getenv("EVENT");

    if (str != NULL) {
	if (strncasecmp(str, "real", 4) == 0) {
	    clock_type = REALTIME_CLOCK_TYPE;
	    clock_name = REALTIME_NAME;
	}
	else if (strncasecmp(str, "cpu", 3) == 0) {
	    clock_type = CPUTIME_CLOCK_TYPE;
	    clock_name = CPUTIME_NAME;
	}
	else {
	    errx(1, "EVENT does not specify 'real' or 'cpu'");
	}

	char *p = strchr(str, '@');
	if (p != NULL) {
	    period = atol(p + 1);
	    if (period <= 0) {
		period = DEFAULT_PERIOD;
	    }
	}
    }

    // one-shot mode, period is usec
    itspec_start.it_value.tv_sec = period / MILLION;
    itspec_start.it_value.tv_nsec = 1000 * (period % MILLION);
    itspec_start.it_interval.tv_sec = 0;
    itspec_start.it_interval.tv_nsec = 0;

    memset(&itspec_stop, 0, sizeof(itspec_stop));

    memset(&act, 0, sizeof(act));
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = my_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    if (sigaction(PROF_SIGNAL, &act, NULL) != 0) {
        err(1, "sigaction failed");
    }
}

//----------------------------------------------------------------------

static void
print_results(void)
{
    double diff = (end.tv_sec - start.tv_sec)
	+ ((double) (end.tv_usec - start.tv_usec)) / MILLION;

    if (diff < 0.001) { diff = 0.001; }
    if (period < 1) { period = 1; }

    printf("event: %s   period: %ld usec   rate: %.1f per sec\n"
	   "time: %.3f sec   before main: %ld   after: %ld   rate: %.1f per sec\n",
	   clock_name, period, ((double) MILLION) / period,
	   diff, before_main, count - before_main, count / diff);
}

//----------------------------------------------------------------------

void
monitor_begin_process_cb(void)
{
    init_profile();

    printf("---> begin process  %s at %ld\n", clock_name, period);

    create_timer();
    gettimeofday(&start, NULL);
    start_timer();
}

void
monitor_at_main_cb(void)
{
    before_main = count;
}

void
monitor_end_process_cb(void)
{
    printf("\n---> end process\n");

    stop_timer();
    gettimeofday(&end, NULL);
    print_results();
}

void
monitor_begin_thread_cb(void)
{
}

void
monitor_end_thread_cb(void)
{
}
