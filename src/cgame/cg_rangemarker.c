#include "cg_local.h"

const vec3_t cg_shaderColors[ SHC_NUM_SHADER_COLORS ] =
{
  { 0.0f,   0.0f,   0.75f  }, // dark blue
  { 0.3f,   0.35f,  0.625f }, // light blue
  { 0.0f,   0.625f, 0.563f }, // green-cyan
  { 0.313f, 0.0f,   0.625f }, // violet
  { 0.625f, 0.625f, 0.0f   }, // yellow
  { 0.875f, 0.313f, 0.0f   }, // orange
  { 0.375f, 0.625f, 0.375f }, // light green
  { 0.0f,   0.438f, 0.0f   }, // dark green
  { 1.0f,   0.0f,   0.0f   }, // red
  { 0.625f, 0.375f, 0.4f   }, // pink
  { 0.313f, 0.313f, 0.313f }  // grey
};

/*
================
CG_RangeMarkerPreferences
================
*/
qboolean CG_GetRangeMarkerPreferences( qboolean *drawSurface, qboolean *drawIntersection,
                                       qboolean *drawFrontline, float *surfaceOpacity,
                                       float *lineOpacity, float *lineThickness )
{
  *drawSurface = !!cg_rangeMarkerDrawSurface.integer;
  *drawIntersection = !!cg_rangeMarkerDrawIntersection.integer;
  *drawFrontline = !!cg_rangeMarkerDrawFrontline.integer;
  *surfaceOpacity = cg_rangeMarkerSurfaceOpacity.value;
  *lineOpacity = cg_rangeMarkerLineOpacity.value;
  *lineThickness = cg_rangeMarkerLineThickness.value;

  if( ( *drawSurface && *surfaceOpacity > 0.0f ) ||
      ( ( *drawIntersection || *drawFrontline ) && *lineOpacity > 0.0f &&
        *lineThickness > 0.0f && cg_binaryShaderScreenScale.value > 0.0f ) )
  {
    if( *surfaceOpacity > 1.0f )
      *surfaceOpacity = 1.0f;
    if( *lineOpacity > 1.0f )
      *lineOpacity = 1.0f;
    return qtrue;
  }

  return qfalse;
}

/*
================
CG_UpdateBuildableRangeMarkerMask
================
*/
void CG_UpdateBuildableRangeMarkerMask( void )
{
  static int mc = 0;

  if( cg_rangeMarkerBuildableTypes.modificationCount != mc )
  {
    int brmMask;
    char buffer[ MAX_CVAR_VALUE_STRING ];
    char *p, *q;
    buildable_t buildable;

    brmMask = 0;

    if( !cg_rangeMarkerBuildableTypes.string[ 0 ] )
      goto empty;

    Q_strncpyz( buffer, cg_rangeMarkerBuildableTypes.string, sizeof( buffer ) );
    p = &buffer[ 0 ];

    for(;;)
    {
      q = strchr( p, ',' );
      if( q )
        *q = '\0';

      while( *p == ' ' )
        ++p;

      buildable = BG_BuildableByName( p )->number;

      if( buildable != BA_NONE )
      {
        brmMask |= 1 << buildable;
      }
      else if( !Q_stricmp( p, "all" ) )
      {
        brmMask |= ( 1 << BA_A_OVERMIND ) | ( 1 << BA_A_SPAWN ) |
                   ( 1 << BA_A_ACIDTUBE ) | ( 1 << BA_A_TRAPPER ) | ( 1 << BA_A_HIVE ) |
                   ( 1 << BA_H_REACTOR ) | ( 1 << BA_H_REPEATER ) | ( 1 << BA_H_DCC ) |
                   ( 1 << BA_H_MGTURRET ) | ( 1 << BA_H_TESLAGEN );
      }
      else
      {
        char *pp;
        int only;

        if( !Q_stricmpn( p, "alien", 5 ) )
        {
          pp = p + 5;
          only = ( 1 << BA_A_OVERMIND ) | ( 1 << BA_A_SPAWN ) |
                 ( 1 << BA_A_ACIDTUBE ) | ( 1 << BA_A_TRAPPER ) | ( 1 << BA_A_HIVE );
        }
        else if( !Q_stricmpn( p, "human", 5 ) )
        {
          pp = p + 5;
          only = ( 1 << BA_H_REACTOR ) | ( 1 << BA_H_REPEATER ) | ( 1 << BA_H_DCC ) |
                 ( 1 << BA_H_MGTURRET ) | ( 1 << BA_H_TESLAGEN );
        }
        else
        {
          pp = p;
          only = ~0;
        }

        if( pp != p && !*pp )
        {
          brmMask |= only;
        }
        else if( !Q_stricmp( pp, "support" ) )
        {
          brmMask |= only & ( ( 1 << BA_A_OVERMIND ) | ( 1 << BA_A_SPAWN ) |
                              ( 1 << BA_H_REACTOR ) | ( 1 << BA_H_REPEATER ) | ( 1 << BA_H_DCC ) );
        }
        else if( !Q_stricmp( pp, "offensive" ) )
        {
          brmMask |= only & ( ( 1 << BA_A_ACIDTUBE ) | ( 1 << BA_A_TRAPPER ) | ( 1 << BA_A_HIVE ) |
                              ( 1 << BA_H_MGTURRET ) | ( 1 << BA_H_TESLAGEN ) );
        }
        else
          Com_Printf( S_COLOR_YELLOW "WARNING: unknown buildable or group: %s\n", p );
      }

      if( q )
        p = q + 1;
      else
        break;
    }

    empty:
    trap_Cvar_Set( "cg_buildableRangeMarkerMask", va( "%i", brmMask ) );

    mc = cg_rangeMarkerBuildableTypes.modificationCount;
  }
}

// cg_drawtools.c 
//
/*
================
CG_DrawSphere
================
*/
void CG_DrawSphere( const vec3_t center, float radius, int customShader, const float *shaderRGBA )
{
  refEntity_t re;
  memset( &re, 0, sizeof( re ) );

  re.reType = RT_MODEL;
  re.hModel = cgs.media.sphereModel;
  re.customShader = customShader;
  re.renderfx = RF_NOSHADOW;
  if( shaderRGBA != NULL )
  {
    int i;
    for( i = 0; i < 4; ++i )
      re.shaderRGBA[ i ] = 255 * shaderRGBA[ i ];
  }

  VectorCopy( center, re.origin );

  radius *= 0.01f;
  VectorSet( re.axis[ 0 ], radius, 0, 0 );
  VectorSet( re.axis[ 1 ], 0, radius, 0 );
  VectorSet( re.axis[ 2 ], 0, 0, radius );
  re.nonNormalizedAxes = qtrue;

  trap_R_AddRefEntityToScene( &re );
}

/*
================
CG_DrawSphericalCone
================
*/
void CG_DrawSphericalCone( const vec3_t tip, const vec3_t rotation, float radius,
                           qboolean a240, int customShader, const float *shaderRGBA )
{
  refEntity_t re;
  memset( &re, 0, sizeof( re ) );

  re.reType = RT_MODEL;
  re.hModel = a240 ? cgs.media.sphericalCone240Model : cgs.media.sphericalCone64Model;
  re.customShader = customShader;
  re.renderfx = RF_NOSHADOW;
  if( shaderRGBA != NULL )
  {
    int i;
    for( i = 0; i < 4; ++i )
      re.shaderRGBA[ i ] = 255 * shaderRGBA[ i ];
  }

  VectorCopy( tip, re.origin );

  radius *= 0.01f;
  AnglesToAxis( rotation, re.axis );
  VectorScale( re.axis[ 0 ], radius, re.axis[ 0 ] );
  VectorScale( re.axis[ 1 ], radius, re.axis[ 1 ] );
  VectorScale( re.axis[ 2 ], radius, re.axis[ 2 ] );
  re.nonNormalizedAxes = qtrue;

  trap_R_AddRefEntityToScene( &re );
}


/*
================
CG_DrawRangeMarker
================
*/
void CG_DrawRangeMarker( rangeMarkerType_t rmType, const vec3_t origin, const float *angles, float range,
                         qboolean drawSurface, qboolean drawIntersection, qboolean drawFrontline,
                         const vec3_t rgb, float surfaceOpacity, float lineOpacity, float lineThickness )
{
  if( drawSurface )
  {
    qhandle_t pcsh;
    vec4_t rgba;

    pcsh = cgs.media.plainColorShader;
    VectorCopy( rgb, rgba );
    rgba[ 3 ] = surfaceOpacity;

    switch( rmType )
    {
      case RMT_SPHERE:
        CG_DrawSphere( origin, range, pcsh, rgba );
        break;
      case RMT_SPHERICAL_CONE_64:
        CG_DrawSphericalCone( origin, angles, range, qfalse, pcsh, rgba );
        break;
      case RMT_SPHERICAL_CONE_240:
        CG_DrawSphericalCone( origin, angles, range, qtrue, pcsh, rgba );
        break;
    }
  }

  if( drawIntersection || drawFrontline )
  {
    const cgMediaBinaryShader_t *mbsh;
    cgBinaryShaderSetting_t *bshs;
    int i;

    if( cg.numBinaryShadersUsed >= NUM_BINARY_SHADERS )
      return;
    mbsh = &cgs.media.binaryShaders[ cg.numBinaryShadersUsed ];

    if( rmType == RMT_SPHERE )
    {
      if( range > lineThickness / 2 )
      {
        if( drawIntersection )
          CG_DrawSphere( origin, range - lineThickness / 2, mbsh->b1, NULL );
        CG_DrawSphere( origin, range - lineThickness / 2, mbsh->f2, NULL );
      }

      if( drawIntersection )
        CG_DrawSphere( origin, range + lineThickness / 2, mbsh->b2, NULL );
      CG_DrawSphere( origin, range + lineThickness / 2, mbsh->f1, NULL );
    }
    else if( rmType == RMT_SPHERICAL_CONE_64 || rmType == RMT_SPHERICAL_CONE_240 )
    {
      qboolean a240;
      float f, r;
      vec3_t forward, tip;

      a240 = ( rmType == RMT_SPHERICAL_CONE_240 );
      f = lineThickness * ( a240 ? 0.26f : 0.8f );
      r = f + lineThickness * ( a240 ? 0.23f : 0.43f );
      AngleVectors( angles, forward, NULL, NULL );

      if( range > r )
      {
        VectorMA( origin, f, forward, tip );
        if( drawIntersection )
          CG_DrawSphericalCone( tip, angles, range - r, a240, mbsh->b1, NULL );
        CG_DrawSphericalCone( tip, angles, range - r, a240, mbsh->f2, NULL );
      }

      VectorMA( origin, -f, forward, tip );
      if( drawIntersection )
        CG_DrawSphericalCone( tip, angles, range + r, a240, mbsh->b2, NULL );
      CG_DrawSphericalCone( tip, angles, range + r, a240, mbsh->f1, NULL );
    }

    bshs = &cg.binaryShaderSettings[ cg.numBinaryShadersUsed ];

    for( i = 0; i < 3; ++i )
      bshs->color[ i ] = 255 * lineOpacity * rgb[ i ];
    bshs->drawIntersection = drawIntersection;
    bshs->drawFrontline = drawFrontline;

    ++cg.numBinaryShadersUsed;
  }
}

// cg_buildable.c

/*
================
CG_BuildableRangeMarkerProperties
================
*/
qboolean CG_GetBuildableRangeMarkerProperties( buildable_t bType, rangeMarkerType_t *rmType, float *range, vec3_t rgb )
{
  shaderColorEnum_t shc;

  switch( bType )
  {
    case BA_A_SPAWN:    *range = CREEP_BASESIZE;    shc = SHC_LIGHT_GREEN; break;
    case BA_A_OVERMIND: *range = CREEP_BASESIZE;    shc = SHC_DARK_GREEN;  break;
    case BA_A_ACIDTUBE: *range = ACIDTUBE_RANGE;    shc = SHC_RED;         break;
    case BA_A_TRAPPER:  *range = TRAPPER_RANGE;     shc = SHC_PINK;        break;
    case BA_A_HIVE:     *range = HIVE_SENSE_RANGE;  shc = SHC_YELLOW;      break;
    case BA_H_MGTURRET: *range = MGTURRET_RANGE;    shc = SHC_ORANGE;      break;
    case BA_H_TESLAGEN: *range = TESLAGEN_RANGE;    shc = SHC_VIOLET;      break;
    case BA_H_DCC:      *range = DC_RANGE;          shc = SHC_GREEN_CYAN;  break;
    case BA_H_REACTOR:  *range = REACTOR_BASESIZE;  shc = SHC_DARK_BLUE;   break;
    case BA_H_REPEATER: *range = REPEATER_BASESIZE; shc = SHC_LIGHT_BLUE;  break;
    default: return qfalse;
  }

  if( bType == BA_A_TRAPPER )
    *rmType = RMT_SPHERICAL_CONE_64;
  else if( bType == BA_H_MGTURRET )
    *rmType = RMT_SPHERICAL_CONE_240;
  else
    *rmType = RMT_SPHERE;

  VectorCopy( cg_shaderColors[ shc ], rgb );

  return qtrue;
}

/*
================
CG_GhostBuildableRangeMarker
================
*/
void CG_GhostBuildableRangeMarker( buildable_t buildable, const vec3_t origin, const vec3_t normal )
{
  qboolean drawS, drawI, drawF;
  float so, lo, th;
  rangeMarkerType_t rmType;
  float range;
  vec3_t rgb;

  if( CG_GetRangeMarkerPreferences( &drawS, &drawI, &drawF, &so, &lo, &th ) &&
      CG_GetBuildableRangeMarkerProperties( buildable, &rmType, &range, rgb ) )
  {
    vec3_t localOrigin, angles;

    if( buildable == BA_A_HIVE || buildable == BA_H_TESLAGEN )
      VectorMA( origin, BG_BuildableConfig( buildable )->maxs[ 2 ], normal, localOrigin );
    else
      VectorCopy( origin, localOrigin );

    if( rmType != RMT_SPHERE )
      vectoangles( normal, angles );

    CG_DrawRangeMarker( rmType, localOrigin, ( rmType != RMT_SPHERE ? angles : NULL ),
                        range, drawS, drawI, drawF, rgb, so, lo, th );
  }
}

// cg_ents.c

/*
================
CG_RangeMarker
================
*/
void CG_RangeMarker( centity_t *cent )
{
  qboolean drawS, drawI, drawF;
  float so, lo, th;
  rangeMarkerType_t rmType;
  float range;
  vec3_t rgb;

  if( CG_GetRangeMarkerPreferences( &drawS, &drawI, &drawF, &so, &lo, &th ) &&
      CG_GetBuildableRangeMarkerProperties( cent->currentState.modelindex, &rmType, &range, rgb ) )
  {
    CG_DrawRangeMarker( rmType, cent->lerpOrigin, ( rmType > 0 ? cent->lerpAngles : NULL ),
                        range, drawS, drawI, drawF, rgb, so, lo, th );
  }
}

