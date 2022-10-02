#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>

int
main (int argc, char *argv[])
{
    int fib, max;
    int num_list[4];

    for(int i = 0; i < argc-1; i++){
        num_list[i] = atoi(argv[i+1]);
    }

    fib = fibonacci(num_list[0]);

    max = max_of_four_int(num_list[0], num_list[2], num_list[3], num_list[4]);

    printf("\n%d %d\n", fib, max);

    return EXIT_SUCCESS;
}
