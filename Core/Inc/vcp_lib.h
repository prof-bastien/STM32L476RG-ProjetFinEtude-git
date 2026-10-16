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
   @brief This is a library to use native VCP on most Nucleo development boards.

   @note This library has the advantage of supporting UTF-8 format for the vcp_puts and vcp_printf
         functions. It does not support UTF-8 for the vcp_putchar function. The vcp_getch and
         vcp_getchar functions were implemented to provide support compatible with simple menu
         programming on PCs.

   @author Dominique Bastien
   @version 1.3
*/
// ------------------------------------------------------------------------------------------------

#ifndef INC_VCP_LIB_H_
#define INC_VCP_LIB_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#include "stm32l4xx_hal.h"

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

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

bool vcp_init(UART_HandleTypeDef* pUartHandle);

uint16_t vcp_putchar(const char c);
uint16_t vcp_puts(const char* string);
uint16_t vcp_printf(const char* fmt, ...);

int vcp_getch(void);
int vcp_getchar(void);

void vcp_test(void);

#endif /* INC_VCP_LIB_H_ */



