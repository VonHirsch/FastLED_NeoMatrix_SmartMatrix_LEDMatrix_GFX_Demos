/*
 * Aurora: https://github.com/pixelmatix/aurora
 * Copyright (c) 2014 Jason Coon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PatternRadar_H

#include "matrix.h"

class PatternRadar : public AuroraDrawable {
  private:
    byte theta = 0;
    byte hueoffset = 0;
    int max_offset;
    float x_aspect_ratio;
    float y_aspect_ratio;


  public:
    PatternRadar() {
      name = (char *)"Radar";
      // handle aspect ratio
      if (MATRIX_CENTER_X < MATRIX_CENTER_Y) {
          max_offset = MATRIX_CENTER_X;
          x_aspect_ratio = 1.0;
          y_aspect_ratio = (float)MATRIX_HEIGHT / (float)MATRIX_WIDTH;
      } else if (MATRIX_CENTER_X > MATRIX_CENTER_Y) {
          max_offset = MATRIX_CENTER_Y;
          x_aspect_ratio = (float)MATRIX_WIDTH / (float)MATRIX_HEIGHT;
          y_aspect_ratio = 1.0;
      } else {
          max_offset = MATRIX_CENTER_X;
          x_aspect_ratio = 1.0;
          y_aspect_ratio = 1.0;
      }
    }

    unsigned int drawFrame() {
      if (MATRIX_WIDTH < 25) {
          effects.DimAll(252);
      } else {
          effects.DimAll(245);
      }

      for (int offset = 0; offset < max_offset; offset++) {
        //byte hue = 255 - (offset * 16 + hueoffset);
	    byte hue = 255 - (offset * (256 / max_offset) + hueoffset);
        CRGB color = effects.ColorFromCurrentPalette(hue);
        uint8_t x_offset = offset * x_aspect_ratio;
        uint8_t y_offset = offset * y_aspect_ratio;
        uint8_t x = mapcos8(theta, x_offset, (MATRIX_WIDTH - 1) - x_offset);
        uint8_t y = mapsin8(theta, y_offset, (MATRIX_HEIGHT - 1) - y_offset);
        uint16_t xy = XY(x, y);
        effects.leds[xy] = color;

        EVERY_N_MILLIS(25) {
          theta += 2;
          hueoffset += 1;
        }
      }

      return 0;
    }
};

#endif
