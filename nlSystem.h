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
 *		Global definitions
 */
#ifndef __NEIGHLOG__
#define __NEIGHLOG__ 1

#include <net/if.h>
#include <netdb.h>

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(*(x)))

#define BUFLENGTH 1024
#define INETLEN 4
#define	INET6LEN 16
#define ETH_ALEN 6
#define ASCII_BUF 50
#define PRINTOUTBUF 100
#define STDOUTPUT 1
#define LOG 1
#define NOLOG 0

#define	DEBUG 5
#define STDOUT 1
#define FILEOUT 2
#define SYSLOG 3

// GC == GARBAGE COLLECTOR
#define IP_GC_INTERVAL 10 // INTERVAL TIMER FOR SIGALARM
#define IP_GC_EXPIRE 360 // EXPIRE TIME HASH ELEMENT

struct neighBourBlock {
	unsigned char addressFamily;

	// network level address can be inet or inet6 address
	union {
		unsigned char inetAddr[INETLEN];
		unsigned char inet6Addr[INET6LEN];
	};
	// local link address, only ethernet is supported
	unsigned char etherAddr[ETH_ALEN];

	// interface index and name
	unsigned int if_index;
	char if_name[IF_NAMESIZE];
	
	// timestamp 
	time_t last_seen; 

	// ptr for hash table collision list
	struct neighBourBlock *t_next;
	struct neighBourBlock *t_prev;

} __attribute__((packed));

#endif
