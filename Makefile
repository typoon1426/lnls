# CC compilatore 
CC = gcc  

# LD Linker 
LD = gcc

# Compila tutto e genera l'eseguibile
all: 
	@echo "Compile link and generate executable binary"
	$(CC) -Wall -c cmdLineParse.c hashHandlers.c logging.c neighLog.c nl2Neigh.c timer.c
	$(LD) cmdLineParse.o hashHandlers.o logging.o neighLog.o nl2Neigh.o timer.o -o neigh_log
	
# Compile only
compile:
	@echo "Compile only"
	$(CC) -Wall -c cmdLineParse.c hashHandlers.c logging.c neighLog.c nl2Neigh.c timer.c

# Link and generate executable
link:
	@echo "Link and generate executable"
	$(LD) cmdLineParse.o hashHandlers.o logging.o neighLog.o nl2Neigh.o timer.o -o neigh_log

# Pulisce tutti i file oggetto e/o eseguibili
clean:
	@echo "Rimuovo i file object/eseguibili"
	rm -f *.o neigh_log  

