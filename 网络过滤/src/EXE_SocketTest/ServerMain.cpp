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
		HIBYTE( wsaData.wVersion ) != 1 ) 
	{
			WSACleanup( );
			return; 
	}

	SOCKET fd,fd_client;
	struct sockaddr_in addr_server,addr_client;
	char buf[100];

	printf("before socket\n");getchar();
	fd=socket(AF_INET,SOCK_STREAM,0); 
	addr_server.sin_family=AF_INET;
	addr_server.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
	addr_server.sin_port=htons(6010);
	printf("after socket\n");getchar();

	printf("before bind\n");getchar();
	bind(fd,(sockaddr *)&addr_server,sizeof(addr_server));
	printf("after bind\n");	getchar();

	printf("befor linsten\n");getchar();
	listen(fd,5);
	printf("after linsten\n");getchar();

	int len=sizeof(sockaddr);
	while(1)
	{
		printf("before accpet\n");getchar();
		fd_client=accept(fd,(sockaddr *)&addr_client,&len);
		printf("after accept\n");getchar();

		printf("before recv\n");getchar();
		recv(fd_client,buf,100,0); 
		printf("after reve\n");getchar();

		cout<<buf<<endl;

		printf("before send\n");getchar();
		send(fd_client,"server:nihao",100,0); //·¢ËÍÊý¾Ý
		printf("after send\n");	getchar();

		printf("before closesocket\n");getchar();
		closesocket(fd_client);
		printf("after closesocket\n");getchar();
		printf("\n\n");
	}
}