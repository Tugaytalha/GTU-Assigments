#include <stdio.h>
#include <string.h>
#define size_of_array 20  //define size of the array
#define MAX_LEN 100

/*
* FUNTION: sorting
* -------------------
* It does: sorting array of string as reverse lexographic and concat the sorted array
*
* size: number of string in array
* MAX_LEN: maximum word lenght
* array: array of strings
* returns: no returns, it's manuplating array (array of strings) and cated array
*/
void sorting(char array[][MAX_LEN], int size,   char cated[300]);

/*
* FUNTION: display
* -------------------
* It does: display array
*
* size: number of string in array
* MAX_LEN: maximum word lenght
* array: array of strings
* returns: no returns
*/
void display(char array[][MAX_LEN], int size);

/*
* FUNTION: tag_parser
* -------------------
* It does: parsing a string acording to it's lenght
*
* size: number of string in array
* MAX_LEN: maximum word lenght
* array:  string
* returns: no returns, it's manuplating arr (string) and temp file
*/
void tag_parser(char temp[MAX_LEN], char arr[MAX_LEN]);

int main()
{
  //create an array of strings
  char array[size_of_array][MAX_LEN],arr[MAX_LEN], temp[MAX_LEN];
  int t = 1,cas = -1,ff = 0;
	char c;
  char cated[300];
	while (1) {
		printf(
			   "\n\nPlease selcet your operation\n"
			   "1. reverse sorting\n"
			   "2. Tag Parsaer\n"
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
        printf("How many string? \n");
        scanf("%d", &ff);

        printf("Enter %d Strings: \n", ff);
        for(int i=0; i<ff; i++){
          scanf("%s", array[i]);
        }

        //display the original array
        printf("Original array: \n");
        display(array,ff);

        sorting(array,ff, cated);

        //display the sorted array
        printf("Sorted Array: \n");
        display(array,ff);

        printf("Result is: \n%s\n", cated);
        break;
      case 2:
        printf("Enter a Strings: ");
        scanf("%s", arr);

        tag_parser(temp, arr);
        printf("\n%s\n", arr);

        break;

      case 0:
        return 0;

      }





  }
  return 0;
}


//function to display array
void display(char array[][MAX_LEN], int size) {
  for(int i=0; i<size; i++){
    printf("%d: %s ", i, array[i]);
    printf("\n");
  }

}



void sorting(char array[][MAX_LEN], int size, char cated[300]) {
  char temp[MAX_LEN];
  int i,j,k = 0;
  //Sort array using the Buuble Sort algorithm
  for(int i=0; i<size; i++){
    for(int j=0; j<size-1-i; j++){
      if(strcmp(array[j], array[j+1]) < 0){
        //swap array[j] and array[j+1]
        strcpy(temp, array[j]);
        strcpy(array[j], array[j+1]);
        strcpy(array[j+1], temp);
      }
    }
  }
  for (i=0; i<size; i++) {
    for (j=0; j<strlen(array[i]); j++) {
      cated[k] = array[i][j];
      k++;
    }
  }
}


void tag_parser(char temp[MAX_LEN], char arr[MAX_LEN]) {
  int i, j;
  strcpy(temp, arr);
  int a = strlen(arr);
  if (a<5) {
    arr[0] = '<';
    arr[1] = '<';
    for (i = 1; i < a-1; i++) {
      arr[i+1] = temp[i];
    }
    arr[a] = '>';
    arr[a+1] = '>';
    arr[a+2] = '\0';
  }
  else if (a<11) {
    arr[0] = '*';
    for (i = 1; i < a-1; i++) {
      arr[i] = temp[i];
    }
    arr[a-1] = '*';
    arr[a] = '\0';
  }
  else {
    arr[0] = '/';
    arr[1] = '+';
    for (i = 1; i < a-1; i++) {
      arr[i+1] = temp[i];
    }
    arr[a] = '+';
    arr[a+1] = '/';
    arr[a+2] = '\0';
  }

}
