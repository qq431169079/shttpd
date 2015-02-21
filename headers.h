/*--------------------------------------------
Name: header.h
Description: header file of header functions       
--------------------------------------------*/

#include <stdio.h>
#define SERVERAGENT "shttpd/0.3.1\r\n"

/*--------------------------------------------
Server status OK header                     */
void headers(int client, const char *filename);

/*--------------------------------------------
Not found heasder                           */
void not_found(int client);

/*--------------------------------------------
Bad request heasder                         */
void bad_request(int client);

/*--------------------------------------------
Internal error heasder                      */
void internal_error(int client);

/*--------------------------------------------
Unimplemented error heasder                 */
void unimplemented(int client);