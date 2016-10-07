//
// Author: blowFish <blowfish@badsec.org>
//

#include "g_local.h"

//-------------------------------------------------------------------------
// Player models
//-------------------------------------------------------------------------
//
static qboolean
_is_playermodel_uniq(const char *model)
{
    int i;
    for ( i = 0; i < level.playerModelCount; i++ )
    {
        if ( !strcmp( model, level.playerModel[i] ) )
            return qfalse;
    }

    return qtrue;
}

static void
G_AddPlayerModel(const char *model)
{
    if (!_is_playermodel_uniq(model))
        return;

    // HACK!
    if (!strcmp(model, "human_bsuit"))
        return;

    level.playerModel[ level.playerModelCount ] = G_CopyString(model);
    level.playerModelCount++;
}

void G_InitPlayerModel(void)
{
    char fileList[ 16*1024 ] = {""};
    char *filePtr;
    int  numFiles;
    int  fileLen = 0;
    int  i;

    // TODO: Add an FS trap which is does correct file globbing
    numFiles = trap_FS_GetFilteredFiles( "/models/players", "",
            "models*players*head_*.skin",
            fileList, sizeof(fileList) );
    filePtr = fileList;

    for( i = 0; i < numFiles && level.playerModelCount < MAX_PLAYER_MODEL;
            i++, filePtr += fileLen + 1 )
    {
        char *start, *c;

        fileLen = strlen( filePtr );

        // skip leading '/'
        start = filePtr + 15;

        // Only want directory names at the current depth.
        for ( c = start; c != '\0'; c++ )
        {
            if ( *c == '/' || *c == '\\' )
            {
                *c = '\0';
                break;
            }
        }

        G_AddPlayerModel(start);
    }
}

qboolean G_IsValidPlayerModel(const char *model)
{
    return !_is_playermodel_uniq(model);
}

void G_FreePlayerModel(void)
{
    int i;
    for ( i = 0; i < level.playerModelCount; i++ )
        BG_Free( level.playerModel[i] );
}

//-------------------------------------------------------------------------
// Skins
//-------------------------------------------------------------------------

void G_GetPlayerModelSkins( const char *modelname, char skins[][ 64 ],
 int maxskins, int *numskins )
{
    char fileList[ 16*1024 ] = {""};
    int nFiles;
    char *filePtr;
    int fileLen = 0;
    int i;

    *numskins = 0;
    nFiles = trap_FS_GetFilteredFiles("models/players", ".skin",
            va("models*players*%s*skin", modelname),
            fileList, sizeof(fileList));
    filePtr = fileList;
    for (i = 0; i < nFiles && i < maxskins; i++ )
    {
        char *start, *end;

        fileLen = strlen( filePtr );

        start = filePtr;
        start += strlen(va("models/players/%s/", modelname));

        end = filePtr + fileLen;
        end -= 5;
        *end = '\0';
        filePtr += fileLen + 1;

        // dumb way to filter out the unique skins of segmented and
        // nonsegmented models.
        // TODO: Stop writing code at 4am.
        if ( start[0] == 'h'
          && start[1] == 'e'
          && start[2] == 'a'
          && start[3] == 'd'
          && start[4] == '_' )
            start += 5;

        else if ( start[0] == 'n'
               && start[1] == 'o'
               && start[2] == 'n'
               && start[3] == 's'
               && start[4] == 'e'
               && start[5] == 'g'
               && start[6] == '_' )
            start += 7;

        else
            continue;

        strncpy(skins[*numskins], start, 64 );
        (*numskins)++;
    }
}

/*
======================
GetSkin

Probably should be called GetSkin[or]Default. Tries to recreate what
appears to be an undocumented set of conventions that must be allowed
in other q3 derives.

This algorithm is not really good enough for Tremulous considering
armour + upgrade/advanced in gameplay

XXX Move this into bg_
======================
*/
char *GetSkin( char *modelname, char *wish )
{
    char skins[ MAX_PLAYER_MODEL ][ 64 ];
    int numskins;
    int i;
    qboolean foundDefault = qfalse;
    qboolean foundSelfNamed = qfalse;
    static char lastpick[ 64 ] = {""};
    lastpick[0] = '\0'; // reset static buf

    G_GetPlayerModelSkins(modelname, skins, MAX_PLAYER_MODEL, &numskins);

    for (i = 0; i < numskins; i++)
    {
        if ( i == 0 )
            strncpy(lastpick, skins[0], 64 );

        if ( !strcmp(wish, skins[i]) )
            return wish;
        else if ( !strcmp("default", skins[i]))
            foundDefault = qtrue;
        else if ( !strcmp(modelname, skins[i]))
            foundSelfNamed = qtrue;
    }

    if (foundDefault)
        return "default";
    else if (foundSelfNamed)
        return modelname;

    return lastpick;
}

