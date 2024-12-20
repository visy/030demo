CC=m68k-amigaos-gcc
VASM=vasmm68k_mot
VASMFLAGS=-Faout -devpac -m68030
CFLAGS = -D__far="" -O3 -fomit-frame-pointer -fstrength-reduce -funroll-loops -m68030 -s -I. 
LDFLAGS = -noixemul -L. 
SOURCES=main.c dlfcn.c starlight/graphics_controller.c starlight/utils.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=demo.exe

all: ptplayer.o c2p_kalms.o c2p.o p2c.o $(SOURCES) $(EXECUTABLE) 

ptplayer.o: ptplayer/ptplayer.asm
	$(VASM) $(VASMFLAGS) -o ptplayer.o ptplayer/ptplayer.asm

c2p_kalms.o: chunkyconverter/c2p_kalms.s
	$(VASM) $(VASMFLAGS) -o c2p_kalms.o chunkyconverter/c2p_kalms.s

c2p.o: chunkyconverter/c2p.s
	$(VASM) $(VASMFLAGS) -o c2p.o chunkyconverter/c2p.s

p2c.o: chunkyconverter/p2c.s
	$(VASM) $(VASMFLAGS) -o p2c.o chunkyconverter/p2c.s

$(EXECUTABLE): $(OBJECTS) p2c.o c2p.o
	$(CC) $(LDFLAGS) $(OBJECTS) p2c.o c2p.o c2p_kalms.o ptplayer.o -o $@
	
clean: 
	rm *.o *.lnk *.info *.uaem $(EXECUTABLE)
