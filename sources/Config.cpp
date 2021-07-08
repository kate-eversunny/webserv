#include "Config.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Config::Config()
{
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Config::~Config()
{
}

/*
** --------------------------------- METHODS ----------------------------------
*/
bool                        IsParenthesesOrDash(char c) {
    switch (c) {
        case '\t':
        case '\r':
        case ' ':
        case '\n':
            return true;
        default:
            return false;
    }
}
void				Config::getFile(std::string file)
{
    std::string file_in_str;
	std::string tmp;
;
	if(file.length() != 0)
	{
        std::ifstream fconfig(file);

        if (!fconfig.is_open())
            throw Config::FileNotOpenException();
		while (getline(fconfig, tmp)) {
            file_in_str += tmp;
        }
		file_in_str.erase(std::remove_if(file_in_str.begin(), file_in_str.end(), &IsParenthesesOrDash), file_in_str.end());

		while(file_in_str.length() > 0)
			file_in_str = parseStr(file_in_str);
        fconfig.close();
	}
	else
		throw Config::FileLengthException();
}

std::string				Config::parseStr(std::string str)  // sega
{
    struct configServer *servNode = new configServer;
    std::string tmp;
    std::string save;

    size_t pos = 0;
    initServNode(servNode);
    initConfigEnv();
    while(str.length() > 0)
    {
        if(str[pos] == '{')
        {
            if(tmp != "server")
                throw Config::ServerNameException();
            str.erase(0, pos + 1);
            pos = 0;
            tmp.clear();
        }
        else if(checkTokens(save, tmp, SERVER) == true)
        {
            if(save.empty())
                throw Config::ValueSaveException();
            str.erase(0, pos);
            pos = 0;
            tmp.clear();
            if(save == "location")
                str = parseLocation(str, servNode);
        }
        else if(str[pos] == ';')
        {
            serverTokenSearch(save, tmp, servNode);
            str.erase(0, pos + 1);
            tmp.clear();
            pos = 0;

        }
        else
            tmp.push_back(str[pos++]);
        if(str[pos] == '}')
        {
            str.erase(0, pos + 1);
            if(checkMainValServ(servNode) == true)
            {
                if(servNode->repeat_error_page == false)
                {
                    servNode->error_page = "./content/error.html";
                    if(checkErrorPage(servNode->error_page) == false)
                        throw Config::ErrorPageException();
                }
                this->servers.push_back(servNode);
                break ;
            }
            else
                throw Config::FullServerValueException();
        }
    }
    return (str);
}

std::string				Config::parseLocation(std::string str,  configServer *servNode)
{
    struct location *locNode = new location;
    std::string tmp;
    std::string save;

    size_t pos = 0;
    initLocNode(locNode);

    while(str.length() > 0)
    {
        if(str[pos] == '{')
        {
            if(tmp.empty() || tmp[0] != '/' || checkLockPath(tmp) == false)
                throw Config::IncorrectLocationPathException();
            locNode->path.assign(tmp);
            locNode->repeat_path = true;
            str.erase(0, pos + 1);
            pos = 0;
            tmp.clear();
        }
        else if(checkTokens(save, tmp, LOCATION) == true)
        {
            if(save == "index" && str[pos] == '.')
            {
                tmp.push_back(str[pos++]);
                continue ;
            }
            if(save.empty() || (save == "method" && locNode->repeat_method == true))
                throw Config::MethodException();
            str.erase(0, pos);
            pos = 0;
            tmp.clear();
        }
        else if(str[pos] == ';')
        {
            locTokenSearch(save, tmp, locNode);
            if(save == "method")
                locNode->repeat_method = true;
            str.erase(0, pos + 1);
            tmp.clear();
            pos = 0;
        }
        else if(checkTokens(save, tmp, METHOD) == true)
        {
            locTokenSearch(save, tmp, locNode);
            locNode->repeat_method = true;
            str.erase(0, pos);
            tmp.clear();
            pos = 0;
        }
        else
            tmp.push_back(str[pos++]);
        if(str[pos] == '}')
        {
            str.erase(0, pos + 1);
            if(checkMainValLoc(locNode) == true)
            {
                if(locNode->repeat_index == false) {
                    locNode->index = ""; }
                servNode->locations.push_back(locNode);
                break ;
            }
            else
                throw Config::FullLocationValueException();
        }
    }
    return(str);
}

void Config::locTokenSearch(std::string save, std::string tmp, location *locNode)
{
    if(save == "maxBody")
    {
        if(locNode->repeat_maxBody == true)
            throw Config::MaxBodyException();
        locNode->maxBody = std::stoi(tmp);
        if(locNode->maxBody > 1000000 || locNode->maxBody < 0)
            throw Config::MaxBodyException();
        locNode->repeat_maxBody = true;
    }
    else if(save == "root")
    {
        if(tmp.empty() || tmp[0] != '/' || locNode->repeat_root == true)
            throw Config::RootException();
        locNode->root.assign(tmp);
        locNode->repeat_root = true;

//        if (locNode->path == "/")
//            locNode->mini_root = locNode->root;
//        else if ((locNode->root.find(locNode->path, 0)) != std::string::npos)
//            locNode->mini_root = locNode->root.substr(0, locNode->root.length() - locNode->path.length());
//        else
//            locNode->mini_root = locNode->root;
    }
    else if(save == "index")
    {
        if(tmp.empty() || locNode->repeat_index == true || checkIndex(locNode->root,tmp) == false)
            throw Config::IndexException();
        locNode->index.assign(tmp);
        locNode->repeat_index = true;
    }
    else if(save == "redirect")
    {
        if(tmp.empty() || locNode->repeat_redirect == true || checkRedirect(tmp) == false)
            throw Config::RedirectException();
        locNode->redirect.assign(tmp);
        locNode->repeat_redirect = true;
    }
    else if(save == "cgi")
    {
        if(tmp.empty() || locNode->repeat_cgi == true || checkCgi(tmp) == false)
            throw Config::CGIException();
        locNode->cgi.assign(tmp);

        size_t  pos = locNode->cgi.rfind("/", std::string::npos); // "cgi_tester";
        locNode->cgi_name = locNode->cgi.substr(pos + 1, locNode->cgi.length());

        locNode->repeat_cgi = true;
    }
    else if(save == "method")
    {
        if(tmp.empty() || checkTokens(save, tmp, METHOD) == false)
            throw Config::MethodException();
        locNode->method.push_back(tmp);
    }
    else if(save == "autoindex")
    {
        if(tmp.empty() || locNode->repeat_autoIndex == true)
            throw Config::AutoindexException();
        if(tmp == "on")
            locNode->autoIndex = ON;
        else if(tmp == "off")
            locNode->autoIndex = OFF;
        else
            throw Config::AutoindexException();
        locNode->repeat_autoIndex = true;
    }
    else if(save == "authentication")
    {
        if(tmp.empty() || locNode->repeat_authentication == true)
            throw Config::AuthenticationException();
        if(tmp == "on")
            locNode->authentication = ON;
        else if(tmp == "off")
            locNode->authentication = OFF;
        else
            throw Config::AuthenticationException();
        locNode->repeat_authentication = true;
    }
}

void Config::serverTokenSearch(std::string save, std::string tmp, configServer *servNode)
{
    if(save == "listen")
    {
        if(tmp.empty() || servNode->repeat_port == true)
            throw Config::ListenException();
        getPortsAndIP(servNode, tmp);
        if(servNode->port.size() > 1 && check_repeat_ports(servNode) == true)
            throw Config::ListenException();
        servNode->repeat_port = true;
    }
    else if(save == "server_name")
    {
        if(tmp.empty() || servNode->repeat_server_name == true)
            throw Config::ServerNameException();
        servNode->server_name.assign(tmp);
        servNode->repeat_server_name = true;
    }
    else if(save == "error_page")
    {
        if(tmp.empty() || tmp[0] != '/' || servNode->repeat_error_page == true || checkErrorPage(tmp) == false)
            throw Config::ErrorPageException();
        servNode->error_page.assign(tmp);
        servNode->repeat_error_page = true;
    }
}

void					Config::initLocNode(location *locNode)
{
    locNode->index.clear();
    locNode->maxBody = 0;
    locNode->path.clear();
    locNode->method.clear();
    locNode->repeat_method = false;
    locNode->repeat_index = false;
    locNode->repeat_maxBody = false;
    locNode->repeat_path = false;
    locNode->repeat_root = false;
	locNode->repeat_cgi  = false;
    locNode->root.clear();
//    locNode->mini_root.clear();
    locNode->autoIndex = -1;
    locNode->repeat_autoIndex = false;
    locNode->authentication = false;
    locNode->repeat_authentication = false;
    locNode->redirect.clear();
    locNode->repeat_redirect = false;
    locNode->cgi.clear();
    locNode->repeat_cgi = false;
    locNode->cgi_name.clear();
}

void					Config::initServNode(configServer *servNode)
{
    servNode->error_page = "";
    servNode->port.clear();
    servNode->server_name = "";
    servNode->repeat_error_page = false;
    servNode->repeat_port = false;
    servNode->repeat_server_name = false;
    servNode->ip = -1;
    servNode->ip_str.clear();
}

bool				Config::checkTokens(std::string &save, std::string str, int config_part)
{
    if (config_part == SERVER)
    {
        for(unsigned int i = 0; i < this->serverTokens.size(); i++)
        {
            if(this->serverTokens[i] == str)
            {
                save = this->serverTokens[i];
                return (true);
            }
        }
        return (false);
    }
    else if(config_part == METHOD)
    {
        for(unsigned int i = 0; i < this->methodTokens.size(); i++)
            if(this->methodTokens[i] == str)
                return (true);
        return (false);
    }
    else
    {
        for(unsigned int i = 0; i <  this->locationTokens.size(); i++)
        {
            if(this->locationTokens[i] == str)
            {
                save = this->locationTokens[i];
                return (true);
            }
        }
        return (false);
    }
}
/*
** --------------------------------- HELP METODS ---------------------------------
*/

bool Config::check_repeat_ports(configServer *servNode)
{
    for(size_t i = 0; i < servNode->port.size(); i++)
    {
        for(size_t j = 0; j < servNode->port.size(); j++)
        {
            if(j != i && servNode->port[j] == servNode->port[i])
                return (true);
        }
    }
    return (false);
}

void Config::initConfigEnv()
{
    this->serverTokens.push_back("listen");
    this->serverTokens.push_back("server_name");
    this->serverTokens.push_back("error_page");
    this->serverTokens.push_back("location");

    this->locationTokens.push_back("index");
    this->locationTokens.push_back("root");
    this->locationTokens.push_back("maxBody");
    this->locationTokens.push_back("method");
    this->locationTokens.push_back("autoindex");
    this->locationTokens.push_back("authentication");
    this->locationTokens.push_back("cgi");
    this->locationTokens.push_back("redirect");

    this->methodTokens.push_back("GET");
    this->methodTokens.push_back("POST");
    this->methodTokens.push_back("PUT");
    this->methodTokens.push_back("HEAD");
    this->methodTokens.push_back("DELETE");
}

bool				Config::checkMainValLoc(struct location *locNode)
{
    if(locNode->repeat_path == true && locNode->repeat_root == true && locNode->repeat_method == true)
        return (true);
    else
        return (false);
}

bool				Config::checkMainValServ(struct configServer *servNode)
{
    if(servNode->ip >= 0 && servNode->repeat_port == true)
        return (true);
    else
        return (false);
}

bool Config::checkLockPath(std::string path)
{
    std::string tmp = "";
    int index = path.find('*');
    if(index > 0)
    {
        tmp = path.substr(index, path.length() - 1);
        if(!tmp.empty() && tmp.length() > 1 &&  tmp[0] == '*' &&  tmp[1] != '.')
            return(false);
    }
    return (true);
}

bool Config::checkErrorPage(std::string path)
{
    path = "." + path;
    std::ifstream err_page(path);
    std::string tmp = "";
    int index = path.find('.', 1);
    if(index > 0)
    {
        tmp = path.substr(index, path.length() - 1);
        if(tmp.length() == 1 || !err_page.is_open())
            return(false);
    }
    else if(index < 0)
        return(false);
    err_page.close();
    return (true);
}

bool Config::checkIndex(std::string root, std::string indexPath)
{

    std::string tmp = "";
    if(root[root.length() - 1] == '/')
        indexPath = '.' + root + indexPath;
    else
        indexPath =  '.' + root + '/' + indexPath;

    std::ifstream fconfig(indexPath);

    int index = 1;
    if(index > 0)
    {
        tmp = indexPath.substr(index,indexPath.length() - 1);
        if(tmp.length() == 1 || !fconfig.is_open())
            return(false);
    }
    else if(index < 0)
        return(false);
    fconfig.close();
    return (true);
}

bool Config::checkRedirect(std::string redirectPath)
{
    if(redirectPath.empty())
        return (false);
    return (true);
}

bool Config::checkCgi(std::string cgiPath)
{
    std::string tmp = "";
    std::ifstream fconfig(cgiPath);
    int index = 1;
    if(index > 0)
    {
        tmp = cgiPath.substr(index,cgiPath.length() - 1);
        if(tmp.length() == 1 || !fconfig.is_open())
            return(false);
    }
    else if(index < 0)
        return(false);
    fconfig.close();
    return (true);
}

void  Config::getPortsAndIP(configServer *servNode, std::string portsStr)
{
    int tmpPort;
    std::string tmp;

    for(size_t i = 0; i < portsStr.length(); i++)
    {
        if(portsStr[i] > '9' ||  portsStr[i] < '0')
        {
            if (portsStr[i] != ',' && portsStr[i] != ':')
                throw Config::PortIPException();
        }
        if(portsStr[i] == ':')
        {
            if(!tmp.empty())
            {
                tmpPort = std::stoi(tmp);
                if(tmpPort > 65535 || tmpPort < 0)
                    throw Config::PortIPException();
                servNode->port.push_back( htons(tmpPort));
                tmp.clear();
            }
            while(++i < portsStr.length())
            {
                tmp.push_back(portsStr[i]);
            }
            if(tmp.empty())
                throw Config::PortIPException();
            servNode->ip = inet_addr(tmp.c_str());
            servNode->ip_str = tmp;
            continue;
        }
        if( portsStr[i] == ',')
        {
            tmpPort = std::stoi(tmp);
            if(tmpPort > 65535 || tmpPort < 0)
                throw Config::PortIPException();
            servNode->port.push_back(htons(tmpPort));
            tmp.clear();
            continue;
        }
        tmp.push_back(portsStr[i]);
    }
}


/*
** --------------------------------- GETTERS AND SETTERS ---------------------------------
*/
	configServer        *Config::getconfigServer(int index) const
	{
		return(servers[index]);
	}
	std::vector<configServer*> Config::getAllServers() const
    {
        return servers;
    }

	size_t		Config::getSize(void) const
	{
		return this->servers.size();
	}

/*
** --------------------------------- EXCEPTIONS ----------------------------------
*/
const char   *Config::FileNotOpenException::what() const throw() { return ("File not open"); }
const char   *Config::FileLengthException::what() const throw() { return ("FileLengthException"); }
const char   *Config::ServerNameException::what() const throw() { return ("ServerNameException"); }
const char   *Config::ValueSaveException::what() const throw() { return ("ValueSaveException"); }
const char   *Config::ErrorPageException::what() const throw() { return ("ErrorPageException"); }
const char   *Config::FullServerValueException::what() const throw() { return ("FullServerValueException"); }
const char   *Config::IncorrectLocationPathException::what() const throw() { return ("IncorrectLocationPathException"); }
const char   *Config::MethodException::what() const throw() { return ("MethodException"); }
const char   *Config::FullLocationValueException::what() const throw() { return ("FullLocationValueException"); }
const char   *Config::MaxBodyException::what() const throw() { return ("MaxBodyException"); }
const char   *Config::RootException::what() const throw() { return ("RootException"); }
const char   *Config::IndexException::what() const throw() { return ("IndexException"); }
const char   *Config::RedirectException::what() const throw() { return ("RedirectException"); }
const char   *Config::CGIException::what() const throw() { return ("CGIException"); }
const char   *Config::AutoindexException::what() const throw() { return ("AutoindexException"); }
const char   *Config::AuthenticationException::what() const throw() { return ("AuthenticationException"); }
const char   *Config::ListenException::what() const throw() { return ("ListenException"); }
const char   *Config::PortIPException::what() const throw() { return ("PortIPException"); }

/* ************************************************************************** */
