/*-----------------------------------------------------
Name: info.h
Date: 2015.05.09
Auth: Sun Dro (a.k.a 7th Ghost / kala13x)
Desc: Get and print additional information
-----------------------------------------------------*/

#include "stdinc.h"

#define VERSION "1.0.3 Snapshot"
#define BUILD 75


/*---------------------------------------------
| Get Decoder version
---------------------------------------------*/
const char* get_version();


/*---------------------------------------------
| Greet users
---------------------------------------------*/
void greet(void);


/*---------------------------------------------
| Print usage
---------------------------------------------*/
void usage(void);
