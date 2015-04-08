#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/select.h>

#define MAXLINE 80
int port = 80;

int main(int argc, char *argv[])
{
	struct sockaddr_in pin;
	int sock_fd;
	char buf[MAXLINE];
	char str[MAXLINE];
	int n;
	fd_set wset;     
    	struct timeval tval; 
	int error, len, code;


	signal(SIGPIPE,SIG_IGN);

	bzero(&pin, sizeof(pin));
	pin.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &pin.sin_addr);
	pin.sin_port = htons(port);

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0)
	{
		perror("call connect");
		exit(1);
	}

	int flags = fcntl(sock_fd, F_GETFL, 0);
	fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);
	printf("fcntl %d\n", flags);	
	n=connect(sock_fd, (void *)&pin, sizeof(pin));
	if (-1 == n)
	{
		printf("Step in\n");
		if(errno != EINPROGRESS)
			exit(1);
		
		FD_ZERO(&wset);     
    		FD_SET(sock_fd, &wset);     
    		tval.tv_sec = 10;     
    		tval.tv_usec = 0; 
		
		printf("Step select\n");
		if ((n = select(sock_fd+1, NULL, &wset, NULL, &tval)) == 0) 
		{     
        		close(sock_fd);  /* timeout */    
    			printf("Connect timeout\n");
			exit(1);
		}
		   
		if (FD_ISSET(sock_fd, &wset)) 
		{
			
			len = sizeof(error);     
        		code = getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
			if(error)
			{
				printf("Connect failed\n");			
				exit(1);
			}
			printf("Step getsockopt\n");
		}
	}
	struct timeval tv = {5,0};
	int ret;

	printf("Connect succeed!\n");
	fcntl(sock_fd, F_SETFL, O_RDWR);
	
	ret = setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(tv));
	printf("ret %d\n", ret);

	ret = recv(sock_fd, buf, 80, 0);
	printf("ret %d\n", ret);

	close(sock_fd);
	return 0;
}
