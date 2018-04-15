IDIR = ../include
CC=gcc
CFLAGS=-Wall -I $(IDIR)

ODIR=obj

LIBS=-lwiringPi

_DEPS = w3150.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

TX_SRC = tx_example.c  w3150.c
TX_OBJ = $(patsubst %.c,$(ODIR)/%.o, $(TX_SRC))

RX_SRC = recv_example.c  w3150.c
RX_OBJ = $(patsubst %.c,$(ODIR)/%.o, $(RX_SRC))

TAP_SRC = tap_example.c  w3150.c
TAP_OBJ = $(patsubst %.c,$(ODIR)/%.o, $(TAP_SRC))

$(ODIR)/%.o: %.c $(DEPS)
	@ mkdir -p obj
	$(CC) -c -o $@ $< $(CFLAGS)

all : tx_example recv_example tap_example

tx_example: $(TX_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

recv_example: $(RX_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

tap_example: $(TAP_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ tx_example recv_example tap_example 
