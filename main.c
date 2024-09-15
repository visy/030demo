// adapted from example blog tutorial by Christian Ammann

#include <clib/timer_protos.h>
#include <clib/exec_protos.h>

#include "demo.h"

#define LUA_IMPL
#include "minilua.h"

#define __builtin_expect(x,y)

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

typedef void (*DrawFunc)(void);

int scene = 0;
int oldscene = 0;
int ex = 0;

ULONG st,et;
UBYTE dt = 0;
UWORD dta;

// gfx buffers for allocating from file

UBYTE* cpic2;
UBYTE* chei;
UBYTE* flypic;

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

// 256 color pal
UWORD cpicpal[] =
{
    0x0000,0x0453,0x0432,0x0764,0x0875,0x0776,0x0573,0x0556,0x0688,0x0362,
    0x0245,0x0379,0x0abb,0x0bbc,0x0331,0x0130,
    // 16-color grayscale (like your example)
    0x0000, 0x0111, 0x0222, 0x0333, 0x0444, 0x0555, 0x0666, 0x0777, 
    0x0888, 0x0999, 0x0AAA, 0x0BBB, 0x0CCC, 0x0DDD, 0x0EEE, 0x0FFF,

    // 16-color red gradient
    0x0000, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 
    0x8000, 0x9000, 0xA000, 0xB000, 0xC000, 0xD000, 0xE000, 0xF000,

    // 16-color green gradient
    0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700, 
    0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00,

    // 16-color blue gradient
    0x0000, 0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 
    0x0080, 0x0090, 0x00A0, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0,

    // 16-color sunset palette
    0x1000, 0x3100, 0x5200, 0x7400, 0x9500, 0xB600, 0xD700, 0xF800,
    0xF820, 0xFA40, 0xFC60, 0xFE80, 0xFFA0, 0xFFC0, 0xFFE0, 0xFFF0,

    // 16-color earth tones
    0x2000, 0x3100, 0x4220, 0x5330, 0x6440, 0x7550, 0x8660, 0x9770, 
    0xA880, 0xB990, 0xCAA0, 0xDBB0, 0xECC0, 0xFDD0, 0xFFEA, 0xFFFA,

    // 16-color ocean blues
    0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 0x1080, 
    0x2090, 0x30A0, 0x40B0, 0x50C0, 0x60D0, 0x70E0, 0x80F0, 0x90FF,

    // 16-color purple-pink gradient
    0x1000, 0x1100, 0x2200, 0x3300, 0x4400, 0x5500, 0x6600, 0x7700, 
    0x8800, 0x9900, 0xAA00, 0xBB00, 0xCC00, 0xDD00, 0xEE00, 0xFF00,

        0x0000, 0x0111, 0x0222, 0x0333, 0x0444, 0x0555, 0x0666, 0x0777, 
    0x0888, 0x0999, 0x0AAA, 0x0BBB, 0x0CCC, 0x0DDD, 0x0EEE, 0x0FFF,

    // 16-color red gradient
    0x0000, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 
    0x8000, 0x9000, 0xA000, 0xB000, 0xC000, 0xD000, 0xE000, 0xF000,

    // 16-color green gradient
    0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700, 
    0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00,

    // 16-color blue gradient
    0x0000, 0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 
    0x0080, 0x0090, 0x00A0, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0,

    // 16-color sunset palette
    0x1000, 0x3100, 0x5200, 0x7400, 0x9500, 0xB600, 0xD700, 0xF800,
    0xF820, 0xFA40, 0xFC60, 0xFE80, 0xFFA0, 0xFFC0, 0xFFE0, 0xFFF0,

    // 16-color earth tones
    0x2000, 0x3100, 0x4220, 0x5330, 0x6440, 0x7550, 0x8660, 0x9770, 
    0xA880, 0xB990, 0xCAA0, 0xDBB0, 0xECC0, 0xFDD0, 0xFFEA, 0xFFFA,

    // 16-color ocean blues
    0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 0x1080, 
    0x2090, 0x30A0, 0x40B0, 0x50C0, 0x60D0, 0x70E0, 0x80F0, 0x90FF,

    // 16-color purple-pink gradient
    0x1000, 0x1100, 0x2200, 0x3300, 0x4400, 0x5500, 0x6600, 0x7700, 
    0x8800, 0x9900, 0xAA00, 0xBB00, 0xCC00, 0xDD00, 0xEE00, 0xFF00   
};

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

// current active palette
ULONG *currentPal;

// chunky buffer for pixels
UBYTE *chunkyBuffer;

int frame = 0;
int totalframes = 0;

UBYTE sine[];
int zmul[512][512];
int zdiv[512][512];
int zmod[512][512];

int ymul[256] = 
{
    0
};

UBYTE drawcolor = 7;

ULONG millis;
ULONG st;

struct Device* TimerBase;
static struct IORequest timereq;

lua_State *L;

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
    bool yLonger=false;
    int shortLen=y2-y;
    int longLen=x2-x;
    int j;
    int decInc;

    if (abs(shortLen)>abs(longLen)) {
        int swap=shortLen;
        shortLen=longLen;
        longLen=swap;               
        yLonger=true;
    }
    if (longLen==0) decInc=0;
    else decInc = (shortLen << 8) / longLen;

    if (yLonger) {
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


int ppx = 8;
int ppy = 8;
int pdir = 1; // up, down, left, right


UBYTE texture[16 * 16] = 
    {7, 7, 7, 7, 7, 7, 7, 6, 6, 7, 7, 7, 7, 7, 7, 7,
    1, 1, 1, 1, 1, 1, 6, 6, 5, 5, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 6, 6, 5, 5, 5, 5, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 6, 6, 5, 5, 5, 5, 5, 5, 1, 1, 1, 1,
    1, 1, 1, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 1,
    1, 1, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1,
    1, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1,
    6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6,
    6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6,
    1, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1,
    1, 1, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1,
    1, 1, 1, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 1,
    1, 1, 1, 1, 6, 6, 5, 5, 5, 5, 5, 5, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 6, 6, 5, 5, 5, 5, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 6, 6, 5, 5, 1, 1, 1, 1, 1, 1,
    7, 7, 7, 7, 7, 7, 7, 6, 6, 7, 7, 7, 7, 7, 7, 7
};

UBYTE world_map[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,3,3,3,3,0,0,0,0,0,0,0,0,0,1},
    {1,0,3,3,3,3,3,0,0,0,0,5,5,0,0,1},
    {1,0,3,3,3,3,0,0,0,0,0,5,5,0,0,1},
    {1,0,6,6,6,6,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,6,6,6,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,4,2,4,6,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,2,0,0,0,0,0,2,7,0,0,7,2},
    {1,0,4,0,0,0,0,0,0,0,7,0,0,0,0,7},
    {1,0,2,0,2,0,0,0,0,0,2,0,0,0,0,2},
    {1,0,4,2,4,0,0,0,0,0,7,0,0,0,0,7},
    {1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,2},
    {1,0,0,0,0,0,0,0,0,0,7,0,0,0,0,7},
    {1,1,1,1,1,1,1,1,1,1,7,2,7,2,7,2},
};

struct Library *MathIeeeDoubBasBase;
struct Library *MathIeeeDoubTransBase;
struct Library *MathIeeeSingBasBase;
struct Library *MathIeeeSingTransBase;
struct Library *UtilityBase;


void init_lookup_tables() {
    int i, ii;

    for(i=0; i < 256; i++) {
        ymul[i] = i * 160;
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

}

void Raycast() {
    UBYTE ray;
    UBYTE col;
    int test_x, test_y;
    register int step_x;
    register int step_y;
    int map_x, map_y, distance;
    UBYTE line_start, line_end, line_height;
    UBYTE hit = 0;
    UBYTE delta_texY;
    register int ray_x;
    register int ray_y;
    int scaled_screen_height = 0;  // Pre-multiplied value for screen height scaling
    int temp_distance = 0;
    int shift_amount = 0;
    int side;  // 0 = vertical wall, 1 = horizontal wall
    int prev_map_x, prev_map_y;
    int texX, texY;
    int reciprocal_line_height;
    int y, d;
    UBYTE cc;
    int span_start;
    int span_texY;
    int span_height;
    UBYTE next_cc;
    int horizon_line;
    int cell_offset_x, cell_offset_y;
        // Variables for pdir interpolation
    int pdir_index1, pdir_index2;
    int ray_x1, ray_x2, ray_y1, ray_y2;
    int pdir_frac;
    int line_gap = 16;  // Initial gap between lines

    // Update player direction (assuming pdir ranges from 1 to 133)
    pdir++;
    if (pdir > 266) {
        pdir = 1;
    }

    // Clear or update the screen buffer as needed
    if (frame == 0) {
        memset(chunkyBuffer, 4, SCREEN_HEIGHT * SCREEN_WIDTH);
    } else {
            horizon_line = SCREEN_HEIGHT >> 1;

            // Fill ceiling
            memset(chunkyBuffer, 4, ymul[horizon_line]);

            // Fill floor
            memset(chunkyBuffer + ymul[horizon_line], 8, ymul[(SCREEN_HEIGHT - horizon_line)]);

            for (y = horizon_line; y<SCREEN_HEIGHT; y+=line_gap) {
                memset(chunkyBuffer + ymul[y], 0, SCREEN_WIDTH);

                if (line_gap > 1) {
                    line_gap -= 1;  // Reduce gap size gradually to create perspective effect
                }
            }
    }

    for (ray = 0; ray < SCREEN_WIDTH; ray += 2) {
        pdir_index1 = (pdir - 1) >> 1;     // Equivalent to (pdir - 1) / 2
        pdir_frac = (pdir - 1) & 1;        // 0 if even, 1 if odd
        pdir_index2 = pdir_index1 + 1;
        if (pdir_index2 >= 133) {
            pdir_index2 = 0;  // Wrap around
        }
        // Retrieve precomputed ray directions
        ray_x1 = lookup_tables[pdir_index1][ray][0];
        ray_y1 = lookup_tables[pdir_index1][ray][1];
        ray_x2 = lookup_tables[pdir_index2][ray][0];
        ray_y2 = lookup_tables[pdir_index2][ray][1];

        if (pdir_frac == 0) {
            // No interpolation needed
            ray_x = ray_x1;
            ray_y = ray_y1;
        } else {
            // Interpolate between ray_x1 and ray_x2
            ray_x = (ray_x1 + ray_x2) >> 1;
            ray_y = (ray_y1 + ray_y2) >> 1;
        }

        // Initialize variables
        test_x = ppx << 10;  // Player's position scaled
        test_y = ppy << 10;
        step_x = ray_x;
        step_y = ray_y;
        distance = 0;
        hit = 0;
        side = 0;

        // Compute initial map positions
        map_x = test_x >> 10;
        map_y = test_y >> 10;

        while (!hit && distance < (MAX_DEPTH << 8)) {
            // Store previous map positions
            prev_map_x = map_x;
            prev_map_y = map_y;

            // Incrementally update test_x and test_y
            test_x += step_x;
            test_y += step_y;

            // Compute new map positions
            map_x = test_x >> 10;
            map_y = test_y >> 10;

            // Determine which grid line was crossed
            if (map_x != prev_map_x && map_y == prev_map_y) {
                side = 0;  // Vertical wall
            } else if (map_y != prev_map_y && map_x == prev_map_x) {
                side = 1;  // Horizontal wall
            } else if (map_x != prev_map_x && map_y != prev_map_y) {
                // Both map_x and map_y changed, determine which side was crossed first

                // Calculate the fractional parts within the cell
                int cell_offset_x = test_x & (1024 - 1);  // scale_factor = 1024
                int cell_offset_y = test_y & (1024 - 1);

                // Adjust for direction
                if (step_x < 0) {
                    cell_offset_x = 1024 - cell_offset_x;
                }
                if (step_y < 0) {
                    cell_offset_y = 1024 - cell_offset_y;
                }

                // Compare adjusted offsets to determine the side
                if (cell_offset_x < cell_offset_y) {
                    side = 0;  // Vertical wall
                } else {
                    side = 1;  // Horizontal wall
                }
            }

            // Collision check
            if (map_x >= 0 && map_x < MAP_WIDTH && map_y >= 0 && map_y < MAP_HEIGHT) {
                col = world_map[map_y][map_x];
                if (col > 0) {
                    hit = 1;
                }
            }

            // Increment the distance by the scaled step size (integer only)
            distance += (1024 >> 10);  // Equivalent to adding 1 unit
        }

        if (hit) {
            // Calculate the height of the line to draw on screen
            scaled_screen_height = SCREEN_HEIGHT << 3;  // Pre-multiply by 8
            line_height = 0;
            temp_distance = distance;
            shift_amount = 0;

            // Calculate line height using bit shifting
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

            // Calculate line start and end positions
            line_start = (SCREEN_HEIGHT >> 1) - (line_height >> 1);
            line_end = (SCREEN_HEIGHT >> 1) + (line_height >> 1);

            reciprocal_line_height = reciprocal_table[line_height];
            delta_texY = reciprocal_line_height;
            texY = 0;

            // Texture mapping
            if (side == 0) {
                texX = (test_y >> 6) & 15;  // Adjusted for 16x16 texture size
            } else {
                texX = (test_x >> 6) & 15;
            }

            // Initialize variables for span rendering
            y = line_start;
            while (y <= line_end) {
                // Get the current color from the texture
                cc = texture[(((texY >> 8) & 15) << 4) + texX]+16;
                    if (side == 1) {
                        cc +=4;
                    }

                // Start of the color span
                span_start = y;
                span_texY = texY;

                // Advance y and texY while the color remains the same
                while (1) {
                    y++;
                    texY += delta_texY;

                    if (y >= line_end) {
                        break;  // Reached the end of the line
                    }

                    // Get the next color
                    next_cc = texture[(((texY >> 8) & 15) << 4) + texX]+16;

                    if (side == 1) {
                        next_cc +=4;
                    }

                    // Check if the color has changed
                    if (next_cc != cc) {
                        break;  // End of the current color span
                    }
                }

                // Calculate the span height
                span_height = y - span_start+1;

                // Draw the vertical line span using vline1
                vline(chunkyBuffer + ymul[span_start] + ray, cc<<8|cc, span_height);
            }
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
    OpenDevice("timer.device", 0, &timereq, 0);
    TimerBase = timereq.io_Device;
    GetSysTime(&startTime);
}
 
ULONG getMilliseconds() {
    struct timeval endTime;
 
    GetSysTime(&endTime);
    SubTime(&endTime, &startTime);

    return (endTime.tv_secs * 1000 + endTime.tv_micro / 1000);
}

static int l_c2p (lua_State *LL) {
  int x = luaL_checknumber(LL, 1);
  int y = luaL_checknumber(LL, 2);
  int w = luaL_checknumber(LL, 3);
  int h = luaL_checknumber(LL, 4);

  c2p1x1_4_c5_bm_word(w, h, x, y, chunkyBuffer, currentBitmap);

  lua_pushnumber(LL, 0);
  return 1;
}

void ReloadLua();

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


int main(void) {
    UWORD oldDMA;
    int i,ii = 0;
    BPTR file_ptr;
    ULONG size;

    // load and initialize everything

    SysBase = *((struct ExecBase **)4);

    UtilityBase = (struct Library*)OpenLibrary("utility.library",0L);
    MathIeeeSingBasBase   = (struct Library *) OpenLibrary("mathieeesingbas.library", 0L);
    MathIeeeSingTransBase   = (struct Library *) OpenLibrary("mathieeesingtrans.library", 0L);
    MathIeeeDoubBasBase   = (struct Library *) OpenLibrary("mathieeedoubbas.library", 0L);
    MathIeeeDoubTransBase = (struct Library *) OpenLibrary("mathieeedoubtrans.library", 0L);

    if ((UtilityBase) &&(MathIeeeSingTransBase) && (MathIeeeSingBasBase) && (MathIeeeDoubBasBase) && (MathIeeeDoubTransBase))
    {
    }
    else {
        Printf("failed to open libraries!!!\n");
        exit(1);        
    }

    init_lookup_tables();

    L = luaL_newstate();
    if(L == NULL) {
        return -1;
    }
    
    luaL_openlibs(L);
    lua_pushcfunction(L, l_c2p);
    lua_setglobal(L, "c2p");

    ReloadLua();

    // hide mouse
    emptyPointer = AllocVec(22 * sizeof(UWORD), MEMF_CHIP | MEMF_CLEAR);
    my_wbscreen_ptr = LockPubScreen("Workbench");
    SetPointer(my_wbscreen_ptr->FirstWindow, emptyPointer, 8, 8, -6, 0);
    UnlockPubScreen(NULL, my_wbscreen_ptr);
    
    if (!initScreen(&mainBitmap1, &mainScreen1)) {
        goto _exit_free_temp_bitmap;
    }

    bufferSelector = TRUE;
    currentScreen = mainScreen1;
    currentBitmap = mainBitmap1;

    c2p2x1_8_c5_030_init(160,256,0,0,320/8,(LONG)currentBitmap->BytesPerRow);

    chunkyBuffer = AllocVec(160 * 256 * sizeof(UBYTE), MEMF_FAST | MEMF_CLEAR);
//    CopyMemQuick(noitapic, chunkyBuffer, 320*256);

    LoadRGB32(&(mainScreen1->ViewPort), custompal);

    moddata = LoadFile("esa.mod", MEMF_CHIP);

    flypic = LoadFile("flypic.raw", MEMF_CHIP);
    chei = LoadFile("chei.raw", MEMF_CHIP);
    cpic2 = LoadFile("cpic2.raw", MEMF_CHIP);
    for (ii=0;ii<256;ii++) {
        for (i=0;i<256;i++) {
            if (cpic2[(ii<<8)+i] == 0x00) {
                cpic2[(ii<<8)+i] = 0x03;
            }
        }
    }

/*
    file_ptr = Open("cpic2.raw", MODE_NEWFILE);
    Write(file_ptr, (cpic2), 256*256);
    Close(file_ptr);
*/

    mt_install_cia(&custom, (APTR)App_GetVBR(), 1);
    mt_init(&custom, moddata, NULL, 0);
    mt_mastervol(&custom, 0x40);
    mt_Enable = 1;

    startup();

    ScreenToFront(currentScreen);
    WaitTOF();

    // ready to go to mainloop

    MainLoop();

    // exit routine follows

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
    FreeVec(chei);
    FreeVec(cpic2);
    FreeVec(moddata);
    FreeVec(chunkyBuffer);
    FreeVec(currentPal);

    CloseScreen(mainScreen1);
    WaitTOF();
    CloseDevice(&timereq);

    if (MathIeeeDoubTransBase) { CloseLibrary(MathIeeeDoubTransBase); }
    if (MathIeeeDoubBasBase)   { CloseLibrary(MathIeeeDoubBasBase); }
    if (MathIeeeSingBasBase)   { CloseLibrary(MathIeeeSingBasBase); }
    if (MathIeeeSingTransBase)   { CloseLibrary(MathIeeeSingTransBase); }
    if (UtilityBase)  { CloseLibrary(UtilityBase); }

    FreeBitMap(mainBitmap1);
_exit_free_temp_bitmap:
    // restore mouse
    my_wbscreen_ptr = LockPubScreen("Workbench");
    ClearPointer(my_wbscreen_ptr->FirstWindow);
    UnlockPubScreen(NULL, my_wbscreen_ptr);
    FreeVec(emptyPointer);

    lua_close(L);

    exit(RETURN_OK);
    
}

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
    UBYTE* destPtr;
    UBYTE drawcolor2;

    int xoff = 0;
    UBYTE lh = 0;
    UBYTE tx,ty,zs = 0;

    if (frame == 0) {
        drawcolor = 10;
        fillrect(0,0,160,94);
    }

    py = -dta;
    px = -py;
    off2 = sine[(64+(dta<<3))&0xff]>>5;
    off3 = sine[(dta<<3)&0xff]>>5;

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
        destPtr = &chunkyBuffer[ymul[y + 26 + off2] + 20 + off3]; // Pre-calculate the destination pointer base
        for (x = 0; x < 64; x++) {
            cc = flypic[(y << 7) + (x<<1)]; // Access the source pixel
            destPtr[x] = (cc != 0x0F) ? cc : destPtr[x]; // Use a ternary operator to avoid an explicit if statement
        }
    }

}

void Lines()
{
    int centerX = 80;
    int centerY = 127;
    int width = 160+(sine[(dta<<1)&0xff]>>3);
    int height = 160+(sine[(dta<<1)&0xff]>>3);
    int i = 0;
    if (frame == 0) 
    {
        memset(chunkyBuffer,0,160*256);
    }
    for(i=frame%4; i < 160<<8; i+=4) {
        if (chunkyBuffer[i] > 0)
            chunkyBuffer[i]-=1;
            chunkyBuffer[i]&=chunkyBuffer[i-1];

    }
    
    for (i = 0; i < 24; i++)
        rotrect(centerX, centerY, width-(i<<3), height-(i<<3), (frame+(i<<2))&0xff, i%15);

}

#define SHIFT 16                  // Number of fractional bits
#define ONE (1 << SHIFT)          // Representation of 1.0 in fixed-point
#define HALF (ONE >> 1)           // Representation of 0.5 in fixed-point

// Time step (Δt) scaled by SHIFT (e.g., Δt = 1/60 ≈ 0.0166667)
#define DELTA_T 1092      // Approximately 0.0166667 * 2^16 ≈ 1092

// Gravity acceleration scaled by SHIFT (e.g., g = 9.81)
#define GRAVITY 643634 // Approximately 9.81 * 65536 ≈ 643634

int fixed_mult(int a, int b) {
    long temp = (long)a * (long)b;
    return (int)(temp >> SHIFT);
}

// Fixed-point division
int fixed_div(int a, int b) {
    long temp;
    if (b == 0) {
        // Handle division by zero as needed
        return 0;
    }
    temp = ((long)a << SHIFT) / b;
    return (int)temp;
}

// Fixed-point representation of 0.5
int fixed_half() {
    return HALF;
}

// Fixed-point addition
int fixed_add(int a, int b) {
    return a + b;
}

// Fixed-point subtraction
int fixed_sub(int a, int b) {
    return a - b;
}

typedef struct {
    int x, y;           // Current position (fixed-point)
    int prev_x, prev_y; // Previous position (fixed-point)
    int radius;         // Radius (fixed-point)
} Vertex;


#define SHIFT_WIDTH (320 << SHIFT)   // Example screen width in fixed-point
#define SHIFT_HEIGHT (256 << SHIFT)  // Example screen height in fixed-point

#define NUM_POINTS 100
Vertex points[NUM_POINTS];


void initialize_point(Vertex *p, int init_x, int init_y, int radius) {
    p->x = init_x;
    p->y = init_y;
    p->prev_x = init_x;
    p->prev_y = init_y;
    p->radius = radius;
}

void verlet_integration(Vertex *p, int accel_x, int accel_y) {
    // Calculate new position using Verlet integration
    // new_pos = 2 * current_pos - prev_pos + accel * Δt^2

    int temp_x = p->x;
    int temp_y = p->y;

    // Calculate 2 * current_pos
    int two_current_x = fixed_mult(2 * ONE, p->x); // 2.0 * x
    int two_current_y = fixed_mult(2 * ONE, p->y); // 2.0 * y

    // Calculate accel * Δt^2
    int accel_dt2_x = fixed_mult(accel_x, fixed_mult(DELTA_T, DELTA_T));
    int accel_dt2_y = fixed_mult(accel_y, fixed_mult(DELTA_T, DELTA_T));

    // Update positions
    p->x = fixed_add(fixed_sub(two_current_x, p->prev_x), accel_dt2_x);
    p->y = fixed_add(fixed_sub(two_current_y, p->prev_y), accel_dt2_y);

    // Update previous positions
    p->prev_x = temp_x;
    p->prev_y = temp_y;
}

void handle_collisions(Vertex *points, int num_points) {
    int i, j, dx,dy, dx_sq,dy_sq,distance_sq, radii_sum, radii_sum_sq;
    for(i = 0; i < num_points; i++) {
        for(j = i + 1; j < num_points; j++) {
            dx = points[j].x - points[i].x;
            dy = points[j].y - points[i].y;

            // Calculate distance squared: dx^2 + dy^2
            dx_sq = fixed_mult(dx, dx);
            dy_sq = fixed_mult(dy, dy);
            distance_sq = fixed_add(dx_sq, dy_sq);

            // Calculate (radius_i + radius_j)^2
            radii_sum = fixed_add(points[i].radius, points[j].radius);
            radii_sum_sq = fixed_mult(radii_sum, radii_sum);

            if(distance_sq < radii_sum_sq) {
                // Points are overlapping, need to push them apart

                // To avoid division and square roots, we'll push along the x and y axes separately
                if(dx != 0 || dy != 0) {
                    // Push each point by half the overlap
                    points[i].x = fixed_sub(points[i].x, fixed_mult(dx, fixed_half()));
                    points[i].y = fixed_sub(points[i].y, fixed_mult(dy, fixed_half()));
                    points[j].x = fixed_add(points[j].x, fixed_mult(dx, fixed_half()));
                    points[j].y = fixed_add(points[j].y, fixed_mult(dy, fixed_half()));
                }
            }
        }
    }
}

void apply_boundaries(Vertex *p) {
    // Left boundary
    if(p->x < 0) {
        p->x = 0;
        p->prev_x = p->x;
    }
    // Right boundary
    if(p->x > SCREEN_WIDTH) {
        p->x = SCREEN_WIDTH;
        p->prev_x = p->x;
    }
    // Top boundary
    if(p->y < 0) {
        p->y = 0;
        p->prev_y = p->y;
    }
    // Bottom boundary
    if(p->y > SCREEN_HEIGHT) {
        p->y = SCREEN_HEIGHT;
        p->prev_y = p->y;
    }
}

void Feedbakker() {
    int i, step, init_x,init_y,radius,x_px, y_px, radius_px;

    if (frame == 0) {
        for(i = 0; i < NUM_POINTS; i++) {
            // Random positions within screen boundaries
            init_x = (rand() % 320) << SHIFT; // Convert to fixed-point
            init_y = (rand() % 256) << SHIFT; // Convert to fixed-point
            radius = (5 << SHIFT);            // Radius of 5.0 units
            initialize_point(&points[i], init_x, init_y, radius);
        }        
    }

    for(step = 0; step < 1000; step++) {
        // Apply Verlet integration to each point with gravity
        for(i = 0; i < NUM_POINTS; i++) {
            verlet_integration(&points[i], 0, GRAVITY);
        }

        // Handle collisions between points
        handle_collisions(points, NUM_POINTS);

        // Apply boundary conditions
        for(i = 0; i < NUM_POINTS; i++) {
            apply_boundaries(&points[i]);
        }

        // Optional: Render the points
        // Since rendering is platform-specific, it's omitted here
        // You can convert fixed-point positions to integer pixels for rendering
        
        for(i = 0; i < NUM_POINTS; i++) {
            x_px = points[i].x >> SHIFT;
            y_px = points[i].y >> SHIFT;
            radius_px = points[i].radius >> SHIFT;
            chunkyBuffer[ymul[y_px]+x_px] = 20+(i%8);
        }
        
    }
}

DrawFunc DrawFuncs[4] = {HeightMap, Lines, Raycast, Feedbakker};

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

    key_buffer = 0;
    return 0;
}

char *luastr;
int luainit = 0;

void ReloadLua()
{
    if (luainit == 1) {
        FreeVec(luastr);
    }
    WaitTOF();
    luastr = LoadFile("demo.lua", MEMF_FAST);
    luainit = 1;
}

void MainLoop() {
    while (ex == 0) {
        if (keys() == 1) {
            ex = 1;
        }

        if (keys() == 2) {
            ReloadLua();
        }

        if (mouseCiaStatus()) 
        {
            ex = 1;
        }
        dt = (et-st)>>5;
        if (dt == 0) dt = 1;
        st = getMilliseconds();
        dta = dta + dt;

        scene = 3;
        //scene = (totalframes>>8)%3;

        if (oldscene != scene) {
            frame = 0;
        }

        oldscene = scene;

        DrawFuncs[scene]();

        c2p2x1_8_c5_030(chunkyBuffer, currentBitmap->Planes[0]);

/*
        lua_pushinteger(L, frame);
        lua_setglobal(L, "frame");

        luaL_dostring(L, luastr);
*/
        et = getMilliseconds();


        frame++;
        totalframes++;

        WaitTOF();

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

    // create one screen which contains the demo logo
    *s = createScreen(*b, TRUE, 0, 0,
                      320, 256,
                      8, NULL);
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
