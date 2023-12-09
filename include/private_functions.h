/* Grupo 58
 * Rodrigo Barrocas 53680
 * Matheus Nunes 47883
 */

#ifndef _PRIVATE_FUNCTIONS_H
#define _PRIVATE_FUNCTIONS_H

// appends a string to another
void strappend(char **str, const char *app);

int parse_address(const char *address_port, char **ip, short *port);

void myIp(char **buffer);

#endif
