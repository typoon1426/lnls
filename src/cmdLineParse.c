/*   Linux Neighbour logging system Version 0.1
 *   developed as part of VirtualSquare project
 *   
 *   Copyright 2010,2012 Michele Cucchi <cucchi@cs.unibo.it>
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
	
	int tokenCount;
	int tokenLenSum;
};

static struct tokenQueueHead external;
static struct tokenQueueHead internal;

static const char usage[] = "Usage: lnls [OPTIONS]\n"
			"Runs Neighbour Logging System.\n"
			"  -h, --help                 		Display this help and exit\n"
			"  -d, --daemonize            		Run on background. Optional argument, not valid with debug, help and stdout\n"
			"  -p, --pidfile filename       		Write the pid of lnls in the file named \"filename\", valid only with daemonize\n"
			"  -D, --debug                		Print all packet log on standard output, without timing handling. Not valid with other options.\n" 
			"  -O, --stdout               		Print all packet log on standard output, not valid with: daemonize, debug, filelog and syslog.\n"
			"  -s, --syslog               		Print all packet log on syslog, not valid with: debug, stdout and filelog\n"
			"  -F, --filelog filename     		Print all packet log on logfile, not valid with: debug, stdout and syslog\n"
			"  -A, --addrfamily inet|inet6		Force to log only packets of address family selected.\n"
			"  -I, --interfaces int1,int2,..   	Force to log only packets from interfaces selected.\n"
			"  -S, --subnets sub/mask,sub/mask1,.. 	Force to log only packets of subnets selected.\n"
			"  -T, --timeout 	Set hash table flushing timeout, default is 360 seconds\n"
			"  -x, --exec-rx4		Set a command to exec when receiving a neighbour with IPv4 address\n"
			"  -X, --exec-rx6		Set a command to exec when receiving a neighbour with IPv6 address\n"
			"  -z, --exec-del4		Set a command to exec when a neighbour with IPv4 address has expired\n"
			"  -Z, --exec-del6		Set a command to exec when a neighbour with IPv6 address has expired\n";

static const char programName[] = "lnls";
static const char started[] = "started";
static unsigned char daemonSet = 0, commandLineRange = 0, afCalled = FALSE, interfacesCalled = FALSE, subnetsCalled = FALSE, timeoutCalled = FALSE, execRX4 = FALSE, execRX6 = FALSE, execDel4 = FALSE, execDel6 = FALSE;

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
	
	newToken->token = token;
	newToken->next = newToken->prev = NULL;
	newToken->tokenLen = strlen(token);

	if(headQ->tokenCount == 0)
	{
		headQ->head = newToken;
	}
	else
	{
		headQ->head->next = newToken;
		newToken->prev = headQ->head;
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
		
		headQ->tokenCount--;
		headQ->tokenLenSum -= deQueuedToken->tokenLen;
	}
	
	return deQueuedToken;
}

static inline void tokenFree(struct argumentToken *token)
{
	free(token);
}

static void flushInternalTokenFifo(void)
{
	// Handling tokes in the fifo
	int len = getTokenLenSum(&internal);
	int tokenCount = getTokenCount(&internal);
	int offset = 0;
	char *argument = (char *) malloc((len+tokenCount)*sizeof(char));

	if(argument == NULL)
	{
		perror("Malloc argument error:");
		exit(1);
	}

	memset(argument, 0, (len+tokenCount)*sizeof(char));
	
	while(!emptyTokenQueue(&internal))
	{
		struct argumentToken *deqToken = dequeueToken(&internal);
		
		memcpy(argument+offset, deqToken->token, deqToken->tokenLen); // QUI NON DEVE COPIARE IL \0
		memset(argument+offset+deqToken->tokenLen, 0x20, 1);
		offset += deqToken->tokenLen+1;
		
		// free token pointer
		free(deqToken->token);
		// free token struct
		tokenFree(deqToken); 
	}

	enqueueToken(&external, argument);
}

static void tokenizeCmdNameArgs(char *commandName, char **commandArgs, char *inputString)
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
				commandName = token;
				enqueueToken(&external, token); // Each token is a pair of a cli argument and is value
			}			
			else
			{
				if(strncmp(token, "-", 1) == 0)
				{
					if(!emptyTokenQueue(&internal))
					{
						flushInternalTokenFifo();
						// insert the new token in the empty fifo
						enqueueToken(&internal, token);
					}
					else
						enqueueToken(&internal, token);
				}
				else
					enqueueToken(&internal, token);
			}
		}
		else
			flushInternalTokenFifo();
	}
	while (token != NULL);

	commandArgs = calloc(getTokenCount(&external)+1, sizeof(char *));
	
	if(commandArgs == NULL)
	{
		perror("Calloc commandargs error:");
		exit(1);
	}
	
	int count = 0;
	
	while(!emptyTokenQueue(&external))
	{
		struct argumentToken *deqToken = dequeueToken(&external);
		
		commandArgs[count] = deqToken->token;
		count++;
		tokenFree(deqToken); //XXX free argument token struct and his internal pointer to token string
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
	if(execRX4 == FALSE)
	{
		execRX4 = TRUE;
		
		char *IP4RxCmdName = NULL;
		char **IP4RxCmdArgs = NULL;
		
		tokenizeCmdNameArgs(IP4RxCmdName, IP4RxCmdArgs, ip4RxCmd);
		//extractCmdNameArgs(IP4RxCmdName, IP4RxCmdArgs, ip4RxCmd, MAXSTRCMD_LEN);
		
		setExecIP4RxCmd(IP4RxCmdName, IP4RxCmdArgs);
	}
	else
		help();
}

static void setIP6RxCommand(char *ip6RxCmd)
{
	if(execRX6 == FALSE)
	{
		execRX6 = TRUE;
		
		char *IP6RxCmdName = NULL;
		char **IP6RxCmdArgs = NULL;

		tokenizeCmdNameArgs(IP6RxCmdName, IP6RxCmdArgs, ip6RxCmd);
		//extractCmdNameArgs(IP6RxCmdName, IP6RxCmdArgs, ip6RxCmd, MAXSTRCMD_LEN);
		
		setExecIP6RxCmd(IP6RxCmdName, IP6RxCmdArgs);
	}
	else
		help();
}

static void setIP4DelCommand(char *ip4DelCmd)
{
	if(execDel4 == FALSE)
	{
		execDel4 = TRUE;
		
		char *IP4DelCmdName = NULL;
		char **IP4DelCmdArgs = NULL;
		
		tokenizeCmdNameArgs(IP4DelCmdName, IP4DelCmdArgs, ip4DelCmd);
		//extractCmdNameArgs(IP4DelCmdName, IP4DelCmdArgs, ip4DelCmd, MAXSTRCMD_LEN);

		setExecIP4DelCmd(IP4DelCmdName, IP4DelCmdArgs);
	}
	else
		help();
}

static void setIP6DelCommand(char *ip6DelCmd)
{
	if(execDel6 == FALSE)
	{
		execDel6 = TRUE;
		
		char *IP6DelCmdName = NULL;
		char **IP6DelCmdArgs = NULL;
		
		tokenizeCmdNameArgs(IP6DelCmdName, IP6DelCmdArgs, ip6DelCmd);
		//extractCmdNameArgs(IP6DelCmdName, IP6DelCmdArgs, ip6DelCmd, MAXSTRCMD_LEN);

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
	{
		help();
	}
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
			if(daemon(0, 0) < 0)
			{
				perror("daemon :");
				exit(1);
			}
			else
			{
				FILE *stream;

				// log lnls started
				switch(getMode())
				{
					case STDOUT:
						fprintf(stdout, "%s %s\n", programName, started);
					break;

					case FILEOUT:	
						fprintf(getFileLogStream(), "%s %s\n", programName, started);
					break;

					case SYSLOG:	
						syslog(LOG_INFO, "%s %s", programName, started);
					break;
				}

				// write in pidfile mypid
				if((stream = fopen(getPidFileName(), "w")) == NULL) {
					syslog(LOG_INFO, "file stream open failed, %u", errno);
					exit(1);
				}
	
				if(fprintf(stream, "%d\n", getpid()) <= 0) {
					syslog(LOG_INFO, "pid fprintf failed, %u", errno);
					exit(1);
				}

				fclose(stream);
			}
		}
	}
	else
	{
		printUsage();
		exit(0);
	}
}
