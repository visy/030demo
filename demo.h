#ifndef DEMO_H_
#define DEMO_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>

#include <graphics/displayinfo.h>
#include <graphics/gfxbase.h>
#include <graphics/rastport.h>
#include <graphics/videocontrol.h>
#include <graphics/gfxmacros.h>

#include <hardware/custom.h>
#include <hardware/cia.h>
#include <hardware/dmabits.h>

#include "starlight/blob_controller.h"
#include "starlight/graphics_controller.h"
#include "starlight/utils.h"

#include "chunkyconverter/chunkyconverter.h"

// #define NATIVE_CONVERTER

// credits to: https://coronax.wordpress.com/2014/01/31/running-with-the-numbers/
#define FIXSHIFT 16        // shift 16 bits = scale factor 65536
#define HALFSHIFT 8
// convert float to fix (and back)
#define FLOATTOFIX(x) ((int)((x) * (1<<FIXSHIFT)))
#define FIXTOFLOAT(x) ((float)(x) / (1<<FIXSHIFT))
// convert int to fix (and back)
#define INTTOFIX(x) ((x)<<FIXSHIFT)
#define FIXTOINT(x) ((x)>>FIXSHIFT)
// multiply and divide
#define FIXMULT(x,y) (((x)>>HALFSHIFT)*((y)>>HALFSHIFT))
#define FIXDIV(x,y) (((x)/(y>>HALFSHIFT))<<HALFSHIFT)

BOOL initScreen(struct BitMap **bm, struct Screen **s);
void execute(void);
void convertChunkyToBitmap(UBYTE* sourceChunky, struct BitMap *destPlanar);
void switchScreenData(void);

#endif  // DEMO_H_
