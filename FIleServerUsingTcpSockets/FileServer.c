#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
//#include <ctypes.h>
/*
		===============	
		Standard Syntax
		===============	

1) File *fPtr;   //Declaring File Pointer. For every handle, file Descriptor like this will be used
2) fPtr = fopen("<Qualified_path/filename>", "<Mode>");
3) fclose(fPtr);
4) getc(fPtr) returns single character from file.
5) fscanf(fPtr,"<Variable specifier(s)>", variable(s));
6) 

		===============
		File Open Modes
		===============
-------------------------------------------------------------------------------------
|  CODE   | DESCRIPTION     														|
-------------------------------------------------------------------------------------
|  r      | Read Only (Text File), Sets pointer at the last location				|
-------------------------------------------------------------------------------------
|  w      | Write Only (Text File), Sets pointer at the last location				|
-------------------------------------------------------------------------------------
|  a      | Append Only (Text File), Sets pointer at the last location				|
-------------------------------------------------------------------------------------
|  r+     | Read Only (Text File), Sets pointer at the last location				|
-------------------------------------------------------------------------------------
|  w+     | Write Only (Text File), Sets pointer at the last location				|
-------------------------------------------------------------------------------------
|  a+     | Append Only (Text File), Sets pointer at the last location				|
-------------------------------------------------------------------------------------
|  rb     | Read Only (Binary File)							|
-------------------------------------------------------------
|  wb     | Write Only (Binary File)						|
-------------------------------------------------------------
|  ab     | Append Only (Binary File)						|
-------------------------------------------------------------
*/

struct student
{
	unsigned short Id;
	unsigned int RollNo;
	unsigned char Name[16];
	unsigned char Dob[16];
};
//struct student	tblStudent;
char RecSelect(unsigned int recNo, struct student *stud);
char RecDelete(unsigned int recNo, struct student *stud);
char RecUpdate(unsigned int recNo, struct student *stud);
char RecInsert(unsigned int recNo, struct student *stud);


struct funcMap
{
	unsigned char key[7];
	char (*fPr)(unsigned int, struct student *);
};
const struct funcMap	tblFuncMap[]={
	{"SELECT", RecSelect,},		//SYNTAX:  SELECT x , x => 0,1,2,3,4,....
	{"INSERT", RecInsert,},
	{"UPDATE", RecUpdate,},
	{"DELETE", RecDelete,}
};	

char FileName[15]="students.bin";
unsigned char funcMapi;
char *sendBuff;
FILE *fPtr;


char RecSelect(unsigned int recNo, struct student *stud)
{
	if((fPtr = fopen(FileName, "r")) == NULL)
	{
		sprintf(sendBuff, "Error in reading %s file\n", FileName);
		return -1;
	}
	if(recNo != 0)
	{
		fseek(fPtr, (recNo-1)*sizeof(struct student), SEEK_SET);
		fread(stud, sizeof(struct student), 1, fPtr);
		sendBuff = realloc(sendBuff, 50);
		sprintf(sendBuff, "%d,%d,%s,%s\n", stud->Id, stud->RollNo, stud->Name, stud->Dob);
	}
	else
	{
		unsigned long address=0,i=1,BuffLen;
		
		fseek(fPtr, address, SEEK_SET);
		fread(stud, sizeof(struct student), 1, fPtr);
		while(!feof(fPtr))
		{
			sendBuff = (char *)realloc(sendBuff, i*60);i++;
			BuffLen = strlen(sendBuff);
			sprintf(sendBuff+BuffLen, "%d,%d,%s,%s\n", stud->Id, stud->RollNo, stud->Name, stud->Dob);
			address += sizeof(struct student);
			fseek(fPtr, address, SEEK_SET);
			fread(stud, sizeof(struct student), 1, fPtr);
		}	
		sendBuff = (char *)realloc(sendBuff, i*60);
		BuffLen = strlen(sendBuff);
		sprintf(sendBuff+BuffLen, "Total Number of Records:%d\n", (unsigned int)i-1);
	}
	fclose(fPtr);
	return 0;
}
char RecDelete(unsigned int recNo, struct student *stud)
{
	if((fPtr = fopen(FileName, "r+")) == NULL)
	{
		sprintf(sendBuff,"Error in reading %s file\n", FileName);
		return -1;
	}
	fseek(fPtr, (recNo-1)*sizeof(struct student), SEEK_SET);
	memset(stud, 0, sizeof(struct student));
	fwrite((unsigned char*)stud, sizeof(struct student), 1, fPtr);
	fclose(fPtr);
	sprintf(sendBuff, "Record Deleted\n");
	return 0;
}
char RecUpdate(unsigned int recNo, struct student *stud)
{
	struct student loc_stud;
	if((fPtr = fopen(FileName, "r+")) == NULL)
	{
		sprintf(sendBuff,"Error in reading %s file\n", FileName);
		return -1;
	}	
	
	fseek(fPtr, (recNo-1)*sizeof(struct student), SEEK_SET);
	fread((unsigned char*)&loc_stud, sizeof(struct student), 1, fPtr);
	if(loc_stud.Id == recNo)
	{		
		fseek(fPtr, (recNo-1)*sizeof(struct student), SEEK_SET);
		fwrite((unsigned char*)stud, sizeof(struct student), 1, fPtr);
		fclose(fPtr);
		sprintf(sendBuff, "success\n");
		return 0;
	}
	else
	{
		sprintf(sendBuff,"No Previous Record found\n");
	}
	fclose(fPtr);
	return -1;
}
char RecInsert(unsigned int recNo, struct student *stud)
{	
	if((fPtr = fopen(FileName, "r+")) == NULL)
	{
		fPtr = fopen(FileName, "w");
		fclose(fPtr);
		fPtr = fopen(FileName, "r+");
	}
	fseek(fPtr, (recNo-1)*sizeof(struct student), SEEK_SET);
	fwrite((unsigned char*)stud, sizeof(struct student), 1, fPtr);
	fclose(fPtr);
	sprintf(sendBuff, "success\n");
	return 0;
}

void toUpper(unsigned char *s)
{
	while(*s!='\0')
	{
		if(*s>='a' && *s<='z')	*s = *s-32;
		s++;
	}
}
void processIxCmd(char *ixMsg)
{
	struct student	tblStudent;

	if(*(ixMsg+6) == ' ')
	{
		char *cmdMsg = (char*)malloc(6 * sizeof(char));
		memcpy(cmdMsg, ixMsg, 6);
		toUpper(cmdMsg);

		for(funcMapi=0; funcMapi<=3; funcMapi++)
		{
			if(strcmp(tblFuncMap[funcMapi].key,cmdMsg) == 0)
			{
				printf("Command Found : %s\n", cmdMsg);
			
				char *ptrI;
				char j=0;
				memset((unsigned char*)&tblStudent, 0, sizeof(struct student));

				ptrI = strtok(ixMsg+7, ",");	
				while(ptrI != NULL)
				{
					if(j==0) 		tblStudent.Id = atoi(ptrI);
					else if(j==1) 	tblStudent.RollNo = atoi(ptrI);
					else if(j==2) 	memcpy(tblStudent.Name, ptrI, strlen(ptrI));
					else if(j==3) 	memcpy(tblStudent.Dob, ptrI, strlen(ptrI));
					j++;
					ptrI = strtok(NULL, ",");
				}
				printf("Date Recvd : %d, %d, %s, %s\n",(unsigned int)tblStudent.Id, (unsigned int)tblStudent.RollNo, tblStudent.Name, tblStudent.Dob);
				(tblFuncMap[funcMapi].fPr)(tblStudent.Id, &tblStudent);
				break;
			}
		}
		free(cmdMsg);
		if(funcMapi == 4)
		{
			strcat(sendBuff, "Command not matched\n");
		}
	}
	else
	{
		strcat(sendBuff, "Invalid command\n");
	}
//	free(ixMsg);

return;
}
#define RECV_BUFF_SIZE	50

void main()
{
	printf("Running FileServer...\n");
	
	int localSock,ClientSock;
	int localPortNumber=9100;
	struct sockaddr_in	localAddress,clientAddress;
	int clientAddressLen = sizeof(clientAddress);
		
	if((localSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		printf("Cannot open socket\n");
		return;
	}
	printf("Socket open @ %d\n", localSock);
	
	memset(&localAddress, 0, sizeof(localAddress));
	localAddress.sin_family = AF_INET;
	localAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	localAddress.sin_port = htons(localPortNumber);
	
	if(bind(localSock, (struct sockaddr*)&localAddress, sizeof(localAddress)) < 0)
	{
		printf("Binding failed\n");
	}
	else
	{
		printf("Binding done @ %d\n", localSock);
		if(listen(localSock, 1) < 0)
		{
			printf("Listening failed\n");
		}
		else
		{
			printf("Listening on %d ...\n", localPortNumber);
			while(1)
			{
				
				if((ClientSock = accept(localSock, (struct sockaddr*)&clientAddress, &clientAddressLen)) < 0)
				{
					printf("accept() failed\n");
				}
				else
				{	
					signed int NoOfBytesRecvd=0, TotalNoOfBytesRecvd=0;
					char *recvBuff = (char *)malloc((TotalNoOfBytesRecvd+RECV_BUFF_SIZE+1)*sizeof(char));
//					while((NoOfBytesRecvd = recv(ClientSock, recvBuff+TotalNoOfBytesRecvd, RECV_BUFF_SIZE, 0)) > 0)
					NoOfBytesRecvd = recv(ClientSock, recvBuff+TotalNoOfBytesRecvd, RECV_BUFF_SIZE, 0);
					TotalNoOfBytesRecvd = TotalNoOfBytesRecvd + NoOfBytesRecvd;
					*(recvBuff+TotalNoOfBytesRecvd) = 0;
//					while((NoOfBytesRecvd = read(ClientSock, recvBuff+TotalNoOfBytesRecvd, RECV_BUFF_SIZE)) > 0)
	//				{
		//				TotalNoOfBytesRecvd = TotalNoOfBytesRecvd + NoOfBytesRecvd;
			//			*(recvBuff+TotalNoOfBytesRecvd) = 0;
				//		recvBuff = realloc(recvBuff, (TotalNoOfBytesRecvd+RECV_BUFF_SIZE+1)*sizeof(char));
					//}
					
//					recvBuff = realloc(recvBuff, (TotalNoOfBytesRecvd+RECV_BUFF_SIZE)*sizeof(char));
	//				*(recvBuff+TotalNoOfBytesRecvd) = 0;
					printf("Incoming connection from %s: %s\n", inet_ntoa(clientAddress.sin_addr), recvBuff);
					//sleep(2);
					
					sendBuff = (char *)malloc(50*sizeof(char));
					processIxCmd(recvBuff);
					
//					signed int NoOfBytesSent=0, TotalNoOfBytesSent=0;
					signed int bytesToSend = strlen(sendBuff), bytesSend=0;
					printf("Bytes to be send %d\n", bytesToSend);
					printf("%s\n",sendBuff);
					bytesSend = send(ClientSock, sendBuff, bytesToSend, 0);// != bytesToSend) printf("Response sent\n");
					printf("Bytes sent %d\n", bytesSend);
					free(sendBuff);
					free(recvBuff);
				}		
			}
		}
	
	}
							close(ClientSock);
				close(localSock);

}

/*
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

pthread_t tId[2];
int counter=0;
pthread_mutex_t mLock;


void *ThreadTask(void *arg)
{
	pthread_mutex_lock(&mLock);
	counter+=1;
	printf("Thread %d created\n", counter);
	sleep(2);
	printf("Thread %d destroyed\n", counter);
	pthread_mutex_unlock(&mLock);
	return NULL;
}

void main()
{
	if(pthread_mutex_init(&mLock, NULL) == -1)	printf("Error in initializing Mutex Lock\n");

	if(pthread_create(&tId[0], NULL, ThreadTask, NULL) != 0) printf("Error in thread %d creation\n", (int)tId[0]);
	if(pthread_create(&tId[1], NULL, ThreadTask, NULL) != 0) printf("Error in thread %d creation\n", (int)tId[1]);
	pthread_join(tId[0], NULL);
	pthread_join(tId[1], NULL);
	pthread_mutex_destroy(&mLock);
	exit(0);
	
}

	RecInsert(1,23,"fggg","ffff");
	RecInsert(2,23,"fggg","ffff");
	RecInsert(31,23,"fggg","ffff");
	RecDelete(31);
	sleep(2);
	RecSelect(1);
	sleep(2);
	RecSelect(0);
	sleep(2);
	RecUpdate(1,23,"Modifiedfggg","ffff");
	RecSelect(1);
	RecSelect(2);
	
*/

