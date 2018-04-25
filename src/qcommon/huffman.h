#ifndef QCOMMON_HUFFMAN_H
#define QCOMMON_HUFFMAN_H 1

#include <stdint.h>

struct msg_t;

/* This is based on the Adaptive Huffman algorithm described in Sayood's Data
 * Compression book.  The ranks are not actually stored, but implicitly defined
 * by the location of a node within a doubly-linked list */

#define NYT HMAX /* NYT = Not Yet Transmitted */
#define INTERNAL_NODE (HMAX + 1)

typedef struct nodetype {
    struct nodetype *left, *right, *parent; /* tree structure */
    struct nodetype *next, *prev; /* doubly-linked list */
    struct nodetype **head; /* highest ranked node in block */
    int weight;
    int symbol;
} huffNode_t;

#define HMAX 256 /* Maximum symbol */

typedef struct {
    int blocNode;
    int blocPtrs;

    huffNode_t *tree;
    huffNode_t *lhead;
    huffNode_t *ltail;
    huffNode_t *loc[HMAX + 1];
    huffNode_t **freelist;

    huffNode_t nodeList[768];
    huffNode_t *nodePtrs[768];
} huff_t;

typedef struct {
    huff_t compressor;
    huff_t decompressor;
} huffman_t;

void Huff_Compress(msg_t *buf, int offset);
void Huff_Decompress(msg_t *buf, int offset);
void Huff_Init(huffman_t *huff);
void Huff_addRef(huff_t *huff, uint8_t ch);
int Huff_Receive(huffNode_t *node, int *ch, uint8_t *fin);
void Huff_transmit(huff_t *huff, int ch, uint8_t *fout, int maxoffset);
void Huff_offsetReceive(huffNode_t *node, int *ch, uint8_t *fin, int *offset, int maxoffset);
void Huff_offsetTransmit(huff_t *huff, int ch, uint8_t *fout, int *offset, int maxoffset);
void Huff_putBit(int bit, uint8_t *fout, int *offset);
int Huff_getBit(uint8_t *fout, int *offset);

// don't use if you don't know what you're doing.
int Huff_getBloc(void);
void Huff_setBloc(int _bloc);

extern huffman_t clientHuffTables;

#endif
