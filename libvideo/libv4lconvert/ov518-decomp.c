/* We would like to embed this inside libv4l, but we cannot as I've failed
   to contact Mark W. McClelland to get permission to relicense this,
   so this lives in an external (GPL licensed) helper */

/* OV518 Decompression Support Module (No-MMX version)
 *
 * Copyright (c) 2002-2003 Mark W. McClelland. All rights reserved.
 * http://alpha.dyndns.org/ov511/
 *
 * Fast integer iDCT by Yuri van Oers <yvanoers AT xs4all.nl>
 * Original OV511 decompression code Copyright 1998-2000 OmniVision Technologies
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.
 */

#include <string.h>
#include <unistd.h>
#include "helper-funcs.h"

/******************************************************************************
 * Compile-time Options
 ******************************************************************************/

/* Defining APPROXIMATE_MUL_BY_SHIFT increases performance by approximation
 * the multiplications by shifts. I think there's no change in the
 * calculated picture, but I'm not sure, so the choice is still in here. */
#undef APPROXIMATE_MUL_BY_SHIFT

#define CLIP(x) ((u8) ((x) > 0xFF ? 0xFF : (x) < 0 ? 0 : (x)))
/******************************************************************************
 * Local Data Types
 ******************************************************************************/

/* Make sure this remains naturally aligned and 2^n bytes in size */
struct tree_node {
	short left;		/* Pointer to left child node */
	short right;		/* Pointer to right child node */
	signed char depth;	/* Depth (starting at 1) if leaf, else -1 */
	signed char coeffbits;	/* Size of coefficient data, or zero if none */
	signed char skip;	/* Number of zero coefficients. Unused w/ DC */
	char padding;		/* Pad out to 8 bytes */
};

struct comp_info {
	int bytes;		/* Number of processed input bytes */
	int bits;		/* Number of unprocessed input bits */
	int rawLen;		/* Total number of bytes in input buffer */
	u8 *qt;	/* Current quantization table */
};

/******************************************************************************
 * Constant Data Definitions
 ******************************************************************************/

/* Zig-Zag Table */
static const u8 ZigZag518[] = {
	0x00, 0x02, 0x03, 0x09,
	0x01, 0x04, 0x08, 0x0a,
	0x05, 0x07, 0x0b, 0x11,
	0x06, 0x0c, 0x10, 0x12,
	0x0d, 0x0f, 0x13, 0x19,
	0x0e, 0x14, 0x18, 0x1a,
	0x15, 0x17, 0x1b, 0x1e,
	0x16, 0x1c, 0x1d, 0x1f
};

/* Huffman trees */

static const struct tree_node treeYAC[] = {
	{  1,   4, -1,  0, -1, 0}, {  2,   3, -1,  0, -1, 0},
	{ -1,  -1,  2,  1,  0, 0}, { -1,  -1,  2,  2,  0, 0},
	{  5,   9, -1,  0, -1, 0}, {  6,   7, -1,  0, -1, 0},
	{ -1,  -1,  3,  3,  0, 0}, {323,   8, -1,  0, -1, 0},
	{ -1,  -1,  4,  4,  0, 0}, { 10,  13, -1,  0, -1, 0},
	{ 38,  11, -1,  0, -1, 0}, { 12,  39, -1,  0, -1, 0},
	{ -1,  -1,  5,  5,  0, 0}, { 59,  14, -1,  0, -1, 0},
	{ 15,  18, -1,  0, -1, 0}, { 16, 113, -1,  0, -1, 0},
	{ 17,  40, -1,  0, -1, 0}, { -1,  -1,  7,  6,  0, 0},
	{ 19,  22, -1,  0, -1, 0}, { 20,  41, -1,  0, -1, 0},
	{ 21,  61, -1,  0, -1, 0}, { -1,  -1,  8,  7,  0, 0},
	{ 23,  27, -1,  0, -1, 0}, {169,  24, -1,  0, -1, 0},
	{208,  25, -1,  0, -1, 0}, { 26,  62, -1,  0, -1, 0},
	{ -1,  -1, 10,  8,  0, 0}, { 44,  28, -1,  0, -1, 0},
	{ 63,  29, -1,  0, -1, 0}, { 30, 191, -1,  0, -1, 0},
	{ 31, 119, -1,  0, -1, 0}, { 32,  82, -1,  0, -1, 0},
	{ 33,  55, -1,  0, -1, 0}, { 34,  48, -1,  0, -1, 0},
	{171,  35, -1,  0, -1, 0}, { 36,  37, -1,  0, -1, 0},
	{ -1,  -1, 16,  9,  0, 0}, { -1,  -1, 16, 10,  0, 0},
	{ -1,  -1,  4,  1,  1, 0}, { -1,  -1,  5,  2,  1, 0},
	{ -1,  -1,  7,  3,  1, 0}, {151,  42, -1,  0, -1, 0},
	{ 43,  79, -1,  0, -1, 0}, { -1,  -1,  9,  4,  1, 0},
	{ 96,  45, -1,  0, -1, 0}, {246,  46, -1,  0, -1, 0},
	{ 47, 115, -1,  0, -1, 0}, { -1,  -1, 11,  5,  1, 0},
	{ 49,  52, -1,  0, -1, 0}, { 50,  51, -1,  0, -1, 0},
	{ -1,  -1, 16,  6,  1, 0}, { -1,  -1, 16,  7,  1, 0},
	{ 53,  54, -1,  0, -1, 0}, { -1,  -1, 16,  8,  1, 0},
	{ -1,  -1, 16,  9,  1, 0}, { 56,  71, -1,  0, -1, 0},
	{ 57,  68, -1,  0, -1, 0}, { 58,  67, -1,  0, -1, 0},
	{ -1,  -1, 16, 10,  1, 0}, { 60,  77, -1,  0, -1, 0},
	{ -1,  -1,  5,  1,  2, 0}, { -1,  -1,  8,  2,  2, 0},
	{ -1,  -1, 10,  3,  2, 0}, {265,  64, -1,  0, -1, 0},
	{ 65, 134, -1,  0, -1, 0}, { 66,  80, -1,  0, -1, 0},
	{ -1,  -1, 12,  4,  2, 0}, { -1,  -1, 16,  5,  2, 0},
	{ 69,  70, -1,  0, -1, 0}, { -1,  -1, 16,  6,  2, 0},
	{ -1,  -1, 16,  7,  2, 0}, { 72,  75, -1,  0, -1, 0},
	{ 73,  74, -1,  0, -1, 0}, { -1,  -1, 16,  8,  2, 0},
	{ -1,  -1, 16,  9,  2, 0}, { 76,  81, -1,  0, -1, 0},
	{ -1,  -1, 16, 10,  2, 0}, { 78,  95, -1,  0, -1, 0},
	{ -1,  -1,  6,  1,  3, 0}, { -1,  -1,  9,  2,  3, 0},
	{ -1,  -1, 12,  3,  3, 0}, { -1,  -1, 16,  4,  3, 0},
	{ 83, 101, -1,  0, -1, 0}, { 84,  91, -1,  0, -1, 0},
	{ 85,  88, -1,  0, -1, 0}, { 86,  87, -1,  0, -1, 0},
	{ -1,  -1, 16,  5,  3, 0}, { -1,  -1, 16,  6,  3, 0},
	{ 89,  90, -1,  0, -1, 0}, { -1,  -1, 16,  7,  3, 0},
	{ -1,  -1, 16,  8,  3, 0}, { 92,  98, -1,  0, -1, 0},
	{ 93,  94, -1,  0, -1, 0}, { -1,  -1, 16,  9,  3, 0},
	{ -1,  -1, 16, 10,  3, 0}, { -1,  -1,  6,  1,  4, 0},
	{ 97, 225, -1,  0, -1, 0}, { -1,  -1, 10,  2,  4, 0},
	{ 99, 100, -1,  0, -1, 0}, { -1,  -1, 16,  3,  4, 0},
	{ -1,  -1, 16,  4,  4, 0}, {102, 109, -1,  0, -1, 0},
	{103, 106, -1,  0, -1, 0}, {104, 105, -1,  0, -1, 0},
	{ -1,  -1, 16,  5,  4, 0}, { -1,  -1, 16,  6,  4, 0},
	{107, 108, -1,  0, -1, 0}, { -1,  -1, 16,  7,  4, 0},
	{ -1,  -1, 16,  8,  4, 0}, {110, 116, -1,  0, -1, 0},
	{111, 112, -1,  0, -1, 0}, { -1,  -1, 16,  9,  4, 0},
	{ -1,  -1, 16, 10,  4, 0}, {114, 133, -1,  0, -1, 0},
	{ -1,  -1,  7,  1,  5, 0}, { -1,  -1, 11,  2,  5, 0},
	{117, 118, -1,  0, -1, 0}, { -1,  -1, 16,  3,  5, 0},
	{ -1,  -1, 16,  4,  5, 0}, {120, 156, -1,  0, -1, 0},
	{121, 139, -1,  0, -1, 0}, {122, 129, -1,  0, -1, 0},
	{123, 126, -1,  0, -1, 0}, {124, 125, -1,  0, -1, 0},
	{ -1,  -1, 16,  5,  5, 0}, { -1,  -1, 16,  6,  5, 0},
	{127, 128, -1,  0, -1, 0}, { -1,  -1, 16,  7,  5, 0},
	{ -1,  -1, 16,  8,  5, 0}, {130, 136, -1,  0, -1, 0},
	{131, 132, -1,  0, -1, 0}, { -1,  -1, 16,  9,  5, 0},
	{ -1,  -1, 16, 10,  5, 0}, { -1,  -1,  7,  1,  6, 0},
	{135, 152, -1,  0, -1, 0}, { -1,  -1, 12,  2,  6, 0},
	{137, 138, -1,  0, -1, 0}, { -1,  -1, 16,  3,  6, 0},
	{ -1,  -1, 16,  4,  6, 0}, {140, 147, -1,  0, -1, 0},
	{141, 144, -1,  0, -1, 0}, {142, 143, -1,  0, -1, 0},
	{ -1,  -1, 16,  5,  6, 0}, { -1,  -1, 16,  6,  6, 0},
	{145, 146, -1,  0, -1, 0}, { -1,  -1, 16,  7,  6, 0},
	{ -1,  -1, 16,  8,  6, 0}, {148, 153, -1,  0, -1, 0},
	{149, 150, -1,  0, -1, 0}, { -1,  -1, 16,  9,  6, 0},
	{ -1,  -1, 16, 10,  6, 0}, { -1,  -1,  8,  1,  7, 0},
	{ -1,  -1, 12,  2,  7, 0}, {154, 155, -1,  0, -1, 0},
	{ -1,  -1, 16,  3,  7, 0}, { -1,  -1, 16,  4,  7, 0},
	{157, 175, -1,  0, -1, 0}, {158, 165, -1,  0, -1, 0},
	{159, 162, -1,  0, -1, 0}, {160, 161, -1,  0, -1, 0},
	{ -1,  -1, 16,  5,  7, 0}, { -1,  -1, 16,  6,  7, 0},
	{163, 164, -1,  0, -1, 0}, { -1,  -1, 16,  7,  7, 0},
	{ -1,  -1, 16,  8,  7, 0}, {166, 172, -1,  0, -1, 0},
	{167, 168, -1,  0, -1, 0}, { -1,  -1, 16,  9,  7, 0},
	{ -1,  -1, 16, 10,  7, 0}, {170, 187, -1,  0, -1, 0},
	{ -1,  -1,  9,  1,  8, 0}, { -1,  -1, 15,  2,  8, 0},
	{173, 174, -1,  0, -1, 0}, { -1,  -1, 16,  3,  8, 0},
	{ -1,  -1, 16,  4,  8, 0}, {176, 183, -1,  0, -1, 0},
	{177, 180, -1,  0, -1, 0}, {178, 179, -1,  0, -1, 0},
	{ -1,  -1, 16,  5,  8, 0}, { -1,  -1, 16,  6,  8, 0},
	{181, 182, -1,  0, -1, 0}, { -1,  -1, 16,  7,  8, 0},
	{ -1,  -1, 16,  8,  8, 0}, {184, 188, -1,  0, -1, 0},
	{185, 186, -1,  0, -1, 0}, { -1,  -1, 16,  9,  8, 0},
	{ -1,  -1, 16, 10,  8, 0}, { -1,  -1,  9,  1,  9, 0},
	{189, 190, -1,  0, -1, 0}, { -1,  -1, 16,  2,  9, 0},
	{ -1,  -1, 16,  3,  9, 0}, {192, 258, -1,  0, -1, 0},
	{193, 226, -1,  0, -1, 0}, {194, 210, -1,  0, -1, 0},
	{195, 202, -1,  0, -1, 0}, {196, 199, -1,  0, -1, 0},
	{197, 198, -1,  0, -1, 0}, { -1,  -1, 16,  4,  9, 0},
	{ -1,  -1, 16,  5,  9, 0}, {200, 201, -1,  0, -1, 0},
	{ -1,  -1, 16,  6,  9, 0}, { -1,  -1, 16,  7,  9, 0},
	{203, 206, -1,  0, -1, 0}, {204, 205, -1,  0, -1, 0},
	{ -1,  -1, 16,  8,  9, 0}, { -1,  -1, 16,  9,  9, 0},
	{207, 209, -1,  0, -1, 0}, { -1,  -1, 16, 10,  9, 0},
	{ -1,  -1,  9,  1, 10, 0}, { -1,  -1, 16,  2, 10, 0},
	{211, 218, -1,  0, -1, 0}, {212, 215, -1,  0, -1, 0},
	{213, 214, -1,  0, -1, 0}, { -1,  -1, 16,  3, 10, 0},
	{ -1,  -1, 16,  4, 10, 0}, {216, 217, -1,  0, -1, 0},
	{ -1,  -1, 16,  5, 10, 0}, { -1,  -1, 16,  6, 10, 0},
	{219, 222, -1,  0, -1, 0}, {220, 221, -1,  0, -1, 0},
	{ -1,  -1, 16,  7, 10, 0}, { -1,  -1, 16,  8, 10, 0},
	{223, 224, -1,  0, -1, 0}, { -1,  -1, 16,  9, 10, 0},
	{ -1,  -1, 16, 10, 10, 0}, { -1,  -1, 10,  1, 11, 0},
	{227, 242, -1,  0, -1, 0}, {228, 235, -1,  0, -1, 0},
	{229, 232, -1,  0, -1, 0}, {230, 231, -1,  0, -1, 0},
	{ -1,  -1, 16,  2, 11, 0}, { -1,  -1, 16,  3, 11, 0},
	{233, 234, -1,  0, -1, 0}, { -1,  -1, 16,  4, 11, 0},
	{ -1,  -1, 16,  5, 11, 0}, {236, 239, -1,  0, -1, 0},
	{237, 238, -1,  0, -1, 0}, { -1,  -1, 16,  6, 11, 0},
	{ -1,  -1, 16,  7, 11, 0}, {240, 241, -1,  0, -1, 0},
	{ -1,  -1, 16,  8, 11, 0}, { -1,  -1, 16,  9, 11, 0},
	{243, 251, -1,  0, -1, 0}, {244, 248, -1,  0, -1, 0},
	{245, 247, -1,  0, -1, 0}, { -1,  -1, 16, 10, 11, 0},
	{ -1,  -1, 10,  1, 12, 0}, { -1,  -1, 16,  2, 12, 0},
	{249, 250, -1,  0, -1, 0}, { -1,  -1, 16,  3, 12, 0},
	{ -1,  -1, 16,  4, 12, 0}, {252, 255, -1,  0, -1, 0},
	{253, 254, -1,  0, -1, 0}, { -1,  -1, 16,  5, 12, 0},
	{ -1,  -1, 16,  6, 12, 0}, {256, 257, -1,  0, -1, 0},
	{ -1,  -1, 16,  7, 12, 0}, { -1,  -1, 16,  8, 12, 0},
	{259, 292, -1,  0, -1, 0}, {260, 277, -1,  0, -1, 0},
	{261, 270, -1,  0, -1, 0}, {262, 267, -1,  0, -1, 0},
	{263, 264, -1,  0, -1, 0}, { -1,  -1, 16,  9, 12, 0},
	{ -1,  -1, 16, 10, 12, 0}, {266, 322, -1,  0, -1, 0},
	{ -1,  -1, 11,  1, 13, 0}, {268, 269, -1,  0, -1, 0},
	{ -1,  -1, 16,  2, 13, 0}, { -1,  -1, 16,  3, 13, 0},
	{271, 274, -1,  0, -1, 0}, {272, 273, -1,  0, -1, 0},
	{ -1,  -1, 16,  4, 13, 0}, { -1,  -1, 16,  5, 13, 0},
	{275, 276, -1,  0, -1, 0}, { -1,  -1, 16,  6, 13, 0},
	{ -1,  -1, 16,  7, 13, 0}, {278, 285, -1,  0, -1, 0},
	{279, 282, -1,  0, -1, 0}, {280, 281, -1,  0, -1, 0},
	{ -1,  -1, 16,  8, 13, 0}, { -1,  -1, 16,  9, 13, 0},
	{283, 284, -1,  0, -1, 0}, { -1,  -1, 16, 10, 13, 0},
	{ -1,  -1, 16,  1, 14, 0}, {286, 289, -1,  0, -1, 0},
	{287, 288, -1,  0, -1, 0}, { -1,  -1, 16,  2, 14, 0},
	{ -1,  -1, 16,  3, 14, 0}, {290, 291, -1,  0, -1, 0},
	{ -1,  -1, 16,  4, 14, 0}, { -1,  -1, 16,  5, 14, 0},
	{293, 308, -1,  0, -1, 0}, {294, 301, -1,  0, -1, 0},
	{295, 298, -1,  0, -1, 0}, {296, 297, -1,  0, -1, 0},
	{ -1,  -1, 16,  6, 14, 0}, { -1,  -1, 16,  7, 14, 0},
	{299, 300, -1,  0, -1, 0}, { -1,  -1, 16,  8, 14, 0},
	{ -1,  -1, 16,  9, 14, 0}, {302, 305, -1,  0, -1, 0},
	{303, 304, -1,  0, -1, 0}, { -1,  -1, 16, 10, 14, 0},
	{ -1,  -1, 16,  1, 15, 0}, {306, 307, -1,  0, -1, 0},
	{ -1,  -1, 16,  2, 15, 0}, { -1,  -1, 16,  3, 15, 0},
	{309, 316, -1,  0, -1, 0}, {310, 313, -1,  0, -1, 0},
	{311, 312, -1,  0, -1, 0}, { -1,  -1, 16,  4, 15, 0},
	{ -1,  -1, 16,  5, 15, 0}, {314, 315, -1,  0, -1, 0},
	{ -1,  -1, 16,  6, 15, 0}, { -1,  -1, 16,  7, 15, 0},
	{317, 320, -1,  0, -1, 0}, {318, 319, -1,  0, -1, 0},
	{ -1,  -1, 16,  8, 15, 0}, { -1,  -1, 16,  9, 15, 0},
	{321,  -1, -1,  0, -1, 0}, { -1,  -1, 16, 10, 15, 0},
	{ -1,  -1, 11,  0, 16, 0}, { -1,  -1,  4,  0, -1, 0},
};

static const struct tree_node treeUVAC[] = {
	{  1,   3, -1,  0, -1, 0}, {323,   2, -1,  0, -1, 0},
	{ -1,  -1,  2,  1,  0, 0}, {  4,   8, -1,  0, -1, 0},
	{  5,   6, -1,  0, -1, 0}, { -1,  -1,  3,  2,  0, 0},
	{  7,  37, -1,  0, -1, 0}, { -1,  -1,  4,  3,  0, 0},
	{  9,  13, -1,  0, -1, 0}, { 10,  60, -1,  0, -1, 0},
	{ 11,  12, -1,  0, -1, 0}, { -1,  -1,  5,  4,  0, 0},
	{ -1,  -1,  5,  5,  0, 0}, { 14,  17, -1,  0, -1, 0},
	{ 15,  97, -1,  0, -1, 0}, { 16,  38, -1,  0, -1, 0},
	{ -1,  -1,  6,  6,  0, 0}, { 18,  21, -1,  0, -1, 0},
	{ 19,  39, -1,  0, -1, 0}, { 20, 135, -1,  0, -1, 0},
	{ -1,  -1,  7,  7,  0, 0}, { 22,  26, -1,  0, -1, 0},
	{ 82,  23, -1,  0, -1, 0}, { 24,  99, -1,  0, -1, 0},
	{ 25,  42, -1,  0, -1, 0}, { -1,  -1,  9,  8,  0, 0},
	{ 27,  31, -1,  0, -1, 0}, {211,  28, -1,  0, -1, 0},
	{248,  29, -1,  0, -1, 0}, { 30,  63, -1,  0, -1, 0},
	{ -1,  -1, 10,  9,  0, 0}, { 43,  32, -1,  0, -1, 0},
	{ 33,  48, -1,  0, -1, 0}, {153,  34, -1,  0, -1, 0},
	{ 35,  64, -1,  0, -1, 0}, { 36,  47, -1,  0, -1, 0},
	{ -1,  -1, 12, 10,  0, 0}, { -1,  -1,  4,  1,  1, 0},
	{ -1,  -1,  6,  2,  1, 0}, {152,  40, -1,  0, -1, 0},
	{ 41,  62, -1,  0, -1, 0}, { -1,  -1,  8,  3,  1, 0},
	{ -1,  -1,  9,  4,  1, 0}, { 84,  44, -1,  0, -1, 0},
	{322,  45, -1,  0, -1, 0}, { 46, 136, -1,  0, -1, 0},
	{ -1,  -1, 11,  5,  1, 0}, { -1,  -1, 12,  6,  1, 0},
	{ 49, 189, -1,  0, -1, 0}, { 50, 119, -1,  0, -1, 0},
	{ 51,  76, -1,  0, -1, 0}, { 66,  52, -1,  0, -1, 0},
	{ 53,  69, -1,  0, -1, 0}, { 54,  57, -1,  0, -1, 0},
	{ 55,  56, -1,  0, -1, 0}, { -1,  -1, 16,  7,  1, 0},
	{ -1,  -1, 16,  8,  1, 0}, { 58,  59, -1,  0, -1, 0},
	{ -1,  -1, 16,  9,  1, 0}, { -1,  -1, 16, 10,  1, 0},
	{ 61,  81, -1,  0, -1, 0}, { -1,  -1,  5,  1,  2, 0},
	{ -1,  -1,  8,  2,  2, 0}, { -1,  -1, 10,  3,  2, 0},
	{ 65,  86, -1,  0, -1, 0}, { -1,  -1, 12,  4,  2, 0},
	{286,  67, -1,  0, -1, 0}, { 68, 304, -1,  0, -1, 0},
	{ -1,  -1, 15,  5,  2, 0}, { 70,  73, -1,  0, -1, 0},
	{ 71,  72, -1,  0, -1, 0}, { -1,  -1, 16,  6,  2, 0},
	{ -1,  -1, 16,  7,  2, 0}, { 74,  75, -1,  0, -1, 0},
	{ -1,  -1, 16,  8,  2, 0}, { -1,  -1, 16,  9,  2, 0},
	{ 77, 102, -1,  0, -1, 0}, { 78,  91, -1,  0, -1, 0},
	{ 79,  88, -1,  0, -1, 0}, { 80,  87, -1,  0, -1, 0},
	{ -1,  -1, 16, 10,  2, 0}, { -1,  -1,  5,  1,  3, 0},
	{ 83, 171, -1,  0, -1, 0}, { -1,  -1,  8,  2,  3, 0},
	{ 85, 117, -1,  0, -1, 0}, { -1,  -1, 10,  3,  3, 0},
	{ -1,  -1, 12,  4,  3, 0}, { -1,  -1, 16,  5,  3, 0},
	{ 89,  90, -1,  0, -1, 0}, { -1,  -1, 16,  6,  3, 0},
	{ -1,  -1, 16,  7,  3, 0}, { 92,  95, -1,  0, -1, 0},
	{ 93,  94, -1,  0, -1, 0}, { -1,  -1, 16,  8,  3, 0},
	{ -1,  -1, 16,  9,  3, 0}, { 96, 101, -1,  0, -1, 0},
	{ -1,  -1, 16, 10,  3, 0}, { 98, 116, -1,  0, -1, 0},
	{ -1,  -1,  6,  1,  4, 0}, {100, 188, -1,  0, -1, 0},
	{ -1,  -1,  9,  2,  4, 0}, { -1,  -1, 16,  3,  4, 0},
	{103, 110, -1,  0, -1, 0}, {104, 107, -1,  0, -1, 0},
	{105, 106, -1,  0, -1, 0}, { -1,  -1, 16,  4,  4, 0},
	{ -1,  -1, 16,  5,  4, 0}, {108, 109, -1,  0, -1, 0},
	{ -1,  -1, 16,  6,  4, 0}, { -1,  -1, 16,  7,  4, 0},
	{111, 114, -1,  0, -1, 0}, {112, 113, -1,  0, -1, 0},
	{ -1,  -1, 16,  8,  4, 0}, { -1,  -1, 16,  9,  4, 0},
	{115, 118, -1,  0, -1, 0}, { -1,  -1, 16, 10,  4, 0},
	{ -1,  -1,  6,  1,  5, 0}, { -1,  -1, 10,  2,  5, 0},
	{ -1,  -1, 16,  3,  5, 0}, {120, 156, -1,  0, -1, 0},
	{121, 138, -1,  0, -1, 0}, {122, 129, -1,  0, -1, 0},
	{123, 126, -1,  0, -1, 0}, {124, 125, -1,  0, -1, 0},
	{ -1,  -1, 16,  4,  5, 0}, { -1,  -1, 16,  5,  5, 0},
	{127, 128, -1,  0, -1, 0}, { -1,  -1, 16,  6,  5, 0},
	{ -1,  -1, 16,  7,  5, 0}, {130, 133, -1,  0, -1, 0},
	{131, 132, -1,  0, -1, 0}, { -1,  -1, 16,  8,  5, 0},
	{ -1,  -1, 16,  9,  5, 0}, {134, 137, -1,  0, -1, 0},
	{ -1,  -1, 16, 10,  5, 0}, { -1,  -1,  7,  1,  6, 0},
	{ -1,  -1, 11,  2,  6, 0}, { -1,  -1, 16,  3,  6, 0},
	{139, 146, -1,  0, -1, 0}, {140, 143, -1,  0, -1, 0},
	{141, 142, -1,  0, -1, 0}, { -1,  -1, 16,  4,  6, 0},
	{ -1,  -1, 16,  5,  6, 0}, {144, 145, -1,  0, -1, 0},
	{ -1,  -1, 16,  6,  6, 0}, { -1,  -1, 16,  7,  6, 0},
	{147, 150, -1,  0, -1, 0}, {148, 149, -1,  0, -1, 0},
	{ -1,  -1, 16,  8,  6, 0}, { -1,  -1, 16,  9,  6, 0},
	{151, 155, -1,  0, -1, 0}, { -1,  -1, 16, 10,  6, 0},
	{ -1,  -1,  7,  1,  7, 0}, {154, 267, -1,  0, -1, 0},
	{ -1,  -1, 11,  2,  7, 0}, { -1,  -1, 16,  3,  7, 0},
	{157, 173, -1,  0, -1, 0}, {158, 165, -1,  0, -1, 0},
	{159, 162, -1,  0, -1, 0}, {160, 161, -1,  0, -1, 0},
	{ -1,  -1, 16,  4,  7, 0}, { -1,  -1, 16,  5,  7, 0},
	{163, 164, -1,  0, -1, 0}, { -1,  -1, 16,  6,  7, 0},
	{ -1,  -1, 16,  7,  7, 0}, {166, 169, -1,  0, -1, 0},
	{167, 168, -1,  0, -1, 0}, { -1,  -1, 16,  8,  7, 0},
	{ -1,  -1, 16,  9,  7, 0}, {170, 172, -1,  0, -1, 0},
	{ -1,  -1, 16, 10,  7, 0}, { -1,  -1,  8,  1,  8, 0},
	{ -1,  -1, 16,  2,  8, 0}, {174, 181, -1,  0, -1, 0},
	{175, 178, -1,  0, -1, 0}, {176, 177, -1,  0, -1, 0},
	{ -1,  -1, 16,  3,  8, 0}, { -1,  -1, 16,  4,  8, 0},
	{179, 180, -1,  0, -1, 0}, { -1,  -1, 16,  5,  8, 0},
	{ -1,  -1, 16,  6,  8, 0}, {182, 185, -1,  0, -1, 0},
	{183, 184, -1,  0, -1, 0}, { -1,  -1, 16,  7,  8, 0},
	{ -1,  -1, 16,  8,  8, 0}, {186, 187, -1,  0, -1, 0},
	{ -1,  -1, 16,  9,  8, 0}, { -1,  -1, 16, 10,  8, 0},
	{ -1,  -1,  9,  1,  9, 0}, {190, 257, -1,  0, -1, 0},
	{191, 224, -1,  0, -1, 0}, {192, 207, -1,  0, -1, 0},
	{193, 200, -1,  0, -1, 0}, {194, 197, -1,  0, -1, 0},
	{195, 196, -1,  0, -1, 0}, { -1,  -1, 16,  2,  9, 0},
	{ -1,  -1, 16,  3,  9, 0}, {198, 199, -1,  0, -1, 0},
	{ -1,  -1, 16,  4,  9, 0}, { -1,  -1, 16,  5,  9, 0},
	{201, 204, -1,  0, -1, 0}, {202, 203, -1,  0, -1, 0},
	{ -1,  -1, 16,  6,  9, 0}, { -1,  -1, 16,  7,  9, 0},
	{205, 206, -1,  0, -1, 0}, { -1,  -1, 16,  8,  9, 0},
	{ -1,  -1, 16,  9,  9, 0}, {208, 217, -1,  0, -1, 0},
	{209, 214, -1,  0, -1, 0}, {210, 213, -1,  0, -1, 0},
	{ -1,  -1, 16, 10,  9, 0}, {212, 230, -1,  0, -1, 0},
	{ -1,  -1,  9,  1, 10, 0}, { -1,  -1, 16,  2, 10, 0},
	{215, 216, -1,  0, -1, 0}, { -1,  -1, 16,  3, 10, 0},
	{ -1,  -1, 16,  4, 10, 0}, {218, 221, -1,  0, -1, 0},
	{219, 220, -1,  0, -1, 0}, { -1,  -1, 16,  5, 10, 0},
	{ -1,  -1, 16,  6, 10, 0}, {222, 223, -1,  0, -1, 0},
	{ -1,  -1, 16,  7, 10, 0}, { -1,  -1, 16,  8, 10, 0},
	{225, 241, -1,  0, -1, 0}, {226, 234, -1,  0, -1, 0},
	{227, 231, -1,  0, -1, 0}, {228, 229, -1,  0, -1, 0},
	{ -1,  -1, 16,  9, 10, 0}, { -1,  -1, 16, 10, 10, 0},
	{ -1,  -1,  9,  1, 11, 0}, {232, 233, -1,  0, -1, 0},
	{ -1,  -1, 16,  2, 11, 0}, { -1,  -1, 16,  3, 11, 0},
	{235, 238, -1,  0, -1, 0}, {236, 237, -1,  0, -1, 0},
	{ -1,  -1, 16,  4, 11, 0}, { -1,  -1, 16,  5, 11, 0},
	{239, 240, -1,  0, -1, 0}, { -1,  -1, 16,  6, 11, 0},
	{ -1,  -1, 16,  7, 11, 0}, {242, 250, -1,  0, -1, 0},
	{243, 246, -1,  0, -1, 0}, {244, 245, -1,  0, -1, 0},
	{ -1,  -1, 16,  8, 11, 0}, { -1,  -1, 16,  9, 11, 0},
	{247, 249, -1,  0, -1, 0}, { -1,  -1, 16, 10, 11, 0},
	{ -1,  -1,  9,  1, 12, 0}, { -1,  -1, 16,  2, 12, 0},
	{251, 254, -1,  0, -1, 0}, {252, 253, -1,  0, -1, 0},
	{ -1,  -1, 16,  3, 12, 0}, { -1,  -1, 16,  4, 12, 0},
	{255, 256, -1,  0, -1, 0}, { -1,  -1, 16,  5, 12, 0},
	{ -1,  -1, 16,  6, 12, 0}, {258, 291, -1,  0, -1, 0},
	{259, 275, -1,  0, -1, 0}, {260, 268, -1,  0, -1, 0},
	{261, 264, -1,  0, -1, 0}, {262, 263, -1,  0, -1, 0},
	{ -1,  -1, 16,  7, 12, 0}, { -1,  -1, 16,  8, 12, 0},
	{265, 266, -1,  0, -1, 0}, { -1,  -1, 16,  9, 12, 0},
	{ -1,  -1, 16, 10, 12, 0}, { -1,  -1, 11,  1, 13, 0},
	{269, 272, -1,  0, -1, 0}, {270, 271, -1,  0, -1, 0},
	{ -1,  -1, 16,  2, 13, 0}, { -1,  -1, 16,  3, 13, 0},
	{273, 274, -1,  0, -1, 0}, { -1,  -1, 16,  4, 13, 0},
	{ -1,  -1, 16,  5, 13, 0}, {276, 283, -1,  0, -1, 0},
	{277, 280, -1,  0, -1, 0}, {278, 279, -1,  0, -1, 0},
	{ -1,  -1, 16,  6, 13, 0}, { -1,  -1, 16,  7, 13, 0},
	{281, 282, -1,  0, -1, 0}, { -1,  -1, 16,  8, 13, 0},
	{ -1,  -1, 16,  9, 13, 0}, {284, 288, -1,  0, -1, 0},
	{285, 287, -1,  0, -1, 0}, { -1,  -1, 16, 10, 13, 0},
	{ -1,  -1, 14,  1, 14, 0}, { -1,  -1, 16,  2, 14, 0},
	{289, 290, -1,  0, -1, 0}, { -1,  -1, 16,  3, 14, 0},
	{ -1,  -1, 16,  4, 14, 0}, {292, 308, -1,  0, -1, 0},
	{293, 300, -1,  0, -1, 0}, {294, 297, -1,  0, -1, 0},
	{295, 296, -1,  0, -1, 0}, { -1,  -1, 16,  5, 14, 0},
	{ -1,  -1, 16,  6, 14, 0}, {298, 299, -1,  0, -1, 0},
	{ -1,  -1, 16,  7, 14, 0}, { -1,  -1, 16,  8, 14, 0},
	{301, 305, -1,  0, -1, 0}, {302, 303, -1,  0, -1, 0},
	{ -1,  -1, 16,  9, 14, 0}, { -1,  -1, 16, 10, 14, 0},
	{ -1,  -1, 15,  1, 15, 0}, {306, 307, -1,  0, -1, 0},
	{ -1,  -1, 16,  2, 15, 0}, { -1,  -1, 16,  3, 15, 0},
	{309, 316, -1,  0, -1, 0}, {310, 313, -1,  0, -1, 0},
	{311, 312, -1,  0, -1, 0}, { -1,  -1, 16,  4, 15, 0},
	{ -1,  -1, 16,  5, 15, 0}, {314, 315, -1,  0, -1, 0},
	{ -1,  -1, 16,  6, 15, 0}, { -1,  -1, 16,  7, 15, 0},
	{317, 320, -1,  0, -1, 0}, {318, 319, -1,  0, -1, 0},
	{ -1,  -1, 16,  8, 15, 0}, { -1,  -1, 16,  9, 15, 0},
	{321,  -1, -1,  0, -1, 0}, { -1,  -1, 16, 10, 15, 0},
	{ -1,  -1, 10,  0, 16, 0}, { -1,  -1,  2,  0, -1, 0},
};

static const struct tree_node treeYDC[] = {
	{  1,   6, -1,  0, 0, 0}, {  2,   3, -1,  0, 0, 0},
	{ -1,  -1,  2,  0, 0, 0}, {  4,   5, -1,  0, 0, 0},
	{ -1,  -1,  3,  1, 0, 0}, { -1,  -1,  3,  2, 0, 0},
	{  7,  10, -1,  0, 0, 0}, {  8,   9, -1,  0, 0, 0},
	{ -1,  -1,  3,  3, 0, 0}, { -1,  -1,  3,  4, 0, 0},
	{ 11,  12, -1,  0, 0, 0}, { -1,  -1,  3,  5, 0, 0},
	{ 13,  14, -1,  0, 0, 0}, { -1,  -1,  4,  6, 0, 0},
	{ 15,  16, -1,  0, 0, 0}, { -1,  -1,  5,  7, 0, 0},
	{ 17,  18, -1,  0, 0, 0}, { -1,  -1,  6,  8, 0, 0},
	{ 19,  20, -1,  0, 0, 0}, { -1,  -1,  7,  9, 0, 0},
	{ 21,  22, -1,  0, 0, 0}, { -1,  -1,  8, 10, 0, 0},
	{ 23,  -1, -1,  0, 0, 0}, { -1,  -1,  9, 11, 0, 0},
};

static const struct tree_node treeUVDC[] = {
	{  1,   4, -1,  0, 0, 0}, {  2,   3, -1,  0, 0, 0},
	{ -1,  -1,  2,  0, 0, 0}, { -1,  -1,  2,  1, 0, 0},
	{  5,   6, -1,  0, 0, 0}, { -1,  -1,  2,  2, 0, 0},
	{  7,   8, -1,  0, 0, 0}, { -1,  -1,  3,  3, 0, 0},
	{  9,  10, -1,  0, 0, 0}, { -1,  -1,  4,  4, 0, 0},
	{ 11,  12, -1,  0, 0, 0}, { -1,  -1,  5,  5, 0, 0},
	{ 13,  14, -1,  0, 0, 0}, { -1,  -1,  6,  6, 0, 0},
	{ 15,  16, -1,  0, 0, 0}, { -1,  -1,  7,  7, 0, 0},
	{ 17,  18, -1,  0, 0, 0}, { -1,  -1,  8,  8, 0, 0},
	{ 19,  20, -1,  0, 0, 0}, { -1,  -1,  9,  9, 0, 0},
	{ 21,  22, -1,  0, 0, 0}, { -1,  -1, 10, 10, 0, 0},
	{ 23,  -1, -1,  0, 0, 0}, { -1,  -1, 11, 11, 0, 0},
};

/******************************************************************************
 * Huffman Decoder
 ******************************************************************************/

/* Note: There is no penalty for passing the tree as an argument, since dummy
 * args are passed anyway (to maintain 16-byte stack alignment), and since the
 * address is loaded into a register either way. */

/* If no node is found, coeffbits and skip will not be modified */
/* Return: Depth of node found, or -1 if invalid input code */
static int
getNodeAC(unsigned int in, signed char *coeffbits, signed char *skip,
		const struct tree_node *tree)
{
	int node = 0;
	int i = 0;
	int depth;

	do {
		if ((in & 0x80000000) == 0)
			node = tree[node].left;
		else
			node = tree[node].right;

		if (node == -1)
			break;

		depth = tree[node].depth;

		/* Is it a leaf? If not, branch downward */
		if (depth != -1) {
			*coeffbits = tree[node].coeffbits;
			*skip = tree[node].skip;
			return depth;
		}

		in <<= 1;
		++i;
	} while (i <= 15);

	return -1;
}

/* If no node is found, coeffbits will not be modified */
/* Return: Depth of node found, or -1 if invalid input code */
static int
getNodeDC(unsigned int in, signed char *coeffbits, const struct tree_node *tree)
{
	int node = 0;
	int i = 0;
	int depth;

	do {
		if ((in & 0x80000000) == 0)
			node = tree[node].left;
		else
			node = tree[node].right;

		if (node == -1)
			break;

		depth = tree[node].depth;

		/* Is it a leaf? If not, branch downward */
		if (depth != -1) {
			*coeffbits = tree[node].coeffbits;
			return depth;
		}

		in <<= 1;
		++i;
	} while (i <= 15);

	return -1;
}

static inline unsigned int
getBytes(int *rawData, struct comp_info *cinfo)
{
	int bufLen = cinfo->rawLen;
	int bits = cinfo->bits;
	int bytes = cinfo->bytes;
	u8 *in = bytes + (u8 *) rawData;
	u8 b1, b2, b3, b4, b5;
	unsigned int packedIn;

	/* Pull 5 bytes out of raw data */
	if (bytes < bufLen - 4) {
		b1 = in[0];
		b2 = in[1];
		b3 = in[2];
		b4 = in[3];
		b5 = in[4];
	} else {
		if (bytes < bufLen - 3) {
			b1 = in[0];
			b2 = in[1];
			b3 = in[2];
			b4 = in[3];
		} else {
			if (bytes < bufLen - 2) {
				b1 = in[0];
				b2 = in[1];
				b3 = in[2];
			} else {
				if (bytes < bufLen - 1) {
					b1 = in[0];
					b2 = in[1];
				} else {
					if (bytes <= bufLen)
						b1 = in[0];
					else
						b1 = 0;
					b2 = 0;
				}
				b3 = 0;
			}
			b4 = 0;
		}
		b5 = 0;
	}

	/* Pack the bytes */
	packedIn  = b1 << 24;
	packedIn += b2 << 16;
	packedIn += b3 << 8;
	packedIn += b4;

	if (bits != 0) {
		packedIn = packedIn << bits;
		packedIn += b5 >> (8 - bits);
	}

	return packedIn;
}

static int
getACCoefficient(int *rawData, int *coeff, struct comp_info *cinfo,
		const struct tree_node *tree)
{
	int input, bits, bytes, tmp_c;
	signed char coeffbits = 0;
	signed char skip = 0;

	input = getBytes(rawData, cinfo);
	bits = getNodeAC(input, &coeffbits, &skip, tree);

	if (coeffbits) {
		input = input << (bits - 1);
		input &= 0x7fffffff;
		if (!(input & 0x40000000))
			input |= 0x80000000;

		tmp_c = input >> (31 - coeffbits);
		if (tmp_c < 0)
			tmp_c++;
		*coeff = tmp_c;

		bits += coeffbits;
	}

	bytes = (bits + cinfo->bits) >> 3;
	cinfo->bytes += bytes;
	cinfo->bits += bits - (bytes << 3);

	return skip;
}

static void
getDCCoefficient(int *rawData, int *coeff, struct comp_info *cinfo,
		const struct tree_node *tree)
{
	int input, bits, bytes, tmp_c;
	signed char coeffbits = 0;

	input = getBytes(rawData, cinfo);
	bits = getNodeDC(input, &coeffbits, tree);

	if (bits == -1) {
		bits = 1;	/* Try to re-sync at the next bit */
		*coeff = 0;	/* Indicates no change from last DC */
	} else {

		input = input << (bits - 1);
		input &= 0x7fffffff;
		if (!(input & 0x40000000))
			input |= 0x80000000;

		tmp_c = input >> (31 - coeffbits);
		if (tmp_c < 0)
			tmp_c++;
		*coeff = tmp_c;

		bits += coeffbits;
	}

	bytes = (bits + cinfo->bits) >> 3;
	cinfo->bytes += bytes;
	cinfo->bits += bits - (bytes << 3);
}

/* For AC coefficients, here is what the "skip" value means:
 *   -1: Either the 8x4 block has ended, or the decoding failed.
 *    0: Use the returned coeff. Don't skip anything.
 * 1-15: The next <skip> coeffs are zero. The returned coeff is used.
 *   16: The next 16 coeffs are zero. The returned coeff is ignored.
 *
 * You must ensure that the C[] array not be overrun, or stack corruption will
 * result.
 */
static void
huffmanDecoderY(int *C, int *pIn, struct comp_info *cinfo)
{
	int coeff = 0;
	int i = 1;
	int k, skip;

	getDCCoefficient(pIn, C, cinfo, treeYDC);

	i = 1;
	do {
		skip = getACCoefficient(pIn, &coeff, cinfo, treeYAC);

		if (skip == -1) {
			break;
		} else if (skip == 0) {
			C[i++] = coeff;
		} else if (skip == 16) {
			k = 16;
			if (i > 16)
				k = 32 - i;

			while (k--)
				C[i++] = 0;
		} else {
			k = skip;
			if (skip > 31 - i)
				k = 31 - i;

			while (k--)
				C[i++] = 0;

			C[i++] = coeff;
		}
	} while (i <= 31);

	if (skip == -1)
		while (i <= 31)
			C[i++] = 0;
	else
		getACCoefficient(pIn, &coeff, cinfo, treeYAC);
}

/* Same as huffmanDecoderY, except for the tables used */
static void
huffmanDecoderUV(int *C, int *pIn, struct comp_info *cinfo)
{
	int coeff = 0;
	int i = 1;
	int k, skip;

	getDCCoefficient(pIn, C, cinfo, treeUVDC);

	i = 1;
	do {
		skip = getACCoefficient(pIn, &coeff, cinfo, treeUVAC);

		if (skip == -1) {
			break;
		} else if (skip == 0) {
			C[i++] = coeff;
		} else if (skip == 16) {
			k = 16;
			if (i > 16)
				k = 32 - i;

			while (k--)
				C[i++] = 0;
		} else {
			k = skip;
			if (skip > 31 - i)
				k = 31 - i;

			while (k--)
				C[i++] = 0;

			C[i++] = coeff;
		}
	} while (i <= 31);

	if (skip == -1)
		while (i <= 31)
			C[i++] = 0;
	else
		getACCoefficient(pIn, &coeff, cinfo, treeUVAC);
}

/******************************************************************************
 * iDCT Functions
 ******************************************************************************/

#ifndef APPROXIMATE_MUL_BY_SHIFT

#define IDCT_MESSAGE "iDCT with multiply"

#define TIMES_16382(u)	((u) ? 16382 * (u) : 0)
#define TIMES_23168(u)	((u) ? 23168 * (u) : 0)
#define TIMES_30270(u)	((u) ? 30270 * (u) : 0)
#define TIMES_41986(u)	((u) ? 41986 * (u) : 0)
#define TIMES_35594(u)	((u) ? 35594 * (u) : 0)
#define TIMES_23783(u)	((u) ? 23783 * (u) : 0)
#define TIMES_8351(u)	((u) ? 8351  * (u) : 0)
#define TIMES_17391(u)	((u) ? 17391 * (u) : 0)
#define TIMES_14743(u)	((u) ? 14743 * (u) : 0)
#define TIMES_9851(u)	((u) ? 9851  * (u) : 0)
#define TIMES_3459(u)	((u) ? 3459  * (u) : 0)
#define TIMES_32134(u)	((u) ? 32134 * (u) : 0)
#define TIMES_27242(u)	((u) ? 27242 * (u) : 0)
#define TIMES_18202(u)	((u) ? 18202 * (u) : 0)
#define TIMES_6392(u)	((u) ? 6392  * (u) : 0)
#define TIMES_39550(u)	((u) ? 39550 * (u) : 0)
#define TIMES_6785(u)	((u) ? 6785  * (u) : 0)
#define TIMES_12538(u)	((u) ? 12538 * (u) : 0)

#else

#define IDCT_MESSAGE "iDCT with shift"

#define TIMES_16382(u) ((u) ? x = (u), (x << 14) - (x << 1) : 0)
#define TIMES_23168(u) ((u) ? x = (u), (x << 14) + (x << 12) + (x << 11) + (x << 9) : 0)
#define TIMES_30270(u) ((u) ? x = (u), (x << 15) - (x << 11) : 0)
#define TIMES_41986(u) ((u) ? x = (u), (x << 15) + (x << 13) + (x << 10) : 0)
#define TIMES_35594(u) ((u) ? x = (u), (x << 15) + (x << 11) + (x << 9) + (x << 8) : 0)
#define TIMES_23783(u) ((u) ? x = (u), (x << 14) + (x << 13) - (x << 9) - (x << 8) : 0)
#define TIMES_8351(u)  ((u) ? x = (u), (x << 13) : 0)
#define TIMES_17391(u) ((u) ? x = (u), (x << 14) + (x << 10) : 0)
#define TIMES_14743(u) ((u) ? x = (u), (x << 14) - (x << 10) - (x << 9) : 0)
#define TIMES_9851(u)  ((u) ? x = (u), (x << 13) + (x << 10) + (x << 9) : 0)
#define TIMES_3459(u)  ((u) ? x = (u), (x << 12) - (x << 9) : 0)
#define TIMES_32134(u) ((u) ? x = (u), (x << 15) - (x << 9) : 0)
#define TIMES_27242(u) ((u) ? x = (u), (x << 14) + (x << 13) + (x << 11) + (x << 9) : 0)
#define TIMES_18202(u) ((u) ? x = (u), (x << 14) + (x << 11) - (x << 8) : 0)
#define TIMES_6392(u)  ((u) ? x = (u), (x << 13) - (x << 11) + (x << 8) : 0)
#define TIMES_39550(u) ((u) ? x = (u), (x << 15) + (x << 12) + (x << 11) + (x << 9) : 0)
#define TIMES_6785(u)  ((u) ? x = (u), (x << 12) + (x << 11) + (x << 9) : 0)
#define TIMES_12538(u) ((u) ? x = (u), (x << 13) + (x << 12) + (x << 8) : 0)

/*
 * The variables C0, C4, C16 and C20 can also be removed from the algorithm
 * if APPROXIMATE_MUL_BY_SHIFTS is defined. They store correction values
 * and can be considered insignificant.
 */

#endif

static void DCT_8x4(int *coeff, u8 *out)
	/* pre: coeff == coefficients
	   post: coeff != coefficients
	 ** DO NOT ASSUME coeff TO BE THE SAME BEFORE AND AFTER CALLING THIS FUNCTION!
	 */
{
	register int base, val1, val2, val3;
	int tmp1, tmp2;
	int C4, C16, C20;
	int C2_18, C6_22, C1_17, C3_19, C5_21, C7_23;
	register int t;
#ifdef APPROXIMATE_MUL_BY_SHIFT
	register int x;
#endif

	C4 = coeff[4];
	C16 = coeff[16];
	C20 = coeff[20];

	coeff[0] = TIMES_23168(coeff[0]);
	coeff[4] = TIMES_23168(coeff[4]);
	coeff[16] = TIMES_23168(coeff[16]);
	coeff[20] = TIMES_23168(coeff[20]);

	C2_18 = coeff[2] + coeff[18];
	C6_22 = coeff[6] + coeff[22];
	C1_17 = coeff[1] + coeff[17];
	C3_19 = coeff[3] + coeff[19];
	C5_21 = coeff[5] + coeff[21];
	C7_23 = coeff[7] + coeff[23];

	// 0,7,25,32

	base = 0x1000000;
	base += coeff[0] + coeff[4] + coeff[16] + coeff[20];
	base += TIMES_30270(C2_18);
	base += TIMES_12538(C6_22);

	val1 = TIMES_41986(coeff[9]);
	val1 += TIMES_35594(coeff[11]);
	val1 += TIMES_23783(coeff[13]);
	val1 += TIMES_8351(coeff[15]);
	val1 += TIMES_17391(coeff[25]);
	val1 += TIMES_14743(coeff[27]);
	val1 += TIMES_9851(coeff[29]);
	val1 += TIMES_3459(coeff[31]);

	val2 = TIMES_32134(C1_17);
	val2 += TIMES_27242(C3_19);
	val2 += TIMES_18202(C5_21);
	val2 += TIMES_6392(C7_23);

	val3 = TIMES_39550(coeff[10]);
	val3 += TIMES_16382(coeff[14] + coeff[26]);
	val3 += TIMES_6785(coeff[30]);
	val3 += TIMES_30270(coeff[8] + coeff[12]);
	val3 += TIMES_12538(coeff[24] + coeff[28]);

	t = (base + val1 + val2 + val3) >> 17;
	out[0] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;
	t = (base - val1 - val2 + val3 - C4 - C20) >> 17;
	out[7] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;
	t = (base - val1 + val2 - val3 - C16 - C20) >> 17;
	out[24] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;
	t = (base + val1 - val2 - val3 - C4 - C16 - C20) >> 17;
	out[31] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;

	//1,6,25,30

	base = 0x1000000;
	base += coeff[0] - coeff[4] + coeff[16] - coeff[20];
	base += TIMES_12538(C2_18);
	base -= TIMES_30270(C6_22);

	val1 = TIMES_35594(coeff[9]);
	val1 -= TIMES_8351(coeff[11]);
	val1 -= TIMES_41986(coeff[13]);
	val1 -= TIMES_23783(coeff[15]);
	val1 -= TIMES_14743(coeff[25]);
	val1 -= TIMES_3459(coeff[27]);
	val1 -= TIMES_17391(coeff[29]);
	val1 -= TIMES_9851(coeff[31]);

	val2 = TIMES_27242(C1_17);
	val2 -= TIMES_6392(C3_19);
	val2 -= TIMES_32134(C5_21);
	val2 -= TIMES_18202(C7_23);

	val3 = TIMES_16382(coeff[10] - coeff[30]);
	val3 -= TIMES_39550(coeff[14]);
	val3 += TIMES_6785(coeff[26]);
	val3 += TIMES_12538(coeff[24] - coeff[28]);
	val3 += TIMES_30270(coeff[8] - coeff[12]);

	t = (base + val1 + val2 + val3 + C4 + C20) >> 17;
	out[1] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;
	t = (base - val1 - val2 + val3) >> 17;
	out[6] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;
	t = (base - val1 + val2 - val3 + C4 - C16 + C20) >> 17;
	out[25] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;
	t = (base + val1 - val2 - val3 + C20) >> 17;
	out[30] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;

	//2,5,26,29

	base = 0x1000000;
	base += coeff[0] - coeff[4] + coeff[16] - coeff[20];
	base -= TIMES_12538(C2_18);
	base += TIMES_30270(C6_22);

	val1 = TIMES_23783(coeff[9]);
	val1 -= TIMES_41986(coeff[11]);
	val1 += TIMES_8351(coeff[13]);
	val1 += TIMES_35594(coeff[15]);
	val1 += TIMES_9851(coeff[25]);
	val1 -= TIMES_17391(coeff[27]);
	val1 += TIMES_3459(coeff[29]);
	val1 += TIMES_14743(coeff[31]);

	val2 = TIMES_18202(C1_17);
	val2 -= TIMES_32134(C3_19);
	val2 += TIMES_6392(C5_21);
	val2 += TIMES_27242(C7_23);

	val3 = -TIMES_16382(coeff[10] - coeff[30]);
	val3 += TIMES_39550(coeff[14]);
	val3 -= TIMES_6785(coeff[26]);
	val3 += TIMES_12538(coeff[24] - coeff[28]);
	val3 += TIMES_30270(coeff[8] - coeff[12]);

	t = (base + val1 + val2 + val3) >> 17;
	out[2] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;
	t = (base - val1 - val2 + val3) >> 17;
	out[5] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;
	t = (base - val1 + val2 - val3 - C16) >> 17;
	out[26] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;
	t = (base + val1 - val2 - val3 + C4 - C16 + C20) >> 17;
	out[29] = t & 0xFFFFFF00 ? t < 0 ? 0 : 255 : (u8)t;

	//3,4,27,28

	base = 0x1000000;
	base += coeff[0] + coeff[4] + coeff[16] + coeff[20];
	base -= TIMES_30270(C2_18);
	base -= TIMES_12538(C6_22);

	val1 = TIMES_8351(coeff[9]);
	val1 -= TIMES_23783(coeff[11]);
	val1 += TIMES_35594(coeff[13]);
	val1 += TIMES_3459(coeff[25]);
	val1 -= TIMES_9851(coeff[27]);
	val1 += TIMES_14743(coeff[29]);

	val2 = TIMES_6392(C1_17);
	val2 -= TIMES_18202(C3_19);
	val2 += TIMES_27242(C5_21);

	val3 = -TIMES_39550(coeff[10]);
	val3 += TIMES_16382(coeff[14] + coeff[26]);
	val3 -= TIMES_6785(coeff[30]);
	val3 += TIMES_30270(coeff[8] + coeff[12]);
	val3 += TIMES_12538(coeff[24] + coeff[28]);

	tmp1 = TIMES_32134(C7_23);
	tmp2 = TIMES_41986(coeff[15]) + TIMES_17391(coeff[31]);

	t = (base + val1 + val2 + val3 - tmp1 - tmp2 - C4 - C20) >> 17;
	out[3] = CLIP(t);
	t = (base - val1 - val2 + val3) >> 17;
	out[4] = CLIP(t);
	t = (base - val1 + val2 - val3 - tmp1 + tmp2) >> 17;
	out[27] = CLIP(t);
	t = (base + val1 - val2 - val3 - C16 - C20) >> 17;
	out[28] = CLIP(t);

	// Second half
	C2_18 = coeff[2] - coeff[18];
	C6_22 = coeff[6] - coeff[22];
	C1_17 = coeff[1] - coeff[17];
	C3_19 = coeff[3] - coeff[19];
	C5_21 = coeff[5] - coeff[21];
	C7_23 = coeff[7] - coeff[23];

	// 8,15,16,23

	base = 0x1000000;
	base += coeff[0] + coeff[4] - coeff[16] - coeff[20];
	base += TIMES_30270(C2_18);
	base += TIMES_12538(C6_22);

	val1 = TIMES_17391(coeff[9]);
	val1 += TIMES_14743(coeff[11]);
	val1 += TIMES_9851(coeff[13]);
	val1 += TIMES_3459(coeff[15]);
	val1 -= TIMES_41986(coeff[25]);
	val1 -= TIMES_35594(coeff[27]);
	val1 -= TIMES_23783(coeff[29]);
	val1 -= TIMES_8351(coeff[31]);

	val2 = TIMES_32134(C1_17);
	val2 += TIMES_27242(C3_19);
	val2 += TIMES_18202(C5_21);
	val2 += TIMES_6392(C7_23);

	val3 = TIMES_16382(coeff[10] - coeff[30]);
	val3 += TIMES_6785(coeff[14]);
	val3 -= TIMES_39550(coeff[26]);
	val3 -= TIMES_30270(coeff[24] + coeff[28]);
	val3 += TIMES_12538(coeff[8] + coeff[12]);

	t = (base + val1 + val2 + val3) >> 17;
	out[8] = CLIP(t);
	t = (base - val1 - val2 + val3 - C4 + C16 + C20) >> 17;
	out[15] = CLIP(t);
	t = (base - val1 + val2 - val3) >> 17;
	out[16] = CLIP(t);
	t = (base + val1 - val2 - val3 - C4 + C20) >> 17;
	out[23] = CLIP(t);

	//9,14,17,22

	base = 0x1000000;
	base += coeff[0] - coeff[4] - coeff[16] + coeff[20];
	base += TIMES_12538(C2_18);
	base -= TIMES_30270(C6_22);

	val1 = TIMES_14743(coeff[9]);
	val1 -= TIMES_3459(coeff[11]);
	val1 -= TIMES_17391(coeff[13]);
	val1 -= TIMES_9851(coeff[15]);
	val1 -= TIMES_35594(coeff[25]);
	val1 += TIMES_8351(coeff[27]);
	val1 += TIMES_41986(coeff[29]);
	val1 += TIMES_23783(coeff[31]);

	val2 = TIMES_27242(C1_17);
	val2 -= TIMES_6392(C3_19);
	val2 -= TIMES_32134(C5_21);
	val2 -= TIMES_18202(C7_23);

	val3 = TIMES_6785(coeff[10]);
	val3 -= TIMES_16382(coeff[14] + coeff[26]);
	val3 += TIMES_39550(coeff[30]);
	val3 += TIMES_12538(coeff[8] - coeff[12]);
	val3 -= TIMES_30270(coeff[24] - coeff[28]);

	t = (base + val1 + val2 + val3 + C4 + C16 - C20) >> 17;
	out[9] = CLIP(t);
	t = (base - val1 - val2 + val3 + C16) >> 17;
	out[14] = CLIP(t);
	t = (base - val1 + val2 - val3 + C4) >> 17;
	out[17] = CLIP(t);
	t = (base + val1 - val2 - val3) >> 17;
	out[22] = CLIP(t);

	//10,13,18,21

	base = 0x1000000;
	base += coeff[0] - coeff[4] - coeff[16] + coeff[20];
	base -= TIMES_12538(C2_18);
	base += TIMES_30270(C6_22);

	val1 = TIMES_9851(coeff[9]);
	val1 -= TIMES_17391(coeff[11]);
	val1 += TIMES_3459(coeff[13]);
	val1 += TIMES_14743(coeff[15]);
	val1 -= TIMES_23783(coeff[25]);
	val1 += TIMES_41986(coeff[27]);
	val1 -= TIMES_8351(coeff[29]);
	val1 -= TIMES_35594(coeff[31]);

	val2 = TIMES_18202(C1_17);
	val2 -= TIMES_32134(C3_19);
	val2 += TIMES_6392(C5_21);
	val2 += TIMES_27242(C7_23);

	val3 = -TIMES_6785(coeff[10]);
	val3 += TIMES_16382(coeff[14] + coeff[26]);
	val3 -= TIMES_39550(coeff[30]);
	val3 += TIMES_12538(coeff[8] - coeff[12]);
	val3 -= TIMES_30270(coeff[24] - coeff[28]);

	t = (base + val1 + val2 + val3) >> 17;
	out[10] = CLIP(t);
	t = (base - val1 - val2 + val3 + C4 + C16 - C20) >> 17;
	out[13] = CLIP(t);
	t = (base - val1 + val2 - val3) >> 17;
	out[18] = CLIP(t);
	t = (base + val1 - val2 - val3 + C4) >> 17;
	out[21] = CLIP(t);

	// 11,12,19,20

	base = 0x1000000;
	base += coeff[0] + coeff[4] - coeff[16] - coeff[20];
	base -= TIMES_30270(C2_18);
	base -= TIMES_12538(C6_22);

	val1 = TIMES_3459(coeff[9]);
	val1 -= TIMES_9851(coeff[11]);
	val1 += TIMES_14743(coeff[13]);
	val1 -= TIMES_8351(coeff[25]);
	val1 += TIMES_23783(coeff[27]);
	val1 -= TIMES_35594(coeff[29]);

	val2 = TIMES_6392(C1_17);
	val2 -= TIMES_18202(C3_19);
	val2 += TIMES_27242(C5_21);

	val3 = -TIMES_16382(coeff[10] - coeff[30]);
	val3 -= TIMES_6785(coeff[14]);
	val3 += TIMES_39550(coeff[26]);
	val3 -= TIMES_30270(coeff[24] + coeff[28]);
	val3 += TIMES_12538(coeff[8] + coeff[12]);

	tmp1 = TIMES_32134(C7_23);
	tmp2 = -TIMES_17391(coeff[15]) + TIMES_41986(coeff[31]);

	t = (base + val1 + val2 + val3 - tmp1 + tmp2 + C16 + C20) >> 17;
	out[11] = CLIP(t);
	t = (base - val1 - val2 + val3 + C16 + C20) >> 17;
	out[12] = CLIP(t);
	t = (base - val1 + val2 - val3 - tmp1 - tmp2 - C4 + C20) >> 17;
	out[19] = CLIP(t);
	t = (base + val1 - val2 - val3) >> 17;
	out[20] = CLIP(t);
}

#undef TIMES_16382
#undef TIMES_23168
#undef TIMES_30270
#undef TIMES_41986
#undef TIMES_35594
#undef TIMES_23783
#undef TIMES_8351
#undef TIMES_17391
#undef TIMES_14743
#undef TIMES_9851
#undef TIMES_3459
#undef TIMES_32134
#undef TIMES_27242
#undef TIMES_18202
#undef TIMES_6392
#undef TIMES_39550
#undef TIMES_6785
#undef TIMES_12538

/******************************************************************************
 * Main Decoder Functions
 ******************************************************************************/

/* This function handles the decompression of a single 8x4 block. It is
 * independent of the palette (YUV422, YUV420, YUV400, GBR422...). cinfo->bytes
 * determines the positin in the input buffer.
 */
static int
decompress8x4(u8	*pOut,
		u8	*pIn,
		int		*lastDC,
		int		uvFlag,
		struct comp_info	*cinfo)
{
	int i, x, y, dc;
	int coeffs[32];
	int deZigZag[32];
	int *dest;
	int *src;
	u8 *qt = cinfo->qt;

	if (!uvFlag) {
		huffmanDecoderY(coeffs, (int *)pIn, cinfo);

		/* iDPCM and dequantize first coefficient */
		dc = (*lastDC) + coeffs[0];
		coeffs[0] = dc * (qt[0] + 1);
		*lastDC = dc;

		/* ...and the second coefficient */
		coeffs[1] = ((qt[1] + 1) * coeffs[1]) >> 1;

		/* Dequantize, starting at 3rd element */
		for (i = 2; i < 32; i++)
			coeffs[i] = (qt[i] + 1) * coeffs[i];
	} else {
		huffmanDecoderUV(coeffs, (int *)pIn, cinfo);

		/* iDPCM */
		dc = (*lastDC) + coeffs[0];
		coeffs[0] = dc;
		*lastDC = dc;

		/* Dequantize */
		for (i = 0; i < 32; i++)
			coeffs[i] = (qt[32 + i] + 1) * coeffs[i];
	}

	/* Dezigzag */
	for (i = 0; i < 32; i++)
		deZigZag[i] = coeffs[ZigZag518[i]];

	/* Transpose the dezigzagged coefficient matrix */
	src = deZigZag;
	dest = coeffs;
	for (y = 0; y <= 3; ++y) {
		for (x = 0; x <= 7; ++x)
			dest[x] = src[x * 4];
		src += 1;
		dest += 8;
	}

	/* Do the inverse DCT transform */
	DCT_8x4(coeffs, pOut);

	return 0;	/* Always returns 0 */
}

static inline void
copyBlock(u8 *src, u8 *dest, int destInc)
{
	int i;
	unsigned int *pSrc, *pDest;

	for (i = 0; i <= 3; i++) {
		pSrc = (unsigned int *) src;
		pDest = (unsigned int *) dest;
		pDest[0] = pSrc[0];
		pDest[1] = pSrc[1];
		src += 8;
		dest += destInc;
	}
}

#if 0
static inline int
decompress400NoMMXOV518(u8	 *pIn,
		u8	 *pOut,
		u8	 *pTmp,
		const int	 w,
		const int	 h,
		const int	 numpix,
		struct comp_info *cinfo)
{
	int iOutY, x, y;
	int lastYDC = 0;

	/* Start Y loop */
	y = 0;
	do {
		iOutY = w * y;
		x = 0;
		do {
			decompress8x4(pTmp, pIn, &lastYDC, 0, cinfo);
			copyBlock(pTmp, pOut + iOutY, w);
			iOutY += 8;
			x += 8;
		} while (x < w);
		y += 4;
	} while (y < h);

	/* Did we decode too much? */
	if (cinfo->bytes > cinfo->rawLen + 897)
		return 1;

	/* Did we decode enough? */
	if (cinfo->bytes >= cinfo->rawLen - 897)
		return 0;
	else
		return 1;
}
#endif

static inline int decompress420NoMMXOV518(u8	 *pIn, u8 *pOut, u8 *pTmp, const int w,
		const int	 h,
		const int	 numpix,
		struct comp_info *cinfo,
		int yvu) {
	u8 *pOutU, *pOutV;
	int iOutY, iOutU, iOutV, x, y;
	int lastYDC = 0;
	int lastUDC = 0;
	int lastVDC = 0;

	if (yvu) {
		pOutV = pOut + numpix;
		pOutU = pOutV + numpix / 4;
	} else {
		pOutU = pOut + numpix;
		pOutV = pOutU + numpix / 4;
	}

	/* Start Y loop */
	y = 0;
	do {
		iOutY = w * y;
		iOutV = iOutU = iOutY / 4;

		x = 0;
		do {
			decompress8x4(pTmp, pIn, &lastYDC, 0, cinfo);
			copyBlock(pTmp, pOut + iOutY, w);
			iOutY += 8;
			x += 8;
		} while (x < w);



		iOutY = w * (y + 4);
		x = 0;
		do {
			decompress8x4(pTmp, pIn, &lastUDC, 1, cinfo);
			copyBlock(pTmp, pOutU + iOutU, w/2);
			iOutU += 8;

			decompress8x4(pTmp, pIn, &lastVDC, 1, cinfo);
			copyBlock(pTmp, pOutV + iOutV, w/2);
			iOutV += 8;

			decompress8x4(pTmp, pIn, &lastYDC, 0, cinfo);
			copyBlock(pTmp, pOut + iOutY, w);
			iOutY += 8;

			decompress8x4(pTmp, pIn, &lastYDC, 0, cinfo);
			copyBlock(pTmp, pOut + iOutY, w);
			iOutY += 8;

			x += 16;
		} while (x < w);

		y += 8;
	} while (y < h);

	/* Did we decode too much? */
	if (cinfo->bytes > cinfo->rawLen + 897)
		return 1;

	/* Did we decode enough? */
	if (cinfo->bytes >= cinfo->rawLen - (897 + 64))
		return 0;
	else
		return 1;
}

/* Get quantization tables from input
 * Returns: <0 if error, or >=0 otherwise */
static int get_qt_dynamic(u8 *pIn, struct comp_info *cinfo) {
	int rawLen = cinfo->rawLen;

	/* Make sure input is actually big enough to hold trailer */
	if (rawLen < 72)
		return -1;

	cinfo->qt = pIn + rawLen - 64;

	return 0;
}

/* Remove all 0 blocks from input */
static void remove0blocks(u8 *pIn, unsigned int *inSize) {
	long long *in = (long long *)pIn;
	long long *out = (long long *)pIn;

	for (unsigned int i = 0; i < *inSize; i += 8, in++)
		/* Skip 8 byte blocks of all 0 */
		if (*in)
			*out++ = *in;

	*inSize -= (in - out) * 8;
}

#if 0 /* not used */
/* Input format is raw isoc. data (with intact SOF header, packet numbers
 * stripped, and all-zero blocks removed).
 * Output format is planar YUV400
 * Returns uncompressed data length if success, or zero if error
 */
static int Decompress400(u8 *pIn, u8 *pOut, int w, int h, unsigned int inSize) {
	struct comp_info cinfo;
	int numpix = w * h;
	u8 pTmp[32];

	remove0blocks(pIn, &inSize);

	cinfo.bytes = 0;
	cinfo.bits = 0;
	cinfo.rawLen = inSize;

	if (get_qt_dynamic(pIn, &cinfo) < 0)
		return 0;

	/* Decompress, skipping the 8-byte SOF header */
	if (decompress400NoMMXOV518(pIn + 8, pOut, pTmp, w, h, numpix, &cinfo))
		/*		return 0; */
		; /* Don't return error yet */

	return numpix;
}
#endif

/* Input format is raw isoc. data (with intact SOF header, packet numbers
 * stripped, and all-zero blocks removed).
 * Output format is planar YUV420
 * Returns uncompressed data length if success, or zero if error
 */
static int v4lconvert_ov518_to_yuv420(u8 *src, u8 *dst, int w, int h, int yvu, int inSize) {
	struct comp_info cinfo;
	int numpix = w * h;
	u8 pTmp[32];

	remove0blocks(src, &inSize);

	cinfo.bytes = 0;
	cinfo.bits = 0;
	cinfo.rawLen = inSize;

	if (get_qt_dynamic(src, &cinfo) < 0)
		return -1;

	/* Decompress, skipping the 8-byte SOF header */
	if (decompress420NoMMXOV518(src + 8, dst, pTmp, w, h, numpix, &cinfo, yvu))
		return -1;

	return 0;
}

int main(int argc, char *argv[]) {
	u32 width, height, yvu, src_size;
	u8 src_buf[200000];
	u8 dest_buf[500000];
	
	if (argc < 1) {
		fprintf(stderr, "1 argument required");
		return 2;
	}

	while (1) {
		if (v4lconvert_helper_read(STDIN_FILENO, &width, sizeof(int), argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */

		if (v4lconvert_helper_read(STDIN_FILENO, &height, sizeof(int), argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */

		if (v4lconvert_helper_read(STDIN_FILENO, &yvu, sizeof(int), argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */

		if (v4lconvert_helper_read(STDIN_FILENO, &src_size, sizeof(int), argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */

		if (src_size > sizeof(src_buf)) {
			fprintf(stderr, "%s: error: src_buf too small, need: %d\n",
					argv[0], src_size);
			return 2;
		}

		if (v4lconvert_helper_read(STDIN_FILENO, src_buf, src_size, argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */


		unsigned int dest_size = width * height * 3 / 2;
		if (dest_size > sizeof(dest_buf)) {
			fprintf(stderr, "%s: error: dest_buf too small, need: %d\n", argv[0], dest_size);
			dest_size = (unsigned) -1;
		} else if (v4lconvert_ov518_to_yuv420(src_buf, dest_buf, width, height, yvu, src_size))
			dest_size = (unsigned) -1;

		if (v4lconvert_helper_write(STDOUT_FILENO, &dest_size, sizeof(int), argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */

		if (dest_size == (unsigned) -1)
			continue;

		if (v4lconvert_helper_write(STDOUT_FILENO, dest_buf, dest_size, argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */
	}
}
#undef CLIP
