/*
 * file:
 * Jack Beecher, CS58, W25
 *
 * description:
 */

#ifndef TRAPS_H
#define TRAPS_H

#include "ykernel.h"
#include "structs.h"
#include "coordination.h"
#include "codes.h"
#include "stubs.h"
#include "sys.h"

/************************ func ************************/
/*
 * this snippet of code does this
 *
 * input:
 *
 * output:
 *
 * notes:
 */

//-------------------------------------------------------

int trap_initTable(void);

//-------------------------------------------------------

/************************ func ************************/
/*
 * this snippet of code does this
 *
 * input:
 *
 * output:
 *
 * notes:
 */

//-------------------------------------------------------


void trap_kernelHandler(UserContext* uc);


//-------------------------------------------------------/

/************************ func ************************/
/*
 * this snippet of code does this
 *
 * input:
 *
 * output:
 *
 * notes:
 */

//-------------------------------------------------------

void trap_clockHandler(UserContext* uc);

//-------------------------------------------------------/

/************************ func ************************/
/*
 * this snippet of code does this
 *
 * input:
 *
 * output:
 *
 * notes:
 */

//-------------------------------------------------------

void trap_illegalHandler(UserContext* uc);

//-------------------------------------------------------/

/************************ func ************************/
/*
 * this snippet of code does this
 *
 * input:
 *
 * output:
 *
 * notes:
 */

//-------------------------------------------------------

void trap_memoryHandler(UserContext* uc);

//-------------------------------------------------------/

/************************ func ************************/
/*
 * this snippet of code does this
 *
 * input:
 *
 * output:
 *
 * notes:
 */

//-------------------------------------------------------

void trap_mathHandler(UserContext* uc);

//-------------------------------------------------------

/*************** TtyReceive ***************/
/*
 * TtyReceive:
 *   Retrieves data received by the terminal hardware.
 *
 * input:
 *   tty_id - the identifier for the terminal from which to read.
 *   buf    - pointer to the buffer where the received data should be stored.
 *   len    - the maximum number of bytes to read.
 *
 * output:
 *   Fills the provided buffer with up to 'len' bytes of received data.
 *
 * returns:
 *   The actual number of bytes received and copied into the buffer, or ERROR if an error occurs.
 * 
 */

//-------------------------------------------------------/

void trap_ttyReceiveHandler(UserContext* uc);

//-------------------------------------------------------/

/*************** TtyTransmit ***************/
/*
 * TtyTransmit:
 *   Initiates hardware transmission of data for the specified terminal.
 *
 * input:
 *   tty_id - the identifier for the terminal on which to transmit.
 *   buf    - pointer to the buffer containing the data to be transmitted.
 *   len    - the number of bytes to transmit.
 *
 * output:
 *   Initiates the hardware transmit mechanism and returns once transmission
 *   is scheduled (or in progress).
 *
 * returns:
 *   Typically, the number of bytes that were scheduled for transmission (normally 'len').
 *   Returns ERROR if any error occurs.
 *
 */

 //-------------------------------------------------------

void trap_ttyTransmitHandler(UserContext* uc);

//-------------------------------------------------------/

/************************ func ************************/
/*
 * this snippet of code does this
 *
 * input:
 *
 * output:
 *
 * notes:
 */

//-------------------------------------------------------

void trap_diskHandler(UserContext* uc);

//-------------------------------------------------------/

/************************ func ************************/
/*
 * this snippet of code does this
 *
 * input:
 *
 * output:
 *
 * notes:
 */

//-------------------------------------------------------

void trap_nullHandler(UserContext* uc);

//-------------------------------------------------------/

#endif
