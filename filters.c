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

struct subNet4 {
	unsigned char inetSubnet[INETLEN];
	unsigned char inetMask[INETLEN];

	struct subNet4 *next;
	struct subNet4 *prev;
};

struct subNet6 {
	unsigned char inet6Subnet[INET6LEN];
	unsigned char inet6Mask[INET6LEN];

	struct subNet6 *next;
	struct subNet6 *prev;
};

static int filters = FALSE, addressFamily = 0;
static unsigned char intTable[INT_TABLE_SIZE];

static struct subNet4 *inet4SubnetList;
static struct subNet6 *inet6SubnetList;
static char *subNetMaskBit[129];

// filtra i pacchetti secondo le direttive di filtro da riga di comando, interfacce e subnet
int filter(struct neighBourBlock *neighBour)
{
	int ret_val = TRUE;

	return ret_val;
}

void filterActive(void)
{
	filters = TRUE;
}

void filterSetAF(int af)
{
	addressFamily = af; 
}

void filterAddInterface(unsigned int int_index)
{
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
	
	//PER CREARE OGNI CODA ORDINATA DALLA MASK PIÙ GENERICA ALLA MENO FARE UNA CODA PER OGNI MASK UGUALE POI FARE IL MERGE
 
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
			memcpy(inetAddr, ((struct sockaddr_in *) tmp->ai_addr)->sin_addr.s_addr, INETLEN);
		}		
		else if(tmp->ai_addrlen == sizeof(struct sockaddr_in6))
		{
			len = INET6LEN;
			memcpy(inet6Addr, ((struct sockaddr_in6 *) tmp->ai_addr)->sin6_addr.s6_addr, INET6LEN);
		}	
	}

	// creo l'indirizzo della sottorete dopo la conversione del token sottorete in int
	if(sscanf(subnetElements[1], "%u", subnetMaskBitNumber) < 0)
	{
		// CAMBIARE QUI INSERIRE UNA MIGLIORE GESTIONE ERRORI DI QUESTA FUNZIONE, SE SI PUÒ
		perror("sscanf");
		return FALSE;
	}

	

	return retval;
}

void filterSubnetEnd(void)
{

}

int filtersActived(void)
{
	return filters;
}

