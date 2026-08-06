/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: achauvie <achauvie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 10:28:50 by achauvie          #+#    #+#             */
/*   Updated: 2026/07/24 09:39:33 by achauvie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <vector>
#include <iostream>

class Client
{
	private:
		const int	_clientFd;
		std::string	_clientBuffer;
		std::string	_clientOutBuffer;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _hostname;
		bool		_passOk;
		bool		_hasNick;
		bool		_hasUser;
		bool		_welcomeSent;
		Client(void);
		Client(const Client &other);
		Client &operator=(const Client &other);

	public:
		Client(int clientFd, const std::string &host);
		~Client(void);
		
		// Process
		void appendClientBuffer(const std::string &buffer);
		void appendClientOutBuffer(const std::string &buffer);
		void consumeOutBuffer(size_t n);
		std::vector<std::string> extractLines(void);

		// Getters
		int getClientFd(void) const;
		const std::string &getClientBuffer(void) const;
		const std::string &getClientOutBuffer(void) const;
		const std::string &getNickname(void) const;
		const std::string &getUsername(void) const;
		const std::string &getRealname(void) const;
		const std::string &getHostname(void) const;
		bool getWelcomeSent(void) const;
		bool isRegistered(void) const;
		std::string getPrefix(void) const;

		// Setters
		void setNickname(const std::string &value);
		void setUsername(const std::string &value);
		void setRealname(const std::string &value);
		void setHostname(const std::string &value);
		void setPassOk(bool value);
		void setWelcomeSent(bool value);
};