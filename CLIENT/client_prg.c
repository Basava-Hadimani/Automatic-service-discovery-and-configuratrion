/******************************************************************************
 *
 *	FILE NAME 	: client_prg.c
 *
 *	DESCRIPTION	: This file consists of the program for client.
 *
 *	AUTHOR			DATE		PURPOSE
 *	Group_06	7/5/2017	Training Project	
 *
 *	Copyright 2017, Aricent Inc.
******************************************************************************/

/******************************************************************************
*	HEADERFILES
******************************************************************************/

#include <utility.h>

/******************************************************************************
*
*	FUNCTION NAME : main
*
*	DESCRIPTION	: This function is the main function for client
*			  from where actual program starts executing. 
*
*	RETURN TYPE	: int	
******************************************************************************/
int main(int argc, char *argv[])
{
	struct  sockaddr_in addr;	
	struct ip_mreq mreq;
	pthread_t tid;
	int 	ret = 0,
		l = 0, 
		nbytes = 0;
	u_int 	yes = 1;        
	char 	sendbuff[PACK];

/***************	Checking Command line input	******************************/
	if ( argc < 2 )
	{
		printf("Enter as <exe> <IP address> <portno> for multicast\n");
		return FAILURE;
	}   

/***************	Creating socket to receive from Multicast	***************/

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		perror("Notify socket");
		return FAILURE;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) 
	{
		perror("Reusing ADDR failed");
		close(fd);
		return FAILURE;
	}

/***************	Creating addr structure	**********************************/
	memset(&addr, 0, sizeof(addr));
	memset(&mreq, 0, sizeof(mreq));
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(atoi(argv[2]));
/***************	Creating multicast structure	**************************/
	mreq.imr_multiaddr.s_addr = inet_addr(argv[1]);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	
/***************	Making socket multicast	**********************************/
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) 
	{
		perror("setsockopt");
		close(fd);
		return FAILURE;
	}

	printf("-----------  client  -------------\n");

/**************	Creating seperate thread for receiving packets	**************/

	ret = pthread_create(&tid, NULL, receive_packet, (void *)&addr);
	if (ret != 0)
	{
		perror("Receive notify");
		close(fd);
		return FAILURE;
	}

	sleep(5);

/*****************************	To send MSEARCH request	 *********************/

	while (SUCCESS)
 	{
		if (flag) 	
		{
			l = snprintf(sendbuff, sizeof(sendbuff), "M-SEARCH * HTTP/1.1\r\n""S: uuid:%s\r\n""Host: %s:%d\r\n""Man: \"ssdp:discover\"\r\n""ST: FTP\r\n""MX: 0\r\n", UUID1, inet_ntoa(mreq.imr_multiaddr), ntohs(addr.sin_port));
			strcat(sendbuff, "\r\n");

			if ((nbytes = sendto(fd, sendbuff, sizeof(sendbuff), 0, (struct sockaddr *)&addr, sizeof(addr))) < 0)
			{
				perror("MSEARCH request sendto");
				close(fd);
				return FAILURE;
			}
			break;
		}
	}
	
	pthread_join(tid, NULL);
	close(fd);
	return FAILURE;
}