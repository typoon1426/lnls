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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "exec.h"
#include "logging.h"

static const char inet[] = "AF=INET\0";
static const char inet6[] = "AF=INET6\0";
static const char l3[] = "L3=";
static const char l2[] = "L2=";
static const char iface[] = "IFACE="; 
static const char tstamp[] = "TSTAMP=";

static char *envVars[5];
static char addressFamily[AF_LEN];
static char L3Addr[L3_LEN];
static char L2Addr[L2_LEN];
static char ifName[IFACE_LEN];
static char lastSeenTS[ASCII_TSTAMP_LEN]; // TODO DEFINIRE LUNGHEZZA E TIPO DEL TIMESTAMP

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
	// array elements
	envVars[0] = addressFamily;
	envVars[1] = L3Addr;
	envVars[2] = L2Addr;
	envVars[3] = ifName;
	envVars[4] = lastSeenTS;
	
	// timestamp
	snprintf(lastSeenTS, ASCII_TSTAMP_LEN, "%s%lld", tstamp, (long long int) neighBour->last_seen);
	
	// interface name
	memcpy(ifName, iface, 6);
	memcpy(ifName+6, neighBour->if_name, IFACE_LEN-6);
	ifName[IFACE_LEN-1] = 0;
	
	// level 2 address
	memcpy(L2Addr, l2, 3);
	L2Addr[L2_LEN-1] = 0;
	etherAddr2Str(neighBour->etherAddr, L2Addr+3, ETH_ALEN, L2_LEN-3);
	
	// level 3 address
	memcpy(L3Addr, l3, 3);
	L3Addr[L3_LEN-1] = 0;
		
	if(neighBour->addressFamily == AF_INET)
	{
		memcpy(addressFamily, inet, AF_LEN-1);
		inet2Ascii(neighBour->inetAddr, L3Addr+3, INETLEN, L3_LEN-3);
	}
	else if(neighBour->addressFamily == AF_INET6)
	{
		memcpy(addressFamily, inet6, AF_LEN);
		inet2Ascii(neighBour->inet6Addr, L3Addr+3, INET6LEN, L3_LEN-3);
	}	
}

static inline void setCmd(char *cmdName, char *cmdArgs, unsigned char opCode, unsigned char AF)
{
	if(opCode == RX)
	{
		if(AF == AF_INET)
		{
			cmdName = ip4RxCmdName;
			cmdArgs = ip4RxCmdArgs;
		}
		else
		{
			cmdName = ip6RxCmdName;
			cmdArgs = ip6RxCmdArgs;
		}
	}
	else if(opCode == DEL)
	{
		if(AF == AF_INET)
		{
			cmdName = ip4DelCmdName;
			cmdArgs = ip4DelCmdArgs;
		}
		else
		{
			cmdName = ip6DelCmdName;
			cmdArgs = ip6DelCmdArgs;
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
				logErrorStatus(errorStr);
			}
			else
				logErrorStatus(genErrorStr);
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
