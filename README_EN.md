# ft_irc
![42 Cursus](https://img.shields.io/badge/42-Cursus-blue?style=for-the-badge)
![Projects](https://img.shields.io/badge/Projects-Common_Core-success?style=for-the-badge)
![Score](https://img.shields.io/badge/score-125%20%2F%20100-success?style=for-the-badge)<br>
[🇫🇷 Français](./README.md)
- [ft\_irc](#ft_irc)
	- [Description](#description)
	- [Instructions](#instructions)
		- [Starting the Server](#starting-the-server)
		- [Connecting to the Server](#connecting-to-the-server)
		- [Commands](#commands)
			- [MODE Command Explanation](#mode-command-explanation)
		- [\[BONUS\] Bot Usage](#bonus-bot-usage)
	- [AI Usage](#ai-usage)
- [Resources](#resources)

## Description
A project consisting of implementing a fully functional IRC (Internet Relay Chat) server in C++, compatible with existing IRC clients. The goal is to handle multiple client connections, discussion channels, user and operator modes, as well as the core IRC protocol commands (JOIN, PART, PRIVMSG, KICK, INVITE, etc.). The server must operate with non-blocking I/O on a single thread. This project builds mastery of network communication protocols, socket management, text protocol parsing and the design of robust multi-client servers in C++.

## Instructions
### Starting the Server
- Use the `make` command to compile the program.
- Use the command `./ircserv <port> <password>` to start the server on the desired port using the password of your choice.

### Connecting to the Server
- You can use `NetCat` from a terminal.
	- Open your terminal.
	- Run the command `nc -C <server_ip_address> <port>`.
		- For a local server, the IP address is `127.0.0.1`.
	- Once connected, enter the following commands:
    	- `PASS <server_password>`.
    	- `USER <username> <mode> <unused> <real_name>`.
        	- You can use the same value for all four parameters. In practice, only the first and the last are used, but all four parameters are still required.
    	- `NICK <nickname>`.
- You can also use an IRC client. There are many available, but we chose to use `HexChat`.

### Commands
> This table lists the commands available once connected to the server.

> When using an `IRC client`, you must prefix commands with `/` (`do not` use it when using `NetCat`).

> **WARNING:** When using `NetCat`, you must prefix any argument containing spaces with `:`. Otherwise, the server will interpret it as multiple parameters. An IRC client such as `HexChat` automatically does this before sending the command to the server. For example, in `NetCat` you should write `TOPIC <channel> :<text with spaces>`.

| Command | Description |
|----------|-------------|
| `NICK <nickname>` | Change your nickname. |
| `JOIN <channel> <(Opt.) key>` | Join a channel. The key should only be provided if the channel is protected by an access key (password).<br>(**WARNING**: The channel name must begin with `#` or `&`.) |
| `PRIVMSG <channel/user>` | Send a message to a channel or directly to a user (*you must have joined the channel before sending messages to it*).<br>(**WARNING**: The channel name must begin with `#` or `&`.) |
| `KICK <channel> <user> <(Opt.) reason>` | Remove a user from the channel (*the reason is optional*). Both you and the target user must be present in the channel, and you must be a channel operator.<br>(**WARNING**: The channel name must begin with `#` or `&`.) |
| `INVITE <user> <channel>` | Invite a user to a channel (*useful when the channel is in `Invite-Only` mode*). Anyone can invite users unless the channel is in `Invite-Only` mode, in which case only channel operators can invite others.<br>(**WARNING**: The channel name must begin with `#` or `&`.) |
| `TOPIC <channel> <(Opt.) new topic>` | View or change the channel topic (if the `new topic` parameter is provided). If the channel is in `Topic-Restricted` mode, only a channel operator can modify the topic. To remove the topic, use the command `TOPIC <channel> :`.<br>(**WARNING**: The channel name must begin with `#` or `&`.) |
| `MODE <channel> <mode list> <(Opt.) params>` | *This command is explained in more detail below.* It allows you to modify the channel modes. You must be a channel operator to use this command.<br>(**WARNING**: The channel name must begin with `#` or `&`.) |

#### MODE Command Explanation
> To add or remove a mode, prefix the mode list with `+` or `-`.

Several modes are available:

- **i**: Enable/Disable `Invite-Only` mode. When enabled, a user may only join the channel if they have previously been invited by a channel operator. (No parameter required.)
- **t**: Enable/Disable `Topic-Restricted` mode. When enabled, only a channel operator can modify the channel topic. (No parameter required.)
- **k**: Add/Remove a channel access key. When enabled, users must provide the correct key when joining with `JOIN <channel> <key>`. (Requires the key as a parameter.)
- **l**: Set/Remove a user limit for the channel. To remove the limit, use `0` as the parameter (sets the channel to `unlimited`). (Requires the limit as a parameter.)
- **o**: Grant/Remove channel operator privileges to/from a user. (Requires the target user as a parameter.)

Examples:

- `MODE <channel> +i`: Set the channel to `Invite-Only`.
- `MODE <channel> +t`: Set the channel to `Topic-Restricted`.
- `MODE <channel> +k <key>`: Add an access key.
- `MODE <channel> +l <limit>`: Set a user limit.
- `MODE <channel> +o <member>`: Grant operator privileges.
- `MODE <channel> -o <member>`: Remove operator privileges (the user becomes a regular member again).

> You can also combine multiple modes into a single command. In that case, the parameters must follow the same order as the corresponding modes.

- `MODE <channel> +tkl <key> <limit>`: Set the channel to `Topic-Restricted`, add an access key, and set a user limit.

### [BONUS] Bot Usage
> The `IRCBot` bot is directly integrated into the server (it does not have its own client connection). Therefore, it does not appear in the channel member list.

- The nickname `IRCBot` is reserved and cannot be used by any client.
- To interact with the bot (and receive a private message), send it one of the commands below.
	- On an `IRC client` (such as HexChat): `/privmsg IRCBot <command>`.
	- On `NetCat`: `PRIVMSG IRCBot <command>`.
- To interact with the bot (and receive a message in the channel), send one of the commands below.
	- On an `IRC client` (such as HexChat): `<command>`.
	- On `NetCat`: `PRIVMSG <channel> <command>`.

| Command | Description |
|----------|-------------|
| `!help` | Display the list of available bot commands. |
| `!ping` | The bot replies with `pong`. |
| `!time` | The bot returns the server's current date and time. |
| `!echo <message>` | The bot repeats the provided message. |

## AI Usage
Artificial intelligence was used as a supporting tool to better understand and organize the key concepts of this project (in addition to the resources listed below), particularly the overall IRC protocol and the understanding of the new functions used to implement the project.

# Resources
- [Network Working Group C - RFC2812](http://abcdrfc.free.fr/rfc-vf/pdf/rfc2812.pdf)
- [IRC Client Protocol Specification](https://modern.ircdocs.horse/#errunknownerror-400)
- [Commands - IRC DevDocs](https://dd.ircdocs.horse/refs/commands)