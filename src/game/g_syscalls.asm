code

equ trap_Printf                     -1
equ trap_Error                      -2
equ trap_Milliseconds               -3
equ trap_Cvar_Register              -4
equ trap_Cvar_Update                -5
equ trap_Cvar_Set                   -6
equ trap_Cvar_VariableIntegerValue  -7
equ trap_Cvar_VariableStringBuffer  -8
equ trap_Argc                       -9
equ trap_Argv                       -10
equ trap_FS_FOpenFile               -11
equ trap_FS_Read                    -12
equ trap_FS_Write                   -13
equ trap_FS_FCloseFile              -14
equ trap_SendConsoleCommand         -15
equ trap_LocateGameData             -16
equ trap_DropClient                 -17
equ trap_SendServerCommand          -18
equ trap_SetConfigstring            -19
equ trap_GetConfigstring            -20
equ trap_GetUserinfo                -21
equ trap_SetUserinfo                -22
equ trap_GetServerinfo              -23
equ trap_SetBrushModel              -24
equ trap_Trace                      -25
equ trap_PointContents              -26
equ trap_InPVS                      -27
equ trap_InPVSIgnorePortals         -28
equ trap_AdjustAreaPortalState      -29
equ trap_AreasConnected             -30
equ trap_LinkEntity                 -31
equ trap_UnlinkEntity               -32
equ trap_EntitiesInBox              -33
equ trap_EntityContact              -34
equ trap_GetUsercmd                 -35
equ trap_GetEntityToken             -36
equ trap_FS_GetFileList             -37
equ trap_RealTime                   -38
equ trap_SnapVector                 -39
equ trap_TraceCapsule               -40
equ trap_EntityContactCapsule       -41
equ trap_FS_Seek                    -42

equ trap_Parse_AddGlobalDefine      -43
equ trap_Parse_LoadSource           -44
equ trap_Parse_FreeSource           -45
equ trap_Parse_ReadToken            -46
equ trap_Parse_SourceFileAndLine    -47

equ trap_SendGameStat               -48


equ memset                          -101
equ memcpy                          -102
equ strncpy                         -103
equ sin                             -104
equ cos                             -105
equ atan2                           -106
equ sqrt                            -107
equ floor                           -111
equ ceil                            -112
equ testPrintInt                    -113
equ testPrintFloat                  -114
