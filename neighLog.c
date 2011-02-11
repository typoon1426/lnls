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
 *		Main Module
 */

/* RIGUARDARE TODO SUPPORTO ARGOMENTI RIGA COMANDO, FILE CONFIGURAZIONE */

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <net/if.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>

#include "neighLog.h"
#include "logging.h"
#include "nl2Neigh.h"
#include "hashHandlers.h"
#include "cmdLineParse.h"
#include "timer.h"

static struct sockaddr_nl src_addr, dest_addr;
static struct iovec iov;
static struct msghdr msg;
static struct nlmsghdr *nlMsg;
static int fd; 

// TODO
static void cleanExit(void)
{
	exit(0);
}

// init structures
static void initStruct(void)
{
	memset(&src_addr, 0, sizeof(src_addr));
 	memset(&dest_addr, 0, sizeof(dest_addr));

	src_addr.nl_family = AF_NETLINK;
 	src_addr.nl_pid = getpid(); // get my pid  
	src_addr.nl_groups = RTMGRP_NEIGH; // set netlink route protocol, neighbour multicast group

	nlMsg = (struct nlmsghdr *) malloc(NLMSG_SPACE(BUFLENGTH));

	if(nlMsg == NULL)
	{
		perror("Malloc");
		exit(1);
	}
	memset(nlMsg, 0, NLMSG_SPACE(BUFLENGTH));

	iov.iov_base = (void *) nlMsg;
 	iov.iov_len = NLMSG_SPACE(BUFLENGTH);

	msg.msg_name = (void *) &dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
}

// bind socket
static void socketBind(void)
{
	int ret;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
 
	if(fd < 0)
	{
		perror("Socket");
		exit(1);
	}

	ret = bind(fd, (struct sockaddr*) &src_addr, sizeof(src_addr));

	if(ret < 0)
	{
		perror("Bind");
		exit(1);
	}
}

// call function to send packet in hash table and log
static void pktSave(struct neighBourBlock *neighBour)
{
	int ret = 0;
	
	// mask SIGALRM 
	mask();

	ret = neighHashFindAdd(neighBour);
	
	unMask();

	if(ret == LOG)
	{
		logWrite(neighBour);
	}
}

static void mainLoop(void)
{
	int ret;

	while(1)
	{
		ret = recvmsg(fd, &msg, 0);

		if((ret < 0) && (errno != EINTR))
		{
			perror("Recvmsg");
			exit(1);
		}
		else if(ret >= 0)
		{
			// check packet integrity
			if(packetTest(nlMsg, ret))
			{
				struct neighBourBlock *neighBour = parseNlPacket(nlMsg);
				
				if(neighBour != NULL)
				{
					if(getMode() == DEBUG) {
						debugPrint(neighBour);
						
						free(neighBour);
					}
					else
					{
						// funzione che verifica nella hash table se è presente ed eventualmente logga secondo il logging configurato
						pktSave(neighBour);						
					}
				}			
			}
		}
	}
}

// XXX VERIFICARE SIG NON SERVE FORSE QUINDI MODIFICARE STAMPE
static void defSigHandler(int sig)
{
	switch (getMode())
	{
		case STDOUT:
		{
			printf("Caught signal %d, close all socket and exiting.\n", sig);
			cleanExit(); //TODO
		}
		break;

		case FILEOUT:	
		{
			fprintf(getFileLogStream(), "Caught signal %d, close all socket and exiting.\n", sig);
			cleanExit(); //TODO
		}			
		break;

		case SYSLOG:
		{	
			char log[PRINTOUTBUF];

			snprintf(log, sizeof(log), "Caught signal %d, close all socket and exiting.\n", sig);
			syslog(LOG_ERR, log);
			cleanExit(); //TODO
		}		
		break;
	}
}

static void setSignalHandlers()
{
	// all signals - SIGALRM
	struct sigaction actionIgnore, actionDefault;
	int i, signals[] = {SIGHUP,/* SIGINT, */SIGPIPE, SIGTERM, SIGUSR1, SIGUSR2, SIGPROF, SIGVTALRM};

	memset(&actionIgnore, 0, sizeof(actionIgnore));
	memset(&actionDefault, 0, sizeof(actionDefault));

	actionIgnore.sa_handler = SIG_IGN;
	actionDefault.sa_handler = defSigHandler;

	for(i = 0; i<ARRAY_LENGTH(signals); i++)
	{
		if((signals[i] == SIGHUP) || (signals[i] == SIGINT) || (signals[i] == SIGTERM))
		{
			if(sigaction(signals[i], &actionDefault, NULL) < 0)
			{
				perror("sigaction:");
				exit(1);
			}
		}
		else
		{
			if(sigaction(signals[i], &actionIgnore, NULL) < 0)
			{
				perror("sigaction:");
				exit(1);
			}
		}
	}

	// intervalTimerStart to set sigalrm handling
	intervalTimerStart();
}

int main(int argc, char *argv[])
{
	#ifdef __DEBUG__
	printf("entry setsig handler\n");
	#endif

	// signal handlers
	setSignalHandlers();

	#ifdef __DEBUG__
	printf("exit setsig handler\n");
	#endif
	
	#ifdef __DEBUG__
	printf("entry mask\n");
	#endif

	// mask SIGALRM 
	mask();

	#ifdef __DEBUG__
	printf("exit mask\n");
	#endif

	#ifdef __DEBUG__
	printf("entry parsecmdline\n");
	#endif

	// parse command line arguments
	parseCmdLine(argc, argv);
	
	#ifdef __DEBUG__
	printf("exit parsecmdline\n");
	#endif

	#ifdef __DEBUG__
	printf("entry initstruct\n");
	#endif

	// init all structures
	initStruct();

	#ifdef __DEBUG__
	printf("exit initstruct\n");
	#endif

	#ifdef __DEBUG__
	printf("entry socketbind\n");
	#endif

	// open socket and bind
	socketBind();

	#ifdef __DEBUG__
	printf("exit socketbind\n");
	#endif

	#ifdef __DEBUG__
	printf("entry unmask\n");
	#endif

	// unmask SIGALRM for start interval timer
	if(getMode() != DEBUG)
		unMask();

	#ifdef __DEBUG__
	printf("exit unmask\n");
	#endif

	#ifdef __DEBUG__
	printf("entry mainloop\n");
	#endif

	// main loop
	mainLoop();
	
	return 0;
}
