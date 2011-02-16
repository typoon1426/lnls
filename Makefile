# CC compilatore 
CC = gcc  

# LD Linker 
LD = gcc

# gcc flags
GCCFLAGS = -D__EXPERIMENTAL__

# Compila tutto e genera l'eseguibile
all: 
	@echo "Compile link and generate executable binary"
	$(CC) $(GCCFLAGS) -Wall -c cmdLineParse.c hashHandlers.c logging.c nlSystem.c nl2Neigh.c timer.c filters.c
	$(LD) cmdLineParse.o hashHandlers.o logging.o nlSystem.o nl2Neigh.o timer.o filters.o -o nLogSystem
	
# Compile only
compile:
	@echo "Compile only"
	$(CC) $(GCCFLAGS) -Wall -c cmdLineParse.c hashHandlers.c logging.c nlSystem.c nl2Neigh.c timer.c filters.c

# Link and generate executable
link:
	@echo "Link and generate executable"
	$(LD) cmdLineParse.o hashHandlers.o logging.o nlSystem.o nl2Neigh.o timer.o filters.o -o nLogSystem

# Pulisce tutti i file oggetto e/o eseguibili
clean:
	@echo "Rimuovo i file object/eseguibili"
	rm -f *.o nLogSystem  

