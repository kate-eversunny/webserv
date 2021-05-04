/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pvivian <pvivian@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/04 18:28:43 by pvivian           #+#    #+#             */
/*   Updated: 2021/05/04 19:02:26 by pvivian          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <cstdlib> //malloc, free, exit
# include <unistd.h> //write, read, close, mkdir, rmdir, unlink, fork, getcwd, chdir, stat, lstat, fstat, lseek, execve, dup, dup2, pipe, usleep, select
# include <sys/wait.h>  //wait, waitpid, wait3, wait4,
# include <fcntl.h>  //open, fcntl
# include <signal.h> // signal, kill
# include <netinet/in.h> //htons, htonl, ntohs, ntohl
# include <sys/types.h> //opendir, readdir, closedir
# include <cstring>  // strerror
# include <cerrno> // errno
# include <sys/time.h> //gettimeofday
# include <time.h> //strptime, strftime
# include <sys/socket.h> // socket, accept, listen, send, recv, bind, connect, inet_addr, setsockopt, getsockname

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

# define INBUFSIZE 1024
# define LISTEN_QLEN 32
# define INIT_SESS_ARR_SIZE 32

# include "Server.hpp"
# include "Session.hpp"
# include "Config.hpp"



#endif