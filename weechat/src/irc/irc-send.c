/*
 * Copyright (c) 2004 by FlashCode <flashcode@flashtux.org>
 * See README for License detail, AUTHORS for developers list.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* irc-send.c: implementation of IRC commands (client to server),
               according to RFC 1459,2810,2811,2812 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/utsname.h>

#include "../common/weechat.h"
#include "irc.h"
#include "../common/command.h"
#include "../common/weeconfig.h"
#include "../gui/gui.h"


/*
 * irc_login: login to irc server
 */

void
irc_login (t_irc_server *server)
{
    char hostname[128];

    if ((server->password) && (server->password[0]))
        server_sendf (server, "PASS %s\r\n", server->password);
    
    gethostname (hostname, sizeof (hostname) - 1);
    hostname[sizeof (hostname) - 1] = '\0';
    if (!hostname[0])
        strcpy (hostname, _("unknown"));
    gui_printf (server->window,
                _("%s: using local hostname \"%s\"\n"),
                PACKAGE_NAME, hostname);
    server_sendf (server,
                  "NICK %s\r\n"
                  "USER %s %s %s :%s\r\n",
                  server->nick, server->username, hostname, "servername",
                  server->realname);
}

/*
 * irc_cmd_send_admin: find information about the administrator of the server
 */

int
irc_cmd_send_admin (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "ADMIN %s\r\n", arguments);
    else
        server_sendf (server, "ADMIN\r\n");
    return 0;
}

/*
 * irc_cmd_send_away: toggle away status
 */

int
irc_cmd_send_away (t_irc_server *server, char *arguments)
{
    char *pos;
    t_irc_server *ptr_server;
    
    if (arguments && (strncmp (arguments, "-all", 4) == 0))
    {
        pos = arguments + 4;
        while (pos[0] == ' ')
            pos++;
        if (!pos[0])
            pos = NULL;
        
        for (ptr_server = irc_servers; ptr_server;
             ptr_server = ptr_server->next_server)
        {
            if (server->is_connected)
            {
                if (pos)
                    server_sendf (ptr_server, "AWAY :%s\r\n", pos);
                else
                    server_sendf (ptr_server, "AWAY\r\n");
            }
        }
    }
    else
    {
        if (arguments)
            server_sendf (server, "AWAY :%s\r\n", arguments);
        else
            server_sendf (server, "AWAY\r\n");
    }
    return 0;
}

/*
 * irc_cmd_send_ctcp: send a ctcp message
 */

int
irc_cmd_send_ctcp (t_irc_server *server, char *arguments)
{
    char *pos, *pos2;
    struct timeval tv;
    struct timezone tz;
    
    pos = strchr (arguments, ' ');
    if (pos)
    {
        pos[0] = '\0';
        pos++;
        while (pos[0] == ' ')
            pos++;
        pos2 = strchr (pos, ' ');
        if (pos2)
        {
            pos2[0] = '\0';
            pos2++;
            while (pos2[0] == ' ')
                pos2++;
        }
        else
            pos2 = NULL;
        
        if (strcasecmp (pos, "version") == 0)
        {
            if (pos2)
                server_sendf (server, "PRIVMSG %s :\01VERSION %s\01\r\n",
                              arguments, pos2);
            else
                server_sendf (server, "PRIVMSG %s :\01VERSION\01\r\n",
                              arguments);
        }
        if (strcasecmp (pos, "action") == 0)
        {
            if (pos2)
                server_sendf (server, "PRIVMSG %s :\01ACTION %s\01\r\n",
                              arguments, pos2);
            else
                server_sendf (server, "PRIVMSG %s :\01ACTION\01\r\n",
                              arguments);
        }
        if (strcasecmp (pos, "ping") == 0)
        {
            gettimeofday (&tv, &tz);
            server_sendf (server, "PRIVMSG %s :\01PING %d %d\01\r\n",
                          arguments, tv.tv_sec, tv.tv_usec);
        }
        
    }
    return 0;
}

/*
 * irc_cmd_send_dcc: starts DCC (file or chat)
 */

int
irc_cmd_send_dcc (t_irc_server *server, char *arguments)
{
    /* TODO: write this command! */
    return 0;
}

/*
 * irc_cmd_send_deop: remove operator privileges from nickname(s)
 */

int
irc_cmd_send_deop (t_irc_server *server, int argc, char **argv)
{
    int i;
    
    if (WIN_IS_CHANNEL(gui_current_window))
    {
        for (i = 0; i < argc; i++)
            server_sendf (server, "MODE %s -o %s\r\n",
                          CHANNEL(gui_current_window)->name,
                          argv[i]);
    }
    else
        gui_printf (server->window,
                    _("%s \"%s\" command can only be executed in a channel window\n"),
                    WEECHAT_ERROR, "deop");
    return 0;
}

/*
 * irc_cmd_send_devoice: remove voice from nickname(s)
 */

int
irc_cmd_send_devoice (t_irc_server *server, int argc, char **argv)
{
    int i;
    
    if (WIN_IS_CHANNEL(gui_current_window))
    {
        for (i = 0; i < argc; i++)
            server_sendf (server, "MODE %s -v %s\r\n",
                          CHANNEL(gui_current_window)->name,
                          argv[i]);
    }
    else
    {
        gui_printf (server->window,
                    _("%s \"%s\" command can only be executed in a channel window\n"),
                    WEECHAT_ERROR, "devoice");
        return -1;
    }
    return 0;
}

/*
 * irc_cmd_send_die: shotdown the server
 */

int
irc_cmd_send_die (t_irc_server *server, char *arguments)
{
    /* make gcc happy */
    (void) arguments;
    
    server_sendf (server, "DIE\r\n");
    return 0;
}

/*
 * irc_cmd_send_info: get information describing the server
 */

int
irc_cmd_send_info (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "INFO %s\r\n", arguments);
    else
        server_sendf (server, "INFO\r\n");
    return 0;
}

/*
 * irc_cmd_send_invite: invite a nick on a channel
 */

int
irc_cmd_send_invite (t_irc_server *server, char *arguments)
{
    server_sendf (server, "INVITE %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_ison: check if a nickname is currently on IRC
 */

int
irc_cmd_send_ison (t_irc_server *server, char *arguments)
{
    server_sendf (server, "ISON %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_join: join a new channel
 */

int
irc_cmd_send_join (t_irc_server *server, char *arguments)
{
    server_sendf (server, "JOIN %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_kick: forcibly remove a user from a channel
 */

int
irc_cmd_send_kick (t_irc_server *server, char *arguments)
{
    if (string_is_channel (arguments))
        server_sendf (server, "KICK %s\r\n", arguments);
    else
    {
        if (WIN_IS_CHANNEL (gui_current_window))
        {
            server_sendf (server,
                          "KICK %s %s\r\n",
                          CHANNEL(gui_current_window)->name, arguments);
        }
        else
        {
            gui_printf (server->window,
                        _("%s \"%s\" command can only be executed in a channel window\n"),
                        WEECHAT_ERROR, "kick");
            return -1;
        }
    }
    return 0;
}

/*
 * irc_cmd_send_kill: close client-server connection
 */

int
irc_cmd_send_kill (t_irc_server *server, char *arguments)
{
    server_sendf (server, "KILL %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_links: list all servernames which are known by the server
 *                     answering the query
 */

int
irc_cmd_send_links (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "LINKS %s\r\n", arguments);
    else
        server_sendf (server, "LINKS\r\n");
    return 0;
}

/*
 * irc_cmd_send_list: close client-server connection
 */

int
irc_cmd_send_list (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "LIST %s\r\n", arguments);
    else
        server_sendf (server, "LIST\r\n");
    return 0;
}

/*
 * irc_cmd_send_lusers: get statistics about ths size of the IRC network
 */

int
irc_cmd_send_lusers (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "LUSERS %s\r\n", arguments);
    else
        server_sendf (server, "LUSERS\r\n");
    return 0;
}

/*
 * irc_cmd_send_me: send a ctcp action to the current channel
 */

int
irc_cmd_send_me (t_irc_server *server, char *arguments)
{
    if (WIN_IS_SERVER(gui_current_window))
    {
        gui_printf (server->window,
                    _("%s \"%s\" command can not be executed on a server window\n"),
                    WEECHAT_ERROR, "me");
        return -1;
    }
    server_sendf (server, "PRIVMSG %s :\01ACTION %s\01\r\n",
                  CHANNEL(gui_current_window)->name, arguments);
    irc_display_prefix (gui_current_window, PREFIX_ACTION_ME);
    gui_printf_color (gui_current_window,
                      COLOR_WIN_CHAT_NICK, "%s", server->nick);
    gui_printf_color (gui_current_window,
                      COLOR_WIN_CHAT, " %s\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_mode: change mode for channel/nickname
 */

int
irc_cmd_send_mode (t_irc_server *server, char *arguments)
{
    server_sendf (server, "MODE %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_msg: send a message to a nick or channel
 */

int
irc_cmd_send_msg (t_irc_server *server, char *arguments)
{
    char *pos, *pos_comma;
    t_irc_channel *ptr_channel;
    t_irc_nick *ptr_nick;
    
    pos = strchr (arguments, ' ');
    if (pos)
    {
        pos[0] = '\0';
        pos++;
        while (pos[0] == ' ')
            pos++;
        
        while (arguments && arguments[0])
        {
            pos_comma = strchr (arguments, ',');
            if (pos_comma)
            {
                pos_comma[0] = '\0';
                pos_comma++;
            }
            if (strcmp (arguments, "*") == 0)
            {
                if (WIN_IS_SERVER(gui_current_window))
                {
                    gui_printf (server->window,
                                _("%s \"%s\" command can not be executed on a server window\n"),
                                WEECHAT_ERROR, "msg *");
                    return -1;
                }
                ptr_channel = CHANNEL(gui_current_window);
                ptr_nick = nick_search (ptr_channel, server->nick);
                if (ptr_nick)
                {
                    irc_display_nick (ptr_channel->window, ptr_nick,
                                      MSG_TYPE_NICK, 1, 1, 0);
                    gui_printf_color_type (ptr_channel->window,
                                           MSG_TYPE_MSG,
                                           COLOR_WIN_CHAT, "%s\n", pos);
                }
                else
                    gui_printf (server->window,
                                _("%s nick not found for \"%s\" command\n"),
                                WEECHAT_ERROR, "msg");
                server_sendf (server, "PRIVMSG %s :%s\r\n", ptr_channel->name, pos);
            }
            else
            {
                if (string_is_channel (arguments))
                {
                    ptr_channel = channel_search (server, arguments);
                    if (ptr_channel)
                    {
                        ptr_nick = nick_search (ptr_channel, server->nick);
                        if (ptr_nick)
                        {
                            irc_display_nick (ptr_channel->window, ptr_nick,
                                              MSG_TYPE_NICK, 1, 1, 0);
                            gui_printf_color_type (ptr_channel->window,
                                                   MSG_TYPE_MSG,
                                                   COLOR_WIN_CHAT, "%s\n", pos);
                        }
                        else
                            gui_printf (server->window,
                                        _("%s nick not found for \"%s\" command\n"),
                                        WEECHAT_ERROR, "msg");
                    }
                    server_sendf (server, "PRIVMSG %s :%s\r\n", arguments, pos);
                }
                else
                {
                    ptr_channel = channel_search (server, arguments);
                    if (!ptr_channel)
                    {
                        ptr_channel = channel_new (server, CHAT_PRIVATE, arguments, 1);
                        if (!ptr_channel)
                        {
                            gui_printf (server->window,
                                        _("%s cannot create new private window \"%s\"\n"),
                                        WEECHAT_ERROR,
                                        arguments);
                            return -1;
                        }
                        gui_redraw_window_title (ptr_channel->window);
                    }
                        
                    gui_printf_color_type (ptr_channel->window,
                                           MSG_TYPE_NICK,
                                           COLOR_WIN_CHAT_DARK, "<");
                    gui_printf_color_type (ptr_channel->window,
                                           MSG_TYPE_NICK,
                                           COLOR_WIN_NICK_SELF,
                                           "%s", server->nick);
                    gui_printf_color_type (ptr_channel->window,
                                           MSG_TYPE_NICK,
                                           COLOR_WIN_CHAT_DARK, "> ");
                    gui_printf_color_type (ptr_channel->window,
                                           MSG_TYPE_MSG,
                                           COLOR_WIN_CHAT, "%s\n", pos);
                    server_sendf (server, "PRIVMSG %s :%s\r\n", arguments, pos);
                }
            }
            arguments = pos_comma;
        }
    }
    else
    {
        gui_printf (server->window,
                    _("%s wrong argument count for \"%s\" command\n"),
                    WEECHAT_ERROR, "msg");
        return -1;
    }
    return 0;
}

/*
 * irc_cmd_send_motd: get the "Message Of The Day"
 */

int
irc_cmd_send_motd (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "MOTD %s\r\n", arguments);
    else
        server_sendf (server, "MOTD\r\n");
    return 0;
}

/*
 * irc_cmd_send_names: list nicknames on channels
 */

int
irc_cmd_send_names (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "NAMES %s\r\n", arguments);
    else
    {
        if (!WIN_IS_CHANNEL(gui_current_window))
        {
            gui_printf (server->window,
                        _("%s \"%s\" command can only be executed in a channel window\n"),
                        WEECHAT_ERROR, "names");
            return -1;
        }
        else
            server_sendf (server, "NAMES %s\r\n",
                          CHANNEL(gui_current_window)->name);
    }
    return 0;
}

/*
 * irc_cmd_send_nick: change nickname
 */

int
irc_cmd_send_nick (t_irc_server *server, int argc, char **argv)
{
    if (argc != 1)
        return -1;
    server_sendf (server, "NICK %s\r\n", argv[0]);
    return 0;
}

/*
 * irc_cmd_send_notice: send notice message
 */

int
irc_cmd_send_notice (t_irc_server *server, char *arguments)
{
    server_sendf (server, "NOTICE %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_op: give operator privileges to nickname(s)
 */

int
irc_cmd_send_op (t_irc_server *server, int argc, char **argv)
{
    int i;
    
    if (WIN_IS_CHANNEL(gui_current_window))
    {
        for (i = 0; i < argc; i++)
            server_sendf (server, "MODE %s +o %s\r\n",
                          CHANNEL(gui_current_window)->name,
                          argv[i]);
    }
    else
    {
        gui_printf (server->window,
                    _("%s \"%s\" command can only be executed in a channel window\n"),
                    WEECHAT_ERROR, "op");
        return -1;
    }
    return 0;
}

/*
 * irc_cmd_send_oper: get oper privileges
 */

int
irc_cmd_send_oper (t_irc_server *server, char *arguments)
{
    server_sendf (server, "OPER %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_part: leave a channel or close a private window
 */

int
irc_cmd_send_part (t_irc_server *server, char *arguments)
{
    char *channel_name, *pos_args;
    t_irc_channel *ptr_channel;
    
    if (arguments)
    {
        if (string_is_channel (arguments))
        {
            channel_name = arguments;
            pos_args = strchr (arguments, ' ');
            if (pos_args)
            {
                pos_args[0] = '\0';
                pos_args++;
                while (pos_args[0] == ' ')
                    pos_args++;
            }
        }
        else
        {
            if (WIN_IS_SERVER(gui_current_window))
            {
                gui_printf (server->window,
                            _("%s \"%s\" command can not be executed on a server window\n"),
                            WEECHAT_ERROR, "part");
                return -1;
            }
            channel_name = CHANNEL(gui_current_window)->name;
            pos_args = arguments;
        }
    }
    else
    {
        if (WIN_IS_SERVER(gui_current_window))
        {
            gui_printf (server->window,
                        _("%s \"%s\" command can not be executed on a server window\n"),
                        WEECHAT_ERROR, "part");
            return -1;
        }
        if (WIN_IS_PRIVATE(gui_current_window))
        {
            ptr_channel = CHANNEL(gui_current_window);
            gui_window_free (ptr_channel->window);
            channel_free (server, ptr_channel);
            gui_redraw_window_status (gui_current_window);
            gui_redraw_window_input (gui_current_window);
            return 0;
        }
        channel_name = CHANNEL(gui_current_window)->name;
        pos_args = NULL;
    }
    
    if (pos_args)
        server_sendf (server, "PART %s :%s\r\n", channel_name, pos_args);
    else
        server_sendf (server, "PART %s\r\n", channel_name);
    return 0;
}

/*
 * irc_cmd_send_ping: ping a server
 */

int
irc_cmd_send_ping (t_irc_server *server, char *arguments)
{
    server_sendf (server, "PING %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_pong: send pong answer to a daemon
 */

int
irc_cmd_send_pong (t_irc_server *server, char *arguments)
{
    server_sendf (server, "PONG %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_quit: disconnect from all servers and quit WeeChat
 */

int
irc_cmd_send_quit (t_irc_server *server, char *arguments)
{
    t_irc_server *ptr_server;
    
    /* make gcc happy */
    (void) server;
    
    for (ptr_server = irc_servers; ptr_server;
         ptr_server = ptr_server->next_server)
    {
        if (ptr_server->is_connected)
        {
            if (arguments)
                server_sendf (ptr_server, "QUIT :%s\r\n", arguments);
            else
                server_sendf (ptr_server, "QUIT\r\n");
        }
    }
    quit_weechat = 1;
    return 0;
}

/*
 * irc_cmd_send_quote: send raw data to server
 */

int
irc_cmd_send_quote (t_irc_server *server, char *arguments)
{
    server_sendf (server, "%s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_rehash: tell the server to reload its config file
 */

int
irc_cmd_send_rehash (t_irc_server *server, char *arguments)
{
    /* make gcc happy */
    (void) arguments;
    
    server_sendf (server, "REHASH\r\n");
    return 0;
}

/*
 * irc_cmd_send_restart: tell the server to restart itself
 */

int
irc_cmd_send_restart (t_irc_server *server, char *arguments)
{
    /* make gcc happy */
    (void) arguments;
    
    server_sendf (server, "RESTART\r\n");
    return 0;
}

/*
 * irc_cmd_send_service: register a new service
 */

int
irc_cmd_send_service (t_irc_server *server, char *arguments)
{
    server_sendf (server, "SERVICE %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_servlist: list services currently connected to the network
 */

int
irc_cmd_send_servlist (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "SERVLIST %s\r\n", arguments);
    else
        server_sendf (server, "SERVLIST\r\n");
    return 0;
}

/*
 * irc_cmd_send_squery: deliver a message to a service
 */

int
irc_cmd_send_squery (t_irc_server *server, char *arguments)
{
    server_sendf (server, "SQUERY %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_squit: disconnect server links
 */

int
irc_cmd_send_squit (t_irc_server *server, char *arguments)
{
    server_sendf (server, "SQUIT %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_stats: query statistics about server
 */

int
irc_cmd_send_stats (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "STATS %s\r\n", arguments);
    else
        server_sendf (server, "STATS\r\n");
    return 0;
}

/*
 * irc_cmd_send_summon: give users who are on a host running an IRC server
 *                      a message asking them to please join IRC
 */

int
irc_cmd_send_summon (t_irc_server *server, char *arguments)
{
    server_sendf (server, "SUMMON %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_time: query local time from server
 */

int
irc_cmd_send_time (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "TIME %s\r\n", arguments);
    else
        server_sendf (server, "TIME\r\n");
    return 0;
}

/*
 * irc_cmd_send_topic: get/set topic for a channel
 */

int
irc_cmd_send_topic (t_irc_server *server, char *arguments)
{
    char *channel_name, *new_topic, *pos;
    
    channel_name = NULL;
    new_topic = NULL;
    
    if (arguments)
    {
        if (string_is_channel (arguments))
        {
            channel_name = arguments;
            pos = strchr (arguments, ' ');
            if (pos)
            {
                pos[0] = '\0';
                pos++;
                while (pos[0] == ' ')
                    pos++;
                new_topic = (pos[0]) ? pos : NULL;
            }
        }
        else
            new_topic = arguments;
    }
    
    /* look for current channel if not specified */
    if (!channel_name)
    {
        if (WIN_IS_SERVER(gui_current_window))
        {
            gui_printf (server->window,
                        _("%s \"%s\" command can not be executed on a server window\n"),
                        WEECHAT_ERROR, "topic");
            return -1;
        }
        channel_name = CHANNEL(gui_current_window)->name;
    }
    
    if (new_topic)
    {
        if (strcmp (new_topic, "-delete") == 0)
            server_sendf (server, "TOPIC %s :\r\n", channel_name);
        else
            server_sendf (server, "TOPIC %s :%s\r\n", channel_name, new_topic);
    }
    else
        server_sendf (server, "TOPIC %s\r\n", channel_name);
    return 0;
}

/*
 * irc_cmd_send_trace: find the route to specific server
 */

int
irc_cmd_send_trace (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "TRACE %s\r\n", arguments);
    else
        server_sendf (server, "TRACE\r\n");
    return 0;
}

/*
 * irc_cmd_send_userhost: return a list of information about nicknames
 */

int
irc_cmd_send_userhost (t_irc_server *server, char *arguments)
{
    server_sendf (server, "USERHOST %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_users: list of users logged into the server
 */

int
irc_cmd_send_users (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "USERS %s\r\n", arguments);
    else
        server_sendf (server, "USERS\r\n");
    return 0;
}

/*
 * irc_cmd_send_version: gives the version info of nick or server (current or specified)
 */

int
irc_cmd_send_version (t_irc_server *server, char *arguments)
{
    if (arguments)
    {
        if (WIN_IS_CHANNEL(gui_current_window) &&
            nick_search (CHANNEL(gui_current_window), arguments))
            server_sendf (server, "PRIVMSG %s :\01VERSION\01\r\n",
                          arguments);
        else
            server_sendf (server, "VERSION %s\r\n",
                          arguments);
    }
    else
    {
        irc_display_prefix (server->window, PREFIX_INFO);
        gui_printf (server->window, _("%s, compiled on %s %s\n"),
                    PACKAGE_STRING,
                    __DATE__, __TIME__);
        server_sendf (server, "VERSION\r\n");
    }
    return 0;
}

/*
 * irc_cmd_send_voice: give voice to nickname(s)
 */

int
irc_cmd_send_voice (t_irc_server *server, int argc, char **argv)
{
    int i;
    
    if (WIN_IS_CHANNEL(gui_current_window))
    {
        for (i = 0; i < argc; i++)
            server_sendf (server, "MODE %s +v %s\r\n",
                          CHANNEL(gui_current_window)->name,
                          argv[i]);
    }
    else
    {
        gui_printf (server->window,
                    _("%s \"%s\" command can only be executed in a channel window\n"),
                    WEECHAT_ERROR, "voice");
        return -1;
    }
    return 0;
}

/*
 * irc_cmd_send_wallops: send a message to all currently connected users who
 *                       have set the 'w' user mode for themselves
 */

int
irc_cmd_send_wallops (t_irc_server *server, char *arguments)
{
    server_sendf (server, "WALLOPS %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_who: generate a query which returns a list of information
 */

int
irc_cmd_send_who (t_irc_server *server, char *arguments)
{
    if (arguments)
        server_sendf (server, "WHO %s\r\n", arguments);
    else
        server_sendf (server, "WHO\r\n");
    return 0;
}

/*
 * irc_cmd_send_whois: query information about user(s)
 */

int
irc_cmd_send_whois (t_irc_server *server, char *arguments)
{
    server_sendf (server, "WHOIS %s\r\n", arguments);
    return 0;
}

/*
 * irc_cmd_send_whowas: ask for information about a nickname which no longer exists
 */

int
irc_cmd_send_whowas (t_irc_server *server, char *arguments)
{
    server_sendf (server, "WHOWAS %s\r\n", arguments);
    return 0;
}
