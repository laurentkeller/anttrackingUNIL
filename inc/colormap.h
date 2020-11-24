/*
 *  colormap.h
 *  
 *
 *  Created by Danielle Mersch.
 *  Copyright UNIL. All rights reserved.
 *
 */

#ifndef __COLORMAP_H
#define __COLORMAP_H

// The 'jet' colormap of MATLAB exported as R, G and B lookup-tables

const uint8_t colors_r[256] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8,12,16,20,24,28,32,36,40,44,48,52,
  56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,131,135,139,
  143,147,151,155,159,163,167,171,175,179,183,187,191,195,199,203,207,211,215,
  219,223,227,231,235,239,243,247,251,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,251,247,
  243,239,235,231,227,223,219,215,211,207,203,199,195,191,187,183,179,175,171,
  167,163,159,155,151,147,143,139,135,131,128
};

const uint8_t colors_g[256] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8,12,16,20,
  24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,
  120,124,128,131,135,139,143,147,151,155,159,163,167,171,175,179,183,187,191,
  195,199,203,207,211,215,219,223,227,231,235,239,243,247,251,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,251,247,243,239,235,231,227,223,219,215,211,207,203,199,195,
  191,187,183,179,175,171,167,163,159,155,151,147,143,139,135,131,128,124,120,
  116,112,108,104,100,96,92,88,84,80,76,72,68,64,60,56,52,48,44,40,36,32,28,24,
  20,16,12,8,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const uint8_t colors_b[256] = {
  131,135,139,143,147,151,155,159,163,167,171,175,179,183,187,191,195,199,203,
  207,211,215,219,223,227,231,235,239,243,247,251,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,251,247,243,239,235,231,227,223,219,215,211,207,203,199,195,191,187,183,
  179,175,171,167,163,159,155,151,147,143,139,135,131,128,124,120,116,112,108,
  104,100,96,92,88,84,80,76,72,68,64,60,56,52,48,44,40,36,32,28,24,20,16,12,8,
  4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

#endif