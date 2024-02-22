#include <stdio.h>
#include <stdlib.h>
#include "util.h"


int main() {
    double PL, PW, SL, SW;
    double x1, x2, x3;
    int x4, x5, cas, res0;
    double alc, exercise, res3, res4,p,h;
    int smoker,t,w;
    char think, gender, res1, res2,s;
 
    while (1) { 
        printf(
			   "\n\nPlease selcet your operation\n"
               "0. Selection 0. \n"
			   "1. Selection 1. \n"
			   "2. Selection 2. \n"
			   "3. Selection 3. \n"
			   "9. Exit."
			   );
		printf("\nYOUR CHOICE: ");  /* Ask for the problem selection (1,2,3) .....  */
		scanf("%d",&cas);
        switch (cas) {
        case 0:
            printf("\nEnter temperature, day_of_the_week(int)     humidity, pressure(double)      s(char, if isn't sunny you have to enter 0");
            scanf("%d %d %lf %lf %c", &t, &w, &h, &p, &s); /* we have to take char s beacuse funtion protoype is this :( */
            res0 = dt0( t,  p,  h,  s-48,  w);
            if (res0)
            printf("Your result : Turn on\n");
            else printf("Your result : Turn off\n");

            break;

		case 1:
            printf("\nEnter PL, PW, SL, SW: ");
            scanf("%lf %lf %lf %lf", &PL, &PW, &SL, &SW);   /* Get the input from the user for the first problem, i.e., to test dt1a and dt1b */
            res1 = dt1a(PL, PW, SL, SW) ;
            res2 = dt1b(PL, PW, SL, SW) ;
            if (res1 == res2) {       /* Compare performances and print results */
                if (res1 == 'I') printf("Virginica");
                else if (res1 == 'E') printf("Versicolor");
                else printf("Setosa");
            }
            else {
                if (res1 == 'I') printf("\nResult 1: Virginica");
                else if (res1 == 'E') printf("\nResult 1: Versicolor");
                else printf("\nResult 1: Setosa");

                if (res2 == 'I') printf("\nResult 2: Virginica");
                else if (res2 == 'E') printf("\nResult 2: Versicolor");
                else printf("\nResult 2: Setosa");
            }    

			break;

		case 2:
            printf("\nEnter x1, x2, x3(real numbers)     x4, x5(binary, 0 = false, other ints = true): ");
            scanf("%lf %lf %lf %d %d", &x1, &x2, &x3, &x4, &x5);      /* Get the input from the user for the second problem, i.e., to test dt2a and dt2b */
            res3 = dt2a(x1, x2, x3, x4, x5 ) ;
            res4 = dt2b(x1, x2, x3, x4, x5 ) ;

            if (res3 == res4) {      /* Compare performances and print results */
                printf("\nYour result is %.2lf",res3);
            }
            else if (res3 + CLOSE_ENOUGH == res4 || res3 - CLOSE_ENOUGH == res4 || res3 + CLOSE_ENOUGH == res4 - CLOSE_ENOUGH || res3 - CLOSE_ENOUGH == res4 + CLOSE_ENOUGH) {
                printf("\nYour average result is %.2lf",(res3 + res4)/2) ;
            }  
            else {
                printf("\nResult 1:  %.2lf",res3);
                printf("\nResult 2:  %.2lf",res4);
                
            }  

			break;
		case 3:
            printf("\n\t\t\t-----------Welcome To Lifetme Tree-----------\n\n");
            getchar();
            printf("\nEnter gender \nE for men \nK for women\n F for ferm\nH for herm\nM for merm\nCHOSE ONE: ");     /* Get the input from the user for the third problem, i.e., to test dt3a and dt3b */
            scanf("%c", &gender);
		
	    	getchar();
            printf("\nEnter thinking style \nR = realist \nP = pessimist  \nN = normal\nO = optimistic\nV = pragmatist\nCHOSE ONE: ");
            scanf("%c", &think);
            
            printf("\nAre you smoker?(binary, 0 = No, other ints = Yes) ");
            scanf("%d", &smoker);
            

            printf("\nRate your exercises : ");
            scanf("%lf", &exercise);

            printf("\nHow many times do you consume alcohol in a year? ");
            scanf("%lf", &alc);

			res3 = dt3a( alc, exercise, smoker, think, gender);
            res4 = dt3b( alc, exercise, smoker, think, gender);
            
            if (res3 == res4) {   /* Compare performances and print results */
                printf("\nYour Lifetime is %.2lf",res3);
            }
            else if (res3 + CLOSE_ENOUGH == res4 || res3 - CLOSE_ENOUGH == res4 || res3 + CLOSE_ENOUGH == res4 - CLOSE_ENOUGH || res3 - CLOSE_ENOUGH == res4 + CLOSE_ENOUGH) {
                printf("\nYour Lifetime average is %.2lf",(res3 + res4)/2) ;
            }  
            else {
                printf("\nLifetime according to the Quick One:  %.2lf",res3);
                printf("\nLifetime according to the Complex One:  %.2lf",res4);
            }  


			break;

		case 9:
			return (0);			
		}

	}
    

  



    return 0;
}
