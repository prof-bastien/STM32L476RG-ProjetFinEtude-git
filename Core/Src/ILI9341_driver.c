// ------------------------------------------------------------------------------------------------
/* TGE CEGEP ST-Jean-sur-Richelieu - www.cstjean.qc.ca
   COPYRIGHT (c) 2017-2026 BY TGE CEGEP ST-Jean-sur-Richelieu. ALL RIGHTS RESERVED. NO PART OF
   THIS PROGRAM OR PUBLICATION MAY BE REPRODUCED, TRANSMITTED, TRANSCRIBED, STORED IN A RETRIEVAL
   SYSTEM, OR TRANSLATED INTO ANY LANGUAGE OR COMPUTER LANGUAGE IN ANY FORM OR BY ANY MEANS,
   ELECTRONIC, MECHANICAL, MAGNETIC, OPTICAL, CHEMICAL, MANUAL, OR OTHERWISE, WITHOUT THE PRIOR
   WRITTEN PERMISSION OF TGE DEPARTMENT.
*/
// ------------------------------------------------------------------------------------------------
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

#include <stdbool.h>
#include "ILI9341_driver.h"
#include "main.h"

// ================================================================================================
// ================================================================================================
//            PRIVATE DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

#define ILI9341_RDPIXFMT  0x0C      ///< Read Display Pixel Format
#define ILI9341_SLPOUT    0x11      ///< Sleep Out

#define ILI9341_GAMMASET  0x26      ///< Gamma Set
#define ILI9341_DISPOFF   0x28      ///< Display OFF
#define ILI9341_DISPON    0x29      ///< Display ON
#define ILI9341_CASET     0x2A      ///< Column Address Set
#define ILI9341_PASET     0x2B      ///< Page Address Set
#define ILI9341_RAMWR     0x2C      ///< Memory Write

#define ILI9341_MADCTL    0x36      ///< Memory Access Control
#define ILI9341_VSCRSADD  0x37      ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT    0x3A      ///< COLMOD: Pixel Format Set

#define ILI9341_BRIGHT    0x51      ///< Write Display Brightness

#define ILI9341_FRMCTR1   0xB1      ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_DFUNCTR   0xB6      ///< Display Function Control

#define ILI9341_PWCTR1    0xC0      ///< Power Control 1
#define ILI9341_PWCTR2    0xC1      ///< Power Control 2

#define ILI9341_VMCTR1    0xC5      ///< VCOM Control 1
#define ILI9341_VMCTR2    0xC7      ///< VCOM Control 2

#define ILI9341_PWCTRA    0xCB      ///< Power Control A
#define ILI9341_PWCTRB    0xCF      ///< Power Control B

#define ILI9341_GMCTRP1   0xE0      ///< Positive Gamma Correction
#define ILI9341_GMCTRN1   0xE1      ///< Negative Gamma Correction
#define ILI9341_DRVTIMEA  0xE8      ///< Driver timing control A
#define ILI9341_DRVTIMEB  0xEA      ///< Driver timing control A

#define ILI9341_PWSEQ     0xED      ///< Power on sequence control

#define ILI9341_PUMPRATE  0xF7      ///< Pump ratio control

#define  RESET_OFF()    HAL_GPIO_WritePin(SCREEN_RESET_GPIO_Port, SCREEN_RESET_Pin, GPIO_PIN_SET)
#define  RESET_ON()     HAL_GPIO_WritePin(SCREEN_RESET_GPIO_Port, SCREEN_RESET_Pin, GPIO_PIN_RESET)
#define  CS_OFF()       HAL_GPIO_WritePin(SCREEN_CS_GPIO_Port, SCREEN_CS_Pin, GPIO_PIN_SET)
#define  CS_ON()        HAL_GPIO_WritePin(SCREEN_CS_GPIO_Port, SCREEN_CS_Pin, GPIO_PIN_RESET)
#define  DC_DATA()      HAL_GPIO_WritePin(SCREEN_DC_GPIO_Port, SCREEN_DC_Pin, GPIO_PIN_SET)
#define  DC_COMMAND()   HAL_GPIO_WritePin(SCREEN_DC_GPIO_Port, SCREEN_DC_Pin, GPIO_PIN_RESET)

#define swap(a,b)     {int16_t t=a;a=b;b=t;}

#define MAX_PIXELS      120
// ================================================================================================
// ================================================================================================
//            PRIVATE MACRO DEFINITION
// ================================================================================================
// ================================================================================================

#define HAL_DELAY( _delay_ )  \
  do {HAL_Delay( _delay_ );}  \
  while (0)

// ================================================================================================
// ================================================================================================
//            PRIVATE STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PRIVATE VARIABLE DECLARATION
// ================================================================================================
// ================================================================================================
static SPI_HandleTypeDef* pLcdSpi;

static uint16_t currentWidth  = 240;
static uint16_t currentHeight = 320;

static uint16_t pixels[MAX_PIXELS];

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================

static void ILI9341_SendCommand(uint8_t command);
static void ILI9341_SendData(uint8_t data);

static void ILI9341_SetAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
static void ILI9341_SendBurst(uint16_t, uint32_t length);
static void ILI9341_SendArray(uint16_t* array, uint32_t len);
static void ILI9341_SendArrayDMA(uint16_t* array, uint32_t len);

static bool ILI9341_SPI_Send(unsigned char data);
static bool ILI9341_SPI_SendMultiple(uint8_t* pData, int size);

static void ILI9341_setCfg(void);

// ================================================================================================
// ================================================================================================
//            PUBLIC FUNCTION SECTION
// ================================================================================================
// ================================================================================================

/**
  * @brief  Initialize the ILI9341 to a known state.
  *
  * @note   This function performs a hardware reset and it resets the commands
  *         and parameters to their software desired values
  *
  * @param  pSpiHandle  Pointer to the Handle to access the STM32 SPI
  *
  * @return none
  */
void ILI9341_Init(SPI_HandleTypeDef* pSpiHandle)
{
  // Save the address of the SPI Handle locally.
  pLcdSpi = pSpiHandle;

  ILI9341_Reset();
  ILI9341_setCfg();
}

/**
  * @brief  Perform a hardware reset followed by a software reset.
  *
  * @note   This function performs a hardware reset based on p.225 of the datasheet.
  *         It is also followed by a software reset. When the Software Reset command is written,
  *         it causes a software reset. It resets the commands and parameters to their software
  *         default values.
  *
  * @note   It will be necessary to wait 5ms before sending a new command following a software reset.
  *         The display module loads all display supplier factory default values to the registers
  *         during this 5ms. The Frame Memory contents are unaffected by this command.
  *
  * @param  none
  *
  * @return none
  */
void ILI9341_Reset(void)
{
  RESET_ON();
  HAL_DELAY(50);
  RESET_OFF();
  HAL_DELAY(50);

  // Send a software reset.
  CS_ON();
  ILI9341_SendCommand(0x01);
  CS_OFF();
  HAL_DELAY(150);
}

/**
  * @brief  Set the rotation of the screen.
  *
  * @note   This function sets the rotation of the screen in one of the 4 possibilities.
  *
  * @param  rotation  new screen orientation
  *
  * @return none
  */
void ILI9341_SetRotation(ili9341ScreenRotation_e rotation)
{
  ILI9341_SendCommand(ILI9341_MADCTL);

  switch (rotation)
  {
    case ILI9341_RIBBON_BOTTOM:
      ILI9341_SendData(0x48);
      currentWidth  = 240;
      currentHeight = 320;
      break;
    case ILI9341_RIBBON_RIGHT:
      ILI9341_SendData(0x28);
      currentWidth  = 320;
      currentHeight = 240;
      break;
    case ILI9341_RIBBON_TOP:
      ILI9341_SendData(0x88);
      currentWidth  = 240;
      currentHeight = 320;
      break;
    case ILI9341_RIBBON_LEFT:
    default:
      ILI9341_SendData(0xE8);
      currentWidth  = 320;
      currentHeight = 240;
      break;
  }
}

void ILI9341_displayOn(void)
{
  ILI9341_SendCommand(ILI9341_DISPON);
}

void ILI9341_displayOff(void)
{
  ILI9341_SendCommand(ILI9341_DISPOFF);
}

void ILI9341_setBrightness(uint8_t level)
{
  ILI9341_SendCommand(ILI9341_BRIGHT);
  ILI9341_SendData(level);
}

/**
  * @brief  Set the entire screen to a given color.
  *
  * @note   This function is used to set the entire screen to the background color.
  *
  * @param  color  the color in the 5-6-5 RGB format
  *
  * @return none
  */
void ILI9341_fillScreen(uint16_t color)
{
  ILI9341_SetAddress(0, 0, currentWidth-1, currentHeight-1);
  ILI9341_SendBurst(color, (long)currentWidth * (long)currentHeight);
}

/**
  * @brief  Draw a simple pixel.
  *
  * @note   This function is used to draw a simple pixel in the screen.	The pixel MUST be
  *         in the valid area.
  *
  * @param  x       X coordinate of the pixel
  * @param  y       Y coordinate of the pixel
  * @param  color   color in a 5-6-5 RGB format
  *
  * @return none
  */
void ILI9341_writePixel(uint16_t x, uint16_t y, uint16_t color)
{
  uint8_t data[2];

  ILI9341_SetAddress(x, y, currentWidth-1, currentHeight-1);
  DC_COMMAND();
  ILI9341_SPI_Send(ILI9341_RAMWR);
  DC_DATA();

  data[0] = color >> 8;
  data[1] = color & 0xFF;

  ILI9341_SPI_SendMultiple(data, 2);
}

/**
  * @brief  Optimize function to draw a vertical line.
  *
  * @note   This function is used to draw a line between two points and used the internal
  *         pointers increment to speed up the process.
  *
  * @param  x0      X of the top starting pixel
  * @param  y0      Y of the top starting pixel
  * @param  h       Height of the line
  * @param  color   color in a 5-6-5 RGB format
  *
  * @return none
  */
void ILI9341_drawFastVLine(uint16_t x0, uint16_t y0, uint16_t h, uint16_t color)
{
  ILI9341_SetAddress(x0, y0, x0, y0+h-1);
  ILI9341_SendBurst(color, h);
}

/**
  * @brief  Optimize function to draw a horizontal line.
  *
  * @note   This function is used to draw a line between two points and used the internal
  *         pointers increment to speed up the process.
  *
  * @param  x0      X of the left starting pixel
  * @param  y0      Y of the left starting pixel
  * @param  w       Width of the line
  * @param  color   color in a 5-6-5 RGB format
  *
  * @return none
  */
void ILI9341_drawFastHLine(uint16_t x0, uint16_t y0, uint16_t w, uint16_t color)
{
  if (w > 0)
  {
    ILI9341_SetAddress(x0, y0, x0+w-1, y0);
    ILI9341_SendBurst(color, w);
  }
}

void ILI9341_drawFastRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
{

  if ((w > 0) && (h > 0))
  {
    ILI9341_SetAddress(x0, y0, x0+w-1, y0+h-1);
    ILI9341_SendBurst(color, w*h);
  }
}

void ILI9341_drawRawBuffer(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t* buffer)
{
  if ((w > 0) && (h > 0))
  {
    ILI9341_SetAddress(x0, y0, x0+w-1, y0+h-1);
    ILI9341_SendArray(buffer, w*h);
  }
}

void ILI9341_drawRawBufferDMA(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t* buffer)
{
  if ((w > 0) && (h > 0))
  {
    ILI9341_SetAddress(x0, y0, x0+w-1, y0+h-1);

    // TODO: For DMA
    ILI9341_SendArrayDMA(buffer, w*h*2);
  }
}

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION SECTION
// ================================================================================================
// ================================================================================================

/**
  * @brief  Send a command to the ILI9341.
  *
  * @note   This function send a command to the ILI9341. Following the timing diagram give in
  *         the specification.
  *
  * @param  command   One of the available ILI9341 command.
  *
  * @return none
  */
static void ILI9341_SendCommand(uint8_t command)
{
  // Put D/C* to 0 and CS* to 0.
  DC_COMMAND();
  CS_ON();
  ILI9341_SPI_Send(command);
  CS_OFF();
}

/**
  * @brief  Send a byte of data to the ILI9341.
  *
  * @note This function send a data byte to theILI9341.  Following the timing diagram give in
  *         the specification.
  *
  * @param  data the data need for the previous command
  *
  * @return none
  */
static void ILI9341_SendData(uint8_t data)
{
  // Put D/C* to 1 and CS* to 0.
  DC_DATA();
  CS_ON();
  ILI9341_SPI_Send(data);
  CS_OFF();
}

/**
  * @brief  Configure the internal pointers to access the GRAM.
  *
  * @note This function configure the internal pointers to access the Graphical RAM.
  *
  * @param  x1 starting column for the internal pointer
  * @param  y1 starting row for the internal pointer
  * @param  x2 ending column for the internal pointer
  * @param  y2 ending row for the internal pointer
  *
  * @return none
  */
static void ILI9341_SetAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  uint8_t data[4];

  data[0] = x1 >> 8;
  data[1] = x1 & 0xFF;
  data[2] = x2 >> 8;
  data[3] = x2 & 0xFF;

  CS_ON();
  DC_COMMAND();
  ILI9341_SPI_Send(ILI9341_CASET);
  DC_DATA();
  ILI9341_SPI_SendMultiple(data, 4);

  data[0] = y1 >> 8;
  data[1] = y1 & 0xFF;
  data[2] = y2 >> 8;
  data[3] = y2 & 0xFF;

  DC_COMMAND();
  ILI9341_SPI_Send(ILI9341_PASET);
  DC_DATA();
  ILI9341_SPI_SendMultiple(data, 4);

  /*
   *  IMPORTANT: Keep the CS* down for the following data transfer !!!
   */
}

/**
  * @brief  Sends the color value for multiple pixels.
  *
  * @note   This function sends the color value for multiple pixels
  *
  * @param  color   color in the 5-6-5 RGB format
  * @param  len     number of pixels affected
  *
  * @return none
  */
static void ILI9341_SendBurst(uint16_t color, uint32_t len)
{
  CS_ON();
  DC_COMMAND();
  ILI9341_SPI_Send(ILI9341_RAMWR);
  DC_DATA();

  for (int i = 0; i < MAX_PIXELS; i++)
  {
    pixels[i] = (color >> 8)|(color << 8);
  }

  uint32_t nbrPixels = len;

  while (nbrPixels > 0)
  {
    if (nbrPixels > MAX_PIXELS)
    {
      ILI9341_SPI_SendMultiple((uint8_t *)pixels, 2*MAX_PIXELS);
      nbrPixels -= MAX_PIXELS;
    }
    else
    {
      ILI9341_SPI_SendMultiple((uint8_t *)pixels, 2*nbrPixels);
      nbrPixels -= nbrPixels;
    }
  }

  CS_OFF();
}

/**
  * @brief  Sends an array of pixels.
  *
  * @note   This function sends an aray of pixels. The set address is already done.
  *
  * @param  array   pointer to the pixels in a 5-6-5 RGB format
  * @param  len     number of affected pixels
  *
  * @return none
  */
static void ILI9341_SendArray(uint16_t* array, uint32_t len)
{
  uint32_t  arrayIdx = 0;

  CS_ON();
  DC_COMMAND();
  ILI9341_SPI_Send(ILI9341_RAMWR);
  DC_DATA();

  uint32_t  nbrPixels = len;

  while (nbrPixels > 0 )
  {
    if (nbrPixels > MAX_PIXELS)
    {
      for (int i=0; i<MAX_PIXELS; i++, arrayIdx++)
      {
        pixels[i] = (array[arrayIdx]>>8)|(array[arrayIdx]<<8);
      }

      ILI9341_SPI_SendMultiple((uint8_t *)pixels, 2*MAX_PIXELS);
      nbrPixels -= MAX_PIXELS;
    }
    else
    {
      for (int i=0; i<nbrPixels; i++, arrayIdx++)
      {
        pixels[i] = (array[arrayIdx]>>8)|(array[arrayIdx]<<8);
      }

      ILI9341_SPI_SendMultiple((uint8_t *)pixels, 2*nbrPixels);
      nbrPixels -= nbrPixels;
    }
  }

  CS_OFF();
}


static void ILI9341_SendArrayDMA(uint16_t* array, uint32_t len)
{
  CS_ON();
  DC_COMMAND();
  ILI9341_SPI_Send(ILI9341_RAMWR);
  DC_DATA();

  HAL_SPI_Transmit_DMA(pLcdSpi, (uint8_t*)array, len);

  //CS_OFF();
}

/**
  * @brief  Sends a single byte via an SPI transaction
  *
  * @note   This function sends a single byte via an SPI transaction.
  *
  * @param  none
  *
  * @return none
  */
static bool ILI9341_SPI_Send(unsigned char data)
{
  bool  retCode = false;

  if (HAL_SPI_Transmit(pLcdSpi, &data, 1, 1) == HAL_OK)
  {
    retCode = true;
  }

  return retCode;
}

/**
  * @breif  Sends multiple bytes via SPI in a single transaction.
  *
  * @note   This function send multiple bytes in single SPI transaction.
  *
  * @param  none
  *
  * @return none
  */
static bool ILI9341_SPI_SendMultiple(uint8_t* pData, int size)
{
  bool  retCode = false;

  if (HAL_SPI_Transmit(pLcdSpi, pData, size, 10) == HAL_OK)
  {
    retCode = true;
  }

  return retCode;
}

/**
  * @brief  Sets all the registers to the desired values.
  *
  * @note   Sets all the registers to the desired values. This function is based
  *         on the work of Adafruit. Some of the registers don't exist in the datasheet
  *         that we have
  *
  *  @note  https://github.com/adafruit/Adafruit_ILI9341/blob/master/Adafruit_ILI9341.cpp
  *
  * @param  none
  *
  * @return none
  */
static void ILI9341_setCfg(void)
{
  //
  ILI9341_SendCommand(0xEF);
  ILI9341_SendData(0x03);
  ILI9341_SendData(0x80);
  ILI9341_SendData(0x02);

  // Power control B (p.196)
  ILI9341_SendCommand(ILI9341_PWCTRB);
  ILI9341_SendData(0x00);
  ILI9341_SendData(0xC1);
  ILI9341_SendData(0x30);

  // Power on sequence control (p.200)
  ILI9341_SendCommand(ILI9341_PWSEQ);
  ILI9341_SendData(0x64);
  ILI9341_SendData(0x03);
  ILI9341_SendData(0x12);
  ILI9341_SendData(0x81);

  // Driver timing control A (p.197)
  ILI9341_SendCommand(ILI9341_DRVTIMEA);
  ILI9341_SendData(0x85);
  ILI9341_SendData(0x00);
  ILI9341_SendData(0x78);

  // Power Control A (p.195)
  ILI9341_SendCommand(ILI9341_PWCTRA);
  ILI9341_SendData(0x39);
  ILI9341_SendData(0x2C);
  ILI9341_SendData(0x00);
  ILI9341_SendData(0x34);
  ILI9341_SendData(0x02);

  // Pump ratio control (p.202)
  ILI9341_SendCommand(ILI9341_PUMPRATE);
  ILI9341_SendData(0x20);

  // Driver timing control B (p.199)
  ILI9341_SendCommand(ILI9341_DRVTIMEB);
  ILI9341_SendData(0x00);
  ILI9341_SendData(0x00);

  // Power Control 1 (p.178)
  ILI9341_SendCommand(ILI9341_PWCTR1);
  ILI9341_SendData(0x23);

  // Power Control 2 (p.178)
  ILI9341_SendCommand(ILI9341_PWCTR2);
  ILI9341_SendData(0x10);

  // VCOM Control 1 (p.180)
  ILI9341_SendCommand(ILI9341_VMCTR1);
  ILI9341_SendData(0x3E);
  ILI9341_SendData(0x28);

  // VCOM Control 2  (p.182)
  ILI9341_SendCommand(ILI9341_VMCTR2);
  ILI9341_SendData(0x86);

  // Memory Access Control (p.127)
  ILI9341_SendCommand(ILI9341_MADCTL);
  ILI9341_SendData(0xE8);

  // Vertical Scrolling Start Address (p.129)
  ILI9341_SendCommand(ILI9341_VSCRSADD);
  ILI9341_SendData(0x00);

  // Pixel Format Set: 16bits (p.134)
  ILI9341_SendCommand(ILI9341_PIXFMT);
  ILI9341_SendData(0x55);

  // Frame Rate Control (In Normal Mode/Full Colors) (p.155)
  ILI9341_SendCommand(ILI9341_FRMCTR1);
  ILI9341_SendData(0x00);
  ILI9341_SendData(0x18);

  // Display Function Control (p.164)
  ILI9341_SendCommand(ILI9341_DFUNCTR);
  ILI9341_SendData(0x08);
  ILI9341_SendData(0x82);
  ILI9341_SendData(0x27);

  // 3Gamma Function Disable
  ILI9341_SendCommand(0xF2);
  ILI9341_SendData(0x00);

  //Gamma set (p.107)
  ILI9341_SendCommand(ILI9341_GAMMASET);
  ILI9341_SendData(0x01);

  //Positive Gamma Correction (p.226)
  ILI9341_SendCommand(ILI9341_GMCTRP1);
  ILI9341_SendData(0x0F);
  ILI9341_SendData(0x31);
  ILI9341_SendData(0x2B);
  ILI9341_SendData(0x0C);
  ILI9341_SendData(0x0E);
  ILI9341_SendData(0x08);
  ILI9341_SendData(0x4E);
  ILI9341_SendData(0xF1);
  ILI9341_SendData(0x37);
  ILI9341_SendData(0x07);
  ILI9341_SendData(0x10);
  ILI9341_SendData(0x03);
  ILI9341_SendData(0x0E);
  ILI9341_SendData(0x09);
  ILI9341_SendData(0x00);

  //Negative Gamma Correction (p.227)
  ILI9341_SendCommand(ILI9341_GMCTRN1);
  ILI9341_SendData(0x00);
  ILI9341_SendData(0x0E);
  ILI9341_SendData(0x14);
  ILI9341_SendData(0x03);
  ILI9341_SendData(0x11);
  ILI9341_SendData(0x07);
  ILI9341_SendData(0x31);
  ILI9341_SendData(0xC1);
  ILI9341_SendData(0x48);
  ILI9341_SendData(0x08);
  ILI9341_SendData(0x0F);
  ILI9341_SendData(0x0C);
  ILI9341_SendData(0x31);
  ILI9341_SendData(0x36);
  ILI9341_SendData(0x0F);

  // Sleep Out (p.101)
  ILI9341_SendCommand(ILI9341_SLPOUT);
  HAL_DELAY(150);

  // Display ON (p.109)
  ILI9341_SendCommand(ILI9341_DISPON);
  HAL_DELAY(150);
}



