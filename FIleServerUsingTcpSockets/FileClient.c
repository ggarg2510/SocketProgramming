#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
/***********  Client APIs  ************

# int socket(int protocolFamily, int type, int protocol)
	@Params: ProtocolFamily = PF_INET;
			 type = SOCK_STREAM;
			 protocol = IPPROTO_TCP
			 
	@Return: -1 = Failure				
			 Non-Negative = SocketDescriptor

# int connect(int socket, struct sockaddr *foreignAddress, unsigned int addressLength)
	@Params: socket = SocketDescriptor;
			 foreignAddress = pointer to struct sockaddress_in;
			 addressLength = sizeof(struct sockaddre_in);
			 
	@Return: ????

# int send(int socket, const void *msg, unsigned int msgLength, int flag)
	@Params: socket = SocketDescriptor;
			 *msg = pointer to the message;
			 msgLength = Number of bytes to be send;
			 flag = 0, fo default functionality;
			 
# int recv(int socket, void *recvBuffer, unsigned int msgLength, int flag)
	@Params: socket = SocketDescriptor;
			 *msg = pointer to the buffer where msg needs to be copy;
			 msgLength = Number of bytes that can be receive at once;
			 flag = 0, fo default functionality;
			 
	
			 
# int close(int socket)
	@Params: SocketDescriptor;
	
	@Return: 0 = success;
			 -1 = failure;


# 	struct sockaddr
	{
		unsigned short sa_family;
		char sa_data[14];
	};
	
#	struct sockaddr_in	
	{
		unsigned short sin_family;
		unsigned short sin_port;
		struct in_addr sin_addr;
		char sin_zero[8];
	};
	
# 	struct in_addr
	{
		unsigned long s_addr;
	};		
		
*/

#define RECV_BUFF_SIZE	500
void main()
{
	int SocketDescriptor;
	struct sockaddr_in serverAddress;
	unsigned char buff[100];
	unsigned int buffLen;
	printf("Enter the command:");
	memset(buff, 0, sizeof(buff));
	fgets(buff, sizeof(buff), stdin);
	printf("%s", buff);
	buffLen=strlen(buff)-1;
	buff[buffLen]=0;
	printf("\n%s:%d\n", buff, buffLen);

	if((SocketDescriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		printf("Cannot create socket\n");
		return;
	}
	
	memset(&serverAddress,0,sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(9100);
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");	
	
	if(connect(SocketDescriptor, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
		printf("Cannot connect to Server\n");
	}
	else
	{
		if(send(SocketDescriptor, buff, buffLen, 0) != buffLen)
		{
			printf("Error in transmission\n");
		}
		else
		{
			signed int NoOfBytesRecvd=0, TotalNoOfBytesRecvd=0;
			char *recvBuff = (char *)malloc((TotalNoOfBytesRecvd+RECV_BUFF_SIZE+1)*sizeof(char));
NoOfBytesRecvd = recv(SocketDescriptor, recvBuff+TotalNoOfBytesRecvd, RECV_BUFF_SIZE, 0);
				TotalNoOfBytesRecvd = TotalNoOfBytesRecvd + NoOfBytesRecvd;
//			while((NoOfBytesRecvd = recv(SocketDescriptor, recvBuff+TotalNoOfBytesRecvd, RECV_BUFF_SIZE, 0)) > 0)
//			{
//				TotalNoOfBytesRecvd = TotalNoOfBytesRecvd + NoOfBytesRecvd;
//				*(recvBuff+TotalNoOfBytesRecvd) = 0;
//				recvBuff = realloc(recvBuff, (TotalNoOfBytesRecvd+RECV_BUFF_SIZE+1)*sizeof(char));
//			}
			recvBuff = realloc(recvBuff, (TotalNoOfBytesRecvd+RECV_BUFF_SIZE)*sizeof(char));
			*(recvBuff+TotalNoOfBytesRecvd) = 0;
			printf("%s", recvBuff);
			free(recvBuff);
		}
	}

	close(SocketDescriptor);
};
