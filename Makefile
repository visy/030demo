CC=m68k-amigaos-gcc
VASM=vasmm68k_mot
VASMFLAGS=-Faout -devpac 
CFLAGS = -D__far="" -Wall -O2 -I. -m68030 -funroll-loops
LDFLAGS = -noixemul -L. 
SOURCES=main.c dlfcn.c starlight/graphics_controller.c starlight/utils.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=demo.exe

all: c2p_kalms.o c2p.o p2c.o $(SOURCES) $(EXECUTABLE) 

c2p_kalms.o: chunkyconverter/c2p_kalms.s
	$(VASM) $(VASMFLAGS) -o c2p_kalms.o chunkyconverter/c2p_kalms.s

c2p.o: chunkyconverter/c2p.s
	$(VASM) $(VASMFLAGS) -o c2p.o chunkyconverter/c2p.s

p2c.o: chunkyconverter/p2c.s
	$(VASM) $(VASMFLAGS) -o p2c.o chunkyconverter/p2c.s

$(EXECUTABLE): $(OBJECTS) p2c.o c2p.o
	$(CC) $(LDFLAGS) $(OBJECTS) p2c.o c2p.o c2p_kalms.o -o $@ -Lmfloat -lmdouble -lmtransdouble -lmtransfloat
	
clean: 
	rm *.o *.lnk *.info *.uaem $(EXECUTABLE)
