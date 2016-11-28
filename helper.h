#ifndef PG_SOCK_HELP
#define PG_SOCK_HELP


#include <unistd.h>             /*  for ssize_t data type  */

#include <string>
using namespace std;
#define LISTENQ        (1024)   /*  Backlog for listen()   */


/*  Function declarations  */

ssize_t ReadMessage(int fd, string &inp, size_t maxlen);
ssize_t SendMessage(int fc, string msg, size_t maxlen);


#endif  /*  PG_SOCK_HELP  */