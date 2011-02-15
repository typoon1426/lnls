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

// DA SPOSTARE DA QUALCHE ALTRA PARTE 
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

static int filters = FALSE, addressFamily = 0;
static unsigned char filtersBitMap = 0x00;
static unsigned char intTable[INT_TABLE_SIZE]; 

static struct subNet4 *inet4SubnetList;
static struct subNet6 *inet6SubnetList;
static struct subNet4 *subNetMaskBit4[NETMASK4_BIT_SIZE]; // STRUTTURA PROVVISORIA DA ALLOCARE A RUNTIME
static struct subNet6 *subNetMaskBit6[NETMASK6_BIT_SIZE]; // IDEM SOPRA

// filtra i pacchetti secondo le direttive di filtro da riga di comando, interfacce e subnet
int filter(struct neighBourBlock *neighBour)
{
	int ret_val = TRUE;

	switch(filtersBitMap)
	{
		case 1:
			ret_val = verifyAF(neighBour->addressFamily);
		break;

		case 2:
			ret_val = verifyInt(neighBour->if_index);
		break;
		
		case 3:
			ret_val = verifySubnet(neighBour);
		break;

		case 4:
			ret_val = verifyS
		break;

		case 5:
		break;

		case 6;
		break;
	
		case 7:
		break;
	};

	return ret_val;
}

void filtersInit(void)
{
	filters = SET;
	
	// DA ALLOCARE QUI LE STRUTTURE CHE SERVONO SOLO SE SONO ATTIVATI I FILTRI
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
}

int filterAddSubnet(char *subNet)
{
	int retval = FALSE, ret, len;
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
			memcpy(new4->inetSubNet, ((struct sockaddr_in *) tmp->ai_addr)->sin_addr.s_addr, INETLEN);
		}		
		else if(tmp->ai_addrlen == sizeof(struct sockaddr_in6))
		{
			len = INET6LEN;
			new6 = malloc(sizeof(struct subNet6));
			memcpy(new6->inet6SubNet, ((struct sockaddr_in6 *) tmp->ai_addr)->sin6_addr.s6_addr, INET6LEN);
		}	
	}

	// creo l'indirizzo della sottorete dopo la conversione del token sottorete in int
	if(sscanf(subnetElements[1], "%u", subnetMaskBitNumber) < 0)
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
			new4->inetMask = nBit2Mask(subnetMaskBitNumber);
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
			new6->inet6Mask = nBit2Mask(subnetMaskBitNumber);
		}		
		break;
	}

	return TRUE;
}

void filterSubnetEnd(void)
{
	// v4
	int i;

	for(i=0;i<NETMASK_BIT_SIZE;i++)
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

	// FARE FREE DELLE STRUTTURE PROVVISORIE
}

int filtersActived(void)
{
	return filters;
}

