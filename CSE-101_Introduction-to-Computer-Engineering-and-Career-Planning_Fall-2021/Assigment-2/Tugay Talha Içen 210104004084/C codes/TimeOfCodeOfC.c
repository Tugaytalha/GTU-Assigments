#include <stdio.h>
#include "arduino-serial-lib.h"
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>


int main() {
	int  n, fd=1, square, buttonpress, i;
	const char* port="/dev/ttyUSB2"; //PUT YOUR PORT HERE  <<<<-------------------
	char buf[256];

	printf(
	"#$       WELCOME TO GTU ARDUINO LAB     $#  \n"
	"#$     STUDENT NAME: TUGAY TALHA İÇEN   $#  \n"
	"#$    PLEASE SELECT FROM THE FOLLOWING  $#\n\n"
	"                 '''MENU'''                 \n"
	"(1) TURN ON LED ON ARDUINO \n"
	"(2) TURN OFF LED ON ARDUINO\n"
	"(3) FLASH ARDUINO LED 3 TIMES\n"
	"(4) SEND A NUMBER TO ARDUINO TO COMPUTE SQUARE BY ARDUINO\n"
	"(5) Button press counter (bonus item)\n"
	"(0) EXIT (Without turn off led)\n"
	"(9) EXIT (Turn off led)\n"
	);

	fd = serialport_init(port, 9600); 
	serialport_flush(fd);

	while (1) {

		printf("\nPLEASE SELECT:\n");
		scanf("%d",&n); 

		if (n==1 || n==2 || n==3) { //send data to our port (I hope the arduino is installed here)
			serialport_writebyte(fd, (uint8_t)n);
    	}
		else if (n==4) {
			printf("\nPLEASE GİVE A NUMBER FOR TAKE SQUARE:\n");
			scanf("%d",&square);
			serialport_writebyte(fd, (uint8_t)n); 
			serialport_writebyte(fd, (uint8_t)square);
			usleep(2000000);
 			serialport_read_until(fd, buf, '\n', 256, 3000);
			printf("SQUARE OF YOUR NUMBER:%s\n",buf);
			
		}
		else if (n==5) {
			printf("\nHOW MANY TIMES YOU WİLL BE PRESS THE BUTTON?:\n");
			scanf("%d",&buttonpress);
			serialport_writebyte(fd, (uint8_t)n); 
			serialport_writebyte(fd, (uint8_t)buttonpress);
			for (i=0; i<buttonpress; i++) {
				memset(buf, 0, sizeof(buf));
				usleep(50000);
		 		serialport_read_until(fd, buf, '\n', 256, 5000);
				printf("Button press count:%s",buf);
			}
		}
		else if (n==0) {
			exit(EXIT_SUCCESS);
		}
		else if (n==9) {
			serialport_writebyte(fd, (uint8_t)2);
			exit(EXIT_SUCCESS);

		}
	}
}







