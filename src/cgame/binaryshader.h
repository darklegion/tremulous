#ifndef BINARY_SHADER_H
#define BINARY_SHADER_H 1

typedef struct
{
    byte color[ 3 ];
    qboolean drawIntersection;
    qboolean drawFrontline;
} cgBinaryShaderSetting_t;

#define NUM_BINARY_SHADERS 256

typedef struct
{
  qhandle_t f1;
  qhandle_t f2;
  qhandle_t f3;
  qhandle_t b1;
  qhandle_t b2;
  qhandle_t b3;
} cgMediaBinaryShader_t;

typedef enum
{
  SHC_DARK_BLUE,
  SHC_LIGHT_BLUE,
  SHC_GREEN_CYAN,
  SHC_VIOLET,
  SHC_YELLOW,
  SHC_ORANGE,
  SHC_LIGHT_GREEN,
  SHC_DARK_GREEN,
  SHC_RED,
  SHC_PINK,
  SHC_GREY,
  SHC_NUM_SHADER_COLORS
} shaderColorEnum_t;

extern  const vec3_t    cg_shaderColors[ SHC_NUM_SHADER_COLORS ];


#endif
