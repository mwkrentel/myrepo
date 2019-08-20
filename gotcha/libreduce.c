/*
 *  Library to use openmp parallel region and dlopen() in init
 *  constructor.
 *
 *  Copyright (c) 2019, Rice University.
 *  See the file LICENSE for details.
 *
 *  Mark W. Krentel
 *  August 2019
 */

#include <sys/types.h>
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <omp.h>

#define LIBM  "libm.so.6"

typedef double sin_fcn_t (double);

static sin_fcn_t * sin_fcn = NULL;

void __attribute__ ((constructor))
libreduce_ctor(void)
{
    printf("libreduce: init ctor\n");

    void * handle = dlopen(LIBM, RTLD_LAZY);
    if (handle == NULL) {
	err(1, "unable to dlopen(libm)");
    }

    sin_fcn = dlsym(handle, "sin");
    if (sin_fcn == NULL) {
	err(1, "unable to dlsym(sin)");
    }

#pragma omp parallel default(none)
    {
	sigset_t * set = (sigset_t *) malloc(sizeof(sigset_t));
	sigemptyset(set);
	sigprocmask(SIG_BLOCK, set, NULL);
    }
}

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
