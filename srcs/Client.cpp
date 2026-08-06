/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: achauvie <achauvie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 10:29:36 by achauvie          #+#    #+#             */
/*   Updated: 2026/07/24 09:42:05 by achauvie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd, const std::string &host): _clientFd(fd), _hostname(host), _passOk(false), _hasNick(false), _hasUser(false), _welcomeSent(false)
{}

Client::~Client(void)
{}

//         //
// Process //
//         //
void Client::appendClientBuffer(const std::string &buffer)
{
	if (!buffer.empty())
		_clientBuffer += buffer;
}

void Client::appendClientOutBuffer(const std::string &buffer)
{
	if (!buffer.empty())
		_clientOutBuffer += buffer;
}

void Client::consumeOutBuffer(size_t n)
{
	if (n == 0)
		return;
	_clientOutBuffer.erase(0, n);
}

std::vector<std::string> Client::extractLines()
{
	std::vector<std::string> cmds;

	std::size_t	pos = _clientBuffer.find("\n");
	while (pos != std::string::npos)
	{

		std::string cmd = _clientBuffer.substr(0, pos);
		_clientBuffer = _clientBuffer.substr(pos + 1);
		if (!cmd.empty())
		{
			if (cmd[cmd.size() - 1] == '\r')
				cmd = cmd.substr(0, cmd.size() - 1);
			cmds.push_back(cmd);
		}
		pos = _clientBuffer.find("\n");
	}

	return cmds;
}

//         //
// Getters //
//         //
int Client::getClientFd(void) const
{
	return _clientFd;
}

const std::string &Client::getClientBuffer(void) const
{
	return _clientBuffer;
}

const std::string &Client::getClientOutBuffer(void) const
{
	return _clientOutBuffer;
}

const std::string &Client::getNickname(void) const
{
	return _nickname;
}

const std::string &Client::getUsername(void) const
{
	return _username;
}

const std::string &Client::getRealname(void) const
{
	return _realname;
}

const std::string &Client::getHostname(void) const
{
	return _hostname;
}

bool Client::getWelcomeSent(void) const
{
	return _welcomeSent;
}

bool Client::isRegistered(void) const
{
	return _passOk && _hasNick && _hasUser;
}

std::string Client::getPrefix(void) const
{
	return _nickname + "!" + _username + "@" + _hostname;
}

//         //
// Setters //
//         //
void Client::setNickname(const std::string &value)
{
	_nickname = value;
	_hasNick = true;
}

void Client::setUsername(const std::string &value)
{
	_username = value;
	_hasUser = true;
}

void Client::setRealname(const std::string &value)
{
	_realname = value;
}

void Client::setHostname(const std::string &value)
{
	_hostname = value;
}

void Client::setPassOk(bool value)
{
	_passOk = value;
}

void Client::setWelcomeSent(bool value)
{
	_welcomeSent = value;
}