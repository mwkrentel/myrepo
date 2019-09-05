/*
 *  Simple program for openmp without init constructor.  This tests if
 *  the runtime library creates threads on its own.
 *
 *  Copyright (c) 2019, Rice University.
 *  See the file LICENSE for details.
 *
 *  Mark W. Krentel
 *  September 2019
 */

#include <sys/types.h>
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <omp.h>

#define LIBM  "libm.so.6"

typedef double sin_fcn_t (double);

static sin_fcn_t * sin_fcn = NULL;

void gotcha_tool_init(void);

//----------------------------------------------------------------------

double
reduce(double A[], int N)
{
    double ans;
    int i;

    ans = 0.0;

#pragma omp parallel for default(none) private(i)  \
    shared(A, N, sin_fcn)  reduction(+ : ans)

    for (i = 0; i < N; i++) {
        ans += (* sin_fcn)(A[i]);
    }

    return ans;
}

int
main(int argc, char **argv)
{
    printf("main: first entry\n");
    gotcha_tool_init();
  
    int i, j, N;

    if (argc < 2 || sscanf(argv[1], "%d", &N) < 1) {
        N = 1000;
    }
    printf("main: N = %d\n", N);

    double * A = (double *) malloc(N * sizeof(double));
    if (A == NULL) {
	err(1, "malloc array failed");
    }

    printf("main: calling dlopen() and sigprocmask() ...\n");

    void * handle = dlopen(LIBM, RTLD_LAZY);
    if (handle == NULL) {
        err(1, "unable to dlopen(libm)");
    }

    sin_fcn = dlsym(handle, "sin");
    if (sin_fcn == NULL) {
        err(1, "unable to dlsym(sin)");
    }

#pragma omp parallel
    {
        sigset_t * set = (sigset_t *) malloc(sizeof(sigset_t));
        sigemptyset(set);
        int ret = sigprocmask(SIG_BLOCK, set, NULL);

	if (ret != 0) {
	    warn("sigprocmask() failed");
	}

	void * handle = dlopen(LIBM, RTLD_LAZY);

	if (handle == NULL) {
	    warn("dlopen() failed");
	}
    }

    double ans = 0.0;

    for (j = 0; j < N; j++) {

#pragma omp parallel for default(none) private(i) shared(A, N)
	for (i = 0; i < N; i++) {
	    A[i] = (double) i;
	}

	ans = reduce(A, N);
    }

    printf("main: ans = %g\n", ans);

    return 0;
}
