/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: achauvie <achauvie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 10:29:36 by achauvie          #+#    #+#             */
/*   Updated: 2026/07/21 09:51:44 by achauvie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(const std::string &channelName, Client *creator): _name(channelName), _topic(""), _inviteOnly(false), _topicRestricted(false), _key(""), _memberLimit(0)
{
	if (!creator)
		return;

	_clients.insert(creator);
	_clientsOp.insert(creator);
}

Channel::~Channel(void)
{}

//         //
// Process //
//         // 
void Channel::addMember(Client *client)
{
	if (!client)
		return;

	_clients.insert(client);
	if (isInvited(client))
		_clientsInvited.erase(client);
}

void Channel::removeMember(Client *client)
{
	if (!client)
		return;

	_clients.erase(client);
	_clientsOp.erase(client);
	_clientsInvited.erase(client);
}

void Channel::promoteMember(Client *client)
{
	if (!client)
		return;

	if (isMember(client) && !isOperator(client))
		_clientsOp.insert(client);
}

void Channel::demoteMember(Client *client)
{
	if (!client)
		return;
	
	if (isMember(client) && isOperator(client))
		_clientsOp.erase(client);
}

void Channel::inviteMember(Client *client)
{
	if (!client)
		return;

	if (!isMember(client) && !isInvited(client))
		_clientsInvited.insert(client);
}

//         //
// Getters //
//         // 
const std::string &Channel::getName(void) const
{
	return _name;
}

const std::set<Client*> &Channel::getClients(void) const
{
	return _clients;
}

const std::set<Client*> &Channel::getClientsOp(void) const
{
	return _clientsOp;
}

const std::string &Channel::getTopic(void) const
{
	return _topic;
}

bool Channel::getInviteOnly(void) const
{
	return _inviteOnly;
}

bool Channel::getTopicRestricted(void) const
{
	return _topicRestricted;
}

const std::string &Channel::getKey(void) const
{
	return _key;
}

size_t Channel::getMemberLimit(void) const
{
	return _memberLimit;
}

const std::set<Client*> &Channel::getClientsInvited(void) const
{
	return _clientsInvited;
}

bool Channel::isMember(Client *client) const
{
	return _clients.find(client) != _clients.end();
}

bool Channel::isOperator(Client *client) const
{
	return _clientsOp.find(client) != _clientsOp.end();
}

size_t Channel::countMembers(void) const
{
	return _clients.size();
}

bool Channel::isEmptyChannel(void) const
{
	return _clients.empty();
}

bool Channel::isInvited(Client *client) const
{
	return _clientsInvited.find(client) != _clientsInvited.end();
}

//         //
// Setters //
//         // 
void Channel::setTopic(const std::string &value)
{
	_topic = value;
}

void Channel::setInviteOnly(bool value)
{
	_inviteOnly = value;
}

void Channel::setTopicRestricted(bool value)
{
	_topicRestricted = value;
}

void Channel::setKey(const std::string &key)
{
	_key = key;
}

void Channel::setMemberLimit(size_t limit)
{
	_memberLimit = limit;
}