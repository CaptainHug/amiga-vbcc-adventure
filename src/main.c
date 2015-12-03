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


struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;

// view
struct View my_view;
struct View *my_old_view;

// viewport
struct ViewPort view_port1;
struct RasInfo ras_info1;


// Music
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
	mod = load_getchipmem((UBYTE *)"assets/miami_vice.mod", 7394);
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

	printf("DEMO IS LOADING!\n");
	
	// Save the current View, so we can restore it later
	my_old_view = GfxBase->ActiView;

	// Prepare the View structure, and give it a pointer to the first ViewPort
	InitView( &my_view );
	my_view.ViewPort = &view_port1;
	
	InitVPort( &view_port1 );
	view_port1.DWidth = DISPL_WIDTH1;		// width
	view_port1.DHeight = DISPL_HEIGHT1;		// height
	view_port1.DxOffset = 0;				// xpos
	view_port1.DyOffset = 0;				// ypos
	view_port1.RasInfo = &ras_info1;		// ptr to raster info
	view_port1.Modes = DUALPF|PFBA;			// low res mode
	view_port1.Next = NULL;					// ptr to next viewport
	
	view_port1.ColorMap = (struct ColorMap *) GetColorMap(COLOURS1 * 4);
	if( view_port1.ColorMap == NULL ) {
		//close_demo( "Could NOT get a ColorMap!" );
		printf("no colormap");
		exit(0);
	}
	
	initMusic();
	
	playMusic();
	
	while(1) {};
	
	return 0;
}
