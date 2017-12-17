/******************************************************************************
 *
 *	FILE NAME 	: client_func.c
 *
 *	DESCRIPTION	: This file consists of the sub function defination 
 *			  required for client.
 *
 *	AUTHOR			DATE		PURPOSE
 *	Group_06	7/5/2017	Training Project	
 *
 *	Copyright 2017, AricentInc.
******************************************************************************/

 /******************************************************************************
*	HEADERFILES
*****************************************************************************/

#include <utility.h>

/******************************************************************************
 *
 *	FUNCTION NAME : receive_packet
 *
 *	DESCRIPTION	: This function is required to receive the notify and MSEARCH
 *			  packet from the server.
 *
 *	RETURN TYPE	: void*	
 ******************************************************************************/
void *receive_packet(void *ad)
{
	int  	nbytes = 0,
		addrlen = 0;
	struct 	sockaddr_in *addr = (struct sockaddr_in *)ad;
	char 	msgbuf[MSGBUFSIZE];
	char    *p = NULL;	

	if (bind(fd, (struct sockaddr *)addr, sizeof(*addr)) < 0) 
	{
		perror("Client Notify Bind");
		close(fd);
		exit(1);
	}

	addrlen = sizeof(*addr);
	
/*********	To receive NOTIFY and MSEARCH packets from server ****************/

	while (SUCCESS)
	{
		if ((nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *)addr, (socklen_t *)&addrlen)) < 0) 
		{
			perror("Client Notify Recvfrom");
			close(fd);
			return FAILURE;
		}

		printf("%s\n", msgbuf);

		if ((p = strstr(msgbuf, NOTIFY)) != NULL)
		{
			flag = 1;
		}

/**************************		validation		*******************************/
		p = strstr(msgbuf, OK);
		if (p != NULL)
		{
			if (SUCCESS != validate_msearch_response(msgbuf))
			{
				printf("error in calling validate\n");
				close(fd);
				return FAILURE;
			}
		}

	}
	
	close(fd);
	pthread_exit(NULL);
	
	return NULL;
}

/******************************************************************************
 *
 *	FUNCTION NAME : validate_msearch_response
 *
 *	DESCRIPTION	: This function is required to open the text file and to 
 *			  configure the clients.
 *
 *	RETURN TYPE	: int	
 ******************************************************************************/

 int validate_msearch_response(char *buff)
{
	int param = 0;
	char *temp = NULL;
	char temp2[MAX_STR];
	FILE *fp = NULL;
	char *token = NULL;
	char tempbuff[MAX_STR];
	char *tok = NULL;
/******	To handle CTRL+C signal	*******/
	signal(SIGINT, tcpsig);

/***********	Checking file for parameters	******************************/
	temp = strstr(buff, AL);
	if (temp != NULL)
	{
		token = strtok(temp, DELIMCOLON);
		token = strtok(NULL, DELIMCOLON);
		if (token != NULL)
		{
			token[strlen(token) - 2] = '\0';	
			fp  = fopen(token, "r");
			if (fp == NULL)
			{
				printf("error in file opening\n");
				return FAILURE;
			}

			while (fgets(tempbuff, sizeof(tempbuff), fp))
			{
				strcpy(temp2, tempbuff);
/***************** 	extracting ip address of server **************************/
				if (strstr(temp2, IPSTRING))
				{
					tok = strtok(temp2, DELIMCOLON);
					tok = strtok(NULL, DELIMCOLON);
					tok = strtok(tok, ENDL);
					strcpy(server_ip, tok);
				}
/***************** 	extracting port number of the service	******************/

				if (strstr(temp2, PORTSTRING))
				{
					tok = strtok(temp2, DELIMCOLON);
					tok = strtok(NULL, DELIMCOLON);
					tok = strtok(tok, ENDL);
					strcpy(server_port, tok);
				}
			}
			fclose(fp);
		}
	}

/******************	Calling configure_paramater func	**********************/
	param = configure_parameter();

	if (SUCCESS != request_parameter(param))
	{
		printf("error in request_parameter function\n");
		return FAILURE;
	}
		
	return SUCCESS;
}

/******************************************************************************
 *
 *	FUNCTION NAME : tcpsig
 *
 *	DESCRIPTION	: This function is required to handle the CTRL+C interrupt 
 *			  during the TCP connection.
 *
 *	RETURN TYPE	: void	
 ******************************************************************************/
void tcpsig(int sig)
{
	printf("\nTCP terminated\n");
	close(sd);
	close(fd);
	exit(FAILURE);
}

