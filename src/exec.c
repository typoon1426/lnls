/*   Linux Neighbour logging system Version 0.2
 *   developed as part of VirtualSquare project
 *   
 *   Copyright 2012 Michele Cucchi <cucchi@cs.unibo.it>
 *   
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2, as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA.
 *
 */

/* 
 *		Exec module
 */
 
#include <unistd.h>
#include "exec.h"

static char *envVars[6];
static char addressFamily[6];
static char L3Addr[ASCII_BUF];
static char L2Addr[ASCII_BUF];
static char ifName[IF_NAMESIZE];
static char lastSeenTS[ASCII_TS_LEN]; // TODO DEFINIRE LUNGHEZZA E TIPO DEL TIMESTAMP

static char *ip4RxCmdName = NULL;
static char *ip4RxCmdArgs = NULL;
static char *ip6RxCmdName = NULL;
static char *ip6RxCmdArgs = NULL;

static char *ip4DelCmdName = NULL;
static char *ip4DelCmdArgs = NULL;
static char *ip6DelCmdName = NULL;
static char *ip6DelCmdArgs = NULL;

static void setEnvVars(struct neighBourBlock *neighBour)
{
	if(neighBour->addressFamily == AF_INET)
	{
		
	}
	else if(neighBour->addressFamily == AF_INET6)
	{
	
	}
}

static inline void setCmd(char *cmdName, char *cmdArgs, unsigned char opCode, unsigned char AF)
{
	if(opCode == RX)
	{
		if(AF == AF_INET)
		{
			commandName = ip4RxCmdName;
			commandArgs = ip4RxCmdArgs:
		}
		else
		{
			commandName = ip6RxCmdName;
			commandArgs = ip6RxCmdArgs:
		}
	}
	else if(opCode == DEL)
	{
		if(AF == AF_INET)
		{
			commandName = ip4DelCmdName;
			commandArgs = ip4DelCmdArgs:
		}
		else
		{
			commandName = ip6DelCmdName;
			commandArgs = ip6DelCmdArgs:
		}
	}
}

void execCmd(struct neighBourBlock *neighBour, unsigned char opCode, unsigned char AF)
{
	pid_t newPid = 0;
	char *commandName = NULL;
	char *commandArgs = NULL;
	
	setCmd(commandName, commandArgs, opCode, AF);
	setEnvVars(neighBour);
	
	newPid = fork();
	
	if(newPid == 0)
	{
		// son process call execve
		int ret = execve(commandName, commandArgs, envVars);

		// unreachable point on success
		if(ret == -1)
		{
			perror("Execve error");
			exit(1);
		}
	}
	else if(newPid < 0)
	{
		// handling error
		perror("Fork error");
		exit(1);
	}
	else if(newPid > 0)
	{
		int status = 0, retValue = 0;
		
		// father process call waitpid
		pid_t retWait = waitpid(newPid, &status, 0);
		
		if(retWait < 0)
		{
			perror("Waitpid error");
			exit(1);
		}
		else
		{
			// Handling waitpid return status
			if(WIFEXITED(status))
			{
				retValue = WEXITSTATUS(status);
				returnStatusLog(//XXX NOME COMANDO, valore ritorno);
			}
			else
			{
				returnStatusLog(//XXX NOME COMANDO, valore ritorno, ERRORE);
			}
		}
	}
}

inline void setExecIP4RxCmd(char *ip4RxCommand, char *ip4RxArguments)
{
	ip4RxCmdName = ip4RxCommand;
	ip4RxCmdArgs = ip4RxArguments;
}

inline void setExecIP6RxCmd(char *ip6RxCommand, char *ip6RxArguments)
{
	ip6RxCmdName = ip6RxCommand;
	ip6RxCmdArgs = ip6RxArguments;
}

inline void setExecIP4DelCmd(char *ip4DelCommand, char *ip4DelArguments)
{
	ip4DelCmdName = ip4DelCommand;
	ip4DelCmdArgs = ip4DelArguments;
}

inline void setExecIP6DelCmd(char *ip6DelCommand, char *ip6DelArguments)
{
	ip6DelCmdName = ip6DelCommand;
	ip6DelCmdArgs = ip6DelArguments;
}
