/*-------------------------------------------------
 Name:               parser.h
 Description:        Header File Parser
--------------------------------------------------*/

#ifndef __INI_H__
#define __INI_H__

/*---------------------------------------------------------
For include header in CPP code */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/*---------------------------------------------------------
Parse File                                           */
int parse_cfg(const char* filename,
              int (*handler)(void* user, const char* section,
                             const char* name, const char* value),
              void* user);

/*---------------------------------------------------------
Same as parse_cfg, but takes FILE* instead of filename.  */
int parse_cfg_file(FILE* file,
                   int (*handler)(void* user, const char* section,
                                  const char* name, const char* value),
                   void* user);

/*---------------------------------------------------------
Nonzero to allow multi-line value parsing                */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

/*---------------------------------------------------------
Nonzero to allow a UTF-8 BOM                             */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

/*---------------------------------------------------------
Nonzero to use stack, zero to use heap (malloc/free)     */
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif

/*---------------------------------------------------------
Stop parsing on first error (default is to keep parsing) */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

/*---------------------------------------------------------
Define maximum line length                               */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

#ifdef __cplusplus
}
#endif

#endif
