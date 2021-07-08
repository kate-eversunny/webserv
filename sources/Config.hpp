/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pvivian <pvivian@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/04 18:31:32 by pvivian           #+#    #+#             */
/*   Updated: 2021/06/07 15:01:40 by pvivian          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "Webserv.hpp"

#define METHOD 2
#define SERVER 1
#define LOCATION 0


	struct 						location
	{
		std::string 			path;
		bool 					repeat_path;
		std::string 			index;
		bool 					repeat_index;
        std::string 			cgi;
        std::string 			cgi_name;
        bool 					repeat_cgi;
		std::string 			root;
		bool 					repeat_root;
		int 					maxBody;
		bool 					repeat_maxBody;
		std::vector<std::string> method;
		bool 					repeat_method;
        bool                    autoIndex;
        bool 					repeat_autoIndex;
        bool                    authentication;
        bool 					repeat_authentication;
        std::string 			redirect;
        bool 					repeat_redirect;
	};

struct 	configServer
{
        std::vector<in_port_t> 	port;
		bool 					repeat_port;
        in_addr_t 		        ip;
        std::string 			ip_str;
		std::string 			server_name;
		bool 					repeat_server_name;
		std::string 			error_page;
		bool 					repeat_error_page;
		std::vector<location*> 	locations;
	};


class Config
{

    private:

        std::vector<configServer*> 	servers;
        std::vector<std::string>    serverTokens;
        std::vector<std::string>    locationTokens;
        std::vector<std::string>    methodTokens;

    public:
        Config();
        ~Config();

        void						getFile(std::string file);
        std::string					parseStr(std::string str);
        std::string					parseLocation(std::string str,  configServer *servNode);
        void 						locTokenSearch(std::string save, std::string tmp, location *locNode);
        void 						serverTokenSearch(std::string save, std::string tmp, configServer *servNode);
        bool						checkTokens(std::string &save, std::string str, int config_part);

        void						initLocNode(location *locNode);
        void						initServNode(configServer *servNode);

        bool						checkMainValLoc(location *locNode);
        bool						checkMainValServ(struct configServer *servNode);
        void                        initConfigEnv();
        bool                        checkIndex(std::string root, std::string indexPath);
        bool                        checkErrorPage(std::string path);
        bool                        checkLockPath(std::string path);
        void                        getPortsAndIP(configServer *servNode, std::string portsStr);
        bool                        check_repeat_ports(configServer *servNode);
        bool                        checkCgi(std::string cgiPath);
        bool                        checkRedirect(std::string redirectPath);

        configServer        		*getconfigServer(int index) const;
        size_t						getSize(void) const;
        std::vector<configServer*>  getAllServers() const;

        class 						FileNotOpenException: public std::exception { const char *what() const throw(); };
        class 						FileLengthException: public std::exception { const char *what() const throw(); };
        class 						ServerNameException: public std::exception { const char *what() const throw(); };
        class 						ValueSaveException: public std::exception { const char *what() const throw(); };
        class 						ErrorPageException: public std::exception { const char *what() const throw(); };
        class 						FullServerValueException: public std::exception { const char *what() const throw(); };
        class 						IncorrectLocationPathException: public std::exception { const char *what() const throw(); };
        class 						MethodException: public std::exception { const char *what() const throw(); };
        class 						FullLocationValueException: public std::exception { const char *what() const throw(); };
        class 						MaxBodyException: public std::exception { const char *what() const throw(); };
        class 						RootException: public std::exception { const char *what() const throw(); };
        class 						IndexException: public std::exception { const char *what() const throw(); };
        class 						RedirectException: public std::exception { const char *what() const throw(); };
        class 						CGIException: public std::exception { const char *what() const throw(); };
        class 						AutoindexException: public std::exception { const char *what() const throw(); };
        class 						AuthenticationException: public std::exception { const char *what() const throw(); };
        class 						ListenException: public std::exception { const char *what() const throw(); };
        class 						PortIPException: public std::exception { const char *what() const throw(); };
};

#endif
