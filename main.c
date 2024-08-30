// adapted from example blog tutorial by Christian Ammann

#include <clib/timer_protos.h>
#include <clib/exec_protos.h>

#include "demo.h"

#define LUA_IMPL
#include "minilua.h"

#define __builtin_expect(x,y)

#include "ptplayer/ptplayer.h"

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
int finesine[];

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

#define MAP_WIDTH  16
#define MAP_HEIGHT 16
#define FOV 64 // 90 degrees FOV
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 256
#define FIXED_POINT_SHIFT 8

UBYTE world_map[MAP_WIDTH][MAP_HEIGHT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,
    1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,
    1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,
    1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

typedef struct {
    UWORD x; // 8-bit fixed-point numbers (where 1.0 == 256)
    UWORD y;
    UBYTE angle;
} Player;

inline UBYTE fast_sin(UBYTE angle) {
    return sine[angle & 0xff];
}

inline UBYTE fast_cos(UBYTE angle) {
    return sine[(angle + 64) & 0xff];
}

void rayline(int column, UWORD distance) {
    UBYTE line_height = SCREEN_HEIGHT / distance;
    int start,end;

    // Calculate start and end points for the line
    start = (SCREEN_HEIGHT / 2) - (line_height / 2);
    end = (SCREEN_HEIGHT / 2) + (line_height / 2);

    // Clamp the start and end to screen boundaries
    if (start < 0) start = 0;
    if (end >= SCREEN_HEIGHT) end = SCREEN_HEIGHT - 1;

    vline(chunkyBuffer+ymul[start]+column, (15<<8)|15, end-start);
}

void ray(Player *player, int column) {
    UBYTE ray_angle = (player->angle - (FOV / 2)) + ((FOV * column) / SCREEN_WIDTH);


    // Get the direction of the ray using sine and cosine, converting the table values to signed
    int step_x = sine[(ray_angle + 64)&0xff] - 128; // Fast cos
    int step_y = sine[ray_angle&0xff] - 128;      // Fast sin
    
    int ray_x = player->x;
    int ray_y = player->y;
    int delta_x,delta_y;
    UBYTE distance,map_x,map_y;

    while (1) {
        map_x = ray_x / 16;  // Convert ray position to map cell position (dividing by 16)
        map_y = ray_y / 16;

        if (map_x >= MAP_WIDTH || map_y >= MAP_HEIGHT) {
            break; // Ray is out of map bounds
        }

        if (world_map[map_x][map_y] == 1) {
            // Calculate the distance to the wall using corrected delta values
            delta_x = (ray_x - player->x)>>4;
            delta_y = (ray_y - player->y)>>4;

            // Use the Pythagorean approximation to calculate distance
            distance = (UBYTE) sqrt((delta_x * delta_x) + (delta_y * delta_y));

            rayline(column, distance);
            break;
        }

        // Move the ray forward
        ray_x += step_x;
        ray_y += step_y;
    }
}

void Raycast()
{
    Player player;
    int column;

    if (frame == 0) 
    {
        memset(chunkyBuffer,0,160*256);
    }

    memset(chunkyBuffer,1,160*256);

    player.x = 128; // Start near the middle of a 256x256 grid
    player.y = 128;
    player.angle = totalframes;

    for (column = 0; column < 160; column++) {
        vline(chunkyBuffer+column,16+((column%16 << 8)) | 16+(column % 16), 32);
        ray(&player, column);
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

int sgn(int x) {
    return (x > 0) - (x < 0);
}

int abs(int x) {
    return (x < 0) ? -x : x;
}

void Feedbakker() {
    int rs = 32;
    int x, y;
    if (frame == 0) 
    {
        memset(chunkyBuffer,0,160*256);

    }

    rs = rs + (fast_sin(dta)>>4);
    drawcolor = 20+((dta>>1)%4);
    rect(80-16,128-16,80+16,128+16);

    for (y = 256+frame%2; y >= 1; y -= 2) {
        for (x = 80 - 2; x >= 1; x--) {
            UBYTE c = chunkyBuffer[ymul[y]+x+(fast_cos(dta+x)>>4)];
            int xx = x - 80;
            int yy = y - 128;
            int xo = sgn(xx);
            int yo = sgn(yy);

            // Set the new pixels
            chunkyBuffer[ymul[y+yo]+x + xo] = (c*3)%32;
            chunkyBuffer[ymul[y+yo]+(160-x + xo)] = (c*3)%32;
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

        scene = (totalframes>>8)%4;

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
int finesine[10240] =
{
    25,75,125,175,226,276,326,376,
    427,477,527,578,628,678,728,779,
    829,879,929,980,1030,1080,1130,1181,
    1231,1281,1331,1382,1432,1482,1532,1583,
    1633,1683,1733,1784,1834,1884,1934,1985,
    2035,2085,2135,2186,2236,2286,2336,2387,
    2437,2487,2537,2587,2638,2688,2738,2788,
    2839,2889,2939,2989,3039,3090,3140,3190,
    3240,3291,3341,3391,3441,3491,3541,3592,
    3642,3692,3742,3792,3843,3893,3943,3993,
    4043,4093,4144,4194,4244,4294,4344,4394,
    4445,4495,4545,4595,4645,4695,4745,4796,
    4846,4896,4946,4996,5046,5096,5146,5197,
    5247,5297,5347,5397,5447,5497,5547,5597,
    5647,5697,5748,5798,5848,5898,5948,5998,
    6048,6098,6148,6198,6248,6298,6348,6398,
    6448,6498,6548,6598,6648,6698,6748,6798,
    6848,6898,6948,6998,7048,7098,7148,7198,
    7248,7298,7348,7398,7448,7498,7548,7598,
    7648,7697,7747,7797,7847,7897,7947,7997,
    8047,8097,8147,8196,8246,8296,8346,8396,
    8446,8496,8545,8595,8645,8695,8745,8794,
    8844,8894,8944,8994,9043,9093,9143,9193,
    9243,9292,9342,9392,9442,9491,9541,9591,
    9640,9690,9740,9790,9839,9889,9939,9988,
    10038,10088,10137,10187,10237,10286,10336,10386,
    10435,10485,10534,10584,10634,10683,10733,10782,
    10832,10882,10931,10981,11030,11080,11129,11179,
    11228,11278,11327,11377,11426,11476,11525,11575,
    11624,11674,11723,11773,11822,11872,11921,11970,
    12020,12069,12119,12168,12218,12267,12316,12366,
    12415,12464,12514,12563,12612,12662,12711,12760,
    12810,12859,12908,12957,13007,13056,13105,13154,
    13204,13253,13302,13351,13401,13450,13499,13548,
    13597,13647,13696,13745,13794,13843,13892,13941,
    13990,14040,14089,14138,14187,14236,14285,14334,
    14383,14432,14481,14530,14579,14628,14677,14726,
    14775,14824,14873,14922,14971,15020,15069,15118,
    15167,15215,15264,15313,15362,15411,15460,15509,
    15557,15606,15655,15704,15753,15802,15850,15899,
    15948,15997,16045,16094,16143,16191,16240,16289,
    16338,16386,16435,16484,16532,16581,16629,16678,
    16727,16775,16824,16872,16921,16970,17018,17067,
    17115,17164,17212,17261,17309,17358,17406,17455,
    17503,17551,17600,17648,17697,17745,17793,17842,
    17890,17939,17987,18035,18084,18132,18180,18228,
    18277,18325,18373,18421,18470,18518,18566,18614,
    18663,18711,18759,18807,18855,18903,18951,19000,
    19048,19096,19144,19192,19240,19288,19336,19384,
    19432,19480,19528,19576,19624,19672,19720,19768,
    19816,19864,19912,19959,20007,20055,20103,20151,
    20199,20246,20294,20342,20390,20438,20485,20533,
    20581,20629,20676,20724,20772,20819,20867,20915,
    20962,21010,21057,21105,21153,21200,21248,21295,
    21343,21390,21438,21485,21533,21580,21628,21675,
    21723,21770,21817,21865,21912,21960,22007,22054,
    22102,22149,22196,22243,22291,22338,22385,22433,
    22480,22527,22574,22621,22668,22716,22763,22810,
    22857,22904,22951,22998,23045,23092,23139,23186,
    23233,23280,23327,23374,23421,23468,23515,23562,
    23609,23656,23703,23750,23796,23843,23890,23937,
    23984,24030,24077,24124,24171,24217,24264,24311,
    24357,24404,24451,24497,24544,24591,24637,24684,
    24730,24777,24823,24870,24916,24963,25009,25056,
    25102,25149,25195,25241,25288,25334,25381,25427,
    25473,25520,25566,25612,25658,25705,25751,25797,
    25843,25889,25936,25982,26028,26074,26120,26166,
    26212,26258,26304,26350,26396,26442,26488,26534,
    26580,26626,26672,26718,26764,26810,26856,26902,
    26947,26993,27039,27085,27131,27176,27222,27268,
    27313,27359,27405,27450,27496,27542,27587,27633,
    27678,27724,27770,27815,27861,27906,27952,27997,
    28042,28088,28133,28179,28224,28269,28315,28360,
    28405,28451,28496,28541,28586,28632,28677,28722,
    28767,28812,28858,28903,28948,28993,29038,29083,
    29128,29173,29218,29263,29308,29353,29398,29443,
    29488,29533,29577,29622,29667,29712,29757,29801,
    29846,29891,29936,29980,30025,30070,30114,30159,
    30204,30248,30293,30337,30382,30426,30471,30515,
    30560,30604,30649,30693,30738,30782,30826,30871,
    30915,30959,31004,31048,31092,31136,31181,31225,
    31269,31313,31357,31402,31446,31490,31534,31578,
    31622,31666,31710,31754,31798,31842,31886,31930,
    31974,32017,32061,32105,32149,32193,32236,32280,
    32324,32368,32411,32455,32499,32542,32586,32630,
    32673,32717,32760,32804,32847,32891,32934,32978,
    33021,33065,33108,33151,33195,33238,33281,33325,
    33368,33411,33454,33498,33541,33584,33627,33670,
    33713,33756,33799,33843,33886,33929,33972,34015,
    34057,34100,34143,34186,34229,34272,34315,34358,
    34400,34443,34486,34529,34571,34614,34657,34699,
    34742,34785,34827,34870,34912,34955,34997,35040,
    35082,35125,35167,35210,35252,35294,35337,35379,
    35421,35464,35506,35548,35590,35633,35675,35717,
    35759,35801,35843,35885,35927,35969,36011,36053,
    36095,36137,36179,36221,36263,36305,36347,36388,
    36430,36472,36514,36555,36597,36639,36681,36722,
    36764,36805,36847,36889,36930,36972,37013,37055,
    37096,37137,37179,37220,37262,37303,37344,37386,
    37427,37468,37509,37551,37592,37633,37674,37715,
    37756,37797,37838,37879,37920,37961,38002,38043,
    38084,38125,38166,38207,38248,38288,38329,38370,
    38411,38451,38492,38533,38573,38614,38655,38695,
    38736,38776,38817,38857,38898,38938,38979,39019,
    39059,39100,39140,39180,39221,39261,39301,39341,
    39382,39422,39462,39502,39542,39582,39622,39662,
    39702,39742,39782,39822,39862,39902,39942,39982,
    40021,40061,40101,40141,40180,40220,40260,40300,
    40339,40379,40418,40458,40497,40537,40576,40616,
    40655,40695,40734,40773,40813,40852,40891,40931,
    40970,41009,41048,41087,41127,41166,41205,41244,
    41283,41322,41361,41400,41439,41478,41517,41556,
    41595,41633,41672,41711,41750,41788,41827,41866,
    41904,41943,41982,42020,42059,42097,42136,42174,
    42213,42251,42290,42328,42366,42405,42443,42481,
    42520,42558,42596,42634,42672,42711,42749,42787,
    42825,42863,42901,42939,42977,43015,43053,43091,
    43128,43166,43204,43242,43280,43317,43355,43393,
    43430,43468,43506,43543,43581,43618,43656,43693,
    43731,43768,43806,43843,43880,43918,43955,43992,
    44029,44067,44104,44141,44178,44215,44252,44289,
    44326,44363,44400,44437,44474,44511,44548,44585,
    44622,44659,44695,44732,44769,44806,44842,44879,
    44915,44952,44989,45025,45062,45098,45135,45171,
    45207,45244,45280,45316,45353,45389,45425,45462,
    45498,45534,45570,45606,45642,45678,45714,45750,
    45786,45822,45858,45894,45930,45966,46002,46037,
    46073,46109,46145,46180,46216,46252,46287,46323,
    46358,46394,46429,46465,46500,46536,46571,46606,
    46642,46677,46712,46747,46783,46818,46853,46888,
    46923,46958,46993,47028,47063,47098,47133,47168,
    47203,47238,47273,47308,47342,47377,47412,47446,
    47481,47516,47550,47585,47619,47654,47688,47723,
    47757,47792,47826,47860,47895,47929,47963,47998,
    48032,48066,48100,48134,48168,48202,48237,48271,
    48305,48338,48372,48406,48440,48474,48508,48542,
    48575,48609,48643,48676,48710,48744,48777,48811,
    48844,48878,48911,48945,48978,49012,49045,49078,
    49112,49145,49178,49211,49244,49278,49311,49344,
    49377,49410,49443,49476,49509,49542,49575,49608,
    49640,49673,49706,49739,49771,49804,49837,49869,
    49902,49935,49967,50000,50032,50065,50097,50129,
    50162,50194,50226,50259,50291,50323,50355,50387,
    50420,50452,50484,50516,50548,50580,50612,50644,
    50675,50707,50739,50771,50803,50834,50866,50898,
    50929,50961,50993,51024,51056,51087,51119,51150,
    51182,51213,51244,51276,51307,51338,51369,51401,
    51432,51463,51494,51525,51556,51587,51618,51649,
    51680,51711,51742,51773,51803,51834,51865,51896,
    51926,51957,51988,52018,52049,52079,52110,52140,
    52171,52201,52231,52262,52292,52322,52353,52383,
    52413,52443,52473,52503,52534,52564,52594,52624,
    52653,52683,52713,52743,52773,52803,52832,52862,
    52892,52922,52951,52981,53010,53040,53069,53099,
    53128,53158,53187,53216,53246,53275,53304,53334,
    53363,53392,53421,53450,53479,53508,53537,53566,
    53595,53624,53653,53682,53711,53739,53768,53797,
    53826,53854,53883,53911,53940,53969,53997,54026,
    54054,54082,54111,54139,54167,54196,54224,54252,
    54280,54308,54337,54365,54393,54421,54449,54477,
    54505,54533,54560,54588,54616,54644,54672,54699,
    54727,54755,54782,54810,54837,54865,54892,54920,
    54947,54974,55002,55029,55056,55084,55111,55138,
    55165,55192,55219,55246,55274,55300,55327,55354,
    55381,55408,55435,55462,55489,55515,55542,55569,
    55595,55622,55648,55675,55701,55728,55754,55781,
    55807,55833,55860,55886,55912,55938,55965,55991,
    56017,56043,56069,56095,56121,56147,56173,56199,
    56225,56250,56276,56302,56328,56353,56379,56404,
    56430,56456,56481,56507,56532,56557,56583,56608,
    56633,56659,56684,56709,56734,56760,56785,56810,
    56835,56860,56885,56910,56935,56959,56984,57009,
    57034,57059,57083,57108,57133,57157,57182,57206,
    57231,57255,57280,57304,57329,57353,57377,57402,
    57426,57450,57474,57498,57522,57546,57570,57594,
    57618,57642,57666,57690,57714,57738,57762,57785,
    57809,57833,57856,57880,57903,57927,57950,57974,
    57997,58021,58044,58067,58091,58114,58137,58160,
    58183,58207,58230,58253,58276,58299,58322,58345,
    58367,58390,58413,58436,58459,58481,58504,58527,
    58549,58572,58594,58617,58639,58662,58684,58706,
    58729,58751,58773,58795,58818,58840,58862,58884,
    58906,58928,58950,58972,58994,59016,59038,59059,
    59081,59103,59125,59146,59168,59190,59211,59233,
    59254,59276,59297,59318,59340,59361,59382,59404,
    59425,59446,59467,59488,59509,59530,59551,59572,
    59593,59614,59635,59656,59677,59697,59718,59739,
    59759,59780,59801,59821,59842,59862,59883,59903,
    59923,59944,59964,59984,60004,60025,60045,60065,
    60085,60105,60125,60145,60165,60185,60205,60225,
    60244,60264,60284,60304,60323,60343,60363,60382,
    60402,60421,60441,60460,60479,60499,60518,60537,
    60556,60576,60595,60614,60633,60652,60671,60690,
    60709,60728,60747,60766,60785,60803,60822,60841,
    60859,60878,60897,60915,60934,60952,60971,60989,
    61007,61026,61044,61062,61081,61099,61117,61135,
    61153,61171,61189,61207,61225,61243,61261,61279,
    61297,61314,61332,61350,61367,61385,61403,61420,
    61438,61455,61473,61490,61507,61525,61542,61559,
    61577,61594,61611,61628,61645,61662,61679,61696,
    61713,61730,61747,61764,61780,61797,61814,61831,
    61847,61864,61880,61897,61913,61930,61946,61963,
    61979,61995,62012,62028,62044,62060,62076,62092,
    62108,62125,62141,62156,62172,62188,62204,62220,
    62236,62251,62267,62283,62298,62314,62329,62345,
    62360,62376,62391,62407,62422,62437,62453,62468,
    62483,62498,62513,62528,62543,62558,62573,62588,
    62603,62618,62633,62648,62662,62677,62692,62706,
    62721,62735,62750,62764,62779,62793,62808,62822,
    62836,62850,62865,62879,62893,62907,62921,62935,
    62949,62963,62977,62991,63005,63019,63032,63046,
    63060,63074,63087,63101,63114,63128,63141,63155,
    63168,63182,63195,63208,63221,63235,63248,63261,
    63274,63287,63300,63313,63326,63339,63352,63365,
    63378,63390,63403,63416,63429,63441,63454,63466,
    63479,63491,63504,63516,63528,63541,63553,63565,
    63578,63590,63602,63614,63626,63638,63650,63662,
    63674,63686,63698,63709,63721,63733,63745,63756,
    63768,63779,63791,63803,63814,63825,63837,63848,
    63859,63871,63882,63893,63904,63915,63927,63938,
    63949,63960,63971,63981,63992,64003,64014,64025,
    64035,64046,64057,64067,64078,64088,64099,64109,
    64120,64130,64140,64151,64161,64171,64181,64192,
    64202,64212,64222,64232,64242,64252,64261,64271,
    64281,64291,64301,64310,64320,64330,64339,64349,
    64358,64368,64377,64387,64396,64405,64414,64424,
    64433,64442,64451,64460,64469,64478,64487,64496,
    64505,64514,64523,64532,64540,64549,64558,64566,
    64575,64584,64592,64601,64609,64617,64626,64634,
    64642,64651,64659,64667,64675,64683,64691,64699,
    64707,64715,64723,64731,64739,64747,64754,64762,
    64770,64777,64785,64793,64800,64808,64815,64822,
    64830,64837,64844,64852,64859,64866,64873,64880,
    64887,64895,64902,64908,64915,64922,64929,64936,
    64943,64949,64956,64963,64969,64976,64982,64989,
    64995,65002,65008,65015,65021,65027,65033,65040,
    65046,65052,65058,65064,65070,65076,65082,65088,
    65094,65099,65105,65111,65117,65122,65128,65133,
    65139,65144,65150,65155,65161,65166,65171,65177,
    65182,65187,65192,65197,65202,65207,65212,65217,
    65222,65227,65232,65237,65242,65246,65251,65256,
    65260,65265,65270,65274,65279,65283,65287,65292,
    65296,65300,65305,65309,65313,65317,65321,65325,
    65329,65333,65337,65341,65345,65349,65352,65356,
    65360,65363,65367,65371,65374,65378,65381,65385,
    65388,65391,65395,65398,65401,65404,65408,65411,
    65414,65417,65420,65423,65426,65429,65431,65434,
    65437,65440,65442,65445,65448,65450,65453,65455,
    65458,65460,65463,65465,65467,65470,65472,65474,
    65476,65478,65480,65482,65484,65486,65488,65490,
    65492,65494,65496,65497,65499,65501,65502,65504,
    65505,65507,65508,65510,65511,65513,65514,65515,
    65516,65518,65519,65520,65521,65522,65523,65524,
    65525,65526,65527,65527,65528,65529,65530,65530,
    65531,65531,65532,65532,65533,65533,65534,65534,
    65534,65535,65535,65535,65535,65535,65535,65535,
    65535,65535,65535,65535,65535,65535,65535,65534,
    65534,65534,65533,65533,65532,65532,65531,65531,
    65530,65530,65529,65528,65527,65527,65526,65525,
    65524,65523,65522,65521,65520,65519,65518,65516,
    65515,65514,65513,65511,65510,65508,65507,65505,
    65504,65502,65501,65499,65497,65496,65494,65492,
    65490,65488,65486,65484,65482,65480,65478,65476,
    65474,65472,65470,65467,65465,65463,65460,65458,
    65455,65453,65450,65448,65445,65442,65440,65437,
    65434,65431,65429,65426,65423,65420,65417,65414,
    65411,65408,65404,65401,65398,65395,65391,65388,
    65385,65381,65378,65374,65371,65367,65363,65360,
    65356,65352,65349,65345,65341,65337,65333,65329,
    65325,65321,65317,65313,65309,65305,65300,65296,
    65292,65287,65283,65279,65274,65270,65265,65260,
    65256,65251,65246,65242,65237,65232,65227,65222,
    65217,65212,65207,65202,65197,65192,65187,65182,
    65177,65171,65166,65161,65155,65150,65144,65139,
    65133,65128,65122,65117,65111,65105,65099,65094,
    65088,65082,65076,65070,65064,65058,65052,65046,
    65040,65033,65027,65021,65015,65008,65002,64995,
    64989,64982,64976,64969,64963,64956,64949,64943,
    64936,64929,64922,64915,64908,64902,64895,64887,
    64880,64873,64866,64859,64852,64844,64837,64830,
    64822,64815,64808,64800,64793,64785,64777,64770,
    64762,64754,64747,64739,64731,64723,64715,64707,
    64699,64691,64683,64675,64667,64659,64651,64642,
    64634,64626,64617,64609,64600,64592,64584,64575,
    64566,64558,64549,64540,64532,64523,64514,64505,
    64496,64487,64478,64469,64460,64451,64442,64433,
    64424,64414,64405,64396,64387,64377,64368,64358,
    64349,64339,64330,64320,64310,64301,64291,64281,
    64271,64261,64252,64242,64232,64222,64212,64202,
    64192,64181,64171,64161,64151,64140,64130,64120,
    64109,64099,64088,64078,64067,64057,64046,64035,
    64025,64014,64003,63992,63981,63971,63960,63949,
    63938,63927,63915,63904,63893,63882,63871,63859,
    63848,63837,63825,63814,63803,63791,63779,63768,
    63756,63745,63733,63721,63709,63698,63686,63674,
    63662,63650,63638,63626,63614,63602,63590,63578,
    63565,63553,63541,63528,63516,63504,63491,63479,
    63466,63454,63441,63429,63416,63403,63390,63378,
    63365,63352,63339,63326,63313,63300,63287,63274,
    63261,63248,63235,63221,63208,63195,63182,63168,
    63155,63141,63128,63114,63101,63087,63074,63060,
    63046,63032,63019,63005,62991,62977,62963,62949,
    62935,62921,62907,62893,62879,62865,62850,62836,
    62822,62808,62793,62779,62764,62750,62735,62721,
    62706,62692,62677,62662,62648,62633,62618,62603,
    62588,62573,62558,62543,62528,62513,62498,62483,
    62468,62453,62437,62422,62407,62391,62376,62360,
    62345,62329,62314,62298,62283,62267,62251,62236,
    62220,62204,62188,62172,62156,62141,62125,62108,
    62092,62076,62060,62044,62028,62012,61995,61979,
    61963,61946,61930,61913,61897,61880,61864,61847,
    61831,61814,61797,61780,61764,61747,61730,61713,
    61696,61679,61662,61645,61628,61611,61594,61577,
    61559,61542,61525,61507,61490,61473,61455,61438,
    61420,61403,61385,61367,61350,61332,61314,61297,
    61279,61261,61243,61225,61207,61189,61171,61153,
    61135,61117,61099,61081,61062,61044,61026,61007,
    60989,60971,60952,60934,60915,60897,60878,60859,
    60841,60822,60803,60785,60766,60747,60728,60709,
    60690,60671,60652,60633,60614,60595,60576,60556,
    60537,60518,60499,60479,60460,60441,60421,60402,
    60382,60363,60343,60323,60304,60284,60264,60244,
    60225,60205,60185,60165,60145,60125,60105,60085,
    60065,60045,60025,60004,59984,59964,59944,59923,
    59903,59883,59862,59842,59821,59801,59780,59759,
    59739,59718,59697,59677,59656,59635,59614,59593,
    59572,59551,59530,59509,59488,59467,59446,59425,
    59404,59382,59361,59340,59318,59297,59276,59254,
    59233,59211,59190,59168,59146,59125,59103,59081,
    59059,59038,59016,58994,58972,58950,58928,58906,
    58884,58862,58840,58818,58795,58773,58751,58729,
    58706,58684,58662,58639,58617,58594,58572,58549,
    58527,58504,58481,58459,58436,58413,58390,58367,
    58345,58322,58299,58276,58253,58230,58207,58183,
    58160,58137,58114,58091,58067,58044,58021,57997,
    57974,57950,57927,57903,57880,57856,57833,57809,
    57785,57762,57738,57714,57690,57666,57642,57618,
    57594,57570,57546,57522,57498,57474,57450,57426,
    57402,57377,57353,57329,57304,57280,57255,57231,
    57206,57182,57157,57133,57108,57083,57059,57034,
    57009,56984,56959,56935,56910,56885,56860,56835,
    56810,56785,56760,56734,56709,56684,56659,56633,
    56608,56583,56557,56532,56507,56481,56456,56430,
    56404,56379,56353,56328,56302,56276,56250,56225,
    56199,56173,56147,56121,56095,56069,56043,56017,
    55991,55965,55938,55912,55886,55860,55833,55807,
    55781,55754,55728,55701,55675,55648,55622,55595,
    55569,55542,55515,55489,55462,55435,55408,55381,
    55354,55327,55300,55274,55246,55219,55192,55165,
    55138,55111,55084,55056,55029,55002,54974,54947,
    54920,54892,54865,54837,54810,54782,54755,54727,
    54699,54672,54644,54616,54588,54560,54533,54505,
    54477,54449,54421,54393,54365,54337,54308,54280,
    54252,54224,54196,54167,54139,54111,54082,54054,
    54026,53997,53969,53940,53911,53883,53854,53826,
    53797,53768,53739,53711,53682,53653,53624,53595,
    53566,53537,53508,53479,53450,53421,53392,53363,
    53334,53304,53275,53246,53216,53187,53158,53128,
    53099,53069,53040,53010,52981,52951,52922,52892,
    52862,52832,52803,52773,52743,52713,52683,52653,
    52624,52594,52564,52534,52503,52473,52443,52413,
    52383,52353,52322,52292,52262,52231,52201,52171,
    52140,52110,52079,52049,52018,51988,51957,51926,
    51896,51865,51834,51803,51773,51742,51711,51680,
    51649,51618,51587,51556,51525,51494,51463,51432,
    51401,51369,51338,51307,51276,51244,51213,51182,
    51150,51119,51087,51056,51024,50993,50961,50929,
    50898,50866,50834,50803,50771,50739,50707,50675,
    50644,50612,50580,50548,50516,50484,50452,50420,
    50387,50355,50323,50291,50259,50226,50194,50162,
    50129,50097,50065,50032,50000,49967,49935,49902,
    49869,49837,49804,49771,49739,49706,49673,49640,
    49608,49575,49542,49509,49476,49443,49410,49377,
    49344,49311,49278,49244,49211,49178,49145,49112,
    49078,49045,49012,48978,48945,48911,48878,48844,
    48811,48777,48744,48710,48676,48643,48609,48575,
    48542,48508,48474,48440,48406,48372,48338,48304,
    48271,48237,48202,48168,48134,48100,48066,48032,
    47998,47963,47929,47895,47860,47826,47792,47757,
    47723,47688,47654,47619,47585,47550,47516,47481,
    47446,47412,47377,47342,47308,47273,47238,47203,
    47168,47133,47098,47063,47028,46993,46958,46923,
    46888,46853,46818,46783,46747,46712,46677,46642,
    46606,46571,46536,46500,46465,46429,46394,46358,
    46323,46287,46252,46216,46180,46145,46109,46073,
    46037,46002,45966,45930,45894,45858,45822,45786,
    45750,45714,45678,45642,45606,45570,45534,45498,
    45462,45425,45389,45353,45316,45280,45244,45207,
    45171,45135,45098,45062,45025,44989,44952,44915,
    44879,44842,44806,44769,44732,44695,44659,44622,
    44585,44548,44511,44474,44437,44400,44363,44326,
    44289,44252,44215,44178,44141,44104,44067,44029,
    43992,43955,43918,43880,43843,43806,43768,43731,
    43693,43656,43618,43581,43543,43506,43468,43430,
    43393,43355,43317,43280,43242,43204,43166,43128,
    43091,43053,43015,42977,42939,42901,42863,42825,
    42787,42749,42711,42672,42634,42596,42558,42520,
    42481,42443,42405,42366,42328,42290,42251,42213,
    42174,42136,42097,42059,42020,41982,41943,41904,
    41866,41827,41788,41750,41711,41672,41633,41595,
    41556,41517,41478,41439,41400,41361,41322,41283,
    41244,41205,41166,41127,41088,41048,41009,40970,
    40931,40891,40852,40813,40773,40734,40695,40655,
    40616,40576,40537,40497,40458,40418,40379,40339,
    40300,40260,40220,40180,40141,40101,40061,40021,
    39982,39942,39902,39862,39822,39782,39742,39702,
    39662,39622,39582,39542,39502,39462,39422,39382,
    39341,39301,39261,39221,39180,39140,39100,39059,
    39019,38979,38938,38898,38857,38817,38776,38736,
    38695,38655,38614,38573,38533,38492,38451,38411,
    38370,38329,38288,38248,38207,38166,38125,38084,
    38043,38002,37961,37920,37879,37838,37797,37756,
    37715,37674,37633,37592,37551,37509,37468,37427,
    37386,37344,37303,37262,37220,37179,37137,37096,
    37055,37013,36972,36930,36889,36847,36805,36764,
    36722,36681,36639,36597,36556,36514,36472,36430,
    36388,36347,36305,36263,36221,36179,36137,36095,
    36053,36011,35969,35927,35885,35843,35801,35759,
    35717,35675,35633,35590,35548,35506,35464,35421,
    35379,35337,35294,35252,35210,35167,35125,35082,
    35040,34997,34955,34912,34870,34827,34785,34742,
    34699,34657,34614,34571,34529,34486,34443,34400,
    34358,34315,34272,34229,34186,34143,34100,34057,
    34015,33972,33929,33886,33843,33799,33756,33713,
    33670,33627,33584,33541,33498,33454,33411,33368,
    33325,33281,33238,33195,33151,33108,33065,33021,
    32978,32934,32891,32847,32804,32760,32717,32673,
    32630,32586,32542,32499,32455,32411,32368,32324,
    32280,32236,32193,32149,32105,32061,32017,31974,
    31930,31886,31842,31798,31754,31710,31666,31622,
    31578,31534,31490,31446,31402,31357,31313,31269,
    31225,31181,31136,31092,31048,31004,30959,30915,
    30871,30826,30782,30738,30693,30649,30604,30560,
    30515,30471,30426,30382,30337,30293,30248,30204,
    30159,30114,30070,30025,29980,29936,29891,29846,
    29801,29757,29712,29667,29622,29577,29533,29488,
    29443,29398,29353,29308,29263,29218,29173,29128,
    29083,29038,28993,28948,28903,28858,28812,28767,
    28722,28677,28632,28586,28541,28496,28451,28405,
    28360,28315,28269,28224,28179,28133,28088,28042,
    27997,27952,27906,27861,27815,27770,27724,27678,
    27633,27587,27542,27496,27450,27405,27359,27313,
    27268,27222,27176,27131,27085,27039,26993,26947,
    26902,26856,26810,26764,26718,26672,26626,26580,
    26534,26488,26442,26396,26350,26304,26258,26212,
    26166,26120,26074,26028,25982,25936,25889,25843,
    25797,25751,25705,25658,25612,25566,25520,25473,
    25427,25381,25334,25288,25241,25195,25149,25102,
    25056,25009,24963,24916,24870,24823,24777,24730,
    24684,24637,24591,24544,24497,24451,24404,24357,
    24311,24264,24217,24171,24124,24077,24030,23984,
    23937,23890,23843,23796,23750,23703,23656,23609,
    23562,23515,23468,23421,23374,23327,23280,23233,
    23186,23139,23092,23045,22998,22951,22904,22857,
    22810,22763,22716,22668,22621,22574,22527,22480,
    22433,22385,22338,22291,22243,22196,22149,22102,
    22054,22007,21960,21912,21865,21817,21770,21723,
    21675,21628,21580,21533,21485,21438,21390,21343,
    21295,21248,21200,21153,21105,21057,21010,20962,
    20915,20867,20819,20772,20724,20676,20629,20581,
    20533,20485,20438,20390,20342,20294,20246,20199,
    20151,20103,20055,20007,19959,19912,19864,19816,
    19768,19720,19672,19624,19576,19528,19480,19432,
    19384,19336,19288,19240,19192,19144,19096,19048,
    19000,18951,18903,18855,18807,18759,18711,18663,
    18614,18566,18518,18470,18421,18373,18325,18277,
    18228,18180,18132,18084,18035,17987,17939,17890,
    17842,17793,17745,17697,17648,17600,17551,17503,
    17455,17406,17358,17309,17261,17212,17164,17115,
    17067,17018,16970,16921,16872,16824,16775,16727,
    16678,16629,16581,16532,16484,16435,16386,16338,
    16289,16240,16191,16143,16094,16045,15997,15948,
    15899,15850,15802,15753,15704,15655,15606,15557,
    15509,15460,15411,15362,15313,15264,15215,15167,
    15118,15069,15020,14971,14922,14873,14824,14775,
    14726,14677,14628,14579,14530,14481,14432,14383,
    14334,14285,14236,14187,14138,14089,14040,13990,
    13941,13892,13843,13794,13745,13696,13646,13597,
    13548,13499,13450,13401,13351,13302,13253,13204,
    13154,13105,13056,13007,12957,12908,12859,12810,
    12760,12711,12662,12612,12563,12514,12464,12415,
    12366,12316,12267,12218,12168,12119,12069,12020,
    11970,11921,11872,11822,11773,11723,11674,11624,
    11575,11525,11476,11426,11377,11327,11278,11228,
    11179,11129,11080,11030,10981,10931,10882,10832,
    10782,10733,10683,10634,10584,10534,10485,10435,
    10386,10336,10286,10237,10187,10137,10088,10038,
    9988,9939,9889,9839,9790,9740,9690,9640,
    9591,9541,9491,9442,9392,9342,9292,9243,
    9193,9143,9093,9043,8994,8944,8894,8844,
    8794,8745,8695,8645,8595,8545,8496,8446,
    8396,8346,8296,8246,8196,8147,8097,8047,
    7997,7947,7897,7847,7797,7747,7697,7648,
    7598,7548,7498,7448,7398,7348,7298,7248,
    7198,7148,7098,7048,6998,6948,6898,6848,
    6798,6748,6698,6648,6598,6548,6498,6448,
    6398,6348,6298,6248,6198,6148,6098,6048,
    5998,5948,5898,5848,5798,5748,5697,5647,
    5597,5547,5497,5447,5397,5347,5297,5247,
    5197,5146,5096,5046,4996,4946,4896,4846,
    4796,4745,4695,4645,4595,4545,4495,4445,
    4394,4344,4294,4244,4194,4144,4093,4043,
    3993,3943,3893,3843,3792,3742,3692,3642,
    3592,3541,3491,3441,3391,3341,3291,3240,
    3190,3140,3090,3039,2989,2939,2889,2839,
    2788,2738,2688,2638,2587,2537,2487,2437,
    2387,2336,2286,2236,2186,2135,2085,2035,
    1985,1934,1884,1834,1784,1733,1683,1633,
    1583,1532,1482,1432,1382,1331,1281,1231,
    1181,1130,1080,1030,980,929,879,829,
    779,728,678,628,578,527,477,427,
    376,326,276,226,175,125,75,25,
    -25,-75,-125,-175,-226,-276,-326,-376,
    -427,-477,-527,-578,-628,-678,-728,-779,
    -829,-879,-929,-980,-1030,-1080,-1130,-1181,
    -1231,-1281,-1331,-1382,-1432,-1482,-1532,-1583,
    -1633,-1683,-1733,-1784,-1834,-1884,-1934,-1985,
    -2035,-2085,-2135,-2186,-2236,-2286,-2336,-2387,
    -2437,-2487,-2537,-2588,-2638,-2688,-2738,-2788,
    -2839,-2889,-2939,-2989,-3039,-3090,-3140,-3190,
    -3240,-3291,-3341,-3391,-3441,-3491,-3541,-3592,
    -3642,-3692,-3742,-3792,-3843,-3893,-3943,-3993,
    -4043,-4093,-4144,-4194,-4244,-4294,-4344,-4394,
    -4445,-4495,-4545,-4595,-4645,-4695,-4745,-4796,
    -4846,-4896,-4946,-4996,-5046,-5096,-5146,-5197,
    -5247,-5297,-5347,-5397,-5447,-5497,-5547,-5597,
    -5647,-5697,-5748,-5798,-5848,-5898,-5948,-5998,
    -6048,-6098,-6148,-6198,-6248,-6298,-6348,-6398,
    -6448,-6498,-6548,-6598,-6648,-6698,-6748,-6798,
    -6848,-6898,-6948,-6998,-7048,-7098,-7148,-7198,
    -7248,-7298,-7348,-7398,-7448,-7498,-7548,-7598,
    -7648,-7697,-7747,-7797,-7847,-7897,-7947,-7997,
    -8047,-8097,-8147,-8196,-8246,-8296,-8346,-8396,
    -8446,-8496,-8545,-8595,-8645,-8695,-8745,-8794,
    -8844,-8894,-8944,-8994,-9043,-9093,-9143,-9193,
    -9243,-9292,-9342,-9392,-9442,-9491,-9541,-9591,
    -9640,-9690,-9740,-9790,-9839,-9889,-9939,-9988,
    -10038,-10088,-10137,-10187,-10237,-10286,-10336,-10386,
    -10435,-10485,-10534,-10584,-10634,-10683,-10733,-10782,
    -10832,-10882,-10931,-10981,-11030,-11080,-11129,-11179,
    -11228,-11278,-11327,-11377,-11426,-11476,-11525,-11575,
    -11624,-11674,-11723,-11773,-11822,-11872,-11921,-11970,
    -12020,-12069,-12119,-12168,-12218,-12267,-12316,-12366,
    -12415,-12464,-12514,-12563,-12612,-12662,-12711,-12760,
    -12810,-12859,-12908,-12957,-13007,-13056,-13105,-13154,
    -13204,-13253,-13302,-13351,-13401,-13450,-13499,-13548,
    -13597,-13647,-13696,-13745,-13794,-13843,-13892,-13941,
    -13990,-14040,-14089,-14138,-14187,-14236,-14285,-14334,
    -14383,-14432,-14481,-14530,-14579,-14628,-14677,-14726,
    -14775,-14824,-14873,-14922,-14971,-15020,-15069,-15118,
    -15167,-15215,-15264,-15313,-15362,-15411,-15460,-15509,
    -15557,-15606,-15655,-15704,-15753,-15802,-15850,-15899,
    -15948,-15997,-16045,-16094,-16143,-16191,-16240,-16289,
    -16338,-16386,-16435,-16484,-16532,-16581,-16629,-16678,
    -16727,-16775,-16824,-16872,-16921,-16970,-17018,-17067,
    -17115,-17164,-17212,-17261,-17309,-17358,-17406,-17455,
    -17503,-17551,-17600,-17648,-17697,-17745,-17793,-17842,
    -17890,-17939,-17987,-18035,-18084,-18132,-18180,-18228,
    -18277,-18325,-18373,-18421,-18470,-18518,-18566,-18614,
    -18663,-18711,-18759,-18807,-18855,-18903,-18951,-19000,
    -19048,-19096,-19144,-19192,-19240,-19288,-19336,-19384,
    -19432,-19480,-19528,-19576,-19624,-19672,-19720,-19768,
    -19816,-19864,-19912,-19959,-20007,-20055,-20103,-20151,
    -20199,-20246,-20294,-20342,-20390,-20438,-20485,-20533,
    -20581,-20629,-20676,-20724,-20772,-20819,-20867,-20915,
    -20962,-21010,-21057,-21105,-21153,-21200,-21248,-21295,
    -21343,-21390,-21438,-21485,-21533,-21580,-21628,-21675,
    -21723,-21770,-21817,-21865,-21912,-21960,-22007,-22054,
    -22102,-22149,-22196,-22243,-22291,-22338,-22385,-22433,
    -22480,-22527,-22574,-22621,-22668,-22716,-22763,-22810,
    -22857,-22904,-22951,-22998,-23045,-23092,-23139,-23186,
    -23233,-23280,-23327,-23374,-23421,-23468,-23515,-23562,
    -23609,-23656,-23703,-23750,-23796,-23843,-23890,-23937,
    -23984,-24030,-24077,-24124,-24171,-24217,-24264,-24311,
    -24357,-24404,-24451,-24497,-24544,-24591,-24637,-24684,
    -24730,-24777,-24823,-24870,-24916,-24963,-25009,-25056,
    -25102,-25149,-25195,-25241,-25288,-25334,-25381,-25427,
    -25473,-25520,-25566,-25612,-25658,-25705,-25751,-25797,
    -25843,-25889,-25936,-25982,-26028,-26074,-26120,-26166,
    -26212,-26258,-26304,-26350,-26396,-26442,-26488,-26534,
    -26580,-26626,-26672,-26718,-26764,-26810,-26856,-26902,
    -26947,-26993,-27039,-27085,-27131,-27176,-27222,-27268,
    -27313,-27359,-27405,-27450,-27496,-27542,-27587,-27633,
    -27678,-27724,-27770,-27815,-27861,-27906,-27952,-27997,
    -28042,-28088,-28133,-28179,-28224,-28269,-28315,-28360,
    -28405,-28451,-28496,-28541,-28586,-28632,-28677,-28722,
    -28767,-28812,-28858,-28903,-28948,-28993,-29038,-29083,
    -29128,-29173,-29218,-29263,-29308,-29353,-29398,-29443,
    -29488,-29533,-29577,-29622,-29667,-29712,-29757,-29801,
    -29846,-29891,-29936,-29980,-30025,-30070,-30114,-30159,
    -30204,-30248,-30293,-30337,-30382,-30426,-30471,-30515,
    -30560,-30604,-30649,-30693,-30738,-30782,-30826,-30871,
    -30915,-30959,-31004,-31048,-31092,-31136,-31181,-31225,
    -31269,-31313,-31357,-31402,-31446,-31490,-31534,-31578,
    -31622,-31666,-31710,-31754,-31798,-31842,-31886,-31930,
    -31974,-32017,-32061,-32105,-32149,-32193,-32236,-32280,
    -32324,-32368,-32411,-32455,-32499,-32542,-32586,-32630,
    -32673,-32717,-32760,-32804,-32847,-32891,-32934,-32978,
    -33021,-33065,-33108,-33151,-33195,-33238,-33281,-33325,
    -33368,-33411,-33454,-33498,-33541,-33584,-33627,-33670,
    -33713,-33756,-33799,-33843,-33886,-33929,-33972,-34015,
    -34057,-34100,-34143,-34186,-34229,-34272,-34315,-34358,
    -34400,-34443,-34486,-34529,-34571,-34614,-34657,-34699,
    -34742,-34785,-34827,-34870,-34912,-34955,-34997,-35040,
    -35082,-35125,-35167,-35210,-35252,-35294,-35337,-35379,
    -35421,-35464,-35506,-35548,-35590,-35633,-35675,-35717,
    -35759,-35801,-35843,-35885,-35927,-35969,-36011,-36053,
    -36095,-36137,-36179,-36221,-36263,-36305,-36347,-36388,
    -36430,-36472,-36514,-36555,-36597,-36639,-36681,-36722,
    -36764,-36805,-36847,-36889,-36930,-36972,-37013,-37055,
    -37096,-37137,-37179,-37220,-37262,-37303,-37344,-37386,
    -37427,-37468,-37509,-37551,-37592,-37633,-37674,-37715,
    -37756,-37797,-37838,-37879,-37920,-37961,-38002,-38043,
    -38084,-38125,-38166,-38207,-38248,-38288,-38329,-38370,
    -38411,-38451,-38492,-38533,-38573,-38614,-38655,-38695,
    -38736,-38776,-38817,-38857,-38898,-38938,-38979,-39019,
    -39059,-39100,-39140,-39180,-39221,-39261,-39301,-39341,
    -39382,-39422,-39462,-39502,-39542,-39582,-39622,-39662,
    -39702,-39742,-39782,-39822,-39862,-39902,-39942,-39982,
    -40021,-40061,-40101,-40141,-40180,-40220,-40260,-40299,
    -40339,-40379,-40418,-40458,-40497,-40537,-40576,-40616,
    -40655,-40695,-40734,-40773,-40813,-40852,-40891,-40931,
    -40970,-41009,-41048,-41087,-41127,-41166,-41205,-41244,
    -41283,-41322,-41361,-41400,-41439,-41478,-41517,-41556,
    -41595,-41633,-41672,-41711,-41750,-41788,-41827,-41866,
    -41904,-41943,-41982,-42020,-42059,-42097,-42136,-42174,
    -42213,-42251,-42290,-42328,-42366,-42405,-42443,-42481,
    -42520,-42558,-42596,-42634,-42672,-42711,-42749,-42787,
    -42825,-42863,-42901,-42939,-42977,-43015,-43053,-43091,
    -43128,-43166,-43204,-43242,-43280,-43317,-43355,-43393,
    -43430,-43468,-43506,-43543,-43581,-43618,-43656,-43693,
    -43731,-43768,-43806,-43843,-43880,-43918,-43955,-43992,
    -44029,-44067,-44104,-44141,-44178,-44215,-44252,-44289,
    -44326,-44363,-44400,-44437,-44474,-44511,-44548,-44585,
    -44622,-44659,-44695,-44732,-44769,-44806,-44842,-44879,
    -44915,-44952,-44989,-45025,-45062,-45098,-45135,-45171,
    -45207,-45244,-45280,-45316,-45353,-45389,-45425,-45462,
    -45498,-45534,-45570,-45606,-45642,-45678,-45714,-45750,
    -45786,-45822,-45858,-45894,-45930,-45966,-46002,-46037,
    -46073,-46109,-46145,-46180,-46216,-46252,-46287,-46323,
    -46358,-46394,-46429,-46465,-46500,-46536,-46571,-46606,
    -46642,-46677,-46712,-46747,-46783,-46818,-46853,-46888,
    -46923,-46958,-46993,-47028,-47063,-47098,-47133,-47168,
    -47203,-47238,-47273,-47308,-47342,-47377,-47412,-47446,
    -47481,-47516,-47550,-47585,-47619,-47654,-47688,-47723,
    -47757,-47792,-47826,-47860,-47895,-47929,-47963,-47998,
    -48032,-48066,-48100,-48134,-48168,-48202,-48236,-48271,
    -48304,-48338,-48372,-48406,-48440,-48474,-48508,-48542,
    -48575,-48609,-48643,-48676,-48710,-48744,-48777,-48811,
    -48844,-48878,-48911,-48945,-48978,-49012,-49045,-49078,
    -49112,-49145,-49178,-49211,-49244,-49278,-49311,-49344,
    -49377,-49410,-49443,-49476,-49509,-49542,-49575,-49608,
    -49640,-49673,-49706,-49739,-49771,-49804,-49837,-49869,
    -49902,-49935,-49967,-50000,-50032,-50065,-50097,-50129,
    -50162,-50194,-50226,-50259,-50291,-50323,-50355,-50387,
    -50420,-50452,-50484,-50516,-50548,-50580,-50612,-50644,
    -50675,-50707,-50739,-50771,-50803,-50834,-50866,-50898,
    -50929,-50961,-50993,-51024,-51056,-51087,-51119,-51150,
    -51182,-51213,-51244,-51276,-51307,-51338,-51369,-51401,
    -51432,-51463,-51494,-51525,-51556,-51587,-51618,-51649,
    -51680,-51711,-51742,-51773,-51803,-51834,-51865,-51896,
    -51926,-51957,-51988,-52018,-52049,-52079,-52110,-52140,
    -52171,-52201,-52231,-52262,-52292,-52322,-52353,-52383,
    -52413,-52443,-52473,-52503,-52534,-52564,-52594,-52624,
    -52653,-52683,-52713,-52743,-52773,-52803,-52832,-52862,
    -52892,-52922,-52951,-52981,-53010,-53040,-53069,-53099,
    -53128,-53158,-53187,-53216,-53246,-53275,-53304,-53334,
    -53363,-53392,-53421,-53450,-53479,-53508,-53537,-53566,
    -53595,-53624,-53653,-53682,-53711,-53739,-53768,-53797,
    -53826,-53854,-53883,-53911,-53940,-53969,-53997,-54026,
    -54054,-54082,-54111,-54139,-54167,-54196,-54224,-54252,
    -54280,-54308,-54337,-54365,-54393,-54421,-54449,-54477,
    -54505,-54533,-54560,-54588,-54616,-54644,-54672,-54699,
    -54727,-54755,-54782,-54810,-54837,-54865,-54892,-54920,
    -54947,-54974,-55002,-55029,-55056,-55084,-55111,-55138,
    -55165,-55192,-55219,-55246,-55274,-55300,-55327,-55354,
    -55381,-55408,-55435,-55462,-55489,-55515,-55542,-55569,
    -55595,-55622,-55648,-55675,-55701,-55728,-55754,-55781,
    -55807,-55833,-55860,-55886,-55912,-55938,-55965,-55991,
    -56017,-56043,-56069,-56095,-56121,-56147,-56173,-56199,
    -56225,-56250,-56276,-56302,-56328,-56353,-56379,-56404,
    -56430,-56456,-56481,-56507,-56532,-56557,-56583,-56608,
    -56633,-56659,-56684,-56709,-56734,-56760,-56785,-56810,
    -56835,-56860,-56885,-56910,-56935,-56959,-56984,-57009,
    -57034,-57059,-57083,-57108,-57133,-57157,-57182,-57206,
    -57231,-57255,-57280,-57304,-57329,-57353,-57377,-57402,
    -57426,-57450,-57474,-57498,-57522,-57546,-57570,-57594,
    -57618,-57642,-57666,-57690,-57714,-57738,-57762,-57785,
    -57809,-57833,-57856,-57880,-57903,-57927,-57950,-57974,
    -57997,-58021,-58044,-58067,-58091,-58114,-58137,-58160,
    -58183,-58207,-58230,-58253,-58276,-58299,-58322,-58345,
    -58367,-58390,-58413,-58436,-58459,-58481,-58504,-58527,
    -58549,-58572,-58594,-58617,-58639,-58662,-58684,-58706,
    -58729,-58751,-58773,-58795,-58818,-58840,-58862,-58884,
    -58906,-58928,-58950,-58972,-58994,-59016,-59038,-59059,
    -59081,-59103,-59125,-59146,-59168,-59190,-59211,-59233,
    -59254,-59276,-59297,-59318,-59340,-59361,-59382,-59404,
    -59425,-59446,-59467,-59488,-59509,-59530,-59551,-59572,
    -59593,-59614,-59635,-59656,-59677,-59697,-59718,-59739,
    -59759,-59780,-59801,-59821,-59842,-59862,-59883,-59903,
    -59923,-59944,-59964,-59984,-60004,-60025,-60045,-60065,
    -60085,-60105,-60125,-60145,-60165,-60185,-60205,-60225,
    -60244,-60264,-60284,-60304,-60323,-60343,-60363,-60382,
    -60402,-60421,-60441,-60460,-60479,-60499,-60518,-60537,
    -60556,-60576,-60595,-60614,-60633,-60652,-60671,-60690,
    -60709,-60728,-60747,-60766,-60785,-60803,-60822,-60841,
    -60859,-60878,-60897,-60915,-60934,-60952,-60971,-60989,
    -61007,-61026,-61044,-61062,-61081,-61099,-61117,-61135,
    -61153,-61171,-61189,-61207,-61225,-61243,-61261,-61279,
    -61297,-61314,-61332,-61350,-61367,-61385,-61403,-61420,
    -61438,-61455,-61473,-61490,-61507,-61525,-61542,-61559,
    -61577,-61594,-61611,-61628,-61645,-61662,-61679,-61696,
    -61713,-61730,-61747,-61764,-61780,-61797,-61814,-61831,
    -61847,-61864,-61880,-61897,-61913,-61930,-61946,-61963,
    -61979,-61995,-62012,-62028,-62044,-62060,-62076,-62092,
    -62108,-62125,-62141,-62156,-62172,-62188,-62204,-62220,
    -62236,-62251,-62267,-62283,-62298,-62314,-62329,-62345,
    -62360,-62376,-62391,-62407,-62422,-62437,-62453,-62468,
    -62483,-62498,-62513,-62528,-62543,-62558,-62573,-62588,
    -62603,-62618,-62633,-62648,-62662,-62677,-62692,-62706,
    -62721,-62735,-62750,-62764,-62779,-62793,-62808,-62822,
    -62836,-62850,-62865,-62879,-62893,-62907,-62921,-62935,
    -62949,-62963,-62977,-62991,-63005,-63019,-63032,-63046,
    -63060,-63074,-63087,-63101,-63114,-63128,-63141,-63155,
    -63168,-63182,-63195,-63208,-63221,-63235,-63248,-63261,
    -63274,-63287,-63300,-63313,-63326,-63339,-63352,-63365,
    -63378,-63390,-63403,-63416,-63429,-63441,-63454,-63466,
    -63479,-63491,-63504,-63516,-63528,-63541,-63553,-63565,
    -63578,-63590,-63602,-63614,-63626,-63638,-63650,-63662,
    -63674,-63686,-63698,-63709,-63721,-63733,-63745,-63756,
    -63768,-63779,-63791,-63803,-63814,-63825,-63837,-63848,
    -63859,-63871,-63882,-63893,-63904,-63915,-63927,-63938,
    -63949,-63960,-63971,-63981,-63992,-64003,-64014,-64025,
    -64035,-64046,-64057,-64067,-64078,-64088,-64099,-64109,
    -64120,-64130,-64140,-64151,-64161,-64171,-64181,-64192,
    -64202,-64212,-64222,-64232,-64242,-64252,-64261,-64271,
    -64281,-64291,-64301,-64310,-64320,-64330,-64339,-64349,
    -64358,-64368,-64377,-64387,-64396,-64405,-64414,-64424,
    -64433,-64442,-64451,-64460,-64469,-64478,-64487,-64496,
    -64505,-64514,-64523,-64532,-64540,-64549,-64558,-64566,
    -64575,-64584,-64592,-64601,-64609,-64617,-64626,-64634,
    -64642,-64651,-64659,-64667,-64675,-64683,-64691,-64699,
    -64707,-64715,-64723,-64731,-64739,-64747,-64754,-64762,
    -64770,-64777,-64785,-64793,-64800,-64808,-64815,-64822,
    -64830,-64837,-64844,-64852,-64859,-64866,-64873,-64880,
    -64887,-64895,-64902,-64908,-64915,-64922,-64929,-64936,
    -64943,-64949,-64956,-64963,-64969,-64976,-64982,-64989,
    -64995,-65002,-65008,-65015,-65021,-65027,-65033,-65040,
    -65046,-65052,-65058,-65064,-65070,-65076,-65082,-65088,
    -65094,-65099,-65105,-65111,-65117,-65122,-65128,-65133,
    -65139,-65144,-65150,-65155,-65161,-65166,-65171,-65177,
    -65182,-65187,-65192,-65197,-65202,-65207,-65212,-65217,
    -65222,-65227,-65232,-65237,-65242,-65246,-65251,-65256,
    -65260,-65265,-65270,-65274,-65279,-65283,-65287,-65292,
    -65296,-65300,-65305,-65309,-65313,-65317,-65321,-65325,
    -65329,-65333,-65337,-65341,-65345,-65349,-65352,-65356,
    -65360,-65363,-65367,-65371,-65374,-65378,-65381,-65385,
    -65388,-65391,-65395,-65398,-65401,-65404,-65408,-65411,
    -65414,-65417,-65420,-65423,-65426,-65429,-65431,-65434,
    -65437,-65440,-65442,-65445,-65448,-65450,-65453,-65455,
    -65458,-65460,-65463,-65465,-65467,-65470,-65472,-65474,
    -65476,-65478,-65480,-65482,-65484,-65486,-65488,-65490,
    -65492,-65494,-65496,-65497,-65499,-65501,-65502,-65504,
    -65505,-65507,-65508,-65510,-65511,-65513,-65514,-65515,
    -65516,-65518,-65519,-65520,-65521,-65522,-65523,-65524,
    -65525,-65526,-65527,-65527,-65528,-65529,-65530,-65530,
    -65531,-65531,-65532,-65532,-65533,-65533,-65534,-65534,
    -65534,-65535,-65535,-65535,-65535,-65535,-65535,-65535,
    -65535,-65535,-65535,-65535,-65535,-65535,-65535,-65534,
    -65534,-65534,-65533,-65533,-65532,-65532,-65531,-65531,
    -65530,-65530,-65529,-65528,-65527,-65527,-65526,-65525,
    -65524,-65523,-65522,-65521,-65520,-65519,-65518,-65516,
    -65515,-65514,-65513,-65511,-65510,-65508,-65507,-65505,
    -65504,-65502,-65501,-65499,-65497,-65496,-65494,-65492,
    -65490,-65488,-65486,-65484,-65482,-65480,-65478,-65476,
    -65474,-65472,-65470,-65467,-65465,-65463,-65460,-65458,
    -65455,-65453,-65450,-65448,-65445,-65442,-65440,-65437,
    -65434,-65431,-65429,-65426,-65423,-65420,-65417,-65414,
    -65411,-65408,-65404,-65401,-65398,-65395,-65391,-65388,
    -65385,-65381,-65378,-65374,-65371,-65367,-65363,-65360,
    -65356,-65352,-65349,-65345,-65341,-65337,-65333,-65329,
    -65325,-65321,-65317,-65313,-65309,-65305,-65300,-65296,
    -65292,-65287,-65283,-65279,-65274,-65270,-65265,-65260,
    -65256,-65251,-65246,-65242,-65237,-65232,-65227,-65222,
    -65217,-65212,-65207,-65202,-65197,-65192,-65187,-65182,
    -65177,-65171,-65166,-65161,-65155,-65150,-65144,-65139,
    -65133,-65128,-65122,-65117,-65111,-65105,-65099,-65094,
    -65088,-65082,-65076,-65070,-65064,-65058,-65052,-65046,
    -65040,-65033,-65027,-65021,-65015,-65008,-65002,-64995,
    -64989,-64982,-64976,-64969,-64963,-64956,-64949,-64943,
    -64936,-64929,-64922,-64915,-64908,-64902,-64895,-64887,
    -64880,-64873,-64866,-64859,-64852,-64844,-64837,-64830,
    -64822,-64815,-64808,-64800,-64793,-64785,-64777,-64770,
    -64762,-64754,-64747,-64739,-64731,-64723,-64715,-64707,
    -64699,-64691,-64683,-64675,-64667,-64659,-64651,-64642,
    -64634,-64626,-64617,-64609,-64601,-64592,-64584,-64575,
    -64566,-64558,-64549,-64540,-64532,-64523,-64514,-64505,
    -64496,-64487,-64478,-64469,-64460,-64451,-64442,-64433,
    -64424,-64414,-64405,-64396,-64387,-64377,-64368,-64358,
    -64349,-64339,-64330,-64320,-64310,-64301,-64291,-64281,
    -64271,-64261,-64252,-64242,-64232,-64222,-64212,-64202,
    -64192,-64181,-64171,-64161,-64151,-64140,-64130,-64120,
    -64109,-64099,-64088,-64078,-64067,-64057,-64046,-64035,
    -64025,-64014,-64003,-63992,-63981,-63971,-63960,-63949,
    -63938,-63927,-63915,-63904,-63893,-63882,-63871,-63859,
    -63848,-63837,-63825,-63814,-63803,-63791,-63779,-63768,
    -63756,-63745,-63733,-63721,-63709,-63698,-63686,-63674,
    -63662,-63650,-63638,-63626,-63614,-63602,-63590,-63578,
    -63565,-63553,-63541,-63528,-63516,-63504,-63491,-63479,
    -63466,-63454,-63441,-63429,-63416,-63403,-63390,-63378,
    -63365,-63352,-63339,-63326,-63313,-63300,-63287,-63274,
    -63261,-63248,-63235,-63221,-63208,-63195,-63182,-63168,
    -63155,-63141,-63128,-63114,-63101,-63087,-63074,-63060,
    -63046,-63032,-63019,-63005,-62991,-62977,-62963,-62949,
    -62935,-62921,-62907,-62893,-62879,-62865,-62850,-62836,
    -62822,-62808,-62793,-62779,-62764,-62750,-62735,-62721,
    -62706,-62692,-62677,-62662,-62648,-62633,-62618,-62603,
    -62588,-62573,-62558,-62543,-62528,-62513,-62498,-62483,
    -62468,-62453,-62437,-62422,-62407,-62391,-62376,-62360,
    -62345,-62329,-62314,-62298,-62283,-62267,-62251,-62236,
    -62220,-62204,-62188,-62172,-62156,-62141,-62125,-62108,
    -62092,-62076,-62060,-62044,-62028,-62012,-61995,-61979,
    -61963,-61946,-61930,-61913,-61897,-61880,-61864,-61847,
    -61831,-61814,-61797,-61780,-61764,-61747,-61730,-61713,
    -61696,-61679,-61662,-61645,-61628,-61611,-61594,-61577,
    -61559,-61542,-61525,-61507,-61490,-61473,-61455,-61438,
    -61420,-61403,-61385,-61367,-61350,-61332,-61314,-61297,
    -61279,-61261,-61243,-61225,-61207,-61189,-61171,-61153,
    -61135,-61117,-61099,-61081,-61062,-61044,-61026,-61007,
    -60989,-60971,-60952,-60934,-60915,-60897,-60878,-60859,
    -60841,-60822,-60803,-60785,-60766,-60747,-60728,-60709,
    -60690,-60671,-60652,-60633,-60614,-60595,-60576,-60556,
    -60537,-60518,-60499,-60479,-60460,-60441,-60421,-60402,
    -60382,-60363,-60343,-60323,-60304,-60284,-60264,-60244,
    -60225,-60205,-60185,-60165,-60145,-60125,-60105,-60085,
    -60065,-60045,-60025,-60004,-59984,-59964,-59944,-59923,
    -59903,-59883,-59862,-59842,-59821,-59801,-59780,-59759,
    -59739,-59718,-59697,-59677,-59656,-59635,-59614,-59593,
    -59572,-59551,-59530,-59509,-59488,-59467,-59446,-59425,
    -59404,-59382,-59361,-59340,-59318,-59297,-59276,-59254,
    -59233,-59211,-59189,-59168,-59146,-59125,-59103,-59081,
    -59059,-59038,-59016,-58994,-58972,-58950,-58928,-58906,
    -58884,-58862,-58840,-58818,-58795,-58773,-58751,-58729,
    -58706,-58684,-58662,-58639,-58617,-58594,-58572,-58549,
    -58527,-58504,-58481,-58459,-58436,-58413,-58390,-58367,
    -58345,-58322,-58299,-58276,-58253,-58230,-58207,-58183,
    -58160,-58137,-58114,-58091,-58067,-58044,-58021,-57997,
    -57974,-57950,-57927,-57903,-57880,-57856,-57833,-57809,
    -57785,-57762,-57738,-57714,-57690,-57666,-57642,-57618,
    -57594,-57570,-57546,-57522,-57498,-57474,-57450,-57426,
    -57402,-57377,-57353,-57329,-57304,-57280,-57255,-57231,
    -57206,-57182,-57157,-57133,-57108,-57083,-57059,-57034,
    -57009,-56984,-56959,-56935,-56910,-56885,-56860,-56835,
    -56810,-56785,-56760,-56734,-56709,-56684,-56659,-56633,
    -56608,-56583,-56557,-56532,-56507,-56481,-56456,-56430,
    -56404,-56379,-56353,-56328,-56302,-56276,-56250,-56225,
    -56199,-56173,-56147,-56121,-56095,-56069,-56043,-56017,
    -55991,-55965,-55938,-55912,-55886,-55860,-55833,-55807,
    -55781,-55754,-55728,-55701,-55675,-55648,-55622,-55595,
    -55569,-55542,-55515,-55489,-55462,-55435,-55408,-55381,
    -55354,-55327,-55300,-55274,-55246,-55219,-55192,-55165,
    -55138,-55111,-55084,-55056,-55029,-55002,-54974,-54947,
    -54920,-54892,-54865,-54837,-54810,-54782,-54755,-54727,
    -54699,-54672,-54644,-54616,-54588,-54560,-54533,-54505,
    -54477,-54449,-54421,-54393,-54365,-54337,-54308,-54280,
    -54252,-54224,-54196,-54167,-54139,-54111,-54082,-54054,
    -54026,-53997,-53969,-53940,-53911,-53883,-53854,-53826,
    -53797,-53768,-53739,-53711,-53682,-53653,-53624,-53595,
    -53566,-53537,-53508,-53479,-53450,-53421,-53392,-53363,
    -53334,-53304,-53275,-53246,-53216,-53187,-53158,-53128,
    -53099,-53069,-53040,-53010,-52981,-52951,-52922,-52892,
    -52862,-52832,-52803,-52773,-52743,-52713,-52683,-52653,
    -52624,-52594,-52564,-52534,-52503,-52473,-52443,-52413,
    -52383,-52353,-52322,-52292,-52262,-52231,-52201,-52171,
    -52140,-52110,-52079,-52049,-52018,-51988,-51957,-51926,
    -51896,-51865,-51834,-51803,-51773,-51742,-51711,-51680,
    -51649,-51618,-51587,-51556,-51525,-51494,-51463,-51432,
    -51401,-51369,-51338,-51307,-51276,-51244,-51213,-51182,
    -51150,-51119,-51087,-51056,-51024,-50993,-50961,-50929,
    -50898,-50866,-50834,-50803,-50771,-50739,-50707,-50675,
    -50644,-50612,-50580,-50548,-50516,-50484,-50452,-50420,
    -50387,-50355,-50323,-50291,-50259,-50226,-50194,-50162,
    -50129,-50097,-50065,-50032,-50000,-49967,-49935,-49902,
    -49869,-49837,-49804,-49771,-49739,-49706,-49673,-49640,
    -49608,-49575,-49542,-49509,-49476,-49443,-49410,-49377,
    -49344,-49311,-49278,-49244,-49211,-49178,-49145,-49112,
    -49078,-49045,-49012,-48978,-48945,-48911,-48878,-48844,
    -48811,-48777,-48744,-48710,-48676,-48643,-48609,-48575,
    -48542,-48508,-48474,-48440,-48406,-48372,-48338,-48305,
    -48271,-48237,-48202,-48168,-48134,-48100,-48066,-48032,
    -47998,-47963,-47929,-47895,-47860,-47826,-47792,-47757,
    -47723,-47688,-47654,-47619,-47585,-47550,-47516,-47481,
    -47446,-47412,-47377,-47342,-47307,-47273,-47238,-47203,
    -47168,-47133,-47098,-47063,-47028,-46993,-46958,-46923,
    -46888,-46853,-46818,-46783,-46747,-46712,-46677,-46642,
    -46606,-46571,-46536,-46500,-46465,-46429,-46394,-46358,
    -46323,-46287,-46251,-46216,-46180,-46145,-46109,-46073,
    -46037,-46002,-45966,-45930,-45894,-45858,-45822,-45786,
    -45750,-45714,-45678,-45642,-45606,-45570,-45534,-45498,
    -45462,-45425,-45389,-45353,-45316,-45280,-45244,-45207,
    -45171,-45135,-45098,-45062,-45025,-44989,-44952,-44915,
    -44879,-44842,-44806,-44769,-44732,-44695,-44659,-44622,
    -44585,-44548,-44511,-44474,-44437,-44400,-44363,-44326,
    -44289,-44252,-44215,-44178,-44141,-44104,-44067,-44029,
    -43992,-43955,-43918,-43880,-43843,-43806,-43768,-43731,
    -43693,-43656,-43618,-43581,-43543,-43506,-43468,-43430,
    -43393,-43355,-43317,-43280,-43242,-43204,-43166,-43128,
    -43091,-43053,-43015,-42977,-42939,-42901,-42863,-42825,
    -42787,-42749,-42711,-42672,-42634,-42596,-42558,-42520,
    -42481,-42443,-42405,-42366,-42328,-42290,-42251,-42213,
    -42174,-42136,-42097,-42059,-42020,-41982,-41943,-41904,
    -41866,-41827,-41788,-41750,-41711,-41672,-41633,-41595,
    -41556,-41517,-41478,-41439,-41400,-41361,-41322,-41283,
    -41244,-41205,-41166,-41127,-41087,-41048,-41009,-40970,
    -40931,-40891,-40852,-40813,-40773,-40734,-40695,-40655,
    -40616,-40576,-40537,-40497,-40458,-40418,-40379,-40339,
    -40299,-40260,-40220,-40180,-40141,-40101,-40061,-40021,
    -39982,-39942,-39902,-39862,-39822,-39782,-39742,-39702,
    -39662,-39622,-39582,-39542,-39502,-39462,-39422,-39382,
    -39341,-39301,-39261,-39221,-39180,-39140,-39100,-39059,
    -39019,-38979,-38938,-38898,-38857,-38817,-38776,-38736,
    -38695,-38655,-38614,-38573,-38533,-38492,-38451,-38411,
    -38370,-38329,-38288,-38248,-38207,-38166,-38125,-38084,
    -38043,-38002,-37961,-37920,-37879,-37838,-37797,-37756,
    -37715,-37674,-37633,-37592,-37550,-37509,-37468,-37427,
    -37386,-37344,-37303,-37262,-37220,-37179,-37137,-37096,
    -37055,-37013,-36972,-36930,-36889,-36847,-36805,-36764,
    -36722,-36681,-36639,-36597,-36556,-36514,-36472,-36430,
    -36388,-36347,-36305,-36263,-36221,-36179,-36137,-36095,
    -36053,-36011,-35969,-35927,-35885,-35843,-35801,-35759,
    -35717,-35675,-35633,-35590,-35548,-35506,-35464,-35421,
    -35379,-35337,-35294,-35252,-35210,-35167,-35125,-35082,
    -35040,-34997,-34955,-34912,-34870,-34827,-34785,-34742,
    -34699,-34657,-34614,-34571,-34529,-34486,-34443,-34400,
    -34358,-34315,-34272,-34229,-34186,-34143,-34100,-34057,
    -34015,-33972,-33929,-33886,-33843,-33799,-33756,-33713,
    -33670,-33627,-33584,-33541,-33498,-33454,-33411,-33368,
    -33325,-33281,-33238,-33195,-33151,-33108,-33065,-33021,
    -32978,-32934,-32891,-32847,-32804,-32760,-32717,-32673,
    -32630,-32586,-32542,-32499,-32455,-32411,-32368,-32324,
    -32280,-32236,-32193,-32149,-32105,-32061,-32017,-31974,
    -31930,-31886,-31842,-31798,-31754,-31710,-31666,-31622,
    -31578,-31534,-31490,-31446,-31402,-31357,-31313,-31269,
    -31225,-31181,-31136,-31092,-31048,-31004,-30959,-30915,
    -30871,-30826,-30782,-30738,-30693,-30649,-30604,-30560,
    -30515,-30471,-30426,-30382,-30337,-30293,-30248,-30204,
    -30159,-30114,-30070,-30025,-29980,-29936,-29891,-29846,
    -29801,-29757,-29712,-29667,-29622,-29577,-29533,-29488,
    -29443,-29398,-29353,-29308,-29263,-29218,-29173,-29128,
    -29083,-29038,-28993,-28948,-28903,-28858,-28812,-28767,
    -28722,-28677,-28632,-28586,-28541,-28496,-28451,-28405,
    -28360,-28315,-28269,-28224,-28179,-28133,-28088,-28042,
    -27997,-27952,-27906,-27861,-27815,-27770,-27724,-27678,
    -27633,-27587,-27542,-27496,-27450,-27405,-27359,-27313,
    -27268,-27222,-27176,-27131,-27085,-27039,-26993,-26947,
    -26902,-26856,-26810,-26764,-26718,-26672,-26626,-26580,
    -26534,-26488,-26442,-26396,-26350,-26304,-26258,-26212,
    -26166,-26120,-26074,-26028,-25982,-25936,-25889,-25843,
    -25797,-25751,-25705,-25658,-25612,-25566,-25520,-25473,
    -25427,-25381,-25334,-25288,-25241,-25195,-25149,-25102,
    -25056,-25009,-24963,-24916,-24870,-24823,-24777,-24730,
    -24684,-24637,-24591,-24544,-24497,-24451,-24404,-24357,
    -24311,-24264,-24217,-24171,-24124,-24077,-24030,-23984,
    -23937,-23890,-23843,-23796,-23750,-23703,-23656,-23609,
    -23562,-23515,-23468,-23421,-23374,-23327,-23280,-23233,
    -23186,-23139,-23092,-23045,-22998,-22951,-22904,-22857,
    -22810,-22763,-22716,-22668,-22621,-22574,-22527,-22480,
    -22432,-22385,-22338,-22291,-22243,-22196,-22149,-22102,
    -22054,-22007,-21960,-21912,-21865,-21817,-21770,-21723,
    -21675,-21628,-21580,-21533,-21485,-21438,-21390,-21343,
    -21295,-21248,-21200,-21153,-21105,-21057,-21010,-20962,
    -20915,-20867,-20819,-20772,-20724,-20676,-20629,-20581,
    -20533,-20485,-20438,-20390,-20342,-20294,-20246,-20199,
    -20151,-20103,-20055,-20007,-19959,-19912,-19864,-19816,
    -19768,-19720,-19672,-19624,-19576,-19528,-19480,-19432,
    -19384,-19336,-19288,-19240,-19192,-19144,-19096,-19048,
    -19000,-18951,-18903,-18855,-18807,-18759,-18711,-18663,
    -18614,-18566,-18518,-18470,-18421,-18373,-18325,-18277,
    -18228,-18180,-18132,-18084,-18035,-17987,-17939,-17890,
    -17842,-17793,-17745,-17697,-17648,-17600,-17551,-17503,
    -17455,-17406,-17358,-17309,-17261,-17212,-17164,-17115,
    -17067,-17018,-16970,-16921,-16872,-16824,-16775,-16727,
    -16678,-16629,-16581,-16532,-16484,-16435,-16386,-16338,
    -16289,-16240,-16191,-16143,-16094,-16045,-15997,-15948,
    -15899,-15850,-15802,-15753,-15704,-15655,-15606,-15557,
    -15509,-15460,-15411,-15362,-15313,-15264,-15215,-15167,
    -15118,-15069,-15020,-14971,-14922,-14873,-14824,-14775,
    -14726,-14677,-14628,-14579,-14530,-14481,-14432,-14383,
    -14334,-14285,-14236,-14187,-14138,-14089,-14040,-13990,
    -13941,-13892,-13843,-13794,-13745,-13696,-13647,-13597,
    -13548,-13499,-13450,-13401,-13351,-13302,-13253,-13204,
    -13154,-13105,-13056,-13007,-12957,-12908,-12859,-12810,
    -12760,-12711,-12662,-12612,-12563,-12514,-12464,-12415,
    -12366,-12316,-12267,-12217,-12168,-12119,-12069,-12020,
    -11970,-11921,-11872,-11822,-11773,-11723,-11674,-11624,
    -11575,-11525,-11476,-11426,-11377,-11327,-11278,-11228,
    -11179,-11129,-11080,-11030,-10981,-10931,-10882,-10832,
    -10782,-10733,-10683,-10634,-10584,-10534,-10485,-10435,
    -10386,-10336,-10286,-10237,-10187,-10137,-10088,-10038,
    -9988,-9939,-9889,-9839,-9790,-9740,-9690,-9640,
    -9591,-9541,-9491,-9442,-9392,-9342,-9292,-9243,
    -9193,-9143,-9093,-9043,-8994,-8944,-8894,-8844,
    -8794,-8745,-8695,-8645,-8595,-8545,-8496,-8446,
    -8396,-8346,-8296,-8246,-8196,-8147,-8097,-8047,
    -7997,-7947,-7897,-7847,-7797,-7747,-7697,-7648,
    -7598,-7548,-7498,-7448,-7398,-7348,-7298,-7248,
    -7198,-7148,-7098,-7048,-6998,-6948,-6898,-6848,
    -6798,-6748,-6698,-6648,-6598,-6548,-6498,-6448,
    -6398,-6348,-6298,-6248,-6198,-6148,-6098,-6048,
    -5998,-5948,-5898,-5848,-5798,-5747,-5697,-5647,
    -5597,-5547,-5497,-5447,-5397,-5347,-5297,-5247,
    -5197,-5146,-5096,-5046,-4996,-4946,-4896,-4846,
    -4796,-4745,-4695,-4645,-4595,-4545,-4495,-4445,
    -4394,-4344,-4294,-4244,-4194,-4144,-4093,-4043,
    -3993,-3943,-3893,-3843,-3792,-3742,-3692,-3642,
    -3592,-3541,-3491,-3441,-3391,-3341,-3291,-3240,
    -3190,-3140,-3090,-3039,-2989,-2939,-2889,-2839,
    -2788,-2738,-2688,-2638,-2588,-2537,-2487,-2437,
    -2387,-2336,-2286,-2236,-2186,-2135,-2085,-2035,
    -1985,-1934,-1884,-1834,-1784,-1733,-1683,-1633,
    -1583,-1532,-1482,-1432,-1382,-1331,-1281,-1231,
    -1181,-1130,-1080,-1030,-980,-929,-879,-829,
    -779,-728,-678,-628,-578,-527,-477,-427,
    -376,-326,-276,-226,-175,-125,-75,-25,
    25,75,125,175,226,276,326,376,
    427,477,527,578,628,678,728,779,
    829,879,929,980,1030,1080,1130,1181,
    1231,1281,1331,1382,1432,1482,1532,1583,
    1633,1683,1733,1784,1834,1884,1934,1985,
    2035,2085,2135,2186,2236,2286,2336,2387,
    2437,2487,2537,2587,2638,2688,2738,2788,
    2839,2889,2939,2989,3039,3090,3140,3190,
    3240,3291,3341,3391,3441,3491,3542,3592,
    3642,3692,3742,3792,3843,3893,3943,3993,
    4043,4093,4144,4194,4244,4294,4344,4394,
    4445,4495,4545,4595,4645,4695,4745,4796,
    4846,4896,4946,4996,5046,5096,5146,5197,
    5247,5297,5347,5397,5447,5497,5547,5597,
    5647,5697,5747,5798,5848,5898,5948,5998,
    6048,6098,6148,6198,6248,6298,6348,6398,
    6448,6498,6548,6598,6648,6698,6748,6798,
    6848,6898,6948,6998,7048,7098,7148,7198,
    7248,7298,7348,7398,7448,7498,7548,7598,
    7648,7697,7747,7797,7847,7897,7947,7997,
    8047,8097,8147,8196,8246,8296,8346,8396,
    8446,8496,8545,8595,8645,8695,8745,8794,
    8844,8894,8944,8994,9043,9093,9143,9193,
    9243,9292,9342,9392,9442,9491,9541,9591,
    9640,9690,9740,9790,9839,9889,9939,9988,
    10038,10088,10137,10187,10237,10286,10336,10386,
    10435,10485,10534,10584,10634,10683,10733,10782,
    10832,10882,10931,10981,11030,11080,11129,11179,
    11228,11278,11327,11377,11426,11476,11525,11575,
    11624,11674,11723,11773,11822,11872,11921,11970,
    12020,12069,12119,12168,12218,12267,12316,12366,
    12415,12464,12514,12563,12612,12662,12711,12760,
    12810,12859,12908,12957,13007,13056,13105,13154,
    13204,13253,13302,13351,13401,13450,13499,13548,
    13597,13647,13696,13745,13794,13843,13892,13941,
    13990,14040,14089,14138,14187,14236,14285,14334,
    14383,14432,14481,14530,14579,14628,14677,14726,
    14775,14824,14873,14922,14971,15020,15069,15118,
    15167,15215,15264,15313,15362,15411,15460,15509,
    15557,15606,15655,15704,15753,15802,15850,15899,
    15948,15997,16045,16094,16143,16191,16240,16289,
    16338,16386,16435,16484,16532,16581,16629,16678,
    16727,16775,16824,16872,16921,16970,17018,17067,
    17115,17164,17212,17261,17309,17358,17406,17455,
    17503,17551,17600,17648,17697,17745,17793,17842,
    17890,17939,17987,18035,18084,18132,18180,18228,
    18277,18325,18373,18421,18470,18518,18566,18614,
    18663,18711,18759,18807,18855,18903,18951,19000,
    19048,19096,19144,19192,19240,19288,19336,19384,
    19432,19480,19528,19576,19624,19672,19720,19768,
    19816,19864,19912,19959,20007,20055,20103,20151,
    20199,20246,20294,20342,20390,20438,20485,20533,
    20581,20629,20676,20724,20772,20819,20867,20915,
    20962,21010,21057,21105,21153,21200,21248,21295,
    21343,21390,21438,21485,21533,21580,21628,21675,
    21723,21770,21817,21865,21912,21960,22007,22054,
    22102,22149,22196,22243,22291,22338,22385,22432,
    22480,22527,22574,22621,22668,22716,22763,22810,
    22857,22904,22951,22998,23045,23092,23139,23186,
    23233,23280,23327,23374,23421,23468,23515,23562,
    23609,23656,23703,23750,23796,23843,23890,23937,
    23984,24030,24077,24124,24171,24217,24264,24311,
    24357,24404,24451,24497,24544,24591,24637,24684,
    24730,24777,24823,24870,24916,24963,25009,25056,
    25102,25149,25195,25241,25288,25334,25381,25427,
    25473,25520,25566,25612,25658,25705,25751,25797,
    25843,25889,25936,25982,26028,26074,26120,26166,
    26212,26258,26304,26350,26396,26442,26488,26534,
    26580,26626,26672,26718,26764,26810,26856,26902,
    26947,26993,27039,27085,27131,27176,27222,27268,
    27313,27359,27405,27450,27496,27542,27587,27633,
    27678,27724,27770,27815,27861,27906,27952,27997,
    28042,28088,28133,28179,28224,28269,28315,28360,
    28405,28451,28496,28541,28586,28632,28677,28722,
    28767,28812,28858,28903,28948,28993,29038,29083,
    29128,29173,29218,29263,29308,29353,29398,29443,
    29488,29533,29577,29622,29667,29712,29757,29801,
    29846,29891,29936,29980,30025,30070,30114,30159,
    30204,30248,30293,30337,30382,30427,30471,30516,
    30560,30604,30649,30693,30738,30782,30826,30871,
    30915,30959,31004,31048,31092,31136,31181,31225,
    31269,31313,31357,31402,31446,31490,31534,31578,
    31622,31666,31710,31754,31798,31842,31886,31930,
    31974,32017,32061,32105,32149,32193,32236,32280,
    32324,32368,32411,32455,32499,32542,32586,32630,
    32673,32717,32760,32804,32847,32891,32934,32978,
    33021,33065,33108,33151,33195,33238,33281,33325,
    33368,33411,33454,33498,33541,33584,33627,33670,
    33713,33756,33799,33843,33886,33929,33972,34015,
    34057,34100,34143,34186,34229,34272,34315,34358,
    34400,34443,34486,34529,34571,34614,34657,34699,
    34742,34785,34827,34870,34912,34955,34997,35040,
    35082,35125,35167,35210,35252,35294,35337,35379,
    35421,35464,35506,35548,35590,35633,35675,35717,
    35759,35801,35843,35885,35927,35969,36011,36053,
    36095,36137,36179,36221,36263,36305,36347,36388,
    36430,36472,36514,36556,36597,36639,36681,36722,
    36764,36805,36847,36889,36930,36972,37013,37055,
    37096,37137,37179,37220,37262,37303,37344,37386,
    37427,37468,37509,37551,37592,37633,37674,37715,
    37756,37797,37838,37879,37920,37961,38002,38043,
    38084,38125,38166,38207,38248,38288,38329,38370,
    38411,38451,38492,38533,38573,38614,38655,38695,
    38736,38776,38817,38857,38898,38938,38979,39019,
    39059,39100,39140,39180,39221,39261,39301,39341,
    39382,39422,39462,39502,39542,39582,39622,39662,
    39702,39742,39782,39822,39862,39902,39942,39982,
    40021,40061,40101,40141,40180,40220,40260,40299,
    40339,40379,40418,40458,40497,40537,40576,40616,
    40655,40695,40734,40773,40813,40852,40891,40931,
    40970,41009,41048,41087,41127,41166,41205,41244,
    41283,41322,41361,41400,41439,41478,41517,41556,
    41595,41633,41672,41711,41750,41788,41827,41866,
    41904,41943,41982,42020,42059,42097,42136,42174,
    42213,42251,42290,42328,42366,42405,42443,42481,
    42520,42558,42596,42634,42672,42711,42749,42787,
    42825,42863,42901,42939,42977,43015,43053,43091,
    43128,43166,43204,43242,43280,43317,43355,43393,
    43430,43468,43506,43543,43581,43618,43656,43693,
    43731,43768,43806,43843,43880,43918,43955,43992,
    44029,44067,44104,44141,44178,44215,44252,44289,
    44326,44363,44400,44437,44474,44511,44548,44585,
    44622,44659,44695,44732,44769,44806,44842,44879,
    44915,44952,44989,45025,45062,45098,45135,45171,
    45207,45244,45280,45316,45353,45389,45425,45462,
    45498,45534,45570,45606,45642,45678,45714,45750,
    45786,45822,45858,45894,45930,45966,46002,46037,
    46073,46109,46145,46180,46216,46252,46287,46323,
    46358,46394,46429,46465,46500,46536,46571,46606,
    46642,46677,46712,46747,46783,46818,46853,46888,
    46923,46958,46993,47028,47063,47098,47133,47168,
    47203,47238,47273,47308,47342,47377,47412,47446,
    47481,47516,47550,47585,47619,47654,47688,47723,
    47757,47792,47826,47861,47895,47929,47963,47998,
    48032,48066,48100,48134,48168,48202,48237,48271,
    48305,48338,48372,48406,48440,48474,48508,48542,
    48575,48609,48643,48676,48710,48744,48777,48811,
    48844,48878,48911,48945,48978,49012,49045,49078,
    49112,49145,49178,49211,49244,49278,49311,49344,
    49377,49410,49443,49476,49509,49542,49575,49608,
    49640,49673,49706,49739,49771,49804,49837,49869,
    49902,49935,49967,50000,50032,50064,50097,50129,
    50162,50194,50226,50259,50291,50323,50355,50387,
    50420,50452,50484,50516,50548,50580,50612,50644,
    50675,50707,50739,50771,50803,50834,50866,50898,
    50929,50961,50993,51024,51056,51087,51119,51150,
    51182,51213,51244,51276,51307,51338,51369,51401,
    51432,51463,51494,51525,51556,51587,51618,51649,
    51680,51711,51742,51773,51803,51834,51865,51896,
    51926,51957,51988,52018,52049,52079,52110,52140,
    52171,52201,52231,52262,52292,52322,52353,52383,
    52413,52443,52473,52503,52534,52564,52594,52624,
    52653,52683,52713,52743,52773,52803,52832,52862,
    52892,52922,52951,52981,53010,53040,53069,53099,
    53128,53158,53187,53216,53246,53275,53304,53334,
    53363,53392,53421,53450,53479,53508,53537,53566,
    53595,53624,53653,53682,53711,53739,53768,53797,
    53826,53854,53883,53912,53940,53969,53997,54026,
    54054,54082,54111,54139,54167,54196,54224,54252,
    54280,54309,54337,54365,54393,54421,54449,54477,
    54505,54533,54560,54588,54616,54644,54672,54699,
    54727,54755,54782,54810,54837,54865,54892,54920,
    54947,54974,55002,55029,55056,55084,55111,55138,
    55165,55192,55219,55246,55274,55300,55327,55354,
    55381,55408,55435,55462,55489,55515,55542,55569,
    55595,55622,55648,55675,55701,55728,55754,55781,
    55807,55833,55860,55886,55912,55938,55965,55991,
    56017,56043,56069,56095,56121,56147,56173,56199,
    56225,56250,56276,56302,56328,56353,56379,56404,
    56430,56456,56481,56507,56532,56557,56583,56608,
    56633,56659,56684,56709,56734,56760,56785,56810,
    56835,56860,56885,56910,56935,56959,56984,57009,
    57034,57059,57083,57108,57133,57157,57182,57206,
    57231,57255,57280,57304,57329,57353,57377,57402,
    57426,57450,57474,57498,57522,57546,57570,57594,
    57618,57642,57666,57690,57714,57738,57762,57785,
    57809,57833,57856,57880,57903,57927,57950,57974,
    57997,58021,58044,58067,58091,58114,58137,58160,
    58183,58207,58230,58253,58276,58299,58322,58345,
    58367,58390,58413,58436,58459,58481,58504,58527,
    58549,58572,58594,58617,58639,58662,58684,58706,
    58729,58751,58773,58795,58818,58840,58862,58884,
    58906,58928,58950,58972,58994,59016,59038,59059,
    59081,59103,59125,59146,59168,59190,59211,59233,
    59254,59276,59297,59318,59340,59361,59382,59404,
    59425,59446,59467,59488,59509,59530,59551,59572,
    59593,59614,59635,59656,59677,59697,59718,59739,
    59759,59780,59801,59821,59842,59862,59883,59903,
    59923,59944,59964,59984,60004,60025,60045,60065,
    60085,60105,60125,60145,60165,60185,60205,60225,
    60244,60264,60284,60304,60323,60343,60363,60382,
    60402,60421,60441,60460,60479,60499,60518,60537,
    60556,60576,60595,60614,60633,60652,60671,60690,
    60709,60728,60747,60766,60785,60803,60822,60841,
    60859,60878,60897,60915,60934,60952,60971,60989,
    61007,61026,61044,61062,61081,61099,61117,61135,
    61153,61171,61189,61207,61225,61243,61261,61279,
    61297,61314,61332,61350,61367,61385,61403,61420,
    61438,61455,61473,61490,61507,61525,61542,61559,
    61577,61594,61611,61628,61645,61662,61679,61696,
    61713,61730,61747,61764,61780,61797,61814,61831,
    61847,61864,61880,61897,61913,61930,61946,61963,
    61979,61995,62012,62028,62044,62060,62076,62092,
    62108,62125,62141,62156,62172,62188,62204,62220,
    62236,62251,62267,62283,62298,62314,62329,62345,
    62360,62376,62391,62407,62422,62437,62453,62468,
    62483,62498,62513,62528,62543,62558,62573,62588,
    62603,62618,62633,62648,62662,62677,62692,62706,
    62721,62735,62750,62764,62779,62793,62808,62822,
    62836,62850,62865,62879,62893,62907,62921,62935,
    62949,62963,62977,62991,63005,63019,63032,63046,
    63060,63074,63087,63101,63114,63128,63141,63155,
    63168,63182,63195,63208,63221,63235,63248,63261,
    63274,63287,63300,63313,63326,63339,63352,63365,
    63378,63390,63403,63416,63429,63441,63454,63466,
    63479,63491,63504,63516,63528,63541,63553,63565,
    63578,63590,63602,63614,63626,63638,63650,63662,
    63674,63686,63698,63709,63721,63733,63745,63756,
    63768,63779,63791,63803,63814,63825,63837,63848,
    63859,63871,63882,63893,63904,63915,63927,63938,
    63949,63960,63971,63981,63992,64003,64014,64025,
    64035,64046,64057,64067,64078,64088,64099,64109,
    64120,64130,64140,64151,64161,64171,64181,64192,
    64202,64212,64222,64232,64242,64252,64261,64271,
    64281,64291,64301,64310,64320,64330,64339,64349,
    64358,64368,64377,64387,64396,64405,64414,64424,
    64433,64442,64451,64460,64469,64478,64487,64496,
    64505,64514,64523,64532,64540,64549,64558,64566,
    64575,64584,64592,64600,64609,64617,64626,64634,
    64642,64651,64659,64667,64675,64683,64691,64699,
    64707,64715,64723,64731,64739,64747,64754,64762,
    64770,64777,64785,64793,64800,64808,64815,64822,
    64830,64837,64844,64852,64859,64866,64873,64880,
    64887,64895,64902,64908,64915,64922,64929,64936,
    64943,64949,64956,64963,64969,64976,64982,64989,
    64995,65002,65008,65015,65021,65027,65033,65040,
    65046,65052,65058,65064,65070,65076,65082,65088,
    65094,65099,65105,65111,65117,65122,65128,65133,
    65139,65144,65150,65155,65161,65166,65171,65177,
    65182,65187,65192,65197,65202,65207,65212,65217,
    65222,65227,65232,65237,65242,65246,65251,65256,
    65260,65265,65270,65274,65279,65283,65287,65292,
    65296,65300,65305,65309,65313,65317,65321,65325,
    65329,65333,65337,65341,65345,65349,65352,65356,
    65360,65363,65367,65371,65374,65378,65381,65385,
    65388,65391,65395,65398,65401,65404,65408,65411,
    65414,65417,65420,65423,65426,65429,65431,65434,
    65437,65440,65442,65445,65448,65450,65453,65455,
    65458,65460,65463,65465,65467,65470,65472,65474,
    65476,65478,65480,65482,65484,65486,65488,65490,
    65492,65494,65496,65497,65499,65501,65502,65504,
    65505,65507,65508,65510,65511,65513,65514,65515,
    65516,65518,65519,65520,65521,65522,65523,65524,
    65525,65526,65527,65527,65528,65529,65530,65530,
    65531,65531,65532,65532,65533,65533,65534,65534,
    65534,65535,65535,65535,65535,65535,65535,65535
};
