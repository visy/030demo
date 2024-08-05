
// adapted from example blog tutorial by Christian Ammann

#include "demo.h"

// gfx data headers
#include "noitapic.h"

struct BitMap *mainBitmap1 = NULL;
struct BitMap *mainBitmap2 = NULL;

struct BitMap *tempBitmap = NULL;
UWORD backupBytesPerRow;
UWORD backupRows;

struct Screen *mainScreen1 = NULL;
struct Screen *mainScreen2 = NULL;
struct Screen *my_wbscreen_ptr;

// empty mouse pointer because we dont want to see a mouse
UWORD *emptyPointer;

// used for double buffering
BOOL bufferSelector;
struct BitMap *currentBitmap = NULL;
struct Screen *currentScreen = NULL;

// current active palette
UWORD *currentPal;

// chunky buffer for pixels
UBYTE *chunkyBuffer;

int frame = 0;

/*
 * Create two Screens and two BitMap as Screen content
 * for double buffering. Create a third BitMap, draw a
 * rectangle into it, rotate and blit into Screen
 */
int main(void) {
	int i = 0;
    // hide mouse
    emptyPointer = AllocVec(22 * sizeof(UWORD), MEMF_CHIP | MEMF_CLEAR);
    my_wbscreen_ptr = LockPubScreen("Workbench");
    SetPointer(my_wbscreen_ptr->FirstWindow, emptyPointer, 8, 8, -6, 0);
    UnlockPubScreen(NULL, my_wbscreen_ptr);

    // allocate temp bitmap
    tempBitmap = AllocBitMap(320, 256, 5, BMF_CLEAR, NULL);
    if (!tempBitmap) {
        goto _exit_main;
    }
    backupRows = tempBitmap->Rows;
    tempBitmap->Rows = 1;
    backupBytesPerRow = tempBitmap->BytesPerRow;
    tempBitmap->BytesPerRow = (((320 + 15) >> 4) << 1);

    // create pal screens for double buffering
    if (!initScreen(&mainBitmap1, &mainScreen1)) {
        goto _exit_free_temp_bitmap;
    }
    if (!initScreen(&mainBitmap2, &mainScreen2)) {
        goto _exit_free_first_screen;
    }
    bufferSelector = TRUE;
    currentScreen = mainScreen1;
    currentBitmap = mainBitmap1;

    chunkyBuffer = AllocVec(320 * 256 * sizeof(UBYTE), MEMF_FAST | MEMF_CLEAR);

    for (i = 0; i < 320 * 256; i++) 
    {
	    chunkyBuffer[i] = *((UBYTE*)noitapic + i);
    }

    currentPal = AllocVec(32 * sizeof(UWORD), MEMF_FAST | MEMF_CLEAR);

    for(i = 0; i < 32; i++) 
    {
	    currentPal[i] = noitapal[i];
    }

    // set 32 color palette on both double buffered screens
    LoadRGB4(&(mainScreen1->ViewPort), currentPal, 32);
    LoadRGB4(&(mainScreen2->ViewPort), currentPal, 32);

    /* main() just does boring boiler plate stuff,
     * real rotation work is done in execute()
     */
    execute();

    FreeVec(chunkyBuffer);
    FreeVec(currentPal);

    CloseScreen(mainScreen2);
    WaitTOF();
    FreeBitMap(mainBitmap2);
_exit_free_first_screen:
    CloseScreen(mainScreen1);
    WaitTOF();
    FreeBitMap(mainBitmap1);
_exit_free_temp_bitmap:
    tempBitmap->Rows = backupRows;
    tempBitmap->BytesPerRow = backupBytesPerRow;
    FreeBitMap(tempBitmap);
_exit_main:
    // restore mouse
    my_wbscreen_ptr = LockPubScreen("Workbench");
    ClearPointer(my_wbscreen_ptr->FirstWindow);
    UnlockPubScreen(NULL, my_wbscreen_ptr);
    FreeVec(emptyPointer);

    exit(RETURN_OK);
}

/*
 * Paint rectangle into BitMap, tranform to chunky buffer, rotate
 * by 10 degreee in a loop, transform back to planar and draw result on
 * Screen
 */



void execute() {
    int x,y;
    ScreenToFront(currentScreen);
    WaitTOF();
    // chunky buffer objects are converted to planar
    while (!mouseCiaStatus()) {
        int o = (frame%2)*320;
        for(y=frame%2;y<256;y+=2) 
    	{
    		for(x=0;x<320;x+=1) 
    		{
    			if (*((UBYTE*)noitapic + o) == 0) {
                    chunkyBuffer[o] = ((x>>2)&(y>>2))+frame;
    			}
                o+=1;
    		}
            o+=320;
    	}

        switchScreenData();
        convertChunkyToBitmap(chunkyBuffer, currentBitmap);
        ScreenToFront(currentScreen);
    	frame++;
    }
}

void switchScreenData() {
    if (bufferSelector) {
        currentScreen = mainScreen2;
        currentBitmap = mainBitmap2;
        bufferSelector = FALSE;
    } else {
        currentScreen = mainScreen1;
        currentBitmap = mainBitmap1;
        bufferSelector = TRUE;
    }
}

void convertChunkyToBitmap(UBYTE* sourceChunky, struct BitMap *destPlanar)
{
#ifdef NATIVE_CONVERTER
    struct RastPort rastPort1 = {0};
    struct RastPort rastPort2 = {0};
    InitRastPort(&rastPort1);
    InitRastPort(&rastPort2);

    rastPort1.BitMap = destPlanar;
    rastPort2.Layer = NULL;
    rastPort2.BitMap = tempBitmap;
    WritePixelArray8(&rastPort1, 0, 0, 320 - 1,
                     256 - 1, sourceChunky, &rastPort2);
#else
    struct c2pStruct c2p;
    c2p.bmap = destPlanar;
    c2p.startX = 0;
    c2p.startY = 0;
    c2p.width = 320;
    c2p.height = 256;
    c2p.chunkybuffer = sourceChunky;
    ChunkyToPlanarAsm(&c2p);
#endif
}

BOOL initScreen(struct BitMap **b, struct Screen **s)
{
    // load onscreen bitmap which will be shown on screen
    *b = AllocBitMap(320, 256,
                     5, BMF_DISPLAYABLE | BMF_CLEAR,
                     NULL);
    if (!*b) {
        printf("Error: Could not allocate memory for screen bitmap\n");
        goto __exit_init_error;
    }

    // create one screen which contains the demo logo
    *s = createScreen(*b, TRUE, 0, 0,
                      320, 256,
                      5, NULL);
    if (!*s) {
        printf("Error: Could not allocate memory for logo screen\n");
        goto __exit_init_bitmap;
    }
    return TRUE;

__exit_init_bitmap:
    FreeBitMap(*b);
__exit_init_error:
    return FALSE;
}
