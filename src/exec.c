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

static char *ip4RecCommand= NULL;
static char *ip4RecArguments = NULL;
static char *ip6RecCommand= NULL;
static char *ip6RecArguments = NULL;

static char *ip4DelCommand= NULL;
static char *ip4DelArguments = NULL;
static char *ip6DelCommand= NULL;
static char *ip6DelArguments = NULL;

static void execRx(struct neighBourBlock *neighBour, unsigned char AF)
{
	pid_t newPid = fork();
	
	if(newPid == 0)
	{
		// son process call execve
		int ret = execve(//XXX NOME COMANDO, ARGOMENTI, VARIABILIAMBIENTE);

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

static void execDel(struct neighBourBlock *neighBour, unsigned char AF)
{

}

void setExec4RecCmd(char *ip4RecCmd)
{
	
}

void setExec6RecCmd(char *ip6RecCmd)
{

}

void setExec4DelCmd(char *ip4DelCmd)
{

}

void setExec6DelCmd(char *ip6DelCmd)
{

}

inline void hookRcvPair4(struct neighBourBlock *neighBour)
{
	execRx(neighBour, AF_INET);
}

inline void hookRcvPair6(struct neighBourBlock *neighBour)
{
	execRx(neighBour, AF_INET6);
}

inline void hookRemPair4(struct neighBourBlock *neighBour)
{
	execDel(neighBour, AF_INET);
}

inline void hookRemPair6(struct neighBourBlock *neighBour)
{
	execDel(neighBour, AF_INET6);
}
