#include <exec/types.h>

#define kBitsPerByte 0x8
#define kBitsPerWord 0x10
#define kBytesPerWord 0x2
#define kUWordMax 0xFFFF

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define ABS(X) ((X) < 0 ? (-(X)) : (X))
#define ARRAY_NELEMS(X) (sizeof(X) / sizeof((X)[0]))
#define WORD_LO(ADDR) (UWORD)(ULONG)(ADDR)
#define WORD_HI(ADDR) (UWORD)((ULONG)(ADDR) >> kBitsPerWord)


typedef enum {
  StatusError       = 0, // Must be zero, unifies !(status) handling with FALSE, NULL
  StatusOK          = (1 << 0),
  StatusOutOfMemory = (1 << 1),
  StatusInvalidPath = (1 << 2),
  StatusInvalidMod  = (1 << 3),
  StatusQuit        = (1 << 4),
  StatusPlay        = (1 << 5),
} Status;

