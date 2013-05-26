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
 *		Main Module
 */

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

#include "nlSystem.h"
#include "logging.h"
#include "nl2Neigh.h"
#include "hashHandlers.h"
#include "cmdLineParse.h"
#include "timer.h"
#include "filters.h"
#include "exec.h"

static struct sockaddr_nl src_addr, dest_addr, toKern_addr;
static struct iovec rcvIov, sndIov;
static struct msghdr rcvMsg, sndMsg;
static struct nlmsghdr *rcvNlMsg;
static struct nlmsghdr sndNlMsg;
static int fd; 

// Close all file handlers and remove pidfile
static void cleanExit(void)
{
	if(getFileLogStream() != NULL)
	{
		closeLogFile();
	}

	char *pidFilePtr = getPidFileName();
	
	if(pidFilePtr != NULL)
	{
		// remove pidfile
		if(unlink(pidFilePtr) != 0)
		{
			FILE *stream = getFileLogStream();
			if(stream != NULL)
			{
				fprintf(stream, "Errore unlink file, impossibile cancellare");
				closeLogFile();
			}
			else
				syslog(LOG_ERR, "Errore unlink file, impossibile cancellare");
		}
	}

	exit(0);
}

// init structures
static void initStruct(void)
{
	// reset structures
	memset(&src_addr, 0, sizeof(src_addr));
 	memset(&dest_addr, 0, sizeof(dest_addr));
	memset(&toKern_addr, 0, sizeof(toKern_addr));
	memset(&rcvMsg, 0, sizeof(rcvMsg));
	memset(&sndMsg, 0, sizeof(sndMsg));
	memset(&rcvIov, 0, sizeof(rcvIov));
	memset(&sndIov, 0, sizeof(sndIov));
	memset(&sndNlMsg, 0, sizeof(sndNlMsg));

	src_addr.nl_family = AF_NETLINK;
 	src_addr.nl_pid = getpid(); // get my pid  
	src_addr.nl_groups = RTMGRP_NEIGH; // set netlink, route protocol, neighbour multicast group

	toKern_addr.nl_family = AF_NETLINK;
	toKern_addr.nl_pid = 0;
	toKern_addr.nl_groups = 0;

	rcvNlMsg = (struct nlmsghdr *) malloc(NLMSG_SPACE(BUFLENGTH));
	

	if(rcvNlMsg == NULL)
	{
		logError("Malloc initstruct\0");
		exit(1);
	}

	
	memset(rcvNlMsg, 0, NLMSG_SPACE(BUFLENGTH));
	
	rcvIov.iov_base = (void *) rcvNlMsg;
 	rcvIov.iov_len = NLMSG_SPACE(BUFLENGTH);

	rcvMsg.msg_name = (void *) &dest_addr;
	rcvMsg.msg_namelen = sizeof(dest_addr);
	rcvMsg.msg_iov = &rcvIov;
	rcvMsg.msg_iovlen = 1;

	sndNlMsg.nlmsg_len = sizeof(sndNlMsg);
	sndNlMsg.nlmsg_type = 0;
	sndNlMsg.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	sndNlMsg.nlmsg_seq = 0;
	sndNlMsg.nlmsg_pid = getpid();

	sndIov.iov_base = (void *) &sndNlMsg;
 	sndIov.iov_len = sizeof(sndNlMsg);

	sndMsg.msg_name = (void *) &toKern_addr;
	sndMsg.msg_namelen = sizeof(toKern_addr);
	sndMsg.msg_iov = &sndIov;
	sndMsg.msg_iovlen = 1;
}

// bind socket
static void socketBind(void)
{
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
 
	if(fd < 0)
	{
		logError("Socket NETLINK\0");
		exit(1);
	}

	if(bind(fd, (struct sockaddr*) &src_addr, sizeof(src_addr)) < 0)
	{
		logError("Bind\0");
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
	
	// unmask SIGALRM 
	unMask();

	if(ret == LOG)
	{
		// Call an hook function to exec an external program on neighbour receive
		switch(neighBour->addressFamily)
		{
			case AF_INET:
				// mask SIGALRM 
				mask();
				
				hookRx4(neighBour);
				
				// unmask SIGALRM 
				unMask();
			break;
			
			case AF_INET6:
				// mask SIGALRM 
				mask();
				
				hookRx6(neighBour);
				
				// unmask SIGALRM 
				unMask();
			break;
		}
		
		// Log new neighbour
		logWrite(neighBour);
	}
}

// send request with NLM_F_DUMP flag to netlink socket
static void netLinkReSync(void)
{
	int ret = 0;
	char loop = FALSE;

	do
	{
		loop = FALSE;
		ret = sendmsg(fd, &sndMsg, 0);
		if((ret < 0) && (errno == EINTR))
			loop = TRUE;
		else if((ret < 0) && (errno == EINTR))
		{
			logError("Secvmsg error\0");
			exit(1);
		}
	} 
	while (loop);
}

static void nlDebug(struct nlmsghdr *nlMsg)
{
	printf("Lunghezza pacchetto: 0x%x\n", nlMsg->nlmsg_len);
	if(nlMsg->nlmsg_type == RTM_NEWNEIGH)
		printf("Tipo Pacchetto: RTM_NEWNEIGH\n");
	else if(nlMsg->nlmsg_type == RTM_DELNEIGH)
		printf("Tipo Pacchetto: RTM_DELNEIGH\n");
	else if(nlMsg->nlmsg_type == RTM_GETNEIGH)
		printf("Tipo Pacchetto: RTM_GETNEIGH\n");

	printf("Flags Pacchetto: 0x%x\n", nlMsg->nlmsg_flags);
	printf("SeqN Pacchetto: 0x%x\n", nlMsg->nlmsg_seq);
	printf("Pid src Pacchetto: 0x%x\n", nlMsg->nlmsg_pid);

}

static void mainLoop(void)
{
	int ret;

	while(1)
	{
		ret = recvmsg(fd, &rcvMsg, 0);

		if((ret < 0) && (errno == ENOBUFS))
		{
			// Send a NLM_F_DUMP request to resync data flow
			netLinkReSync();
		}
		else if((ret < 0) && (errno != EINTR))
		{
			logError("Recvmsg error\0");
			exit(1);
		}
		else if(ret >= 0)
		{
			// check packet integrity
			if(packetTest(rcvNlMsg, ret))
			{
				// TODO PUNTO DI CONTROLLO PACCHETTI NETLINK
				nlDebug(rcvNlMsg);

				struct neighBourBlock *neighBour = parseNlPacket(rcvNlMsg);
				
				if(neighBour != NULL)
				{
					if(getMode() == DEBUG) {
						debugPrint(neighBour);
						free(neighBour);
					}
					else
					{
						if((filtersActived()) && (filter(neighBour) == FALSE))
						{
							free(neighBour);
						}						
						else
						{
							// test if packet is present in hash table and log if necessary
							pktSave(neighBour);
						}	
					}
				}			
			}
		}
	}
}

static void defSigHandler(int sig)
{
	switch (getMode())
	{
		case DEBUG:
		case STDOUT:
		{
			printf("Caught signal %d, close all socket and exiting.\n", sig);
			cleanExit(); 
		}
		break;

		case FILEOUT:	
		{
			fprintf(getFileLogStream(), "Caught signal %d, close all socket and exiting.\n", sig);
			cleanExit(); 
		}			
		break;

		case SYSLOG:
		{	
			char log[PRINTOUTBUF];

			snprintf(log, sizeof(log), "Caught signal %d, close all socket and exiting.\n", sig);
			syslog(LOG_ERR, log);
			cleanExit();
		}		
		break;
	}
}

static void setSignalHandlers()
{
	// all signals - SIGALRM
	struct sigaction actionIgnore, actionDefault;
	int i, signals[] = {SIGHUP, SIGINT, SIGPIPE, SIGTERM, SIGUSR1, SIGUSR2, SIGPROF, SIGVTALRM};

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
				logError("sigaction\0");
				exit(1);
			}
		}
		else
		{
			if(sigaction(signals[i], &actionIgnore, NULL) < 0)
			{
				logError("sigaction\0");
				exit(1);
			}
		}
	}

	// intervalTimerStart to set sigalrm handling
	intervalTimerStart();
}

inline void hookRx4(struct neighBourBlock *neighBour)
{
	if(execRX4Setted())
		execCmd(neighBour, RX, AF_INET);
}

inline void hookRx6(struct neighBourBlock *neighBour)
{
	if(execRX6Setted())
		execCmd(neighBour, RX, AF_INET6);
}

inline void hookDel4(struct neighBourBlock *neighBour)
{
	if(execDel4Setted())
		execCmd(neighBour, DEL, AF_INET);
}

inline void hookDel6(struct neighBourBlock *neighBour)
{
	if(execDel6Setted())
		execCmd(neighBour, DEL, AF_INET6);
}

int main(int argc, char *argv[])
{
	// parse command line arguments
	parseCmdLine(argc, argv);

	// signal handlers
	setSignalHandlers();

	// mask SIGALRM 
	mask();
	
	// init all structures
	initStruct();
	
	// open socket and bind
	socketBind();
	
	// unmask SIGALRM for start interval timer
	if(getMode() != DEBUG)
		unMask();

	// main loop
	mainLoop();
	
	return 0;
}
