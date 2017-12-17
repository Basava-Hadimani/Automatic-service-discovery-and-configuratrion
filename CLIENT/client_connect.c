/******************************************************************************
 *
 *	FILE NAME 	: client_connect.c
 *
 *	DESCRIPTION	: This file consists of the sub function defination 
 *			  required for connecting client to server.
 *
 *	AUTHOR			DATE		PURPOSE
 *	Group_06	7/5/2017	Training Project	
 *
 *	Copyright 2017, AricentInc. ******************************************************************************/

 /******************************************************************************
*	HEADERFILES
*****************************************************************************/

#include <utility.h>


/******************************************************************************
 *
 *	FUNCTION NAME : configure_parameter
 *
 *	DESCRIPTION	: This function is required to change configurations.
 *
 *	RETURN TYPE	: integer	
 ******************************************************************************/

int configure_parameter()
{
	int c = 0;
	int p = 0;
	
/**************	Menu to change parameters	**********************************/

	printf("-----------Parameters-------------\n");
	printf("1.IP: %s\n2.PORT: %s\n", server_ip, server_port);
	printf("-----------configuration----------\n");
	printf("enter as per the number to configure parameters\n");
	printf("1.IP\n2.PORT\n3.EXIT\n");

	while(SUCCESS)
	{
		printf("Enter your choice\n");
		scanf("%d", &c);
		
		switch(c)
		{
			case IP:
		       		printf("Cannot change the IP address\n");
				break;

			case PORT:
				printf("Enter the PORT no\n");
				scanf("%d", &p);
				break;

			case EXIT:
				printf("Configuration exited\n");
				return p;
		
			default:
				printf("Enter the correct option");
				break;
		}
	}
	
	return p;	
}		

/******************************************************************************
 *
 *	FUNCTION NAME : request_parameter
 *
 *	DESCRIPTION	: This function is required to make connection with the server.
 *
 *	RETURN TYPE	: Integer.	
 ******************************************************************************/
 
int request_parameter(int param)
{  
	int sd = 0;
	char buffer[MAX_STR];
	char sendbuff[MAX_STR];
	struct sockaddr_in config;
	
/***********	To handle CTRL+C signal	**************************************/

	signal(SIGINT, tcpsig);
	
/***************	socket for TCP connection	******************************/
	
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0)
	{
		perror("socket response");
		return FAILURE;
	}

	memset(&config, 0, sizeof(config));
	
	config.sin_family = AF_INET;
	config.sin_addr.s_addr = inet_addr(server_ip);
	config.sin_port = htons(atoi(server_port));
	
/**************	Connection req to TCP server	******************************/

	if (connect(sd, (struct sockaddr*)&config, sizeof(config)) < 0)
	{
		perror("error in connecting");
		close(sd);
		return FAILURE;
	}

/******	Sending configuration req to server and receiving response	**********/

	while(1)
	{
		memset(sendbuff, '\0', sizeof(sendbuff));	
		
		if (param == 0)
		{
			sprintf(sendbuff, "IP :%s\nPORT :%s", server_ip, server_port);
			
 
			if (write(sd, sendbuff, sizeof(sendbuff)) < 0)
			{
				perror("Error Write Configure parameters\n");
				close(sd);
				exit(FAILURE);
			}
			if (read(sd, buffer, sizeof(buffer)) < 0)
			{
				perror("Error Read Configuration response");
				close(sd);
				exit(FAILURE);
			}

			printf("TCP server response :\n %s\n", buffer);
			
		}
		else
		{
			sprintf(sendbuff, "IP :%s\nPORT :%d", server_ip, (int)param);
 
			if (write(sd, sendbuff, sizeof(sendbuff)) < 0)
			{
				perror("Error Write Configure parameters\n");
				close(sd);
				return FAILURE;
			}
			sprintf(server_port, "%d", (int)param);
			param = 0;
			
			if (read(sd, buffer, sizeof(buffer)) < 0)
			{
				perror("Error Read Configuration response");		
				close(sd);
				return FAILURE;
			}

			printf("TCP server response :\n %s\n", buffer);
		}	

	}

	close(sd);
	return SUCCESS;
}	