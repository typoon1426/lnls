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
 *		NetLink to Neighbour functions module
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
#include <time.h>

#include "nlSystem.h"

// always inline functions prototype
int packetTest(struct nlmsghdr *nlMsgHdr, int len) __attribute__((always_inline));
static struct neighBourBlock *packetConverter(struct nlmsghdr *nlMsgHdr) __attribute__((always_inline));

static struct neighBourBlock *packetConverter(struct nlmsghdr *nlMsgHdr)
{
	struct neighBourBlock *newNeigh = NULL;

	struct ndmsg 	*neighMsg =  NLMSG_DATA(nlMsgHdr);
	struct rtattr 	*netAddr = (struct rtattr *) (NLMSG_DATA(nlMsgHdr) + sizeof(struct ndmsg)), 
			*linkAddr = RTA_NEXT(netAddr, nlMsgHdr->nlmsg_len);

	// verify rtattr integrity 
	if((!RTA_OK(netAddr, nlMsgHdr->nlmsg_len)) || (!RTA_OK(linkAddr, nlMsgHdr->nlmsg_len)))
		return newNeigh;
	
	// verify that RT-struct are of type 1 and 2 (network and link local address)
	if((netAddr->rta_type != NDA_DST) || (linkAddr->rta_type != NDA_LLADDR))
		return newNeigh;

	// allocate a struct neighBlock to return
	newNeigh = malloc(sizeof(struct neighBourBlock));
	if(newNeigh == NULL)
	{
		perror("malloc");
		exit(1);
	}

	// set net level address family
	newNeigh->addressFamily = neighMsg->ndm_family;
	
	// set if_index and name
	newNeigh->if_index = neighMsg->ndm_ifindex;

	if(if_indextoname(neighMsg->ndm_ifindex, newNeigh->if_name) == NULL)
	{
		perror("if_index-toname");
		exit(1);
	}

	// set net level address
	if(neighMsg->ndm_family == AF_INET)
		memcpy(newNeigh->inetAddr, (unsigned char *) RTA_DATA(netAddr), INETLEN);
	else if(neighMsg->ndm_family == AF_INET6)
		memcpy(newNeigh->inet6Addr, (unsigned char *) RTA_DATA(netAddr), INET6LEN);
	
	// set link local addr
	memcpy(newNeigh->etherAddr, (unsigned char *) RTA_DATA(linkAddr), ETH_ALEN);

	// set last seen time_t struct 
	newNeigh->last_seen = time(NULL);

	return newNeigh;
}

struct neighBourBlock *parseNlPacket(struct nlmsghdr *nlMsgHdr)
{
	// verify if received packet is from kernel, with pid 0
	if(nlMsgHdr->nlmsg_pid == 0)
	{
		// message sent from kernel
		
		// select only RTM_NEWNEIGH packet type
		if(nlMsgHdr->nlmsg_type == RTM_NEWNEIGH)
		{
			return packetConverter(nlMsgHdr);
		}
	}

	return NULL;
}

int packetTest(struct nlmsghdr *nlMsgHdr, int len)
{
	return NLMSG_OK(nlMsgHdr, len);
}
