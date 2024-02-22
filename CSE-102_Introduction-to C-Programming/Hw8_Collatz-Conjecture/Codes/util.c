// file for functions
#include <stdio.h>
#include "util.h"
#include <stdlib.h>
#include <string.h>


void generate_sequence (int xs, int currentlen, int seqlen, int *seq) {
    int i, j;

    if (currentlen == seqlen)  // if reach desired lenght
        return ;

    seq[currentlen++] = xs;  // add elemnt to sequence

    // sequence rule: if number is odd next element is 3x+1, if even x/2
    if (xs % 2 == 0)
        generate_sequence(xs/2, currentlen, seqlen, seq);
    else
        generate_sequence(xs*3+1, currentlen, seqlen, seq);
}

void print_list (int *seq, int len) {
    int i;
    printf("{%d",seq[0]);
    for (i = 1; i<len; i++) printf(", %d",seq[i]);
    printf("}\n");
}

void check_loop_iterative(void (*f)(), int xs, int seqlen, int *loop, int *looplen) {
    int i, j, k, b = 0, d, *sequence, *loop_start, *loop_end;
    //allocating
    loop_start = (int *) malloc(sizeof(int));
    loop_end = (int *) malloc(sizeof(int));
    sequence = (int *) malloc(seqlen * sizeof(int));
    f(xs, 0, seqlen, sequence);  // generate sequence

    if (*looplen == -1) {
        printf("\nSequence: ");
        print_list(sequence, seqlen);
        *looplen = seqlen/2 + 1; // maximum possible loop lenght + 1 (will decreased)
        printf("\n");
    }
    if (*looplen == 2) { // check for 2 loop and terminate
        if (has_loop(sequence, seqlen, *looplen, loop_start, loop_end)) {
            printf("The indexes of the loop's first occurance: %d (first digit), %d (last digit)\n", *loop_start, *loop_end);
            for (i = (*loop_start), k = 0; i < (*loop_end) ; k++, i++) {

                loop[k] = sequence[i];
            }

        }
        *looplen = 0;
    }
    else {
        if (!has_loop(sequence, seqlen, *looplen, loop_start, loop_end)) { // check for loop
            *looplen -= 1;
            printf("Checking if there is a loop of lenght %d...\n", *looplen);
            check_loop_iterative(generate_sequence, xs, seqlen, loop, looplen);
           }
        else {  // if there is a loop
            printf("The indexes of the loop's first occurance: %d (first digit), %d (last digit)\n", *loop_start, *loop_end);
            for (i = (*loop_start), k = 0; i < (*loop_end); k++, i++) {
                loop[k] = sequence[i];
            }
        }
    }
    //memory free, It can be more efficient if done above
    free(loop_start);
    free(loop_end);
    free(sequence);
}

int has_loop(int *arr, int n, int looplen, int *ls, int *le) {
    int i, j, k, t;
    for (i = 1; i <= looplen; i++) {
        if (arr[n-i] != arr[n-looplen-i])
            return 0;
    } i--;
    printf("\n\nLoop detected with a lenght of %d.\n",looplen);
    (*ls) = n-looplen-i;
    (*le) = n-i;
    while (i < n- looplen){
        i += looplen;
        t = 1;
        for (k = 0; k < looplen; k++) {
            if (arr[n-looplen+k] != arr[n+k-i])
                t = 0;
        }
        if (t) {
            *ls = n-i;
            *le = n+looplen-i;
        }
    }
    return 1;
}

int pow10i(int n) {
    int i, top = 1;
    for (i = 0; i < n; i++)
        top *= 10;
    return top;
}

int first_digit(int x) {
    int ct = 0, b = x, i;

    while(b > 0) {
        b /= 10;
        ct += 1;
    }
    return x/pow10i(ct-1);
}

void hist_of_firstdigits(void (*f)(), int xs, int seqlen, int *h, int *digit) {
    int i, j, b = 0, *sequence;

    //allocating
    sequence = (int *) malloc(seqlen * sizeof(int));
    f(xs, 0, seqlen, sequence);  // generate sequence

    for (i = 0; i < seqlen; i++) {
        if (first_digit(sequence[i]) == *digit)
            h[*digit-1]++;
    }
    free(sequence);
    (*digit)++;
    if (*digit < 10)
        hist_of_firstdigits(f, xs, seqlen, h, digit);
}
