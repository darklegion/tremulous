#ifndef CL_SCREEN_H
#define CL_SCREEN_H

#include "qcommon/q_shared.h"

void SCR_Init(void);
void SCR_DebugGraph(float value);
int SCR_GetBigStringWidth(const char *str);  // returns in virtual 640x480 coordinates

void SCR_AdjustFrom640(float *x, float *y, float *w, float *h);
void SCR_FillRect(float x, float y, float width, float height, const float *color);
void SCR_DrawPic(float x, float y, float width, float height, qhandle_t hShader);
void SCR_DrawNamedPic(float x, float y, float width, float height, const char *picname);

void SCR_DrawBigString(int x, int y, const char *s, float alpha, bool noColorEscape);  // draws a string with embedded color control characters with fade
void SCR_DrawBigStringColor(int x, int y, const char *s, vec4_t color, bool noColorEscape);  // ignores embedded color control characters
void SCR_DrawSmallStringExt(int x, int y, const char *string, float *setColor, bool forceColor, bool noColorEscape);
void SCR_DrawSmallChar(int x, int y, int ch);

SO_PUBLIC void SCR_UpdateScreen(void);

#endif
