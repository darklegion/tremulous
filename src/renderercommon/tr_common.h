#ifndef TR_COMMON_H
#define TR_COMMON_H

#include "../qcommon/q_shared.h"
/*#include "../qcommon/qfiles.h"*/
/*#include "../qcommon/qcommon.h"*/
#include "../renderercommon/tr_public.h"
#include "qgl.h"

extern	refimport_t		ri;
extern glconfig_t	glConfig;		// outside of TR since it shouldn't be cleared during ref re-init

//
// cvars
//
extern cvar_t *r_stencilbits;			// number of desired stencil bits
extern cvar_t *r_depthbits;			// number of desired depth bits
extern cvar_t *r_colorbits;			// number of desired color bits, only relevant for fullscreen
extern cvar_t *r_texturebits;			// number of desired texture bits
extern cvar_t *r_ext_multisample;
										// 0 = use framebuffer depth
										// 16 = use 16-bit textures
										// 32 = use 32-bit textures
										// all else = error

extern cvar_t	*r_width;
extern cvar_t	*r_height;
extern cvar_t	*r_pixelAspect;
extern cvar_t *r_noborder;
extern cvar_t *r_fullscreen;
extern cvar_t *r_ignorehwgamma;		// overrides hardware gamma capabilities
extern cvar_t *r_drawBuffer;
extern cvar_t *r_swapInterval;

extern cvar_t *r_allowExtensions;				// global enable/disable of OpenGL extensions
extern cvar_t *r_ext_compressed_textures;		// these control use of specific extensions
extern cvar_t *r_ext_multitexture;
extern cvar_t *r_ext_compiled_vertex_array;
extern cvar_t *r_ext_texture_env_add;

extern cvar_t *r_ext_texture_filter_anisotropic;
extern cvar_t *r_ext_max_anisotropy;

extern cvar_t *r_stereoEnabled;

qboolean	R_GetModeInfo( int *width, int *height, float *windowAspect, int mode );


/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void		GLimp_Init( void );
void		GLimp_Shutdown( void );
void		GLimp_EndFrame( void );

void		GLimp_LogComment( char *comment );
void		GLimp_Minimize(void);

void		GLimp_SetGamma( unsigned char red[256],
		unsigned char green[256],
		unsigned char blue[256] );


#endif
