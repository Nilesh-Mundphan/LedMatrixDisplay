/*
 * wifi.h
 *
 *  Created on: Dec 30, 2014
 *      Author: Minh
 */

#ifndef USER_LED_H_
#define USER_LED_H_
#include "os_type.h"

#define HIGH 1
#define LOW 0

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
 #define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
 #define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
//#define ssd1306_swap(a, b) { int16_t t = a; a = b; b = t; }
#define GFX_SCREEN_WIDTH 96
#define GFX_SCREEN_HEIGHT 16
#define GFX_CHAR_SPACING 2

#define BLACK 0
#define WHITE 1
#define INVERSE 2
#define boolean uint8_t
typedef struct { // Data stored PER GLYPH
        uint16_t bitmapOffset;     // Pointer into GFXfont->bitmap
        uint8_t  width, height;    // Bitmap dimensions in pixels
        uint8_t  xAdvance;         // Distance to advance cursor (x axis)
        int8_t   xOffset, yOffset; // Dist from cursor pos to UL corner
} GFXglyph;

typedef struct { // Data stored for FONT AS A WHOLE:
        uint8_t  *bitmap;      // Glyph bitmaps, concatenated
        GFXglyph *glyph;       // Glyph array
        uint8_t   first, last; // ASCII extents
        uint8_t   yAdvance;    // Newline distance (y axis)
} GFXfont;


#define DISPLAY_WIDTH		96
#define DISPLAY_HEIGHT		16
#define GFX_CHAR_SPACING	2
#define BUFF_SIZE 		192

void init();
void clear(void);
void HC595Pulse(void);
		  void HC595Latch(void);
		  void HC595Write(uint8_t data);

			
		  void EMGfxInit(void);
		  void EMGfxSelCh(uint8_t ch);
		  void EMGfxDispOff(void);
		  void EMGfxDispOn(void);
		  void EMGfxClear(void);
		  void writePixel(int16_t x, int16_t y, uint16_t color);
		  void brightness(int16_t br);		

		  void update_disp(void);

		  void setCursor(int16_t x, int16_t y);
		  size_t write(uint8_t c);
		  int drawChar(int16_t x, int16_t y, unsigned char c,uint16_t color, uint16_t bg, uint8_t size);
		  void setFont(const GFXfont *f);
		  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color);
		  void writeFastVLine(int16_t x, int16_t y,int16_t h, uint16_t color);
		  void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint16_t color);
		  void disp_init();
		  void DisplayClear();
		  void print_str(char* str);
		  void print_line(char *str);

extern int16_t _width, _height, cursor_x, cursor_y;	

#endif /* USER_WIFI_H_ */
