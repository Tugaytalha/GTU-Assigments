#include <stdio.h>
#include <string.h>
#define MAX 100


void reverse(char inputString[MAX], char reversed[MAX], int i);

void merge_sort(int arr[],int l, int r);

void merge(int arr[],int q,int p,int r);

void printf_arr(int arr[], int n);


int main () {
	char c,inp[MAX], revers[MAX];
	int array[] = {6,5,12,10,9,1},cas;

	while (1) {
		printf(
			   "\n\nPlease selcet your operation\n"
			   "1. Poindrome check\n"
			   "2. Merge Sort\n"
         "0. Exit\n"
			   "------------------------------\n"
			   );
		printf("\nYOUR CHOICE: ");
    if(scanf("%d",&cas) != 1) {
			printf("ERROR, please enter a valid entry."); // if input isn't valid(char etc.) will print this
			continue;
		}
		while((c = getchar()) != '\n' && c != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time
		switch(cas) { // chose apllier
      case 1:
				printf("Enter a string for polindrome check\n" );
				scanf("%s", inp);

				reverse(inp, revers, 0);
				printf("%s\n",revers );
				if(!(strcmp(inp,revers))) { // if equal
					printf("%s is palindrome\n", inp );
				}
				else
					printf("%s isn't palindrome\n", inp );

				break;
      case 2:

				merge_sort(array, 0, 5);
				printf_arr(array,6);
				break;

			case 0:
				return 0;
			}
		}
	return 0;
}

void printf_arr(int arr[], int n) { // printf("%s\n", ); int array
	int i;
	for(i = 0; i<n; i++) {
		printf("%d ",arr[i] );
	}
	printf("\n" );
}

void reverse(char inputString[MAX], char reversed[MAX], int i){
     /* Input Validation */
		 if (inputString[0]){ 	// check NULL charaxter if null finish
			 reversed[strlen(inputString)-i-1] = inputString[0]; // index i = index len - i - 1
			 reverse(&inputString[1], &reversed[1], i+1);
		 }


 }


void merge_sort(int arr[],int low,int high)
 {
   int mid;
   if(low<high) // while array size == 1
   {
     mid=(low+high)/2;

     merge_sort(arr,low,mid);   // Divide and conqurer array
     merge_sort(arr,mid+1,high);

     merge(arr,low,mid,high); 		// Combine devided arrays
   }

   return ;
 }

void merge(int arr[],int p,int q,int r)
{
   int n1 = q-p+1;
   int n2 = r-q,i,j,k;
	 int L[n1],M[n2];

   for(i=0;i<n1;i++)
     L[i]=arr[p+i];
   for(j=0;j<n2;j++)
     M[j]=arr[q+j+1];

		 i = 0;
		 j = 0;
		 k = p;

		 while(i < n1 && j < n2) {
			 if(L[i] <= M[j]) {
				 arr[k] = L[i];
				 i++;

			 }
			 else {
				 arr[k] = M[j];
				 j++;
			 }
			 k++;
		 }

		 while(i < n1) {
			 arr[k] = L[i];
			 i++;
			 k++;
		 }
		 while(j < n2) {
			 arr[k] = M[j];
			 j++;
			 k++;
		 }
}
