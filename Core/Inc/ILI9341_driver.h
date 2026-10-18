// ------------------------------------------------------------------------------------------------
/* TGE CEGEP ST-Jean-sur-Richelieu - www.cstjean.qc.ca
   COPYRIGHT (c) 2017-2026 BY TGE CEGEP ST-Jean-sur-Richelieu. ALL RIGHTS RESERVED. NO PART OF
   THIS PROGRAM OR PUBLICATION MAY BE REPRODUCED, TRANSMITTED, TRANSCRIBED, STORED IN A RETRIEVAL
   SYSTEM, OR TRANSLATED INTO ANY LANGUAGE OR COMPUTER LANGUAGE IN ANY FORM OR BY ANY MEANS,
   ELECTRONIC, MECHANICAL, MAGNETIC, OPTICAL, CHEMICAL, MANUAL, OR OTHERWISE, WITHOUT THE PRIOR
   WRITTEN PERMISSION OF TGE DEPARTMENT.
*/
// ------------------------------------------------------------------------------------------------
/*!@file
   @brief   Simple library to control the ILI9341 TFT LCD Single Chip Driver.

   @note  Only necessary functions are implemented in this driver. High level functions must be
      implemented in a different file. For example, the functions of circle, triangle,
      and so on.

   @author  Dominique Bastien
   @version 1.0
*/
// ------------------------------------------------------------------------------------------------

#ifndef INC_ILI9341_DRIVER_H_
#define INC_ILI9341_DRIVER_H_

#include "stm32l4xx_hal.h"

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

#define BLACK         0x0000
#define BLUE          0x001F
#define GREEN         0x07E0
#define RED           0xF800
#define YELLOW        0xFFE0
#define WHITE         0xFFFF
#define ORANGE        0xFBE0
#define PINK          0xF81F

// ================================================================================================
// ================================================================================================
//            ENUM DECLARATION
// ================================================================================================
// ================================================================================================

/**
  * @brief  Possible display orientation of this TFT LCD
  */
typedef enum {

  ILI9341_RIBBON_BOTTOM = 0U,   /*!<  The ribbon is visible at the bottom of the viewer */
  ILI9341_RIBBON_RIGHT,         /*!<  The ribbon is visible at the right of the viewer  */
  ILI9341_RIBBON_TOP,           /*!<  The ribbon is visible at the top of the viewer  */
  ILI9341_RIBBON_LEFT,          /*!<  The ribbon is visible at the left of the viewer   */
  ILI9341_RIBBON_MAX

} ili9341ScreenRotation_e;

// ================================================================================================
// ================================================================================================
//            STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            EXTERNAL FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================

void ILI9341_Init(SPI_HandleTypeDef* pSpiHandle);
void ILI9341_Reset(void);
void ILI9341_SetRotation(ili9341ScreenRotation_e rotation);

void ILI9341_displayOn(void);
void ILI9341_displayOff(void);

void ILI9341_setBrightness(uint8_t level);

void ILI9341_fillScreen(uint16_t color);
void ILI9341_writePixel(uint16_t x0, uint16_t y0, uint16_t color);

void ILI9341_drawFastVLine(uint16_t x0, uint16_t y0, uint16_t h, uint16_t color);
void ILI9341_drawFastHLine(uint16_t x0, uint16_t y0, uint16_t w, uint16_t color);

void ILI9341_drawFastRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_drawRawBuffer(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t* buffer);
void ILI9341_drawRawBufferDMA(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t* buffer);

#endif /* INC_ILI9341_DRIVER_H_ */

