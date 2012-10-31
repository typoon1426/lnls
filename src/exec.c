/*   Linux Neighbour logging system Version 0.2
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
 *		Exec module
 */

#include "exec.h"

static void execRx(struct neighBourBlock *neighBour, unsigned char AF)
{
	
}

static void execDel(struct neighBourBlock *neighBour, unsigned char AF)
{

}

inline void hookRcvPair4(struct neighBourBlock *neighBour)
{
	execRx(neighBour, AF_INET);
}

inline void hookRcvPair6(struct neighBourBlock *neighBour)
{
	execRx(neighBour, AF_INET6);
}

inline void hookRemPair4(struct neighBourBlock *neighBour)
{
	execDel(neighBour, AF_INET);
}

inline void hookRemPair6(struct neighBourBlock *neighBour)
{
	execDel(neighBour, AF_INET6);
}
