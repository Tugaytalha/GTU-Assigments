#include <stdio.h>
#define pi 3.14

int main() {
	float x1, y1, x2, y2;
	float midx, midy, slope, tan, temp1, temp2, r, area;
	printf("x1:");
	scanf("%f", &x1);
	printf("y1:");
	scanf("%f", &y1);
	printf("x2:");
	scanf("%f", &x2);
	printf("y2:");
	scanf("%f", &y2);
	
	midx = (x1+x2)/2;
	midy = (y1+y2)/2;
	
	printf("\nMidpoint coordinates:\nx=%.1f\ny=%.1f", midx, midy);
	
	slope = (y2 - y1)/(x2 - x1);
	
	printf("\n\nSlope=%.1f\n", slope);	
	
	temp2 = (x1 - midx);
	if (temp1<0) temp1 *= -1; //Positive can better
	temp1 = (y1 - midy);
	if (temp1<0) temp1 *= -1;
	/*tan = (temp1/temp2)/(temp2/temp1);
	r = temp1*tan;
	area = pi*r*r;*/
	area= pi*((temp1*temp1)+(temp2*temp2));
	printf("\nArea=%.1f\n", area);	
	
	printf("\nWaiting enter...");
	getchar();
	getchar();

	return (0);
	
}
