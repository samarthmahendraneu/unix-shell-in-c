#ifndef CS5600_OSPSH_H
#define CS5600_OSPSH_H

/*
 * Header file for Lab 2 - Shell processing
 * This file contains the definitions required for executing commands
 * parsed by the code in cmdparse.c
 */

#include "cmdparse.h"

/* Execute the command list. */
int cmd_line_exec(command_t *);

#endif
