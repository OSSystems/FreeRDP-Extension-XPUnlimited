/* -*- c-basic-offset: 8 -*-
   freerdp: A Remote Desktop Protocol client.
   XP Unlimited protocol - Load balancing client extension

   Copyright (C) 2010 Eduardo Beloni <beloni@ossystems.com.br>
   Copyright (C) 2010 O.S. Systems Software LTDA.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/limits.h>
#include <freerdp/freerdp.h>
#include <freerdp/rdpset.h>

#include "xpu.h"

#define BUFFSIZE         10000
#define LINESIZE         300

/* from variable definitions in rdp_set.h */
#define USERLEN          256
#define SERVERLEN        64


struct xpu_server
{
	char addr[SERVERLEN];
	char user[USERLEN];
	int rdp_port;
	int xpu_port;
};

static int
xpu_create_socket(const char *server, int port)
{
	struct sockaddr_in sockaddr;
	int fd;
	struct hostent *host;

	host = gethostbyname(server);
	if (!host)
	{
		ERROR("XPU: get host by name failed\n");
		return -1;
	}

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		ERROR("XPU: unable to create socket\n");
		return -1;
	}

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr = *((struct in_addr *)host->h_addr);
	memset(&sockaddr.sin_zero, 0, 8);

	if(connect(fd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)) < 0)
	{
		ERROR("failed to connect %s:%d...\n", server, port);
		return -1;
	}

	return fd;
}

static int
xpu_send_request_c001(int fd, const char *user)
{
	char line[LINESIZE];

	sprintf(line, "v001c001%s\r\n", user);
	return send(fd, line, strlen(line) - 1, 0);
}

static void
xpu_remove_cr(char *buff)
{
	int i, j;
	char c[BUFFSIZE];

	for (i = 0, j = 0; buff[i]; i++)
	{
		if (buff[i] == '\r')
			continue;
		c[j++] = buff[i];
	}
	c[j] = '\0';
	strcpy(buff, c);
}

static char *
xpu_get_filepath(const struct xpu_server *server)
{
	char *homedir;
	char path[PATH_MAX];

	homedir = getenv("HOME");
	if (!homedir)
	{
		ERROR("getenv failed to retrieve home path\n");
		return NULL;
	}

	sprintf(path, "%s/.freerdp/%s.servers", homedir, server->addr);

	return strdup(path);
}

static void
xpu_store_buff_into_file(const char *buff, const struct xpu_server *server)
{
	char *path;
	FILE *fdout;

	path = xpu_get_filepath(server);
	if (!path)
	{
		ERROR("unable to get config file path\n");
		return;
	}

	fdout = fopen(path, "w");
	free(path);

	if (!fdout)
	{
		ERROR("when trying to open file %s\n", path);
		return;
	}

	fprintf(fdout, "%s", buff);

	fclose(fdout);
}

static int
xpu_request_file_and_store_prefered(struct xpu_server *server)
{
	char buff[BUFFSIZE];
	char line[LINESIZE];
	char fmt[20];
	ssize_t len, total;
	int sockfd;

	DEBUG("requesting file from server %s port %d\n", server->addr, server->xpu_port);

	sockfd = xpu_create_socket(server->addr, server->xpu_port);
	if (sockfd < 0)
	{
		ERROR("unable to create socket for %s:%d\n", server->addr, server->xpu_port);
		return 0;
	}

	len = xpu_send_request_c001(sockfd, server->user);
	if (len <= 0)
	{
		ERROR("send request c001 failed %s %s %d\n", server->user, server->addr, server->xpu_port);
		return 0;
	}

	len = total = 0;
	do
	{
		if ((len = recv(sockfd, line, LINESIZE, 0)) <= 0)
		{
			ERROR("recv there is no more data on socket\n");
			return 0;
		}

		if ((total + len) >= BUFFSIZE)
		{
			ERROR("recv buffer too small to store data\n");
			return 0;
		}

		memcpy(&buff[total], line, len);
		total += len;
		buff[total] = '\0'; /* needed to check strcmp */
	}
	while (strcmp(&buff[total - 5], "EOF\r\n"));

#ifndef WIN32
	xpu_remove_cr(buff);
#endif
	DEBUG("file contents:\n%s---------------------------\n", buff);

	/* first write into file */
	xpu_store_buff_into_file(buff, server);

	/* then modify server data according to the data received */
	sprintf(fmt, "%%%ds %%d", SERVERLEN - 1);
	sscanf(buff, fmt, server->addr, &server->rdp_port);
	DEBUG("prefered %s %d\n", server->addr, server->rdp_port);

	return 1;
}

/* reads v001c001 file, fills servers (puts the prefered one at index 0) and returns array size */
static int
xpu_read_file_into_array(FILE *fdin, struct xpu_server *servers)
{
	int i;
	char line[LINESIZE];
	char fmt[20];

	if (fgets(line, LINESIZE, fdin))
	{
		sprintf(fmt, "%%%ds %%d", SERVERLEN - 1);
		sscanf(line, fmt, servers[0].addr, &servers[0].rdp_port);
		servers[0].xpu_port = -1;
	}
	else
		return 0;

	sprintf(fmt, "%%%ds %%d %%d", SERVERLEN - 1);
	i = 1;
	while (fgets(line, LINESIZE, fdin) && strncmp(line, "EOF", 3))
	{
		int inc = 1; /* tells us if 'i' must be incremented or not */

		if (line[0] == '#') /* comment found */
		{
			inc = 0;

			/* read this line till the end */
			while (line[strlen(line) - 1] != '\n')
				if (!fgets(line, LINESIZE, fdin))
					break;
		}

		if (inc)
		{
			sscanf(line, fmt, servers[i].addr, &servers[i].rdp_port, &servers[i].xpu_port);
			if (strcmp(servers[i].addr, servers[0].addr) == 0 &&
				servers[i].rdp_port == servers[0].rdp_port)
			{
				servers[0].xpu_port = servers[i].xpu_port;
				inc = 0;
			}
		}

		i += inc;
	}

	return i;
}

static int
xpu_iterate_thru_file(struct xpu_server *server)
{
	struct xpu_server servers[LINESIZE];
	int success = 0;
	int i, size;
	FILE *fdin;
	char *path;

	path = xpu_get_filepath(server);
	if (!path)
	{
		ERROR("unable to get config file path\n");
		return 0;
	}

	fdin = fopen(path, "r");
	free(path);

	if (!fdin)
	{
		ERROR("when trying to open file %s\n", path);
		return 0;
	}

	size = xpu_read_file_into_array(fdin, servers);
	fclose(fdin);

	for (i = 0; i < size; i++)
	{
		strcpy(servers[i].user, server->user);
		success = xpu_request_file_and_store_prefered(&servers[i]);
		if (success)
		{
			strcpy(server->addr, servers[i].addr);
			server->rdp_port = servers[i].rdp_port;
			break;
		}
	}

	return success;
}

void
xpu_prefered_server(char *rdp_server, int *rdp_port, const char *user, int xpu_port)
{
	int success = 0;
	struct xpu_server pref_server;

	/* insert rdp settings into prefered object */
	strcpy(pref_server.addr, rdp_server);
	strcpy(pref_server.user, user);
	pref_server.rdp_port = *rdp_port;
	pref_server.xpu_port = xpu_port;

	success = xpu_iterate_thru_file(&pref_server);

	/* there is no such file or we were unable to get
		something useful from the file: try cmd line parameters */
	if (!success)
		success = xpu_request_file_and_store_prefered(&pref_server);

	if (success)
	{
		/* fill rdp settings with prefered */
		strcpy(rdp_server, pref_server.addr);
		*rdp_port = pref_server.rdp_port;
	}
}
