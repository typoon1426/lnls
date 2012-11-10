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
#include <unistd.h>

#include "nlSystem.h"

static const char intString[] = "Interface :";
static const char inetString[] = "IPv4 Address :";
static const char inet6String[] = "IPv6 Address :";
static const char etherAddrString[] = "Local Link Address :";
static const char timeFormat[] = "%d/%m/%y - %H:%M:%S";
static char lineBuf[1024];

static FILE *logFile = NULL; 
static unsigned char mode = 0;
static char pidFilePtr[512];

void etherAddr2Str(const unsigned char *ether, char *buf, int srclen, int dstlen)
{
	int i, w = 0;

	for(i=0; i<srclen; i++)
	{
		if(w == 0)
			w += snprintf(buf+w, dstlen, "%02hhx", *((unsigned char *) ether+i));
		else
			w += snprintf(buf+w, dstlen, ":%02hhx", *((unsigned char *) ether+i));
	}
}

void inet2Ascii(const unsigned char *inet, char *buf, int inetLen, int bufLen)
{
	struct sockaddr_in afinet;
	memset(&afinet, 0, sizeof(afinet));

	afinet.sin_family = AF_INET;
	memcpy(&(afinet.sin_addr.s_addr), inet, inetLen);

	if(getnameinfo((struct sockaddr *) &afinet, sizeof(afinet), buf, bufLen, 0, 0, NI_NUMERICHOST) < 0)
	{
		logError("inet2Ascii getnameinfo ipv4\0");
		exit(1);
	}
}

void inet62Ascii(const unsigned char *inet6, char *buf, int inet6Len, int bufLen)
{
	struct sockaddr_in6 afinet6;
	memset(&afinet6, 0, sizeof(afinet6));

	afinet6.sin6_family = AF_INET6;		
	memcpy(&(afinet6.sin6_addr.s6_addr), inet6, inet6Len);

	if(getnameinfo((struct sockaddr *) &afinet6, sizeof(afinet6), buf, bufLen, 0, 0, NI_NUMERICHOST) < 0)
	{
		logError("inet62Ascii getnameinfo ipv6\0");
		exit(1);
	}
}

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
		inet2Ascii(neigh->inetAddr, netAddr, INETLEN, ASCII_BUF);
		w += snprintf(printOutBuf+w, outLen, "%s %s, ", inetString, netAddr);
	}
	else if(neigh->addressFamily == AF_INET6)
	{
		inet62Ascii(neigh->inetAddr, netAddr, INET6LEN, ASCII_BUF);
		w += snprintf(printOutBuf+w, outLen, "%s %s, ", inet6String, netAddr);
	}
	
	// add mac address to string
	etherAddr2Str(neigh->etherAddr, linkAddr, ETH_ALEN, ASCII_BUF);
	w += snprintf(printOutBuf+w, outLen, "%s %s", etherAddrString, linkAddr);	
}

static inline void timeStamp(time_t *time, char *buf, int len)
{
	strftime(buf, len, timeFormat, localtime(time));
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

inline unsigned char getMode(void)
{
	return mode;
}

void debugPrint(struct neighBourBlock *neigh)
{
	char timeStampBuf[ASCII_BUF];
	char packetBuf[PRINTOUTBUF];
	
	memset(timeStampBuf, 0, ASCII_BUF);
	memset(packetBuf, 0, PRINTOUTBUF);

	timeStamp(&(neigh->last_seen), timeStampBuf, ASCII_BUF);
	neigh2Ascii(neigh, packetBuf, PRINTOUTBUF);

	fprintf(stdout, "[ %s ] %s\n", timeStampBuf, packetBuf);
}

void setFileLogStream(FILE *logStream)
{
	logFile = logStream;

	// set stream line buffered
	if(setvbuf(logFile, lineBuf, _IOLBF, LINEBUFLEN) != 0)
	{
		perror("setvbuf error:");
		logError("setvbuf error:\0");
		closeLogFile();
		exit(1);
	}
}

FILE *getFileLogStream(void)
{
	return logFile;
}

void logError(char *errorString)
{
	int ret = 0;
	char errorBuf[PRINTOUTBUF];
	memset(errorBuf, 0, PRINTOUTBUF);

	ret = sprintf(errorBuf, "%s:", errorString);

	strerror_r(errno, errorBuf+ret, PRINTOUTBUF-ret);
	
	logPrint(errorBuf, NULL, TRUE);
}

void logPrint(char *string, char *timeS, unsigned char error)
{
	if(timeS == NULL)
	{
		char timeStampBuf[ASCII_BUF];
		time_t tStamp = time(NULL);
	
		timeStamp(&tStamp, timeStampBuf, ASCII_BUF);

		timeS = timeStampBuf;
	}

	switch (mode)
	{
		case STDOUT:
			fprintf(stdout, "[ %s ] %s\n", timeS, string);
		break;

		case FILEOUT:	
			fprintf(logFile, "[ %s ] %s\n", timeS, string);
		break;

		case SYSLOG:	
			if(error)
				syslog(LOG_ERR, string);
			else
				syslog(LOG_INFO, string);
		break;
	}
}

void logWrite(struct neighBourBlock *neigh)
{
	char timeStampBuf[ASCII_BUF];
	char packetBuf[PRINTOUTBUF];
	
	memset(timeStampBuf, 0, ASCII_BUF);
	memset(packetBuf, 0, PRINTOUTBUF);

	timeStamp(&(neigh->last_seen), timeStampBuf, ASCII_BUF);
	neigh2Ascii(neigh, packetBuf, PRINTOUTBUF);
	

	logPrint(packetBuf, PRINTOUTBUF, timeStampBuf, ASCII_BUF, FALSE);
}

void saveFileName(char *pidFileName)
{
	size_t len = strnlen(pidFileName, 512);

	if(pidFileName[0] != '/')
	{
		if(getcwd(pidFilePtr, 512) == NULL)
		{
			perror("getcwd");
			exit(1);
		}
		
		strncat(pidFilePtr, "/", 1);
		strncat(pidFilePtr, pidFileName, len);
	}
	else
	{
		strncpy(pidFilePtr, pidFileName, len);
	}
}

char *getPidFileName(void)
{
	return pidFilePtr;
}
