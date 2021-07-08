
#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <cstdlib> //malloc, free, exit
# include <unistd.h> //write, read, close, mkdir, rmdir, unlink, fork, getcwd, chdir, stat, lstat, fstat, lseek, execve, dup, dup2, pipe, usleep, select
# include <sys/stat.h> //stat, lstat, fstat
# include <sys/wait.h>  //wait, waitpid, wait3, wait4,
# include <fcntl.h>  //open, fcntl
# include <signal.h> // signal, kill
# include <netinet/in.h> //htons, htonl, ntohs, ntohl
# include <arpa/inet.h> // inet_addr
# include <sys/types.h> //opendir, readdir, closedir
# include <cstring>  // strerror
# include <cerrno> // errno
# include <sys/time.h> //gettimeofday
# include <time.h> //strptime, strftime
# include <sys/socket.h> // socket, accept, listen, send, recv, bind, connect, inet_addr, setsockopt, getsockname
# include <dirent.h> // opendir, readdir, closedir
#include <errno.h>

# include <iostream>
# include <string>
# include <vector>
# include <list>
# include <queue>
# include <stack>
# include <map>
# include <algorithm>
# include <exception>
# include <ctime>
# include <fstream>
# include <sstream>

# define INBUFSIZE 4096
# define LISTEN_QLEN 128
# define INIT_SESS_ARR_SIZE 128

# define REQUEST_PARSE 0
# define REQUEST_READY 1

#define ON true
#define OFF false

#define BW "\033[0;0m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"
#define YELLOW "\033[0;33m"
#define PURPLE "\033[0;35m"

/* BODY ENCODING TYPES */
#define TRANSFER_ENCODING_CHANG 0
#define TRANSFER_ENCODING_COMPRESS 1
#define TRANSFER_ENCODING_DEFLATE 2
#define TRANSFER_ENCODING_GZIP 3
#define TRANSFER_ENCODING_IDENTYTY 4
#define CONTENT_LENGTH 5

struct data
{
    std::string							    	method;
    std::string							    	path;
    std::string							    	version;
    std::multimap <std::string, std::string> *  headers;
    std::string							    	body;
    int                                         bodyLen;
    size_t                                      status;
    int                                         nmb;
    int                                         bodyEncryption;

    std::string                                 formData;
};

struct user
{
    bool            signIn;
    std::string     login;
};

#endif
