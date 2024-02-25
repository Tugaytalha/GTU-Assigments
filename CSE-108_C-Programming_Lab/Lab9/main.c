#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int *findMaximumDistance(int distanceOfCities[], int start, int end);

int recursiveMinStrDiff(char str1[], char str2[], int m, int n) ;

int main() {
  int i, *b, a[5] = {120,200,105,300,295};
  char str1[999], str2[999];
  for (i = 0; i<5; i++) {
    printf("Enter first int of array:" );
    scanf("%d",&a[i] );
  }
  b = findMaximumDistance(a, 0, 4);
  printf("%d %d\n", b[0], b[1]);

  printf("Enter str1:" );
  scanf("%s", str1);

  printf("Enter str1:" );
  scanf("%s", str2);
  printf("%d\n",recursiveMinStrDiff(str1,str2,strlen(str1),strlen(str2)) );
  return 0;
}


int *findMaximumDistance(int distanceOfCities[], int start, int end) {
  int *mai, *b, mid, *a, i, j;
  mai = (int *) malloc (sizeof(int)*2);  // allocate memeory
  a = (int *) malloc (sizeof(int)*2);// allocate memeory
  b = (int *) malloc (sizeof(int)*2);// allocate memeory
  mai[0] = 0;  // reset values
  mai[1] = 99999; // reset values
  if (start == end) {  //we have 1 elemnt array
    mai[0] = distanceOfCities[start];
    mai[1] = distanceOfCities[start];
    return mai;
  }
  else if (start +1 == end) { //we have 2 elemnt array
    if (distanceOfCities[start] >  distanceOfCities[end]) {  // big elemnt to first elemnt
      mai[0] = distanceOfCities[start];
      mai[1] = distanceOfCities[end];
    }
    else {
      mai[0] = distanceOfCities[end];
      mai[1] = distanceOfCities[start];
    }
    return mai;
  }
  mid = (start + end)/2;
  a = findMaximumDistance(distanceOfCities, start, mid);  // recursive
  b = findMaximumDistance(distanceOfCities, mid+1, end); // recursive

  //comprassions
  if (a[0]>mai[0]) {
    mai[0] = a[0];
  }
  if (b[0]>mai[0]) {
    mai[0] = b[0];
  }
  if (a[1]<mai[1]) {
    mai[1] = a[1];
  }
  if (b[1]<mai[1]) {
    mai[1] = b[1];
  }

  //return maximum distance array
  return mai;
}


int recursiveMinStrDiff(char str1[], char str2[], int m, int n) {
  int i ,j, ct = 0;
  if (n == 0)
    return m;
  else if (m == 0)
    return n;
  if (m>n) {
    ct += 1;  // remove a char
    ct += recursiveMinStrDiff(str1, str2, m-1, n);  //recursive
  }
  else if(n>m) {
    ct += 1;  // add a char
    ct += recursiveMinStrDiff(str1, str2, m, n-1); //recursive
  }
  else {
    if (str1[m-1] == str2[n-1]) {  // same character
      ct += recursiveMinStrDiff(str1,str2,m-1,n-1); //recursive
    }
    else {
      ct += 1; // replace them
      ct += recursiveMinStrDiff(str1, str2,m-1,n-1); //recursive
    }
  }
  return ct;
}
