/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRC.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: achauvie <achauvie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 09:00:19 by achauvie          #+#    #+#             */
/*   Updated: 2026/07/20 11:02:06 by achauvie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <exception>
#include <cstdlib>
#include <cctype>

struct s_cmd
{
	std::string					prefix;
	std::string					name;
	std::vector<std::string>	params;
};

long parsingPort(const std::string &str);
void parsingPassword(const std::string &str);

std::string ftStrToUpper(const std::string &str);
s_cmd parseCmd(const std::string &line);