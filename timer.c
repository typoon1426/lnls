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
 *		Timer functions module
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "nlSystem.h"
#include "hashHandlers.h"

static sigset_t oldState, maskedState;

void unMask(void)
{
	
	if (sigprocmask(SIG_SETMASK,&oldState,NULL) < 0)
	{
		perror("sigprocmask:");
		exit(1);
	}
}

void mask(void)
{
	if (sigprocmask(SIG_BLOCK,&maskedState,&oldState) < 0)
	{
		perror("sigprocmask:");
		exit(1);
	}
}

void intervalTimerStart(void)
{
	struct sigaction actionAlarm;
	struct itimerval intervalTimer;

	// set sigset struct for future signal masking
	sigemptyset(&maskedState);
	sigaddset(&maskedState,SIGALRM);

	// set handler for siglarm the hash garbage collector
	actionAlarm.sa_handler = (void *) ip_hash_gc;
	actionAlarm.sa_flags = SA_RESTART;
	
	// set sign handler
	if(sigaction(SIGALRM, &actionAlarm, NULL) < 0)
	{
		perror("sigaction:");
		exit(1);
	}

	intervalTimer.it_value.tv_sec = IP_GC_INTERVAL;
	intervalTimer.it_value.tv_usec = 0 ;
	intervalTimer.it_interval.tv_sec = IP_GC_INTERVAL;
	intervalTimer.it_interval.tv_usec = 0 ;

	// set interval timer
	if(setitimer(ITIMER_REAL, &intervalTimer, NULL) < 0)
	{
		perror("setitimer:");
		exit(1);	
	}
}
