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
 *		Command Line parser export functions module
 */
#ifndef __CMDLINEPARSE_H__
#define __CMDLINEPARSE_H__ 1

#define MAXSTRCMD_LEN 100
#define DAEMONIZE_WEIGHT 1
#define PIDFILE_WEIGHT 1
#define GROUPBYINT_WEIGHT 2
#define SYSLOG_WEIGHT 11
#define FILELOG_WEIGHT 11
#define STDOUT_WEIGHT 14
#define HELP_WEIGHT 15
#define DEBUG_WEIGHT 14
#define MINRANGE 10
#define MAXRANGE 15

void parseCmdLine(int argc, char *argv[]);
inline unsigned char execRX4Setted(void);
inline unsigned char execRX6Setted(void);
inline unsigned char execDel4Setted(void);
inline unsigned char execDel6Setted(void);

#endif
