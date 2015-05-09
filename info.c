/*-----------------------------------------------------
Name: info.c
Date: 2015.05.09
Auth: Sun Dro (a.k.a 7th Ghost / kala13x)
Desc: Get and print additional information
-----------------------------------------------------*/


#include "stdinc.h"
#include "info.h"


/*---------------------------------------------
| Get Decoder version
---------------------------------------------*/
const char* get_version()
{
    static char verstr[128];
    sprintf(verstr, "%s Build %d (%s)", VERSION, BUILD, __DATE__);
    return verstr;
}


/*---------------------------------------------
| Greet users
---------------------------------------------*/
void greet(void) 
{
    printf("===================================================\n");
    printf("shttpd: %s\n", get_version());
    printf("===================================================\n");
} 


/*---------------------------------------------
| Print usage
---------------------------------------------*/
void usage(void) 
{
    greet();
    printf("Usage: shttpd [-p <port>] [-a <addr>] [-t <number>] [-h]\n");
    printf("options are:\n");
    printf(" -a <addr>       # address of server\n");
    printf(" -p <port>       # listening port of server\n");
    printf(" -t <number>     # threads proces of server\n");
    printf(" -h              # prints version and usage\n\n");
}
