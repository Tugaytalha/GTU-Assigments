#include <stdio.h>
#include "util.h"

int find_divisible(int x, int y, int z) {
    int i, f, j;
	for (i = x; i <= y;i++) { //loop until find first
		if (i%z == 0) return (y-i);
	}
	return (-1);
}  


int find_nth_divisible(int n, int f_I, int z) {  
    if ((n)*z <= f_I) return (f_I - (n)*z); // remaining - diffrent
	return (-999);
}  


int validate_identity_number(char identity_number [ ]) {
    int top=0, top_odd=0, top_even=0, r, i, j, p = 0 ;
    
    for (i = 1; identity_number [i] != 0;i++) /*do nothing*/;
    if (i != 11) return (0); // Is it 11 characters?
    for (i=0;i<11;i++) if (!(identity_number[i]<58 && identity_number[i] > 47)) return (0); // Are all characters digit?

    if (identity_number[0] - 48== 0) return(0); // Isn't firt number 0?

	for (i=0;i<9;i+=2) top_even += identity_number[i] - 48;

	for (i=1;i<9;i+=2) top_odd += identity_number[i] - 48;

	if ((top_even* 7 - top_odd)%10 != identity_number[9] -48) return (0); // (7 times sum of 1, 3, 5, 7th digits)  - (sum of the digits 2, 4, 6, 8) == 10th digit?

	for (i=0;i<10;i++) top += identity_number[i] - 48;

	if (top % 10 != identity_number[10] - 48) return (0); //Is sum of first 10 digit equal to 11th digit? 

	return (1);
} 


int create_customer(char identity_number [ ], int password) {
    FILE *Tfile1;
    Tfile1 = fopen("customeraccount.txt", "w");
    fprintf(Tfile1, "%s,%d", identity_number, password);
    fclose(Tfile1);	
    return (1);
}  


int check_login(char identity_number [ ], int password) {
	char c;
    char identity_number1[50];
    if (validate_identity_number(identity_number) == 0) return (0);
    int pass1, i = 0, tr = 1;
    FILE *Tfile1;
    Tfile1 = fopen("customeraccount.txt", "r");
    while (c != ',') {
    	fscanf(Tfile1, "%c", &c);
    	identity_number1[i] = c;
       	i++; 
	}
	identity_number1[i] = '\0';
	fscanf(Tfile1, "%d", &pass1);
	fclose(Tfile1);	
	for (i=0;i<11;i++) if ((identity_number1[i] != identity_number[i]) || password != pass1) return (0); 
    
    return (1);
}  

int withdrawable_amount(float cash_amount) {
	float withdrawable;
	int sa;
	sa = cash_amount/10;
	sa *= 10;
	withdrawable = cash_amount-(cash_amount-sa);
    return (withdrawable);
}  

	




