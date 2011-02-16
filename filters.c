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
 *		Filter module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nlSystem.h"

#define INT_TABLE_SIZE 256
#define NETMASK4_BIT_SIZE 33
#define NETMASK6_BIT_SIZE 129
#define FILTER_AF 1
#define FILTER_INT 2
#define FILTER_SUBNET 4

struct subNet4 {
	unsigned char inetSubnet[INETLEN];
	unsigned char inetMask[INETLEN];
	unsigned int nMaskBitNumber;

	struct subNet4 *next;
	struct subNet4 *prev;
};

struct subNet6 {
	unsigned char inet6Subnet[INET6LEN];
	unsigned char inet6Mask[INET6LEN];
	unsigned int nMaskBitNumber;

	struct subNet6 *next;
	struct subNet6 *prev;
};

static int filters = FALSE;
static unsigned char filtersBitMap = 0x00, addressFamily = 0;
static unsigned char *intTable = NULL; 

static struct subNet4 *inet4SubnetList = NULL;
static struct subNet6 *inet6SubnetList = NULL;
static struct subNet4 **subNetMaskBit4 = NULL;
static struct subNet6 **subNetMaskBit6 = NULL;

void filtersInit(void)
{
	filters = TRUE;
	
	// int structures
	if(intTable == NULL)
	{
		intTable = calloc(INT_TABLE_SIZE, sizeof(unsigned char));
		if(intTable == NULL)
		{
			perror("calloc intTable");
			exit(1);
		}
	}

	if((subNetMaskBit4 == NULL) && (subNetMaskBit6 == NULL))
	{
		// subnet structures
		subNetMaskBit4 = calloc(NETMASK4_BIT_SIZE, sizeof(struct subNet4 *));
		if(subNetMaskBit4 == NULL)
		{
			perror("calloc subNetMaskBit4");
			exit(1);
		}

		subNetMaskBit6 = calloc(NETMASK6_BIT_SIZE, sizeof(struct subNet6 *));
		if(subNetMaskBit4 == NULL)
		{
			perror("calloc subNetMaskBit4");
			exit(1);
		}
	}
}

static int verifyAF(unsigned char af)
{
	if(af == addressFamily)
		return TRUE;
	else
		return FALSE;
}

static int verifyInt(unsigned int if_index)
{
	printf("codice interfaccia arrivato %u\n", if_index);
	printf("interfaccia selezionata array %u\n", intTable[if_index]);
	
	if(intTable[if_index] == TRUE)
		return TRUE;
	else 
		return FALSE;
}

static int verifySubnet(struct neighBourBlock *neighBour)
{
	if(neighBour->addressFamily == AF_INET)
	{
		struct subNet4 *temp = inet4SubnetList;
		unsigned int hostAddress = *((unsigned int *) neighBour->inetAddr);
		unsigned int subMask = 0, subNetAddress = 0;

		if(temp != NULL)
		{
			while(1) {
				subMask = *((unsigned int *) temp->inetMask);
				subNetAddress = *((unsigned int *) temp->inetSubnet);

				if(((hostAddress) & (subMask)) == subNetAddress)
					return TRUE;
				else
				{
					if(temp->next != inet4SubnetList)
						temp = temp->next;
					else
						return FALSE;
				}
			}
		}
		else
			return FALSE;
	}
	else if(neighBour->addressFamily == AF_INET6)
	{
		struct subNet6 *temp = inet6SubnetList;
		unsigned int hostAddress[] = {(*((unsigned int *) neighBour->inet6Addr)), 
						(*(((unsigned int *) neighBour->inet6Addr)+1)), 
						(*(((unsigned int *) neighBour->inet6Addr)+2)), 
						(*(((unsigned int *) neighBour->inet6Addr)+3))};
		unsigned int subMask[4], subNetAddress[4];

		if(temp != NULL)
		{
			while(1) {
				subMask[0] = *((unsigned int *) temp->inet6Mask);
				subMask[1] = *(((unsigned int *) temp->inet6Mask)+1);
				subMask[2] = *(((unsigned int *) temp->inet6Mask)+2);
				subMask[3] = *(((unsigned int *) temp->inet6Mask)+3);
				subNetAddress[0] = *((unsigned int *) temp->inet6Subnet);
				subNetAddress[1] = *(((unsigned int *) temp->inet6Subnet)+1);
				subNetAddress[2] = *(((unsigned int *) temp->inet6Subnet)+2);
				subNetAddress[3] = *(((unsigned int *) temp->inet6Subnet)+3);

				if( (((hostAddress[0]) & (subMask[0])) == subNetAddress[0]) &&
					(((hostAddress[1]) & (subMask[1])) == subNetAddress[1]) &&
					(((hostAddress[2]) & (subMask[2])) == subNetAddress[2]) &&
					(((hostAddress[3]) & (subMask[3])) == subNetAddress[3]))
					return TRUE;
				else
				{
					if(temp->next != inet6SubnetList)
						temp = temp->next;
					else
						return FALSE;
				}
			}
		}
		else
			return FALSE;
	}

	// NON DOVREBBE MAI RAGGIUNGERE QUESTO PUNTO, RETURN PER EVITARE WARNING
	return FALSE;
}

// filtra i pacchetti secondo le direttive di filtro da riga di comando, interfacce e subnet
int filter(struct neighBourBlock *neighBour)
{
	int ret_val = TRUE;

	switch(filtersBitMap)
	{
		case 1:
		{
			ret_val = verifyAF(neighBour->addressFamily);
			// DEBUG
			printf("address family %u\n", neighBour->addressFamily);
			printf("address family memorizzato %u\n", addressFamily);
		}		
		break;

		case 2:
			ret_val = verifyInt(neighBour->if_index);
		break;
		
		case 3:
			ret_val = (verifyAF(neighBour->addressFamily) && verifyInt(neighBour->if_index));
		break;

		case 4:
			ret_val = verifySubnet(neighBour);
		break;

		case 5:
			ret_val = (verifyAF(neighBour->addressFamily) && verifySubnet(neighBour));
		break;

		case 6:
			ret_val = (verifyInt(neighBour->if_index) && verifySubnet(neighBour));
		break;
	
		case 7:
			ret_val = (verifyAF(neighBour->addressFamily) && verifyInt(neighBour->if_index) && verifySubnet(neighBour));
		break;
	};

	return ret_val;
}

void filterSetAF(int af)
{
	if((filtersBitMap & FILTER_AF) == 0)
		filtersBitMap |= FILTER_AF; 

	addressFamily = af; 
}

void filterAddInterface(unsigned int int_index)
{
	if((filtersBitMap & FILTER_INT) == 0)
		filtersBitMap |= FILTER_INT; 

	intTable[int_index] = TRUE;
	printf("codice int prima %u\n", int_index);
	printf("interfaccia settata array %u\n", intTable[int_index]);
}

// XXX TODO
void nBit2Mask(unsigned char *mask, unsigned int nBit, unsigned int len)
{
	unsigned int nByteSet = nBit/8;
	unsigned int nRemainedBitSet = nBit%8;
	unsigned int i = 0;

	// COSA MOLTO SPORCA E BECERA	
	if(nByteSet > len)
		nByteSet = len;

	if(nByteSet > 0)
	{
		for(i=0;i<nByteSet;i++)
			*(mask + i) |= 0xFF;

		if(nRemainedBitSet != 0)
		{
			unsigned int j = i;
			unsigned char bitMask = 0x01;

			for(i=0;i<nRemainedBitSet;i++)
				*(mask + j) |= (bitMask << i);
		}
	}
	else
	{
		if(nRemainedBitSet != 0)
		{
			unsigned char bitMask = 0x01;

			for(i=0;i<nRemainedBitSet;i++)
				*(mask + i) |= (bitMask << i);
		}
	}
}

int filterAddSubnet(char *subNet)
{
	int ret, len;
	unsigned int counter = 0, subnetMaskBitNumber;
	char *token = NULL;
	char *subnetElements[2];
	struct addrinfo *info;
	struct addrinfo *tmp;
	struct subNet4 *new4 = NULL;
	struct subNet6 *new6 = NULL;

	if((filtersBitMap & FILTER_SUBNET) == 0)
		filtersBitMap |= FILTER_SUBNET;
 
	do
	{
		if(token != NULL)
			subNet = NULL;

		token = strtok(subNet, "/");
		
		if(token != NULL)
		{
			if(counter < 2)
			{
				subnetElements[counter] = token;
				counter++;
			}
			else
			{
				// RITORNO FALSO PERCHÈ HO PIÙ DI DUE TOKEN SUBNET MALFORMATA
				return FALSE;
			}
		}
	}
	while (token != NULL);	

	// converto l'indirizzo da ascii a network byte order
	ret = getaddrinfo(subnetElements[0], NULL, NULL, &info);
	if (ret != 0)
  	{   
		// CAMBIARE QUI INSERIRE UNA MIGLIORE GESTIONE ERRORI DI QUESTA FUNZIONE
     		fprintf(stderr, "error in getaddrinfo: %s\n", gai_strerror(ret));
     		return FALSE; 
  	}   
	
	for(tmp = info; tmp != NULL; tmp = tmp->ai_next)
	{
		if(tmp->ai_addrlen == sizeof(struct sockaddr_in))
		{
			len = INETLEN;
			new4 = malloc(sizeof(struct subNet4));
			memcpy(new4->inetSubnet, &(((struct sockaddr_in *) tmp->ai_addr)->sin_addr.s_addr), INETLEN);
		}		
		else if(tmp->ai_addrlen == sizeof(struct sockaddr_in6))
		{
			len = INET6LEN;
			new6 = malloc(sizeof(struct subNet6));
			memcpy(new6->inet6Subnet, &(((struct sockaddr_in6 *) tmp->ai_addr)->sin6_addr.s6_addr), INET6LEN);
			
			// DEBUG
			printf("%02hhx", *((unsigned char *) new6->inet6Subnet));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+1));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+2));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+3));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+4));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+5));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+6));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+7));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+8));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+9));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+10));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+11));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+12));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+13));
			printf("%02hhx", *(((unsigned char *) new6->inet6Subnet)+14));
			printf("%02hhx\n", *(((unsigned char *) new6->inet6Subnet)+15));
		}	
	}

	// creo l'indirizzo della sottorete dopo la conversione del token sottorete in int
	if(sscanf(subnetElements[1], "%u", &subnetMaskBitNumber) < 0)
	{
		// CAMBIARE QUI INSERIRE UNA MIGLIORE GESTIONE ERRORI DI QUESTA FUNZIONE, SE SI PUÒ
		perror("sscanf");
		return FALSE;
	}

	switch(len)
	{
		case INETLEN:
		{
			if(subNetMaskBit4[subnetMaskBitNumber] != NULL)
			{
				new4->next = subNetMaskBit4[subnetMaskBitNumber];
				new4->prev = subNetMaskBit4[subnetMaskBitNumber]->prev;
				subNetMaskBit4[subnetMaskBitNumber]->prev->next = new4;
				subNetMaskBit4[subnetMaskBitNumber]->prev = new4;
				subNetMaskBit4[subnetMaskBitNumber] = new4;
			}
			else
			{
				subNetMaskBit4[subnetMaskBitNumber] = new4;
				new4->next = new4;
				new4->prev = new4;
			}

			new4->nMaskBitNumber = subnetMaskBitNumber;
			nBit2Mask(new4->inetMask, subnetMaskBitNumber, INETLEN);
		}
		break;

		case INET6LEN:
		{
			if(subNetMaskBit6[subnetMaskBitNumber] != NULL)
			{
				new6->next = subNetMaskBit6[subnetMaskBitNumber];
				new6->prev = subNetMaskBit6[subnetMaskBitNumber]->prev;
				subNetMaskBit6[subnetMaskBitNumber]->prev->next = new6;
				subNetMaskBit6[subnetMaskBitNumber]->prev = new6;
				subNetMaskBit6[subnetMaskBitNumber] = new6;
			}
			else
			{
				subNetMaskBit6[subnetMaskBitNumber] = new6;
				new6->next = new6;
				new6->prev = new6;
			}
			
			new6->nMaskBitNumber = subnetMaskBitNumber;
			nBit2Mask(new6->inet6Mask, subnetMaskBitNumber, INET6LEN);
		}		
		break;
	}

	return TRUE;
}

void filterSubnetEnd(void)
{
	// v4
	int i;

	for(i=0;i<NETMASK4_BIT_SIZE;i++)
	{
		if(subNetMaskBit4[i] != NULL)
		{
			if(inet4SubnetList != NULL)
			{
				struct subNet4 *tmp = subNetMaskBit4[i]->prev;

				subNetMaskBit4[i]->prev = inet4SubnetList->prev;
				inet4SubnetList->prev->next = subNetMaskBit4[i];
				inet4SubnetList->prev = tmp;
				tmp->next = inet4SubnetList;

			}
			else
			{
				inet4SubnetList = subNetMaskBit4[i];
			}
		}
	}
	// v6
	for(i=0;i<NETMASK6_BIT_SIZE;i++)
	{
		if(subNetMaskBit6[i] != NULL)
		{
			if(inet6SubnetList != NULL)
			{
				struct subNet6 *tmp = subNetMaskBit6[i]->prev;

				subNetMaskBit6[i]->prev = inet6SubnetList->prev;
				inet6SubnetList->prev->next = subNetMaskBit6[i];
				inet6SubnetList->prev = tmp;
				tmp->next = inet6SubnetList;

			}
			else
			{
				inet6SubnetList = subNetMaskBit6[i];
			}
		}
	}

	// free strutture provvisorie
	free(subNetMaskBit4);
	free(subNetMaskBit6);
}

void filterInterfaceEnd(void)
{
	// free strutture provvisorie
	free(intTable);
}

int filtersActived(void)
{
	return filters;
}

