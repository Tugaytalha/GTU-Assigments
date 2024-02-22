#include <stdio.h>
#include "util.h"





int main() {
	char identity_number[50] = "60795076932";
	
	int t = 1, n, x, y, z, first_int, Nth_int, cas, cnt = 1, pass, draweble;
	float cashes;
	printf("\n\t\t\t-----------Welcome To ATM-----------\n");
	while (1) { 
		printf(
			   "\nPlease selcet your operation\n"
			   "1. Part 1. (divisible finder)\n"
			   "2. Part 2. (Register to bank)\n"
			   "3. Part 3. (Log in to ATM)  \n"
			   "0. Exit."
			   );
		printf("\nYOUR CHOICE: ");
		scanf("%d",&cas);
		switch (cas) {
		case 1:
			printf("\nYOUR CHOICE: Divisible finder");
			while ( t == 1) {
				printf("\nEnter the first integer: ");
				scanf("%d", &x);
				printf("\nEnter the second integer: ");
				scanf("%d", &y);
				printf("\nEnter the divisor: ");
				scanf("%d", &z);
			if (z == 0) {
				printf("\nOpps!! Please enter diffrent divisor");
				continue;
			}
			t = 0;
			
			}
			first_int = find_divisible( x,  y,  z);
			if (first_int == -1) {
				printf("\nThere is not any integer between %d and %d can be divided by %d\n", x, y, z);
				return (0);	
			}    
			printf("\nThe first integer between %d and %d divided by %d is %d", x, y, z, y - first_int);
				
			printf("\nEnter the number how many next:  ");
			scanf("%d", &n);



			Nth_int = find_nth_divisible( n,  first_int,  z);
			if (Nth_int == -999) {
				printf("\nThere is not any integer between %d and %d can be divided by %d\n", x, y, z);
				return (0);	
			}  
			if (n = 0) printf("\nThe %dst integer between %d and %d divided by %d is %d\n", n+1, x, y, z, y - Nth_int);
			else if (n = 1) printf("\nThe %dnd integer between %d and %d divided by %d is %d\n", n+1, x, y, z, y - Nth_int);
			else if (n = 2) printf("\nThe %drd integer between %d and %d divided by %d is %d\n", n+1, x, y, z, y - Nth_int);
			else printf("\nThe %dth integer between %d and %d divided by %d is %d\n", n+1, x, y, z, y - Nth_int);
			break;

		case 2:
			printf("\nYOUR CHOICE: Register\n");
			while (cnt == 1) {

				printf("\nEnter the identity number: ");
				scanf("%s",identity_number);

				if (validate_identity_number(identity_number) == 0) printf("\nInvalid identity number, please enter a valid identity number\n");
					
				else {
					while (1) {
						printf("\nEnter the Password: ");
						scanf("%d",&pass);
						if (pass<10000 && pass>999) break;
						printf("\nPassword must be 4 digit and can't start with 0, please enter a valid password\n");
					}
					if (create_customer( identity_number,  pass)) printf("____________________________________\nREGISTIRING SUCCESSFULLY COMPLETED\n‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
					cnt = 0;
				}				
			}
			break;
		case 3:
			printf("\nEnter your identity number: ");
			scanf("%s",identity_number);
			printf("\nEnter the Password: ");
			scanf("%d",&pass);
			if (check_login( identity_number ,  pass) == 0) printf("\nInvalid password or identity_number");
			else {
				printf("Login Successful\n");
				printf("\n\t\t\t-----------Welcome To Your Account-----------\n");
				printf("\nEnter your identity number: %s",identity_number);
				printf("\nEnter your withdraw amount: ");
				scanf("%f",&cashes);
				printf("Withdrawable amount is: %d \n", withdrawable_amount(cashes));

			}
			break;

		case 0:
			return (0);			
		}

	}
    return(0);
}
