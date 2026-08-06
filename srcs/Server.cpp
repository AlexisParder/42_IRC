/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: achauvie <achauvie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 10:29:36 by achauvie          #+#    #+#             */
/*   Updated: 2026/08/06 11:21:15 by achauvie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const long port, const std::string &password, const std::string &serverName) : _port(port), _password(password), _serverName(serverName)
{}

Server::~Server(void)
{
	for (size_t i = 0; i < _pollFd.size(); i++)
		close(_pollFd[i].fd);

	std::map<std::string, Channel*>::iterator channel = _channels.begin();
	std::map<std::string, Channel*>::iterator channelEnd = _channels.end();
	while (channel != channelEnd)
	{
		delete channel->second;
		channel++;
	}
	_channels.clear();

	std::map<int, Client*>::iterator client = _clients.begin();
	std::map<int, Client*>::iterator clientEnd = _clients.end();
	while (client != clientEnd)
	{
		delete client->second;
		client++;
	}
	_clients.clear();
}

//        //
// Signal //
//        //
volatile sig_atomic_t gStop = 0;

void signalHandler(int sig)
{
	(void)sig;
	gStop = 1; 
}

//         //
// Process //
//         //
void Server::setup(void)
{
	signal(SIGINT, signalHandler);
	signal(SIGPIPE, SIG_IGN);

	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0)
		throw Server::CreateSocketException();

	int opt = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(_serverFd);
		throw Server::CreateSocketException();
	}
	
	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(_serverFd);
		throw Server::CreateSocketException();
	}

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(_serverFd, (sockaddr*)&addr, sizeof(addr)) < 0)
	{
		close(_serverFd);
		throw Server::CreateSocketException();
	}

	if (listen(_serverFd, SOMAXCONN) < 0)
	{
		close(_serverFd);
		throw Server::CreateSocketException();
	}

	pollfd pollTmp;
	pollTmp.fd = _serverFd;
	pollTmp.events = POLLIN;
	_pollFd.push_back(pollTmp);
}

void Server::addClient(int fd, const std::string &host)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(fd);
		return;
	}

	_clients[fd] = new Client(fd, host);

	pollfd pollTmp;
	pollTmp.fd = fd;
	pollTmp.events = POLLIN;
	pollTmp.revents = 0;
	_pollFd.push_back(pollTmp);
}

void Server::removeClient(int fd)
{

	std::map<int, Client*>::iterator client = _clients.find(fd);
	if (client == _clients.end())
		return;

	std::map<std::string, Channel*>::iterator channel = _channels.begin();
	std::map<std::string, Channel*>::iterator channelEnd = _channels.end();
	while (channel != channelEnd)
	{
		channel->second->removeMember(client->second);
		if (channel->second->isEmptyChannel())
		{
			delete channel->second;
			_channels.erase(channel++);
		}
		else
			channel++;
	}

	delete client->second;
	_clients.erase(client);

	for (size_t i = 0; i < _pollFd.size(); i++)
	{
		if (_pollFd[i].fd == fd)
		{
			_pollFd.erase(_pollFd.begin() + i);
			break;
		}
	}
	close(fd);
}

void Server::run(void)
{
	while (!gStop)
	{
		int res = poll(&_pollFd[0], _pollFd.size(), -1);
		if (gStop)
			break;
		if (res < 0)
			throw Server::CreatePollException();

		for (size_t i = 0; i < _pollFd.size(); i++)
		{
			if (_pollFd[i].revents & POLLOUT)
			{
				int clientFd = _pollFd[i].fd;

				if (_clients.find(clientFd) == _clients.end())
					continue;

				Client *client = _clients[clientFd];
				const std::string &buffer = client->getClientOutBuffer();
				if (!buffer.empty())
				{
					ssize_t res = send(client->getClientFd(), buffer.c_str(), buffer.size(), 0);
					if (res > 0)
						client->consumeOutBuffer(res);
				}

				if(client->getClientOutBuffer().empty())
					setPollOut(client->getClientFd(), false);
			}
			if (_pollFd[i].revents & POLLIN)
			{
				if (_pollFd[i].fd == _serverFd)
				{
					struct sockaddr_in	clientAddr;
					socklen_t			len = sizeof(clientAddr);

					int client = accept(_serverFd, (sockaddr*)&clientAddr, &len);
					if (client < 0)
						continue;

					std::string clientHostname = inet_ntoa(clientAddr.sin_addr);
					addClient(client, clientHostname);
					break;
				}
				else
				{
					char buffer[512];
					int clientFd = _pollFd[i].fd;
					
					ssize_t readLen = recv(clientFd, buffer, sizeof(buffer), 0);
					if (readLen > 0)
					{
						if (_clients.find(clientFd) == _clients.end())
							continue;

						Client *client = _clients[clientFd];
						client->appendClientBuffer(std::string(buffer, readLen));
						std::vector<std::string> lines = client->extractLines();
						for (size_t i = 0; i < lines.size(); i++)
						{
							s_cmd cmd = parseCmd(lines[i]);
							if (!handleCommand(client, cmd))
								break;
						}
					}
					else
					{
						int clientFd = _pollFd[i].fd;

						std::map<int, Client*>::iterator client = _clients.find(clientFd);
						if (client != _clients.end())
							broadcastQuit(client->second, "Connection closed");
						removeClient(clientFd);
						break;
					}
				}
			}
		}
	}
}

void Server::setPollOut(int clientFd, bool enable)
{
	for (size_t i = 0; i < _pollFd.size(); i++)
	{
		if (_pollFd[i].fd == clientFd)
		{
			if (enable)
				_pollFd[i].events |= POLLOUT;
			else
				_pollFd[i].events &= ~POLLOUT;
			break;
		}
	}
}

void Server::sendToClient(Client *client, const std::string &message)
{
	client->appendClientOutBuffer(message + "\r\n");

	const std::string &buffer = client->getClientOutBuffer();
	if (!buffer.empty())
	{
		ssize_t res = send(client->getClientFd(), buffer.c_str(), buffer.size(), 0);
		if (res > 0)
			client->consumeOutBuffer(res);
	}

	if(!client->getClientOutBuffer().empty())
		setPollOut(client->getClientFd(), true);
}

void Server::broadcastToChannel(Channel *channel, const std::string &message, Client *exclude)
{
	const std::set<Client*> &members = channel->getClients();
	std::set<Client*>::const_iterator it = members.begin();
	std::set<Client*>::const_iterator ite = members.end();
	while (it != ite)
	{
		Client *member = *it;
		if (!exclude || *it != exclude)
			sendToClient(member, message);
		it++;
	}
}

void Server::broadcastQuit(Client *client, const std::string &reason)
{
	std::map<std::string, Channel*>::iterator channel = _channels.begin();
	std::map<std::string, Channel*>::iterator channelEnd = _channels.end();
	while (channel != channelEnd)
	{
		if (channel->second->isMember(client))
			broadcastToChannel(channel->second, ":" + client->getPrefix() + " QUIT" + (!reason.empty() ? (" :" + reason) : ""), client);
		channel++;
	}
}

std::string Server::buildReply(const std::string &code, Client *client, const std::string &message)
{
	return ":" + _serverName + " " + code + " " + (client->getNickname().empty() ? "*" : client->getNickname()) + " " + message;
}

void Server::checkRegistration(Client *client)
{
	if (client->isRegistered() && !client->getWelcomeSent())
	{
		sendToClient(client, buildReply("001", client, ":Welcome to the IRC network, " + client->getPrefix()));
		client->setWelcomeSent(true);
	}
}

//          //
// Commands //
//          // 
bool Server::handleCommand(Client *client, const s_cmd &cmd)
{
	if (cmd.name.empty())
		return true;

	if (cmd.name == "PASS")
		handlePass(client, cmd);
	else if (cmd.name == "NICK")
		handleNick(client, cmd);
	else if (cmd.name == "USER")
		handleUser(client, cmd);
	else if (cmd.name == "CAP")
		return true;
	else if (!client->isRegistered())
	{
		sendToClient(client, buildReply("451", client, ":You have not registered"));
		return true;
	}
	else if (cmd.name == "JOIN")
		handleJoin(client, cmd);
	else if (cmd.name == "PRIVMSG")
		handlePrivMsg(client, cmd);
	else if (cmd.name == "KICK")
		handleKick(client, cmd);
	else if (cmd.name == "INVITE")
		handleInvite(client, cmd);
	else if (cmd.name == "TOPIC")
		handleTopic(client, cmd);
	else if (cmd.name == "MODE")
		handleMode(client, cmd);
	else if (cmd.name == "PART")
		handlePart(client, cmd);
	else if (cmd.name == "QUIT")
	{
		handleQuit(client, cmd);
		return false;
	}
	return true;
}

void Server::handlePass(Client *client, const s_cmd &cmd)
{
	if (client->isRegistered())
	{
		sendToClient(client, buildReply("462", client, ":You may not reregister"));
		return;
	}
	
	if (cmd.params.empty())
	{
		sendToClient(client, buildReply("461", client, "PASS :Not enough parameters"));
		return;
	}
	
	if (cmd.params[0] == _password)
	{
		client->setPassOk(true);
		checkRegistration(client);
	}
	else
		sendToClient(client, buildReply("464", client, ":Password incorrect"));
}

void Server::handleNick(Client *client, const s_cmd &cmd)
{
	if (cmd.params.empty() || cmd.params[0].empty())
	{
		sendToClient(client, buildReply("431", client, ":No nickname given"));
		return;
	}

	if (std::isdigit(static_cast<unsigned char>(cmd.params[0][0])))
	{
		sendToClient(client, buildReply("432", client, cmd.params[0] + " :Erroneous nickname"));
		return;
	}
	for (size_t i = 0; i < cmd.params[0].size(); i++)
	{
		if (std::isspace(static_cast<unsigned char>(cmd.params[0][i])) || cmd.params[0][i] == ':' || cmd.params[0][i] == '#')
		{
			sendToClient(client, buildReply("432", client, cmd.params[0] + " :Erroneous nickname"));
			return;	
		}
	}
	
	if (ftStrToUpper(cmd.params[0]) == ftStrToUpper(BOT_NICK))
	{
		sendToClient(client, buildReply("433", client, cmd.params[0] + " :Nickname is already in use"));
		return;
	}

	std::map<int, Client*>::iterator it = _clients.begin();
	std::map<int, Client*>::iterator ite = _clients.end();
	while (it != ite)
	{
		if (it->second->getClientFd() == client->getClientFd())
		{
			it++;
			continue;
		}
		if (ftStrToUpper(it->second->getNickname()) == ftStrToUpper(cmd.params[0]))
		{
			sendToClient(client, buildReply("433", client, cmd.params[0] + " :Nickname is already in use"));
			return;
		}
		it++;
	}
	client->setNickname(cmd.params[0]);

	checkRegistration(client);
}

void Server::handleUser(Client *client, const s_cmd &cmd)
{
	if (client->isRegistered())
	{
		sendToClient(client, buildReply("462", client, ":You may not reregister"));
		return;
	}

	if (cmd.params.size() < 4)
	{
		sendToClient(client, buildReply("461", client, "USER :Not enough parameters"));
		return;
	}

	client->setUsername(cmd.params[0]);
	client->setRealname(cmd.params[3]);

	checkRegistration(client);
}

void Server::handleJoin(Client *client, const s_cmd &cmd)
{
	if (cmd.params.empty() || cmd.params[0].empty())
	{
		sendToClient(client, buildReply("461", client, "JOIN :Not enough parameters"));
		return;
	}

	if (cmd.params[0][0] == '0')
	{
		std::vector<std::string> toPart;
		std::map<std::string, Channel*>::iterator channel = _channels.begin();
		std::map<std::string, Channel*>::iterator channelEnd = _channels.end();
		while (channel != channelEnd)
		{
			if (channel->second->isMember(client))
				toPart.push_back(channel->first);
			channel++;
		}
		for (size_t i = 0; i < toPart.size(); i++)
		{
			s_cmd partCmd;
			partCmd.prefix = client->getPrefix();
			partCmd.name = "PART";
			partCmd.params.push_back(toPart[i]);
			handlePart(client, partCmd);
		}
		return;
	}

	if (cmd.params[0][0] != '#' && cmd.params[0][0] != '&')
	{
		sendToClient(client, buildReply("403", client, cmd.params[0] + " :No such channel"));
		return;
	}

	std::map<std::string, Channel*>::iterator channel = _channels.find(cmd.params[0]);
	if (channel == _channels.end())
	{
		_channels[cmd.params[0]] = new Channel(cmd.params[0], client);
		channel = _channels.find(cmd.params[0]);
	}
	else
	{
		if (channel->second->isMember(client))
			return;

		std::string channelKey = channel->second->getKey();
		if (!channelKey.empty() && (cmd.params.size() < 2 || cmd.params[1].empty() || cmd.params[1] != channelKey))
		{
			sendToClient(client, buildReply("475", client, cmd.params[0] + " :Cannot join channel (+k)"));
			return;
		}

		if (channel->second->getInviteOnly() && !channel->second->isInvited(client))
		{
			sendToClient(client, buildReply("473", client, cmd.params[0] + " :Cannot join channel (+i)"));
			return;
		}

		size_t memberLimit = channel->second->getMemberLimit();
		if (memberLimit > 0 && channel->second->countMembers() >= memberLimit)
		{
			sendToClient(client, buildReply("471", client, cmd.params[0] + " :Cannot join channel (+l)"));
			return;
		}

		channel->second->addMember(client);
	}

	if (channel != _channels.end())
	{
		std::string joinMsg = ":" + client->getPrefix() + " JOIN " + cmd.params[0];
		broadcastToChannel(channel->second, joinMsg, NULL);

		std::string channelTopic = channel->second->getTopic();
		if (!channelTopic.empty())
			sendToClient(client, buildReply("332", client, cmd.params[0] + " :" + channelTopic));
		else
			sendToClient(client, buildReply("331", client, cmd.params[0] + " :No topic is set"));

		const std::set<Client*> &members = channel->second->getClients();
		std::set<Client*>::const_iterator membersIt = members.begin();
		std::set<Client*>::const_iterator membersIte = members.end();

		std::string messageMembersList;
		while (membersIt != membersIte)
		{
			if (!messageMembersList.empty())
				messageMembersList += " ";
			if (channel->second->isOperator(*membersIt))
				messageMembersList += "@";
			messageMembersList += (*membersIt)->getNickname();
			membersIt++;
		}
		sendToClient(client, buildReply("353", client, "= " + cmd.params[0] + " :" + messageMembersList));
		sendToClient(client, buildReply("366", client, cmd.params[0] + " :End of /NAMES list"));
	}
}

void Server::handlePrivMsg(Client *client, const s_cmd &cmd)
{
	if (cmd.params.size() < 2)
	{
		sendToClient(client, buildReply("461", client, "PRIVMSG :Not enough parameters"));
		return;
	}

	if (cmd.params[0].empty())
	{
		sendToClient(client, buildReply("411", client, ":No recipient given"));
		return;
	}
	else if (cmd.params[1].empty())
	{
		sendToClient(client, buildReply("412", client, ":No text to send"));
		return;
	}

	if (cmd.params[0][0] == '#' || cmd.params[0][0] == '&')
	{
		std::map<std::string, Channel*>::iterator channel = _channels.find(cmd.params[0]);
		if (channel == _channels.end())
		{
			sendToClient(client, buildReply("403", client, cmd.params[0] + " :No such channel"));
			return;
		}

		if (!channel->second->isMember(client))
		{
			sendToClient(client, buildReply("404", client, cmd.params[0] + " :Cannot send to channel"));
			return;
		}

		broadcastToChannel(channel->second, ":" + client->getPrefix() + " PRIVMSG " + cmd.params[0] + " :" + cmd.params[1], client);

		if (cmd.params[1][0] == '!')
			handleBot(client, cmd.params[1], channel->second);
	}
	else if (ftStrToUpper(cmd.params[0]) == ftStrToUpper(BOT_NICK))
		handleBot(client, cmd.params[1], NULL);
	else
	{
		std::map<int, Client*>::iterator target = _clients.begin();
		std::map<int, Client*>::iterator clientEnd = _clients.end();
		while (target != clientEnd)
		{
			if (ftStrToUpper(target->second->getNickname()) == ftStrToUpper(cmd.params[0]))
			{
				sendToClient(target->second, ":" + client->getPrefix() + " PRIVMSG " + target->second->getNickname() + " :" + cmd.params[1]);
				return;
			}
			target++;
		}
		
		sendToClient(client, buildReply("401", client, cmd.params[0] + " :No such nick"));
	}
}

void Server::sendBotReply(Client *client, Channel *channel, const std::string &text)
{
	if (channel)
		broadcastToChannel(channel, text, NULL);
	else
		sendToClient(client, text);
}

void Server::handleBot(Client *client, const std::string &text, Channel *channel)
{
	std::string	word = text.substr(0, text.find(' '));
	std::string	reply;

	if (word == "!help")
	{
		static const char *help[] = {
			"Available commands:",
			"!help to know the commands you can use",
			"!ping the bot will answer you <pong>",
			"!time to get time",
			"!echo <message> to see your message via the bot",
			"/NICK <username> to edit your username",
			"/PRIVMSG <channel/user> to send a private message to a user present in the chanel you choosed (You need to be in the same channel of the user you want to text with)",
			"/KICK <channel> <user> <(optionnal) reason>",
			"/INVITE <user> <channel> to invite a user into a channel on invite only mode",
			"/TOPIC <channel> <(optionnal) new topic> to see or edit a topic in a channel",
			"/MODE <channel> <modes list> <(optionnal) params> used to change the mode of a channel",
			"WARNING: A CHANNEL NEED TO BEGIN WITH # or &"
		};
		for (size_t i = 0; i < sizeof(help) / sizeof(help[0]); i++)
			sendBotReply(client, channel, ":" + std::string(BOT_NICK) + "!bot@" + _serverName + " PRIVMSG " + (channel ? channel->getName() : client->getNickname()) + " :" + help[i]);
		return;
	}
	else if (word == "!ping")
		reply = "pong";
	else if (word == "!time")
	{
		std::time_t	now = std::time(NULL);
		reply = std::ctime(&now);
		if (!reply.empty() && reply[reply.size() - 1] == '\n')
			reply.erase(reply.size() - 1);
	}
	else if (word == "!echo")
	{
		if (text.size() > word.size() + 1)
			reply = text.substr(word.size() + 1);
		else
			reply = "Usage: !echo <message>";
	}
	else
		reply = "Unknown command. Type !help for the list.";

	sendBotReply(client, channel, ":" + std::string(BOT_NICK) + "!bot@" + _serverName + " PRIVMSG " + (channel ? channel->getName() : client->getNickname()) + " :" + reply);
}

void Server::handleKick(Client *client, const s_cmd &cmd)
{
	if (cmd.params.size() < 2 || cmd.params[0].empty() || cmd.params[1].empty())
	{
		sendToClient(client, buildReply("461", client, "KICK :Not enough parameters"));
		return;
	}

	if (cmd.params[0][0] != '#' && cmd.params[0][0] != '&')
	{
		sendToClient(client, buildReply("403", client, cmd.params[0] + " :No such channel"));
		return;
	}

	std::map<std::string, Channel*>::iterator channel = _channels.find(cmd.params[0]);
	if (channel == _channels.end())
	{
		sendToClient(client, buildReply("403", client, cmd.params[0] + " :No such channel"));
		return;
	}

	if (!channel->second->isMember(client))
	{
		sendToClient(client, buildReply("442", client, cmd.params[0] + " :You're not on that channel"));
		return;
	}

	if (!channel->second->isOperator(client))
	{
		sendToClient(client, buildReply("482", client, cmd.params[0] + " :You're not channel operator"));
		return;
	}

	std::map<int, Client*>::iterator target = _clients.begin();
	std::map<int, Client*>::iterator clientEnd = _clients.end();
	while (target != clientEnd)
	{
		if (ftStrToUpper(target->second->getNickname()) == ftStrToUpper(cmd.params[1]))
		{
			if (!channel->second->isMember(target->second))
				break;

			broadcastToChannel(channel->second, ":" + client->getPrefix() + " KICK " + cmd.params[0] + " " + cmd.params[1] + (cmd.params.size() > 2 ? " :" + cmd.params[2] : ""), NULL);

			channel->second->removeMember(target->second);
			if (channel->second->isEmptyChannel())
			{
				delete channel->second;
				_channels.erase(channel);
			}

			return;
		}
		target++;
	}

	sendToClient(client, buildReply("441", client, cmd.params[1] + " " + cmd.params[0] + " :They aren't on that channel"));
}

void Server::handleInvite(Client *client, const s_cmd &cmd)
{
	if (cmd.params.size() < 2 || cmd.params[0].empty() || cmd.params[1].empty())
	{
		sendToClient(client, buildReply("461", client, "INVITE :Not enough parameters"));
		return;
	}

	if (cmd.params[1][0] != '#' && cmd.params[1][0] != '&')
	{
		sendToClient(client, buildReply("403", client, cmd.params[1] + " :No such channel"));
		return;
	}

	std::map<std::string, Channel*>::iterator channel = _channels.find(cmd.params[1]);
	if (channel == _channels.end())
	{
		sendToClient(client, buildReply("403", client, cmd.params[1] + " :No such channel"));
		return;
	}

	if (!channel->second->isMember(client))
	{
		sendToClient(client, buildReply("442", client, cmd.params[1] + " :You're not on that channel"));
		return;
	}

	if (channel->second->getInviteOnly() && !channel->second->isOperator(client))
	{
		sendToClient(client, buildReply("482", client, cmd.params[1] + " :You're not channel operator"));
		return;
	}

	std::map<int, Client*>::iterator target = _clients.begin();
	std::map<int, Client*>::iterator clientEnd = _clients.end();
	while (target != clientEnd)
	{
		if (ftStrToUpper(target->second->getNickname()) == ftStrToUpper(cmd.params[0]))
		{
			if (channel->second->isMember(target->second))
			{
				sendToClient(client, buildReply("443", client, cmd.params[0] + " " + cmd.params[1] + " :is already on channel"));
				return;
			}

			channel->second->inviteMember(target->second);

			sendToClient(target->second, ":" + client->getPrefix() + " INVITE " + cmd.params[0] + " " + cmd.params[1]);
			sendToClient(client, buildReply("341", client, cmd.params[0] + " " + cmd.params[1]));
			return;
		}
		target++;
	}
	
	sendToClient(client, buildReply("401", client, cmd.params[0] + " :No such nick"));
}

void Server::handleTopic(Client *client, const s_cmd &cmd)
{
	if (cmd.params.empty() || cmd.params[0].empty())
	{
		sendToClient(client, buildReply("461", client, "TOPIC :Not enough parameters"));
		return;
	}

	if (cmd.params[0][0] != '#' && cmd.params[0][0] != '&')
	{
		sendToClient(client, buildReply("403", client, cmd.params[0] + " :No such channel"));
		return;
	}

	std::map<std::string, Channel*>::iterator channel = _channels.find(cmd.params[0]);
	if (channel == _channels.end())
	{
		sendToClient(client, buildReply("403", client, cmd.params[0] + " :No such channel"));
		return;
	}

	if (!channel->second->isMember(client))
	{
		sendToClient(client, buildReply("442", client, cmd.params[0] + " :You're not on that channel"));
		return;
	}

	if (cmd.params.size() == 1)
	{
		std::string channelTopic = channel->second->getTopic();
		if (!channelTopic.empty())
			sendToClient(client, buildReply("332", client, cmd.params[0] + " :" + channelTopic));
		else
			sendToClient(client, buildReply("331", client, cmd.params[0] + " :No topic is set"));
	}
	else
	{
		if (channel->second->getTopicRestricted() && !channel->second->isOperator(client))
		{
			sendToClient(client, buildReply("482", client, cmd.params[0] + " :You're not channel operator"));
			return;
		}
		
		channel->second->setTopic(cmd.params[1]);
		
		broadcastToChannel(channel->second, ":" + client->getPrefix() + " TOPIC " + cmd.params[0] + " :" + channel->second->getTopic(), NULL);
	}
}

void Server::handleMode(Client *client, const s_cmd &cmd)
{
	if (cmd.params.empty() || cmd.params[0].empty())
	{
		sendToClient(client, buildReply("461", client, "MODE :Not enough parameters"));
		return;
	}

	if (cmd.params[0][0] != '#' && cmd.params[0][0] != '&')
	{
		sendToClient(client, buildReply("403", client, cmd.params[0] + " :No such channel"));
		return;
	}

	std::map<std::string, Channel*>::iterator channel = _channels.find(cmd.params[0]);
	if (channel == _channels.end())
	{
		sendToClient(client, buildReply("403", client, cmd.params[0] + " :No such channel"));
		return;
	}

	if (cmd.params.size() == 1)
	{
		std::string actualModes = "+";
		if (channel->second->getInviteOnly())
			actualModes += 'i';
		if (channel->second->getTopicRestricted())
			actualModes += 't';
		if (!channel->second->getKey().empty())
			actualModes += 'k';
		if (channel->second->getMemberLimit() > 0)
			actualModes += 'l';

		sendToClient(client, buildReply("324", client, cmd.params[0] + " " + actualModes));
		return;
	}
	else
	{
		if (!channel->second->isMember(client))
		{
			sendToClient(client, buildReply("442", client, cmd.params[0] + " :You're not on that channel"));
			return;
		}

		if (!channel->second->isOperator(client))
		{
			sendToClient(client, buildReply("482", client, cmd.params[0] + " :You're not channel operator"));
			return;
		}

		bool adding = true;
		size_t argIdx = 2;
		for (size_t i = 0; i < cmd.params[1].size(); i++)
		{
			if (cmd.params[1][i] == '+')
				adding = true;
			else if (cmd.params[1][i] == '-')
				adding = false;
			else if (cmd.params[1][i] == 'i')
				channel->second->setInviteOnly(adding);
			else if (cmd.params[1][i] == 't')
				channel->second->setTopicRestricted(adding);
			else if (cmd.params[1][i] == 'k')
			{
				if (adding)
				{
					if (argIdx < cmd.params.size())
					{
						channel->second->setKey(cmd.params[argIdx]);
						argIdx++;
					}
					else
						sendToClient(client, buildReply("461", client, "MODE :Not enough parameters"));
				}
				else
					channel->second->setKey("");
			}
			else if (cmd.params[1][i] == 'l')
			{
				if (adding)
				{
					if (argIdx < cmd.params.size())
					{
						long limit = std::atol(cmd.params[argIdx].c_str());
						if (limit < 0 || limit > INT_MAX)
							limit = 0;
						channel->second->setMemberLimit(limit);
						argIdx++;
					}
					else
						sendToClient(client, buildReply("461", client, "MODE :Not enough parameters"));
				}
				else
					channel->second->setMemberLimit(0);
			}
			else if (cmd.params[1][i] == 'o')
			{
				if (argIdx < cmd.params.size())
				{
					std::map<int, Client*>::iterator target = _clients.begin();
					std::map<int, Client*>::iterator clientEnd = _clients.end();
					while (target != clientEnd)
					{
						if (ftStrToUpper(target->second->getNickname()) == ftStrToUpper(cmd.params[argIdx]))
						{
							if (!channel->second->isMember(target->second))
							{
								sendToClient(client, buildReply("441", client, cmd.params[argIdx] + " " + cmd.params[0] + " :They aren't on that channel"));
								break;
							}

							if (adding)
								channel->second->promoteMember(target->second);
							else
								channel->second->demoteMember(target->second);
						}
						target++;
					}
					argIdx++;
				}
				else
					sendToClient(client, buildReply("461", client, "MODE :Not enough parameters"));
			}
		}
		
		std::string params;
		bool display = true;
		for (size_t i = 0; i < cmd.params.size(); i++)
		{
			if (!params.empty())
				params += " ";
			if (i == 1)
			{
				for (size_t j = 0; j < cmd.params[i].size(); j++)
				{
					if (std::strchr("itkol+-", cmd.params[i][j]))
						params += std::string(1, cmd.params[i][j]);
					else
						sendToClient(client, buildReply("472", client, std::string(1, cmd.params[i][j]) + " :is unknown mode char to me"));
				}

				if (std::strchr("+-", params[params.size() - 1]))
					display = false;
			}
			else
				params += cmd.params[i];
		}
		
		if (display)
			broadcastToChannel(channel->second, ":" + client->getPrefix() + " MODE " + params, NULL);
	}
}

void Server::handlePart(Client *client, const s_cmd &cmd)
{
	if (cmd.params.size() < 1 || cmd.params[0].empty())
	{
		sendToClient(client, buildReply("461", client, "PART :Not enough parameters"));
		return;
	}

	if (cmd.params[0][0] != '#' && cmd.params[0][0] != '&')
	{
		sendToClient(client, buildReply("403", client, cmd.params[0] + " :No such channel"));
		return;
	}

	std::map<std::string, Channel*>::iterator channel = _channels.find(cmd.params[0]);
	if (channel == _channels.end())
	{
		sendToClient(client, buildReply("403", client, cmd.params[0] + " :No such channel"));
		return;
	}

	if (!channel->second->isMember(client))
	{
		sendToClient(client, buildReply("442", client, cmd.params[0] + " :You're not on that channel"));
		return;
	}

	broadcastToChannel(channel->second, ":" + client->getPrefix() + " PART " + cmd.params[0] + ((cmd.params.size() > 1 && !cmd.params[1].empty()) ? (" :" + cmd.params[1]) : ""), NULL);
	
	channel->second->removeMember(client);
	if (channel->second->isEmptyChannel())
	{
		delete channel->second;
		_channels.erase(channel);
	}
}

void Server::handleQuit(Client *client, const s_cmd &cmd)
{
	broadcastQuit(client, (cmd.params.size() > 0 && !cmd.params[0].empty()) ? cmd.params[0] : "");
	removeClient(client->getClientFd());
}

//            //
// Exceptions //
//            //
const char* Server::CreateSocketException::what() const throw()
{
	return ("Failure during socket creation.");
}

const char* Server::CreatePollException::what() const throw()
{
	return ("Failure during poll creation.");
}