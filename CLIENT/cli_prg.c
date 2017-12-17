#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#define HELLO_PORT 12345
#define CLI_PORT 12346
#define PACK 512
#define HELLO_GROUP "225.0.0.37"
#define MSGBUFSIZE 256
#define MAX_STR 81
#define SUCCESS 1
#define FAILURE 0

int fd = 0;

void *mythread(void *cli)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)cli;
	int addrlen = 0;
	int nbytes = 0;
	char 	msgbuf[MSGBUFSIZE];

	addrlen=sizeof(*addr);
	while (1)
	{
		if ((nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *)addr,(socklen_t *) &addrlen)) < 0) 
		{
			perror("recvfrom");
			close(fd);
			return FAILURE;
		}
		printf("packet from server :\n %s\n", msgbuf);
	}
}

int main(int argc, char *argv[])
{
	struct  sockaddr_in cliaddr;
	struct 	sockaddr_in addr;
	int	l = 0, 
		sd = 0,
		nbytes = 0,
		addrlen = 0;
	struct 	ip_mreq mreq;
	char 	sendbuff[PACK];
	char 	recvbuff[MAX_STR];
	char 	hi[MAX_STR] = "hi";
	int 	size = 0;
	pthread_t tid;
	u_int 	yes = 1;            
	int ret = 0;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		perror("socket");
		return FAILURE;
	}

	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket response");
		return FAILURE;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) 
	{
		perror("Reusing ADDR failed");
		close(fd);
		return FAILURE;
	}


	memset(&addr, 0, sizeof(addr));
	memset(&mreq, 0, sizeof(mreq));
	memset(&cliaddr, 0, sizeof(cliaddr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(HELLO_PORT);

	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	cliaddr.sin_port = htons(CLI_PORT);


	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
	{
		perror("bind");
		close(fd);
		return FAILURE;
	}

	mreq.imr_multiaddr.s_addr = inet_addr(HELLO_GROUP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) 
	{
		perror("setsockopt");
		close(fd);
		return FAILURE;
	}

	
	printf("-----------client-------------\n");
	ret = pthread_create(&tid, NULL, mythread, (void *)&addr);
	if (ret != 0)
	{
		perror("thread create");
		close(fd);
		return FAILURE;
	}


	while (1) 
	{
		
		

		l = snprintf(sendbuff, sizeof(sendbuff), "M-SEARCH * HTTP/1.1\r\n""USN : palindrome\r\n""Host: %s:%d\r\n""Man: \"ssdp:discover\"\r\n",inet_ntoa(addr.sin_addr), CLI_PORT);
		strcat(sendbuff, "\r\n");

		if ((nbytes = sendto(sd, sendbuff, sizeof(sendbuff), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr))) < 0)
		{
			perror("cli sendto");
			close(fd);
			return FAILURE;
		}

		size = sizeof(cliaddr);

		if (recvfrom(sd, recvbuff, sizeof(recvbuff), 0, (struct sockaddr *)&cliaddr, (socklen_t *)&size) < 0)
		{
			perror("recv response");
			close(fd);
			return FAILURE;
		}

		printf("response from server : %s\n", recvbuff);


	}
	close(sd);
	close(fd);
	
	return FAILURE;

}

