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
 *  Mark W. Krentel
 *  January 2020
 *
 *  ----------------------------------------------------------------------
 *
 *  Use libmonitor to attach to a program, turn on REALTIME or CPUTIME
 *  interrupts per process and thread, and report the elapsed time and
 *  number of interrupts.  If the program segfaults, dump the last few
 *  interrupts per thread.
 *
 *  This tests if a high rate of interrupts causes problems for an
 *  application.
 *
 *  Link with -lrt and -lpthread.
 *
 *  Usage:
 *    export EVENT='name@period'
 *    monitor-run -i librealtime.so app ...
 *
 *  where name is 'real' or 'cpu', and period is time in
 *  micro-seconds.
 *
 *  ----------------------------------------------------------------------
 *
 *  Todo:
 *    3. support many threads, threads that end before the process,
 *    etc.
 *
 *    5. add alarm in case of deadlock.
 *
 *    6. for pc addrs in .so library, compute file name, load address
 *    and offset.
 */

#define _GNU_SOURCE

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

#include <pthread.h>
#include <ucontext.h>

#include "monitor.h"

#define REALTIME_NAME  "REALTIME"
#define CPUTIME_NAME   "CPUTIME"
#define REALTIME_CLOCK_TYPE  CLOCK_REALTIME
#define CPUTIME_CLOCK_TYPE   CLOCK_THREAD_CPUTIME_ID
#define NOTIFY_METHOD   SIGEV_THREAD_ID
#define PROF_SIGNAL    (SIGRTMIN + 4)

#define MAX_THREADS  550
#define NUM_SAMPLES   40

#define DEFAULT_PERIOD  4000
#define MILLION   1000000

#define MAGIC  0x004ea1004ea1

struct thread_info {
    long  magic;
    long  tnum;
    long  count;
    struct sigevent sigev;
    timer_t  timerid;
    struct timeval  start;
    struct sample_info * sinfo;
};

struct sample_info {
    void *pc;
    long  usec;
};

static struct thread_info thread_array[MAX_THREADS];

static long next_thread = 1;

static struct itimerspec itspec_start;
static struct itimerspec itspec_stop;

static clockid_t clock_type;
static char *clock_name;
static long  period;

static pthread_key_t key;

static struct timeval proc_start;

static int at_end_of_process = 0;

static int my_pid = 0;

static void dump_samples(void);

//----------------------------------------------------------------------
//  POSIX timer functions
//----------------------------------------------------------------------

static void
create_timer(struct thread_info *tid)
{
    memset(&tid->sigev, 0, sizeof(tid->sigev));
    tid->sigev.sigev_notify = NOTIFY_METHOD;
    tid->sigev.sigev_signo = PROF_SIGNAL;
    tid->sigev.sigev_value.sival_ptr = &tid->timerid;
    tid->sigev._sigev_un._tid = syscall(SYS_gettid);

    if (timer_create(clock_type, &tid->sigev, &tid->timerid) != 0) {
	err(1, "timer create failed");
    }
}

static void
start_timer(struct thread_info *tid)
{
    if (timer_settime(tid->timerid, 0, &itspec_start, NULL) != 0) {
	err(1, "timer start failed");
    }
}

static void
stop_timer(struct thread_info *tid)
{
    if (timer_settime(tid->timerid, 0, &itspec_stop, NULL) != 0) {
	err(1, "timer stop failed");
    }
}

static void
delete_timer(struct thread_info *tid)
{
    if (timer_delete(tid->timerid) != 0) {
	warn("timer delete failed");
    }
}

//----------------------------------------------------------------------
//  Interrupt and analysis functions
//----------------------------------------------------------------------

static void
do_sample(struct thread_info *tid, void *context)
{
    struct ucontext *ucontext = (struct ucontext *) context;
    mcontext_t *mcontext = &(ucontext->uc_mcontext);
    struct timeval now;
    void *pc = NULL;

    gettimeofday(&now, NULL);

    long usec = MILLION * (now.tv_sec - proc_start.tv_sec)
	+ (now.tv_usec - proc_start.tv_usec);

#if defined(__x86_64__)
#ifndef REG_RIP
#define REG_RIP  16
#endif
    pc = (void *) mcontext->gregs[REG_RIP];

#elif defined(__powerpc64__)
#ifndef PPC_REG_PC
#define PPC_REG_PC  32
#endif
    pc = (void *) mcontext->gp_regs[PPC_REG_PC];

#elif defined(__aarch64__)
    pc = (void *) mcontext->pc;

#else
#error architecture not supported
#endif

    long slot = tid->count % NUM_SAMPLES;
    tid->sinfo[slot].pc = pc;
    tid->sinfo[slot].usec = usec;

    tid->count++;
}

//----------------------------------------------------------------------
//  Signal handler functions
//----------------------------------------------------------------------

static void
my_handler(int sig, siginfo_t *info, void *context)
{
    if (at_end_of_process) {
	return;
    }

    struct thread_info *tid = pthread_getspecific(key);

    if (tid == NULL) {
	//
	// probably a stray "drain the swamp" signal at thread tear
	// down
	//
	warnx("pthread_getspecific returns NULL");
	return;
    }
    else if (tid->magic != MAGIC) {
	//
	// internal corruption, should not happen
	//
	at_end_of_process = 1;
	warnx("pthread_getspecific bad magic");
	dump_samples();
	abort();
    }

    do_sample(tid, context);
    start_timer(tid);
}

static void
segv_handler(int sig, siginfo_t *info, void *context)
{
    struct sigaction act;
    sigset_t set;

    at_end_of_process = 1;

    // reset handler to default
    memset(&act, 0, sizeof(act));
    sigemptyset(&act.sa_mask);
    act.sa_handler = SIG_DFL;
    if (sigaction(SIGSEGV, &act, NULL) != 0) {
        err(1, "reset sigaction failed");
    }
    if (sigaction(SIGBUS, &act, NULL) != 0) {
        err(1, "reset sigaction failed");
    }

    // unblock segfaults
    sigemptyset(&set);
    sigaddset(&set, SIGSEGV);
    sigaddset(&set, SIGBUS);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    dump_samples();
    abort();
}

//----------------------------------------------------------------------
//  Printing functions
//----------------------------------------------------------------------

/*
 *  Normal summary on success.
 */
static void
print_summary(void)
{
    struct timeval now;
    long total = 0;
    double diff;

    gettimeofday(&now, NULL);

    if (period < 1) { period = 1; }
    if (next_thread > MAX_THREADS) { next_thread = MAX_THREADS; }

    printf("event: %s   period: %ld usec   rate: %.1f per sec\n",
	   clock_name, period, ((double) MILLION) / period);

    for (int i = 0; i < next_thread; i++) {
	diff = (now.tv_sec - thread_array[i].start.tv_sec)
	    + ((double) (now.tv_usec - thread_array[i].start.tv_usec)) / MILLION;

	if (diff < 0.001) { diff = 0.001; }

	printf("tid: %3d   time: %.3f sec   count: %ld   rate: %.1f per sec\n",
	       i, diff, thread_array[i].count, thread_array[i].count / diff);

	total += thread_array[i].count;
    }

    diff = (now.tv_sec - thread_array[0].start.tv_sec)
	+ ((double) (now.tv_usec - thread_array[0].start.tv_usec)) / MILLION;

    if (diff < 0.001) { diff = 0.001; }

    printf("time: %.3f sec   total: %ld   rate: %.1f per sec\n",
	   diff, total, total / diff);
}

//----------------------------------------------------------------------

/*
 *  On error, dump recent interrupts.
 */
static void
dump_samples(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);

    for (int i = 0; i < next_thread; i++) {
	struct thread_info *tid = &thread_array[i];

	double diff = (now.tv_sec - tid->start.tv_sec)
	    + ((double) (now.tv_usec - tid->start.tv_usec)) / MILLION;

	printf("\npid: %6d    tid: %4d    ----------------------------------------\n"
	       "pid: %6d    tid: %4d    time: %.3f sec    count: %ld\n",
	       my_pid, i, my_pid, i, diff, tid->count);

	long slot = (tid->count < NUM_SAMPLES) ? 0 : (tid->count % NUM_SAMPLES);
	long num =  (tid->count < NUM_SAMPLES) ? tid->count : NUM_SAMPLES;

	for (long j = 0; j < num; j++) {
	    long sec = tid->sinfo[slot].usec / MILLION;
	    long usec = tid->sinfo[slot].usec % MILLION;

	    printf("pid: %6d    tid: %4d    usec: %4ld.%06ld    %p\n",
		   my_pid, i, sec, usec, tid->sinfo[slot].pc);

	    slot = (slot + 1) % NUM_SAMPLES;
	}
    }
}

//----------------------------------------------------------------------
//  Initialization functions
//----------------------------------------------------------------------

static void
init_process(void)
{
    struct sigaction act, segv_act;

    // default clock
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
	    warnx("EVENT does not specify 'real' or 'cpu'");
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

    // profiling handler
    memset(&act, 0, sizeof(act));
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = my_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    if (sigaction(PROF_SIGNAL, &act, NULL) != 0) {
        err(1, "sigaction failed");
    }

    // segfault handler
    memset(&segv_act, 0, sizeof(segv_act));
    sigemptyset(&segv_act.sa_mask);
    segv_act.sa_sigaction = segv_handler;
    segv_act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &segv_act, NULL) != 0) {
        err(1, "segv sigaction failed");
    }
    if (sigaction(SIGBUS, &segv_act, NULL) != 0) {
        err(1, "segv sigaction failed");
    }

    if (pthread_key_create(&key, NULL) != 0) {
	err(1, "pthread_key_create failed");
    }

    my_pid = getpid();

    gettimeofday(&proc_start, NULL);
}

//----------------------------------------------------------------------

static void
mk_thread_info(long tnum)
{
    if (tnum < 0 || tnum >= MAX_THREADS) {
	errx(1, "thread num out of range: %ld", tnum);
    }

    struct thread_info *tid = &thread_array[tnum];

    memset(tid, 0, sizeof(*tid));
    tid->magic = MAGIC;
    tid->tnum = tnum;
    tid->count = 0;
    gettimeofday(&tid->start, NULL);

    tid->sinfo = (struct sample_info *) malloc(NUM_SAMPLES * sizeof(struct sample_info));
    if (tid->sinfo == NULL) {
	err(1, "malloc for sample info array failed");
    }

    create_timer(tid);

    if (pthread_setspecific(key, tid) != 0) {
	err(1, "pthread_setspecific failed");
    }
}

//----------------------------------------------------------------------
//  Monitor callback functions
//----------------------------------------------------------------------

/*
 *  Drain the queue of any leftover signals and block the profiling
 *  signal at end of process or thread time.  There's a risk that a
 *  leftover signal might run in the handler after thread specific
 *  data has gone away.
 */
static void
drain_signal_queue(void)
{
    struct timespec tspec;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, PROF_SIGNAL);
    tspec.tv_sec = 0;
    tspec.tv_nsec = 10;

    while (sigtimedwait(&set, NULL, &tspec) > 0)
	;

    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void
monitor_begin_process_cb(void)
{
    init_process();

    printf("---> begin process  (pid %d)  %s at %ld\n",
	   my_pid, clock_name, period);

    mk_thread_info(0);
    start_timer(&thread_array[0]);
}

void
monitor_at_main_cb(void)
{
}

void
monitor_end_process_cb(void)
{
    at_end_of_process = 1;

    stop_timer(&thread_array[0]);
    delete_timer(&thread_array[0]);
    drain_signal_queue();

    printf("\n---> end process  (pid %d)\n", my_pid);

    print_summary();
}

void
monitor_begin_thread_cb(void)
{
    long tnum = __sync_fetch_and_add(&next_thread, 1);

    if (tnum < 0 || tnum >= MAX_THREADS) {
	warnx("out of threads: %ld", tnum);
	return;
    }

    mk_thread_info(tnum);
    start_timer(&thread_array[tnum]);
}

void
monitor_end_thread_cb(void)
{
    struct thread_info *tid = pthread_getspecific(key);

    if (tid == NULL || tid->magic != MAGIC) {
	errx(1, "pthread_getspecific failed");
    }

    stop_timer(tid);
    delete_timer(tid);
    drain_signal_queue();
}
