/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pvivian <pvivian@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/06 16:47:36 by pvivian           #+#    #+#             */

/*   Updated: 2021/05/10 15:41:58 by pvivian          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLER_HPP
# define HANDLER_HPP

# include "Webserv.hpp"
# include "Config.hpp"

class Handler
{
private:

    Handler(void);

	std::string 	response;
	data 			request;

	bool			isCgiReading;
	std::string		tmp;
	int				cgiFd;

	int 			index_location;
	std::string		path;
	std::string 	location_path;

	configServer	config;
	user            _userData;
  
	std::vector<std::string> arrDir;
	bool			isDir;

	std::string		lastModTime;
	std::string		contentLength;
	std::string     ip;

public:

    Handler(int sessionFd, unsigned long ip);
	~Handler(void);

	/* LAUNCH */
	std::string const &     handle(configServer	config, data const & request);
	std::string const &     handle(void);
    void                    makePath(void);

    /* LOGIN */
    void                    checkUserLogInByCookie();
    void                    checkUserLogIn();

    /* AUTOINDEX */
    void	                getFilesOrDirFromRoot();
    void                    makeAutoindexPage(std::string * body);
    std::string             getLink(std::string file_name);

    /* HEAD */
    void                    handle_head(void);
    bool                    ResponseFromSessionManagement();
    void                    AddResponseToSessionManagement();
    int                     checkFile(void);

    /* POST */
    void                    handle_post(void);
    char **                 create_env(void);
    void                    add_env(std::vector<std::string> * envs);
    int                     launchCgi(char **args, char ** env, std::string * body);
    int                     readCgi(std::string * body);

    /* PUT */
    void                    handle_put(void);

    /* DELETE */
    void                    handle_delete(void);

    /* ERRORS */

	int isRequestCorrect(void);
    int isLocation(std::vector<location *> locations, std::string path);
    int isFiles(std::string path, std::string locPath);
    void searchPath(std::string &locTmp, std::string &reqTmp, std::string &locPath,size_t &j,size_t i,
                    int &theBestLocation, std::string &reqPath, std::vector<location *> locations);
    int searchlocPath(std::string &locTmp, std::string &reqTmp, size_t &j,size_t i, int &theBestLocation,  std::vector<location *> locations,
                      std::string &reqPath, int flag, std::string &locPath);
    int searchreqPath(std::string &locTmp, std::string &reqTmp, size_t &j,size_t i,
                      int &theBestLocation, std::vector<location *> locations, std::string &reqPath, std::string &locPath);
    int putVal(std::string locPath,size_t j,size_t i, int theBestLocation, std::vector<location *> locations);
    int doesLocationAnswersMethod(void);
    void error_message(int const & status_code);

    /* ADD HEADERS */
    void addHeaderStatus(int status);
    void addHeaderServer(void);
    void addHeaderDate(void);
    void addHeaderSetCookie(void);
    void addHeaderContentLanguage(void);
    void addHeaderContentLocation(void);
    void addHeaderContentLength(std::string size);
    void addHeaderLocation(void);
    void addHeaderContentType(void);
    void addHeaderLastModified(void);
	void addHeaderAllow(void);

  	/* LIBFT */

    int             ft_strlen(const char *str);
    void		    ft_free_array(char **to_free);
    char *          ft_strdup(const char *s);
	std::string     lltostr(long long number, int base);

    /* EXTRA */
    bool loadBodyFromFile(std::string * body);
    bool loadBodyFromFile(std::string * body, std::string path);
    std::string getPresentTime(void);
    std::string getLastModificationTime(time_t const & time);

    /* GET */
    int getCgiFd(void) const;
    bool isReading(void) const;

};

#endif