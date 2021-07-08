#ifndef SESSION_HPP
# define SESSION_HPP

# include "Webserv.hpp"
# include "ParseRequest.hpp"
# include "Handler.hpp"

class Session
{

public:

	int             fd;
    unsigned long   from_ip;
    unsigned short  from_port;
    unsigned long   ip;
    unsigned short  port;

    std::string     buf;
	std::string     wr_buf;

private:

    std::vector<configServer*>  confServer;
	ParseRequest *              parseRequest;
	Handler *                   handler;
	bool                        request_left;
	bool                        closeConnection;

public:

    Session(std::vector<configServer*> config, int fd, struct sockaddr_in *from);
    ~Session(void);

	int             send_message(void);
	int             do_read(void);
	void            handle_request(fd_set * writefds);
	void            handle_cgi(fd_set * writefds);
	bool            isRequestLeft(void);
	int             getCgiFd(void) const;
    void            setAuthenticationOff();
    configServer *  getConfig(void);


private:

	Session(void);
};

#endif
