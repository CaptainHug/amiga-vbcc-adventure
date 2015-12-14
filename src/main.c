#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <graphics/gfxbase.h>
#include <intuition/intuitionbase.h>
#include <hardware/custom.h>
#include <hardware/cia.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/diskfont_pragmas.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>


#include "board.h"
#include "screen_size.h"
#include "ptreplay.h"
#include "ptreplay_protos.h"

// assets
#include "Background.h"


// system libs
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;
extern struct Custom custom;

// display
struct View view;
struct View *oldView;
struct ViewPort viewPort1;
struct RasInfo rasInfo1;
struct BitMap bitMap1;
struct RastPort rastPort1;

struct Task *myTask = NULL;
BYTE oldPri;

// music
struct Library *PTReplayBase;
struct Module *theMod;
UBYTE *mod = NULL;


PLANEPTR load_getchipmem(UBYTE *name, ULONG size)
{
  BPTR fileHandle;
  PLANEPTR mem;

  if (!(fileHandle = Open(name, MODE_OLDFILE)))
    return (NULL);

  if (!(mem = AllocMem(size, MEMF_CHIP)))
    return (NULL);

  Read(fileHandle, mem, size);
  Close(fileHandle);

  return (mem);
}


void initMusic(void)
{
	if (!(PTReplayBase = OpenLibrary((UBYTE *)"libs/ptreplay.library", 0)))
	{
		exit(0); //FIXME
	}

	mod = NULL;
	mod = load_getchipmem((UBYTE *)"assets/intro129.mod", 46868);
}

void playMusic(void)
{
	if (mod != NULL)
	{
		theMod = PTSetupMod((APTR)mod);
		PTPlay(theMod);
	}
}

void stopMusic(void)
{
	/*	Stop music */
	if (mod != NULL)
	{
		PTStop(theMod);
		PTFreeMod(theMod);
		FreeMem(mod, 221128);
	}

	if (PTReplayBase) CloseLibrary(PTReplayBase);
}


int main(int argc, char** argv)
{
	/* Open the Intuition library: */
	IntuitionBase = (struct IntuitionBase *)
	OpenLibrary( "intuition.library", 0 );
	if( !IntuitionBase ) {
		printf( "Could NOT open the Intuition library!" );
		return 0;
	}

	/* Open the Graphics library: */
	GfxBase = (struct GfxBase *)
	OpenLibrary( "graphics.library", 0 );
	if( !GfxBase ) {
		printf( "Could NOT open the Graphics library!" );
		return 0;
	}

	printf("RESiSTANCE - LOADiNG & DECRUNCHiNG...\n");
	
	// Save the current View, so we can restore it later
	oldView = GfxBase->ActiView;

	// Prepare the View structure, and give it a pointer to the first ViewPort
	InitView( &view );
	view.ViewPort = &viewPort1;
	
	InitVPort( &viewPort1 );
	viewPort1.DWidth = DISPL_WIDTH1;		// width
	viewPort1.DHeight = DISPL_HEIGHT1;		// height
	viewPort1.DxOffset = 0;				// xpos
	viewPort1.DyOffset = 0;				// ypos
	viewPort1.RasInfo = &rasInfo1;		// ptr to raster info
	viewPort1.Modes = PFBA;			// low res mode
	viewPort1.Next = NULL;					// ptr to next viewport
	
	viewPort1.ColorMap = (struct ColorMap *) GetColorMap(COLOURS1 * 4);
	if( viewPort1.ColorMap == NULL ) {
		//close_demo( "Could NOT get a ColorMap!" );
		printf("no colormap");
		exit(0);
	}
	
	// setup view
	// ViewPort 1, Bitmap 1
	InitBitMap( &bitMap1, DEPTH1, WIDTH1, HEIGHT1 + 8);
	
	int i=0;
	
	/* Allocate memory for the Raster: */ 
	for(i=0; i<DEPTH1; i++)
	{
		bitMap1.Planes[i] = (PLANEPTR) AllocRaster( WIDTH1, HEIGHT1 + 8);
		if( bitMap1.Planes[i] == NULL ) {
			printf( "Could NOT allocate enough memory for the raster!" );
			exit(0);
		}
	/* Clear the display memory with help of the Blitter: */
		BltClear( bitMap1.Planes[i], RASSIZE( WIDTH1, HEIGHT1 + 8), 0 );
	}
	
	// raster
	/* ViewPort 1, Raster 1 */
	rasInfo1.BitMap = &bitMap1; /* Pointer to the BitMap structure.  */
	rasInfo1.RxOffset = 0;       /* The top left corner of the Raster */
	rasInfo1.RyOffset = 0;       /* should be at the top left corner  */
	
	/* ViewPort 1 */
	InitRastPort( &rastPort1 );
	rastPort1.BitMap = &bitMap1;
	
	// create display
	MakeVPort(&view, &viewPort1);
	
	
	UWORD* Background_img_chip = AllocMem(sizeof(UWORD) * sizeof(Background_img), MEMF_CHIP);
	CopyMem(Background_img, Background_img_chip, sizeof(UWORD) * sizeof(Background_img));
	Background_image.ImageData = Background_img_chip;
	
	LoadRGB4(&viewPort1, Background_palRGB4, 32);
	
	initMusic();
	
	WaitTOF();
	MrgCop(&view);
	LoadView( &view );
	WaitTOF();
	WaitTOF();
	
	myTask = FindTask(NULL);
	oldPri = SetTaskPri(myTask, 127);
	
	//Forbid();
	//Disable();
	WaitBlit();
	
	OFF_SPRITE;
	
	
	DrawImage(&rastPort1, &Background_image, 0, 0);
	
	
	playMusic();
	
	while(1) {};
	
	return 0;
}
