/*---------------------------------------------------------------------------

 shttpd is simple http non-blocking web server which uses tcp sockets
 and select().

 Copyright (c) 2015 Sun Dro (a.k.a. 7th Ghost)
 Web: http://off-sec.com/ ; E-Mail: kala0x13@gmail.com

 This is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This software is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

---------------------------------------------------------------------------*/


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netdb.h>
#include <signal.h>
#include "parser.h"
#include "headers.h"

#define ISspace(x) isspace((int)(x))
#define MAXMSG  512
#define VERSION "0.3 Nightly"
#define BUILD 9


/*-----------------------------------------
Main structure of variables              */
typedef struct {
	int port;
	int threads;
	short verbose;
	char *root;
} values;


/*-----------------------------------------
Initialise variables                     */
void MainValues_init(values *val)
{
	val->verbose = 0;
	val->port = 69;
	val->threads = 0;
	val->root = NULL;
}


/*---------------------------------------------------------------------
Get version                                                  */
const char* GetVersion()
{
	static char verstr[128];
	sprintf(verstr, "%s Build %d", VERSION, BUILD);
	return verstr;
}


/*---------------------------------------------------------------------
Print help message                                                   */
void usage(void) 
{
	printf("=========================================\n");
	printf("shttpd Version: %s\n", GetVersion());
	printf("=========================================\n");
	printf("Usage: shttpd [-p <port>] [-t <number>] [-v] [-h]\n");
	printf("options are:\n");
	printf(" -p <port>       # listening port of server\n");
	printf(" -t <number>     # threads proces of server\n");
	printf(" -v              # verbose\n");
	printf(" -h              # prints version and usage\n\n");
}


/*--------------------------------------------
Read SIGPIPE signal                         */
void sig_handler() 
{
	printf("%s\n", "SIGPIPE: Broken Pipe!");
}


/* Create socket */
int create_socket(uint16_t port)
{
	int sock;
	struct sockaddr_in name;

	/* Create the socket. */
	sock = socket (PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	/* Give the socket a name. */
	name.sin_family = AF_INET;
	name.sin_port = htons (port);
	name.sin_addr.s_addr = htonl (INADDR_ANY);

	/* Bind socket */
	if (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	return sock;
}


/*------------------------------------------
Cat file                                  */
void cat_file(int client, FILE *resource)
{
	char buf[1024];

	fgets(buf, sizeof(buf), resource);
	while (!feof(resource))
	{
		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}
}


/*------------------------------------------
Read request from client                  */
int read_request(int sock, char *buf, int size)
{
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n'))
	{
		n = read(sock, &c, 1);
		if (n > 0)
		{
			if (c == '\r')
			{
				n = read(sock, &c, 1);
				if ((n > 0) && (c == '\n')) read(sock, &c, 1);
				else c = '\n';
			}
			buf[i] = c;
			i++;
		}
		else c = '\n';
	}
	buf[i] = '\0';
 
	return i;
}


/*------------------------------------------
Return file to client                     */
void return_file(int client, const char *filename)
{
	FILE *resource = NULL;
	int nbytes = 1;
	char buf[1024];

	buf[0] = 'A'; buf[1] = '\0';
	while ((nbytes > 0) && strcmp("\n", buf)) nbytes = read_request(client, buf, sizeof(buf));

	resource = fopen(filename, "r");
	if (resource == NULL) not_found(client);
	else
	{
		headers(client, filename);
		cat_file(client, resource);
	}
	fclose(resource);
}


/*------------------------------------------
Read request from client and answer       */
int accept_request(int client, int verbose, char *path)
{
	char buf[MAXMSG];
	int nbytes;
	char method[255];
	char url[255];
	size_t i, j;
	struct stat st;
	char *query_string = NULL;

	nbytes = read_request(client, buf, sizeof(buf));
	i = 0; j = 0;
	while (!ISspace(buf[j]) && (i < sizeof(method) - 1))
	{
		method[i] = buf[j];
		i++; j++;
	}
	method[i] = '\0';

	if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
	{
		unimplemented(client);
		return 1;
	}

	i = 0;
	while (ISspace(buf[j]) && (j < sizeof(buf))) j++;
	while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
	{
		url[i] = buf[j];
		i++; j++;
	}
	url[i] = '\0';

	if (strcasecmp(method, "GET") == 0)
	{
		query_string = url;
		while ((*query_string != '?') && (*query_string != '\0'))
			query_string++;
	}

	sprintf(path, "%s%s", path, url);
	if (path[strlen(path) - 1] == '/') strcat(path, "index.html");
	if (stat(path, &st) == -1) 
	{
		while ((nbytes > 0) && strcmp("\n", buf))
		nbytes = read_request(client, buf, sizeof(buf));
		not_found(client);
	}
	else
	{
		if ((st.st_mode & S_IFMT) == S_IFDIR) strcat(path, "/index.html");
		return_file(client, path);
	}

	return 1;
}


/*---------------------------------------------------------------------
Parse config file                                                    */
static int handler(void *config, const char *section, const char *name,
                   const char *value)
{
	values *val = (values*)config;

	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("config", "threads")) {
		val->threads = atoi(value);
	} else if (MATCH("config", "port")) {
		val->port = atoi(value);
	} else if (MATCH("config", "verbose")) {
		val->verbose = atoi(value);
	} else if (MATCH("config", "root")) {
		val->root = strdup(value);
	} else {
		return -1;
	}

	if(val->verbose) printf("Config parsed successfully.\n");

	return 1;
}


/*---------------------------------------------------------------------
Parse commandline arguments                                          */
int parse_arguments(int argc, char *argv[], values *val)
{
	int c;
	while ( (c = getopt(argc, argv, "p:t:v1:h1")) != -1) {
		switch (c) {
		case 'p':
			val->port = atoi(optarg);
			break;
		case 't':
			val->threads = atoi(optarg);
			break;
		case 'v':
			val->verbose = 1;
			break;
		case 'h':
		default:
			usage();
			return -1;
		}
	}

	if(val->verbose) printf("Argument parsed successfully.\n");

	return 0;
}


/* Main function */
int main(int argc, char *argv[])
{
	int i;
	int sock;
	int s_new;
	struct sockaddr_in clientname;
	fd_set active_fd_set, read_fd_set;
	socklen_t size;

	/* Read SIGPIPE signal */
	signal(SIGPIPE, sig_handler);

	/* Used variables */
	values val;

	/* Initialise used variables */
	MainValues_init(&val);

	/* Read Config File */
	if (parse_cfg("config.cfg", handler, &val) < 0) 
		printf("%s\n", "Unable to parse file 'config.cfg'.");

	/* Parse Commandline Arguments */
	if (parse_arguments(argc, argv, &val)) return 0;

	/* Create the socket and listen to cennections */
	sock = create_socket(val.port);
	if (listen(sock, 1) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	/* Print starting information */
	printf("%s%d\n", "shttpd server started at port: ", val.port);

	/* Initialize the set of active sockets. */
	FD_ZERO(&active_fd_set);
	FD_SET(sock, &active_fd_set);

	/* Main loop (never ends) */
	while(1)
	{
		/* Block until input arrives on one or more active sockets. */
		read_fd_set = active_fd_set;
		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
		{
			perror("select");
			exit(EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i) 
		{
			if (FD_ISSET(i, &read_fd_set))
			{
				if (i == sock)
				{
					/* Connection request on original socket. */
					size = sizeof(clientname);
					s_new = accept(sock, (struct sockaddr *)&clientname, &size);
					if (s_new < 0)
					{
						perror("accept");
						exit(EXIT_FAILURE);
					}
					FD_SET(s_new, &active_fd_set);
				}
				else
				{
					/* Read request and answer */
					if(accept_request(i, val.verbose, val.root)) 
					{
						close(i);
						FD_CLR(i, &active_fd_set);
					}
				}
			}
		}
	}
}
