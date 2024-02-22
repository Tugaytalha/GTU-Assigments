#include <stdio.h>
#include <stdlib.h>
#include "util.h"



int main() {
    int i, lp = -1, sequence_lenght, first_element, one = 1;
    int *loop_len, *loop, *histogram, *digit;


    printf("Please enter the sequence lenght: ");
    scanf("%d", &sequence_lenght);
    printf("Please enter the first element: ");
    scanf("%d", &first_element);

    //allocating
    digit = &one;
    loop_len = &lp;
    loop = (int *) malloc(sequence_lenght/2 * sizeof(int));
    histogram = (int *) calloc(9, sizeof(int));

    check_loop_iterative(generate_sequence, first_element, sequence_lenght, loop, loop_len);  // check for loop
    if (*loop_len > 0) {  // if there is a loop print this
        printf("Loop: {%d", loop[0]);
        for (i = 1; i < *loop_len; i++)
            printf(", %d",loop[i]);
        printf("}\n\n");
    }
    else
        printf("No loop found.\n\n");

    //create and print sequence
    hist_of_firstdigits(generate_sequence, first_element, sequence_lenght, histogram, digit);
    printf("Histogram of the sequence: {%d", histogram[0]);
    for (i = 1; i < 9; i++)
        printf(", %d", histogram[i]);
    printf("}\n\n");

    //memory free
    free(loop);

    free(histogram);
    return 0;
}
