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
 *		Logging functions module
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
#include <netdb.h>
#include <time.h>
#include <syslog.h>

#include "nlSystem.h"

static const char intString[] = "Interface :";
static const char inetString[] = "IPv4 Address :";
static const char inet6String[] = "IPv6 Address :";
static const char etherAddrString[] = "Local Link Address :";
static const char timeFormat[] = "%d/%m/%y - %H:%M:%S";

static FILE *logFile = NULL; 
static unsigned char mode = 0;

static void etherAddr2Str(const unsigned char *ether, char *buf, int srclen, int dstlen)
{
	int i, w = 0;
	for(i=0; i<srclen; i++)
	{
		w += snprintf(buf+w, dstlen, "%02hhx", *((unsigned char *) ether+i));
	}
}

// STAMPARE EVENTUALMENTE ANCHE INTERFACE NAME O INTERFACE INDEX
static void neigh2Ascii(struct neighBourBlock *neigh, char *printOutBuf, int outLen)
{
	char netAddr[ASCII_BUF];
	char linkAddr[ASCII_BUF];
	int w = 0;
	
	// add interface name to string
	w += snprintf(printOutBuf+w, outLen, "%s %s, ", intString, neigh->if_name);
	
	// add ip to string
	if(neigh->addressFamily == AF_INET)
	{
		struct sockaddr_in inet;
		memset(&inet, 0, sizeof(inet));

		inet.sin_family = AF_INET;
		//inet.sin_addr.s_addr = *((unsigned int *) neigh->inetAddr); // FORSE CI VUOLE HOST TO INET 
		memcpy(&(inet.sin_addr.s_addr), neigh->inetAddr, INETLEN);

		if(getnameinfo((struct sockaddr *) &inet, sizeof(inet), netAddr, sizeof(netAddr), 0, 0, NI_NUMERICHOST) < 0)
		{
			perror("neigh2Ascii getnameinfo ipv4");
			exit(1);
		}

		w += snprintf(printOutBuf+w, outLen, "%s %s, ", inetString, netAddr);
	}
	else if(neigh->addressFamily == AF_INET6)
	{
		struct sockaddr_in6 inet6;
		memset(&inet6, 0, sizeof(inet6));

		inet6.sin6_family = AF_INET6;
		//inet6.sin6_addr.s6_addr = neigh->inet6Addr;		
		memcpy(&(inet6.sin6_addr.s6_addr), neigh->inetAddr, INET6LEN);

		if(getnameinfo((struct sockaddr *) &inet6, sizeof(inet6), netAddr, sizeof(netAddr), 0, 0, NI_NUMERICHOST) < 0)
		{
			perror("neigh2Ascii getnameinfo ipv6");
			exit(1);
		}

		w += snprintf(printOutBuf+w, outLen, "%s %s, ", inet6String, netAddr);
	}
	
	// add mac address to string
	etherAddr2Str(neigh->etherAddr, linkAddr, ETH_ALEN, sizeof(linkAddr));
	w += snprintf(printOutBuf+w, outLen, "%s %s", etherAddrString, linkAddr);	
}

static void timeStamp(time_t *time, char *buf, int len)
{
	strftime(buf, len, timeFormat, localtime(time));
}

static void str2FdPrint(struct neighBourBlock *neigh, FILE *fd)
{
	char str[PRINTOUTBUF];
	char timeStampBuf[ASCII_BUF];

	timeStamp(&(neigh->last_seen), timeStampBuf, sizeof(timeStampBuf));

	neigh2Ascii(neigh, str, sizeof(str));

	fprintf(fd, "%s %s\n", str, timeStampBuf);
}

static void sysLogPrint(struct neighBourBlock *neigh)
{
	char log[PRINTOUTBUF];

	neigh2Ascii(neigh, log, sizeof(log));
	syslog(LOG_INFO, log);
}

void closeLogFile(void)
{
	if(fclose(logFile) == EOF)
	{
		perror("fclose:");
		exit(1);
	}
}

void setMode(unsigned int newMode)
{
	mode = newMode;
}

unsigned char getMode(void)
{
	return mode;
}

void debugPrint(struct neighBourBlock *neigh)
{
	str2FdPrint(neigh, stdout);
}

void setFileLogStream(FILE *logStream)
{
	logFile = logStream;
}

FILE *getFileLogStream(void)
{
	return logFile;
}

void logWrite(struct neighBourBlock *neigh)
{
	switch (mode)
	{
		case STDOUT:
			str2FdPrint(neigh, stdout);
		break;

		case FILEOUT:	
			str2FdPrint(neigh, logFile);
		break;

		case SYSLOG:	
			sysLogPrint(neigh);
		break;
	}
}
