/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: achauvie <achauvie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 10:28:50 by achauvie          #+#    #+#             */
/*   Updated: 2026/07/25 15:59:33 by achauvie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <vector>
#include <map>
#include <poll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <csignal>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <arpa/inet.h>

#define BOT_NICK "IRCBot"

#include "Client.hpp"
#include "Channel.hpp"
#include "IRC.hpp"

class Server
{
	private:
		const long		 				_port;
		const std::string				_password;
		const std::string				_serverName;
		int								_serverFd;
		std::vector<struct pollfd>		_pollFd; 
		std::map<int, Client*>			_clients;
		std::map<std::string, Channel*>	_channels;
		Server(void);
		Server(const Server &other);
		Server &operator=(const Server &other);

	public:
		Server(const long port, const std::string &password, const std::string &serverName);
		~Server(void);

		// Process
		void setup(void);
		void addClient(int fd, const std::string &host);
		void removeClient(int fd);
		void run(void);
		void setPollOut(int clientFd, bool enable);
		void sendToClient(Client *client, const std::string &message);
		void broadcastToChannel(Channel *channel, const std::string &message, Client *exclude);
		void broadcastQuit(Client *client, const std::string &reason);
		std::string buildReply(const std::string &code, Client *client, const std::string &message);
		void checkRegistration(Client *client);

		// Commands
		bool handleCommand(Client *client, const s_cmd &cmd);
		void handlePass(Client *client, const s_cmd &cmd);
		void handleNick(Client *client, const s_cmd &cmd);
		void handleUser(Client *client, const s_cmd &cmd);
		void handleJoin(Client *client, const s_cmd &cmd);
		void handlePrivMsg(Client *client, const s_cmd &cmd);
		void handleKick(Client *client, const s_cmd &cmd);
		void handleInvite(Client *client, const s_cmd &cmd);
		void handleTopic(Client *client, const s_cmd &cmd);
		void handleMode(Client *client, const s_cmd &cmd);
		void handlePart(Client *client, const s_cmd &cmd);
		void handleQuit(Client *client, const s_cmd &cmd);

		// Bot (bonus)
		void sendBotReply(Client *client, Channel *channel, const std::string &text);
		void handleBot(Client *client, const std::string &text, Channel *channel);

		// Exceptions
		class CreateSocketException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
		class CreatePollException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
};