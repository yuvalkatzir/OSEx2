#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

#define TIMEOUT 60
int nCounter = TIMEOUT;

int calc(char* buf);

void SIGUSR1_handler(int sig)
{
	char *tokptr;      /* a strtok return value; */
	int proc_pid;      /* holds the pid of the client */
	int fd;	
	char buf[256]; 
	int tmp;

	signal(SIGUSR1,SIGUSR1_handler);
	memset(buf,0,256);
	printf("pid=%d receives a signal %d\n",getpid(),sig);
	sleep(1);	
	/* read the file content */
		fd=open("to_server",O_RDONLY);
		if(fd < 0) 
		{
			perror("cant open: com_file"); 
			exit(0);
		}
		if ( read(fd,buf,256) > 0)
			printf("received: %s\n",buf);
		close(fd);
        remove("to_server");
		/* parse file content */
		tokptr=strtok(strdup(buf)," ");
		proc_pid = atoi(tokptr);
		/* do some complicated calculation */
		tmp = calc(buf);

		/* reply with result */
		memset(buf,0,256);
		fd=open("to_client",O_WRONLY |O_CREAT|O_TRUNC,S_IRWXU);
		if( fd<0 )
		{ 
			perror("cant open: com_file"); 
			exit(0);
		}
		sprintf(buf,"%d\0",tmp);
		write(fd,buf,(int)(sizeof(int) * strlen(buf)));
		close(fd);
	
		kill(proc_pid,SIGUSR1);
		printf("\n\n");
		nCounter = TIMEOUT;
	}
	
	int main(int argc, char **argv)
	{
		signal(SIGUSR1,SIGUSR1_handler); /* handles signal from other process */
		printf("server pid is= %d\n",getpid());
		while (nCounter != 0) {
			//pause();  /* waiting for other to signal */
			nCounter--;
			sleep(1);
			printf("%d\n",nCounter);
		}

		printf("Server Closed Do To Inactivity\n");
	}
	
	int calc(char* buf){
		strcat(buf," ");
		char buffy[4][256];
		int nIndex = 0;
		while(nIndex != 4){
			strcpy(buffy[nIndex],strtok(strdup(buf)," "));
			buf = strchr(buf,' ');
			buf += 1;
			nIndex++;
		}
		int returnValue = 0;
		switch(atoi(buffy[2])){
			case 1: returnValue = atoi(buffy[1]) + atoi(buffy[3]);
				break;
			case 2: returnValue = atoi(buffy[1]) - atoi(buffy[3]);
				break;
			case 3: returnValue = atoi(buffy[1]) * atoi(buffy[3]);
				break;
			case 4: returnValue = atoi(buffy[1]) / atoi(buffy[3]);
				break;
		}

		return returnValue;
	}