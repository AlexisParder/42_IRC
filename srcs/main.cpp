/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: achauvie <achauvie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 08:35:01 by achauvie          #+#    #+#             */
/*   Updated: 2026/08/06 11:20:49 by achauvie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"
#include "Server.hpp"

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Error: invalid format\n./ircserv <port> <password>" << std::endl;
		return 1;
	}

	try
	{
		long port = parsingPort(av[1]);
		parsingPassword(av[2]);

		Server srv(port, av[2], "myirc.server");
		srv.setup();
		srv.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
	
	return 0;
}