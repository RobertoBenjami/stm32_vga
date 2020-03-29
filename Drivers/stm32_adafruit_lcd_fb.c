/*
 * Modify: Roberto Benjami
 * date: 2020.02
 *
 * - Modify to framebuffer mode
 *
 * - BSP_LCD_DrawCircle : delete the BSP_LCD_SetFont(&LCD_DEFAULT_FONT); (interesting bug)
 * - BSP_LCD_Init : DrawProp.pFont = &Font24 change to DrawProp.pFont = &LCD_DEFAULT_FONT
 * - FillTriangle -> BSP_LCD_FillTriangle, change to public, changed to fast algorythm
 * - Add : BSP_LCD_ReadID
 * - Add : BSP_LCD_ReadPixel
 * - Add : BSP_LCD_DrawRGB16Image
 * - Add : BSP_LCD_ReadRGB16Image
 * - Add : BSP_LCD_FillTriangle (faster algorithm)
 * - Modify : BSP_LCD_Init (default font from header file, default colors from header file, otptional clear from header file)
 * */

/**
  ******************************************************************************
  * @file    stm32_adafruit_lcd.c
  * @author  MCD Application Team
  * @brief   This file includes the driver for Liquid Crystal Display (LCD) module
  *          mounted on the Adafruit 1.8" TFT LCD shield (reference ID 802), 
  *          that is used with the STM32 Nucleo board through SPI interface.     
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Dependencies
- fonts.h
- font24.c
- font20.c
- font16.c
- font12.c
- font8.c"
EndDependencies */
    
/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "main.h"
#include "lcd.h"
#include "bmp.h"
#include "stm32_adafruit_lcd_fb.h"
#include "Fonts/fonts.h"

/* @defgroup STM32_ADAFRUIT_LCD_Private_Defines */
#define POLY_X(Z)             ((int32_t)((Points + (Z))->X))
#define POLY_Y(Z)             ((int32_t)((Points + (Z))->Y))

/* @defgroup STM32_ADAFRUIT_LCD_Private_Macros */
#define ABS(X) ((X) > 0 ? (X) : -(X))
#define SWAP16(a, b) {uint16_t t = a; a = b; b = t;}

/* @defgroup STM32_ADAFRUIT_LCD_Private_Variables */ 
LCD_DrawPropTypeDef DrawProp;

/* @defgroup STM32_ADAFRUIT_LCD_Private_FunctionPrototypes */ 
static void DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *c);
// static void SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);

enum
{
  RIGHT_THEN_DOWN = 0,
  RIGHT_THEN_UP
} BitmapDrawDir = RIGHT_THEN_DOWN;

#if   LCD_COLORBITDEPTH == 8
const uint32_t DefaultPalette[]=
{ 0x000000, 0x000055, 0x0000AA, 0x0000FF, 0x002400, 0x002455, 0x0024AA, 0x0024FF,
  0x004800, 0x004855, 0x0048AA, 0x0048FF, 0x006D00, 0x006D55, 0x006DAA, 0x006DFF,
  0x009100, 0x009155, 0x0091AA, 0x0091FF, 0x00B600, 0x00B655, 0x00B6AA, 0x00B6FF,
  0x00DA00, 0x00DA55, 0x00DAAA, 0x00DAFF, 0x00FF00, 0x00FF55, 0x00FFAA, 0x00FFFF,
  0x240000, 0x240055, 0x2400AA, 0x2400FF, 0x242400, 0x242455, 0x2424AA, 0x2424FF,
  0x244800, 0x244855, 0x2448AA, 0x2448FF, 0x246D00, 0x246D55, 0x246DAA, 0x246DFF,
  0x249100, 0x249155, 0x2491AA, 0x2491FF, 0x24B600, 0x24B655, 0x24B6AA, 0x24B6FF,
  0x24DA00, 0x24DA55, 0x24DAAA, 0x24DAFF, 0x24FF00, 0x24FF55, 0x24FFAA, 0x24FFFF,
  0x480000, 0x480055, 0x4800AA, 0x4800FF, 0x482400, 0x482455, 0x4824AA, 0x4824FF,
  0x484800, 0x484855, 0x4848AA, 0x4848FF, 0x486D00, 0x486D55, 0x486DAA, 0x486DFF,
  0x489100, 0x489155, 0x4891AA, 0x4891FF, 0x48B600, 0x48B655, 0x48B6AA, 0x48B6FF,
  0x48DA00, 0x48DA55, 0x48DAAA, 0x48DAFF, 0x48FF00, 0x48FF55, 0x48FFAA, 0x48FFFF,
  0x6D0000, 0x6D0055, 0x6D00AA, 0x6D00FF, 0x6D2400, 0x6D2455, 0x6D24AA, 0x6D24FF,
  0x6D4800, 0x6D4855, 0x6D48AA, 0x6D48FF, 0x6D6D00, 0x6D6D55, 0x6D6DAA, 0x6D6DFF,
  0x6D9100, 0x6D9155, 0x6D91AA, 0x6D91FF, 0x6DB600, 0x6DB655, 0x6DB6AA, 0x6DB6FF,
  0x6DDA00, 0x6DDA55, 0x6DDAAA, 0x6DDAFF, 0x6DFF00, 0x6DFF55, 0x6DFFAA, 0x6DFFFF,
  0x910000, 0x910055, 0x9100AA, 0x9100FF, 0x912400, 0x912455, 0x9124AA, 0x9124FF,
  0x914800, 0x914855, 0x9148AA, 0x9148FF, 0x916D00, 0x916D55, 0x916DAA, 0x916DFF,
  0x919100, 0x919155, 0x9191AA, 0x9191FF, 0x91B600, 0x91B655, 0x91B6AA, 0x91B6FF,
  0x91DA00, 0x91DA55, 0x91DAAA, 0x91DAFF, 0x91FF00, 0x91FF55, 0x91FFAA, 0x91FFFF,
  0xB60000, 0xB60055, 0xB600AA, 0xB600FF, 0xB62400, 0xB62455, 0xB624AA, 0xB624FF,
  0xB64800, 0xB64855, 0xB648AA, 0xB648FF, 0xB66D00, 0xB66D55, 0xB66DAA, 0xB66DFF,
  0xB69100, 0xB69155, 0xB691AA, 0xB691FF, 0xB6B600, 0xB6B655, 0xB6B6AA, 0xB6B6FF,
  0xB6DA00, 0xB6DA55, 0xB6DAAA, 0xB6DAFF, 0xB6FF00, 0xB6FF55, 0xB6FFAA, 0xB6FFFF,
  0xDA0000, 0xDA0055, 0xDA00AA, 0xDA00FF, 0xDA2400, 0xDA2455, 0xDA24AA, 0xDA24FF,
  0xDA4800, 0xDA4855, 0xDA48AA, 0xDA48FF, 0xDA6D00, 0xDA6D55, 0xDA6DAA, 0xDA6DFF,
  0xDA9100, 0xDA9155, 0xDA91AA, 0xDA91FF, 0xDAB600, 0xDAB655, 0xDAB6AA, 0xDAB6FF,
  0xDADA00, 0xDADA55, 0xDADAAA, 0xDADAFF, 0xDAFF00, 0xDAFF55, 0xDAFFAA, 0xDAFFFF,
  0xFF0000, 0xFF0055, 0xFF00AA, 0xFF00FF, 0xFF2400, 0xFF2455, 0xFF24AA, 0xFF24FF,
  0xFF4800, 0xFF4855, 0xFF48AA, 0xFF48FF, 0xFF6D00, 0xFF6D55, 0xFF6DAA, 0xFF6DFF,
  0xFF9100, 0xFF9155, 0xFF91AA, 0xFF91FF, 0xFFB600, 0xFFB655, 0xFFB6AA, 0xFFB6FF,
  0xFFDA00, 0xFFDA55, 0xFFDAAA, 0xFFDAFF, 0xFFFF00, 0xFFFF55, 0xFFFFAA, 0xFFFFFF};
#endif

LCD_FrameBuffer fb;
/**
  * @brief  Initializes the LCD.
  * @param  None
  * @retval LCD state
  */
uint8_t BSP_LCD_Init(void)
{ 
  uint8_t ret = LCD_ERROR;
  
  /* Default value for draw propriety */
  DrawProp.BackColor = LCD_DEFAULT_BACKCOLOR;
  DrawProp.TextColor = LCD_DEFAULT_TEXTCOLOR;
  DrawProp.pFont     = &LCD_DEFAULT_FONT;
  
  /* LCD Init */
  #if DEF_SIZEX > 0 && DEF_SIZEY > 0 && defined(LCD_MALLOC)
  fb.SizeX = DEF_SIZEX;
  fb.SizeY = DEF_SIZEY;
  fb.pixels = LCD_MALLOC(DEF_SIZEX * DEF_SIZEY * LCD_BPP);
  if(fb.pixels)
  {
    //hltdc.LayerCfg[1].FBStartAdress = (uint32_t)fb.pixels;
    LTDC_Layer1->CFBAR = (uint32_t)fb.pixels;
    #if   LCD_COLORBITDEPTH == 8
    LTDC_Layer1->CR |= LTDC_LxCR_CLUTEN;
    for(uint32_t i = 0; i < 256; i++)
      LTDC_Layer1->CLUTWR = (i << 24) | DefaultPalette[i];
    #endif
    LTDC->SRCR = LTDC_SRCR_VBR;
    ret = LCD_OK;
    /* Clear the LCD screen */
    #if LCD_INIT_CLEAR == 1
    BSP_LCD_Clear(LCD_DEFAULT_BACKCOLOR);
    #endif
  }
  #endif
  
  return ret;
}

/**
  * @brief  Gets the LCD X size.
  * @param  None    
  * @retval Used LCD X size
  */
uint16_t BSP_LCD_GetXSize(void)
{
  return(fb.SizeX);
}

/**
  * @brief  Gets the LCD Y size.
  * @param  None   
  * @retval Used LCD Y size
  */
uint16_t BSP_LCD_GetYSize(void)
{
  return(fb.SizeY);
}

/**
  * @brief  Gets the LCD text color.
  * @param  None 
  * @retval Used text color.
  */
COLORVAL BSP_LCD_GetTextColor(void)
{
  return DrawProp.TextColor;
}

/**
  * @brief  Gets the LCD background color.
  * @param  None
  * @retval Used background color
  */
COLORVAL BSP_LCD_GetBackColor(void)
{
  return DrawProp.BackColor;
}

/**
  * @brief  Sets the LCD text color.
  * @param  Color: Text color code RGB(5-6-5)
  * @retval None
  */
void BSP_LCD_SetTextColor(COLORVAL Color)
{
  DrawProp.TextColor = Color;
}

/**
  * @brief  Sets the LCD background color.
  * @param  Color: Background color code RGB(5-6-5)
  * @retval None
  */
void BSP_LCD_SetBackColor(COLORVAL Color)
{
  DrawProp.BackColor = Color;
}

/**
  * @brief  Sets the LCD text font.
  * @param  fonts: Font to be used
  * @retval None
  */
void BSP_LCD_SetFont(sFONT *pFonts)
{
  DrawProp.pFont = pFonts;
}

/**
  * @brief  Gets the LCD text font.
  * @param  None
  * @retval Used font
  */
sFONT *BSP_LCD_GetFont(void)
{
  return DrawProp.pFont;
}

/**
  * @brief  Clears the hole LCD.
  * @param  Color: Color of the background
  * @retval None
  */
void BSP_LCD_Clear(COLORVAL Color)
{
  int is = fb.SizeX * fb.SizeY;
  for(int i = 0; i < is; i++)
    fb.pixels[i] = Color;
}

/**
  * @brief  Clears the selected line.
  * @param  Line: Line to be cleared
  *          This parameter can be one of the following values:
  *            @arg  0..9: if the Current fonts is Font16x24
  *            @arg  0..19: if the Current fonts is Font12x12 or Font8x12
  *            @arg  0..29: if the Current fonts is Font8x8
  * @retval None
  */
void BSP_LCD_ClearStringLine(uint16_t Line)
{ 
  uint32_t color_backup = DrawProp.TextColor; 
  DrawProp.TextColor = DrawProp.BackColor;;
    
  /* Draw a rectangle with background color */
  BSP_LCD_FillRect(0, (Line * DrawProp.pFont->Height), BSP_LCD_GetXSize(), DrawProp.pFont->Height);
  
  DrawProp.TextColor = color_backup;
  BSP_LCD_SetTextColor(DrawProp.TextColor);
}

/**
  * @brief  Displays one character.
  * @param  Xpos: Start column address
  * @param  Ypos: Line where to display the character shape.
  * @param  Ascii: Character ascii code
  *           This parameter must be a number between Min_Data = 0x20 and Max_Data = 0x7E 
  * @retval None
  */
void BSP_LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
  DrawChar(Xpos, Ypos, &DrawProp.pFont->table[(Ascii-' ') *\
    DrawProp.pFont->Height * ((DrawProp.pFont->Width + 7) / 8)]);
}

/**
  * @brief  Displays characters on the LCD.
  * @param  Xpos: X position (in pixel)
  * @param  Ypos: Y position (in pixel)   
  * @param  Text: Pointer to string to display on LCD
  * @param  Mode: Display mode
  *          This parameter can be one of the following values:
  *            @arg  CENTER_MODE
  *            @arg  RIGHT_MODE
  *            @arg  LEFT_MODE   
  * @retval None
  */
void BSP_LCD_DisplayStringAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text, Line_ModeTypdef Mode)
{
  uint16_t refcolumn = 1, i = 0;
  uint32_t size = 0, xsize = 0; 
  uint8_t  *ptr = Text;
  
  /* Get the text size */
  while (*ptr++) size ++ ;
  
  /* Characters number per line */
  xsize = (BSP_LCD_GetXSize()/DrawProp.pFont->Width);
  
  switch (Mode)
  {
  case CENTER_MODE:
    {
      refcolumn = Xpos + ((xsize - size)* DrawProp.pFont->Width) / 2;
      break;
    }
  case LEFT_MODE:
    {
      refcolumn = Xpos;
      break;
    }
  case RIGHT_MODE:
    {
      refcolumn =  - Xpos + ((xsize - size)*DrawProp.pFont->Width);
      break;
    }    
  default:
    {
      refcolumn = Xpos;
      break;
    }
  }
  
  /* Send the string character by character on lCD */
  while ((*Text != 0) & (((BSP_LCD_GetXSize() - (i*DrawProp.pFont->Width)) & 0xFFFF) >= DrawProp.pFont->Width))
  {
    /* Display one character on LCD */
    BSP_LCD_DisplayChar(refcolumn, Ypos, *Text);
    /* Decrement the column position by 16 */
    refcolumn += DrawProp.pFont->Width;
    /* Point on the next character */
    Text++;
    i++;
  }
}

/**
  * @brief  Displays a character on the LCD.
  * @param  Line: Line where to display the character shape
  *          This parameter can be one of the following values:
  *            @arg  0..19: if the Current fonts is Font8
  *            @arg  0..12: if the Current fonts is Font12
  *            @arg  0...9: if the Current fonts is Font16
  *            @arg  0...7: if the Current fonts is Font20
  *            @arg  0...5: if the Current fonts is Font24
  * @param  ptr: Pointer to string to display on LCD
  * @retval None
  */
void BSP_LCD_DisplayStringAtLine(uint16_t Line, uint8_t *ptr)
{
  BSP_LCD_DisplayStringAt(0, LINE(Line), ptr, LEFT_MODE);
}

/**
  * @brief  Draws a pixel on LCD.
  * @param  Xpos: X position 
  * @param  Ypos: Y position
  * @param  RGB_Code: Pixel color in RGB mode (5-6-5)  
  * @retval None
  */
void BSP_LCD_DrawPixel(uint16_t Xpos, uint16_t Ypos, COLORVAL RGB_Code)
{
  fb.pixels[fb.SizeX * Ypos + Xpos] = RGB_Code;
}
  
/**
  * @brief  Draws an horizontal line.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Length: Line length
  * @retval None
  */
void BSP_LCD_DrawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  int32_t ip = fb.SizeX * Ypos + Xpos;
  while(Length--)
  {
    fb.pixels[ip] = DrawProp.TextColor;
    ip++;
  }
}

/**
  * @brief  Draws a vertical line.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Length: Line length
  * @retval None
  */
void BSP_LCD_DrawVLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  int32_t ip = fb.SizeX * Ypos + Xpos;
  while(Length--)
  {
    fb.pixels[ip] = DrawProp.TextColor;
    ip+= fb.SizeX;
  }
}

/**
  * @brief  Draws an uni-line (between two points).
  * @param  x1: Point 1 X position
  * @param  y1: Point 1 Y position
  * @param  x2: Point 2 X position
  * @param  y2: Point 2 Y position
  * @retval None
  */
void BSP_LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
  curpixel = 0;
  
  deltax = ABS(x2 - x1);        /* The difference between the x's */
  deltay = ABS(y2 - y1);        /* The difference between the y's */
  x = x1;                       /* Start x off at the first pixel */
  y = y1;                       /* Start y off at the first pixel */
  
  if (x2 >= x1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (y2 >= y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    BSP_LCD_DrawPixel(x, y, DrawProp.TextColor);  /* Draw the current pixel */
    num += numadd;                            /* Increase the numerator by the top of the fraction */
    if (num >= den)                           /* Check if numerator >= denominator */
    {
      num -= den;                             /* Calculate the new numerator value */
      x += xinc1;                             /* Change the x as appropriate */
      y += yinc1;                             /* Change the y as appropriate */
    }
    x += xinc2;                               /* Change the x as appropriate */
    y += yinc2;                               /* Change the y as appropriate */
  }
}

/**
  * @brief  Draws a rectangle.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Width: Rectangle width  
  * @param  Height: Rectangle height
  * @retval None
  */
void BSP_LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  /* Draw horizontal lines */
  BSP_LCD_DrawHLine(Xpos, Ypos, Width);
  BSP_LCD_DrawHLine(Xpos, (Ypos + Height - 1), Width);
  
  /* Draw vertical lines */
  BSP_LCD_DrawVLine(Xpos, Ypos, Height);
  BSP_LCD_DrawVLine((Xpos + Width - 1), Ypos, Height);
}
                            
/**
  * @brief  Draws a circle.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Radius: Circle radius
  * @retval None
  */
void BSP_LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t  D;       /* Decision Variable */ 
  uint32_t  CurX;   /* Current X Value */
  uint32_t  CurY;   /* Current Y Value */ 
  
  D = 3 - (Radius << 1);
  CurX = 0;
  CurY = Radius;
  
  while (CurX <= CurY)
  {
    BSP_LCD_DrawPixel((Xpos + CurX), (Ypos - CurY), DrawProp.TextColor);
    BSP_LCD_DrawPixel((Xpos - CurX), (Ypos - CurY), DrawProp.TextColor);
    BSP_LCD_DrawPixel((Xpos + CurY), (Ypos - CurX), DrawProp.TextColor);
    BSP_LCD_DrawPixel((Xpos - CurY), (Ypos - CurX), DrawProp.TextColor);
    BSP_LCD_DrawPixel((Xpos + CurX), (Ypos + CurY), DrawProp.TextColor);
    BSP_LCD_DrawPixel((Xpos - CurX), (Ypos + CurY), DrawProp.TextColor);
    BSP_LCD_DrawPixel((Xpos + CurY), (Ypos + CurX), DrawProp.TextColor);
    BSP_LCD_DrawPixel((Xpos - CurY), (Ypos + CurX), DrawProp.TextColor);   

    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  } 
}

/**
  * @brief  Draws an poly-line (between many points).
  * @param  Points: Pointer to the points array
  * @param  PointCount: Number of points
  * @retval None
  */
void BSP_LCD_DrawPolygon(pPoint Points, uint16_t PointCount)
{
  int16_t X = 0, Y = 0;

  if(PointCount < 2)
  {
    return;
  }

  BSP_LCD_DrawLine(Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y);
  
  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    BSP_LCD_DrawLine(X, Y, Points->X, Points->Y);
  }
}

/**
  * @brief  Draws an ellipse on LCD.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  XRadius: Ellipse X radius
  * @param  YRadius: Ellipse Y radius
  * @retval None
  */
void BSP_LCD_DrawEllipse(int Xpos, int Ypos, int XRadius, int YRadius)
{
  int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
  float K = 0, rad1 = 0, rad2 = 0;
  
  rad1 = XRadius;
  rad2 = YRadius;
  
  K = (float)(rad2/rad1);
  
  do {      
    BSP_LCD_DrawPixel((Xpos-(uint16_t)(x/K)), (Ypos+y), DrawProp.TextColor);
    BSP_LCD_DrawPixel((Xpos+(uint16_t)(x/K)), (Ypos+y), DrawProp.TextColor);
    BSP_LCD_DrawPixel((Xpos+(uint16_t)(x/K)), (Ypos-y), DrawProp.TextColor);
    BSP_LCD_DrawPixel((Xpos-(uint16_t)(x/K)), (Ypos-y), DrawProp.TextColor);      
    
    e2 = err;
    if (e2 <= x) {
      err += ++x*2+1;
      if (-y == x && e2 <= y) e2 = 0;
    }
    if (e2 > y) err += ++y*2+1;     
  }
  while (y <= 0);
}

/**
  * @brief  Draws a bitmap picture loaded in the STM32 MCU internal memory.
  * @param  Xpos: Bmp X position in the LCD
  * @param  Ypos: Bmp Y position in the LCD
  * @param  pBmp: Pointer to Bmp picture address
  * @retval None
  */
void BSP_LCD_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pBmp)
{
  uint32_t height = 0;
  uint32_t width  = 0;
  BITMAPSTRUCT *bm;
  
  bm = (BITMAPSTRUCT *)pBmp;
  width = bm->infoHeader.biWidth;
  height = bm->infoHeader.biHeight;
  BitmapDrawDir = RIGHT_THEN_UP;
  BSP_LCD_DrawRGB16Image(Xpos, Ypos, width, height, bm->data);
  BitmapDrawDir = RIGHT_THEN_DOWN;
}

/**
  * @brief  Draws a full rectangle.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Width: Rectangle width  
  * @param  Height: Rectangle height
  * @retval None
  */
void BSP_LCD_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  int32_t ip = fb.SizeX * Ypos + Xpos;
  int32_t ih = fb.SizeX - Width;
  while(Height--)
  {
    int32_t iw = Width;
    while(iw--)
    {
      fb.pixels[ip] = DrawProp.TextColor;
      ip++;
    }
    ip += ih;
  }
}

/**
  * @brief  Draws a full circle.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Radius: Circle radius
  * @retval None
  */
void BSP_LCD_FillCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t  D;        /* Decision Variable */ 
  uint32_t  CurX;    /* Current X Value */
  uint32_t  CurY;    /* Current Y Value */ 
  
  D = 3 - (Radius << 1);

  CurX = 0;
  CurY = Radius;
  
  BSP_LCD_SetTextColor(DrawProp.TextColor);

  while (CurX <= CurY)
  {
    if(CurY > 0) 
    {
      BSP_LCD_DrawHLine(Xpos - CurY, Ypos + CurX, 2*CurY);
      BSP_LCD_DrawHLine(Xpos - CurY, Ypos - CurX, 2*CurY);
    }

    if(CurX > 0) 
    {
      BSP_LCD_DrawHLine(Xpos - CurX, Ypos - CurY, 2*CurX);
      BSP_LCD_DrawHLine(Xpos - CurX, Ypos + CurY, 2*CurX);
    }
    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }

  BSP_LCD_DrawCircle(Xpos, Ypos, Radius);
}

/**
  * @brief  Draws a full poly-line (between many points).
  * @param  Points: Pointer to the points array
  * @param  PointCount: Number of points
  * @retval None
  */
void BSP_LCD_FillPolygon(pPoint Points, uint16_t PointCount)
{
  int16_t X = 0, Y = 0, X2 = 0, Y2 = 0, X_center = 0, Y_center = 0, X_first = 0, Y_first = 0, pixelX = 0, pixelY = 0, counter = 0;
  uint16_t  IMAGE_LEFT = 0, IMAGE_RIGHT = 0, IMAGE_TOP = 0, IMAGE_BOTTOM = 0;  
  
  IMAGE_LEFT = IMAGE_RIGHT = Points->X;
  IMAGE_TOP= IMAGE_BOTTOM = Points->Y;
  
  for(counter = 1; counter < PointCount; counter++)
  {
    pixelX = POLY_X(counter);
    if(pixelX < IMAGE_LEFT)
    {
      IMAGE_LEFT = pixelX;
    }
    if(pixelX > IMAGE_RIGHT)
    {
      IMAGE_RIGHT = pixelX;
    }
    
    pixelY = POLY_Y(counter);
    if(pixelY < IMAGE_TOP)
    {
      IMAGE_TOP = pixelY;
    }
    if(pixelY > IMAGE_BOTTOM)
    {
      IMAGE_BOTTOM = pixelY;
    }
  }  
  
  if(PointCount < 2)
  {
    return;
  }
  
  X_center = (IMAGE_LEFT + IMAGE_RIGHT)/2;
  Y_center = (IMAGE_BOTTOM + IMAGE_TOP)/2;
  
  X_first = Points->X;
  Y_first = Points->Y;
  
  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    X2 = Points->X;
    Y2 = Points->Y;    
    
    BSP_LCD_FillTriangle(X, Y, X2, Y2, X_center, Y_center);
    BSP_LCD_FillTriangle(X, Y, X_center, Y_center, X2, Y2);
    BSP_LCD_FillTriangle(X_center, Y_center, X2, Y2, X, Y);
  }
  
  BSP_LCD_FillTriangle(X_first, Y_first, X2, Y2, X_center, Y_center);
  BSP_LCD_FillTriangle(X_first, Y_first, X_center, Y_center, X2, Y2);
  BSP_LCD_FillTriangle(X_center, Y_center, X2, Y2, X_first, Y_first);
}

/**
  * @brief  Draws a full ellipse.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  XRadius: Ellipse X radius
  * @param  YRadius: Ellipse Y radius  
  * @retval None
  */
void BSP_LCD_FillEllipse(uint16_t Xpos, uint16_t Ypos, uint16_t XRadius, uint16_t YRadius)
{
  int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
  float K = 0, rad1 = 0, rad2 = 0;
  
  rad1 = XRadius;
  rad2 = YRadius;
  
  K = (float)(rad2/rad1);    
  
  do 
  { 
    BSP_LCD_DrawHLine((Xpos-(uint16_t)(x/K)), (Ypos+y), (2*(uint16_t)(x/K) + 1));
    BSP_LCD_DrawHLine((Xpos-(uint16_t)(x/K)), (Ypos-y), (2*(uint16_t)(x/K) + 1));
    
    e2 = err;
    if (e2 <= x) 
    {
      err += ++x*2+1;
      if (-y == x && e2 <= y) e2 = 0;
    }
    if (e2 > y) err += ++y*2+1;
  }
  while (y <= 0);
}

/**
  * @brief  Enables the display.
  * @param  None
  * @retval None
  */
void BSP_LCD_DisplayOn(void)
{
}

/**
  * @brief  Disables the display.
  * @param  None
  * @retval None
  */
void BSP_LCD_DisplayOff(void)
{
}

/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
  * @brief  Draws a character on LCD.
  * @param  Xpos: Line where to display the character shape
  * @param  Ypos: Start column address
  * @param  pChar: Pointer to the character data
  * @retval None
  */
static void DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *pChar)
{
  uint32_t counterh = 0, counterw = 0;
  uint16_t height = 0, width = 0;
  uint8_t offset = 0;
  uint8_t *pchar = NULL;
  uint32_t line = 0;
  int32_t ip;

  height = DrawProp.pFont->Height;
  width  = DrawProp.pFont->Width;
  
  offset =  8 *((width + 7)/8) - width ;
  
  for(counterh = 0; counterh < height; counterh++)
  {
    pchar = ((uint8_t *)pChar + (width + 7)/8 * counterh);
    
    if(((width + 7)/8) == 3)
    {
      line =  (pchar[0]<< 16) | (pchar[1]<< 8) | pchar[2];
    }
    
    if(((width + 7)/8) == 2)
    {
      line =  (pchar[0]<< 8) | pchar[1];
    }
    
    if(((width + 7)/8) == 1)
    {
      line =  pchar[0];
    }    
    
    ip = fb.SizeX * (Ypos + counterh) + Xpos;

    for (counterw = 0; counterw < width; counterw++)
    {
      if(line & (1 << (width- counterw + offset- 1))) 
      {
        fb.pixels[ip] = DrawProp.TextColor;
        ip++;
      }
      else
      {
        fb.pixels[ip] = DrawProp.BackColor;
        ip++;
      } 
    }
  }
}

/**
  * @brief  Fills a triangle (between 3 points).
  * @param  Points: Pointer to the points array
  * @param  x1: Point 1 X position
  * @param  y1: Point 1 Y position
  * @param  x2: Point 2 X position
  * @param  y2: Point 2 Y position
  * @param  x3: Point 3 X position
  * @param  y3: Point 3 Y position
  * @retval None
  */
void BSP_LCD_FillTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3)
{
  int16_t a, b, y, last;

  // Sort coordinates by Y order (y3 >= y2 >= y1)
  if (y1 > y2)
  {
    SWAP16(y1, y2); SWAP16(x1, x2);
  }
  if (y2 > y3) {
    SWAP16(y3, y2); SWAP16(x3, x2);
  }
  if (y1 > y2) {
    SWAP16(y1, y2); SWAP16(x1, x2);
  }

  if(y1 == y3)
  { // Handle awkward all-on-same-line case as its own thing
    a = b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    if(x3 < a)      a = x3;
    else if(x3 > b) b = x3;
    BSP_LCD_DrawHLine(a, y1, b - a + 1);
    return;
  }

  int16_t
  dx12 = x2 - x1,
  dy12 = y2 - y1,
  dx13 = x3 - x1,
  dy13 = y3 - y1,
  dx23 = x3 - x2,
  dy23 = y3 - y2;
  int32_t
  sa   = 0,
  sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 1-2 and 1-3.  If y2=y3 (flat-bottomed triangle), the scanline y2
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y2 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y1=y2
  // (flat-topped triangle).
  if(y2 == y3) last = y2;   // Include y2 scanline
  else         last = y2 - 1; // Skip it

  for(y = y1; y <= last; y++)
  {
    a   = x1 + sa / dy12;
    b   = x1 + sb / dy13;
    sa += dx12;
    sb += dx13;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x1 + (x3 - x1) * (y - y2) / (y3 - y1);
    */
    if(a > b) SWAP16(a, b);
    BSP_LCD_DrawHLine(a, y, b - a + 1);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 1-3 and 2-3.  This loop is skipped if y1=y2.
  sa = (int32_t)dx23 * (y - y2);
  sb = (int32_t)dx13 * (y - y1);
  for(; y <= y3; y++)
  {
    a   = x2 + sa / dy23;
    b   = x1 + sb / dy13;
    sa += dx23;
    sb += dx13;
    /* longhand:
    a = x2 + (x3 - x2) * (y - y2) / (y3 - y2);
    b = x1 + (x3 - x1) * (y - y1) / (y3 - y1);
    */
    if(a > b) SWAP16(a, b);
    BSP_LCD_DrawHLine(a, y, b - a + 1);
  }
}

#ifdef   LCD_DRVTYPE_V1_1
uint16_t BSP_LCD_ReadID(void)
{
  return 0;
}

COLORVAL BSP_LCD_ReadPixel(uint16_t Xpos, uint16_t Ypos)
{
  return fb.pixels[fb.SizeX * Ypos + Xpos];
}

void BSP_LCD_DrawRGB16Image(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData)
{
  int32_t ip, ih;
  uint16_t cv16;
  if(BitmapDrawDir == RIGHT_THEN_DOWN)
  {
    ip = fb.SizeX * Ypos + Xpos;
    ih = fb.SizeX - Xsize;
  }
  else /* if(BitmapDrawDir == RIGHT_THEN_UP) */
  {
    ip = fb.SizeX * (Ypos + Ysize - 1) + Xpos;
    ih = 0 - (fb.SizeX + Xsize);
  }

  while(Ysize--)
  {
    int32_t iw = Xsize;
    while(iw--)
    {
      cv16 = *pData;
      fb.pixels[ip] = LCD_COLOR_FROM_RGB565(cv16);
      ip++;
      pData++;
    }
    ip += ih;
  }
}

void BSP_LCD_ReadRGB16Image(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData)
{
  int32_t ip, ih;
  COLORVAL cv;
  if(BitmapDrawDir == RIGHT_THEN_DOWN)
  {
    ip = fb.SizeX * Ypos + Xpos;
    ih = fb.SizeX - Xsize;
  }
  else /* if(BitmapDrawDir == RIGHT_THEN_UP) */
  {
    ip = fb.SizeX * (Ypos + Ysize - 1) + Xpos;
    ih = 0 - (fb.SizeX + Xsize);
  }

  while(Ysize--)
  {
    int32_t iw = Xsize;
    while(iw--)
    {
      cv = fb.pixels[ip];
      *pData = LCD_COLOR_TO_RGB565(cv);
      ip++;
      pData++;
    }
    ip += ih;
  }
}

#endif  /* #ifdef   LCD_DRVTYPE_V1_1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
