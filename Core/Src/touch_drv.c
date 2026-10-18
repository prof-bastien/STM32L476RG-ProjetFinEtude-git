// ------------------------------------------------------------------------------------------------
/* TGE CEGEP ST-Jean-sur-Richelieu - www.cstjean.qc.ca
   COPYRIGHT (c) 2017-2025 BY TGE CEGEP ST-Jean-sur-Richelieu. ALL RIGHTS RESERVED. NO PART OF
   THIS PROGRAM OR PUBLICATION MAY BE REPRODUCED, TRANSMITTED, TRANSCRIBED, STORED IN A RETRIEVAL
   SYSTEM, OR TRANSLATED INTO ANY LANGUAGE OR COMPUTER LANGUAGE IN ANY FORM OR BY ANY MEANS,
   ELECTRONIC, MECHANICAL, MAGNETIC, OPTICAL, CHEMICAL, MANUAL, OR OTHERWISE, WITHOUT THE PRIOR
   WRITTEN PERMISSION OF TGE DEPARTMENT.
*/
// ------------------------------------------------------------------------------------------------
/*!@file
   @brief Simple driver to extract from the FT6X06 controller the coordinates of the TFT screen.

   @author Dominique Bastien
   @version 1.0
*/
// ------------------------------------------------------------------------------------------------

#include <stdlib.h>
#include "touch_drv.h"

// ================================================================================================
// ================================================================================================
//            PRIVATE DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

#define FT6X06_I2C_ADDR (0x38<<1)   /*!< Address of the FocalTech, Self-Capacitive Touch ctrl. */

#define FT6X06_NUMTOUCHES 0x02      /*!< Number of touch points */
#define FT6X06_THRESHHOLD 0x80      /*!< Threshold for touch detection */
#define FT6X06_POINTRATE  0x88      /*!< Point rate           */
#define FT6X06_FIRMVERS   0xA6      /*!< Firmware version     */
#define FT6X06_CHIPID     0xA3      /*!< Chip selecting       */
#define FT6X06_VENDID     0xA8      /*!< FocalTech's panel ID */

#define FT6X06_VENDID_VAL 0x11      /*!< FocalTech's panel ID */
#define FT6206_CHIPID_VAL 0x06      /*!< Chip selecting       */

#define FT6206_DEF_THRES  40        /*!< Default threshold for touch detection */

// ================================================================================================
// ================================================================================================
//            PRIVATE MACRO DEFINITION
// ================================================================================================
// ================================================================================================

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

static I2C_HandleTypeDef * pI2cBus = NULL;

static touchScreenOrientation_e CurrentOrientation = TOUCH_RIBBON_LEFT;

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================

static uint8_t touchReadReg(uint8_t addr);
static void touchWriteReg(uint8_t addr, uint8_t data);

// ================================================================================================
// ================================================================================================
//            PUBLIC FUNCTION SECTION
// ================================================================================================
// ================================================================================================

/**
  * @brief Initialize the Touch Controller.
  *
  * @param  pI2cHandle pointer to the I2C bus handle.
  *
  * @return true in case of success, false otherwise.
  */
bool touchInit(I2C_HandleTypeDef * pI2cHandle)
{
  bool      retCode = false;
  uint8_t   vendorId;
  uint8_t   chipId;


  if (pI2cHandle != NULL)
  {
    pI2cBus = pI2cHandle;
    retCode = true;

    vendorId  = touchReadReg(FT6X06_VENDID);
    chipId    = touchReadReg(FT6X06_CHIPID);

    // Verify if it's the right Capacitive controller.
    if ((vendorId == FT6X06_VENDID_VAL) && (chipId == FT6206_CHIPID_VAL))
    {
      retCode = true;
      // Set the threshold to the minimum value.
      touchWriteReg(FT6X06_THRESHHOLD, FT6206_DEF_THRES);
    }
  }

  return retCode;
}

/**
  * @brief Get the coordinates
  *
  * @note This function is used to Get the coordinates from the touch controller.
  *
  * @param pX pointer to a location to store the X coordinate
  * @param pY pointer to a location to store the Y coordinate
  *
  * @return touchPressStatus_e TOUCHPAD_DATA_OK or TOUCHPAD_DATA_NOISY
  */
touchDataStatus_e touchReadCoordinates(uint16_t * pX, uint16_t * pY)
{
  touchDataStatus_e retCode = TOUCHPAD_DATA_NOISY;
  uint8_t           registers[16];
  uint8_t           addr = 0;

  uint16_t touchX[2], touchY[2];

  if ((pI2cBus != NULL) && (pX != NULL) && (pY != NULL))
  {
    if(HAL_I2C_Master_Transmit(pI2cBus, FT6X06_I2C_ADDR, &addr, 1, 5) == HAL_OK)
    {
      if (HAL_I2C_Master_Receive(pI2cBus, FT6X06_I2C_ADDR, registers, 16, 5) == HAL_OK)
      {
        // Extract X and Y.
        for (uint8_t i = 0; i < 2; i++)
        {
          touchX[i] = registers[0x03 + i * 6] & 0x0F;
          touchX[i] <<= 8;
          touchX[i] |= registers[0x04 + i * 6];
          touchY[i] = registers[0x05 + i * 6] & 0x0F;
          touchY[i] <<= 8;
          touchY[i] |= registers[0x06 + i * 6];
        }

        // Saturation des coordonnées brutes (écran 240x320 : bornes 0..239 et 0..319)
        if (touchX[0] > 239) touchX[0] = 239;
        if (touchY[0] > 319) touchY[0] = 319;

        // Support for screen orientation.
        switch(CurrentOrientation)
        {
          case TOUCH_RIBBON_BOTTOM:
            *pX = 239 - touchX[0];
            *pY = 319 - touchY[0];
            break;
          case TOUCH_RIBBON_RIGHT:
            *pX = 319 - touchY[0];
            *pY = touchX[0];
            break;
          case TOUCH_RIBBON_TOP:
            *pX = touchX[0];
            *pY = touchY[0];
            break;
          case TOUCH_RIBBON_LEFT:
          default:
            *pX = touchY[0];
            *pY = 239 - touchX[0];
            break;
        }

        retCode = TOUCHPAD_DATA_OK;
      }
    }
  }

  return retCode;
}

/**
  * @brief Check the touch controller for events.
  *
  * @note This function is used to check the touch controller register numTouches. If the
  *       counter is not zero we can read the data inside the controller.
  *
  * @param none
  *
  * @return touchPressStatus_e TOUCHPAD_PRESSED or TOUCHPAD_NOT_PRESSED
  */
touchPressStatus_e touchPressed(void)
{
  touchPressStatus_e status = TOUCHPAD_NOT_PRESSED;
  uint8_t numTouchPoints = 0;

  numTouchPoints = touchReadReg(FT6X06_NUMTOUCHES);

  // Masque sur les 4 bits de poids faible pour extraire le nombre de touches valide (1 ou 2)
  if ((numTouchPoints & 0x0F) > 0 && (numTouchPoints & 0x0F) <= 2)
  {
    status = TOUCHPAD_PRESSED;
  }

  return status;
}

/**
  * @brief Set the rotation of the screen.
  *
  * @note This function set the rotation of the screen in one of the 4 possibilities.
  *
  * @param  rotation  new rotation of the screen
  *
  * @return none
  */
void touchSetRotation(touchScreenOrientation_e orientation)
{
  if ((orientation < TOUCH_RIBBON_MAX) && (orientation >= TOUCH_RIBBON_BOTTOM))
    CurrentOrientation = orientation;
}

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION SECTION
// ================================================================================================
// ================================================================================================

/**
  * @brief Simple function to read a single register.
  *
  * @note This function is use to read a single register, useful to simplify initialization code.
  *
  * @param addr Address of the register
  *
  * @return the register value
  */
static uint8_t touchReadReg(uint8_t addr)
{
  uint8_t regVal = 0;

  if (pI2cBus != NULL)
  {
    if(HAL_I2C_Master_Transmit(pI2cBus, FT6X06_I2C_ADDR, &addr, 1, 5) == HAL_OK)
    {
      HAL_I2C_Master_Receive(pI2cBus, FT6X06_I2C_ADDR, &regVal, 1, 5);
    }
  }

  return regVal;
}

/**
  * @brief Simple function to write a single register.
  *
  * @note This function is use to write a single register, useful to simplify initialization code.
  *
  * @param addr Address of the register
  * @param data Data to write inside the register
  *
  * @return none
  */
static void touchWriteReg(uint8_t addr, uint8_t data)
{
  uint8_t registers[2];

  registers[0] = addr;
  registers[1] = data;

  if (pI2cBus != NULL)
  {
    HAL_I2C_Master_Transmit(pI2cBus, FT6X06_I2C_ADDR, registers, 2, 5);
  }
}
