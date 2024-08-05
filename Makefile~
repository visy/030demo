CC=m68k-amigaos-gcc
VASM=vasmm68k_mot
VASMFLAGS=-Faout -devpac 
CFLAGS = -D__far="" -Wall -O3 -I. -m68000
LDFLAGS = -noixemul 
SOURCES=main.c starlight/graphics_controller.c starlight/utils.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=demo.exe

all: c2p.o p2c.o $(SOURCES) $(EXECUTABLE) 

c2p.o: chunkyconverter/c2p.s
	$(VASM) $(VASMFLAGS) -o c2p.o chunkyconverter/c2p.s

p2c.o: chunkyconverter/p2c.s
	$(VASM) $(VASMFLAGS) -o p2c.o chunkyconverter/p2c.s

$(EXECUTABLE): $(OBJECTS) p2c.o c2p.o
	$(CC) $(LDFLAGS) $(OBJECTS) p2c.o c2p.o -o $@
	
clean: 
	rm *.o *.lnk *.info *.uaem $(EXECUTABLE)
