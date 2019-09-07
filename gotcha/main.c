/*
 *  Main program with an init constructor and preinit function.
 *
 *  Copyright (c) 2019, Rice University.
 *  See the file LICENSE for details.
 *
 *  Mark W. Krentel
 *  September 2019
 */

#include <errno.h>
#include <stdio.h>

void sidelib_early(void);

void
main_preinit_func(void)
{
    printf("main: preinit function\n");
    sidelib_early();
}

__attribute__ ((section(".preinit_array")))
void (* __preinit_function) (void) = &main_preinit_func;

void __attribute__ ((constructor))
main_init_ctor(void)
{
    printf("main: init ctor\n");
}

int
main(int argc, char **argv)
{
    printf("main\n");

    return 0;
}
