/*   Linux Neighbour logging system Version 0.1
 *   developed as part of VirtualSquare project
 *   
 *   Copyright 2010 Michele Cucchi <cucchi@cs.unibo.it>
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

#include "logging.h"
#include "nlSystem.h"
#include "filters.h"

#define DAEMONIZE_WEIGHT 1
#define GROUPBYINT_WEIGHT 2
#define SYSLOG_WEIGHT 11
#define FILELOG_WEIGHT 11
#define CONF_WEIGHT 13
#define STDOUT_WEIGHT 14
#define HELP_WEIGHT 15
#define DEBUG_WEIGHT 14
#define MINRANGE 10
#define MAXRANGE 15

const char usage[] = "Usage: nlogger [OPTIONS]\n"
			"Runs Neighbour Logging System.\n"
			"  -h, --help                 		Display this help and exit\n"
			"  -d, --daemonize            		Run on background. Optional argument, not valid with debug, help and stdout\n"
			"  -D, --debug                		Print all packet log on standard output, without timing handling. Not valid with other options.\n" 
			"  -O, --stdout               		Print all packet log on standard output, not valid with: daemonize, debug, filelog and syslog.\n"
			"  -s, --syslog               		Print all packet log on syslog, not valid with: debug, stdout and filelog\n"
			"  -F, --filelog filename     		Print all packet log on logfile, not valid with: debug, stdout and syslog\n"
			"  -A, --addrfamily inet|inet6		Force to log only packets of address family selected.\n"
			"  -I, --interfaces int1,int2,..   	Force to log only packets from interfaces selected.\n"
			"  -S, --subnets sub/mask,sub/mask1,.. 	Force to log only packets of subnets selected.\n";

static char programName[] = "nlogger";

static unsigned char daemonSet = 0, commandLineRange = 0, afCalled = FALSE, interfacesCalled = FALSE, subnetsCalled = FALSE;

static void printUsage(void)
{
	fprintf(stdout, "%s", usage);
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
	openlog(programName, 0, LOG_SYSLOG);
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

#ifdef __EXPERIMENTAL__
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
#endif

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
			{"addrfamily", 1, 0, 'A'},
			{"interfaces", 1, 0, 'I'},
			{"subnets", 1, 0, 'S'},
			{0, 0, 0, 0}
			};

			c = getopt_long(argc, argv, "dDhOsF:A:I:S:", long_options, &option_index);
			if (c == -1)
				break;

			switch (c)
			{
				case 'd':
					daemonize();
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
				#ifdef __EXPERIMENTAL__
				case 'A':
					filterAddrFamily(optarg);
				break;

				case 'I':
					filterInterfaces(optarg);
				break;

				case 'S':
					filterSubnets(optarg);
				break;
				#endif
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
		}
	}
	else
	{
		printUsage();
		exit(0);
	}
}
