/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pvivian <pvivian@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/04 18:28:09 by pvivian           #+#    #+#             */
/*   Updated: 2021/05/04 19:24:38 by pvivian          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "Webserv.hpp"

class Server
{
private:
	int listenSocket;
	FILE *res;
	Session **sess_array;
	int sess_array_size;

public:
	void init(Config const & config)
	{
		int sock, opt;
		struct sockaddr_in addr;
		FILE *f;

		sock = socket(AF_INET, SOCK_STREAM, 0);
		if(sock == -1)
			throw std::runtime_error("Could not create a socket");

		opt = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(config.getPort());
		if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1)
			throw std::runtime_error("Could not bind socket");

		listen(sock, LISTEN_QLEN);
		this->listenSocket = sock;

		f = fopen(config.getLogFile(), "wb");
		if(!f) {
			close(sock);
			throw std::runtime_error("Could not create a log file");
		}
		this->res = f;
		
		this->sess_array = (Session **)malloc(sizeof(Session *) * INIT_SESS_ARR_SIZE);
		this->sess_array_size = INIT_SESS_ARR_SIZE;
		for(int i = 0; i < INIT_SESS_ARR_SIZE; i++)
			this->sess_array[i] = NULL;
	}
	
	int run(void)
	{
		fd_set readfds;
		int i, sr, ssr, maxfd;
		for(;;) {
			FD_ZERO(&readfds);
			FD_SET(this->listenSocket, &readfds);
			maxfd = this->listenSocket;
			for(i = 0; i < this->sess_array_size; i++) {
				if(this->sess_array[i]) {
					FD_SET(i, &readfds);
					if(i > maxfd)
						maxfd = i;
				}
			}
			sr = select(maxfd+1, &readfds, NULL, NULL, NULL);
			if(sr == -1) {
				// perror("select");
				return 4;
			}
			if(FD_ISSET(this->listenSocket, &readfds))
				server_accept_client(serv);
			for(i = 0; i < this->sess_array_size; i++) {
				if(this->sess_array[i] && FD_ISSET(i, &readfds)) {
					ssr = session_do_read(this->sess_array[i]);
					if(!ssr)
						server_close_session(serv, i);
				}
			}
		}
		return 0;
	}
	
};

#endif