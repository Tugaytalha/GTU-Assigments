#include <stdio.h>
#include <math.h>


int sum(int n1, int n2, int flag);

int multi(int n1, int n2, int flag);

int isprime(int a);

void write_file(int p);

void sort_file();

void print_file();

void copy_file();

void write_Temp_file(int p);

int main() {
	int cas, opt, flag,n1,n2,res1,res2,i,j,t = 1, tempi, tr;
	while (1) { 
		t = 1;
		printf(
			   "\n\nPlease selcet your operation\n"
			   "1. Calculate sum/multiplication between two numbers\n"
			   "2. Calculate prime numbers\n"
			   "3. Show number sequence in file  \n"
			   "4. Sort number sequence in file  \n"
			   "0. Exit.\n"
			   "------------------------------\n"
			   );
		printf("\nYOUR CHOICE: ");
		scanf("%d",&cas);
		switch (cas) {
		case 1:
			
			while (t) {
				printf(
					"\nPlease selcet your operation\n"
					"0. Sum\n"
					"1. Multiplication\n"
					);
				printf("\nYOUR CHOICE: ");
				scanf("%d",&opt);

				printf(
				"0. Work on even numbers\n"
				"1. Work on odd numbers\n"
				);
				printf("\nYOUR CHOICE: ");
				scanf("%d",&flag);
				
				if ((flag != 0 && flag != 1) || (opt != 0 && opt != 1)) { //validity check
					printf("Invalid flag or opt, please enter again\n");
				}
				else t = 0;
			}	

			t = 1;
			while (t) {
				
				printf(
					"\nPlease enter two diffrenet numbers:(negative numbers not allowed)\n"
					"Number 1: "
				);
				
				scanf("%d",&n1);
				printf(
					"Number 2: "
				);
				
				scanf("%d",&n2);

				if ((n1<0) || (n2<0) || (n1 == n2)) { // negative or equal not allowed
					printf("Invalid numbers, please enter again\n");
				}
				else t = 0;
			}
			if (n2<n1) {
				tempi = n1;
				n1 = n2;
				n2 = tempi;
				printf("Numbers locations changed!!\n");
			}

			switch (opt) { /*normally if is much easier but I didn't understand "The operation selection should be determined by using operation flag with switch-case, after that, the integer and the other flag should be used to call the related function." sentence. */
			case 0:
				tr = sum (n1,n2,flag);
				write_file(tr);
				printf("Result is succesfully writed to file\n");
				break;
			case 1: 
				tr = multi(n1,n2,flag);
				write_file(tr);
				printf("Result is succesfully writed to file\n");
				break;
			}
			break;

		case 2:
			t = 1;
			while (t) {
				printf(
					"\nPlease enter an integer greater than 2:\n" );
				scanf("%d",&n1);
				if (n1<2) { // negative not allowed
					printf("Invalid numbers, please enter again\n");
				}
				else t = 0;
			}

			for (i = 2; i<n1; i++) {
				res1 = isprime(i);
				if (res1)
					printf("%d is not a prime number, it is dividible by %d.\n",i,res1);
				else 
					printf("%d is a prime number.\n",i);
			}
			
			break;
		case 3:
			print_file();
			break;
  
		case 4:
			sort_file();
			copy_file();
			print_file();
			break;
		case 0:
            printf("Exiting.........\n\n");	

			return (0);	

		default:
            printf("Error! operator is not correct\n\n");	
		}

	}
    return(0);
}


int sum(int n1, int n2, int flag) {
    int i, f = 0, j,t = 1;
	printf("Result\n");
	if (n1%2 == flag){
		t = 2;
	}
	for (i = n1+t; i<n2; i += 2) {
			f += i;
			if (i != n2-1 && i != n2-2) {
				printf("%d + ",i);
			}
			else printf("%d = ",i);
	}
	printf("%d\n",f);
	return (f);
}  


int multi(int n1, int n2, int flag) {  
	int i, f = 1, j,t = 1;
	printf("Result\n");
	if (n1%2 == flag){
		t = 2;
	}
	for (i = n1+t; i<n2; i += 2) {
			f *= i;
			if (i != n2-1 && i != n2-2) { //until reach to end, print this
				printf(" %d *",i);
			}
			else printf(" %d =",i);
	}
	printf(" %d\n",f);
	return (f);
}  


int isprime(int a) {
    int top=0, top_odd=0, top_even=0, r, i, j;
	for (i = 2; i<=sqrt(a); i++) { // Trying all numbers between 2 and squareroot of number 
		if (a%i == 0) {
			return i; //if it is divisible returning divider number 
		}
	}
	return (0);  // else return 0 (Meaning it is prime)
} 
  


void write_file(int p) { // write number to results.txt
	FILE *fil;
	fil = fopen("results.txt","a+");
	fprintf(fil, "%d ",p);
	fclose(fil);
    return ;
}  


void write_Temp_file(int p) { // Writing number to Temp.txt file 
	FILE *fil;
	fil = fopen("Temp.txt","a+");
	fprintf(fil, "%d ",p);
	fclose(fil);
    return ;
}  

void sort_file() {
	int inm, min1 = 2147483646, min2 = 2147483646, min3 = 2147483646, ming = 0, i4, i5, i6,  calc, t1, t2, t3, i1, i2, i3, i = 0, say = 0, j;
	FILE *fil,*temp;
	temp = fopen("Temp.txt","w");
	fclose(temp);
	fil = fopen("results.txt","r");
	if (fil == NULL) printf("File couldn't open");
	say = 0;
	while(fscanf(fil,"%d",&inm)) { // this loop calculating number of numbers in file
		
        if (fgetc(fil) == EOF) break; //if reach to End of file exit
		say++;
		
		
	}
	calc = say%3;
	say /= 3;
	if (calc) say++;
	fclose(fil);
	for(j = 0; j<say; j++) {
		fil = fopen("results.txt","r");
		min1 = 2147483646; 
		min2 = 2147483646; 
		min3 = 2147483646;
		i = 0;
		while(fscanf(fil,"%d",&inm)) {
			
			if ((inm>ming) || (inm == ming && (i1 != i && i2 != i && i3 != i && i4 != i && i5 != i && i6 != i)) ) {
				
				if (inm<min1) {
	//				printf("%d min1\n",min1);
					min3 = min2;
					t3 = t2;
					min2 = min1;
					t2 = t1;
					min1 = inm;
					t1 = i;
				}
				else if (inm<min2) {
	//				printf("%d min2\n",min2);
					min3 = min2;
					t3 = t2;
					min2 = inm;
					t2 = i;
				}
				else if (inm<min3) {
	//				printf("%d min3\n",min3);
					min3 = inm;
					t3 = i;
				}
			}
			if (fgetc(fil) == EOF) break; //if reach to End of file exit
			i++;
			
		}
		if (j%2 == 0) { //if there is more than 1 of the same number, not to be deleted
			i1 = t1;
			i2 = t2;
			i3 = t3;
		}
		else { //if there is more than 3 of the same number, so that it doesn't print the same to the whole file
			i4 = t1;
			i5 = t2;
			i6 = t3;
		}
		if (min3 != 2147483646)
			ming = min3;

		if (min1 != 2147483646) {// if there are less than 3 numbers left, don't add 
			write_Temp_file(min1);
		}
		if (min2 != 2147483646) {// if there are less than 3 numbers left, don't add 
			write_Temp_file(min2);
		}
		if (min3 != 2147483646) { // if there are less than 3 numbers left, don't add 
			write_Temp_file(min3);
			
		}
		
		fclose(fil);
		
	}
	printf("Sorting is complete.\n");
    return ;
}  

void print_file() {
	FILE *fil;
	int inm;
	printf("Result\n");
	fil = fopen("results.txt","r");
	while(fscanf(fil,"%d",&inm)) {
		
        if (fgetc(fil) == EOF) break; //if reach to End of file exit
		printf("%d ",inm);
		
	}
	fclose(fil);
    return ;
}  

void copy_file() { // This coppies Temp.txt file to results.txt
	FILE *fil, *temp;
	int inm;
	fil = fopen("results.txt","w");
	temp = fopen("Temp.txt","r");
	while(fscanf(temp,"%d",&inm)) { //read from Temp.txt
		
        if (fgetc(temp) == EOF) break; //if reach to End of file exit
		fprintf(fil,"%d ",inm);
	}
	fclose(fil);
	fclose(temp);
	return;
}
