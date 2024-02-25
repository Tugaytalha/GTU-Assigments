#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int *array;
  int currentsize;
} dynamic_array;

dynamic_array read_from_file(char *file_name, dynamic_array arr);  // read numbers from file to array

dynamic_array removeData(dynamic_array arr, int number); // remove data from array

void print_array(dynamic_array arr);  //Print the array

int *resize_array(int *array, int currentsize); // increase array size

int *reduce_array(int *array, int currentsize);  //reduce array size

void goto_last(int *array, int index, int size);  // take from last index to index that will be deleted

int main() {
  int cas;
  dynamic_array dyna;
  dyna.currentsize = 0;
  dyna = read_from_file("abc.txt", dyna);
  printf("Data in source file\n**********************\n" );
  print_array(dyna);

  //delete part
  printf("enter number you want to be deleted: \n" );
  scanf("%d", &cas);
  dyna = removeData(dyna, cas);
  printf("After delete\n**********************\n" );
  print_array(dyna);


  return 0;
}



dynamic_array read_from_file(char *file_name, dynamic_array arr) {
  FILE *ff;
  int i, a;
  ff = fopen(file_name, "r");
  if (ff == NULL) {  // if there is no file
    printf("404 Not found\n");
    return arr;
  }
  while(fscanf(ff, "%d", &a) == 1) { // read int from while until end
    arr.array = resize_array(arr.array, arr.currentsize);  // increase array size
    arr.currentsize++;
    arr.array[arr.currentsize-1] = a;
  }
  fclose(ff);
  return arr;
}


void print_array(dynamic_array arr) {
  int i;
  // until size of the array print it
  for(i = 0; i<arr.currentsize; i++) {
    printf("%d\n", arr.array[i]);
  }
}


int *resize_array(int *array, int currentsize) {
  int *arri, i;
  arri = (int *) calloc(currentsize+1, sizeof(int));  // increase array size
  for(i = 0; i < currentsize; i++) {
    arri[i] = array[i];  // copy data to new array
  }
  free(array);
  return arri;
}

void goto_last(int *array, int index, int size) {
  int temp;
  array[index] = array[size-1];  // change last element and delete elemnet
}

int *reduce_array(int *array, int currentsize) {
  int *arri, i;
  currentsize--;
 //reduce array size
  arri = (int *) calloc(currentsize, sizeof(int));
  for(i = 0; i < currentsize; i++) {
    arri[i] = array[i]; // copy Data
  }
  free(array);
  return arri;
}


dynamic_array removeData(dynamic_array arr, int number) {  // remove data
  int i;
  for (i = 0; i < arr.currentsize; i++) {
    if(arr.array[i] == number) {
      goto_last(arr.array, i, arr.currentsize); // delete elemnt to last index
      arr.array = reduce_array(arr.array, arr.currentsize);  // reduze size f array
      arr.currentsize--;
      return arr;
    }
  }
}
