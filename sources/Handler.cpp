#include "Handler.hpp"

Handler::Handler(void){ return; } // private

Handler::Handler(int sessionFd, unsigned long ip)
{
	this->isCgiReading = false;
	this->ip = std::to_string(ip);
	this->isDir = false;
	this->tmp = "./cgi/temp/" + lltostr(sessionFd, 10) + ".txt";
	this->cgiFd = -1;

    this->_userData.login.clear();
    this->_userData.signIn = false;
    return;
}

Handler::~Handler(void)
{
	if (this->cgiFd > 0)
		close(this->cgiFd);
	remove(this->tmp.c_str());
	this->response.clear();
	return;
}

/*
 * ----------------------------------------- HANDLE LAUNCH --------------------------------------
 */

std::string const & Handler::handle(configServer config, data const & req)
{
	this->response.clear();
	this->request = req;
	this->config = config;

	checkUserLogInByCookie();

	if (!isRequestCorrect())
		return this->response;

	makePath();
	if(this->isDir && config.locations[this->index_location]->autoIndex == ON)
		getFilesOrDirFromRoot();

	if (request.method == "HEAD" || request.method == "GET")
		handle_head();
	else if (request.method == "POST")
	    handle_post();
	else if (request.method == "PUT")
		handle_put();
	else if (request.method == "DELETE")
		handle_delete();

	this->path.clear();
	this->location_path.clear();
	this->arrDir.clear();
	this->contentLength.clear();
	this->lastModTime.clear();
    this->isDir = false;
	return this->response;
}

std::string const & Handler::handle(void)
{
    std::string body;
    int status;

    this->response.clear();
    status = readCgi(&body);
    if (status == 1)
        error_message(500);
    else
        this->response.append(body);
    return this->response;
}

void                Handler::makePath(void)
{
    DIR	*dir;
    size_t pos = 0;
    size_t end = 0;

    /* path */
    this->path = ".";
    this->path.append(request.path);
    std::string tmp_location = this->config.locations[index_location]->path;

    if (tmp_location[tmp_location.length() - 1] != '/')
        tmp_location = tmp_location + "/";

    if ((pos = tmp_location.find("*", 0)) != std::string::npos)
        tmp_location = tmp_location.substr(0, pos);

    if ((pos = this->path.find(tmp_location, 0)) == std::string::npos)
    {
        this->path.append("/");
        pos = this->path.find(tmp_location, 0);
        end = config.locations[index_location]->path.length();
        this->path.replace(pos, end, config.locations[index_location]->root);
    }
    else
    {
        pos = this->path.find(tmp_location, 0);
        end = tmp_location.length() - 1;
        if(pos != std::string::npos)
            this->path.replace(pos, end, config.locations[index_location]->root);
    }

    this->location_path = this->path.substr(1, std::string::npos);

    // index
    dir = opendir(path.c_str());
    if (dir)
    {
        if (config.locations[index_location]->index.length() > 0) {
            this->path.append("/");
            this->path.append(config.locations[index_location]->index);

            this->location_path.append("/");
            this->location_path.append(config.locations[index_location]->index);
        }
        else
            this->isDir = true;
        closedir(dir);
    }
}

/*
 * ----------------------------------------- LOG IN --------------------------------------
 */

void                Handler::checkUserLogInByCookie()
{
    if (request.headers->find("Cookie") != request.headers->end())
    {
        size_t pos = request.headers->find("Cookie")->second.find("=");
        _userData.signIn = true;
        _userData.login = request.headers->find("Cookie")->second.substr(pos + 1);
        pos = _userData.login.find(";");
        if (pos != std::string::npos)
            _userData.login = _userData.login.substr(0, pos);
    }
}

void                Handler::checkUserLogIn() // check from file after cgi
{
    std::fstream ifs("./content/website1/authentication/authentication.txt", std::ios_base::in);
    if (!ifs.good())
        return error_message(500);

    std::string str;
    std::string tmp;
    while (getline(ifs, tmp))
        str += tmp;
    ifs.close();

    size_t pos = str.find("true", 0);
    if (pos != std::string::npos)
    {
        str = str.substr(0, pos - 1);
        pos = str.find_last_of(";", std::string::npos);
        str = str.substr(pos + 1, std::string::npos);
        pos = str.find("=", 0);
        str = str.substr(0, pos);
        _userData.login = str;
        _userData.signIn = true;
    }
    else
    {
        _userData.login.clear();
        _userData.signIn = false;
    }
}

/*
 * ----------------------------------------- AUTOINDEX --------------------------------------
 */

void                Handler::getFilesOrDirFromRoot()
{
    DIR *dir;
    struct dirent *dirStruct;
    this->arrDir.clear();

    if ((dir = opendir(this->path.c_str())) == nullptr)
        std::cout << RED<< "ERROR OPEN DIR"<< BW<<std::endl;

    while((dirStruct = readdir(dir)) != nullptr)
        this->arrDir.push_back(dirStruct->d_name);
    closedir(dir);
}

void                Handler::makeAutoindexPage(std::string * body)
{
    body->append("<html>");
    for (int i = 1; i < (int)arrDir.size(); i++)
    {
        if (arrDir[i] != "." && arrDir[i] != ".." && arrDir[i] != "authentication")
            body->append(getLink(arrDir[i]));
    }
    body->append("</html>");

    this->lastModTime = getPresentTime();
    this->contentLength = lltostr(body->length(), 10);
}

std::string         Handler::getLink(std::string file_name)
{
    std::string file_link = request.path + "/";
    file_link += file_name;
    std::string link;
    link = "<div style=\"display: inline-block;width: 35%;\"></span><a href=\"" + file_link +"\">"+ file_name + "</a></div>"
                        "<div style=\"display: inline-block\">"+ getPresentTime() +"</div></br>";
    return (link);
}

/*
 * ----------------------------------------- HEAD --------------------------------------
 */

void                Handler::handle_head(void)
{
    if (config.locations[index_location]->authentication && _userData.signIn == false)
        return error_message(511);

    if (config.locations[index_location]->authentication == OFF && !config.locations[index_location]->repeat_redirect && ResponseFromSessionManagement())
        return;

	std::string body;
	int status_code = 200;

	if (isDir && config.locations[this->index_location]->autoIndex == ON)
		makeAutoindexPage(&body);
	else if (checkFile() != 0)
		return;

    if (config.locations[index_location]->repeat_redirect == true)
        status_code = 301;
    else if (config.locations[index_location]->repeat_redirect == true)
        status_code = 308;

    addHeaderStatus(status_code);
    addHeaderServer();
    addHeaderDate();
    addHeaderContentLanguage();
    addHeaderContentLocation();
    if (status_code == 301 || status_code == 308)
        addHeaderLocation();
    addHeaderContentType();
    addHeaderLastModified(); 

	if (request.method == "GET")
	{
		if (body.length() == 0)
            loadBodyFromFile(&body);
        if (this->path == "./content/website1/close/user_profile.html")
        {
            size_t pos = body.find("<?php userName(); ?>", 0);
            while (pos != std::string::npos)
            {
                body.replace(pos, 20, _userData.login);
                pos = body.find("<?php userName(); ?>", 0);
            }
        }
        addHeaderContentLength(std::to_string(body.length()));
        this->response.append("\r\n");
        std::cout << PURPLE << "RESPONSE" << BW << std::endl << this->response << std::endl;
		this->response.append(body);
	}
	else
    {
        addHeaderContentLength(this->contentLength);
        this->response.append("\r\n");
        std::cout << PURPLE << "RESPONSE" << BW << std::endl << this->response << std::endl;
    }
	if (config.locations[index_location]->authentication == OFF && !config.locations[index_location]->repeat_redirect)
        AddResponseToSessionManagement();
}

bool                Handler::ResponseFromSessionManagement()
{
    std::string sm_path = config.locations[0]->root.substr(1, std::string::npos) + "/sessionManagement";
    std::string sm_ip_path = sm_path + "/" + this->ip;

    mkdir(sm_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(sm_ip_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

    std::string sm_link = "./" + sm_ip_path;
    std::string file_name = this->request.path;
    std::replace( file_name.begin(), file_name.end(), '/', '_');
    std::replace( file_name.begin(), file_name.end(), '.', '_');
    sm_link = sm_link + "/" + file_name + ".txt";

    return loadBodyFromFile(&this->response, sm_link);
}

void                Handler::AddResponseToSessionManagement()
{
    std::string sm_path = config.locations[0]->root.substr(1, std::string::npos) + "/sessionManagement/" + this->ip;
    mkdir(sm_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

    std::string file_name = this->request.path;
    std::replace( file_name.begin(), file_name.end(), '/', '_');
    std::replace( file_name.begin(), file_name.end(), '.', '_');
    sm_path = sm_path + "/" + file_name + ".txt";

    std::ofstream outfile (sm_path);
    outfile << this->response;
    outfile.close();
}

int                 Handler::checkFile(void)
{
    int fd;
    struct stat file_stat;

    if ( (fd = open(this->path.c_str(), O_RDONLY)) == -1)
    {
        if (errno == ENOENT || errno == EFAULT || errno == ENOTDIR)
            error_message(404);
        else
            error_message(500);
        return 1;
    }

    fstat(fd, &file_stat);
    this->contentLength = lltostr(file_stat.st_size, 10);
    this->lastModTime = getLastModificationTime(file_stat.st_mtime);
    close(fd);
    return 0;
}

/*
 * ----------------------------------------- POST --------------------------------------
 */

void                Handler::handle_post(void)
{
    if (config.locations[index_location]->cgi_name == "python_upload.py" && request.formData.length() == 0)
    {
        // create directory
        std::string pathBody = "." + config.locations[index_location]->root;
        if (_userData.login.length() != 0)
            pathBody += "/" + _userData.login;
        mkdir(pathBody.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

        //  create file
        pathBody += "/tmp.txt";
        std::ofstream outfile (pathBody);
        outfile.close();

        this->path = pathBody;
    }

    if (request.formData.length() == 0)
    {
        std::ofstream ofs(this->path.c_str(), std::ios_base::trunc);
        if (!ofs.good())
            return error_message(500);
        ofs << request.body;
        ofs.close();
    }
    else
    {
        std::fstream ifs(this->path.c_str(), std::ios_base::in);
        if (!ifs.good())
            return error_message(500);
        ifs.close();
    }

    std::string body;
    std::string lengthHeader;

    if (config.locations[index_location]->cgi.size() != 0)
    {
        char ** envPost = create_env();
        if (!(envPost))
            return error_message(500);

        const char *pathToCGI = config.locations[index_location]->cgi.c_str();
        char *args[3] = { (char *)pathToCGI, (char *)this->path.c_str(), NULL};

        if (launchCgi(args, envPost, &body) == 1)
            return ft_free_array(envPost);

        ft_free_array(envPost);

        if (config.locations[index_location]->cgi_name != "python_upload.py")
            lengthHeader = "Transfer-Encoding: chunked\r\n";
        else
        {
            body.clear();
            body.append("\r\n\r\n");
            loadBodyFromFile(&body);
            lengthHeader = "Content-Length: " + std::to_string(body.length() - 4) + "\r\n";
        }

    }
    else
    {

        body.append("\r\n");
        body.append(request.body);
        lengthHeader = "Content-Length: " + lltostr(request.body.length(), 10) + "\r\n";
    }
    if (request.formData.length() != 0)
        checkUserLogIn();

    if (config.locations[index_location]->cgi_name == "python_upload.py" && request.formData.length() == 0)
        remove(this->path.c_str());

    if (config.locations[index_location]->authentication && _userData.signIn == false)
        return error_message(511);

    addHeaderStatus(200);
    addHeaderServer();
    addHeaderDate();
    addHeaderContentLanguage();
    addHeaderContentLocation();
    addHeaderSetCookie();
    this->response.append(lengthHeader);
    this->response.append("Location: ");
    this->response.append(this->location_path);
    this->response.append("\r\n");
    std::cout << PURPLE << "RESPONSE" << BW << std::endl << this->response << BW << std::endl;
    this->response.append(body);
}

char **             Handler::create_env(void)
{
    char **env;
    std::vector<std::string> envs;
    int envNum;

    add_env(&envs);
    envNum = envs.size();

    if (!(env = (char **)calloc(envNum + 1, sizeof(char*))))
        return NULL;
    env[envNum] = NULL;

    for (int i = 0; i < envNum; i++)
    {
        if (!(env[i] = ft_strdup(envs[i].c_str())))
        {
            ft_free_array(env);
            return (NULL);
        }
    }
    return env;
}

void                Handler::add_env(std::vector<std::string> * envs)
{
    std::string contentType;
    if (request.headers->find("Content-Type") != request.headers->end())
        contentType = request.headers->find("Content-Type")->second;

    envs->push_back("AUTH_TYPE=Anonymous");
    envs->push_back("CONTENT_LENGTH=" + lltostr(request.body.length(), 10));
    envs->push_back("CONTENT_TYPE=" + contentType);
    envs->push_back("GATEWAY_INTERFACE=CGI/1.1");
    envs->push_back("PATH_INFO=" + request.path);
    envs->push_back("PATH_TRANSLATED=" + this->path);
    envs->push_back("QUERY_STRING=" + request.formData);
    envs->push_back("REMOTE_ADDR=");
    envs->push_back("REMOTE_IDENT=");
    envs->push_back("REMOTE_USER=" + _userData.login);
    envs->push_back("REQUEST_METHOD=" + request.method);
    envs->push_back("REQUEST_URI=" + request.path);
    envs->push_back("SCRIPT_NAME=" + config.locations[index_location]->cgi_name);
    envs->push_back("SERVER_NAME=" + config.server_name);
    envs->push_back("SERVER_PORT=" + lltostr(config.port[0], 10)); // hardcode
    envs->push_back("SERVER_PROTOCOL=HTTP/1.1");
    envs->push_back("SERVER_SOFTWARE=Webserv/1.1");

    std::multimap<std::string, std::string>::iterator it = request.headers->begin();
    for (; it != request.headers->end(); it++)
        envs->push_back("HTTP_" + it->first + "=" + it->second);
}

int                 Handler::launchCgi(char **args, char **env, std::string * body)
{
     int     status = 0;
    pid_t   pid;

	int in = -1;
    int out = -1;
    while (in < 0 )
    {
        in = open(this->path.c_str(), O_RDWR);
        if (in >= 0)
            break;
        std::cout << "Please clean memory" << std::endl;
        getchar();
    }
    while (out < 0)
    {
        out = open(this->tmp.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (out >= 0)
            break;
        std::cout << "Please clean memory" << std::endl;
        getchar();
    }

    pid = fork();
    if (pid == 0)
    {
		int     std_input = dup(0);
   		int     std_output = dup(1);
        dup2(out, 1);
        dup2(in, 0);
        if (execve(args[0], args, env) == -1)
        {
            close(in);
            close(out);
            dup2(std_input, 0);
            dup2(std_output, 1);
            exit(EXIT_FAILURE);
        }
    }
    else if (pid < 0)
    {
        error_message(500);
        status = 1;
    }
    else
    {
        int stat;
        waitpid(pid, &stat, WUNTRACED);
        while (!WIFEXITED(stat) && !WIFSIGNALED(stat))
            waitpid(pid, &stat, WUNTRACED);
		if ((status = WEXITSTATUS(stat)) != 0)
		{
			error_message(500);
			close(in);
    		close(out);
			return status;
		}
        status = readCgi(body);
        if (status == 1)
            error_message(500);
    }
	close(in);
    close(out);
    return status;
}

int                 Handler::readCgi(std::string * body)
{
    char buffer[INBUFSIZE] = "";
    size_t offset = 0;
    int status = 0;
    int res = 0;

    if (!isCgiReading) {
        cgiFd = -1;
        while (cgiFd < 0) {
            cgiFd = open(this->tmp.c_str(), O_RDONLY);
            if (cgiFd >= 0)
                break;
            std::cout << "Please clean memory" << std::endl;
            getchar();
        }
    }

    if ((res = read(cgiFd, buffer, INBUFSIZE - 1)) > 0) {
        buffer[res] = 0;
        if (!isCgiReading) {
            std::string temp(buffer);
            offset = temp.find("\r\n\r\n");
            if (offset != std::string::npos) {
                body->append(temp, 0, offset);
                body->append("\r\n\r\n");
                offset += 4;
            }
            else
                offset = 0;
            if (res - offset > 0) {
                body->append(lltostr(res - offset, 16));
                body->append("\r\n");
                body->append(temp, offset, res - offset);
                body->append("\r\n");
            }
            isCgiReading = true;
        }
        else
        {
            body->append(lltostr(res, 16));
            body->append("\r\n");
            body->append(buffer, 0, res);
            body->append("\r\n");
        }
    }
    else {
        if (res == 0)
            body->append("0\r\n\r\n");
        else
        {
            status = 1;
            error_message(500);
        }
        isCgiReading = false;
        close(cgiFd);
        remove(this->tmp.c_str());
        cgiFd = -1;
    }
    return status;
}

/*
 * ----------------------------------------- PUT --------------------------------------
 */

void                Handler::handle_put(void)
{
    int fd;
    int status_code = 204;

    fd = open(this->path.c_str(), O_RDWR);
    if (fd < 0)
        status_code = 201;
    else
        close(fd);

    std::ofstream ofs(this->path.c_str(), std::ios_base::trunc);
    if (!ofs.good())
        return error_message(500);

    ofs << request.body;
    ofs.close();

    addHeaderStatus(status_code);
    addHeaderServer();
    addHeaderDate();
    addHeaderContentLanguage();
    addHeaderContentLocation();
    this->response.append("Content-Type: text/plain\r\n");
    addHeaderContentLength(lltostr(request.body.size(), 10));
    addHeaderLocation();
    this->response.append("\r\n");
    std::cout << PURPLE << "RESPONSE" << BW << std::endl << this->response << BW << std::endl;
    if (status_code == 201)
        this->response.append(request.body);
}

/*
 * ---------------------------------------- DELETE --------------------------------------
 */

void                Handler::handle_delete(void)
{
    int     status = 200;
    bool    deleteAccess = OFF;

    std::string truePath = "./content/website1/users/";
    truePath += _userData.login;
    truePath += "/";

    size_t pos = this->path.find(truePath, 0);

    if (_userData.signIn == true && pos == 0)
        deleteAccess = ON;
    else
        status = 403;

    if (deleteAccess == OFF)
        return error_message(status);
    else
    {
        if (remove(this->path.c_str()) != 0 )
            return error_message(404);
        std::string body = "<html>\n<body>\n<h1>File deleted.</h1>\n</body>\n</html>";
        addHeaderStatus(status);
        addHeaderServer();
        addHeaderDate();
        addHeaderContentLanguage();
        addHeaderContentLocation();
        this->response.append("Content-Type: text/html\r\n");
        addHeaderContentLength(std::to_string(body.length()));
        addHeaderLocation();
        addHeaderLastModified();
        this->response.append("\r\n");
        this->response.append(body);
        std::cout << PURPLE << "RESPONSE" << BW << std::endl << this->response << BW << std::endl;
    }
}

/*
 * ------------------------- ERRORS ----------------------------
 */

int         Handler::isRequestCorrect(void)
{
    std::string methods = "GET, HEAD, PUT, POST, DELETE";
    index_location = isLocation(config.locations, request.path);
    int status_code = 0;

    if (methods.find(request.method) == std::string::npos)
        status_code = 501;
    else if (!doesLocationAnswersMethod())
        status_code = 405;
    else if (index_location < 0)
        status_code = 404;
    else if (request.version != "HTTP/1.1" && request.version != "HTTP/1.0")
        status_code = 505;
    else if (request.headers->count("Host") > 1)
        status_code = 400;
    else if (config.locations[index_location]->maxBody > 0 && (int)request.body.length() > config.locations[index_location]->maxBody)
        status_code = 413;
    else if (request.method != "POST" && config.locations[index_location]->authentication && _userData.signIn == false)
        status_code = 511;

    if (status_code != 0)
    {
        error_message(status_code);
        return 0;
    }
    return 1;
}

int         Handler::isLocation(std::vector<location *> locations, std::string path)
{
    int theBestLocation = -1;

    int tmp = -1;
    int flag = 1;
    for(size_t i = 0; i < locations.size(); i++)
    {
        if(locations[i]->path == path)
        {
            theBestLocation = i;
            return (theBestLocation);
        }
        else
        {
            std::string locPath = locations[i]->path;
            std::string reqPath = path;
            std::string locTmp = "";
            std::string reqTmp = "";
            int res = 1;
            for(size_t j = 0; j < locPath.length(); j++)
            {
                locTmp.push_back(locPath[j]);
                reqTmp.push_back(reqPath[j]);
                if(locPath[j] == '*' && flag > 0)
                {
                    flag = isFiles(reqPath, locPath);
                    if((theBestLocation == -1 && flag != -1) || flag > tmp)
                    {
                        tmp = flag;
                        theBestLocation = i;
                    }
                    break ;
                }
                if(locPath[j] == '/')
                    searchPath(locTmp, reqTmp,locPath,j,i,theBestLocation,reqPath,locations);
                else if(j == locPath.length() - 1)
                    res  = searchlocPath(locTmp, reqTmp, j, i, theBestLocation, locations, reqPath, flag, locPath);
                else if(j == reqPath.length() - 1)
                    res  = searchreqPath(locTmp, reqTmp, j, i, theBestLocation, locations, reqPath, locPath);
                if(res == 0)
                    break ;
            }
        }
    }
    return(theBestLocation);
}

int         Handler::isFiles(std::string path, std::string locPath)
{
    std::string expansion = "";
    std::string tmp = "";
    int flag = 0;
    for(size_t i = 0; i < path.length(); i++)
    {
        if(path[i] == '.' || flag == 1)
        {
            expansion.push_back(path[i]);
            flag = 1;
        }
    }
    if(locPath == "*" && expansion.length() > 1)
        return(1);
    else if(locPath[1] == '.' && expansion.length() > 1)
    {
        size_t pos = locPath.find('.');
        if(pos != std::string::npos)
        {
            tmp = locPath.substr(pos, locPath.length() - 1);
            if(tmp == expansion)
                return(2);
        }
    }
    return (-1);
}

void        Handler::searchPath(std::string &locTmp, std::string &reqTmp, std::string &locPath,size_t &j,size_t i, int &theBestLocation, std::string &reqPath, std::vector<location *> locations)
{
    if(locTmp == reqTmp &&  (j == locPath.length() - 1 || j == reqPath.length() - 1))
        theBestLocation = putVal(locTmp, j, i, theBestLocation, locations);
    if(locTmp == reqTmp)
    {
        locTmp.clear();
        reqTmp.clear();
        reqPath.erase(0, j + 1);
        locPath.erase(0, j + 1);
        j = -1;
    }
}

int         Handler::searchlocPath(std::string &locTmp, std::string &reqTmp, size_t &j,size_t i, int &theBestLocation,  std::vector<location *> locations, std::string &reqPath, int flag, std::string &locPath)
{
    flag = 0;
    reqPath.length();
    locPath.length();
    if(locTmp == reqTmp )
    {
        if(reqPath[j + 1] == '/' || j == reqPath.length() - 1 || locPath == "/")
        {
            theBestLocation = putVal(locTmp, j, i, theBestLocation, locations);
            return(1);
        }
    }
    return(0);
}

int         Handler::searchreqPath(std::string &locTmp, std::string &reqTmp, size_t &j,size_t i, int &theBestLocation, std::vector<location *> locations, std::string &reqPath, std::string &locPath)
{
    if(locTmp == reqTmp)
    {
        if(locPath[j + 1] == '/' || j == locPath.length() - 1 || reqPath == "/")
        {
            if (theBestLocation == -1 || config.locations[theBestLocation]->path == "/")
            {
                theBestLocation = putVal(locTmp, j, i, theBestLocation, locations);
                return 1;
            }
            if (config.locations[theBestLocation]->path.length() - reqTmp.length() < config.locations[i]->path.length() - reqTmp.length())
                return 0;

        }
    }
    return(0);
}

int         Handler::putVal(std::string locPath,size_t j,size_t i, int theBestLocation, std::vector<location *> locations)
{
    if((theBestLocation == -1 && j == locPath.length() - 1) || (theBestLocation >= 0 &&
                                                                locations[i]->path.length() > locations[theBestLocation]->path.length()))
        return(i);
    return (theBestLocation);
}

int         Handler::doesLocationAnswersMethod(void)
{
    std::vector<std::string> methods = config.locations[index_location]->method;
    for (size_t i = 0; i < methods.size(); i++) {
        if (methods[i] == request.method)
            return 1;
    }
    return 0;
}

void        Handler::error_message(int const & status_code)
{
    std::string message;
    this->response.append("HTTP/1.1 ");

    switch(status_code)
    {
        case 400:
            this->response.append("400 Bad Request\r\n");
            break;
        case 401:
            this->response.append("401 Unauthorized\r\n");
            this->response.append("WWW-Authenticate: Basic realm=\"Access to the staging site\", charset=\"UTF-8\"\r\n");
            this->response.append("Content-Disposition: inline\r\n");
            break;
        case 403:
            this->response.append("403 Forbidden\r\n");
            break;
        case 404:
            this->response.append("404 Not found\r\n");
            break;
        case 405:
            this->response.append("405 Method Not Allowed\r\n");
            addHeaderAllow();
            break;
        case 413:
            this->response.append("413 Payload Too Large\r\n");
            break;
        case 500:
            this->response.append("500 Internal Server Error\r\n");
            break;
        case 501:
            this->response.append("501 Not Implemented\r\n");
            addHeaderAllow();
            break;
        case 505:
            this->response.append("505 HTTP Version Not Supported\r\n");
            break;
        case 511:
            this->response.append("511 Network Authentication Required\r\n");
            this->response.append("WWW-Authenticate: Basic realm=\"Access to the staging site\", charset=\"UTF-8\"\r\n");
            this->response.append("Content-Disposition: inline\r\n");
            break;
    }

    std::string body;
    std::string errorPagePath = "." + config.error_page;
    loadBodyFromFile(&body, errorPagePath);

    size_t pos = body.find("<?php errorStatus(); ?>", 0);
    if (pos != std::string::npos)
    {
        std::string errorStatus = "<h1></h1><h2></h2>";
        errorStatus.insert(13, message);
        errorStatus.insert(4, std::to_string(status_code));
        body.erase(pos, 23);
        body.insert(pos, errorStatus);
    }

    addHeaderServer();
    addHeaderDate();
    addHeaderContentLanguage();
    this->response.append("Content-Type: text/html\r\n");
    this->response.append("Location: /error_page/\r\n");
    this->response.append("Content-Location: /error_page/\r\n");
    addHeaderContentLength(std::to_string(body.length()));
    this->response.append("\r\n");
    std::cout << this->response  << std::endl;
    this->response.append(body);
    return ;
}

/*
 * ------------------------- ADD HEADERS ----------------------------
 */

void Handler::addHeaderStatus(int status)
{
    this->response.append("HTTP/1.1 ");
    switch (status) {
        case 200:
            this->response.append("200 OK\r\n");
            break;
        case 201:
            this->response.append("201 Created\r\n");
            break;
        case 204:
            this->response.append("204 No Content\r\n");
            break;
        case 301:
            this->response.append("301 Moved Permanently\r\n");
            break;
        case 308:
            this->response.append("308 Permanent Redirect\r\n");
            break;
        case 404:
            this->response.append("404 Not Found\r\n");
            break;
    }
}
void Handler::addHeaderServer() { this->response.append("Server: Webserv/1.1\r\n"); }
void Handler::addHeaderDate()
{
    this->response.append("Date: ");
    this->response.append(getPresentTime());
    this->response.append("\r\n");
}
void Handler::addHeaderSetCookie()
{
    if (_userData.login.length() != 0)
    {
        this->response.append("Set-Cookie: ");
        this->response.append("login=" + _userData.login + "; ");
        this->response.append("Expires=Thu, 26 Oct 2021 00:00:00 GMT; ");
//        this->response.append("Max-Age=; ");
        this->response.append("Path=/; ");
        this->response.append("Domain=" + config.ip_str);
        this->response.append("\r\n");
    } else
    {
        this->response.append("Set-Cookie: ");
        this->response.append("login=deleted; ");
        this->response.append("Expires=Thu, 01 Jan 1970 00:00:00 GMT; ");
        this->response.append("Max-Age=0; ");
        this->response.append("Path=/; ");
        this->response.append("Domain=" + config.ip_str);
        this->response.append("\r\n");
    }
}
void Handler::addHeaderContentLanguage() { this->response.append("Content-Language: en\r\n"); }
void Handler::addHeaderContentLocation()
{
    this->response.append("Content-Location: ");
    this->response.append(request.path);
    this->response.append("\r\n");
}
void Handler::addHeaderContentLength(std::string size)
{
    this->response.append("Content-Length: ");
    this->response.append(size);
    this->response.append("\r\n");
}
void Handler::addHeaderLocation(void)
{
    this->response.append("Location: ");

    if (config.locations[index_location]->repeat_redirect == true)
        this->response.append(config.locations[index_location]->redirect);
    else
        this->response.append(this->location_path);
    this->response.append("\r\n");
}
void Handler::addHeaderContentType(void)
{
    this->response.append("Content-Type: ");

    if (this->request.path.find(".png", 0) != std::string::npos)
        this->response.append("image/png");
    if (this->request.path.find(".jpeg", 0) != std::string::npos)
        this->response.append("image/jpeg");
    else
        this->response.append("text/html");
    this->response.append("\r\n");
}
void Handler::addHeaderLastModified(void)
{
    this->response.append("Last-Modified: ");
    this->response.append(this->lastModTime);
    this->response.append("\r\n");
}
void Handler::addHeaderAllow(void)
{
	location * loc = config.locations[index_location];
	size_t size = loc->method.size();
	
	this->response.append("Allow: ");
	for (size_t i = 0; i < size; i++)
	{
		this->response.append(loc->method[i].c_str());
		if (i != size - 1)
			this->response.append(", ");
	}
	this->response.append("\r\n");
}

/*
 * ------------------------------ LIBFT ------------------------------
 */

int             Handler::ft_strlen(const char *str)
{
    int i;

    i = 0;
    while (str[i] != '\0')
        i++;
    return (i);
}

void		    Handler::ft_free_array(char **to_free)
{
    char	**tmp;

    tmp = to_free;
   while (*tmp != NULL)
   {
       free(*tmp);
       tmp++;
   }
   free(to_free);
   to_free = NULL;
}

char *          Handler::ft_strdup(const char *s)
{
    char	*res;
    int		i;
    int		len;

    len = ft_strlen((char *)s);
    if ((res = (char *)malloc(len + 1)) == NULL)
        return (NULL);
    i = 0;
    while (len)
    {
        res[i] = s[i];
        i++;
        len--;
    }
    res[i] = '\0';
    return (res);
}

std::string     Handler::lltostr(long long number, int base)
{
	std::string res;
	
	if (number >= 10)
	{
		while (number > 0)
		{
			if (base == 16 && number % base >= 10)
				res.insert(res.begin(), (number % base + 87));
			else
				res.insert(res.begin(), (number % base + '0'));
			number = number / base;
		}
	}
	else
		res.insert(res.begin(), (number + '0'));
	return res;
}

/*
 * ------------------------------ EXTRA ------------------------------
 */

bool           Handler::loadBodyFromFile(std::string * body)
{
    std::ifstream ifs(this->path.c_str());

    if (!ifs.is_open())
        return false;

    std::stringstream ss;
    ss << ifs.rdbuf();

    body->append(ss.str());
    ss.clear();
    ifs.close();
    return true;
}

bool            Handler::loadBodyFromFile(std::string * body, std::string path)
{
    std::ifstream ifs(path.c_str());

    if (!ifs.is_open())
        return false;

    std::stringstream ss;
    ss << ifs.rdbuf();

    body->append(ss.str());
    ss.clear();
    ifs.close();

    return true;
}

std::string     Handler::getPresentTime(void)
{
    char buffer[80];
    time_t seconds = time(NULL);
    std::string format = "%a, %d %b %Y %I:%M:%S";

    tm* timeinfo = localtime(&seconds);
    strftime(buffer, 80, format.c_str(), timeinfo);
    std::string resultTime = std::string(buffer);
    resultTime = resultTime + " GMT";
    return (resultTime);
}

std::string     Handler::getLastModificationTime(time_t const & time)
{
    tm *ltm = std::localtime(&time);
    char mbstr[100];
    std::strftime(mbstr, 100, "%a, %d %b %Y %I:%M:%S", ltm);
    std::string lastModifTime(mbstr);
    lastModifTime += " GMT";
    return (lastModifTime);
}

/*
 * ------------------------------ GET ------------------------------
 */

int		        Handler::getCgiFd(void) const { return this->cgiFd; }

bool            Handler::isReading(void) const { return this->isCgiReading; }

