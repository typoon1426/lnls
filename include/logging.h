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
 *		Logging module exported functions
 */

#ifndef __LOGGING_H__
#define __LOGGING_H__ 1
#include "nlSystem.h"
#include <stdio.h>

void debugPsrint(struct neighBourBlock *neigh);
void logWrite(struct neighBourBlock *neigh);
void logPrint(char *string, char *timeS, unsigned char error);
void logError(char *errorString);
void closeLogFile(void);
void setFileLogStream(FILE *logStream);
FILE *getFileLogStream(void);
void setMode(unsigned int newMode);
unsigned char getMode(void);
void debugPrint(struct neighBourBlock *neigh);
void saveFileName(char *pidFileName);
char *getPidFileName(void);
void etherAddr2Str(const unsigned char *ether, char *buf, int srclen, int dstlen);
void inet2Ascii(const unsigned char *inet, char *buf, int inetLen, int bufLen);
void inet62Ascii(const unsigned char *inet6, char *buf, int inet6Len, int bufLen);
#endif
