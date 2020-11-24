/*
 *  trackcvt.h
 *  
 *
 *  Created by Alessandro Crespi.
 *  Copyright EPFL. All rights reserved.
 *
 */

#ifndef __TRACKCVT_H
#define __TRACKCVT_H

#include <stdint.h>

const int IMAGE_WIDTH = 3048;
const int IMAGE_HEIGHT = 4560;

const int tag_count = 226;  //< 225 tags listed plus tag 520 (reference)

/// List of known tags, the binary format is based on this list
const int tag_list[tag_count] = {
   4,  57, 260,   5,  52,  59,  65, 244, 397, 540, 929,   2,   6,   7,   9,  14,
  15,  19,  24,  25,  26,  30,  38,  60,  64,  72,  73,  74,  76,  99, 102, 148,
 158, 161, 191, 192, 196, 202, 209, 227, 237, 245, 252, 268, 278, 308, 311, 329,
 518, 534, 604, 620, 646, 673, 740, 838, 954,   0,   1,  12,  13,  17,  20,  23,
  27,  28,  29,  32,  35,  40,  42,  43,  44,  48,  49,  50,  51,  53,  54,  55,
  62,  63,  66,  68,  70,  71,  80,  81,  82,  85,  91,  96,  97,  98, 100, 105,
 109, 111, 112, 113, 114, 115, 117, 118, 120, 124, 126, 127, 131, 135, 137, 139,
 140, 142, 144, 147, 149, 153, 156, 159, 169, 170, 172, 173, 174, 175, 176, 178,
 186, 190, 194, 195, 198, 206, 207, 210, 212, 215, 217, 218, 219, 220, 221, 226,
 229, 232, 238, 242, 243, 250, 251, 255, 256, 257, 261, 262, 263, 264, 265, 289,
 294, 296, 298, 299, 301, 305, 313, 318, 320, 324, 326, 331, 332, 334, 342, 347,
 353, 356, 376, 380, 387, 390, 391, 394, 395, 398, 400, 433, 437, 458, 462, 475,
 476, 482, 492, 501, 502, 507, 509, 527, 530, 538, 549, 552, 556, 560, 564, 575,
 576, 580, 592, 593, 599, 600, 607, 610, 614, 621, 632, 635, 638, 647, 650, 663,
 665, 520
};

const int box_count = 22; ///< 22 experimental boxes
/// list of known boxes
const int box_list[box_count] ={11, 12, 21, 22, 31, 32, 41, 42, 51, 52, 61, 62, 71, 72, 81, 82, 91, 92, 101, 102, 111, 112};

// Spatial location of a tag
struct tag_pos {
  int16_t x;		///< x-coordinate of position
  int16_t y;		///< y-coordinate of position
  int16_t a;    ///< angle, as "fixed point" décimal (= real angle * 100)
  uint8_t id;   ///< box ID
  uint8_t padding; ///< empty : only used to fill a multiple of 8, otherwise we have portability issues due to different alignments on different computer architectures 
};

// Framerec structure
struct framerec {
  double time;          ///< unix time of frame acquisition with 100 millisec precisions
  uint32_t frame;       ///< frame number
  tag_pos tags[tag_count];  ///< table with information of tags
  uint32_t padding;		///< filling to generate a multiple of 8 and avoid alignment issues
};

// Information for each tag (statistics and rotation)
struct tag_info {
  unsigned long count;
  signed short rot;
};

#endif
