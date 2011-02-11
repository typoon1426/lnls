/*   Linux Neighbour logging system
 *   developed within the VirtualSquare project
 *  
 *   Copyright 2010 Michele Cucchi <cucchi@cs.unibo.it>
 *
 *   This module is mostly based on Virtual Distributed Ethernet vde_switch, iplog plugin hash table
 *   Copyright 2010 Renzo Davoli University of Bologna - Italy
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
 *		Hash Table Module
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "neighLog.h"

/* hash table of recently seen ip addresses, collision lists are double linked */
#define IP_HASH_SIZE 1024

struct neighBourBlock *hashTable[IP_HASH_SIZE];
static int ip_gc_expire = IP_GC_EXPIRE;

/* hash function */
static inline int ip_hash(int len,unsigned char *addr)
{
	if (len == 4)
		return((addr[0]+2*addr[1]+3*addr[2]+5*addr[3]) % IP_HASH_SIZE);
	else
		return((addr[0]+2*addr[1]+3*addr[2]+5*addr[3]+
				7*addr[4]+11*addr[5]+13*addr[6]+17*addr[7]+
				19*addr[8]+23*addr[9]+29*addr[10]+31*addr[11]+
				37*addr[12]+41*addr[13]+43*addr[14]+47*addr[15]) % IP_HASH_SIZE);
}

int neighHashFindAdd(struct neighBourBlock *neighBlock) 
{
	int key, len;
	struct neighBourBlock *listEntry, *preEntry;

	if(neighBlock->addressFamily == AF_INET) {
		key = ip_hash(INETLEN, neighBlock->inetAddr);
		len = INETLEN;
	}
	else if(neighBlock->addressFamily == AF_INET6) {
		key = ip_hash(INET6LEN, neighBlock->inet6Addr);
		len = INET6LEN;
	}

	if(hashTable[key] == NULL) {
		hashTable[key] = neighBlock;
		neighBlock->t_next = neighBlock->t_prev = NULL;

		// log packet
		return LOG;
	}
	else {
		for(listEntry=hashTable[key]; listEntry && memcmp(listEntry->inetAddr, neighBlock->inetAddr, INETLEN) 
			&& memcmp(listEntry->etherAddr, neighBlock->etherAddr, ETH_ALEN); (preEntry = listEntry) && (listEntry = listEntry->t_next));

		if(listEntry == NULL) {
			neighBlock->t_next = NULL;
			neighBlock->t_prev = preEntry;
			preEntry->t_next = neighBlock;
			
			// log packet
			return LOG;
		}
			listEntry->last_seen = time(NULL);
	}

	return NOLOG;
}

/* pass through the hash table and execute function f for each element */
static void ip_for_all_hash(void (*f)(struct neighBourBlock *, void *), void *arg)
{
	int i;
	struct neighBourBlock *e = NULL;

	// hash elements
	for(i = 0; i < IP_HASH_SIZE; i++){

		// collision queue elements
		if(hashTable[i] != NULL)
		{
			for(e = hashTable[i]; e ; e = e->t_next){
				
				(*f)(e, arg);
			}
		}
	}
}

/* delete a hash table entry */
static inline void delete_hash_entry(struct neighBourBlock *old) 
{
	int key;

	if(old->addressFamily == AF_INET) {
		key = ip_hash(INETLEN, old->inetAddr);
	}
	else if(old->addressFamily == AF_INET6) {
		key = ip_hash(INET6LEN, old->inet6Addr);
	}

	if((old->t_next == NULL) && (old->t_prev == NULL)) {
		hashTable[key] = NULL;
	} 
	else if(old->t_prev == NULL) {
		old->t_next->t_prev = old->t_prev;

		hashTable[key] = old->t_next;
	}
	else if(old->t_next == NULL) {
		old->t_prev->t_next = old->t_next;
	}
	else {
		old->t_prev->t_next = old->t_next;
		old->t_next->t_prev = old->t_prev;
	}

	free(old);
}

/* clean from the hash table entries older than IP_GC_EXPIRE seconds, given that
 * 'now' points to a time_t structure describing the current time */
static void ip_gc(struct neighBourBlock *e, void *now)
{
	if((*((time_t *) now) - e->last_seen) >= ip_gc_expire)
		delete_hash_entry(e);
}

/* clean old entries in the hash table 'h', and prepare the timer to be called * again between GC_INTERVAL seconds */
void ip_hash_gc(void)
{
	time_t t = time(NULL);

	ip_for_all_hash(ip_gc, &t);
}
