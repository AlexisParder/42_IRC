# ft_irc
![42 Cursus](https://img.shields.io/badge/42-Cursus-blue?style=for-the-badge)
![Projects](https://img.shields.io/badge/Projects-Common_Core-success?style=for-the-badge)
![Score](https://img.shields.io/badge/score-125%20%2F%20100-success?style=for-the-badge)<br>
[🇬🇧 English](./README_EN.md)<br>
- [ft\_irc](#ft_irc)
	- [Description](#description)
	- [Instructions](#instructions)
		- [Démarrer le serveur](#démarrer-le-serveur)
		- [Se connecter au serveur](#se-connecter-au-serveur)
		- [Commandes](#commandes)
			- [Explication de la commande MODE](#explication-de-la-commande-mode)
		- [\[BONUS\] Utilisation du bot](#bonus-utilisation-du-bot)
	- [Utilisation de l'IA](#utilisation-de-lia)
- [Ressources](#ressources)

## Description
Un projet consistant à implémenter un serveur IRC (Internet Relay Chat) fonctionnel en C++, compatible avec les clients IRC existants. L'objectif est de gérer les connexions multiples de clients, les canaux de discussion, les modes utilisateurs et opérateurs, ainsi que les commandes fondamentales du protocole IRC (JOIN, PART, PRIVMSG, KICK, INVITE, etc.). Le serveur doit fonctionner en I/O non-bloquant avec un seul thread. Ce projet développe une maîtrise des protocoles de communication réseau, de la gestion des sockets, de l'analyse de protocoles textuels et de la conception de serveurs multi-clients robustes en C++.

## Instructions
### Démarrer le serveur
- Utilisez la commande `make` pour compiler le programme.
- Utilisez la commande `./ircserv <port> <password>` pour démarrer le serveur sur le port souhaité en utilisant le mot de passe de votre choix.

### Se connecter au serveur
- Vous pouvez utiliser `NetCat` depuis un terminal.
	- Ouvrez votre terminal.
	- Exécutez la commande `nc -C <adresse_ip_du_serveur> <port>`.
		- Pour un serveur local, l'adresse IP est `127.0.0.1`.
	- Une fois connecté, entrez les commandes suivantes :
    	- `PASS <mot_de_passe_du_serveur>`.
    	- `USER <username> <mode> <unused> <real_name>`.
        	- Vous pouvez utiliser la même valeur pour les quatre paramètres. En pratique, seuls le premier et le dernier sont utilisés, mais les quatre paramètres restent obligatoires.
    	- `NICK <nickname>`.
- Vous pouvez également utiliser un client IRC. Il en existe beaucoup, mais nous avons choisi d'utiliser `HexChat`.

### Commandes
> Ce tableau liste les commandes disponibles une fois connecté au serveur.

> Lorsque vous utilisez un `client IRC`, vous devez préfixer les commandes avec `/` (`ne pas` l'utiliser avec `NetCat`).

> **ATTENTION :** Avec `NetCat`, vous devez préfixer tout argument contenant des espaces avec `:`. Sinon, le serveur l'interprétera comme plusieurs paramètres. Un client IRC tel que `HexChat` le fait automatiquement avant d'envoyer la commande au serveur. Par exemple, avec `NetCat`, vous devez écrire `TOPIC <channel> :<texte avec espaces>`.

| Commande | Description |
|----------|-------------|
| `NICK <nickname>` | Changer votre pseudonyme. |
| `JOIN <channel> <(Opt.) key>` | Rejoindre un canal. La clé ne doit être fournie que si le canal est protégé par une clé d'accès (mot de passe).<br>(**ATTENTION** : Le nom du canal doit commencer par `#` ou `&`.) |
| `PRIVMSG <channel/user>` | Envoyer un message à un canal ou directement à un utilisateur (*vous devez avoir rejoint le canal avant de pouvoir y envoyer des messages*).<br>(**ATTENTION** : Le nom du canal doit commencer par `#` ou `&`.) |
| `KICK <channel> <user> <(Opt.) reason>` | Expulser un utilisateur du canal (*la raison est optionnelle*). Vous et l'utilisateur ciblé devez tous les deux être présents dans le canal, et vous devez être opérateur du canal.<br>(**ATTENTION** : Le nom du canal doit commencer par `#` ou `&`.) |
| `INVITE <user> <channel>` | Inviter un utilisateur dans un canal (*utile lorsque le canal est en mode `Invite-Only`*). Tout le monde peut inviter des utilisateurs sauf si le canal est en mode `Invite-Only`, auquel cas seuls les opérateurs du canal peuvent inviter d'autres personnes.<br>(**ATTENTION** : Le nom du canal doit commencer par `#` ou `&`.) |
| `TOPIC <channel> <(Opt.) new topic>` | Afficher ou changer le sujet du canal (si le paramètre `new topic` est fourni). Si le canal est en mode `Topic-Restricted`, seul un opérateur du canal peut modifier le sujet. Pour supprimer le sujet, utilisez la commande `TOPIC <channel> :`.<br>(**ATTENTION** : Le nom du canal doit commencer par `#` ou `&`.) |
| `MODE <channel> <mode list> <(Opt.) params>` | *Cette commande est expliquée plus en détail ci-dessous.* Elle permet de modifier les modes du canal. Vous devez être opérateur du canal pour utiliser cette commande.<br>(**ATTENTION** : Le nom du canal doit commencer par `#` ou `&`.) |

#### Explication de la commande MODE
> Pour ajouter ou retirer un mode, préfixez la liste des modes avec `+` ou `-`.

Plusieurs modes sont disponibles :

- **i** : Activer/Désactiver le mode `Invite-Only`. Lorsqu'il est activé, un utilisateur ne peut rejoindre le canal que s'il a été préalablement invité par un opérateur du canal. (Aucun paramètre requis.)
- **t** : Activer/Désactiver le mode `Topic-Restricted`. Lorsqu'il est activé, seul un opérateur du canal peut modifier le sujet du canal. (Aucun paramètre requis.)
- **k** : Ajouter/Retirer une clé d'accès au canal. Lorsqu'elle est activée, les utilisateurs doivent fournir la bonne clé lorsqu'ils rejoignent avec `JOIN <channel> <key>`. (Nécessite la clé comme paramètre.)
- **l** : Définir/Retirer une limite d'utilisateurs pour le canal. Pour retirer la limite, utilisez `0` comme paramètre (le canal devient `unlimited`). (Nécessite la limite comme paramètre.)
- **o** : Accorder/Retirer les privilèges d'opérateur du canal à un utilisateur. (Nécessite l'utilisateur cible comme paramètre.)

Exemples :

- `MODE <channel> +i` : Mettre le canal en mode `Invite-Only`.
- `MODE <channel> +t` : Mettre le canal en mode `Topic-Restricted`.
- `MODE <channel> +k <key>` : Ajouter une clé d'accès.
- `MODE <channel> +l <limit>` : Définir une limite d'utilisateurs.
- `MODE <channel> +o <member>` : Accorder les privilèges d'opérateur.
- `MODE <channel> -o <member>` : Retirer les privilèges d'opérateur (l'utilisateur redevient un membre ordinaire).

> Vous pouvez également combiner plusieurs modes dans une seule commande. Dans ce cas, les paramètres doivent suivre le même ordre que les modes correspondants.

- `MODE <channel> +tkl <key> <limit>` : Mettre le canal en mode `Topic-Restricted`, ajouter une clé d'accès et définir une limite d'utilisateurs.

### [BONUS] Utilisation du bot
> Le bot `IRCBot` est directement intégré au serveur (il n'a pas sa propre connexion client). Par conséquent, il n'apparaît pas dans la liste des membres du canal.

- Le pseudonyme `IRCBot` est réservé et ne peut être utilisé par aucun client.
- Pour interagir avec le bot (et recevoir un message privé), envoyez-lui l'une des commandes ci-dessous.
	- Sur un `client IRC` (tel que HexChat) : `/privmsg IRCBot <command>`.
	- Sur `NetCat` : `PRIVMSG IRCBot <command>`.
- Pour interagir avec le bot (et recevoir un message dans le canal), envoyez l'une des commandes ci-dessous.
	- Sur un `client IRC` (tel que HexChat) : `<command>`.
	- Sur `NetCat` : `PRIVMSG <channel> <command>`.

| Commande | Description |
|----------|-------------|
| `!help` | Afficher la liste des commandes disponibles du bot. |
| `!ping` | Le bot répond avec `pong`. |
| `!time` | Le bot renvoie la date et l'heure actuelles du serveur. |
| `!echo <message>` | Le bot répète le message fourni. |

## Utilisation de l'IA
L'intelligence artificielle a été utilisée comme outil d'appui pour mieux comprendre et organiser les concepts clés de ce projet (en complément des ressources listées ci-dessous), en particulier le protocole IRC dans son ensemble et la compréhension des nouvelles fonctions utilisées pour réaliser le projet.

# Ressources
- [Network Working Group C - RFC2812](http://abcdrfc.free.fr/rfc-vf/pdf/rfc2812.pdf)
- [IRC Client Protocol Specification](https://modern.ircdocs.horse/#errunknownerror-400)
- [Commands - IRC DevDocs](https://dd.ircdocs.horse/refs/commands)