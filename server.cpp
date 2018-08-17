#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#define WM_SOCKET (WM_USER + 1)

static int TCPWorkerThread(int *nSerTcpSocket)
{
	printf("Workerthread Here nSerTcpSocket=%d\n", *nSerTcpSocket);

	FILE *pFile = NULL;
	long fileLen = 0;
 	char *pchSendData = NULL;
	const char *pchFileName = "a";	
	int nFileLen = 0;
	int nReadNum = 0;
	
 	int nServer = -1;
	unsigned long lRet = 1;	
	int nRet = -1;
	sockaddr_in tRemoteAddr;
	memset(&tRemoteAddr, 0, sizeof(tRemoteAddr));
	struct timeval tm;
	memset(&tm , 0, sizeof(tm));
	fd_set adwFdRead;
    int nAddrlen = sizeof(tRemoteAddr);
	tm.tv_sec = 5;
	tm.tv_usec = 0;

	//select 阻塞
	while(1)
	{
		FD_ZERO(&adwFdRead);
		FD_SET(*nSerTcpSocket, &adwFdRead);

		nRet = select(64, &adwFdRead, NULL, NULL,&tm);
		if(nRet < 0)
		{
			nRet = WSAGetLastError();
			printf("select error! nRet = %d\n", nRet);	
			getchar();
			getchar();
			return -1;
		}
		else if(nRet == 0)
		{
			printf("select time out\n");
			continue;
		}

		if(FD_ISSET(*nSerTcpSocket, &adwFdRead) > 0)
		{
			printf("accept client\n");
			nServer = accept(*nSerTcpSocket, (SOCKADDR *)&tRemoteAddr, &nAddrlen);
			FD_SET(nServer, &adwFdRead);
			ioctlsocket(nServer, FIONBIO, &lRet);
		}
		if(FD_ISSET(nServer, &adwFdRead) > 0)
		{
			bool reading = true;
			printf("data communicat\n");
			if((pFile = fopen(pchFileName, "rb")) == NULL)
			{
				printf("a does not exit\n");
				getchar();
				return -1;
			}
			//查看包长度
			fseek(pFile, 0L, SEEK_END);
			fileLen = ftell(pFile);
			printf("fileLen = %d\n",fileLen);
			fseek(pFile, 0L, SEEK_SET);
			pchSendData = (char*)malloc(1024);
			while(!feof(pFile))
			{

				nReadNum = fread(pchSendData, 1, 1024, pFile);
				if(nReadNum < 1024)
				{
					reading = false;
					send(nServer, pchSendData, nReadNum, 0);
					break;
				}
				else
				{
					send(nServer, pchSendData, 1024, 0);
				}
			}

			if(!reading)
			{
				break;
			}
		}
	}
	
	getchar();
	getchar();
	return 0;
}
int CreateTcpServer()
{
    //创建套接字
    int nSerTcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(nSerTcpSocket == INVALID_SOCKET)
    {
        printf("socket error !");
        return -1;
    }

    //绑定IP和端口
    sockaddr_in tSin;
	memset(&tSin, 0, sizeof(tSin));
    tSin.sin_family = AF_INET;
    tSin.sin_port = htons(8888);
    tSin.sin_addr.S_un.S_addr = INADDR_ANY; 
    if(bind(nSerTcpSocket, (LPSOCKADDR)&tSin, sizeof(tSin)) == SOCKET_ERROR)
    {
        printf("bind error !");
		return -1;
    }

    //开始监听
    if(listen(nSerTcpSocket, 5) == SOCKET_ERROR)
    {
        printf("listen error !");
        return -1;
    }

	fd_set adwFdRead;
	FD_ZERO(&adwFdRead);
	FD_SET(nSerTcpSocket, &adwFdRead);

	int nReady = -1;
	int nConnFd = -1;
	int nMaxFd = nSerTcpSocket;
	sockaddr_in tRemoteAddr;
	memset(&tRemoteAddr, 0, sizeof(tRemoteAddr));
    int nAddrlen = sizeof(tRemoteAddr);
	struct timeval timeout = {10, 0};
	DWORD dwThread = 0;

	if(	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TCPWorkerThread, &nSerTcpSocket, 0, &dwThread) != 0)
	{
		Sleep(100000000000000);
		printf("thread ceate error!\n");
		return -1;
	}

    return 0;

}  

static int UDPWorkerThread(int *nSerUdpSocket)
{
	printf("udpWorkerThread Here nSerUdpSocket = %d\n", *nSerUdpSocket);

	bool reading = true;
	FILE *pFile = NULL;
	int nReadNum = 0;
	int nRet = -1;
	const char *pchFileName = "a";	
	long fileLen = 0;

	struct timeval timeout;
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	sockaddr_in tRemoteAddr;
	memset(&tRemoteAddr, 0, sizeof(tRemoteAddr));
	int nAddrLen = sizeof(tRemoteAddr); 

	char achSendData[1024];
	memset(&achSendData, 0, sizeof(achSendData));

	fd_set adwFdRead;
	FD_ZERO(&adwFdRead);
	FD_SET(*nSerUdpSocket, &adwFdRead);

	while(1)
	{
		
		if((nRet = select(64, NULL, &adwFdRead, NULL,&timeout)) < 0)
		{
			nRet = WSAGetLastError();
			printf("select error! nRet = %d\n", nRet);	
			getchar();
			getchar();
			return -1;
		}
		else if(nRet == 0)
		{
			printf("select time out\n");
			continue;			
		}

		if(FD_ISSET(*nSerUdpSocket, &adwFdRead) > 0)
		{
			bool reading = true;
			printf("data communicate\n");		
			if((pFile = fopen(pchFileName, "rb")) == NULL)
			{
				printf("a does not exit\n");
				getchar();
				return -1;
			}
			//查看包长度
			fseek(pFile, 0L, SEEK_END);
			fileLen = ftell(pFile);
			printf("fileLen = %d\n",fileLen);
			fseek(pFile, 0L, SEEK_SET);

			nRet = sendto(*nSerUdpSocket, achSendData, strlen(achSendData), 0, (sockaddr *)&tRemoteAddr, nAddrLen);
			while(!feof(pFile))
			{

				nReadNum = fread(achSendData, 1, 1024, pFile);
				if(nReadNum < 1024)
				{
					reading = false;
					nRet = sendto(*nSerUdpSocket, achSendData, strlen(achSendData), 0, (sockaddr *)&tRemoteAddr, nAddrLen);
					break;
				}
				else
				{
					nRet = sendto(*nSerUdpSocket, achSendData, 1024, 0, (sockaddr *)&tRemoteAddr, nAddrLen);
				}
			}

			if(!reading)
			{
				break;
			}

		}
	}
	return 0;
}
int CreateUdpServer()
{
    WSADATA tData;
	memset(&tData, 0, sizeof(tData));
    WORD wSockVersion = MAKEWORD(2,2);
    if(WSAStartup(wSockVersion, &tData) != 0)
    {
        return 0;
    }

    int nSerUdpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); 
    if(nSerUdpSocket == INVALID_SOCKET)
    {
        printf("socket error !");
        return 0;
    }

    sockaddr_in tSerAddr;
	memset(&tSerAddr, 0, sizeof(tSerAddr));
    tSerAddr.sin_family = AF_INET;
    tSerAddr.sin_port = htons(8888);
    tSerAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    if(bind(nSerUdpSocket, (sockaddr *)&tSerAddr, sizeof(tSerAddr)) == SOCKET_ERROR)
    {
        printf("bind error !");
        closesocket(nSerUdpSocket);
        return -1;
    }

	unsigned long dwThread = 1;
	if( CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UDPWorkerThread, &nSerUdpSocket, 0, &dwThread) != 0)
	{
		Sleep(100000000000000);
		printf("thread ceate error!\n");
		return -1;
	}
	    
//    sockaddr_in tRemoteAddr;
//	memset(&tRemoteAddr, 0, sizeof(tRemoteAddr));
//    int nAddrLen = sizeof(tRemoteAddr); 
//    while (true)
//    {
//        char achRecvData[255]; 
//		memset(&achRecvData, 0, sizeof(achRecvData));
//        int ret = recvfrom(nSerUdpSocket, achRecvData, 255, 0, (sockaddr *)&tRemoteAddr, &nAddrLen);
//        if (ret > 0)
//        {
//            achRecvData[ret] = 0x00;
//        }

//		//打开文件
//        char * pchSendData = NULL;
//		const char * pchFileName = "a";
//		int nFileLen = 0;
//		FILE *pFile = NULL;
//		pFile = fopen(pchFileName, "rb");
//		if(NULL == pFile)
//		{
//			return -1;
//		}
//		fseek(pFile, 0, SEEK_END);
//		nFileLen = ftell(pFile);
//		fseek(pFile, 0, SEEK_SET);
//		pchSendData = (char*)malloc(nFileLen);
//		if(NULL == pchSendData)
//		{
//			return -1;
//		}

//		int nRet = fread(pchSendData, 1, nFileLen, pFile);
//        sendto(nSerUdpSocket, pchSendData, strlen(pchSendData), 0, (sockaddr *)&tRemoteAddr, nAddrLen); 
//		free(pchSendData);
//		pchSendData = NULL;
//		fclose(pFile);
//		pFile = NULL;

//    }
    closesocket(nSerUdpSocket); 
    WSACleanup();
    return 0;

}

int main()
{
	int  nProtoFlag = -1;

	printf( "Please Enter 0(using the TCP protocol) OR 1(using the UDP protocol)" );
	scanf( "%d", &nProtoFlag );
	while( (0 != nProtoFlag) && (1 != nProtoFlag) )
	{
		printf( "Please Enter 0(using the TCP protocol) OR 1(using the UDP protocol)" );
	}
	int nRet = -1;
	    //初始化WSA
    WORD wSockVersion = MAKEWORD(2,2);
    WSADATA tData;
	memset(&tData, 0, sizeof(tData));
    if(WSAStartup(wSockVersion, &tData)!=0)
    {
        return 0;
    }
	switch( nProtoFlag )
	{
		case 0:
			nRet = CreateTcpServer();
			if(nRet != 0)
			{
				printf("CreateTcpServer Failed!\n");
				return -1;
			}
			break;
		case 1:
			nRet = CreateUdpServer();
			if(nRet != 0)
			{
				printf("CreateUdpServer Failed!\n");
				return -1;
			}
			break;
		default:
			break;
	}
	printf("SUCCESS\n");

	getchar();
	getchar();
	return 0;
}

