#ifndef _UTIL_H_
#define _UTIL_H_

#define CLOSE_ENOUGH 0.001

/* Example decision tree - see the HW2 description */
int dt0(int t, double p, double h, char s, int w);

char dt1a(double PL, double PW, double SL, double SW) ;
char dt1b(double PL, double PW, double SL, double SW);

double dt2a(double x1, double x2, double x3, int x4, int x5);
double dt2b(double x1, double x2, double x3, int x4, int x5);

/* Write the prototype of the functions implementing the decision trees for the third problem */
double dt3a(double alc, double exercise, int smoker, char think, char gender);
double dt3b(double alc, double exercise, int smoker, char think, char gender);

#endif /* _UTIL_H_ */