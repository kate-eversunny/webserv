/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pvivian <pvivian@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/04 18:31:32 by pvivian           #+#    #+#             */
/*   Updated: 2021/05/04 18:56:58 by pvivian          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "Webserv.hpp"

class Config
{
private:
	long port;
	char * logFile;
	
public:
	long & getPort(void) const
	{
		return this->port;
	}

	char * getLogFile(void) const
	{
		return this->logFile;
	}
	
	void setPort(long const & p)
	{
		this->port = p;
	}

	void setLogFile(char * name)
	{
		this->logFile = name;
	}
};

#endif