
OBJ = app
SRC = main.c linkqueue.c link.c mailbox.c mysqlite.c mqtt.c
CC = arm-linux-gnueabihf-gcc
INC = ../include
INC += -I../lib/sqlite3/include
LIBS = ../lib/sqlite3/lib


LIBS+= -L../lib/build/output 
LIBS+= -L../lib/openssl_install/lib
INC+=-I../include/src
INC+= -I../lib/openssl_install/include

FLAGS = -g -lpthread -lsqlite3 -g -lpaho-mqtt3c -lrt -lssl -lcrypto 

$(OBJ) : $(SRC)
	$(CC) $(SRC) -o $(OBJ) -I$(INC) -L$(LIBS) $(FLAGS) 
 
clean:
	rm $(OBJ)
