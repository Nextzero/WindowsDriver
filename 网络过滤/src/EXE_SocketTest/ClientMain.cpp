#include<iostream>
#include <Winsock2.h>
#pragma comment(lib,"WS2_32")
using namespace std;
void main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 1, 1 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		return;
	}

	if ( LOBYTE( wsaData.wVersion ) != 1 ||
		HIBYTE( wsaData.wVersion ) != 1 ) {
			WSACleanup( );
			return; 
	}
	SOCKET fd;
	struct sockaddr_in addr;
	char buf[100]="client:nihao";

	printf("before socket\n");getchar();
	fd=socket(AF_INET,SOCK_STREAM,0);
	printf("after socket\n");getchar();

	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=inet_addr("192.168.20.10");
	addr.sin_port=htons(6010);

	printf("before connect\n");getchar();
	connect(fd,( sockaddr *)&addr,sizeof(addr));
	printf("after connect\n");getchar();

	printf("before send\n");getchar();
	send(fd,buf,100,0);
	printf("after send\n");getchar();

	printf("before reve\n");getchar();
	recv(fd,buf,100,0);
	printf("after revc\n");getchar();

	cout<<buf<<endl;
	printf("before closecocket\n");getchar();
	closesocket(fd);
	printf("after closecocket\n");getchar();
}