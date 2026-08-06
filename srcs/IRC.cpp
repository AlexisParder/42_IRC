/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRC.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: achauvie <achauvie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 10:52:22 by achauvie          #+#    #+#             */
/*   Updated: 2026/07/21 09:52:03 by achauvie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

/**
 * A TCP port is stored as a 16-bit unsigned integer (valid range: 0 to 65535).
 * Ports 0 to 1023 are well-known ports (HTTP, SSH, etc.) reserved for the system.
 */
long parsingPort(const std::string &str)
{
	if (str.empty())
		throw std::invalid_argument("Invalid port: port number cannot be empty.");

	for (size_t i = 0; i < str.size(); i++)
		if (!std::isdigit(str[i]))
			throw std::invalid_argument("Invalid port: port number must contain only digits.");

	size_t realStart = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] != '0')
		{
			realStart = i;
			break;
		}
	}

	if (str.size() - realStart > 5)
		throw std::invalid_argument("Invalid port: port number must be between 1024 and 65535");
	
	long port = std::atol(str.c_str());
	if (port < 1024 || port > 65535)
		throw std::invalid_argument("Invalid port: port number must be between 1024 and 65535");

	return port;
}

void parsingPassword(const std::string &str)
{
	if (str.empty())
		throw std::invalid_argument("Invalid password: password cannot be empty.");
}

std::string ftStrToUpper(const std::string &str)
{
	std::string newStr = "";
	for (size_t i = 0; i < str.size(); i++)
		newStr += std::toupper(static_cast<unsigned char>(str[i]));
	return newStr;
}

s_cmd parseCmd(const std::string &line)
{
	s_cmd	cmd;
	size_t	lineIdx = line.find_first_not_of(" \t");
	size_t	endIdx;

	if (lineIdx == std::string::npos)
		return cmd;

	if (line[lineIdx] == ':')
	{
		endIdx = line.find_first_of(" \t", lineIdx);
		if (endIdx == std::string::npos)
			endIdx = line.size();
		cmd.prefix = line.substr(lineIdx + 1, endIdx - (lineIdx + 1));
		lineIdx = endIdx;
	}

	while (lineIdx < line.size())
	{
		lineIdx = line.find_first_not_of(" \t", lineIdx);
		if (lineIdx == std::string::npos)
			break;

		if (line[lineIdx] == ':' && !cmd.name.empty())
		{
			cmd.params.push_back(line.substr(lineIdx + 1));
			break;
		}
		endIdx = line.find_first_of(" \t", lineIdx);
		if (endIdx == std::string::npos)
			endIdx = line.size();

		std::string word = line.substr(lineIdx, endIdx - lineIdx);
		if (cmd.name.empty())
			cmd.name = ftStrToUpper(word);
		else
			cmd.params.push_back(word);
		lineIdx = endIdx;
	}

	return cmd;
}