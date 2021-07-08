#include "ParseRequest.hpp"

ParseRequest::ParseRequest()
{
    _data.nmb      = 0;
    _data.headers  = new std::multimap<std::string, std::string>;
	clearData();
}

ParseRequest::~ParseRequest() { delete _data.headers; }

/*
** --------------------------------- METHODS ----------------------------------
*/

	bool					ParseRequest::addToBuffer(std::string str)
	{
	    if (_data.status == REQUEST_READY)
	       clearData();

        _buff += str;

        this->parseHTTPRequest();

		if (_data.status == REQUEST_READY)
        {
		    if (_data.method.length() == 0)
            {
                _data.status = REQUEST_PARSE;
                if (_buff.length() != 0)
                    return true; // запустить парсинг снова
                return false; // буфер пустой
            }
//		    std::cout << YELLOW << "|" << _data.body << "|" << std::endl << std::endl;
            std::cout << GREEN << "REQUEST_READY" << BW << std::endl << std::endl;
        }
		if (_buff.length() != 0)
            return true; // запустить парсинг снова
        return false; // буфер пустой
	}

	void					ParseRequest::parseHTTPRequest()
	{
		size_t pos;
		std::string tmp;
		
		while (_parsPart != BODY_PART && (pos = _buff.find("\r\n", 0)) != std::string::npos)
		{
		    if (pos == 0)
            {
                _buff.erase(0, 2);
                checkIfBody();
                if (_data.status == REQUEST_READY)
                    return;
                break;
            }

			tmp.clear();
			tmp.insert(0, _buff, 0, pos);
			_buff.erase(0, pos + 2);

			if (_parsPart == START_LINE_PART || _parsPart == HEADERS_PART)
				parseHeaders(tmp);
            else if (_parsPart == PRE_PART)
                parseStartingLine(tmp.c_str());
		}

		int status = 0;

        while  (_parsPart == BODY_PART && _data.status != REQUEST_READY && !status)
        {
            switch (_data.bodyEncryption) {

                case TRANSFER_ENCODING_CHANG:
                    while (_data.status != REQUEST_READY && !status)
                        status = parseBodyChang();
                    break;
                case TRANSFER_ENCODING_COMPRESS:
                    break;
                case TRANSFER_ENCODING_DEFLATE:
                    break;
                case TRANSFER_ENCODING_GZIP:
                    break;
                case TRANSFER_ENCODING_IDENTYTY:
                    break;
                case CONTENT_LENGTH:
                    while (_data.status != REQUEST_READY && !status)
                        status = parseContentLength();
                    break;
            }
        }
	}

	void					ParseRequest::parseStartingLine(std::string head)
	{
        _data.formData.clear();
        std::cout << YELLOW << "REQUEST #" << _data.nmb++ << std::endl << BW;
        _parsPart = START_LINE_PART;
		std::cout << RED << "Starting Line " << BW;
		size_t pos = 0;
		if ((pos = head.find(" ", 0)) != std::string::npos)
		{
			_data.method.insert(0, head, 0, pos);
			head.erase(0, pos + 1);
		}
		if ((pos = head.find(" ", 0)) != std::string::npos)
		{
			_data.path.insert(0, head, 0, pos);
			head.erase(0, pos + 1);
		}
        pos = _data.path.find("?", 0);
		if (pos != std::string::npos)
        {
            _data.formData = _data.path.substr(pos + 1);
            _data.path = _data.path.substr(0, pos);
            _data.method = "POST";
        }
		pos = head.size();
		_data.version.insert(0, head, 0, pos);

		if (_data.formData.length() != 0)
		    std::cout << BLUE << _data.method << " " << _data.path << "?" << _data.formData << " " << _data.version << BW << std::endl << std::endl;
        else
            std::cout << BLUE << _data.method << " " << _data.path << " " << _data.version << BW << std::endl << std::endl;
	}

	void					ParseRequest::parseHeaders(std::string header)
    {
	    _parsPart = HEADERS_PART;

		std::cout << RED << "Header " << BW;

		size_t pos;

		pos = header.find(":", 0);
		std::string key;
		key.clear();
		key.insert(0, header, 0, pos);
		header = header.erase(0, pos + 2);

		pos = header.length();
		std::string value;
		value.clear();
		value.insert(0, header, 0, pos);
	
		_data.headers->insert(std::make_pair(key, value));
		std::cout << BLUE << key << ": " << value << BW << std::endl;
	}

    void					ParseRequest::checkIfBody()
    {
        std::multimap <std::string, std::string>::iterator itCL = _data.headers->find("Content-Length");

        std::multimap <std::string, std::string>::iterator itTE = _data.headers->find("Transfer-Encoding");

        if (itCL != _data.headers->end())
        {
            _data.bodyEncryption = CONTENT_LENGTH;
            _data.bodyLen = stoi(itCL->second, 0, 10);
            _parsPart = BODY_PART;
        }
        else if (itTE != _data.headers->end())
        {
            if (itTE->second == "chunked")
                _data.bodyEncryption = TRANSFER_ENCODING_CHANG;
            else if (itTE->second == "compress")
                _data.bodyEncryption = TRANSFER_ENCODING_COMPRESS;
            else if (itTE->second == "deflate")
                _data.bodyEncryption = TRANSFER_ENCODING_DEFLATE;
            else if (itTE->second == "gzip")
                _data.bodyEncryption = TRANSFER_ENCODING_GZIP;
            else if (itTE->second == "identity")
                _data.bodyEncryption = TRANSFER_ENCODING_IDENTYTY;
            _parsPart = BODY_PART;
        }
        else
            _data.status = REQUEST_READY;
    }

	int				    	ParseRequest::parseBodyChang()
	{
	    /*
	     * <HEX><CRLF>
	     */
	    if (_packetPart == HEX_CRLF_PART)
        {
            if (_buff.find("\r\n", 0) == std::string::npos)
                return 1; // need more data
            int pos = _buff.find("\r\n", 0);
            _data.bodyLen = stoi (_buff, 0, 16);
            _buff.erase(0, pos + 2);
            _packetPart = BODY_CRLF_PART;
        }
        /*
	     * <data>
	     */
        if (_packetPart == BODY_CRLF_PART)
        {
            if ((int)_buff.length() >= _data.bodyLen)
            {
                _data.body.append(_buff, 0, _data.bodyLen);
                _buff.erase(0, _data.bodyLen);
                _packetPart = CRLF_PART;
            }
            else
                return 1;
        }
        if (_packetPart == CRLF_PART)
        {
            if (_buff.find("\r\n", 0) != std::string::npos)
            {
                _buff.erase(0, 2);
                _packetPart = HEX_CRLF_PART;
                if (_data.bodyLen == 0)
                    _data.status = REQUEST_READY;
            } else
                return 1;
        }
        return 0;
	}

	int                     ParseRequest::parseContentLength()
    {
        if ((int)_buff.length() >= _data.bodyLen)
        {
            _data.body.append(_buff, 0, _data.bodyLen);
            _buff.erase(0, _data.bodyLen);
            _data.status = REQUEST_READY;

            std::multimap <std::string, std::string>::iterator itCT = _data.headers->find("Content-Type");
            if (itCT != _data.headers->end() && itCT->second == "application/x-www-form-urlencoded")
            {
                size_t pos = 0;
                std::string tmp = _data.body;
                std::string key;
                std::string value;

                while (tmp.length() > 0)
                {
                    key.clear();
                    value.clear();

                    pos = tmp.find("=", 0);
                    key.insert(0, tmp, 0, pos);
                    tmp = tmp.erase(0, pos + 1);

                    if ((pos = tmp.find("&", 0)) != std::string::npos)
                    {
                        value.insert(0, tmp, 0, pos);
                        tmp = tmp.erase(0, pos + 1);
                    }
                    else
                    {
                        value.insert(0, tmp, 0, tmp.length());
                        tmp.clear();
                    }
                }
            }
        }
        else
            return 1;
        return 0;
    }

	void					ParseRequest::clearData()
	{
        _parsPart = PRE_PART;
        _packetPart = HEX_CRLF_PART;

		_data.method.clear();
		_data.path.clear();
		_data.version.clear();
		_data.headers->clear();
		_data.body.clear();
		_data.bodyLen = -1;
		_data.status = REQUEST_PARSE;
        _data.bodyEncryption = -1;
        _data.formData.clear();
	}

/*
** --------------------------------- GET ----------------------------------
*/

    data                        ParseRequest::getData() const { return _data; }
	bool						ParseRequest::isRequestReady(void) const
	{
		return _data.status == REQUEST_READY;
	}
    std::string             ParseRequest::getHost(void) const
    {
        std::map<std::string, std::string>::iterator it = _data.headers->find("Host");
        return it->second;
    }

    std::string             ParseRequest::getConnection(void) const
{
    std::map<std::string, std::string>::iterator it = _data.headers->find("Connection");
    if (it != _data.headers->end())
        return it->second;
    return ("");
}
