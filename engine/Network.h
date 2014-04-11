#pragma once

#include <stdint.h>
#include <string>


class HttpRequest
{
	//
	// Spec: http://www.w3.org/TR/XMLHttpRequest/
	//
public:

	enum HttpRequestEnum
	{
		HTTP_GET			= 0,
		HTTP_POST			= 1,

		STATE_NOTINIT		= 0,
		STATE_CONNECTED		= 1,
		STATE_RECEIVED		= 2,
		STATE_PROCESSING	= 3,
		STATE_READY			= 4
	};

	// -- Cancels the current request
	void abort();
	// -- Specifies the type of request, the URL, if the request should be handled asynchronously
	// or not, and other optional attributes of a request
	void open( uint32_t method, std::string url, bool async = false, std::string uname = "", std::string pswd = "" );
	// -- Sends the request off to the server.
	void send( std::string p_post = "" );
	//getAllResponseHeaders();	//Returns header information
	//getResponseHeader();	//Returns specific header information
	//setRequestHeader()	Adds a label/value pair to the header to be sent


	// -- Stores a function (or the name of a function) to be called automatically each time the readyState property changes
	void (*onreadystatechange)(void);
	// -- Returns the response data as a string
	std::string		responseText;
	// -- Holds the status of the XMLHttpRequest
	uint32_t		readyState;
	// -- Returns the response data as XML data
	//?				responseXML;
	// -- Returns the status-number (e.g. "404" for "Not Found" or "200" for "OK")
	uint32_t		status;
	// -- Returns the status-text (e.g. "Not Found" or "OK")
	std::string		statusText;
	// -- Sets the time in milliseconds that processing/fetching can occur before aborting
	uint32_t		timeout;
};

class Network
{
public:
	Network();
	~Network();

	bool init();
	bool shutdown();



};
