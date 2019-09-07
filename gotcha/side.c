/*
 *  Side library for main to print 'hello, world' from init
 *  constructor.
 *
 *  Copyright (c) 2019, Rice University.
 *  See the file LICENSE for details.
 *
 *  Mark W. Krentel
 *  September 2019
 */

#include <errno.h>
#include <stdio.h>

static int num = 0;

void __attribute__ ((constructor))
sidelib_ctor(void)
{
    printf("sidelib: init ctor, num = %d\n", num);
}

void
sidelib_early(void)
{
    printf("sidelib: early entry\n");
    num = 1;
}
