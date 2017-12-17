/******************************************************************************
 *
 *	FILE NAME 	: server_connect.c
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
*	FUNCTION NAME 	: parameter_validate_response
*
*	DESCRIPTION	: This function is the main function for server
*			  from where actual program starts executing. 
*
*	RETURN TYPE	: integer
******************************************************************************/

int parameter_validate_response(char *buff, struct sockaddr_in config)
{
	char resp_buff[MAX_STR];
	char *token = NULL;
	char *p = NULL;
	flag_bind = 0;
	
/****************	Validating PORT Number	**********************************/

	p = strstr(buff, PORTSTRING);
	if (p != NULL)	
	{
		token = strtok(p, DELIMCOLON);
		token = strtok(NULL, DELIMCOLON);
		token = strtok(token, ENDL);
	
		if (strcmp(par.port, token) == 0)
		{
			sprintf(resp_buff, "You can continue with port : %s ", par.port);
			
/****************	Sending response back to client	**************************/

			if (write(client_id, resp_buff, strlen(resp_buff) + 1) < 0)
			{
				perror("write final");
				close(client_id);
				close(td);
				return FAILURE;
			}
		}

		else
		{
			serverlog(CONFIG_REQ, config);
			flag_bind = 1;
			strcpy(par.port, token);	
			sprintf(resp_buff, "change in port, server gonna restart with port: %s ", par.port);
			serverlog(CONFIG_RESP, config);
			
/****************	Sending response back to client	**************************/
			
			if (write(client_id, resp_buff, strlen(resp_buff) + 1) < 0)
			{
				perror("write final");
				close(client_id);
				return FAILURE;
			}
						
		}
	}
	return SUCCESS;
}

/******************************************************************************
*
*	FUNCTION NAME 		: receive_parameter_request
*
*	DESCRIPTION		: This function is required to make the actual TCP
*			 	  connection with the client.
*
*	RETURN TYPE		: integer	
******************************************************************************/
int receive_parameter_request(struct sockaddr_in config)
{
	int td = 0;
	char recvbuff[MAX_STR];	
	int size = sizeof(config);
	
	signal(SIGINT, tcpsig);
	
/*********	Creating socket for TCP connection	******************************/
	
	td = socket(AF_INET, SOCK_STREAM, 0);
	if (td < 0)
	{
		perror("socket response");
		return FAILURE;
	}	

/**************	Assigning values to destination structure	******************/
	
	memset(&config, 0, sizeof(config));

	config.sin_family = AF_INET;
	config.sin_addr.s_addr = inet_addr(IPADDR);
	config.sin_port = htons(atoi(par.port));

	printf("Server starts with port number----->%d\n", ntohs(config.sin_port));
	
/**************	Binding the destination structure to socket	******************/
	
	if ((flag_server == 1) || (flag_bind > 0))
	{
		if (bind(td, (struct sockaddr *)&config, sizeof(config)) < 0)
		{
			perror("bind tcp connect");
			close(td);
			return FAILURE;
		}
	}
	
/**********	Listening for TCP connection	**********************************/
	
	if (listen(td, BACKLOG) < 0)
	{
		perror("error in listen\n");
		close(td);
		return FAILURE;	
	}

/*********	To accept TCP connetion from client	******************************/

	while(SUCCESS)
	{
		client_id = accept(td, (struct sockaddr *)&config, (socklen_t *)&size);
		if (client_id < 0)
		{
			perror("tcp accept");	
			close(td);
			return FAILURE;
		}
		
		sigflag = 0;
		
		printf("Configure request from client \n");

		if (read(client_id, recvbuff, sizeof(recvbuff)) < 0)
		{
			perror("read configuration parameters");
			close(client_id);
			close(td);
			return FAILURE;
		}
		printf("%s\n", recvbuff); 
		if (SUCCESS != parameter_validate_response(recvbuff, config))
		{
			printf("error in calling parameter_validate_response func\n");
			close(client_id);
			close(td);
			return FAILURE;
		}			
	}

	close(client_id);
	close(td);
	return SUCCESS;
}
