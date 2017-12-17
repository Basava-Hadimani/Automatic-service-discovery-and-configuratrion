/******************************************************************************
 *
 *	FILE NAME 	: server_prg.c
 *
 *	DESCRIPTION	: This file consists of the program for server.
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
*	FUNCTION NAME 	: main
*
*	DESCRIPTION	: This function is the main function for server
*			  from where actual program starts executing. 
*
*	RETURN TYPE	: int	
******************************************************************************/
int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr;
	pthread_t tid;
	pthread_t thd[MAX_CLI];
	int ret = 0;
	int i = 0;
	int size = 0;
	size = sizeof(servaddr);	
	
	flag_server = 0;

/************** 	Opening file to maintain log	**************************/
	fptr = fopen(LOGPATH, "a+");
	if (fptr == NULL)
	{
		printf("Error in opening log file..\n");
		return FAILURE;
	}

/***************	Checking Command line input	******************************/	

	if (argc < 2)
	{
		printf("Enter as <exe> <IP address> <portno> for multicast\n");
		return FAILURE;
	}

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		perror("socket");
		return FAILURE;
	}

/***************	Creating socket to send notify	**************************/	
	
	memset(&addr, 0, sizeof(addr));
	memset(&servaddr, 0, sizeof(servaddr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(atoi(argv[2]));

	printf("------------- FTP server---------------\n");

/*******************To send NOTIFY periodically*******************************/
	ret = pthread_create(&tid, NULL, notify, NULL);
	if (ret != 0)
	{
		perror("thread create");
		close(fd);
		return FAILURE;
	}

	if (bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("bind msearch");
		close(fd);
		return FAILURE;
	}
	
/************** To receive MSEARCH request ***********************************/	
	while (1) 
	{
		if (recvfrom(fd, recvbuff, sizeof(recvbuff), 0, (struct sockaddr *)&servaddr, (socklen_t *)&size) < 0)
		{
			perror("serv recv");
			close(fd);
			return FAILURE;
		}
		flag_server++;

		printf("MSEARCH request from client :\n%s\n", recvbuff);
		
		serverlog(MSEARCH, servaddr);

/*****	Creating thread for respective client	******************************/
		ret = pthread_create(&thd[i], NULL, client_service_handle, (void *)&servaddr);
		if (ret != 0)
		{
			perror("thread cli");
			close(fd);
			return FAILURE;
		}
		i++;
	}	
	close(fd);
	
	fclose(fptr);
	return SUCCESS;
}

