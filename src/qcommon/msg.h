#ifndef QCOMMON_MSG_H
#define QCOMMON_MSG_H 1

#include <stdint.h>

struct usercmd_t;
struct playerState_t;
struct alternatePlayerState_t;
struct entityState_t;

//
// msg.c
//
struct msg_t {
    bool allowoverflow;  // if false, do a Com_Error
    bool overflowed;  // set to true if the buffer size failed (with allowoverflow set)
    bool oob;  // set to true if the buffer size failed (with allowoverflow set)
    uint8_t *data;
    int maxsize;
    int cursize;
    int readcount;
    int bit;  // for bitwise reads and writes
};

void MSG_Init(msg_t *buf, uint8_t *data, int length);
void MSG_InitOOB(msg_t *buf, uint8_t *data, int length);
void MSG_Clear(msg_t *buf);
void MSG_WriteData(msg_t *buf, const void *data, int length);
void MSG_Bitstream(msg_t *buf);

// TTimo
// copy a msg_t in case we need to store it as is for a bit
// (as I needed this to keep an msg_t from a static var for later use)
// sets data buffer as MSG_Init does prior to do the copy
void MSG_Copy(msg_t *buf, uint8_t *data, int length, msg_t *src);

void MSG_WriteBits(msg_t *msg, int value, int bits);

void MSG_WriteChar(msg_t *sb, int c);
void MSG_WriteByte(msg_t *sb, int c);
void MSG_WriteShort(msg_t *sb, int c);
void MSG_WriteLong(msg_t *sb, int c);
void MSG_WriteFloat(msg_t *sb, float f);
void MSG_WriteString(msg_t *sb, const char *s);
void MSG_WriteBigString(msg_t *sb, const char *s);
void MSG_WriteAngle16(msg_t *sb, float f);

int MSG_HashKey(int alternateProtocol, const char *string, int maxlen);

void MSG_BeginReading(msg_t *sb);
void MSG_BeginReadingOOB(msg_t *sb);

int MSG_ReadBits(msg_t *msg, int bits);

int MSG_ReadChar(msg_t *sb);
int MSG_ReadByte(msg_t *sb);
int MSG_ReadShort(msg_t *sb);
int MSG_ReadLong(msg_t *sb);
float MSG_ReadFloat(msg_t *sb);
char *MSG_ReadString(msg_t *sb);
char *MSG_ReadBigString(msg_t *sb);
char *MSG_ReadStringLine(msg_t *sb);
float MSG_ReadAngle16(msg_t *sb);
void MSG_ReadData(msg_t *sb, void *buffer, int size);
int MSG_LookaheadByte(msg_t *msg);

void MSG_WriteDeltaUsercmdKey(msg_t *msg, int key, usercmd_t *from, usercmd_t *to);
void MSG_ReadDeltaUsercmdKey(msg_t *msg, int key, usercmd_t *from, usercmd_t *to);

void MSG_WriteDeltaEntity(int alternateProtocol, msg_t *msg, entityState_t *from, entityState_t *to, bool force);
void MSG_ReadDeltaEntity(int alternateProtocol, msg_t *msg, entityState_t *from, entityState_t *to, int number);

void MSG_WriteDeltaPlayerstate(int alternateProtocol, msg_t *msg, playerState_t *from, playerState_t *to);
void MSG_ReadDeltaPlayerstate(msg_t *msg, playerState_t *from, playerState_t *to);

void MSG_ReadDeltaAlternatePlayerstate(msg_t *msg, alternatePlayerState_t *from, alternatePlayerState_t *to);

void MSG_ReportChangeVectors_f(void);

#endif
