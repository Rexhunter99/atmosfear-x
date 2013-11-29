#pragma once

class Network
{
public:

	Network();
	~Network();

	bool ConnectTCP( const char* p_address, const char* p_port = "80" );
	bool ConnectUDP( const char* p_address, const char* p_port );

	void Disconnect( );

private:


};
