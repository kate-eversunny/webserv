#ifndef CLUSTER_HPP
# define CLUSTER_HPP

# include "Webserv.hpp"
# include "Server.hpp"
# include "Session.hpp"
# include "Config.hpp"

class Cluster
{
private:

	fd_set 					            readfds;
	fd_set 					            writefds;
	std::vector<Server *> 	            servers;
	std::vector<int> 		            listenSockets;
    std::vector<struct sockaddr_in>     addr;
	std::vector<Session *>	            sessions;
    std::vector<configServer*>          config;

public:
	Cluster(void);
	~Cluster(void);

	void        init(const Config & config);
	void        run(void);
	void        updateSelectSets(int * maxfd);
	void        acceptClient(int pos);
	Session *   make_new_session(int fd, struct sockaddr_in *from, int pos);
	void        closeSession(int sd);
	void        closeAllSessions(void);
};

#endif
