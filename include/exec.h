/*   Linux Neighbour logging system Version 0.2
 *   developed as part of VirtualSquare project
 *   
 *   Copyright 2012 Michele Cucchi <cucchi@cs.unibo.it>
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
 *		Exec exported functions module
 */
#ifndef __EXEC_H__
#define __EXEC_H__ 1

#include "nlSystem.h"
#include "timer.h"

#define	RX 0
#define DEL 1
#define AF_LEN 10
#define L3_LEN 44
#define L2_LEN 22
#define CRC16ASCII_LEN 14
#define IFACE_LEN 18
#define ASCII_TSTAMP_LEN 28

inline void setExecIP4RxCmd(char *ip4RxCommand, char **ip4RxArguments);
inline void setExecIP6RxCmd(char *ip6RxCommand, char **ip6RxArguments);
inline void setExecIP4DelCmd(char *ip4DelCommand, char **ip4DelArguments);
inline void setExecIP6DelCmd(char *ip6DelCommand, char **ip6DelArguments);
void execCmd(struct neighBourBlock *neighBour, unsigned char opCode, unsigned char AF);
#endif
