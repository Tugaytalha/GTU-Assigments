#include <stdio.h>
#include "util.h"

void fraction_print(int numerator, int denominator) {
    printf("%d//%d", numerator, denominator);
}  /* end fraction_print */

void fraction_add(int n1, int d1, int n2, int d2, int * n3, int * d3) {
    *n3 = n1*d2 + n2*d1;
    *d3 = d1 * d2;
    fraction_simplify(n3, d3);
} /* end fraction_add */

void fraction_sub(int n1, int d1, int n2, int d2, int * n3, int * d3) {
	*n3 = n1*d2 - n2*d1;
	*d3 = d1 * d2;
    fraction_simplify(n3, d3);
} /* end fraction_sub */

void fraction_mul(int n1, int d1, int n2, int d2, int * n3, int * d3) {
	*n3 = n1*n2;
    *d3 = d1*d2;
    fraction_simplify(n3, d3);
} /* end fraction_mul */

void fraction_div(int n1, int d1, int n2, int d2, int * n3, int * d3) {
	if (n2 == 0) { // If second number is zero, division can't be done (x/0).
		printf("Zero division error. Please enter a valid number.");
		return;
	}
	n1 = n1*d2*n2;
	d1 = d1*d2*n2;
	*n3 = n1 / n2;
	*d3 = d1 / d2;
    fraction_simplify(n3, d3);
} /* end fraction_div */

/* Simplify the given fraction such that they are divided by their GCD */
void fraction_simplify(int * n, int * d) {
	int i,t = 0;
	int pos = 1; //keeping number's sign
	int a = *n;
	int b = *d;
	if (*n<0) { // If negative convert positive and keep sign
		pos *= -1;
		*n *= -1;
	}
	if (*d<0) { // If negative convert positive and keep sign
		pos *= -1;
		*d *= -1;
	}
	if (n>d) {
		a = *d;
		b = *n;
		t = 1;
	}
	for (i = 2; i<=a; i++) { //simplify
		if (a%i == 0 && b%i == 0) {
			a /= i;
			b /= i;
			i = 1;
		} // end if 

	} // end for
	
	*d = b;
	*n = a;
	if (t == 1) {
		*d = a;
		*n = b;

	}
	*n *= pos; // Sign to numerator from keeper




	
		
	
} /* end fraction_div */



