/*   This is part of VDE Virtual Distributed Internet
 *
 *   iplog: ip logging plugin for vde_switch
 *   
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

/*   Linux Neighbour logging system
 *   developed within VirtualSquare project
 *  
 */
/* 
 *		Hash Table Module
 */

int neighHashFindAdd(struct neighBourBlock *neighBlock);
void ip_hash_gc(void);
