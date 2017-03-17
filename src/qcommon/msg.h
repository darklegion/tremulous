#ifndef QCOMMON_MSG_H
#define QCOMMON_MSG_H 1

#include <stdint.h>

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

void MSG_Init(struct msg_t *buf, uint8_t *data, int length);
void MSG_InitOOB(struct msg_t *buf, uint8_t *data, int length);
void MSG_Clear(struct msg_t *buf);
void MSG_WriteData(struct msg_t *buf, const void *data, int length);
void MSG_Bitstream(struct msg_t *buf);

// TTimo
// copy a struct msg_t in case we need to store it as is for a bit
// (as I needed this to keep an struct msg_t from a static var for later use)
// sets data buffer as MSG_Init does prior to do the copy
void MSG_Copy(struct msg_t *buf, uint8_t *data, int length, struct msg_t *src);

typedef struct usercmd_s usercmd_t;
typedef struct entityState_s entityState_t;
typedef struct playerState_s playerState_t;

void MSG_WriteBits(struct msg_t *msg, int value, int bits);

void MSG_WriteChar(struct msg_t *sb, int c);
void MSG_WriteByte(struct msg_t *sb, int c);
void MSG_WriteShort(struct msg_t *sb, int c);
void MSG_WriteLong(struct msg_t *sb, int c);
void MSG_WriteFloat(struct msg_t *sb, float f);
void MSG_WriteString(struct msg_t *sb, const char *s);
void MSG_WriteBigString(struct msg_t *sb, const char *s);
void MSG_WriteAngle16(struct msg_t *sb, float f);
int MSG_HashKey(int alternateProtocol, const char *string, int maxlen);

void MSG_BeginReading(struct msg_t *sb);
void MSG_BeginReadingOOB(struct msg_t *sb);

int MSG_ReadBits(struct msg_t *msg, int bits);

int MSG_ReadChar(struct msg_t *sb);
int MSG_ReadByte(struct msg_t *sb);
int MSG_ReadShort(struct msg_t *sb);
int MSG_ReadLong(struct msg_t *sb);
float MSG_ReadFloat(struct msg_t *sb);
char *MSG_ReadString(struct msg_t *sb);
char *MSG_ReadBigString(struct msg_t *sb);
char *MSG_ReadStringLine(struct msg_t *sb);
float MSG_ReadAngle16(struct msg_t *sb);
void MSG_ReadData(struct msg_t *sb, void *buffer, int size);
int MSG_LookaheadByte(struct msg_t *msg);

void MSG_WriteDeltaUsercmdKey(struct msg_t *msg, int key, usercmd_t *from, usercmd_t *to);
void MSG_ReadDeltaUsercmdKey(struct msg_t *msg, int key, usercmd_t *from, usercmd_t *to);

void MSG_WriteDeltaEntity(int alternateProtocol, struct msg_t *msg, struct entityState_s *from, struct entityState_s *to, bool force);
void MSG_ReadDeltaEntity(int alternateProtocol, struct msg_t *msg, entityState_t *from, entityState_t *to, int number);

void MSG_WriteDeltaPlayerstate(int alternateProtocol, struct msg_t *msg, struct playerState_s *from, struct playerState_s *to);
void MSG_ReadDeltaPlayerstate(struct msg_t *msg, struct playerState_s *from, struct playerState_s *to);

struct alternatePlayerState_t;
void MSG_ReadDeltaAlternatePlayerstate(struct msg_t *msg, struct alternatePlayerState_t *from, struct alternatePlayerState_t *to);

void MSG_ReportChangeVectors_f(void);

#endif
