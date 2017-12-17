#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#define HELLO_PORT 12345
#define SERV_PORT 12346
#define HELLO_GROUP "225.0.0.37"
#define MAX_STR 256
#define PACK 512
#define SUCCESS 1
#define FAILURE 0

int fd = 0;

void *mythread(void *srv)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)srv;
	char sendbuff[PACK];
	int l = 0;

	l = snprintf(sendbuff, sizeof(sendbuff), "NOTIFY * HTTP/1.1\r\n""Host: %s:%d\r\n""NT: stringmanipulation\r\n""NTS: ssdp:alive\r\n""USN: palindrome\r\n",inet_ntoa(addr->sin_addr), SERV_PORT);
	strcat(sendbuff, "\r\n");
	while(1)
	{
		if (sendto(fd, sendbuff, sizeof(sendbuff), 0, (struct sockaddr *)addr, sizeof(*addr)) < 0) 
		{
			perror("sendto");
			close(fd);
			return FAILURE;
		}
		sleep(3);
	}

}



int main(int argc, char *argv[])
{
	struct sockaddr_in addr;
	struct sockaddr_in servaddr;
	pthread_t tid;
	int ret = 0;
	char message[MAX_STR] = "NOTIFY PACKET !";
	//char buff[MAX_STR] = "Bye, Bye!"; 
	char recvbuff[PACK];
	int size = 0;
	size = sizeof(addr);


	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		perror("socket");
		return FAILURE;
	}

	memset(&addr, 0, sizeof(addr));
	memset(&servaddr, 0, sizeof(servaddr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(HELLO_GROUP);
	addr.sin_port = htons(HELLO_PORT);

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);		


	ret = pthread_create(&tid, NULL, mythread, (void *)&addr);
	if (ret != 0)
	{
		perror("thread create");
		close(fd);
		return FAILURE;
	}

	while (1) 
	{
		printf("%s\n", message);	

		if (recvfrom(fd, recvbuff, sizeof(recvbuff), 0, (struct sockaddr *)&addr, (socklen_t *)&size) < 0)
		{
			perror("serv recv");
			close(fd);
			return FAILURE;
		}

		printf("packet from client :%s\n", recvbuff);

		sleep(3);
	}	
	close(fd);

	return SUCCESS;
}


