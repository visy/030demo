// adapted from example blog tutorial by Christian Ammann

#include <clib/timer_protos.h>
#include <clib/exec_protos.h>

#include "demo.h"

#include "ptplayer/ptplayer.h"

#define MAP_WIDTH  16
#define MAP_HEIGHT 16
#define FOV 20.0f
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 256
#define MAX_DEPTH 16
#define SCALE_FACTOR_SHIFT 10  // 1024 is 2^10

#define MAX_LINE_HEIGHT SCREEN_HEIGHT
unsigned int reciprocal_table[MAX_LINE_HEIGHT + 1];

#include "ray_lookup.h"
#include "quadansi.h"

typedef void (*DrawFunc)(void);

int scene = 1;
int oldscene = 1;
int ex = 0;
int nowscene = 0;

ULONG st,et = 0;
UBYTE dt = 0;
UWORD dta;
UWORD scenedta = 0;
UWORD alkudta = 0;

// gfx buffers for allocating from file

UBYTE* cpic2;
UBYTE* chei;
UBYTE* flypic;
UBYTE* shockpic;
UBYTE* sideflypic;
UBYTE* listrippic;
UBYTE* gpspic;
UBYTE* gps2pic;
UBYTE* tinyfont;
UBYTE* title;
UBYTE* door;
UBYTE* wall2;
UBYTE* lava;
UBYTE* sidefly2pic;
UBYTE* witch;

ULONG blackpal[] = {
    16l << 16 + 0,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0,0,0
};


ULONG custompal[] = {
    256l << 16 + 0,

    // Convert each 32-bit color to 3 left-justified longwords
    0x00000000, 0x00000000, 0x00000000,  // 0x00000000 -> Black
    0x66000000, 0x55000000, 0x33000000,  // 0x00665533 -> (R=0x66, G=0x55, B=0x33)
    0x66000000, 0x33000000, 0x22000000,  // 0x00663322 -> (R=0x66, G=0x33, B=0x22)
    0x99000000, 0x77000000, 0x44000000,  // 0x00997744 -> (R=0x99, G=0x77, B=0x44)
    0xAA000000, 0x88000000, 0x55000000,  // 0x00AA8855 -> (R=0xAA, G=0x88, B=0x55)
    0x99000000, 0x77000000, 0x66000000,  // 0x00997766 -> (R=0x99, G=0x77, B=0x66)
    0x88000000, 0x55000000, 0x33000000,  // 0x00885533 -> (R=0x88, G=0x55, B=0x33)
    0x88000000, 0x55000000, 0x66000000,  // 0x00885566 -> (R=0x88, G=0x55, B=0x66)
    0xAA000000, 0x88000000, 0x88000000,  // 0x00AA8888 -> (R=0xAA, G=0x88, B=0x88)
    0x55000000, 0x33000000, 0x22000000,  // 0x00553322 -> (R=0x55, G=0x33, B=0x22)
    0x33000000, 0x44000000, 0x55000000,  // 0x00334455 -> (R=0x33, G=0x44, B=0x55)
    0x55000000, 0x77000000, 0x99000000,  // 0x00557799 -> (R=0x55, G=0x77, B=0x99)
    0xAA000000, 0xBB000000, 0xBB000000,  // 0x00AABBBB -> (R=0xAA, G=0xBB, B=0xBB)
    0xBB000000, 0xBB000000, 0xCC000000,  // 0x00BBBBCC -> (R=0xBB, G=0xBB, B=0xCC)
    0x55000000, 0x33000000, 0x11000000,  // 0x00553311 -> (R=0x55, G=0x33, B=0x11)
    0x22000000, 0x11000000, 0x00000000,   // 0x00221100 -> (R=0x22, G=0x11, B=0x00)

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    // 16-color grayscale
    0x00000000, 0x00000000, 0x00000000,  // Black
    0x11000000, 0x11000000, 0x11000000,  // 0x00111111
    0x22000000, 0x22000000, 0x22000000,  // 0x00222222
    0x33000000, 0x33000000, 0x33000000,  // 0x00333333
    0x44000000, 0x44000000, 0x44000000,  // 0x00444444
    0x55000000, 0x55000000, 0x55000000,  // 0x00555555
    0x66000000, 0x66000000, 0x66000000,  // 0x00666666
    0x77000000, 0x77000000, 0x77000000,  // 0x00777777
    0x88000000, 0x88000000, 0x88000000,  // 0x00888888
    0x99000000, 0x99000000, 0x99000000,  // 0x00999999
    0xAA000000, 0xAA000000, 0xAA000000,  // 0x00AAAAAA
    0xBB000000, 0xBB000000, 0xBB000000,  // 0x00BBBBBB
    0xCC000000, 0xCC000000, 0xCC000000,  // 0x00CCCCCC
    0xDD000000, 0xDD000000, 0xDD000000,  // 0x00DDDDDD
    0xEE000000, 0xEE000000, 0xEE000000,  // 0x00EEEEEE
    0xFF000000, 0xFF000000, 0xFF000000,  // 0x00FFFFFF

    0 , 0 , 0
};

#include "font.h"

extern struct Custom custom;
extern struct CIA ciaa;

extern ULONG mt_get_vbr(void);
// kalms: c2p for 320x256,160x256 etc.
extern void c2p1x1_4_c5_bm_word(int chunkyx __asm("d0"), int chunkyy __asm("d1"), int offsx __asm("d2"), int offsy __asm("d3"), void* c2pscreen __asm("a0"), struct BitMap* bitmap __asm("a1"));
extern void c2p2x1_4_c5_bm(int chunkyx __asm("d0"), int chunkyy __asm("d1"), int offsx __asm("d2"), int offsy __asm("d3"), void* c2pscreen __asm("a0"), struct BitMap* bitmap __asm("a1"));

extern void c2p2x1_8_c5_030_init(WORD chunkyx __asm("d0") ,
                                       WORD chunkyy __asm("d1") ,
                                       WORD scroffsx __asm("d2") ,
                                       WORD scroffsy __asm("d3") ,
                                       WORD rowlen __asm("d4") ,
                                       LONG bplsize __asm("d5") );

extern void c2p2x1_8_c5_030(void* c2pscreen __asm("a0"), void* bitplanes __asm("a1"));

// Lynxx: fast lines
extern void ChunkyLine(void* ChunkyScreen __asm("a0"), int x0 __asm("d0"), int y0 __asm("d1"), int x1 __asm("d2"), int y1 __asm("d3"), int Color __asm("d4"), int pixelwidth __asm("d5"), int pixelheight __asm("d6"));

extern void ChunkyLine(void* ChunkyScreen __asm("a0"), int x0 __asm("d0"), int y0 __asm("d1"), int x1 __asm("d2"), int y1 __asm("d3"), int Color __asm("d4"), int pixelwidth __asm("d5"), int pixelheight __asm("d6"));
extern void vline(void* ChunkyScreen __asm("a0"), int color __asm("d0"), int height __asm("d1"));
extern void vline1(void* ChunkyScreen __asm("a0"), UBYTE color __asm("d0"), int height __asm("d1"));

extern UBYTE mt_E8Trigger;

struct ExecBase     *SysBase;

UBYTE* moddata;

struct BitMap *mainBitmap1 = NULL;
struct Screen *mainScreen1 = NULL;
struct Screen *my_wbscreen_ptr;

// empty mouse pointer because we dont want to see a mouse
UWORD *emptyPointer;

// used for double buffering
BOOL bufferSelector;
struct BitMap *currentBitmap = NULL;
struct Screen *currentScreen = NULL;

// chunky buffer for pixels
UBYTE *chunkyBuffer;

int frame = 0;
int totalframes = 0;

UBYTE sine[];
int zmul[512][512];
int zdiv[512][512];
int zmod[512][512];
int pmul[256][256];

#define MAX_FRAME 256  // Define a maximum frame range for the lookup table

int diagBlockLookup[MAX_FRAME][MAX_FRAME];  // Lookup table for diagBlock

int ymul[256] = 
{
    0
};

UBYTE drawcolor = 7;

ULONG millis;
ULONG st;

struct Device* TimerBase;

struct MsgPort *timer_msgport;
struct timerequest *timer_ioreq;

static struct timeval startTime;

inline int mod(int dividend, int divisor) {
    int absDividend = 0;
    int absDivisor = 0;
    int result = 0;
    int tempDivisor;

    if (divisor == 0) {
        // Modulo by zero is undefined
        return -1; // or handle the error as needed
    }

    // Get absolute values
    absDividend = abs(dividend);
    absDivisor = abs(divisor);
    result = absDividend;

    // Efficiently subtract divisor by shifting it closer to the dividend
    while (result >= absDivisor) {
        tempDivisor = absDivisor;

        // Find the largest multiple of divisor that is less than or equal to result
        while ((tempDivisor << 1) <= result) {
            tempDivisor <<= 1;
        }

        // Subtract that multiple from result
        result -= tempDivisor;
    }

    // Adjust result for the original sign of the dividend
    if (dividend < 0 && result != 0) {
        result = absDivisor - result;
    }

    return result;
}

inline void pixel(int x,int y) {
    // PLOT x,y point on surface
    chunkyBuffer[ymul[y]+x] = drawcolor;
}


// THE EXTREMELY FAST LINE ALGORITHM Variation E (Addition Fixed Point PreCalc Small Display)
// Small Display (256x256) resolution.
void line(int x, int y, int x2, int y2) {
    UBYTE yLonger=0;
    int shortLen=y2-y;
    int longLen=x2-x;
    int j;
    int decInc;

    if (abs(shortLen)>abs(longLen)) {
        int swap=shortLen;
        shortLen=longLen;
        longLen=swap;               
        yLonger=1;
    }
    if (longLen==0) decInc=0;
    else decInc = (shortLen << 8) / longLen;

    if (yLonger == 1) {
        if (longLen>0) {
            longLen+=y;
            for (j=0x80+(x<<8);y<=longLen;++y) {
                pixel(j >> 8,y);  
                j+=decInc;
            }
            return;
        }
        longLen+=y;
        for (j=0x80+(x<<8);y>=longLen;--y) {
            pixel(j >> 8,y);  
            j-=decInc;
        }
        return; 
    }

    if (longLen>0) {
        longLen+=x;
        for (j=0x80+(y<<8);x<=longLen;++x) {
            pixel(x,j >> 8);
            j+=decInc;
        }
        return;
    }
    longLen+=x;
    for (j=0x80+(y<<8);x>=longLen;--x) {
        pixel(x,j >> 8);
        j-=decInc;
    }

}

void square(int x, int y, int x2, int y2) {
    line(x,y,x2,y2);
    line(x2,y2,x2+(y-y2),y2+(x2-x));
    line(x,y,x+(y-y2),y+(x2-x));
    line(x+(y-y2),y+(x2-x),x2+(y-y2),y2+(x2-x));
}


void rect(int x, int y, int x2, int y2) {
    line(x,y,x2,y);
    line(x2,y,x2,y2);
    line(x2,y2,x,y2);
    line(x,y2,x,y);
}

void fillrect(int x, int y, int x2, int y2) {
    int fy = 0;
    rect(x,y,x2,y2);

    for (fy=y;fy < y2; fy++) {
        line(x,fy,x2,fy);
    }
}

void aaplot(int x, int y, int alpha) {
    int index;
    UBYTE current_color;
    int blended_color;

    if (x >= 0 && x < 160 && y >= 0 && y < 256) {
        index = (y << 8) + (y << 6) + x;
        current_color = chunkyBuffer[index];
        blended_color = current_color + ((15 - current_color) * alpha) / 15;

        if (blended_color > 15) blended_color = 15;
        if (blended_color < 0) blended_color = 0;

        chunkyBuffer[index] = (UBYTE)blended_color;
    }
}

void aaline(int x0, int y0, int x1, int y1, UBYTE color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    int e2;
    int alpha_y, alpha_x;
    int intensity_y, intensity_x;

    while (1) {
        aaplot(x0, y0, 15); // Full intensity for the main pixel

        if (x0 == x1 && y0 == y1) break;

        e2 = err << 1;

        if (e2 > -dy) {
            alpha_y = (255 * abs(e2 + dy)) / (2 * dx);
            intensity_y = 15 - ((alpha_y * 15) / 255);
            if (intensity_y < 0) intensity_y = 0;
            if (intensity_y > 15) intensity_y = 15;
            aaplot(x0, y0 + sy, intensity_y);
        }

        if (e2 < dx) {
            alpha_x = (255 * abs(dx - e2)) / (2 * dy);
            intensity_x = 15 - ((alpha_x * 15) / 255);
            if (intensity_x < 0) intensity_x = 0;
            if (intensity_x > 15) intensity_x = 15;
            aaplot(x0 + sx, y0, intensity_x);
        }

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void aalinethick(int x0, int y0, int x1, int y1, UBYTE color, int thickness) {
    int half_thickness = thickness / 2;
    int i;

    for (i = -half_thickness; i <= half_thickness; i++) {
        if (abs(x1 - x0) >= abs(y1 - y0)) {
            aaline(x0, y0 + i, x1, y1 + i, color); // Adjust y for horizontal-ish lines
        } else {
            aaline(x0 + i, y0, x1 + i, y1, color); // Adjust x for vertical-ish lines
        }
    }
}

#define TGA_HEADER_SIZE 18
#define MAX_COLORS 16
#define PALETTE_COLORS 256 

typedef struct {
    UBYTE idLength;
    UBYTE colorMapType;
    UBYTE imageType;
    UBYTE colorMapSpec[5];  // Ignored for simplicity
    UWORD xOrigin;
    UWORD yOrigin;
    UWORD width;   // Needs endian conversion
    UWORD height;  // Needs endian conversion
    UBYTE pixelDepth;  // 24-bit (RGB) or 32-bit (RGBA)
    UBYTE imageDescriptor;
} TGAHeader;

UWORD LittleEndianToBigEndian(UWORD val) {
    return (val << 8) | (val >> 8);
}

void ConvertToAmigaRGB(UBYTE r, UBYTE g, UBYTE b, ULONG* amigaR, ULONG* amigaG, ULONG* amigaB) {
    *amigaR = (ULONG)(b & 0xFF) << 24;  // Red channel occupies the full 32 bits
    *amigaG = (ULONG)(g & 0xFF) << 24;  // Green channel occupies the full 32 bits
    *amigaB = (ULONG)(r & 0xFF) << 24;  // Blue channel occupies the full 32 bits
}

int FindColorInPaletteBlock(ULONG* palette, ULONG amigaR, ULONG amigaG, ULONG amigaB, int startIdx) {
    int i;
    for (i = startIdx; i < startIdx + MAX_COLORS; i++) {
        if (palette[1+i * 3] == amigaR && palette[1+i * 3 + 1] == amigaG && palette[1+i * 3 + 2] == amigaB) {
            return i - startIdx;  // Return index relative to the block
        }
    }
    return -1;  // Color not found
}

void DrawPic(UBYTE* pic, int x, int y, int picWidth, int picHeight) {
    int i, j;
    int destX;
    int destY;
    UBYTE pix;

    // Loop through each pixel of the source picture
    for (j = 0; j < picHeight; j++) {
        destY = y + j;
        if (destY < 0 || destY >= 256) continue;

        for (i = 0; i < picWidth; i++) {
            // Calculate the position in chunkyBuffer using the x, y coordinates
            destX = x + i;

            // Only draw if within screen bounds (160x256)
            if (destX >= 0 && destX < 160) {
                pix = pic[pmul[j][picWidth] + i];
                chunkyBuffer[ymul[destY] + destX] = pix;
            }
        }
    }
}


void DrawPicV(UBYTE* pic, int x, int y, int picWidth, int picHeight) {
    int i, j;
    int destX;
    int destY;
    UBYTE pix;

    // Loop through each pixel of the source picture
    for (j = 0; j < picHeight; j++) {
        destY = y + j;
        if (destY < 0 || destY >= 256) continue;

        for (i = 0; i < picWidth; i++) {
            // Calculate the position in chunkyBuffer using the x, y coordinates
            destX = x + i;

            // Only draw if within screen bounds (160x256)
            if (destX >= 0 && destX < 160) {
                pix = pic[pmul[j>>1][picWidth] + (i)];
                chunkyBuffer[ymul[destY] + destX] = pix;
            }
        }
    }
}

void DrawPicT(UBYTE* pic, int x, int y, int picWidth, int picHeight) {
    int i, j;
    int destX;
    int destY;
    UBYTE pix;

    // Loop through each pixel of the source picture
    for (j = 0; j < picHeight; j++) {
        destY = y + j;
        if (destY < 0 || destY >= 256) continue;

        for (i = 0; i < picWidth; i++) {
            // Calculate the position in chunkyBuffer using the x, y coordinates
            destX = x + i;

            // Only draw if within screen bounds (160x256)
            if (destX >= 0 && destX < 160) {
                pix = pic[pmul[j][picWidth] + i];
                if (pix != 144) {
                    chunkyBuffer[ymul[destY] + destX] = pix;
                }
            }
        }
    }
}

UBYTE* LoadTarga(const char* filename, int imageSlot) {
    BPTR file_ptr;
    TGAHeader header;
    UBYTE* image_data;
    UBYTE* remapped_data;
    ULONG amigaR, amigaG, amigaB;
    ULONG imageSize;
    int paletteIndex, colorCount;
    int i, j, index, row;
    UBYTE r, g, b;
    int paletteOffset = imageSlot * MAX_COLORS * 3 ;  // Start index in the palette for this image slot (3 ULONGs per color)
    int topDown = 0;  // Default to bottom-up

    // Open the TGA file
    file_ptr = Open(filename, MODE_OLDFILE);
    if (!file_ptr) {
        printf("Error: Could not open file %s\n", filename);
        return NULL;
    }

    // Read the TGA header
    if (Read(file_ptr, &header, TGA_HEADER_SIZE) != TGA_HEADER_SIZE) {
        printf("Error: Could not read TGA header from file %s\n", filename);
        Close(file_ptr);
        return NULL;
    }

    // Convert width and height from little-endian to big-endian
    header.width = LittleEndianToBigEndian(header.width);
    header.height = LittleEndianToBigEndian(header.height);


    // Ensure it's a 24-bit (RGB) or 32-bit (RGBA) image
    if (header.pixelDepth != 24 && header.pixelDepth != 32) {
        printf("Error: Unsupported pixel depth %d in file %s. Only 24-bit or 32-bit supported.\n", header.pixelDepth, filename);
        Close(file_ptr);
        return NULL;
    }

    // Calculate the size of the image data
    imageSize = header.width * header.height * (header.pixelDepth / 8);
    image_data = (UBYTE*)AllocVec(imageSize, MEMF_PUBLIC | MEMF_CLEAR);
    if (!image_data) {
        printf("Error: Could not allocate memory for image data in file %s\n", filename);
        Close(file_ptr);
        return NULL;
    }

    // Read the image pixel data
    if (Read(file_ptr, image_data, imageSize) != imageSize) {
        printf("Error: Could not read image data from file %s\n", filename);
        FreeVec(image_data);
        Close(file_ptr);
        return NULL;
    }

    // Allocate memory for the remapped data (palette indices)
    remapped_data = (UBYTE*)AllocVec(header.width * header.height, MEMF_PUBLIC | MEMF_CLEAR);
    if (!remapped_data) {
        printf("Error: Could not allocate memory for remapped data in file %s\n", filename);
        FreeVec(image_data);
        Close(file_ptr);
        return NULL;
    }

    // Clear the current 16-color block in the global palette for this image slot
    for (i = 0; i < MAX_COLORS * 3; i++) {
        custompal[1+paletteOffset + i] = 0x00000000;  // Clear the current 16 colors (3 ULONGs per color) to black
    }

    // Process the pixel data and map to palette indices
    colorCount = 0;
    for (j = 0; j < header.height; j++) {
        row = j;  // Adjust for image orientation
        for (i = 0; i < header.width; i++) {
            index = (row * header.width + i) * (header.pixelDepth / 8);
            r = image_data[index];
            g = image_data[index + 1];
            b = image_data[index + 2];

            // Convert to Amiga 12-bit color (3 separate ULONGs for each channel)
            ConvertToAmigaRGB(r, g, b, &amigaR, &amigaG, &amigaB);

            // Find or add the color to the current palette block
            paletteIndex = FindColorInPaletteBlock(custompal, amigaR, amigaG, amigaB, paletteOffset / 3);

            if (paletteIndex == -1) {
                // New unique color found
                if (colorCount < MAX_COLORS) {
                    custompal[1+paletteOffset + colorCount * 3] = amigaR;      // Add red channel to the global palette
                    custompal[1+paletteOffset + colorCount * 3 + 1] = amigaG;  // Add green channel to the global palette
                    custompal[1+paletteOffset + colorCount * 3 + 2] = amigaB;  // Add blue channel to the global palette
                    paletteIndex = colorCount;
                    colorCount++;
                } else {
                    printf("Error: Too many unique colors in file %s. Max supported is %d.\n", filename, MAX_COLORS);
                    FreeVec(image_data);
                    FreeVec(remapped_data);
                    Close(file_ptr);
                    return NULL;
                }
            }

            // Map the pixel to the palette index (relative to the current 16-color block)
            remapped_data[(header.height - 1 - j) * header.width + i] = (UBYTE)(paletteIndex + (paletteOffset / 3));
        }
    }

    // Clean up
    FreeVec(image_data);
    Close(file_ptr);

    return remapped_data;  // Return the remapped image data
}

typedef struct {
    int x, y;          // Position in the texture atlas
    int width, height; // Dimensions of the character
    int xoffset, yoffset; // Character offsets
    int xadvance;      // Space to advance to the next character
} CharInfo;

// Font character information for the full character set
CharInfo font_chars[94] = {
    {0, 0, 0, 0, 0, 0, 11},  // ' ' (space)
    {243, 68, 10, 14, -3, 22, 6},  // ',' 
    {243, 83, 7, 7, -2, 23, 4},    // '.' 
    {160, 98, 15, 25, -1, 5, 13},  // '?' 
    {28, 68, 22, 28, -1, 5, 21},   // 'A'
    {176, 98, 19, 24, -1, 6, 18},  // 'B'
    {165, 68, 18, 26, -1, 5, 16},  // 'C'
    {184, 68, 19, 26, -1, 6, 16},  // 'D'
    {196, 98, 19, 24, -1, 5, 16},  // 'E'
    {216, 98, 19, 24, -1, 6, 17},  // 'F'
    {21, 0, 21, 34, -2, 5, 19},    // 'G'
    {117, 0, 20, 32, -1, 6, 18},   // 'H'
    {0, 125, 19, 24, -2, 6, 15},   // 'I'
    {113, 36, 24, 30, -6, 6, 17},  // 'J'
    {51, 68, 25, 28, -1, 6, 19},   // 'K'
    {204, 68, 18, 26, -2, 5, 15},  // 'L'
    {0, 36, 30, 31, -1, 7, 28},    // 'M'
    {31, 36, 21, 31, -1, 7, 19},   // 'N'
    {236, 98, 20, 24, -1, 6, 18},  // 'O'
    {62, 98, 18, 25, -1, 6, 16},   // 'P'
    {43, 0, 30, 33, -1, 6, 20},    // 'Q'
    {138, 0, 30, 32, -1, 6, 19},   // 'R'
    {232, 36, 24, 26, -4, 5, 18},  // 'S'
    {20, 125, 20, 24, -1, 6, 17},  // 'T'
    {41, 125, 23, 23, -2, 7, 20},  // 'U'
    {74, 0, 22, 33, -1, -2, 20},   // 'V'
    {169, 0, 27, 32, -1, -2, 25},  // 'W'
    {227, 0, 29, 31, -1, 5, 22},   // 'X'
    {53, 36, 29, 31, -5, 6, 23},   // 'Y'
    {138, 36, 29, 30, -1, 6, 21},  // 'Z'
    {65, 125, 18, 21, -1, 10, 16}, // 'a'
    {81, 98, 17, 25, -1, 5, 15},   // 'b'
    {84, 125, 18, 20, -2, 10, 15}, // 'c'
    {77, 68, 19, 28, -2, 2, 16},   // 'd'
    {187, 125, 17, 18, -1, 12, 15},// 'e'
    {197, 0, 29, 32, -8, 4, 18},   // 'f'
    {97, 0, 19, 33, -2, 10, 16},   // 'g'
    {0, 0, 20, 35, -1, 4, 18},     // 'h'
    {99, 98, 15, 25, -1, 5, 13},   // 'i'
    {208, 36, 23, 29, -8, 8, 14},  // 'j'
    {223, 68, 19, 26, -2, 5, 15},  // 'k'
    {115, 98, 15, 25, -1, 5, 13},  // 'l'
    {97, 68, 28, 27, -2, 11, 25},  // 'm'
    {168, 36, 19, 30, -1, 11, 17}, // 'n'
    {103, 125, 18, 20, 0, 11, 19}, // 'o'
    {126, 68, 19, 27, 0, 11, 20},  // 'p'
    {188, 36, 19, 30, 0, 9, 20},   // 'q'
    {0, 98, 30, 26, 0, 11, 19},    // 'r'
    {122, 125, 21, 20, -4, 11, 15},// 's'
    {144, 125, 20, 19, -2, 11, 17},// 't'
    {165, 125, 21, 19, -1, 11, 18},// 'u'
    {146, 68, 18, 27, -1, 3, 16},  // 'v'
    {0, 68, 27, 29, -2, 1, 24},    // 'w'
    {31, 98, 30, 26, -5, 9, 24},   // 'x'
    {83, 36, 29, 31, -6, 10, 22},  // 'y'
    {131, 98, 28, 25, -1, 11, 18}  // 'z'
};

#define FONT_ATLAS_WIDTH 256  // Width of the font atlas image

void draw_font_bitmap(int x, int y, int width, int height, int srcX, int srcY, int ander) {
    int row, col;
    UBYTE pixel_value;
    int atlas_index;
    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            // Calculate the source position in the font atlas
            atlas_index = (srcY + row) * FONT_ATLAS_WIDTH + (srcX + col);
            
            // Read the pixel value from the font atlas (grayscale value 16-31)
            pixel_value = font_atlas[atlas_index];
            
            // Only render non-transparent pixels (16-30), 31 is fully white (transparent)
            if (pixel_value > 16 && ander == 1) {
                chunkyBuffer[ymul[y + row] + (x + col)] |= pixel_value;
                chunkyBuffer[ymul[y + row+1] + (x + col)] |= 16;
            }
            if (pixel_value > 16 && ander == 0) {
                chunkyBuffer[ymul[y + row] + (x + col)] = pixel_value;
                chunkyBuffer[ymul[y + row+1] + (x + col)] = 16;
            }

        }
    }
}

const int base_height = 33;  

// Function to render a character at a given position
void render_char(char ch, int xpos, int ypos, int baseline_y, int ander) {
    int char_ypos, char_index;
    // Lookup character info
    CharInfo *char_info = NULL;

    if (ch == ' ') {
        return;
    }

    char_index = ch - ' ' - 29;  // Original logic for other characters
    if (ch >= 'a') {
        char_index -= 6;
    }

    if (ch == '.') {
        char_index = 2;
    }

    if (ch == ',') {
        char_index = 1;
    }

    if (ch == '?') {
        char_index = 3;
    }

    char_ypos = baseline_y - (base_height - font_chars[char_index].yoffset);

    // Draw the character (this is where you would implement the drawing logic)
    draw_font_bitmap(xpos, char_ypos, font_chars[char_index].width, font_chars[char_index].height,
                     font_chars[char_index].x, font_chars[char_index].y, ander);

}

// Function to render a string of text
void render_text(const char *text, int xpos, int ypos, int ander) {
    int baseline_y = ypos;
    unsigned char ch;
    int char_index;
    while (*text) {
        ypos = baseline_y - (base_height - font_chars[(unsigned char)*text - ' ' - 29].yoffset);
        if (*text == ' ') {
            xpos += 8;  // Advance for space without rendering
        } else {
            render_char(*text, xpos, ypos, baseline_y, ander);
            char_index = *text - ' ' - 29;  // Original logic for other characters
            if (*text >= 'a') {
                char_index -= 6;
            }
            if (*text == '.') {
                char_index = 2;
            }

            if (*text == ',') {
                char_index = 1;
            }

            if (*text == '?') {
                char_index = 3;
            }

            xpos += font_chars[char_index].xadvance;
        }
        text++;
    }
}

// Function to render a character at a given position
void render_tinychar(char ch, int xpos, int ypos, UBYTE col) {
    int char_x, char_y;
    UBYTE x,y,pix;

    if (ch == ' ') {
        return;
    }

    char_x = ((ch - 32) % 19)*5;
    char_y = ((ch - 32) / 19)*5;

    for (y = 0; y < 5; y++) {
        for (x = 0; x < 5; x++) {
            pix = tinyfont[(((char_y+y)*96))+(char_x+x+1)];
            if (pix != 224) {
                chunkyBuffer[ymul[ypos+(y<<1)]+xpos+x] = col;
                chunkyBuffer[ymul[ypos+(y<<1)+1]+xpos+x] = col;
            }
        }
    }
}

void render_tinytext(const char *text, int xpos, int ypos, UBYTE col) {
    int baseline_y = ypos;
    unsigned char ch;
    int char_index;
    while (*text) {
        ypos = baseline_y;
        if (*text == ' ') {
            xpos += 6;
        } else {
            render_tinychar(*text, xpos, ypos, col);
            char_index = *text - 32;  // Original logic for other characters
            xpos += 6;
        }
        text++;
    }
}

void render_tinytext_d(const char *text, int xpos, int ypos, UBYTE col,int tt) {
    int baseline_y = ypos;
    unsigned char ch;
    int char_index;
    int lala = 0;
    while (*text) {
        ypos = baseline_y;
        if (*text == ' ') {
            xpos += 6;
        } else {
            render_tinychar(*text, xpos, ypos, col);
            char_index = *text - 32;  // Original logic for other characters
            xpos += 6;
        }
        text++;
        if (lala > tt>>7) break;
        lala++;
    }
}

int ppx = 6<<10;
int ppy = 7<<10;
int pdir = 1; // up, down, left, right


UBYTE texture[32 * 32] = 
{
    25, 25, 24, 24, 24, 24, 25, 23, 21, 22, 24, 25, 25, 25, 25, 25, 24, 24, 24, 24, 25, 25, 24, 23, 22, 20, 23, 25, 25, 25, 25, 25,
    25, 25, 25, 24, 24, 24, 24, 23, 21, 22, 24, 25, 25, 25, 25, 25, 24, 24, 24, 25, 25, 25, 24, 23, 22, 20, 23, 25, 25, 25, 25, 25,
    25, 25, 25, 25, 24, 24, 24, 22, 21, 22, 24, 25, 25, 25, 24, 24, 25, 25, 25, 25, 25, 25, 24, 23, 22, 20, 23, 25, 25, 25, 25, 24,
    25, 25, 25, 25, 24, 24, 24, 23, 21, 22, 24, 25, 25, 24, 24, 24, 25, 25, 25, 25, 25, 25, 24, 24, 22, 20, 23, 25, 25, 25, 24, 24,
    24, 25, 25, 25, 24, 24, 25, 23, 21, 22, 24, 25, 24, 24, 24, 24, 25, 25, 24, 24, 24, 24, 25, 24, 22, 20, 23, 25, 24, 24, 24, 24,
    23, 23, 23, 23, 24, 24, 25, 23, 21, 21, 23, 23, 22, 22, 22, 23, 23, 23, 23, 22, 22, 22, 23, 23, 22, 21, 22, 23, 24, 24, 24, 24,
    21, 21, 21, 22, 24, 24, 24, 23, 21, 21, 21, 20, 20, 20, 20, 21, 21, 21, 21, 20, 20, 21, 21, 21, 21, 21, 21, 21, 22, 24, 24, 24,
    21, 21, 22, 23, 24, 24, 24, 24, 22, 22, 22, 22, 22, 22, 22, 21, 21, 21, 22, 22, 22, 22, 23, 23, 22, 22, 22, 22, 22, 22, 22, 22,
    20, 22, 24, 25, 25, 25, 25, 24, 24, 24, 24, 24, 24, 24, 24, 23, 21, 21, 24, 25, 25, 25, 25, 25, 24, 24, 24, 23, 22, 21, 21, 21,
    20, 22, 25, 25, 25, 25, 25, 25, 24, 24, 24, 24, 25, 25, 24, 23, 21, 21, 24, 26, 26, 25, 26, 25, 25, 24, 24, 24, 22, 20, 21, 21,
    20, 22, 25, 25, 25, 25, 25, 25, 25, 25, 24, 24, 25, 25, 24, 23, 21, 21, 24, 26, 25, 24, 25, 25, 24, 24, 24, 24, 22, 21, 22, 23,
    20, 22, 24, 24, 25, 25, 25, 25, 25, 25, 24, 24, 25, 25, 24, 23, 21, 21, 24, 25, 25, 24, 25, 25, 25, 24, 24, 24, 23, 22, 23, 24,
    20, 21, 22, 23, 24, 25, 25, 25, 25, 25, 24, 24, 24, 24, 25, 24, 21, 21, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 24, 23, 24, 25,
    20, 20, 21, 22, 23, 23, 23, 24, 24, 23, 23, 23, 23, 23, 23, 23, 21, 21, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 24, 25, 25,
    21, 21, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    22, 22, 21, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 23, 23, 23, 23, 23, 24, 22, 20, 20, 21, 21, 21, 21, 21, 21, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    25, 25, 24, 24, 24, 24, 25, 23, 21, 21, 22, 22, 22, 22, 22, 22, 21, 21, 21, 21, 21, 21, 21, 21, 20, 20, 21, 21, 21, 21, 21, 21,
    25, 25, 25, 24, 24, 24, 24, 23, 21, 21, 23, 24, 24, 24, 23, 23, 23, 24, 24, 24, 24, 23, 23, 23, 22, 21, 22, 24, 24, 24, 23, 23,
    25, 25, 25, 25, 24, 24, 24, 23, 21, 22, 24, 25, 25, 25, 24, 24, 25, 25, 25, 25, 25, 25, 24, 24, 22, 21, 23, 25, 25, 25, 24, 24,
    24, 25, 25, 25, 24, 24, 25, 23, 21, 22, 24, 25, 24, 24, 24, 24, 25, 25, 25, 24, 24, 24, 25, 24, 22, 21, 23, 25, 25, 24, 24, 24,
    24, 24, 25, 25, 24, 25, 25, 24, 22, 22, 24, 25, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 23, 22, 24, 25, 25, 24, 24, 24,
    22, 24, 26, 25, 25, 25, 26, 25, 24, 25, 25, 26, 25, 25, 25, 25, 23, 23, 24, 25, 25, 25, 26, 25, 25, 24, 25, 26, 25, 25, 25, 25,
    21, 23, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 25, 22, 22, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    20, 23, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 26, 26, 25, 22, 22, 24, 26, 26, 27, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27,
    20, 23, 27, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 26, 24, 21, 21, 24, 26, 26, 27, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27,
    20, 23, 26, 26, 25, 26, 26, 26, 26, 26, 25, 25, 26, 26, 25, 24, 21, 21, 22, 24, 25, 25, 25, 25, 25, 25, 25, 26, 26, 25, 25, 25,
    20, 22, 25, 25, 25, 25, 25, 25, 25, 25, 24, 24, 25, 25, 24, 23, 21, 20, 21, 22, 23, 24, 24, 24, 24, 24, 25, 25, 24, 24, 24, 24,
    20, 22, 25, 25, 24, 25, 25, 25, 25, 25, 24, 24, 24, 24, 25, 23, 21, 21, 22, 23, 24, 24, 24, 24, 24, 25, 25, 25, 24, 24, 24, 25,
    20, 22, 25, 24, 24, 24, 25, 25, 25, 24, 24, 24, 24, 24, 25, 24, 21, 21, 23, 24, 24, 24, 24, 24, 24, 25, 25, 25, 24, 24, 24, 25,
    20, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 21, 20, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

UBYTE world_map[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,2,1,1,2,2,1,2,1,2,1,2,1},
    {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {2,0,0,0,0,0,0,0,0,0,0,1,1,1,0,2},
    {1,0,0,1,2,1,0,0,0,0,0,1,1,1,0,1},
    {1,0,0,1,1,2,0,0,0,0,0,1,0,0,0,2},
    {1,1,2,1,1,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,2,1,0,0,0,0,0,0,0,0,0,2},
    {1,0,1,0,1,0,1,0,0,0,0,0,1,0,1,1},
    {1,0,1,0,0,0,2,0,0,0,1,0,2,0,0,2},
    {2,0,1,1,0,0,2,0,0,0,1,0,1,0,0,1},
    {1,0,1,1,0,0,1,1,0,0,2,0,2,0,0,2},
    {1,0,0,0,0,0,0,1,0,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,2,0,2,0,0,1},
    {1,1,1,1,2,1,1,2,2,1,1,2,1,1,1,1},
};

struct Library *UtilityBase;

#define DiagShiftAmount 5

UBYTE texture_lookup[32 * 32];
UBYTE door_lookup[32 * 32];
UBYTE wall2_lookup[32 * 32];

inline UBYTE GetColumnMajorPixelAt(UBYTE* texture, int x, int y) {
    int index;
    // Wrap the coordinates to stay within texture bounds (for tiling)
    x &= 31;
    y &= 31;

    // Calculate the index in column-major order
    index = (x << 5) + y;
    return texture[index];
}

void init_lookup_tables() {
    int i, ii;
    int xFrame, yFrame;
    int texX, texY;

    for (xFrame = 0; xFrame < MAX_FRAME; xFrame++) {
        for (yFrame = 0; yFrame < MAX_FRAME; yFrame++) {
            diagBlockLookup[xFrame][yFrame] = ((xFrame + yFrame) >> DiagShiftAmount) ^ ((xFrame - yFrame) >> DiagShiftAmount);
        }
    }

    for(i=0; i < 256; i++) {
        ymul[i] = i * 160;
    }

    for(ii=0; ii < 256; ii++) {
            for(i=0; i < 256; i++) {
                pmul[ii][i] = ii*i;
            }
    }

    for(ii=0; ii < 512; ii++) {
        for(i=0; i < 512; i++) {
            zmod[ii][i] = (ii+1) % (i+1);
            zmul[ii][i] = (ii-160) * (i);
            zdiv[ii][i] = (ii+1) / (i+1);
        }
    }

    for (i = 1; i <= MAX_LINE_HEIGHT; i++) {
        reciprocal_table[i] = 4096 / i;  // Precompute reciprocal_line_height
    }

    // Precompute texture addresses for each possible texX, texY pair
    for (texY = 0; texY < 32; texY++) {
        for (texX = 0; texX < 32; texX++) {
            texture_lookup[(texY << 5) + texX] = texture[(texY << 5) + texX];
            door_lookup[(texY << 5) + texX] = door[(texY << 5) + texX];
            wall2_lookup[(texY << 5) + texX] = wall2[(texY << 5) + texX];
        }
    }

}

int cos_table[266] = {
    128, 127, 127, 127, 126, 126, 125, 124, 123, 122, 121, 120, 118, 117, 115, 114, 112, 110, 108, 106, 104, 101, 99, 96, 93, 91, 88, 85, 82, 79, 76, 72, 69, 66, 62, 59, 55, 51, 48, 44, 40, 36, 32, 28, 24, 20, 16, 12, 8, 4, 0, -4, -8, -12, -16, -20, -24, -28, -32, -36, -40, -44, -48, -51, -55, -59, -62, -66, -69, -72, -76, -79, -82, -85, -88, -91, -93, -96, -99, -101, -104, -106, -108, -110, -112, -114, -115, -117, -118, -120, -121, -122, -123, -124, -125, -126, -126, -127, -127, -127, -128, -127, -127, -127, -126, -126, -125, -124, -123, -122, -121, -120, -118, -117, -115, -114, -112, -110, -108, -106, -104, -101, -99, -96, -93, -91, -88, -85, -82, -79, -76, -72, -69, -66, -62, -59, -55, -51, -48, -44, -40, -36, -32, -28, -24, -20, -16, -12, -8, -4, 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 51, 55, 59, 62, 66, 69, 72, 76, 79, 82, 85, 88, 91, 93, 96, 99, 101, 104, 106, 108, 110, 112, 114, 115, 117, 118, 120, 121, 122, 123, 124, 125, 126, 126, 127, 127, 127
};

int sin_table[266] = {
    0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 51, 55, 59, 62, 66, 69, 72, 76, 79, 82, 85, 88, 91, 93, 96, 99, 101, 104, 106, 108, 110, 112, 114, 115, 117, 118, 120, 121, 122, 123, 124, 125, 126, 126, 127, 127, 127, 128, 127, 127, 127, 126, 126, 125, 124, 123, 122, 121, 120, 118, 117, 115, 114, 112, 110, 108, 106, 104, 101, 99, 96, 93, 91, 88, 85, 82, 79, 76, 72, 69, 66, 62, 59, 55, 51, 48, 44, 40, 36, 32, 28, 24, 20, 16, 12, 8, 4, 0, -4, -8, -12, -16, -20, -24, -28, -32, -36, -40, -44, -48, -51, -55, -59, -62, -66, -69, -72, -76, -79, -82, -85, -88, -91, -93, -96, -99, -101, -104, -106, -108, -110, -112, -114, -115, -117, -118, -120, -121, -122, -123, -124, -125, -126, -126, -127, -127, -127, -128, -127, -127, -127, -126, -126, -125, -124, -123, -122, -121, -120, -118, -117, -115, -114, -112, -110, -108, -106, -104, -101, -99, -96, -93, -91, -88, -85, -82, -79, -76, -72, -69, -66, -62, -59, -55, -51, -48, -44, -40, -36, -32, -28, -24, -20, -16, -12, -8, -4
};

void move_forward() {
    int pdir_index1, pdir_frac, pdir_index2;
    int ray_x1, ray_y1, ray_x2, ray_y2;
    int ray_x, ray_y;
    int move_speed;
    int new_ppx, new_ppy;

    // Calculate pdir indexes for interpolation (just like in Raycast)
    pdir_index1 = (pdir - 1) >> 1;    // Equivalent to (pdir - 1) / 2
    pdir_frac = (pdir - 1) & 1;       // 0 if even, 1 if odd
    pdir_index2 = pdir_index1 + 1;

    if (pdir_index2 >= 133) {
        pdir_index2 = 0;  // Wrap around
    }

    // Retrieve precomputed ray directions from lookup tables (scaled by 128)
    ray_x1 = lookup_tables[pdir_index1][200][0];
    ray_y1 = lookup_tables[pdir_index1][200][1];
    ray_x2 = lookup_tables[pdir_index2][200][0];
    ray_y2 = lookup_tables[pdir_index2][200][1];

    // Interpolate between the two directions if necessary
    if (pdir_frac == 0) {
        ray_x = ray_x1;
        ray_y = ray_y1;
    } else {
        ray_x = (ray_x1 + ray_x2) >> 1;
        ray_y = (ray_y1 + ray_y2) >> 1;
    }

    // Adjust the movement speed (scaled by 128)
    move_speed = (128*dt)>>6;  // Define MOVE_SPEED as needed

    // Move the player forward based on the interpolated ray direction
    new_ppx = ppx + (ray_x * move_speed / 128);
    new_ppy = ppy + (ray_y * move_speed / 128);

    // Update player position (you can add collision checks here)
    ppx = new_ppx;
    ppy = new_ppy;
}

void move_backward() {
    int pdir_index1, pdir_frac, pdir_index2;
    int ray_x1, ray_y1, ray_x2, ray_y2;
    int ray_x, ray_y;
    int move_speed;
    int new_ppx, new_ppy;

    // Calculate pdir indexes for interpolation (just like in Raycast)
    pdir_index1 = (pdir - 1) >> 1;    // Equivalent to (pdir - 1) / 2
    pdir_frac = (pdir - 1) & 1;       // 0 if even, 1 if odd
    pdir_index2 = pdir_index1 + 1;

    if (pdir_index2 >= 133) {
        pdir_index2 = 0;  // Wrap around
    }

    // Retrieve precomputed ray directions from lookup tables (scaled by 128)
    ray_x1 = lookup_tables[pdir_index1][200][0];
    ray_y1 = lookup_tables[pdir_index1][200][1];
    ray_x2 = lookup_tables[pdir_index2][200][0];
    ray_y2 = lookup_tables[pdir_index2][200][1];

    // Interpolate between the two directions if necessary
    if (pdir_frac == 0) {
        ray_x = ray_x1;
        ray_y = ray_y1;
    } else {
        ray_x = (ray_x1 + ray_x2) >> 1;
        ray_y = (ray_y1 + ray_y2) >> 1;
    }

    // Adjust the movement speed (scaled by 128)
    move_speed = (128*dt)>>6;  // Define MOVE_SPEED as needed

    // Move the player forward based on the interpolated ray direction
    new_ppx = ppx - (ray_x * move_speed / 128);
    new_ppy = ppy - (ray_y * move_speed / 128);

    // Update player position (you can add collision checks here)
    ppx = new_ppx;
    ppy = new_ppy;
}

#define DIST 10

int32_t lerp(int32_t start, int32_t end, int32_t delta_ms, int32_t duration_ms) {
    int32_t range;
    if (delta_ms <= 0) return start;           // If called too early, return start value
    if (delta_ms >= duration_ms) return end;    // If called too late, return end value

    range = end - start;
    return start + (range * delta_ms / duration_ms);
}

UBYTE* curTex;
void Raycast() {
    int ray;
    UBYTE col;
    int test_x, test_y;
    int step_x, step_y;
    int map_x, map_y, distance;
    UBYTE line_start, line_end, line_height;
    UBYTE hit;
    UBYTE delta_texY;
    int ray_x, ray_y;
    int scaled_screen_height;
    int temp_distance;
    int shift_amount;
    int side,sidecol;
    int prev_map_x, prev_map_y;
    int texX, texY;
    int reciprocal_line_height;
    int y, d;
    UBYTE cc;
    int span_start;
    int span_texY;
    int span_height;
    UBYTE next_cc;
    int horizon_line = SCREEN_HEIGHT >> 1;
    int cell_offset_x, cell_offset_y;
    int pdir_index1, pdir_index2;
    int last_pdir, target_pdir;
    int ray_x1, ray_x2, ray_y1, ray_y2;
    int pdir_frac;
    int line_gap;
    int offcol;
    int x;

    int zpp;
    int radius;

    int hit_x;
    int hit_y;

    int x_ind_hit;
    int y_ind_hit;
    int x_rem_hit;
    int y_rem_hit;

    int texture_x;
    int texture_y;
    UBYTE color;
    UBYTE rc = 4;
    UBYTE fc = 8;

    if (scenedta < 10 && nowscene < 8) {
        ppx = 6<<10;
        ppy = 7<<10;
        pdir = 1;
        last_pdir = 1;
    }
    else if (scenedta < 10 && nowscene >= 8) {
        ppx = 5<<10;
        ppy = 6<<10;
        pdir = 4;
        last_pdir = 4;
    }

    if (nowscene > 8) {
        rc = 12;
        fc = 11;
    }

    if (scenedta > 1000 && scenedta < 3000) {
        move_forward();
    }


    target_pdir = 60;
    last_pdir = pdir;

    if (scenedta >= 3000 && scenedta < 5000) {
        last_pdir = lerp(last_pdir, target_pdir, scenedta - 3000, 5000 - 3000);
        pdir = -last_pdir;
        if (pdir < 0) pdir += 267;  // Wrap around if pdir goes negative
    }

    if (scenedta >= 5500 && scenedta < 9500) {
        target_pdir = 160;
        last_pdir = lerp(last_pdir, target_pdir, scenedta - 5500, 9500 - 5500);
        pdir = last_pdir;
        if (pdir < 0) pdir += 267;  // Wrap around if pdir goes negative
    }

    if (scenedta > 9000 && scenedta < 11500) {
        move_backward();
        target_pdir = 140;
        last_pdir = lerp(last_pdir, target_pdir, scenedta - 9500, 11500 - 9500);
        pdir = last_pdir;
        if (pdir < 0) pdir += 267;  // Wrap around if pdir goes negative
    }

    if (scenedta >= 11500 && scenedta < 14000) {
        move_forward();
        target_pdir = 145;
        last_pdir = lerp(last_pdir, target_pdir, scenedta - 11500, 14000 - 11500);
        pdir = last_pdir;
        if (pdir < 0) pdir += 267;  // Wrap around if pdir goes negative
    }

/*
    if (scenedta > 6500 && scenedta < 10500) {
        pdir+=((dt<<5)/768);
        if (pdir > 266) pdir = 1;
    }

    if (scenedta > 11000 && scenedta < 13500) {
        pdir-=((dt<<5)/768);
        if (pdir <= 0) pdir = 266;
        move_backward();
    }
*/
    line_gap = 16;  // Initial gap between lines

    if (frame == 0) {
        memset(chunkyBuffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH);
     }
    {
        memset(chunkyBuffer, rc, ymul[horizon_line]);
        if (nowscene > 8) {
        memset(chunkyBuffer + ymul[horizon_line], fc, ymul[SCREEN_HEIGHT - horizon_line]);

        for (y = horizon_line; y < SCREEN_HEIGHT; y += line_gap) {
            memset(chunkyBuffer + ymul[y], 0, SCREEN_WIDTH);
            if (line_gap > 1) line_gap--;
        }
        }
    }

    #define TEXTURE_SIZE 32
    #define WALL_HEIGHT 32


    pdir_index1 = (pdir - 1) >> 1;
    pdir_frac = (pdir - 1) & 1;
    pdir_index2 = pdir_index1 + 1;
    if (pdir_index2 >= 133) pdir_index2 = 0;
            ray_x1 = lookup_tables[pdir_index1][0][0];
            ray_y1 = lookup_tables[pdir_index1][0][1];
            ray_x2 = lookup_tables[pdir_index2][0][0];
            ray_y2 = lookup_tables[pdir_index2][0][1];

            if (pdir_frac == 0) {
                ray_x = ray_x1;
                ray_y = ray_y1;
            } else {
                ray_x = (ray_x1 + ray_x2) >> 1;
                ray_y = (ray_y1 + ray_y2) >> 1;
            }
    if (nowscene < 8) {

        // Render floor and ceiling
        for (y = 0; y < 128; y++) {
            for (x = 0; x < SCREEN_WIDTH; x++) {


                texture_x = (x + ray_x) & 31;
                texture_y = (y + ray_y) & 31;
                chunkyBuffer[ymul[128+y] + (x)] = lava[(texture_x<<5)+texture_y];
            }
        }
    }

    // render walls

    for (ray = 0; ray < SCREEN_WIDTH; ray += 2) {
        ray_x1 = lookup_tables[pdir_index1][ray][0];
        ray_y1 = lookup_tables[pdir_index1][ray][1];
        ray_x2 = lookup_tables[pdir_index2][ray][0];
        ray_y2 = lookup_tables[pdir_index2][ray][1];

        if (pdir_frac == 0) {
            ray_x = ray_x1;
            ray_y = ray_y1;
        } else {
            ray_x = (ray_x1 + ray_x2) >> 1;
            ray_y = (ray_y1 + ray_y2) >> 1;
        }

        test_x = ppx;
        test_y = ppy;
        step_x = ray_x;
        step_y = ray_y;
        distance = 0;
        hit = 0;
        side = 0;
        map_x = test_x >> DIST;
        map_y = test_y >> DIST;

        while (!hit && distance < (MAX_DEPTH << 8)) {
            prev_map_x = map_x;
            prev_map_y = map_y;
            test_x += step_x;
            test_y += step_y;
            map_x = test_x >> DIST;
            map_y = test_y >> DIST;
            cell_offset_x = test_x & (1024 - 1);
            cell_offset_y = test_y & (1024 - 1);
            cell_offset_x = step_x >> 31 ? (1024 - cell_offset_x) : cell_offset_x;
            cell_offset_y = step_y >> 31 ? (1024 - cell_offset_y) : cell_offset_y;
            side = (cell_offset_x - cell_offset_y) >> 31 & 1;
            side |= !(world_map[map_y][prev_map_x] == 0) & 1;
            side &= (world_map[prev_map_y][map_x] == 0) | 0;

            if (map_x >= 0 && map_x < MAP_WIDTH && map_y >= 0 && map_y < MAP_HEIGHT) {
                col = world_map[map_y][map_x];
                if (col > 0) 
                {
                    hit = 1;
                    if (col == 1) {
                        curTex = texture_lookup;
                        sidecol = side;
                        offcol = 2;
                    }
                    if (col == 2) {
                        curTex = door_lookup;
                        sidecol = 0;
                        offcol = 0;
                    }

                    if (nowscene > 8) {
                        curTex = wall2_lookup;
                        sidecol = 0;
                        offcol = 0;
                    }
                }
            }

            distance += (1024 >> DIST);
        }

        if (hit) {
            scaled_screen_height = SCREEN_HEIGHT << 4;
            line_height = 0;
            temp_distance = distance;
            shift_amount = 0;

            while ((temp_distance << 1) <= scaled_screen_height) {
                temp_distance <<= 1;
                shift_amount++;
            }
            while (shift_amount >= 0) {
                if (scaled_screen_height >= temp_distance) {
                    scaled_screen_height -= temp_distance;
                    line_height += (1 << shift_amount);
                }
                temp_distance >>= 1;
                shift_amount--;
            }

            line_start = (SCREEN_HEIGHT >> 1) - (line_height >> 1);
            line_end = (SCREEN_HEIGHT >> 1) + (line_height >> 1);
            reciprocal_line_height = reciprocal_table[line_height];
            delta_texY = reciprocal_line_height;
            texY = 0;

            if (side == 0) texX = (test_y >> 5) & 31;
            else texX = (test_x >> 5) & 31;

            y = line_start;
            while (y <= line_end) {
                cc = curTex[((texY >> 7) & 31) << 5 | texX] - offcol;
                if (sidecol == 1) cc += 6;
                span_start = y;
                span_texY = texY;

                while (1) {
                    y++;
                    texY += delta_texY;
                    if (y >= line_end) break;
                    next_cc = curTex[((texY >> 7) & 31) << 5 | texX] - offcol;
                    if (sidecol == 1) next_cc += 6;
                    if (next_cc != cc) break;
                }

                span_height = y - span_start + 2;
                vline(chunkyBuffer + ymul[span_start] + ray, cc << 8 | cc, span_height);
            }
        }
    }

    if (scenedta > 2500 && scenedta < 8500 && nowscene > 7) {
        render_tinytext("code+music visy - gfx kide" , 2, 20, 161);
        render_tinytext("code+music visy - gfx kide" , 3, 21, 31);
    }


    if (scenedta > 1000 && scenedta < 3000) {
        render_text("ACTIVATE", 7, 128, 0);
    }
    if (scenedta > 4000 && scenedta <= 8500 && nowscene < 8) {
        render_text("Delivery", 16, 128, 0);
    }
    if (scenedta > 4000 && scenedta <= 8500 && nowscene >= 8) {
        render_text("offline", 24, 128, 0);
    }
    if (scenedta > 6500 && scenedta < 8500) {
        render_text("   Mode", 16, 128+32, 0);
    }


    if (scenedta > 10500 && scenedta < 16000) {
        render_tinytext("Guess I'll leave it here..." , 0, 20, 161);
        render_tinytext("Guess I'll leave it here..." , 1, 21, 31);
    }

}

void RenderCheckerboard2(UBYTE color1, UBYTE color2) {
    UBYTE x, y;
    UBYTE currentColor;
    int index = 0;
    UBYTE *chunkyBufferPtr = chunkyBuffer;  // Pointer to chunky buffer for faster access
    int diagBlock;
    int lastDiagBlock = -1;
    UBYTE adjustedColor1, adjustedColor2;    
    UBYTE yFrame,xFrame;

    for (y = frame&1; y < 128; y += 2) {  // Handle two rows at a time
        index = ymul[y];  // Start index of the row
        yFrame = y + frame;

        for (x = 0; x < 160; x += 1) {  // Handle two pixels at a time
            xFrame = x+frame;
            diagBlock = diagBlockLookup[xFrame % MAX_FRAME][yFrame % MAX_FRAME];

            // Check if we're in a new diamond block
            if (diagBlock != lastDiagBlock) {
                // Update colors only when we switch to a new diamond block
                adjustedColor1 = color1 + (diagBlock & 0x07);  // Subtle variation for each diamond
                adjustedColor2 = color2 + (diagBlock & 0x07);  // Same for color2
                lastDiagBlock = diagBlock;  // Track the current diamond block
            }

            // Alternating color per diagonal block
            currentColor = (diagBlock & 1) ? adjustedColor2 : adjustedColor1;

            if (shockpic[index] == 64) { 
                // Set the current and next pixel to the same color for two rows
                chunkyBufferPtr[index] = currentColor;
            }
            if (shockpic[index+ymul[128]] == 64) { 
                chunkyBufferPtr[index+ymul[128]] = currentColor;
            }

            // Increment the buffer index directly
            index += 1;
        }
    }
}


void RenderCheckerboard(UBYTE color1, UBYTE color2) {
    UBYTE x, y;
    UBYTE currentColor;
    int index = 0;
    UBYTE *chunkyBufferPtr = chunkyBuffer;  // Pointer to chunky buffer for faster access
    int diagBlock;
    int lastDiagBlock = -1;
    UBYTE adjustedColor1, adjustedColor2;    
    UBYTE yFrame,xFrame;

    for (y = 0; y < 40; y += 2) {  // Handle two rows at a time
        index = ymul[(y << 1)];  // Start index of the row
        yFrame = y + frame;

        for (x = 0; x < 160; x += 1) {  // Handle two pixels at a time
            xFrame = x+frame;
            diagBlock = diagBlockLookup[xFrame % MAX_FRAME][yFrame % MAX_FRAME];

            // Check if we're in a new diamond block
            if (diagBlock != lastDiagBlock) {
                // Update colors only when we switch to a new diamond block
                adjustedColor1 = color1 + (diagBlock & 0x07);  // Subtle variation for each diamond
                adjustedColor2 = color2 + (diagBlock & 0x07);  // Same for color2
                lastDiagBlock = diagBlock;  // Track the current diamond block
            }

            // Alternating color per diagonal block
            currentColor = (diagBlock & 1) ? adjustedColor2 : adjustedColor1;

            // Set the current and next pixel to the same color for two rows
            chunkyBufferPtr[index] = currentColor;
            chunkyBufferPtr[index+160] = currentColor&diagBlock;

            chunkyBufferPtr[ymul[255]-index-1] = currentColor;
            chunkyBufferPtr[ymul[255]-index+160-1] = currentColor&diagBlock;

            // Increment the buffer index directly
            index += 1;
        }
    }
}

void rotrect(int centerX, int centerY, int width, int height, int angle, UBYTE color) {
    int halfWidth = width>>1;
    int halfHeight = height>>1;

    int sinAngle = sine[angle & 0xFF];         // sine value
    int cosAngle = sine[(angle + 64) & 0xFF];  // cosine is sine shifted by 90 degrees (64 positions)
    int x0,y0;
    int x1,y1;
    int x2,y2;
    int x3,y3;

    sinAngle = sinAngle - 128;
    cosAngle = cosAngle - 128;

    x0 = centerX + ((-halfWidth * cosAngle - -halfHeight * sinAngle) >> 8);
    y0 = centerY + ((-halfWidth * sinAngle + -halfHeight * cosAngle) >> 7);

    x1 = centerX + ((halfWidth * cosAngle - -halfHeight * sinAngle) >> 8);
    y1 = centerY + ((halfWidth * sinAngle + -halfHeight * cosAngle) >> 7);
    
    x2 = centerX + ((halfWidth * cosAngle - halfHeight * sinAngle) >> 8);
    y2 = centerY + ((halfWidth * sinAngle + halfHeight * cosAngle) >> 7);

    x3 = centerX + ((-halfWidth * cosAngle - halfHeight * sinAngle) >> 8);
    y3 = centerY + ((-halfWidth * sinAngle + halfHeight * cosAngle) >> 7);

    drawcolor = 16+color;
    line(x0, y0, x1, y1);
    line(x1, y1, x2, y2);
    line(x2, y2, x3, y3);
    line(x3, y3, x0, y0);
}

 
void startup() {
    timer_msgport = CreateMsgPort();
    timer_ioreq = CreateIORequest(timer_msgport, sizeof(*timer_ioreq));

    OpenDevice("timer.device", UNIT_VBLANK, (APTR) timer_ioreq, 0);
    TimerBase = (APTR) timer_ioreq->tr_node.io_Device;
    GetSysTime(&startTime);
}

static void closetimer(void){
    if (TimerBase){
        CloseDevice((APTR) timer_ioreq);
    }
    DeleteIORequest(timer_ioreq);
    DeleteMsgPort(timer_msgport);
    TimerBase = 0;
    timer_ioreq = 0;
    timer_msgport = 0;
}
 
ULONG getMilliseconds() {
    struct timeval endTime;
 
    GetSysTime(&endTime);
    SubTime(&endTime, &startTime);

    return (endTime.tv_secs * 1000 + endTime.tv_micro / 1000);
}

static ULONG App_GetVBR(void)
{
    // VBR is 0 on 68000, supervisor register on 68010+.
    ULONG   vbr = 0;
    LONG    userstack;

    if (SysBase->AttnFlags & (1U << AFB_68010))
    {
        // enter supervisor mode and get user stack
        userstack = SuperState();

        vbr = mt_get_vbr();     

        // restore user stack
        UserState((char*)userstack);
    }

    return vbr;
}

UBYTE* LoadFile(const char* filename, ULONG mem_type)
{
    BPTR file_ptr;
    LONG size;
    UBYTE* data = NULL;

    if ((file_ptr = Open(filename, MODE_OLDFILE)))
    {
        Seek(file_ptr, 0, OFFSET_END);
        size = Seek(file_ptr, 0, OFFSET_BEGINNING);
        data = (UBYTE*)AllocVec(size, mem_type);
        if (data)
        {
            Read(file_ptr, data, size);
        }
        Close(file_ptr);
    }
    
    return data;
}

void OnExit() {
    mt_mastervol(&custom, 0);
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();
    WaitTOF();

    mt_Enable = 0;
    mt_end(&custom);
    mt_remove_cia(&custom);

    FreeVec(flypic);
    FreeVec(tinyfont);
    FreeVec(shockpic);
    FreeVec(lava);
    FreeVec(listrippic);
    FreeVec(gpspic);
    FreeVec(gps2pic);
    FreeVec(title);
    FreeVec(door);
    FreeVec(wall2);
    FreeVec(sideflypic);
    FreeVec(sidefly2pic);
    FreeVec(chei);
    FreeVec(cpic2);
    FreeVec(moddata);
    FreeVec(chunkyBuffer);

    CloseScreen(mainScreen1);
    WaitTOF();

    closetimer();

    if (UtilityBase)  { CloseLibrary(UtilityBase); }

    FreeBitMap(mainBitmap1);
    // restore mouse
    my_wbscreen_ptr = LockPubScreen("Workbench");
    ClearPointer(my_wbscreen_ptr->FirstWindow);
    UnlockPubScreen(NULL, my_wbscreen_ptr);
    FreeVec(emptyPointer);    
}

int main(void) {
    UWORD oldDMA;
    UBYTE o;
    int i,ii,j = 0;
    BPTR file_ptr;
    ULONG size;

    printf(quadansi);
    printf("\e[0m\n");

    for(i=0;i<128;i++) {
        WaitTOF();
    }

    // load and initialize everything

    SysBase = *((struct ExecBase **)4);

    UtilityBase = (struct Library*)OpenLibrary("utility.library",0L);

    if (UtilityBase)
    {
    }
    else {
        Printf("failed to open utility.library!\n");
        OnExit();
        exit(1);        
    }

    // hide mouse
    emptyPointer = AllocVec(22 * sizeof(UWORD), MEMF_CHIP | MEMF_CLEAR);
    my_wbscreen_ptr = LockPubScreen("Workbench");
    SetPointer(my_wbscreen_ptr->FirstWindow, emptyPointer, 8, 8, -6, 0);
    UnlockPubScreen(NULL, my_wbscreen_ptr);
    
    if (!initScreen(&mainBitmap1, &mainScreen1)) {
        OnExit();
        exit(1);
    }

    LoadRGB32(&(mainScreen1->ViewPort), blackpal);

    bufferSelector = TRUE;
    currentScreen = mainScreen1;
    currentBitmap = mainBitmap1;

    c2p2x1_8_c5_030_init(160,256,0,0,320/8,(LONG)currentBitmap->BytesPerRow);

    chunkyBuffer = AllocVec(160 * 256 * sizeof(UBYTE), MEMF_FAST | MEMF_CLEAR);
//    CopyMemQuick(noitapic, chunkyBuffer, 320*256);

    moddata = LoadFile("esa3.mod", MEMF_CHIP);

    chei = LoadFile("chei.raw", MEMF_CHIP);
    cpic2 = LoadFile("cpic2.raw", MEMF_CHIP);
    for (ii=0;ii<256;ii++) {
        for (i=0;i<256;i++) {
            if (cpic2[(ii<<8)+i] == 0x00) {
                cpic2[(ii<<8)+i] = 0x03;
            }
        }
    }

    sidefly2pic = LoadTarga("sidefly2.tga", 2);
    if (sidefly2pic == NULL) {
        Printf("failed to load sidefly2.tga\n");
        OnExit();
        exit(1);
    }

    shockpic = LoadTarga("shock.tga", 4);
    if (shockpic == NULL) {
        Printf("failed to load shock.tga\n");
        OnExit();
        exit(1);
    }

    sideflypic = LoadTarga("sidefly.tga", 5);
    if (sideflypic == NULL) {
        Printf("failed to load sidefly.tga\n");
        OnExit();
        exit(1);
    }

    flypic = LoadTarga("fly2pic.tga",6);
    if (flypic == NULL) {
        Printf("failed to load fly2pic.tga\n");
        OnExit();
        exit(1);
    }

    listrippic = LoadTarga("listrip.tga",7);
    if (listrippic == NULL) {
        Printf("failed to load listrip.tga\n");
        OnExit();
        exit(1);
    }


    gps2pic = LoadTarga("gps2.tga",9);
    if (gps2pic == NULL) {
        Printf("failed to load gps2.tga\n");
        OnExit();
        exit(1);
    }

    door = LoadTarga("door.tga",10);
    if (door == NULL) {
        Printf("failed to load door.tga\n");
        OnExit();
        exit(1);
    }

    witch = LoadTarga("witch.tga",11);
    if (witch == NULL) {
        Printf("failed to load witch.tga\n");
        OnExit();
        exit(1);
    }

    wall2 = LoadTarga("wall2.tga",12);
    if (wall2 == NULL) {
        Printf("failed to load door.tga\n");
        OnExit();
        exit(1);
    }

    gpspic = LoadTarga("gps.tga",13);
    if (gpspic == NULL) {
        Printf("failed to load gps.tga\n");
        OnExit();
        exit(1);
    }


    tinyfont = LoadTarga("tinyfont.tga",14);
    if (tinyfont == NULL) {
        Printf("failed to load tinyfont.tga\n");
        OnExit();
        exit(1);
    }


    init_lookup_tables();

    title = LoadTarga("title.tga",15);
    if (title == NULL) {
        Printf("failed to load title.tga\n");
        OnExit();
        exit(1);
    }
    
    ScreenToFront(currentScreen);

    if (nowscene == 0) {
        LoadRGB32(&(mainScreen1->ViewPort), custompal);
        WaitTOF();
        DrawPic(title,0,0,160,256);
        c2p2x1_8_c5_030(chunkyBuffer, currentBitmap->Planes[0]);
        for(i=0;i<128;i++) {
            WaitTOF();
        }

        for(ii=0;ii<256;ii++) {
            for(j=239;j<=255;j++) {
                if (custompal[1+(j*3)+0] > 0) custompal[1+(j*3)+0]-=0x01000000;
                if (custompal[1+(j*3)+1] > 0) custompal[1+(j*3)+1]-=0x01000000;
                if (custompal[1+(j*3)+2] > 0) custompal[1+(j*3)+2]-=0x01000000;
            }
            LoadRGB32(&(mainScreen1->ViewPort), custompal);

            o = sine[(ii<<3)&1023]>>3;
            for(i=0;i<160;i+=8) {
                chunkyBuffer[ymul[ii]+i+o]|=chunkyBuffer[ymul[ii]+i+1+o];
                chunkyBuffer[ymul[ii]+i+3+o]|=chunkyBuffer[ymul[ii]+i+2+o];
            }
            c2p2x1_8_c5_030(chunkyBuffer, currentBitmap->Planes[0]);
            WaitTOF();
        }
    }

    LoadRGB32(&(mainScreen1->ViewPort), blackpal);

    lava = LoadTarga("lava.tga", 15);
    if (lava == NULL) {
        Printf("failed to load lava.tga\n");
        OnExit();
        exit(1);
    }

    mt_install_cia(&custom, (APTR)App_GetVBR(), 1);
    mt_init(&custom, moddata, NULL, 0);
    mt_mastervol(&custom, 0x40);
    mt_Enable = 1;

    // ready to go to mainloop

    startup();

    LoadRGB32(&(mainScreen1->ViewPort), custompal);

    MainLoop();

    // exit routine follows

    OnExit();
    exit(RETURN_OK);
    
}

UBYTE myp = 0;
UBYTE myx = 0;
UBYTE lok = 0;
void HeightMap() 
{
    int x,y,z;
    int off = 0;
    UBYTE off2,off3;
    int height = 500;
    int to,tomul = 0;
    UBYTE px,py=0;
    UBYTE bil = 0;
    UBYTE cc;
    UBYTE raja;
    UBYTE raja2;
    UBYTE alku = 92-((dta-alkudta)>>5);
    UBYTE* destPtr;
    UBYTE drawcolor2;

    int xoff = 0;
    UBYTE lh = 0;
    UBYTE tx,ty,zs = 0;

    if (lok == 1) alku = 0;

    if (alku <= 0 || alku > 200) { alku = 0; lok = 1; }

    raja = ((dta-alkudta)/1000);
    if (raja < 1) raja = 1;
    if (raja > 4) raja = 4;

    raja2 = ((dta-alkudta)/2000);
    if (raja2 < 1) raja2 = 1;
    if (raja2 > 4) raja2 = 4;

    if (frame == 0) {
        drawcolor = 10;
        fillrect(0,0,160,94);
    }

    myp-=(dt*raja)>>6;
    myx+=(dt*raja2)>>6;
    py = myp;
    px = myx;
    off2 = alku+(sine[(64+(dta>>3))&0xff]>>5);
    off3 = sine[(dta>>3)&0xff]>>5;

    for(x=0;x<65;x++)
    {
        memset(chunkyBuffer+(33*160)+ymul[x]+22,10,65);
    }

    memset(chunkyBuffer+160*94, 10, 160*12);

    for (z = 46;z > 11;z-=1) {
        
        ty = -z+py;

        tomul = ty<<8;

        for(x=0;x<160;x+=4) {
            tx = (zmul[x<<1][z >> 1] >> 5) + px;
            to = tomul+tx;
            off = (zdiv[(height - chei[to]) - 1][z - 1] << 2) + 64;
            drawcolor = (cpic2[to]);
            drawcolor2 = (cpic2[to-1]);
            xoff = ymul[off]+x; 
            vline(chunkyBuffer+xoff, (drawcolor2 << 8) | drawcolor, ((off+32)>>4)+1);

            tx = (zmul[(x<<1)+2][z >> 1] >> 5) + px;
            to = tomul+tx;
            off = (zdiv[(height - chei[to]) - 1][z - 1] << 2) + 64;
            drawcolor = (cpic2[to]);
            drawcolor2 = (cpic2[to-1]);
            xoff = ymul[off]+x+2; 
            vline(chunkyBuffer+xoff, (drawcolor2 << 8) | drawcolor, ((off+32)>>4)+1);

        }
    }


    // stretch last row to screen edge
    z = 11;
    ty = -z+py;
    tomul = ty<<8;

    for(x=0;x<160;x+=2) {
        tx = (zmul[x<<1][z >> 1] >> 5) + px;
        to = tomul+tx;
        off = (zdiv[(height - chei[to]) - 1][z - 1] << 2) + 64;
        xoff = ymul[off]+x; 
        lh = ((off+32)>>4)+1;
        lh = 255-lh;
        vline(chunkyBuffer+xoff, 0, lh);
    }

    for (y = 0; y < 128; y++) {
        destPtr = &chunkyBuffer[ymul[y + 26 + off2] - 20 + off3]; // Pre-calculate the destination pointer base
        for (x = 31; x < 31+68; x++) {
            cc = flypic[(y << 7) + (x)]; // Access the source pixel
            destPtr[x] = (cc != 96) ? cc : destPtr[x]; // Use a ternary operator to avoid an explicit if statement
        }
    }

}

void Lines()
{
    int centerX = 56+(sine[(dta>>3)&0xff]>>5);
    int centerY = 127;
    int width = 160+(sine[(dta>>3)&0xff]>>3);
    int height = 160+(sine[(dta>>3)&0xff]>>3);
    int i = 0;
    int linec = (scenedta-5000)>>7;

    if (linec >= 24) {
        linec = 24;
    }
    if (frame == 0) 
    {
        memset(chunkyBuffer,0,160*256);
    }

    if (scenedta > 6500) {
        for(i=frame%4; i < 160<<8; i+=4) {
            if (chunkyBuffer[i] > 0) {
                chunkyBuffer[i]-=1;
                chunkyBuffer[i]&=chunkyBuffer[i-1];
            }
        }
    }
    if (scenedta >= 5000)
    {
        for (i = 24; i > 24-linec; i--) {
            rotrect(centerX, centerY, width-(i<<3), height-(i<<3), ((scenedta>>4)+(i<<2))&0xff, i%15);
        }

        if (scenedta > 7500) {

            render_text("Magic", centerX-50, 40, 1);
        }
        if (scenedta > 8500) {
            render_text("Circle", centerX-50, 230, 1);
        }

        if (scenedta > 9000 && scenedta <= 10000) {
            render_tinytext("Same Day" , 0, 120, 29);
        }

        if (scenedta > 10000 && scenedta <= 11000) {
            render_tinytext("Same Day Guaranteed" , 0, 120, 30);
        }

        if (scenedta > 11000 && scenedta <= 12000) {
            render_tinytext("Same Day Guaranteed" , 0, 120, 29);
            render_tinytext(" Dungeon" , 0, 130, 30);

        }

        if (scenedta > 12000) {
            render_tinytext("Same Day Guaranteed" , 0, 120, 31);
            render_tinytext(" Dungeon Delivery" , 0, 130, 31);

        }

    }

    if (scenedta < 3600) {
        DrawPicT(gps2pic,48,0,160,256);
        if (scenedta > 1600) {
        render_tinytext("Witchfinder" , 0, 60, 160);
        }
        if (scenedta > 2400) {
        render_tinytext("Logging in..." , 0, 73, (frame>>1)%8);
        }


    }

}


void Sidefly() {

    if (frame == 0) {
        memset(chunkyBuffer, 0, 160*256);
    }

    if (nowscene <= 6) {
        RenderCheckerboard(0x03, 0x08);
        DrawPic(sideflypic, -64+(scenedta>>5),128-40+(sine[(scenedta>>4)&255]>>4),64,64);
        render_tinytext("Standard takeoff procedure" , 2, 238, 0);
        render_tinytext("Standard takeoff procedure" , 3, 239, 31);
    } else {
        RenderCheckerboard(0x16, 0x32);
        DrawPic(sidefly2pic, (160)-(scenedta>>5),128-40+(sine[(scenedta>>4)&255]>>4),64,64);
        render_tinytext("Landing gears are engaged" , 5, 238, 0);
        render_tinytext("Landing gears are engaged" , 6, 239, 31);
    }
}
void Shock() {
    int x,y;
    int picop;
    int raj = 0;
    if (frame == 0) {
        DrawPic(shockpic,0,0,160,256);

    }

    if (scenedta > 5500) {
        RenderCheckerboard2(32+((frame>>1)&7), 33+((frame>>1)&7));
    }

    if (scenedta > 4500) {
        raj = (scenedta-4500)>>7;
        if (raj > 36) raj = 36;
        for (y=0;y<raj;y++) {
            for (x=0;x<160;x++) {
                picop = y*45+((x+(frame<<1))%45);

                if (shockpic[ymul[(y<<1)+96]+x] == 64) { 
                    chunkyBuffer[ymul[(y<<1)+96]+x] = listrippic[picop];
                    chunkyBuffer[ymul[(y<<1)+97]+x] = listrippic[picop];
                }
            }
        }

    }

    if (scenedta >= 5000) {
        render_tinytext_d("review */***** not arrived" , 2, 20, 0,scenedta-5000);
        render_tinytext_d("review */***** not arrived" , 3, 21, 31,scenedta-5000);
    }

}

void Blank() {
    if (frame == 0) {
        memset(chunkyBuffer, 0, 160*256);
    }

}

int textindex = 0;

int t = 0;

void Intro() {
    int i = 0;

    if (frame == 0) {
        memset(chunkyBuffer, 11*16, 160*256);

    }
    i = ((scenedta-2500)>>5);
    if (i < 0) i = 0;
    if (i >= 54) i = 54;

    DrawPicV(witch,0,0,160,202+i);
}

void Gps()
{
    int x,y,picop,i,kk,aa;
    if (frame == 0) {
        DrawPic(gpspic, 0,0,160,256);
    }

    for (i=0;i < 3; i++) {
        kk = i*37;
        for (y=frame%2;y<36;y+=1) {
            aa = y*45;
            for (x=0;x<160;x+=1) {
                picop = (aa)+((x+((i<<2)+frame<<2))%45);

                if (gpspic[ymul[((y+(kk))<<1)]+x] == 208) { 
                    chunkyBuffer[ymul[((y+(kk))<<1)]+x] = listrippic[picop]<<i;
                    chunkyBuffer[ymul[((y+(kk))<<1)+1]+x] = listrippic[picop]<<i;
                }
            }

        }
    }
}


DrawFunc DrawFuncs[12] = {Blank, Blank, Intro, Lines, Sidefly, HeightMap, Raycast, Sidefly, Shock, Gps, Raycast, Blank};


#define KEY_EXIT          0x01  // Q or ESC
#define KEY_RELOAD        0x02  // R
#define KEY_MOVE_FORWARD  0x04  // W or Up Arrow
#define KEY_MOVE_BACKWARD 0x08  // S or Down Arrow
#define KEY_STRAFE_LEFT    0x10 // A or Left Arrow
#define KEY_STRAFE_RIGHT   0x20 // D or Right Arrow
#define KEY_ROT_LEFT       0x40 // Rotate Left (Left Arrow)
#define KEY_ROT_RIGHT      0x80 // Rotate Right (Right Arrow)
#define KEY_SPECIAL_2B     0x100 // Special Key with Key Code $2B
#define KEY_SPECIAL_30     0x200 // Special Key with Key Code $30

static volatile UBYTE key_buffer = 0x80;
int keys()
{
    UBYTE keycode = 0;
    int i, zz = 0;
    keycode = ~ciaa.ciasdr;
    ciaa.ciacra |= 0x40;
    /* wait until 85 us have expired */
    for(i = 0; i < 256; i++) {zz = zz + 1;}
    /* switch CIA serial port to input mode */
    ciaa.ciacra &= ~0x40;

    key_buffer = (keycode >> 1) | (keycode << 7); /* ROR 1 */

    // Q or ESC -> exit
    if (key_buffer == 0x10 ||key_buffer == 0x45) {
        key_buffer = 0;
        return 1;
    }

    // R: reload
    if (key_buffer == 0x13) {
        key_buffer = 0;
        return 2;
    }

    // W: forward
    if (key_buffer == 0x11) {
        key_buffer = 0;
        return 3;
    }

    // A
    if (key_buffer == 0x20) {
        key_buffer = 0;
        return 4;
    }

    // D
    if (key_buffer == 0x22) {
        key_buffer = 0;
        return 5;
    }

    key_buffer = 0;
    return 0;
}

const char* numberToText(int num) {
    // Array holding the text representation of numbers from 0 to 9
    const char* numbers[] = { "Zero", "One", "Two", "Three", "Four", "Five", 
                              "Six", "Seven", "Eight", "Nine" };
    
    // Check if the number is in the valid range (0 to 9)
    if (num >= 0 && num <= 9) {
        return numbers[num];
    } else {
        return "Invalid number"; // For numbers outside the range
    }
}

char textbuffer[128];

void MainLoop() {
    int key = 0;
    int ray_x,ray_y,move_speed,new_ppx,new_ppy;
    while (ex == 0) {
        key = keys();
        if (key == 1) {
            ex = 1;
        }

        if (key == 3) {
            move_forward();
               
        }

        if (key == 4) {
            pdir-=2;

            if (pdir <= 0) pdir = 266;
        }

        if (key == 5) {
            pdir+=2;
            if (pdir > 266) pdir = 1;
        }

        
        if (mouseCiaStatus()) 
        {
            ex = 1;
        }
        st = getMilliseconds();
        if (et != 0) {
            dt = (st-et);
        }
        et = st;
        dta = dta + dt;
        t = dta;

        scenedta = scenedta + dt;
        scene = mt_E8Trigger;

        if (scene == 8) {
            ex = 1;
        }

        if (oldscene != scene) {
            frame = 0;
            scenedta = 0;
            alkudta = dta;
            nowscene++;
        }

        oldscene = scene;
        DrawFuncs[nowscene]();


///////////////// DEBUG
//        snprintf(textbuffer, sizeof(textbuffer), "sct: %d", scenedta);

//        memset(chunkyBuffer,0,160*10);
//        render_tinytext(textbuffer, 0,0, 69);
///////////////// DEBUG
        c2p2x1_8_c5_030(chunkyBuffer, currentBitmap->Planes[0]);

        WaitTOF();
        frame++;
        totalframes++;

    }
}

BOOL initScreen(struct BitMap **b, struct Screen **s)
{
    // load onscreen bitmap which will be shown on screen
    *b = AllocBitMap(320, 256,
                     8, BMF_CLEAR|BMF_DISPLAYABLE,
                     NULL);
    if (!*b) {
        printf("Error: Could not allocate memory for screen bitmap\n");
        goto __exit_init_error;
    }

    *s = createScreen(*b, TRUE, 0, 0,
                      320, 256,
                      8, NULL);
    if (!*s) {
        printf("Error: Could not allocate memory for screen\n");
        goto __exit_init_bitmap;
    }
    return TRUE;

__exit_init_bitmap:
    FreeBitMap(*b);
__exit_init_error:
    return FALSE;
}

UBYTE  sine[256] = {
  0x80, 0x83, 0x86, 0x89, 0x8C, 0x90, 0x93, 0x96,
  0x99, 0x9C, 0x9F, 0xA2, 0xA5, 0xA8, 0xAB, 0xAE,
  0xB1, 0xB3, 0xB6, 0xB9, 0xBC, 0xBF, 0xC1, 0xC4,
  0xC7, 0xC9, 0xCC, 0xCE, 0xD1, 0xD3, 0xD5, 0xD8,
  0xDA, 0xDC, 0xDE, 0xE0, 0xE2, 0xE4, 0xE6, 0xE8,
  0xEA, 0xEB, 0xED, 0xEF, 0xF0, 0xF1, 0xF3, 0xF4,
  0xF5, 0xF6, 0xF8, 0xF9, 0xFA, 0xFA, 0xFB, 0xFC,
  0xFD, 0xFD, 0xFE, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFD,
  0xFD, 0xFC, 0xFB, 0xFA, 0xFA, 0xF9, 0xF8, 0xF6,
  0xF5, 0xF4, 0xF3, 0xF1, 0xF0, 0xEF, 0xED, 0xEB,
  0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC,
  0xDA, 0xD8, 0xD5, 0xD3, 0xD1, 0xCE, 0xCC, 0xC9,
  0xC7, 0xC4, 0xC1, 0xBF, 0xBC, 0xB9, 0xB6, 0xB3,
  0xB1, 0xAE, 0xAB, 0xA8, 0xA5, 0xA2, 0x9F, 0x9C,
  0x99, 0x96, 0x93, 0x90, 0x8C, 0x89, 0x86, 0x83,
  0x80, 0x7D, 0x7A, 0x77, 0x74, 0x70, 0x6D, 0x6A,
  0x67, 0x64, 0x61, 0x5E, 0x5B, 0x58, 0x55, 0x52,
  0x4F, 0x4D, 0x4A, 0x47, 0x44, 0x41, 0x3F, 0x3C,
  0x39, 0x37, 0x34, 0x32, 0x2F, 0x2D, 0x2B, 0x28,
  0x26, 0x24, 0x22, 0x20, 0x1E, 0x1C, 0x1A, 0x18,
  0x16, 0x15, 0x13, 0x11, 0x10, 0x0F, 0x0D, 0x0C,
  0x0B, 0x0A, 0x08, 0x07, 0x06, 0x06, 0x05, 0x04,
  0x03, 0x03, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03,
  0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x0A,
  0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11, 0x13, 0x15,
  0x16, 0x18, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24,
  0x26, 0x28, 0x2B, 0x2D, 0x2F, 0x32, 0x34, 0x37,
  0x39, 0x3C, 0x3F, 0x41, 0x44, 0x47, 0x4A, 0x4D,
  0x4F, 0x52, 0x55, 0x58, 0x5B, 0x5E, 0x61, 0x64,
  0x67, 0x6A, 0x6D, 0x70, 0x74, 0x77, 0x7A, 0x7D
};
