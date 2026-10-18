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

#ifndef INC_TOUCH_DRV_H_
#define INC_TOUCH_DRV_H_

#include <stdbool.h>
#include "stm32l4xx_hal.h"

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

/**
  * @brief  return values for read coordinates
  */
typedef enum {
  TOUCHPAD_DATA_NOISY = 0,    /*!<    */
  TOUCHPAD_DATA_OK            /*!<    */

} touchDataStatus_e;

/**
  * @brief  return values for touch pressure detection
  */
typedef enum {
  TOUCHPAD_NOT_PRESSED = 0,   /*!<    */
  TOUCHPAD_PRESSED            /*!<    */

} touchPressStatus_e;

/**
  * @brief  Possible display orientation of this TFT LCD and the touch.
  *
  * note: this orientation is tightly couple with the screen. It's base on the conception
  *       of the entire screen module CH280QV10-CT.
  */
typedef enum {

  TOUCH_RIBBON_BOTTOM = 0U,   /*!<  The ribbon is visible at the bottom of the viewer */
  TOUCH_RIBBON_RIGHT,         /*!<  The ribbon is visible at the right of the viewer  */
  TOUCH_RIBBON_TOP,           /*!<  The ribbon is visible at the top of the viewer    */
  TOUCH_RIBBON_LEFT,          /*!<  The ribbon is visible at the left of the viewer   */
  TOUCH_RIBBON_MAX

} touchScreenOrientation_e;

// ================================================================================================
// ================================================================================================
//            ENUM DECLARATION
// ================================================================================================
// ================================================================================================

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

bool touchInit(I2C_HandleTypeDef* pI2cHandle);

touchDataStatus_e touchReadCoordinates(uint16_t* pX, uint16_t* pY);
touchPressStatus_e touchPressed(void);

void touchSetRotation(touchScreenOrientation_e orientation);

#endif /* INC_TOUCH_DRV_H_ */

