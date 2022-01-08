#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/random.h>

#define TIMEOUT 30
int nCounter = TIMEOUT;

int proc_pid;
int server_pid;

/* handles signal from other process */
void SIGUSR1_handler(int sig) 
{
	int fd;
	char buf[256];
	int tmp;
	signal(SIGUSR1,SIGUSR1_handler);
	printf("pid=%d receives a signal %d\n",getpid(),sig);
	sleep(1);
	fd=open("to_client",O_RDONLY);
	if (fd < 0) 
	{
		perror("cant open: com_file"); exit(0);
	}
	if (read(fd,buf,256) > 0) printf("received: %s\n",buf);	
	close(fd);
	nCounter = -1;
}
/* send data and signal other process */
void sendReq(char *data) 
{
	int fd;
    unsigned int nTime;
    int nIndex = 0;
    if( access( "to_server", F_OK ) == 0 ) {
        for(nIndex;nIndex < 10; nIndex++){
            if( access( "to_server", F_OK ) == 0 ) {
                getrandom(&nTime, sizeof(unsigned int), GRND_NONBLOCK);
                nTime = nTime % 5 + 1;
                while(nTime > 0) 
                { 
                    sleep(1);
                    nTime--;
                }
            } else {
                fd=open("to_server",O_WRONLY|O_CREAT|O_TRUNC|O_SYNC,S_IRWXU);
                write(fd,data,strlen(data));
                close(fd);
                kill(server_pid,SIGUSR1);
                break;
            }
        }
        if (nIndex == 10)
        {
            printf("ERROR_FROM_EX2\n");
            exit(0);
        }
    } else {
        fd=open("to_server",O_WRONLY|O_CREAT|O_TRUNC|O_SYNC,S_IRWXU);
        write(fd,data,strlen(data));
	    close(fd);
	    kill(server_pid,SIGUSR1);
    }
}
int main(int argc, char **argv)
{
	char buf[256];
	signal(SIGUSR1,SIGUSR1_handler); /* handles signal from other process */
	if (argc != 5)
	{
		printf("Wrong amount of parameters\n");
        exit(0);
	} 
	proc_pid = getpid();
	server_pid = atoi(argv[1]);
	printf("client pid is= %d\n",getpid());
	memset(buf,0,256);
	sprintf(buf,"%d \0",proc_pid);
	strcat(buf,argv[2]);
	strcat(buf," ");
	strcat(buf,argv[3]);
	strcat(buf," ");
	strcat(buf,argv[4]);
	printf("%s\n",buf);
	sendReq(buf);
	
	while (nCounter > 0)
	{
		sleep(1);
		nCounter--;
	}
	if(nCounter == 0){
		printf("Client closed because no response was received from the server\n");
	}
}
