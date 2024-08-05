#include "demo.h"

struct Screen* createScreen(struct BitMap* b, BOOL hidden, 
        WORD x, WORD y, UWORD width, UWORD height, UWORD depth, 
        struct Rectangle* clip){
    struct TagItem screentags[11] = {
        {SA_BitMap, NULL},
        {SA_Left, 0}, 
        {SA_Top, 0}, 
        {SA_Width, 0}, 
        {SA_Height, 0}, 
        {SA_Depth, 0}, 
        {SA_Type, CUSTOMSCREEN|CUSTOMBITMAP}, 
        {SA_Quiet, TRUE},
        {TAG_DONE, NULL},
        {NULL, NULL},
        {NULL, NULL}
    };

    screentags[0].ti_Data = (ULONG) b;
    screentags[1].ti_Data = x;
    screentags[2].ti_Data = y;
    screentags[3].ti_Data = width;
    screentags[4].ti_Data = height;
    screentags[5].ti_Data = depth;

    return OpenScreenTagList(NULL, screentags);
}
