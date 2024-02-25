#include <stdio.h>

typedef struct number {
  int a;
  char s[50];
} num;

typedef struct combination {
  num n;
  num r;
  int res;
} com;


int base_converter(int decimal_number, int base) ;

int str_len(char *str) ;

int powi(int n);

int factorial(int a);

int is_number (char *number_as_str, int lenght_of_str);

int parse_to_int(char *number_as_str, int lenght_of_str);

int logi(int n, int ct);

int combi(int n, int r);

void reverse(char *s,int st, int end);

int main()
{
    com sa;
    int b, a, t = 0;

    sa.n.s[0] = 's';
    sa.r.s[0] = 's';
    printf("%d\n",str_len("123434"));


    //take input as a string until they are valid int
    while(!is_number(sa.n.s,str_len(sa.n.s)) && !is_number(sa.r.s,str_len(sa.r.s))) {
      if(t) printf("\nInputs are not valid. Please try agan..\n\n");
      t = 1;
      printf("n:" );
      scanf("%s", sa.n.s);

      printf("r:" );
      scanf("%s", sa.r.s);
    }

    // reverse the string (beacsue my parse_to_int function works reverse)
    reverse(sa.n.s,0,str_len(sa.n.s)-1);
    reverse(sa.r.s,0,str_len(sa.r.s)-1);
    sa.n.a = parse_to_int(sa.n.s,str_len(sa.n.s));
    sa.r.a = parse_to_int(sa.r.s,str_len(sa.r.s));
    //printf("%d %d\n",sa.n.a,sa.r.a );
    sa.res = combi(sa.n.a, sa.r.a);
    printf("COmbination (%d,%d) = %d\n\n",sa.n.a,sa.r.a, sa.res );



    printf("dECiMAL:" );
    scanf("%d", &a);
    printf("base:" );
    scanf("%d", &b);
    printf("\n\n%d in base %d is: %d\n", a, b, base_converter(a,b));
    getchar();
    getchar();

    return 0;
}


int factorial(int a) {  // take facktorial
  if (a == 1) return 1;
  return a*factorial(a-1);
}

int is_number (char *number_as_str, int lenght_of_str) {  // is valid number controller
  if (lenght_of_str == 0) return 1;
  if (!(number_as_str[--lenght_of_str]<='9' && number_as_str[lenght_of_str]>='0')) return 0;  // if it is not digit terminate recursive
  return is_number(number_as_str, --lenght_of_str);
}


int parse_to_int(char *number_as_str, int lenght_of_str) {  // str to int
  int res = 0;
  if (lenght_of_str <= 0) return 0;  // if str ended teminate the recursive
  res = parse_to_int(number_as_str, --lenght_of_str);  // recursive
  return (number_as_str[lenght_of_str] - '0')*powi(lenght_of_str) + res;  // return last result and last digit
}

void reverse(char *s,int st, int end) {   // reverse the string
  char t;
  if (st >= end) return ;
  t = s[st];  // temp value
  s[st] = s[end];
  s[end] = t;
  reverse(s,st+1,end-1);
}

int str_len(char *str) {  // find string lenght
  int i = 0;
  if (str[0] == 0) return 0;
  i += str_len(str+1);
  i++;
  return i;
}

int combi(int n, int r) {  //take combination(n,r)
  return factorial(n)/(factorial(r)*factorial(n-r));
}

int powi(int n) {  // take power base 10
  int i, res = 1;
  for (i = 0; i<n; i++) {
    res *= 10;
  }
  return res;
}

int logi(int n, int ct) {  // take log base 10
  if (n == 0) return ct;
  ct = logi(n/10,++ct);
  return ct;
}

int base_converter(int decimal_number, int base) {  // base converter of int
  int res;
  if (decimal_number == 0) return 0;  // we reached the result
  res = base_converter(decimal_number/base, base);
  return res*10 + (decimal_number%base);
}