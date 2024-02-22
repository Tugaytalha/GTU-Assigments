#include <stdio.h>
#include "util.h"


int main() {

    /* A fractional number: 13/7 */
    int num1 = 13, den1 = 7;
    /* A fractional number: 13/7 */
    int num2 = 30, den2 = 11;
    /* An unitilized fractional number */
    int num3, den3;

    printf("First number: ");
    fraction_print(num1, den1);
    printf("\n");

    printf("Second number: ");
    fraction_print(num2, den2);
    printf("\n");

    printf("Addition: ");
    fraction_add(num1, den1, num2, den2, &num3, &den3);
    fraction_print(num3, den3);
    printf("\n");

    printf("Subtraction: ");
    fraction_sub(num1, den1, num2, den2, &num3, &den3);
    fraction_print(num3, den3);
    printf("\n");

    printf("Multiplication: ");
    fraction_mul(num1, den1, num2, den2, &num3, &den3);
    fraction_print(num3, den3);
    printf("\n");

    printf("Division: ");
    fraction_div(num1, den1, num2, den2, &num3, &den3);
    fraction_print(num3, den3);
    printf("\n");



    /*
     TODO: Complete this code to read two fractional numbers and print their 
             multiplication and division results...
    */
	int t = 1;
	//* hello guys 
	while ( t == 1) { // Take frictional numbers until they are valid

		printf("\nPlease enter your first fractional number: "); 
		scanf("%d", &num1);
	 	printf("//");
		scanf("%d", &den1);
		printf("Please enter your second fractional number: ");
		scanf("%d", &num2);
	 	printf("//");
		scanf("%d", &den2);
		if (den1 == 0 || den2 == 0) { //Denominator  mustn't zero
			printf("\nZero division error. Please enter a valid numbers.");
			continue;
		}
		t = 0;
	}

	printf("\n\nYour first fractional number: ");
    fraction_print(num1, den1);
	printf("\n"); 

	printf("Your second fractional number: ");
    fraction_print(num2, den2);
	printf("\n");
	
	printf("\n\nMultiplication your numbers: ");
    fraction_mul(num1, den1, num2, den2, &num3, &den3);
    fraction_print(num3, den3);
    printf("\n");

    printf("Division your numbers: ");
    fraction_div(num1, den1, num2, den2, &num3, &den3);
    fraction_print(num3, den3);
    printf("\n");

	printf("\nBonus(?):\nAddition: ");
    fraction_add(num1, den1, num2, den2, &num3, &den3);
    fraction_print(num3, den3);
    printf("\n");

	printf("Subtraction: ");
    fraction_sub(num1, den1, num2, den2, &num3, &den3);
    fraction_print(num3, den3);
    printf("\n");
    //"TODO: Remove this printf and add the code to read two fractional numbers and .....\n");

    return(0);
}
