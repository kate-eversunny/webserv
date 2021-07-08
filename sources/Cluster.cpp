# include "Cluster.hpp"

Cluster::Cluster(void) 
{
	this->sessions = std::vector<Session *>(INIT_SESS_ARR_SIZE, NULL);
    mkdir("cgi/temp", S_IRWXU | S_IRWXG | S_IRWXO);
	return;
}
Cluster::~Cluster(void)
{
	closeAllSessions();
	for (size_t i = 0; i < servers.size(); i++)
		delete servers[i];
	return;
}

void Cluster::init(const Config & config) 
{
	Server * server;
	configServer *confServer;
	std::vector<int> sockets;
    std::vector<struct sockaddr_in> addrs;
    this->config = config.getAllServers();

	for (size_t  i = 0; i < config.getSize(); i++)
	{
		confServer = config.getconfigServer((int)i);
		server = new Server;
		server->init(*confServer);
		servers.push_back(server);
		sockets = server->getListenSockets();
		listenSockets.insert(listenSockets.end(), sockets.begin(), sockets.end());
		addrs = server->getAddrs();
		addr.insert(addr.end(), addrs.begin(), addrs.end());
	}
	return;
}

void Cluster::run(void)
{
	size_t i;
	int sr, ssr, maxfd;

	if (listenSockets.size() == 0)
        throw std::runtime_error("All ports are unavailable");

	for(;;) {
        updateSelectSets(&maxfd);
        sr = select(maxfd + 1, &readfds, &writefds, NULL, NULL);
        if (sr == -1)
            throw std::runtime_error("Select error");
        for (i = 0; i < this->listenSockets.size(); i++)
            if (FD_ISSET(this->listenSockets[i], &readfds))
                acceptClient(i);
        for (i = 0; i < sessions.size(); i++) {
            if (sessions[i]) {
                if (sessions[i]->getCgiFd() > 0 && FD_ISSET(sessions[i]->getCgiFd(), &readfds))
                    sessions[i]->handle_cgi(&writefds);
                else if (FD_ISSET(i, &readfds)) {
                    ssr = sessions[i]->do_read();
                    if (ssr == 1 || sessions[i]->isRequestLeft())
                        sessions[i]->handle_request(&writefds);
                    if (!ssr)
                        closeSession(i);
                }
                if (FD_ISSET(i, &writefds)) {
                    ssr = sessions[i]->send_message();
                    if (!ssr)
                        closeSession(i);
                }
            }
        }
    }
    return;
}

void Cluster::updateSelectSets(int * maxfd)
{
	int cgiFd = 0;
	size_t i = 0;

	FD_ZERO(&readfds);                  // зачистить сет для чтения
	FD_ZERO(&writefds);                 // зачистить сет для записи
	for (; i < this->listenSockets.size(); i++)
		FD_SET(this->listenSockets[i], &readfds);
	*maxfd = this->listenSockets.back();

	for(i = 0; i < sessions.size(); i++) {
		if(sessions[i]) {
			FD_SET(i, &readfds);
			if((int)i > *maxfd)
				*maxfd = i;
			if ((cgiFd = sessions[i]->getCgiFd()) > 0)
			{
				FD_SET(cgiFd, &readfds);
				if(cgiFd > *maxfd)
					*maxfd = cgiFd;
			}
		}
	}
	return;
}

void Cluster::acceptClient(int pos)
{
	int sd;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	sd = accept(this->listenSockets[pos], (struct sockaddr*) &addr, &len);
	if (sd == -1)
		throw std::runtime_error("Accept error");

	if (sd >= (int)this->sessions.size()) 
	{
		int newlen = this->sessions.size();
		while(sd >= newlen)
			newlen += INIT_SESS_ARR_SIZE;
		this->sessions.resize(newlen, NULL);
	}

	this->sessions[sd] = make_new_session(sd, &addr, pos);
}

Session * Cluster::make_new_session(int fd, struct sockaddr_in *from, int pos)
{
    Session *sess = new Session(this->config, fd, from);
    sess->from_ip = ntohl(from->sin_addr.s_addr);
    sess->from_port = ntohs(from->sin_port);
    sess->ip = addr[pos].sin_addr.s_addr;
    sess->port = addr[pos].sin_port;
    return sess;
}

void Cluster::closeSession(int sd)
{
	close(sd);
	delete this->sessions[sd];
	this->sessions[sd] = NULL;
	return;
}

void Cluster::closeAllSessions(void)
{
	for (size_t i = 0; i < this->sessions.size(); i++)
	{
		if (this->sessions[i])
			closeSession(i);
	}
	return;
}