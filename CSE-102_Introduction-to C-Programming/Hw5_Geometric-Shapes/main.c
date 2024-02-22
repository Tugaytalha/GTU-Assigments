#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define PI 3.14

enum shapes {EXlT,Triangle,Quadrilateral,Circle,Pyramid,Cylinder};
enum calcs {Area=1,Perimeter,Volume};


int calc_triangle(int opt);

int calc_quadrilateral(int opt);

int calc_circle(int opt);

int calc_pyramid(int opt);

int calc_cylinder(int opt);

int calculate(int (*f1)(),int (*f2)());

int select_shape();

int select_calc();

int main() {
	printf("Welcome to the geometric calculator!");
	int cas;

	calculate(select_shape,select_calc);
    return(0);
}


int select_shape() {
	int t = 1,cas = -1,ff = 1;
	char c;
	while (t) { 
		printf(
			   "\n\nPlease selcet your shape\n"
			   "1. Triangle\n"
			   "2. Quadrilateral\n"
			   "3. Circle  \n"
			   "4. Pyramid  \n"
			   "5. Cylinder\n"
			   "0. Exit.\n"
			   "------------------------------\n"
			   );
		printf("\nYOUR CHOICE: ");
	if(scanf("%d",&cas) != 1) {
			printf("ERROR, please enter a valid entry."); // if input isn't valid(char etc.) will print this
			ff = 0;
		}		
		while((c = getchar()) != '\n' && c != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
		if ((cas>5 || cas<0) && ff) printf("Wrong shape code, please enter again.");
		else if ((cas == EXlT) && ff) {
			printf("Exiting.........\n\n");	
			exit(0);
		}
		else if (ff) t = 0;
		else ff = 1;
	}
    return(cas);
}

int select_calc() {
	int t = 1,cas = -1,ff = 1;
	char c;
	while (t) { 
		printf(
			   "\n\nPlease selcet your calculator\n"
			   "1. Area\n"
			   "2. Perimeter\n"
			   "3. Volume  \n"
			   "0. Exit.\n"
			   "------------------------------\n"
			   );
		printf("\nYOUR CHOICE: ");
		if(scanf("%d",&cas) != 1) {
			printf("ERROR, please enter a valid entry."); // if input isn't valid(char etc.) will print this
			ff = 0;
		}
		while((c = getchar()) != '\n' && c != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
		if ((cas>3 || cas<0) && ff) printf("Wrong calculator code, please enter again.");
		else if ((cas == EXlT) && ff) {
			printf("Exiting.........\n\n");	
			exit(0);
		}
		else if (ff) t = 0;
		else ff = 1;
	}
    return(cas);
}

int calc_triangle(int opt) {
	int t = 1,ff = 1;
	float a=0,b=0,c=0;
	char d;
	double s=0,A = 0,peri = 0;
	switch (opt) {
		case Area:
			while (t) {
				printf("Please enter three sides of Triangle.\n");
				if(scanf("%f %f %f",&a,&b,&c) != 3)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				s = ((double)(a+b+c))/2.0;
				if ((a<=0 || b<=0 || c<=0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if ( (s<=(double)a || s<=(double)b || s<=(double)c) && ff) {
					printf("ERROR, please enter a valid entry.\n");
				}
				else if (ff) t = 0;
				else ff = 1;
			}
			A = sqrt(s * (s - a) * (s - b) * (s - c));
			printf("Area of TRIANGLE : %.2lf\n",A);
			return A;
			
			break;

		case Perimeter:
			while (t) {
				printf("Please enter three sides of Triangle.\n");
				if(scanf("%f %f %f",&a,&b,&c) != 3)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				s = ((double)(a+b+c))/2.0;
				if ((a<=0 || b<=0 || c<=0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if ( (s<=(double)a || s<=(double)b || s<=(double)c) && ff) {
					printf("ERROR, please enter a valid entry.\n");
				}
				else if (ff) t = 0;
				else ff = 1;
			}
			peri = 2*s;
			printf("Perimater of TRIANGLE : %.2lf\n",peri);
			return peri;
			break;

		case Volume:
			printf("Error! You cannot calculate the volume of triangle. Please try again.\n");	
			break;
  
		case EXlT:
            printf("Exiting.........\n\n");	

			exit(0);

		default:
            printf("Error! operator is not correct\n\n");	
		}
}

int calc_quadrilateral(int opt) {
	int t = 1,ff = 1;
	float a=0,b=0,c=0,e=0;
	char d;
	double s=0,A = 0,peri = 0;
	switch (opt) {
		case Area:
			while (t) {
				printf("Please enter four sides of Quadrilateral.\n");
				if(scanf("%f %f %f %f",&a,&b,&c,&e) != 4)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				s = ((double)(a+b+c+e))/2.0;
				if ((a<=0 || b<=0 || c<=0 || e<= 0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if ( (s<=(double)a || s<=(double)b || s<=(double)c) && ff) {
					printf("ERROR, please enter a valid entry.\n");
				}
				else if (ff) t = 0;
				else ff = 1;
			}
			A = sqrt((s - e) * (s - a) * (s - b) * (s - c));
			printf("Area of QUADRILATERAL : %.2lf\n",A);
			return A;
			
			break;

		case Perimeter:
			while (t) {
				printf("Please enter four sides of Quadrilateral.\n");
				if(scanf("%f %f %f %f",&a,&b,&c,&e) != 4)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				s = ((double)(a+b+c+e))/2.0;
				if ((a<=0 || b<=0 || c<=0 || e<= 0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if ( (s<=(double)a || s<=(double)b || s<=(double)c) && ff) {
					printf("ERROR, please enter a valid entry.\n");
				}
				else if (ff) t = 0;
				else ff = 1;
			}
			peri = 2.0*s;
			printf("Perimater of QUADRILATERAL : %.2lf\n",peri);
			return peri;
			break;

		case Volume:
			printf("Error! You cannot calculate the volume of Quadrilateral. Please try again.\n");	
			break;
  
		case EXlT:
            printf("Exiting.........\n\n");	

			exit(0);

		default:
            printf("Error! operator is not correct\n\n");	
		}
}

int calc_circle(int opt) {
	int t = 1,ff = 1;
	float a=0,b=0,c=0,e=0;
	char d;
	double s=0,A = 0,peri = 0;
	switch (opt) {
		case Area:
			while (t) {
				printf("Please enter radius of Circle.\n");
				if(scanf("%f",&a) != 1)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				if ((a<=0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if (ff) t = 0;
				else ff = 1;
			}
			A = PI*(pow(a,2));
			printf("Area of CIRCLE : %.2lf\n",A);
			return A;
			
			break;

		case Perimeter:
			while (t) {
				printf("Please enter radius of Circle.\n");
				if(scanf("%f",&a) != 1)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				if ((a<=0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if (ff) t = 0;
				else ff = 1;
			}
			peri = 2*PI*a;
			printf("Perimater of CIRCLE : %.2lf\n",peri);
			return peri;
			break;

		case Volume:
			printf("Error! You cannot calculate the volume of Circle. Please try again.\n");	
			break;
  
		case EXlT:
            printf("Exiting.........\n\n");	

			exit(0);

		default:
            printf("Error! operator is not correct\n\n");	
		}
}

int calc_pyramid(int opt){
	int t = 1,ff = 1;
	float a=0,b=0,c=0;
	char d;
	double A,peri = 0,B = 0,L = 0;
	switch (opt) {
		case Area:
			while (t) {
				printf("Please enter  base side and slant height of pyramid.\n");
				if(scanf("%f %f",&a,&b) != 2)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				if ((a<=0 || b<=0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if (ff) t = 0;
				else ff = 1;
			}
			B = a*a;
			L = 2*a*b;
			A = B + L;
			printf("Base Surface Area of a PYRAMID : %.2lf\n",B);
			printf("Lateral SurfaceArea of a PYRAMID : %.2lf\n",L);
			printf("Surface Area of a PYRAMID : %.2lf\n",A);
			return A;
			
			break;

		case Perimeter:
			while (t) {
				printf("Please enter base side and height of pyramid.\n");
				if(scanf("%f %f",&a,&b) != 2)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				if ((a<=0 || b<=0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if (ff) t = 0;
				else ff = 1;
			}
			peri = 4*a;
			printf("base surface Perimater of PYRAMID : %.2lf\n",peri);
			return peri;
			break;

		case Volume:
			while (t) {
				printf("Please enter base side and height of pyramid.\n");
				if(scanf("%f %f",&a,&b) != 2)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				if ((a<=0 || b<=0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if (ff) t = 0;
				else ff = 1;
			}
			A = a*a*b/3.0;
			printf("Volume of PYRAMID : %.2lf\n",A);
			return A;
						break;
  
		case EXlT:
            printf("Exiting.........\n\n");	

			exit(0);

		default:
            printf("Error! operator is not correct\n\n");	
		}
}

int calc_cylinder(int opt){
	int t = 1,ff = 1;
	float a=0,b=0,c=0;
	char d;
	double A,peri = 0,B = 0,L = 0;
	switch (opt) {
		case Area:
			while (t) {
				printf("Please enter  radius and slant height of Cylinder.\n");
				if(scanf("%f %f",&a,&b) != 2)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				if ((a<=0 || b<=0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if (ff) t = 0;
				else ff = 1;
			}
			B = PI*a*a;
			L = 2*PI*a*b;
			A = 2*B+L;
			printf("Base Surface Area of a PYRAMID : %.2lf\n",B);
			printf("Lateral SurfaceArea of a PYRAMID : %.2lf\n",L);
			printf("Surface Area of a PYRAMID : %.2lf\n",A);
			return A;
			
			break;

		case Perimeter:
			while (t) {
				printf("Please enter radius and height of Cylinder.\n");
				if(scanf("%f %f",&a,&b) != 2)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				if ((a<=0 || b<=0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if (ff) t = 0;
				else ff = 1;
			}
			peri = 2*PI*a;
			printf("base surface Perimater of CYLINDER : %.2lf\n",peri);
			return peri;
			break;

		case Volume:
			while (t) {
				printf("Please enter radius and height of Cylinder.\n");
				if(scanf("%f %f",&a,&b) != 2)  {
					printf("ERROR, please enter a valid entry.\n"); // if input isn't valid(char etc.) will print this
					ff = 0;
				}
				while((d = getchar()) != '\n' && d != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
				if ((a<=0 || b<=0) && ff) printf("ERROR, please enter a valid entry.\n");				
				else if (ff) t = 0;
				else ff = 1;
			}
			A = a*a*b*PI;
			printf("Volume of CYLINDER : %.2lf\n",A);
			return A;
						break;
  
		case EXlT:
            printf("Exiting.........\n\n");	

			exit(0);

		default:
            printf("Error! operator is not correct\n\n");	
		}
}



int calculate(int (*f1)(),int (*f2)()) {
	int cas, opt,i,j,t = 1, tr;
	double res1;
	while (1) { 
		cas = f1();
		opt = f2();
		switch (cas) {
		case Triangle:
			res1 = calc_triangle(opt);
			
			break;

		case Quadrilateral:
			res1 = calc_quadrilateral(opt);
			
			break;
		case Circle:
			res1 = calc_circle(opt);

			break;
  
		case Pyramid:
			res1 = calc_pyramid(opt);

			break;
		case Cylinder:
			res1 = calc_cylinder(opt);

			break;
		case EXlT:
            printf("Exiting.........\n\n");	

			exit(0);

		default:
            printf("Error! operator is not correct\n\n");	
		}

	}

}



