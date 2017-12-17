/******************************************************************************
 *
 *	FILE NAME 	: server_func.c
 *
 *	DESCRIPTION	: This file consists of the sub function defination 
 *			  required for server.
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
*	FUNCTION NAME 	: sighandler
*
*	DESCRIPTION	: This function is required to handle the interrupt
*			  i.e CTRL+C.
*
*	RETURN TYPE	: void	
******************************************************************************/
void sighandler(int sig)
{
	char sendbuff[PACK];
	int l = 0;
	
	printf("\nTerminated using ctrl+c\n");

	l = snprintf(sendbuff, sizeof(sendbuff), "NOTIFY * HTTP/1.1\r\n""Host: %s:%d\r\n""NT: FTP\r\n""NTS: ssdp:byebye\r\n""USN:uuid: %s\r\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), UUID1);
	strcat(sendbuff, "\r\n");

/*******Sending BYEBYE packet on system exit**********************************/

	if (sendto(fd, sendbuff, sizeof(sendbuff), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
	{
		perror("sendto");
		close(fd);
		exit(FAILURE);
	}	
	close(td);
	close(fd);
	exit(FAILURE);
}

/******************************************************************************
*
*	FUNCTION NAME 	: notify
*
*	DESCRIPTION	: This function is required to send the notify 
*				  packet to all interested clients periodically.
*
*	RETURN TYPE	: void*	
******************************************************************************/
void *notify()
{
	int l = 0;
	char sendbuff[PACK];

	signal(SIGINT, sighandler);
	
	while(1)
	{
		l = snprintf(sendbuff, sizeof(sendbuff), "NOTIFY * HTTP/1.1\r\n""Host: %s:%d\r\n""NT: FTP\r\n""NTS: ssdp:alive\r\n""USN:uuid: %s\r\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), UUID1);
		strcat(sendbuff, "\r\n");
		
/**************	Sending NOTIFY packets periodically	**************************/

		if (sendto(fd, sendbuff, sizeof(sendbuff), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
		{
			perror("sendto");
			close(fd);
			return FAILURE;
		}

		sleep(3);
	}
	return NULL;
}

/******************************************************************************
*
*	FUNCTION NAME : server_responseftp
*
*	DESCRIPTION	: This function is required to send the respose to 
*			  the client requesting for FTP after receiving the M SEARCH 
*			  packet
*
*	RETURN TYPE	: int	
******************************************************************************/
int server_responseftp(struct sockaddr_in *servaddr)
{
	int sd = 0;
	struct sockaddr_in config;
	char *pat = NULL;
	char sbuff[PACK];
	int k = 0;
	FILE *fp = NULL;

/********* 	Validating UUID for service	**************************************/	
	
	pat = strstr(recvbuff, UUID1);
	if (pat != NULL)
	{
		sd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sd < 0)
		{
			perror("socket response");
			return FAILURE;
		}

		if (flag_server == 1)
		{
			sprintf(par.port, "%d", CONFIG_PORT);
		}

		else if (flag_server > 1)
		{
			sprintf(par.port, "%d", atoi(par.port));
		}

/**********	Opening file to read configuration parameters	******************/

		fp = fopen(FILEPATH, "w");
		if (fp == NULL)
		{
			printf("error in writing file\n");
			close(sd);
			return FAILURE;
		}

		fprintf(fp, "IP: %s\nPORT: %s", "127.0.0.1", par.port);
		fclose(fp);
			
		k = snprintf(sbuff, sizeof(sbuff), "HTTP/1.1 200 OK\r\n""S: uuid:%s\r\n""ST: FTP\r\n""AL: ../../data/ftpfile.txt", UUID1);
		strcat(sbuff, "\r\n");
		
/****************	Sending MSEARCH response	******************************/	

		if (sendto(sd, sbuff, sizeof(sbuff), 0, (struct sockaddr *)servaddr, sizeof(*servaddr)) < 0)
		{
			perror("send response\n");
			close(sd);
			return FAILURE;
		}
		close(sd);

/*********	Calling function to receive parameter request	******************/		
		
		if (SUCCESS != receive_parameter_request(config))
		{
			printf("error in receive_parameter_request function");
			return FAILURE;
		}
	}	
			
return SUCCESS; 
}
/******************************************************************************
*
*	FUNCTION NAME 	: client_service_handle
*
*	DESCRIPTION		: This function is validate the M search packet
*			 	  sent by clients.
*
*	RETURN TYPE		: void *
******************************************************************************/

void *client_service_handle(void *srv)
{
	char *pat = NULL;
	struct sockaddr_in *temp;
	
	temp = (struct sockaddr_in *)srv;
	
/*************	Validating UUID in MSEARCH response	**************************/

	if ((pat = strstr(recvbuff, UUID1)) != NULL)
	{  
		if (SUCCESS != server_responseftp(temp))
		{
			printf("error in sending ftp response\n");
			return FAILURE;
		}
	}
	else 
	{
		printf("MSEARCH response packet undefined\n");
		pthread_exit(FAILURE);
	}

	pthread_exit(NULL);
}


/******************************************************************************
*
*	FUNCTION NAME 	: serverlog
*
*	DESCRIPTION		: This function is maintaining a lof of requests and 
*					responses
*
*	RETURN TYPE		: void 
******************************************************************************/
void serverlog(char *buf, struct sockaddr_in servaddr)
{
	time_t rawtime;
	struct tm * timeinfo;
	char buff[MAX_STR];
	int len = 0;
	
/************************	To find current time	**************************/

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	len = strlen(asctime (timeinfo));
	stpcpy(buff, asctime(timeinfo));
	buff[len - 1] = '\0';
	
/***********	Writing log to file	******************************************/
	fprintf (fptr, "%s\t%s\t%d\t%s\n", buff,inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port), buf);
}

/******************************************************************************
*
*	FUNCTION NAME 	: tcpsig
*
*	DESCRIPTION		: This function is called on getting signal, ctrl+c.
*
*	RETURN TYPE		: void 
******************************************************************************/

void tcpsig(int sig)
{

	printf("\nTCP connection terminated by server.. Server Exited\n");
	char buff[MAX_STR] = EXIT;
	if (sigflag == 0)
	{
		sigflag++;
		if (write(client_id, buff, strlen(buff) + 1) < 0)
		{
			perror("write final");
			close(client_id);
			close(td);
			exit(FAILURE);
		}
	}
	close(client_id);
	close(td);
	exit(FAILURE);
}