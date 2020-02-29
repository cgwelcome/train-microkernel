#ifndef __SERVER_UI_H__
#define __SERVER_UI_H__

#include <stdint.h>

/**
 * @defgroup ui
 * @ingroup server
 *
 * @{
 */

#define CLOCK_PRECISION 100
#define CMD_BUFFER_SIZE  64

/**
 * Creates a UI Server
 */
void CreateUIServer();

/** @}*/

#endif /*__SERVER_UI_H__*/
