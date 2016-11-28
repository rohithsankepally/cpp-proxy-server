#include "helper.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <string.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
using namespace std;
/*  Read a line from a socket  */

ssize_t ReadMessage(int sockd, string &message, size_t maxlen, bool isServer) {
	// Local Variables
	stringstream ss;
	int bufferSize = 1000; 
	int totalSize = 0;
	int bytesRecv;
	char buffer[bufferSize];
	char* buffPTR = buffer;
	/*
	time_t timer;
	time_t check;
	time(&timer);
	*/
	memset(buffPTR, '\0', bufferSize);

  // Handle communications
	while (true) {
		bytesRecv = read(sockd, (void*) buffPTR, bufferSize);
		if (bytesRecv < 0) {
			cerr << "Error occured while trying to receive data." << endl;
			close(sockd);
			pthread_exit(NULL);
			return -1;
		} 
		else if (bytesRecv == 0) {
			break;
		} 
		else {
			totalSize += bytesRecv;
			for (int i = 0; i < bytesRecv; i++) {
				ss << buffPTR[i];
			}
			
			if (totalSize > 4 && !isServer){
				
				string tmpMsg = ss.str();
				if (tmpMsg[tmpMsg.length()-4] == '\r'
					&& tmpMsg[tmpMsg.length()-3] == '\n'
					&& tmpMsg[tmpMsg.length()-2] == '\r'
					&& tmpMsg[tmpMsg.length()-1] == '\n')
				{
					break;
				}
			}
			/*
			time(&check);
			if (difftime(check,timer) > 4) {
				cout << "Time Failed: " << difftime(check, timer) << endl;
				return -1;
			}*/

		}
	}
	string tmpStr = ss.str();
	if (tmpStr.find("Connection: keep-alive") != string::npos)
		tmpStr.replace(tmpStr.find("Connection: keep-alive"), 24, "");
  // Return HttpRequestObj
	message = tmpStr;
	return tmpStr.size();
}


/*  Write a line to a socket  */

ssize_t SendMessage(int sockd, string message, size_t n) {
	//Local
	string messageToSend = message;
	int msgLength = messageToSend.length();
	int msgSent = 0;
	char msgBuff[msgLength];

  // Transfer message.
	memcpy(msgBuff, messageToSend.c_str(), msgLength);

  // Send message
	while (msgSent != msgLength) {
		msgSent = write(sockd,(void*) msgBuff, msgLength);
	}

	return msgSent;

}

