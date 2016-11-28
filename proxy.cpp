#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */
#include <netinet/in.h>
#include <netdb.h>

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <pthread.h>

#include "helper.h"           /*  our own helper functions  */
using namespace std;

#define DEF_PORT          (2007)

#define MAX_LINE           (1000)
const int MAXPENDING = 25;

struct threadArgs {
	int clientSock;
};


vector<string> split_string(string str, char delimiter) {
	vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;
  
  while(getline(ss, tok, delimiter)) {
  	internal.push_back(tok);
  }
  
  return internal;
}

bool fetchServerName(string request, string& ip) {
	int posn = request.find("Host: ");
	if(posn != string::npos) {
		posn += 6;
		while(request[posn] != '\r') {
			if(request[posn] == ':')
				break;
			ip += request[posn];
			posn++;
		}
		return true;
	}
	else
		return false;
}

bool fetchUrlData(string request, string& response) {
	struct sockaddr_in servaddr;
	int serverSock, status = 0;
	string serverName = "";
	unsigned short serverPort = 80;
	struct hostent* server = NULL;
	unsigned long serverIP;
	memset(&servaddr, 0, sizeof(servaddr));
	/*
	A socket to get response from the server
	*/
	serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	/*
	Fetch the server IP from the request
	*/
	if(!fetchServerName(request, serverName)) {
		cerr << "Unable to fetch server's name" << endl;
		return false;
	}
	cout << "Server Name: " << serverName;
	cout << " @ ";
	//serverName = "www.google.com";
	server = gethostbyname(serverName.c_str());
	if (server == NULL) {
    	cerr << "Unable to resolve server's IP Address. Exiting..." << endl;
    	return false;
  	}
	
	
  	char *tmpIP = inet_ntoa(*(struct in_addr *)server->h_addr_list[0]);
	cout  << tmpIP << endl;
	status = inet_pton(AF_INET, tmpIP, (void*) &serverIP);
  	if (status <= 0) return false;
  	status = 0;
  	//cout << "Server IP: " << serverIP << endl;
	/*
	Populate the server address and the port 
	*/
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = serverIP;
	servaddr.sin_port        = htons(serverPort);

	/*
	Connect to the server IP
	*/
	status = connect(serverSock, (struct sockaddr *) &servaddr, sizeof(servaddr));
  	if (status < 0) {
    	cerr << "Error opening host connection." << endl;
    	return false;
  	}
  	SendMessage(serverSock, request, request.size());

  	ReadMessage(serverSock, response, MAX_LINE, true);

  	if ( close(serverSock) < 0 ) {
		cerr << "PROXY-SERVER: Error calling close()" << endl;
		exit(EXIT_FAILURE);
	}
	else {
		cout << "Closed connection with server successfully !!!!" << endl;
	}

  	return true;
}

void* clientThread(void* args_p){

  // Local Variables
	threadArgs* tmp = (threadArgs*) args_p;
	int conn_s = tmp -> clientSock;
	delete tmp;
	int sent_msg_size;
	vector<string> request_params;
	time_t rawtime;
  // Detach Thread to ensure that resources are deallocated on return.
	pthread_detach(pthread_self());

	string request;
	/*  Retrieve an input line from the connected socket
	    then simply write it back to the same socket.     */
	ReadMessage(conn_s, request, MAX_LINE-1, false);
	
	cout << "Browser Message: " << endl << request << endl;
	request_params = split_string(request, ' ');
	if(request_params.size() > 0) {

		cout << "Fetching data....." << endl;

		string response;
		if(!fetchUrlData(request, response)) {
			cerr << "Error fetching data from URL! " << endl;
			//return;
		}
		//cout << response << endl;
		/*
		stringstream ss;
		ss << "HTTP/1.0 200 OK\n";
      	ss << "Date: " << ctime(&rawtime);
      	ss << "Server: CSS2\n";
      	ss << "Connection: Closed\n";
      	ss << "Content-Type: text/html\n";
      	ss << "\n";
      	ss << "<!DOCTYPE html>";
      	ss << "<html>\n";
      	ss << "<body>Hello World !!!!</body>\n";
      	ss << "</html>";
		cout << "Sending message......" << endl;
		cout << ss.str() << endl;
		*/


		try {
			sent_msg_size = SendMessage(conn_s, response, response.size());	
			cout << "> " << sent_msg_size << endl;
		} catch( ... ) {
			cerr << "Browser Closed Connection! " << endl;
		}

	}

	/*  Close the connected socket  */

	if ( close(conn_s) < 0 ) {
			//fprintf(stderr, "PROXY-SERVER: Error calling close()\n");
		cerr << "PROXY-SERVER: Error calling close()" << endl;
		exit(EXIT_FAILURE);
	}
	else {
		cout << "Closed connection with proxy server successfully !!!!" << endl;;
	}

  // Quit thread
	pthread_exit(NULL);
}


int main(int argc, char const *argv[])
{
	int       list_s;                /*  listening socket          */
    int       conn_s;                /*  connection socket         */
	unsigned int  cli_len;
    short int port;                  /*  port number               */
    struct    sockaddr_in servaddr;  /*  socket address structure  */
	struct    sockaddr_in cli_addr;
    char      buffer[MAX_LINE];      /*  character buffer          */
    char     *endptr;                /*  for strtol()              */

	/*  Get port number from the command line, and
        set to default port if no arguments were supplied  */
	if ( argc == 2 ) {
		port = strtol(argv[1], &endptr, 0);
		if ( *endptr ) {
			//fprintf(stderr, "PROXY-SERVER: Invalid port number.\n");
			cerr << "PROXY-SERVER: Invalid port number." << endl;
			exit(EXIT_FAILURE);
		}
	}
	else if ( argc < 2 ) {
		port = DEF_PORT;
	}
	else {
		//fprintf(stderr, "PROXY-SERVER: Invalid arguments.\n");
		cerr << "PROXY-SERVER: Invalid arguments." << endl;
		exit(EXIT_FAILURE);
	}

    /*  Create the listening socket  */
	if ( (list_s = socket(AF_INET, SOCK_STREAM,  IPPROTO_TCP)) < 0 ) {
		//fprintf(stderr, "PROXY-SERVER: Error creating listening socket.\n");
		cerr << "PROXY-SERVER: Error creating listening socket." << endl;
		exit(EXIT_FAILURE);
	}

    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

    /*  Bind our socket addresss to the 
	listening socket, and call listen()  */

	if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
		//fprintf(stderr, "PROXY-SERVER: Error calling bind()\n");
		cerr << "PROXY-SERVER: Error calling bind()" << endl;
		exit(EXIT_FAILURE);
	}

	if ( listen(list_s, MAXPENDING) < 0 ) {
		//fprintf(stderr, "PROXY-SERVER: Error calling listen()\n");
		cerr << "PROXY-SERVER: Error calling listen()" << endl;
		exit(EXIT_FAILURE);
	}

     /*  Enter an infinite loop to respond
        to client requests and process them  */

	while ( 1 ) {
		cli_len = sizeof(cli_addr);
	/*  Wait for a connection, then accept() it  */

		if ( (conn_s = accept(list_s, (struct sockaddr *)&cli_addr, &cli_len) ) < 0) {
			cerr << "PROXY-SERVER: Error calling accept()" << endl;
			//fprintf(stderr, "PROXY-SERVER: Error calling accept()\n");
			exit(EXIT_FAILURE);
		}
		/*
		cout << "> " << inet_ntoa(cli_addr.sin_addr)
		<< ":" << (int) ntohs(cli_addr.sin_port) 
		<< " is connected" << endl;
		*/
		
		// Create child thread to handle process
		struct threadArgs* args_p = new threadArgs;
		args_p -> clientSock = conn_s;
		pthread_t tid;
		int threadStatus = pthread_create(&tid, NULL, clientThread, (void*)args_p);
		if (threadStatus != 0){
      		// Failed to create child thread
			cerr << "Failed to create child process." << endl;
			close(conn_s);
			pthread_exit(NULL);
		}
	}

	return 0;
}