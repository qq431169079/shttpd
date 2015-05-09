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


#include "stdinc.h"
#include "headers.h"
#include "parser.h"
#include "info.h"
#include "slog.h"

#define ISspace(x) isspace((int)(x))
#define MAXMSG  512


/*---------------------------------------------
| Main structure of variables
---------------------------------------------*/
typedef struct {
    int port;
    int threads;
    char* root;
    char* addr;
} values;


/*---------------------------------------------
| Initialise variables
---------------------------------------------*/
void init_values(values *val)
{
    val->port = 69;
    val->threads = 0;
    val->root = NULL;
}


/*---------------------------------------------
| Read SIGPIPE signal
---------------------------------------------*/
void sig_handler(int sig) 
{
    /* Handle signals */
    if (sig == SIGILL || sig == SIGSEGV) 
    {
        slog(0, "[ERROR] Internal server error");
        exit(-1);
    }

    /* Handle exit signal */
    if (sig == SIGINT) 
    {
        slog(0, "[LIVE] Cleanup on exit");
        exit(0);    
    }

    slog(0, "[SIGPIPE] Broken Pipe");
}


/*---------------------------------------------
| Create and return socket
---------------------------------------------*/
int create_socket(uint16_t port)
{
    /* Used variables */
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


/*---------------------------------------------
| Cat file
---------------------------------------------*/
void cat_file(int client, FILE *resource)
{
    char buf[1024];

    /* Read file context */
    fgets(buf, sizeof(buf), resource);
    while (!feof(resource))
    {
        send(client, buf, strlen(buf), 0);
        fgets(buf, sizeof(buf), resource);
    }
}


/*---------------------------------------------
| Return file to client
---------------------------------------------*/
void return_file(int client, const char *filename)
{
    FILE *resource = NULL;

    /* Read file */
    resource = fopen(filename, "r");
    if (resource == NULL) not_found(client, (char*)filename);
    else
    {
        headers(client, filename);
        cat_file(client, resource);
    }
    fclose(resource);
}


/*---------------------------------------------
| Read request from client and answer
---------------------------------------------*/
int accept_request(int client, char *root)
{
    /* Used variables */
    struct stat st;
    char *query_string = NULL;
    char buffer[MAXMSG];
    char method[255];
    char path[256];
    char url[256];
    int nbytes;
    size_t i, j;

    /* Read Reaquest */
    nbytes = read(client, buffer, MAXMSG);

    if (nbytes < 0)
    {
        perror("read");
        return 0;
    }
    else if (!nbytes) return 1;
    else
    {
        /* Print incoming request if verbose flag enabled */
        slog(2, "[LIVE] Incoming request:");
        slog(2, "%s", buffer);

        i = 0; j = 0;
        while (!ISspace(buffer[j]) && (i < sizeof(method) - 1))
        {
            method[i] = buffer[j];
            i++; j++;
        }
        method[i] = '\0';

        /* Get request type */
        if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
        {
            unimplemented(client);
            return 1;
        }

        i = 0;
        while (ISspace(buffer[j]) && (j < sizeof(buffer))) j++;
        while (!ISspace(buffer[j]) && (i < sizeof(url) - 1) && (j < sizeof(buffer)))
        {
            url[i] = buffer[j];
            i++; j++;
        }
        url[i] = '\0';

        if (strcasecmp(method, "GET") == 0)
        {
            query_string = url;
            while ((*query_string != '?') && (*query_string != '\0'))
                query_string++;
        }

        sprintf(path, "%s%s", root, url);
        slog(0, "[LIVE] Get * %s", url);

        if (path[strlen(path) - 1] == '/') strcat(path, "index.html");
        if (stat(path, &st) == -1) 
        {
            slog(1, "[ERROR] 404! Requested content not found: %s", path);
            not_found(client, url);
        }
        else
        {
            if ((st.st_mode & S_IFMT) == S_IFDIR) strcat(path, "/index.html");
            return_file(client, path);
        }
    }

    return 1;
}


/*---------------------------------------------
| Parse config file
---------------------------------------------*/
static int handler(void *config, const char *section, const char *name,
                   const char *value)
{
    values *val = (values*)config;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("config", "threads")) {
        val->threads = atoi(value);
    } else if (MATCH("config", "port")) {
        val->port = atoi(value);
    } else if (MATCH("config", "server")) {
        val->addr = strdup(value);;
    } else if (MATCH("config", "root")) {
        val->root = strdup(value);
    } else {
        return -1;
    }

    return 1;
}


/*---------------------------------------------
| Parse commandline arguments
---------------------------------------------*/
int parse_arguments(int argc, char *argv[], values *val)
{
    int c;
    while ( (c = getopt(argc, argv, "a:p:t:h1")) != -1) {
        switch (c) {
        case 'a':
            val->addr = strdup(optarg);
            break;
        case 'p':
            val->port = atoi(optarg);
            break;
        case 't':
            val->threads = atoi(optarg);
            break;
        case 'h':
        default:
            usage();
            return -1;
        }
    }

    return 0;
}


/*---------------------------------------------
| Main function
---------------------------------------------*/
int main(int argc, char *argv[])
{
    /* Used variables */
    int i, sock, s_new;
    struct sockaddr_in clientname;
    fd_set active_fd_set, read_fd_set;
    socklen_t size;
    values val;

    /* Read signals */
    signal(SIGPIPE, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGSEGV, sig_handler);
    signal(SIGILL , sig_handler);

    /* Greet */
    greet();

    /* Initialise variables */
    init_values(&val);
    init_slog("shttpd", 3);

    /* Read Config File */
    if (parse_cfg("config.cfg", handler, &val) < 0) 
        slog(0, "[ERROR] Unable to parse file 'config.cfg'");
    else
        slog(3, "[LIVE] Config parsed successfully.");

    /* Parse Commandline Arguments */
    if (parse_arguments(argc, argv, &val) >= 0) 
        slog(3, "[LIVE] Argument parsed successfully.");
    else return 0;

    /* Create the socket and listen to cennections */
    sock = create_socket(val.port);
    if (listen(sock, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /* Print starting information */
    slog(0, "[LIVE] Server started listen at port: %d ", val.port);

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
                    if(accept_request(i, val.root)) 
                    {
                        close(i);
                        FD_CLR(i, &active_fd_set);
                    }
                }
            }
        }
    }
}
