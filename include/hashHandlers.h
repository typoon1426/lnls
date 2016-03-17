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
 *		Hash Table Module
 */

#ifndef __HASHHANDLERS_H__
#define __HASHHANDLERS_H__ 1

#define MAXTIMEOUT 3600

int neighHashFindAdd(struct neighBourBlock *neighBlock);
void ip_hash_gc(void);
void setIpGcExpire(int expireTime);
#endif
