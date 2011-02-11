/*   Linux Neighbour logging system
 *   developed within the VirtualSquare project
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

#include "logging.h"
#include "neighLog.h"

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

// DA MODIFICARE LA SPIEGAZIONE DEL DEBUG
const char usage[] = "Usage: neigh_log [OPTIONS]\n"
			"Runs Neighbour Logging System.\n"
			"  -h, --help                 Display this help and exit\n"
			"  -d, --daemonize            Run on background, not valid alone\n"
			"  -D, --debug                Print all packet log on standard output, without timing handling, this option must used alone\n" 
			"  -O,  --std-output           Print all packet log on standard output, this option must used alone\n"
			"  -s, --syslog               Print all packet log on syslog\n"
			"  -F,  --log-file filename    Print all packet log on logfile\n";

static char programName[] = "neigh_log";

static unsigned char commandLineRange = 0;
static unsigned char daemonSet = 0;

static void printUsage(void)
{
	fprintf(stdout, "%s", usage);
}

// EXPERIMENTAL
/*static void confFile(char *fileName)
{
	// add weight
	commandLineRange += CONF_WEIGHT;
}
*/
// EXPERIMENTAL
/*static void groupByInterface(char *intName)
{
	// add weight
	commandLineRange += GROUPBYINT_WEIGHT;
	if(interfaceExist())
	{
		// SETTA INTERFACCIA PER RAGGRUPPARE I LOG
	}
	else
	{
		// STAMPA INTERFACCIA NON ESISTE + EXIT1
	}
}
*/
static void fileLog(char *logFileName)
{
	FILE *stream = NULL;

	// add weight
	commandLineRange += FILELOG_WEIGHT;

	stream = fopen(logFileName, "a");
	if(stream == NULL)
	{
		perror("logfile open:");
		exit(1);
	}
	setMode(FILEOUT);

	#ifdef __DEBUG1__
	printf("logfile handler\n");
	#endif
}

static void daemonize(void)
{
	// add weight
	commandLineRange += DAEMONIZE_WEIGHT;
	// TODO EXEC DAEMONIZE
	daemonSet = 1;

	#ifdef __DEBUG1__
	printf("daemonize handler\n");
	#endif
}

static void stdOutput(void)
{
	// add weight
	commandLineRange += STDOUT_WEIGHT;
	setMode(STDOUT);

	#ifdef __DEBUG1__
	printf("stdout handler\n");
	#endif
}

static void sysLog(void)
{
	// add weight
	commandLineRange += SYSLOG_WEIGHT;
	// set syslog logging
	openlog(programName, 0, LOG_SYSLOG);
	setMode(SYSLOG);

	#ifdef __DEBUG1__
	printf("syslog handler\n");
	#endif
}

static void help(void)
{
	// add weight
	commandLineRange += HELP_WEIGHT;

	#ifdef __DEBUG1__
	printf("help handler\n");
	#endif
}

static void debug(void)
{
	// add weight
	commandLineRange += DEBUG_WEIGHT;

	setMode(DEBUG);

	#ifdef __DEBUG1__
	printf("debug handler\n");
	#endif
}

// parsing command line
void parseCmdLine(int argc, char *argv[])
{
	if(argc > 1)
	{
		int i;
		for(i=1; i<argc; i++)
		{
			if((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
			{
				help();

				#ifdef __DEBUG1__
				printf("help\n");
				#endif
			}
			/*else if((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--conf-file") == 0))
			{
				confFile();
			}*/
			else if((strcmp(argv[i], "-D") == 0) || (strcmp(argv[i], "--debug") == 0))
			{
				debug();
				#ifdef __DEBUG1__
				printf("debug\n");
				#endif
			}
			else if((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--daemonize") == 0))
			{
				daemonize();
				#ifdef __DEBUG1__
				printf("daemonize\n");
				#endif
			}
			else if((strcmp(argv[i], "-O") == 0) || (strcmp(argv[i], "--std-output") == 0))
			{
				stdOutput();
				#ifdef __DEBUG1__
				printf("stdout\n");
				#endif
			}
			/*else if((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--group-by-interface") == 0))
			{
				groupByInterface();
			}*/
			else if((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--syslog") == 0))
			{
				sysLog();
				#ifdef __DEBUG1__
				printf("syslog\n");
				#endif
			}
			else if((strcmp(argv[i], "-F") == 0) || (strcmp(argv[i], "--log-file") == 0))
			{
				if((i+1) < argc)
				{
					fileLog(argv[i+1]);
				}
				else
					commandLineRange = MAXRANGE;

				#ifdef __DEBUG1__
				printf("logfile\n");
				#endif
			}
		}

		#ifdef __DEBUG1__
		printf("commandLineRange %d\n", commandLineRange);
		#endif

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
			return;
		}
		else
		{
			printUsage();
			exit(1);
		}
	}
	else
	{
		printUsage();
		exit(1);
	}
}


