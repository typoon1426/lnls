/*   Linux Neighbour logging system Version 0.2
 *   developed as part of VirtualSquare project
 *   
 *   Copyright 2010,2012 Michele Cucchi <cucchi@cs.unibo.it>
 *   
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 or 
 *   (at your opinion) any later version, as published by the Free Software Foundation.
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
 *		Command Line parser module
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "logging.h"
#include "nlSystem.h"
#include "filters.h"
#include "hashHandlers.h"
#include "cmdLineParse.h"
#include "exec.h"

struct argumentToken {
	char *token;
	int tokenLen;
	
	struct argumentToken *next;
	struct argumentToken *prev;
};

struct tokenQueueHead {
	struct argumentToken *head;
	struct argumentToken *tail;

	int tokenCount;
	int tokenLenSum;
};

static struct tokenQueueHead external;
static struct tokenQueueHead internal;

static const char usage[] = 	"Usage: lnls [OPTIONS]\n"
				"Runs Neighbour Logging System.\n"
				"  -h, --help                 		Display this help and exit.\n"
				"  -d, --daemonize            		Run on background. Optional argument, not valid with debug, help and stdout.\n"
				"  -p, --pidfile filename       		Write the pid of lnls in the file named \"filename\", mandatory and usable only with daemonize.\n"
				"  -D, --debug                		Print all packet log on standard output, without timing handling. Not valid with other options.\n" 
				"  -O, --stdout               		Print all packet log on standard output, not valid with: daemonize, debug, filelog and syslog.\n"
				"  -s, --syslog               		Print all packet log on syslog, not valid with: debug, stdout and filelog.\n"
				"  -F, --filelog filename     		Print all packet log on logfile, not valid with: debug, stdout and syslog.\n"
				"  -A, --addrfamily inet|inet6		Force to log only packets of address family selected.\n"
				"  -I, --interfaces int1,int2,..   	Force to log only packets from interfaces selected.\n"
				"  -S, --subnets sub/mask,sub/mask1,.. 	Force to log only packets of subnets selected.\n"
				"  -T, --timeout 			Set hash table flushing timeout, default is 360 seconds, min timeout is 1 second and max timeout is 3600 seconds.\n"
				"  -x, --exec-rx4			Set a command to exec when receiving a neighbour with IPv4 address.\n"
				"  -X, --exec-rx6			Set a command to exec when receiving a neighbour with IPv6 address.\n"
				"  -z, --exec-del4			Set a command to exec when a neighbour with IPv4 address has expired.\n"
				"  -Z, --exec-del6			Set a command to exec when a neighbour with IPv6 address has expired.\n";

static const char programNameStarted[] = "lnls started";
static const char programName[] = "lnls";

static unsigned char daemonSet = 0, commandLineRange = 0, afCalled = FALSE, interfacesCalled = FALSE, subnetsCalled = FALSE, timeoutCalled = FALSE, execRX4Called = FALSE, execRX6Called = FALSE, execDel4Called = FALSE, execDel6Called = FALSE;

inline unsigned char execRX4Setted(void)
{
	return execRX4Called;
}

inline unsigned char execRX6Setted(void)
{
	return execRX6Called;
}

inline unsigned char execDel4Setted(void)
{
	return execDel4Called;
}

inline unsigned char execDel6Setted(void)
{
	return execDel6Called;
}

static inline void printUsage(void)
{
	fprintf(stdout, "%s", usage);
}

static inline int getTokenLenSum(struct tokenQueueHead *headQ)
{
	return headQ->tokenLenSum;
}

static inline int getTokenCount(struct tokenQueueHead *headQ)
{
	return headQ->tokenCount;
}

static inline unsigned char emptyTokenQueue(struct tokenQueueHead *headQ)
{
	if(headQ->tokenCount > 0)
		return FALSE;
	else	
		return TRUE;
}

static void enqueueToken(struct tokenQueueHead *headQ, char *token)
{
	struct argumentToken *newToken = malloc(sizeof(struct argumentToken));
	
	if(newToken == NULL)
	{
		perror("Malloc newToken error:");
		exit(1);
	}
	
	memset(newToken, 0, sizeof(struct argumentToken));

	newToken->token = token;
	newToken->next = newToken->prev = NULL;
	newToken->tokenLen = strlen(token);

	if(headQ->tokenCount == 0)
		headQ->head = headQ->tail = newToken;
	else
	{
		
		headQ->tail->next = newToken;
		newToken->prev = headQ->tail;
		headQ->tail = newToken;
	}
	
	headQ->tokenCount++;
	headQ->tokenLenSum += newToken->tokenLen;
}

static struct argumentToken *dequeueToken(struct tokenQueueHead *headQ)
{
	struct argumentToken *deQueuedToken = NULL;

	if(headQ->tokenCount != 0)
	{
		deQueuedToken = headQ->head;
		headQ->head = deQueuedToken->next;

		if(headQ->head != NULL)
			headQ->head->prev = NULL;
		else
			headQ->tail = headQ->head;
		
		headQ->tokenCount--;
		headQ->tokenLenSum -= deQueuedToken->tokenLen;
	}
	
	return deQueuedToken;
}

static inline void tokenFree(struct argumentToken *token)
{
	free(token);
}

static void tokenizeCmdNameArgs(char **commandName, char ***commandArgs, char *inputString)
{
	char *token = NULL;
	char *newSubToken = NULL;
	
	do
	{
		if(token != NULL)
			inputString = NULL;
	
		token = strtok_r(inputString, " ", &newSubToken);
		
		if(token != NULL)
		{
			if(inputString != NULL)
			{
				*commandName = token;
				enqueueToken(&external, token);
			}			
			else
				enqueueToken(&external, token);
		}
	}
	while (token != NULL);

	int count = 0;
	
	*commandArgs = calloc(getTokenCount(&external)+1, sizeof(char *));

	if(*commandArgs == NULL)
	{
		perror("Calloc commandargs error:");
		exit(1);
	}

	while(!emptyTokenQueue(&external))
	{
		struct argumentToken *deqToken = dequeueToken(&external);
	
		(*commandArgs)[count] = deqToken->token;
		count++;
		tokenFree(deqToken); 
	}
}

static void fileLog(char *logFileName)
{
	FILE *stream = NULL;

	// add weight
	commandLineRange += FILELOG_WEIGHT;

	stream = fopen(logFileName, "a+");
	if(stream == NULL)
	{
		perror("logfile open:");
		exit(1);
	}

	setMode(FILEOUT);
	setFileLogStream(stream);
}

static void daemonize(void)
{
	// add weight
	commandLineRange += DAEMONIZE_WEIGHT;
	daemonSet = 1;
}

static void pidFile(char *pidFileName)
{
	commandLineRange += PIDFILE_WEIGHT;

	// save pidfile filename
	saveFileName(pidFileName);
}

static void stdOutput(void)
{
	// add weight
	commandLineRange += STDOUT_WEIGHT;
	setMode(STDOUT);
}

static void sysLog(void)
{
	// add weight
	commandLineRange += SYSLOG_WEIGHT;
	// set syslog logging
	openlog(programName, LOG_PID, LOG_SYSLOG);
	setMode(SYSLOG);
}

static void help(void)
{
	// add weight
	commandLineRange += HELP_WEIGHT;
}

static void debug(void)
{
	// add weight
	commandLineRange += DEBUG_WEIGHT;
	setMode(DEBUG);
}

static void filterAddrFamily(char *addrFamily)
{
	
	if(afCalled == FALSE)
	{
		afCalled = TRUE;

		filtersInit();

		if(strcmp((const char *) addrFamily, "inet") == 0)
			filterSetAF(AF_INET);
		else if(strcmp((const char *) addrFamily, "inet6") == 0)
			filterSetAF(AF_INET6);
		else
			help();
	}
	else
		help();
}

// parse argument and call function to add interfaces to filters
static void filterInterfaces(char *interfaces)
{
	if(interfacesCalled == FALSE)
	{
		interfacesCalled = TRUE;

		char *token = NULL;
		char *newSubToken = NULL;
		unsigned int int_index = 0;

		filtersInit();

		do
		{
			if(token != NULL)
				interfaces = NULL;

			token = strtok_r(interfaces, ",", &newSubToken);
			
			if(token != NULL)
			{
				int_index = if_nametoindex(token);
		
				if(int_index  != 0)
					filterAddInterface(int_index);
				else
				{
					help();
					token = NULL;
				}
			}
		}
		while (token != NULL);
	}
	else
		help();
}

static void filterSubnets(char *subNetsArg)
{
	char *subnets = subNetsArg;

	if(subnetsCalled == FALSE)
	{
		char *token = NULL;
		char *newSubToken = NULL;

		subnetsCalled = TRUE;
		filtersInit();

		token = strtok_r(subnets, ",", &newSubToken);

		while(token != NULL)
		{			
			if(!filterAddSubnet(token))
			{
				help();
				token = NULL;
			}

			token = strtok_r(NULL, ",", &newSubToken);
		}

		if(token == NULL)
		{
			filterSubnetEnd();
		}	
	}
	else
		help();
}

static void setTimeout(char *timeOut)
{
	if(timeoutCalled == FALSE)
	{
		int intTimeout = atoi(timeOut);

		timeoutCalled = TRUE;
		
		if((intTimeout <= 0) || (intTimeout >= MAXTIMEOUT))
			help();
		else
			setIpGcExpire(intTimeout);
	}
	else
		help();
}

static void setIP4RxCommand(char *ip4RxCmd)
{
	if(execRX4Called == FALSE)
	{
		execRX4Called = TRUE;
		
		char *IP4RxCmdName = NULL;
		char **IP4RxCmdArgs = NULL;
		
		tokenizeCmdNameArgs(&IP4RxCmdName, &IP4RxCmdArgs, ip4RxCmd);
		setExecIP4RxCmd(IP4RxCmdName, IP4RxCmdArgs);
	}
	else
		help();
}

static void setIP6RxCommand(char *ip6RxCmd)
{
	if(execRX6Called == FALSE)
	{
		execRX6Called = TRUE;
		
		char *IP6RxCmdName = NULL;
		char **IP6RxCmdArgs = NULL;

		tokenizeCmdNameArgs(&IP6RxCmdName, &IP6RxCmdArgs, ip6RxCmd);
		
		setExecIP6RxCmd(IP6RxCmdName, IP6RxCmdArgs);
	}
	else
		help();
}

static void setIP4DelCommand(char *ip4DelCmd)
{
	if(execDel4Called == FALSE)
	{
		execDel4Called = TRUE;
		
		char *IP4DelCmdName = NULL;
		char **IP4DelCmdArgs = NULL;
		
		tokenizeCmdNameArgs(&IP4DelCmdName, &IP4DelCmdArgs, ip4DelCmd);

		setExecIP4DelCmd(IP4DelCmdName, IP4DelCmdArgs);
	}
	else
		help();
}

static void setIP6DelCommand(char *ip6DelCmd)
{
	if(execDel6Called == FALSE)
	{
		execDel6Called = TRUE;
		
		char *IP6DelCmdName = NULL;
		char **IP6DelCmdArgs = NULL;
		
		tokenizeCmdNameArgs(&IP6DelCmdName, &IP6DelCmdArgs, ip6DelCmd);

		setExecIP6DelCmd(IP6DelCmdName, IP6DelCmdArgs);
	}
	else
		help();
}

// parsing command line with getopt_long function
void parseCmdLine(int argc, char *argv[])
{
	opterr = 0;
	int c = 0;
	
	if(argc == 1)
		help();
	else
	{
		while(1)
		{
			int option_index = 0;
			static struct option long_options[] = {
			{"daemonize", 0, 0, 'd'},
			{"debug", 0, 0, 'D'},
			{"help", 0, 0, 'h'},
			{"stdout", 0, 0, 'O'},
			{"syslog", 0, 0, 's'},
			{"filelog", 1, 0, 'F'},
			{"pidfile", 1, 0, 'p'},
			{"addrfamily", 1, 0, 'A'},
			{"interfaces", 1, 0, 'I'},
			{"subnets", 1, 0, 'S'},
			{"timeout", 1, 0, 'T'},
			{"exec-rx4", 1, 0, 'x'},
			{"exec-rx6", 1, 0, 'X'},
			{"exec-del4", 1, 0, 'z'},
			{"exec-del6", 1, 0, 'Z'},
			{0, 0, 0, 0}
			};

			c = getopt_long(argc, argv, "dDhOsF:A:I:S:p:T:x:X:z:Z:", long_options, &option_index);
			if (c == -1)
				break;

			switch (c)
			{
				case 'd':
					daemonize();
				break;

				case 'p':
					pidFile(optarg);
				break;

				case 'D':
					debug();
				break;

				case 'O':
					stdOutput();
				break;

				case 's':
					sysLog();
				break;

				case 'F':
					fileLog(optarg);
				break;

				case 'A':
					filterAddrFamily(optarg);
				break;

				case 'I':
					filterInterfaces(optarg);
				break;

				case 'S':
					filterSubnets(optarg);
				break;

				case 'T':
					setTimeout(optarg);
				break;
				
				case 'x':
					setIP4RxCommand(optarg);
				break;
				
				case 'X':
					setIP6RxCommand(optarg);
				break;
				
				case 'z':
					setIP4DelCommand(optarg);
				break;
				
				case 'Z':
					setIP6DelCommand(optarg);
				break;

				case 'h':
				case '?':
					help();
				break;
			}
		}
	}
	
	if((commandLineRange > MINRANGE) && (commandLineRange < MAXRANGE))
	{
		if(daemonSet == 1)
		{
			char* pidFileName = getPidFileName();

			if(strlen(pidFileName) != 0)
			{
				if(daemon(0, 0) < 0)
				{
					perror("daemon syscall error:");
					exit(1);
				}
				else
				{
					FILE *stream;
				
					logPrint((char *) programNameStarted, NULL, FALSE);

					// write in pidfile mypid
					if((stream = fopen(pidFileName, "w")) == NULL) {
						logError("pidfile file stream open failed\0");
						exit(1);
					}

					if(fprintf(stream, "%d\n", getpid()) <= 0) {
						logError("pid fprintf failed\0");
						exit(1);
					}

					fclose(stream);
				}
			}
			else
			{
				printUsage();
				exit(0);
			}
		}
	}
	else
	{
		printUsage();
		exit(0);
	}
}
