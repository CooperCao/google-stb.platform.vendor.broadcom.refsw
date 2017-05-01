/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_PIXEL_H__
#define __BSG_PIXEL_H__

#include "bsg_glapi.h"

class Pixel
{
public:
   Pixel(float r, float g, float b, float a)
   {
      m_r = r; m_g = g; m_b = b; m_a = a;
   };

   bool operator== (const Pixel &p) const;
   bool operator!= (const Pixel &p) const;

   static Pixel ReadPixel(int x, int y);

private:
   // Helper structures and functions

   typedef struct
   {
      union
      {
         GLfloat f;
         GLuint ui;
      };
   } TYPE_CONVERT_T;

   // Convert float into GLunint
   static unsigned int qpu_float_pack_uint8(unsigned int fl);
   // Convert unsigned int into float
   static unsigned int qpu_float_unpack_uint_n(unsigned int data, unsigned int n);

   float m_r;
   float m_g;
   float m_b;
   float m_a;
};

#endif // __BSG_PIXEL_H__
