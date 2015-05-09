/*-----------------------------------------------------
Name: header.h
Date: 2015.05.09
Auth: Sun Dro (a.k.a 7th Ghost / kala13x)
Desc: Header file of header functions
-----------------------------------------------------*/


#include "stdinc.h"
#define SERVERAGENT "shttpd/1.0.3\r\n"


/*---------------------------------------------
| Server status OK header
---------------------------------------------*/
void headers(int client, const char *filename);


/*---------------------------------------------
| Not found heasder
---------------------------------------------*/
void not_found(int client, char* path);


/*---------------------------------------------
| Bad request heasder
---------------------------------------------*/
void bad_request(int client);


/*---------------------------------------------
| Internal error heasder
---------------------------------------------*/
void internal_error(int client);


/*---------------------------------------------
| Unimplemented error heasder                 
---------------------------------------------*/
void unimplemented(int client);