/*
 * HC-SR04.c
 *
 *  Created on: Dec 17, 2017
 *      Author: ollo
 */


/******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include "hc-sr04.h"

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#define DEBUG_PRINT( ... )	chprintf((BaseSequentialStream *) &SD6, __VA_ARGS__);/**< Uart print */

/******************************************************************************
 * LOCAL VARIABLES for this module
 ******************************************************************************/
static WORKING_AREA(waDistanceThread, 4096);

/******************************************************************************
 * LOCAL FUNCTIONS for this module
 ******************************************************************************/

static msg_t
distanceThread(void *arg)
{
	(void) arg;
	systime_t time = chTimeNow();
	chRegSetThreadName("distsensor");

	while (TRUE) {

		/* each half second one measurement */
		chThdSleepMilliseconds(500);
	}

	return RDY_OK;
}

/******************************************************************************
 * GLOBAL INTERFACE of this module
 ******************************************************************************/

void hcsr04_init(void)
{
	DEBUG_PRINT("Start distance thread ...");
	  chThdCreateStatic(waDistanceThread, sizeof(waDistanceThread), NORMALPRIO,
	      distanceThread, NULL);
	DEBUG_PRINT(" Done\r\n");
}
