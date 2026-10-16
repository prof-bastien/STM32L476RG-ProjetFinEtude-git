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

#include "vcp_lib.h"

// ================================================================================================
// ================================================================================================
//            PRIVATE DEFINE DECLARATION
// ================================================================================================
// ================================================================================================
#define VCP_VERSION "1.3"
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

// Keep the memory location of the handle locally to avoid external reference.
UART_HandleTypeDef* pUart;

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PUBLIC FUNCTION SECTION
// ================================================================================================
// ================================================================================================

/*
 *  @brief Initialize the debug serial port.
 *
 *  @param  pUartHandle pointer to the UART handle.
 *
 *  @return true in case of success, false otherwise.
 */
bool vcp_init(UART_HandleTypeDef* pUartHandle)
{
  bool retCode = false;

  if (pUartHandle != NULL)
  {
    pUart   = pUartHandle;
    retCode = true;
  }

  return retCode;
}

/*
 *  @brief Send a character to the debug serial port.
 *
 *  @param  character to be sent.
 *
 *  @return number of character sent.
 */
uint16_t vcp_putchar(const char c)
{
  uint16_t nbrChar = 0;

  if (HAL_UART_Transmit(pUart, (const uint8_t *)&c, 1, 100) == HAL_OK)
  {
    nbrChar = 1;
  }

  return nbrChar;
}

/*
 *  @brief Send a string of character to the debug serial port.
 *
 *  @note The functions snprintf() and vsnprintf() do not write more than size bytes (including
 *        the terminating null byte. If the output was truncated due to this limit then the
 *        return value is the number of characters (excluding the terminating null byte) which
 *        would have been written to the final string if enough space had been available.
 *        Thus, a return value of size or more means that the output was truncated.)
 *
 *  @param  string to be sent.
 *
 *  @return number of character sent.
 */
uint16_t vcp_puts(const char* string)
{
  char      txBuffer[128];
  int       retValue;

  uint16_t  txLen     = 0;
  uint16_t  nbrChar   = 0;
  uint32_t  timeout   = 0;
  uint32_t  baudrate  = pUart->Init.BaudRate;


  retValue = snprintf(txBuffer, sizeof(txBuffer), "%s", string);

  if(retValue > 0)
  {
    txLen = retValue;

    // Adjust the txLen before passing it to the UART transmit if it's necessary.
    if(txLen >= sizeof(txBuffer))
      txLen = sizeof(txBuffer) - 1;

    // Evaluate the time to transmit all the bytes.
    timeout = ((txLen * 10 * 1000) / baudrate) + 2;

    if (HAL_UART_Transmit(pUart, (const uint8_t *)txBuffer, txLen, timeout) == HAL_OK)
    {
      nbrChar = txLen;
    }
  }

  return nbrChar;
}

/*
 *  @brief Send a format string to the debug serial port.
 *
 *  @note The functions snprintf() and vsnprintf() do not write more than size bytes (including
 *        the terminating null byte. If the output was truncated due to this limit then the
 *        return value is the number of characters (excluding the terminating null byte) which
 *        would have been written to the final string if enough space had been available.
 *        Thus, a return value of size or more means that the output was truncated.)
 *
 *  @param  fmt   format.
 *  @param  ...   list of parameter to process.
 *
 *  @return number of character sent.
 */
uint16_t vcp_printf(const char* fmt, ...)
{
  char      txBuffer[128];
  va_list   vl;
  int       retValue;
  uint16_t  txLen     = 0;
  uint16_t  nbrChar   = 0;
  uint32_t  timeout   = 0;
  uint32_t  baudrate  = pUart->Init.BaudRate;

  va_start(vl, fmt);
  retValue = vsnprintf(txBuffer, sizeof(txBuffer), fmt, vl);
  va_end(vl);

  if(retValue > 0)
  {
    txLen = retValue;

    // Adjust the txLen before passing it to the UART transmit if it's necessary.
    if(txLen >= sizeof(txBuffer))
      txLen = sizeof(txBuffer) - 1;

    // Evaluate the time to transmit all the bytes.
    timeout = ((txLen * 10 * 1000) / baudrate) + 2;

    if(HAL_UART_Transmit(pUart, (const uint8_t*) txBuffer, txLen, timeout) == HAL_OK)
    {
      nbrChar = txLen;
    }
  }

  return nbrChar;
}

/*
 *  @brief  Receive a character in non-blocking mode
 *
 *  @param  none
 *
 *  @return the character read as an unsigned char cast to an int or EOF on end of file or error.
 */
int vcp_getch(void)
{
  uint8_t   data;
  int       retCode = -1;

  // non blocking
  if(HAL_UART_Receive(pUart, &data, 1, 0) == HAL_OK)
  {
    retCode = data;
  }

  return retCode;
}

/*
 *  @brief Receive a character in blocking mode
 *
 *  @param  none
 *
 *  @return the integer ERR upon failure and an integer value other than ERR
 */
int vcp_getchar(void)
{
  uint8_t   data;
  int       retCode = -1;

  // blocking
  if(HAL_UART_Receive(pUart, &data, 1, HAL_MAX_DELAY) == HAL_OK)
  {
    retCode = data;
  }

  return retCode;
}

/*
 *  @brief test the VCP library.
 *
 *  @param  none
 *
 *  @return none
 */
void vcp_test(void)
{
  vcp_printf("\r\n\r\n");
  vcp_printf("VCP Debug Library: version(%s) \r\n", VCP_VERSION);

  for (int i=0; i<26 ;i++)
  {
    vcp_putchar('a'+i);
  }
  vcp_putchar(0xD);
  vcp_putchar(0xA);

  vcp_getch();

  vcp_puts("Test Completed!");
  vcp_printf("\r\n\r\n");
}

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION SECTION
// ================================================================================================
// ================================================================================================
