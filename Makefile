#   Linux Neighbour logging system Version 0.1
#   developed as part of VirtualSquare project
#   
#   Copyright 2010 Michele Cucchi <cucchi@cs.unibo.it>
#   
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License, version 2, as
#   published by the Free Software Foundation.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA.
#
#	Makefile

# CC compiler
CC = gcc  

# LD Linker 
LD = gcc

# gcc compile flags
GCCFLAGS = -D__EXPERIMENTAL__ 

# gcc linker flags
LDFLAGS = 

# Compile all and generate executable
all: 
	@echo "Compile link and generate executable binary"
	$(CC) $(GCCFLAGS) -Wall -c cmdLineParse.c hashHandlers.c logging.c nlSystem.c nl2Neigh.c timer.c filters.c
	$(LD) $(LDFLAGS) cmdLineParse.o hashHandlers.o logging.o nlSystem.o nl2Neigh.o timer.o filters.o -o lnls
	
# Compile only
compile:
	@echo "Compile only"
	$(CC) $(GCCFLAGS) -Wall -c cmdLineParse.c hashHandlers.c logging.c nlSystem.c nl2Neigh.c timer.c filters.c

# Link and generate executable
link:
	@echo "Link and generate executable"
	$(LD) $(LDFLAGS) cmdLineParse.o hashHandlers.o logging.o nlSystem.o nl2Neigh.o timer.o filters.o -o lnls

# clean of all object and executable
clean:
	@echo "Rimuovo i file object/eseguibili"
	rm -f *.o nlogger  

