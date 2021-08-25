#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

#define BAUDRATE B4800			// Baudrate for NMEA 0183 communication
#define PORT0 "/dev/ttyACM0"
#define PORT1 "/dev/ttyACM1"
#define PORT2 "/dev/ttyUSB0"
#define PORT3 "/dev/ttyUSB1"
#define OUTFILE "serial.log"	// Output file name

const uint8_t COM1=0,COM2=1;
struct termios *oldtio1,*oldtio2;
int port[]={-1,-1};
FILE *f;

void signaux(int sigtype)
	{
	printf ("\nSignal received - Programme terminé\n");
	tcsetattr(port[COM1],TCSANOW,oldtio1);	// reload previous configuration
	tcsetattr(port[COM2],TCSANOW,oldtio2);	//
	free(oldtio1);
	free(oldtio2);
	close(port[COM1]);
	close(port[COM2]);
	fclose (f);
	exit(0);
	}

struct termios *init_com(int com, struct termios *newtio)
	{
	struct termios *tio;
	tio = malloc(sizeof(struct termios));
	if (com >= 0)
		{
		tcgetattr(com,tio); // save current configuration
		tcsetattr(com,TCSAFLUSH,newtio);
		//fcntl(com, F_SETFL, FNDELAY);	// non blocking
		fcntl(com, F_SETFL, 0);	// blocking
		}
	return tio;
	}

void *read_and_print(void* nport)
// Read serial port and print received data to a file,
// adding a time tag before each sentence.
	{
	time_t t;
	struct tm tv;
	int res=0;
	uint8_t n=*((uint8_t*)nport);
	char buf[100];

	if (port[n] >=0 )
		{
		tcflush(port[n], TCIFLUSH);
		res = read(port[n],buf,sizeof(buf)); // one empty reading
		while(1)
			{
			res = read(port[n],buf,sizeof(buf));
			if (res>0)
				{
				//if (!memcmp(buf+3,"GGA",3) || !memcmp(buf+3,"DPT",3)) // filter for some NMEA sentences
				//if (buf[0]=='$')										// filter only NMEA sentences
				//	{
					t=time(NULL);
    				localtime_r (&t,&tv);
					buf[res-1]=0;	// for NMEA
					fprintf(f,"%ld,%02d%02d%02d,%s\n",t,tv.tm_hour,tv.tm_min,tv.tm_sec,buf);
					fflush(f);
					if (n==COM2) printf("\E[31m%s\E[0m\n",buf);	// print COM2 output in color
					else puts(buf);
				//	}
				}
			}
		}
	return NULL;
	}

int main()
{
pthread_t t1,t2;
uint8_t i;
struct termios newtio;
int com;

const char *port_name[4];
port_name[0] = PORT0;
port_name[1] = PORT1;
port_name[2] = PORT2;
port_name[3] = PORT3;

f=fopen (OUTFILE,"w");
signal(SIGINT,signaux);
signal(SIGTERM,signaux);

for (i=0;i<4;i++) // Initialisation serial port
	{
	com = open(port_name[i], O_RDWR | O_NOCTTY);
	//printf("%d com = %d\n",i,com);
	if (com >= 0)
		{
		if (port[COM1] <0)
			{
			port[COM1] = com;
			printf("port[COM1] = %s\n",port_name[i]);
			}
		else if (port[COM2] <0)
			{
			port[COM2] = com;
			printf("\E[31mport[COM2] = %s\E[0m\n",port_name[i]);
			}
		}
	}

if (port[COM1] <0 && port[COM2] <0) {perror("Serial"); return(-1); }

bzero(&newtio, sizeof(newtio));
newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
newtio.c_oflag = 0;
newtio.c_lflag = ICANON;

oldtio1=init_com(port[COM1],&newtio);
oldtio2=init_com(port[COM2],&newtio);

pthread_create(&t1,NULL,read_and_print,(void *)&COM1);
pthread_create(&t2,NULL,read_and_print,(void *)&COM2);

pthread_join(t1,NULL);
}
