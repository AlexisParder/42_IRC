/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: achauvie <achauvie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 10:28:50 by achauvie          #+#    #+#             */
/*   Updated: 2026/07/21 09:51:27 by achauvie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <set>
#include "Client.hpp"

class Channel
{
	private:
		const std::string	_name;
		std::set<Client*>	_clients;
		std::set<Client*>	_clientsOp;
		std::string			_topic;
		bool				_inviteOnly;
		bool				_topicRestricted;
		std::string			_key;
		size_t				_memberLimit;
		std::set<Client*>	_clientsInvited;
		Channel(void);
		Channel(const Channel &other);
		Channel &operator=(const Channel &other);

	public:
		Channel(const std::string &channelName, Client *creator);
		~Channel(void);

		// Process
		void addMember(Client *client);
		void removeMember(Client *client);
		void promoteMember(Client *client);
		void demoteMember(Client *client);
		void inviteMember(Client *client);

		// Getters
		const std::string &getName(void) const;
		const std::set<Client*> &getClients(void) const;
		const std::set<Client*> &getClientsOp(void) const;
		const std::string &getTopic(void) const;
		bool getInviteOnly(void) const;
		bool getTopicRestricted(void) const;
		const std::string &getKey(void) const;
		size_t getMemberLimit(void) const;
		const std::set<Client*> &getClientsInvited(void) const;
		bool isMember(Client *client) const;
		bool isOperator(Client *client) const;
		size_t countMembers(void) const;
		bool isEmptyChannel(void) const;
		bool isInvited(Client *client) const;

		// Setters
		void setTopic(const std::string &value);
		void setInviteOnly(bool value);
		void setTopicRestricted(bool value);
		void setKey(const std::string &key);
		void setMemberLimit(size_t limit);
};