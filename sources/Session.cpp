/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Session.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pvivian <pvivian@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/05 17:43:31 by pvivian           #+#    #+#             */

/*   Updated: 2021/05/10 13:27:45 by pvivian          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Session.hpp" 

Session::Session(void) { return; } // private
Session::Session(std::vector<configServer*> config, int fd, struct sockaddr_in *from)
{
    this->confServer = config;
    this->from_ip = ntohl(from->sin_addr.s_addr);
    this->from_port = ntohs(from->sin_port);
	this->parseRequest = new ParseRequest;
  	this->handler      = new Handler(fd,from_ip);
	this->fd = fd;
	this->closeConnection = OFF;
    setAuthenticationOff();
	return; 
}

Session::~Session(void) 
{
	delete this->handler;
	delete this->parseRequest;
	return; 
}

int Session::send_message(void)
{
    if ((write(this->fd, wr_buf.c_str(), wr_buf.length())) < 0)
	{
		wr_buf.clear();
		std::cout << "Client disconnected" << std::endl;
		return 0;
	}
	wr_buf.clear();
    if (closeConnection && !this->handler->isReading())
        return 0;
	return 1;
}

int Session::do_read(void)
{
	ssize_t read_res;
	char tmp[INBUFSIZE];

	this->buf.clear();

	read_res = read(this->fd, tmp, INBUFSIZE);
	if (read_res <= 0) {
		if (read_res < 0) {
		// internal server error?
		; }
		else
			std::cout << "Client disconnected" << std::endl;
		return 0;
	}
	this->buf.append(tmp, read_res);
	return 1;
}

void Session::handle_request(fd_set * writefds)
{
    this->request_left = this->parseRequest->addToBuffer((std::string) this->buf);
    if (parseRequest->isRequestReady()) 
    {
        configServer *config = getConfig();
        if (parseRequest->getConnection() == "close")
            closeConnection = ON;
        this->wr_buf = this->handler->handle(*(config), parseRequest->getData());
        FD_SET(this->fd, writefds);
    }
}

void Session::handle_cgi(fd_set * writefds)
{
    this->wr_buf = this->handler->handle();
    FD_SET(this->fd, writefds);
}

bool Session::isRequestLeft(void) { return this->request_left; }
int  Session::getCgiFd(void) const { return this->handler->getCgiFd(); }

void Session::setAuthenticationOff()
{
    std::fstream ifs("./content/website1/authentication/authentication.txt", std::ios_base::in);
    if (!ifs.good())
        return;
    std::string str;
    std::string tmp;
    while (getline(ifs, tmp))
        str += tmp;
    ifs.close();

    size_t pos = str.find("true", 0);
    while (pos != std::string::npos)
    {
        str.erase(pos, 4);
        str.insert(pos, "false");

        pos = str.find("true", 0);
    }
    std::ofstream ofs("./content/website1/authentication/authentication.txt", std::ios_base::trunc);
    if (!ofs.good())
        return;
    ofs << str;
}

configServer *Session::getConfig(void)
{
    std::string host = parseRequest->getHost();
    int num = -1;
    std::string serverName = host.substr(0, host.find(":"));
    for (size_t i = 0; i < confServer.size(); i++)
    {
        if (confServer[i]->ip == ip)
        {
            for (size_t j = 0; j < confServer[i]->port.size(); j++)
            {
                if (this->port == confServer[i]->port[j])
                {
                    if(num == -1)
                        num = (int)i;
                    if (serverName == this->confServer[i]->server_name) {
                        return confServer[i];
                    }
                }
            }
        }
    }
    if (num == -1)
        num = 0;
    return confServer[num];
}
