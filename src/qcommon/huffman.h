#ifndef QCOMMON_HUFFMAN_H
#define QCOMMON_HUFFMAN_H 1

#include <stdint.h>

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
} node_t;

#define HMAX 256 /* Maximum symbol */

typedef struct {
    int blocNode;
    int blocPtrs;

    node_t *tree;
    node_t *lhead;
    node_t *ltail;
    node_t *loc[HMAX + 1];
    node_t **freelist;

    node_t nodeList[768];
    node_t *nodePtrs[768];
} huff_t;

typedef struct {
    huff_t compressor;
    huff_t decompressor;
} huffman_t;

void Huff_Compress(struct msg_t *buf, int offset);
void Huff_Decompress(struct msg_t *buf, int offset);
void Huff_Init(huffman_t *huff);
void Huff_addRef(huff_t *huff, uint8_t ch);
int Huff_Receive(node_t *node, int *ch, uint8_t *fin);
void Huff_transmit(huff_t *huff, int ch, uint8_t *fout, int maxoffset);
void Huff_offsetReceive(node_t *node, int *ch, uint8_t *fin, int *offset, int maxoffset);
void Huff_offsetTransmit(huff_t *huff, int ch, uint8_t *fout, int *offset, int maxoffset);
void Huff_putBit(int bit, uint8_t *fout, int *offset);
int Huff_getBit(uint8_t *fout, int *offset);

// don't use if you don't know what you're doing.
int Huff_getBloc(void);
void Huff_setBloc(int _bloc);

extern huffman_t clientHuffTables;

#endif
