
#ifdef USE_NETWORK

#include "Platform.h"
//#include "Network.h"


#if defined( AF_PLATFORM_LINUX )

#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>

#elif defined( AF_PLATFORM_WINDOWS )

#	include <WinSock2.h>
#	include <Ws2tcpip.h>

#endif


enum {
	NETWORK_TCPIP	= 0,
	NETWORK_UDP		= 1,
};


Network::Network()
{
	#if defined( AF_PLATFORM_WINDOWS ) || defined( _WINDOWS ) || defined( _WIN32 )
	if( WSAStartup( WINSOCK_VERSION,&wsaData ) )
	{
		//PrintLog( "WinSock initialization failed!\n" );
		return;
	}
	#endif
}

Network::~Network()
{
	#if defined( AF_PLATFORM_WINDOWS ) || defined( _WINDOWS ) || defined( _WIN32 )
	WSACleanup();
	#endif
}


bool Network::SetupServer( int p_port )
{
	struct addrinfo	hints, *result;

	if ( getaddrinfo( NULL, htons(p_port), &hints, &result ) )
	{
		// error
		return false;
	}

	return true;
}

#endif
