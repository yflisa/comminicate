#include <WINSOCK2.H>
#include <STDIO.H>
#pragma  comment(lib,"ws2_32.lib")
typedef struct tagRtpHead
{
	unsigned char  byVersion;
	unsigned char  byPadding;
	unsigned char  byExtension;
	unsigned char  byCC;
	unsigned char  byMark;
	unsigned char  byPT;
	unsigned short wSeq;
	unsigned int   dwTime;
	unsigned int   dwSSRC;
	unsigned int   dwCFG;
	unsigned int   dwExtLen;
}TRtpHead;

static DWORD WINAPI TCPWorkThread(int *nClient)
{
	printf("tcp Thread create success! nClient = %d\n", *nClient);

    int nRet = -1;

	//连接
	sockaddr_in tSerAddr;
	memset(&tSerAddr, 0, sizeof(tSerAddr));
    tSerAddr.sin_family = AF_INET;
    tSerAddr.sin_port = htons(8888);
    tSerAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); 	
	if((nRet = connect(*nClient, (sockaddr *)&tSerAddr, sizeof(tSerAddr))) < 0)
	{
		nRet = WSAGetLastError();
		printf("connect error ret = %d\n", nRet);
        closesocket(*nClient);
		getchar();
		getchar();
        return -1;
	}

	//设为非阻塞
	unsigned long lRet = 1;
	if((nRet = ioctlsocket(*nClient, FIONBIO, &lRet)) < 0)
	{

		printf("set socket error!\n");
		closesocket(*nClient);
		WSACleanup();
		return -1;
	}

	long nSize = 1;
    char achRecData[1024];
	memset(achRecData, 0, sizeof(achRecData));
	struct timeval tm;
	memset(&tm, 0, sizeof(tm));
	tm.tv_sec = 5;
	tm.tv_usec = 0;
	fd_set dwRset;

	FD_ZERO(&dwRset);
	FD_SET(*nClient, &dwRset);
	bool finish = false;
	while(1)
	{
		if((nRet = select(64, NULL, &dwRset, NULL, &tm)) < 0)
		{
			printf("select socket error!\n");
			closesocket(*nClient);
			WSACleanup();
			getchar();
			getchar();
			return -1;
		}
		else if(nRet == 0)
		{
			printf("select timeout, running\n");
			continue;
		}

		if((nRet = FD_ISSET(*nClient, &dwRset)))
		{
			printf("fd_isset nclient\n");
			FILE *nFile= fopen("b", "wbx");
			//strncpy(achRecData, "fanyun", strlen("fanyun"));
			//fwrite(achRecData, 1, strlen("fanyun"), nFile);
			nSize = recv(*nClient, achRecData, 1024, 0);
			if(nSize < 0)
			{
				nRet = WSAGetLastError();
				printf("recv socket error! 111 nRet = %d\n", nRet);
				closesocket(*nClient);
				WSACleanup();
				getchar();
				getchar();
				return -1;
			}

			while(nSize)
			{
				if(nSize != 1024)
				{
					if((nRet = fwrite(achRecData, 1, nSize, nFile))  < 0)
					{
						finish = true;
						//fclose(nFile);
						printf("nRet < 0\n");
						break;
					}
					finish = true;
					printf("writing file to finish\n");
					fclose(nFile);
					break;
				}
				else
				{
					nRet = fwrite(achRecData, 1, 1024, nFile);
					nSize = recv(*nClient, achRecData, 1024, 0);
				}
			}

			if(finish)
			{
				break;
			}
		}
	}

	getchar();
	getchar();
	return 0;
}

int ParseRtpPacket(char *pbyBuffer, int nLen, TRtpHead *ptRtpHead)
{
	if(NULL == pbyBuffer || 0 == nLen)
	{
		return -1;
	}

	ptRtpHead->byVersion    = pbyBuffer[0] & 0x80;
	ptRtpHead->byPadding    = pbyBuffer[0] & 0x20;
	ptRtpHead->byExtension  = pbyBuffer[0] & 0x10;
	ptRtpHead->byCC         = pbyBuffer[0] & 0xf;
	ptRtpHead->byMark       = pbyBuffer[1] & 0x80;
	ptRtpHead->byPT         = pbyBuffer[1] & 0x7f;
	ptRtpHead->wSeq         = pbyBuffer[2] << 8;
	ptRtpHead->wSeq        |= pbyBuffer[3];
	ptRtpHead->dwTime       = pbyBuffer[4] << 8;
	ptRtpHead->dwTime      |= pbyBuffer[5];
	ptRtpHead->dwTime     <<=16;
	ptRtpHead->dwTime      |= pbyBuffer[6] << 8;
	ptRtpHead->dwTime      |= pbyBuffer[7];
	ptRtpHead->dwSSRC       = pbyBuffer[8] << 8;
	ptRtpHead->dwSSRC      |= pbyBuffer[9];
	ptRtpHead->dwSSRC     <<=16;
	ptRtpHead->dwSSRC      |= pbyBuffer[10] << 8;
	ptRtpHead->dwSSRC      |= pbyBuffer[11];
	ptRtpHead->dwCFG        = pbyBuffer[12] << 8;
	ptRtpHead->dwCFG       |= pbyBuffer[13];
	ptRtpHead->dwExtLen		= pbyBuffer[14] << 8;
	ptRtpHead->dwExtLen	   |= pbyBuffer[15];

	return 0;
}

int CreateTcpClient()
{
    WORD wSockVersion = MAKEWORD(2,2);
	int nRet = 0;
    WSADATA tData; 

	memset(&tData, 0, sizeof(tData));
    if((nRet = WSAStartup(wSockVersion, &tData)) != 0)
    {
		nRet = WSAGetLastError();
		printf("send error ret = %d\n", nRet);
		getchar();
		getchar();        
		return -1;
    }
	printf("start up nRet = %d\n", nRet);
    int nClient = -1;
    if((nClient = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
		nRet = WSAGetLastError();
		printf("send error ret = %d\n", nRet);
		getchar();
		getchar();		
		return -1;
    }
	printf("socket nClient = %d\n", nClient);

	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE )TCPWorkThread, &nClient, 0, NULL);
	WaitForSingleObject(handle, -1);

	getchar();
	getchar();
    return 0;

}
static DWORD WINAPI UDPWorkThread(int *nClient)
{	
	printf("udpclient create thread success! nClient=%d\n", *nClient);
    sockaddr_in tSin;
	memset(&tSin, 0, sizeof(tSin));
    tSin.sin_family = AF_INET;
    tSin.sin_port = htons(8888);
    tSin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int len = sizeof(tSin);

	
//    char *pchSendData = "client data\n";
//    int nRet = sendto(*nClient, pchSendData, strlen(pchSendData), 0, (sockaddr *)&tSin, len);
//	if(nRet < 0)
//	{
//		nRet = WSAGetLastError();
//		printf("sendto error nRet=%d\n", nRet);
//		return -1;
//	}
	bool finish = true;
	long nSize = 0;
	int nRet = -1;
    char achRecvData[1025];
	memset(achRecvData, 0, sizeof(achRecvData));
	struct timeval tm;
	memset(&tm, 0, sizeof(tm));
	tm.tv_sec = 5;
	tm.tv_usec = 0;	
	fd_set adwReadFd;
	FD_ZERO(&adwReadFd);
	FD_SET(*nClient, &adwReadFd);
	
	while(1)
	{
		if((nRet = select(64, NULL, &adwReadFd, NULL, &tm)) < 0)
		{
			printf("select socket error!\n");
			closesocket(*nClient);
			WSACleanup();
			getchar();
			getchar();
			return -1;
		}
		else if(nRet == 0)
		{
			printf("select timeout, running\n");
			continue;
		}

		if((nRet = FD_ISSET(*nClient, &adwReadFd)))
		{
			printf("fd_isset nclient\n");
			FILE *nFile= fopen("b", "wbx");
			
			nSize = recvfrom(*nClient, achRecvData, 1025, 0, (sockaddr *)&tSin, &len);
			if(nSize < 0)
			{
				nRet = WSAGetLastError();
				printf("recvfrom socket error! 111 nRet = %d\n", nRet);
				closesocket(*nClient);
				WSACleanup();
				getchar();
				getchar();
				return -1;
			}

			while(nSize)
			{
				if(nSize != 1024)
				{
					if((nRet = fwrite(achRecvData, 1, nSize, nFile))  < 0)
					{
						finish = true;
						//fclose(nFile);
						printf("nRet < 0\n");
						break;
					}
					finish = true;
					printf("writing file to finish\n");
					fclose(nFile);
					break;
				}
				else
				{
					nRet = fwrite(achRecvData, 1, 1025, nFile);
					nSize = recvfrom(*nClient, achRecvData, 1025, 0, (sockaddr *)&tSin, &len);;
				}
			}

			if(finish)
			{
				break;
			}
		}

//		nRet = recvfrom(*nClient, achRecvData, 1024, 0, (sockaddr *)&tSin, &len);
//		if(nRet > 0)
//		{
//			achRecvData[nRet] = 0x00;
//		}
//		else if(nRet < 0)
//		{
//			nRet = WSAGetLastError();
//			printf("recvfrom error nRet=%d\n", nRet);
//			return -1;
//		}
//		else
//		{
//			printf("break\n");
//			break;
//		}
//		printf("recvfrom success\n");
	}

	return 0;
}

int CreateUdpClient()
{
    WORD wSockVersion = MAKEWORD(2,2);
    WSADATA tData; 
	memset(&tData, 0, sizeof(tData));
    if(WSAStartup(wSockVersion, &tData) != 0)
    {
        return 0;
    }
    int nClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
    sockaddr_in tSin;
	memset(&tSin, 0, sizeof(tSin));
    tSin.sin_family = AF_INET;
    tSin.sin_port = htons(8888);
    tSin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int len = sizeof(tSin);

    
	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE )UDPWorkThread, &nClient, 0, NULL);
	WaitForSingleObject(handle, -1);
	
//    char achRecvData[1412];
//	memset(achRecvData, 0, sizeof(achRecvData));
//	while(1)
//	{
//	    int nRet = recvfrom(nClient, achRecvData, sizeof(achRecvData), 0, (sockaddr *)&tSin, &len);
//	    if(nRet > 0)
//	    {
//	        achRecvData[nRet] = 0x00;
//	    }
//		else if(nRet < 0)
//		{
//			nRet =WSAGetLastError();
//			printf("recvfrom error nRet=%d\n", nRet);
//			return -1;
//		}
//		else
//		{
//			printf("break\n");
//			break;
//		}

//		TRtpHead tRtpHead;
//		memset( &tRtpHead, 0, sizeof(TRtpHead) );
//		nRet = ParseRtpPacket(achRecvData, 1412, &tRtpHead);
//		if(0 != nRet)
//		{
//			printf("Parse rtp packet failed\n");
//			return -1;
//		}
//		printf("version = %d, Padding = %d, Extension = %d, CC = %d, Mark = %d, PayloadType = %d, Seq = %d, TimeStamp = %d, SSRC = 0x%x, CFG = 0x%x, ExtLen = %d\n", 
//		tRtpHead.byVersion, tRtpHead.byPadding, tRtpHead.byExtension,tRtpHead.byCC, tRtpHead.byMark, 
//		tRtpHead.byPT, tRtpHead.wSeq, tRtpHead.dwTime, tRtpHead.dwSSRC, tRtpHead.dwCFG, tRtpHead.dwExtLen);
//	}

//    closesocket(nClient);
//    WSACleanup();
    return 0;

}

int main()
{
	int  nProtoFlag         = -1;

	printf( "Please Enter 0(using the TCP protocol) OR 1(using the UDP protocol):" );
	scanf( "%d", &nProtoFlag );
	while( (0 != nProtoFlag) && (1 != nProtoFlag) )
	{
		printf( "Please Enter 0(using the TCP protocol) OR 1(using the UDP protocol):" );
	}
	int nRet = -1;
	switch( nProtoFlag )
	{
		case 0:
			nRet = CreateTcpClient();
			if(nRet != 0)
			{
				printf("CreateTcpClient Failed!\n");
				return -1;
			}
			break;
		case 1:
			nRet = CreateUdpClient();
			if(nRet != 0)
			{
				printf("CreateUdpClient Failed!\n");
				return -1;
			}
			break;
		default:
			;
	}
	printf("SUCCESS\n");
	
	getchar();
	getchar();
	return 0;
}

