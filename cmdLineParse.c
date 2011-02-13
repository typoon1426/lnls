/*   Linux Neighbour logging system
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

struct subnetToken {
	char *token;
	struct subnetToken *next;
};

// DA MODIFICARE LA SPIEGAZIONE DEL DEBUG
const char usage[] = "Usage: nlSystem [OPTIONS]\n"
			"Runs Neighbour Logging System.\n"
			"  -h, --help                 Display this help and exit\n"
			"  -d, --daemonize            Run on background, not valid alone\n"
			"  -D, --debug                Print all packet log on standard output, without timing handling, this option must be used alone\n" 
			"  -O, --stdout               Print all packet log on standard output, this option must be used alone\n"
			"  -s, --syslog               Print all packet log on syslog\n"
			"  -F, --filelog filename     Print all packet log on logfile\n"
			"  -A, --addrfamily           Select address family of packets according to argument\n"
			"  -I, --interfaces	      Select interfaces of packets according to argument\n"
			"  -S, --subnets	      Select subnets of packets according to argument\n";

static char programName[] = "nlSystem";

static unsigned char commandLineRange = 0;
static unsigned char daemonSet = 0;

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

static void filterAddrFamily(char *addrFamily)
{
/*	switch(addrFamily)
	{
		case 'inet':
			setFilter();
			setAF(AF_INET);
		break;

		case 'inet6':
			setFilter();
			setAF(AF_INET6);
		break;

		default:
			// PRE ORA CHIAMA HELP MA SAREBBE MEGLIO STAMPARGLI IL SUO MESSAGGIO DI ERRORE
			help();
		break;
	}*/
}

static void filterInterfaces(char *interfaces)
{
	// VEDERE SE È MEGLIO, MOLTO PROBABILMENTE LO È, FARE DUE PASSAGGIO PRIMA CREAZIONE TOKEN IN UNA LISTA POI CONVERSIONE TOKEN DELLA LISTA.

	/* parsa la stringa passata come argomento e restituisce il token 
	alla funzione di conversione nome int-index che restituisce errore se l'int non esiste
	*/
/*	char *token = NULL;
	unsigned int int_index = 0;

	setFilter();

	do
	{
		if(token != NULL)
			interfaces = NULL;

		token = strtok(interfaces, ",");
		int_index = if_nametoindex(token);

		if(int_index  != 0)
		{
			addInterface(int_index);
		}
		else
		{
			// PRE ORA CHIAMA HELP MA SAREBBE MEGLIO STAMPARGLI IL SUO MESSAGGIO DI ERRORE
			help();
			token = NULL;
		}
	}
	while (token != NULL);
*/
}

static void filterSubnets(char *subnets)
{
	/*char *token = NULL;
	unsigned int counter = 0;
	struct subnetToken *tail = NULL;

	setFilter();

	do
	{
		if(token != NULL)
			subnets = NULL;

		token = strtok(subnets, ";");
		
		if(token != NULL)
		{
			if(tail == NULL)
			{
				tail = malloc(sizeof(struct subnetToken));
				tail->token = token;
				tail->next = NULL;
				counter++;
			}
			else
			{
				struct subnetToken *new = NULL;
				new = malloc(sizeof(struct subnetToken));
				new->token = token;
				new->next = tail;
				tail = new;
				counter++;
			}	
		}
	}
	while (token != NULL);
*/
	
}

// verify if the sum of weight is correct and if daemon is set daemonize the process
static void verifyCmd(void)
{
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
		else
			return;
	}
	else
	{
		printUsage();
		exit(0);
	}
	
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
				
				case 'A':
					filterAddrFamily(optarg);
				break;

				case 'I':
					filterInterfaces(optarg);
				break;

				case 'S':
					filterSubnets(optarg);
				break;

				case 'h':
				case '?':
					help();
				break;
			}
		}
	}
	
	verifyCmd();
}
