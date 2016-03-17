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
 *		Filter module exported functions
 */

#ifndef __FILTERS_H__
#define __FILTERS_H__ 1

#include "nlSystem.h"

void filtersInit(void);
int filter(struct neighBourBlock *neighBour);
void filterSetAF(int af);
void filterAddInterface(unsigned int int_index);
int filterAddSubnet(char *subNet);
void filterSubnetEnd(void);
int filtersActived(void);
#endif
