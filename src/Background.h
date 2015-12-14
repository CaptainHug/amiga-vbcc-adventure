#include <intuition/intuition.h>

extern UWORD Background_palRGB4[32];
extern UWORD Background_img[20000];

struct Image Background_image = {
	0, 0, 320, 200, 5, Background_img,
	31, 0, NULL
};
