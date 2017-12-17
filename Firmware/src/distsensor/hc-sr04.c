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
#include "board.h"

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#define DEBUG_PRINT( ... )	chprintf((BaseSequentialStream *) &SD6, __VA_ARGS__);/**< Uart print */

#define ST2US(st)                                                         \
  ((systime_t) (((((uint32_t)(st)) - 1UL) * 1000000UL) + 1UL) / ((uint32_t)CH_FREQUENCY) )

#define HCSR04_MEASUREMENT_INTERVALL	500000 /* in us */


/******************************************************************************
 * Interface of LOCAL FUNCTIONS for this module
 ******************************************************************************/
static void extcb1(EXTDriver *extp, expchannel_t channel);

/******************************************************************************
 * LOCAL VARIABLES for this module
 ******************************************************************************/
static WORKING_AREA(waDistanceThread, 4096);
static systime_t mTrigTime = 0U;

static const EXTConfig extcfg = {
  {
    {EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extcb1},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL}
  }
};

/******************************************************************************
 * LOCAL FUNCTIONS for this module
 ******************************************************************************/

static void extcb1(EXTDriver *extp, expchannel_t channel) {
	(void)extp;
	(void)channel;

	chSysLockFromIsr();
	mTrigTime = chTimeNow();
	chSysUnlockFromIsr();
}

static msg_t
distanceThread(void *arg)
{
	systime_t time = chTimeNow();
	unsigned int duration;
	(void) arg;
	chRegSetThreadName("distsensor");


	while (TRUE) {
		extChannelDisable(&EXTD1, 0);
		mTrigTime=0U;
		/* According to the datasheet a high level of 10us is required */
		palSetPad(GPIOA, GPIOA_HCSR04_TRIG);
		chThdSleepMicroseconds(10);
		palClearPad(GPIOA, GPIOA_HCSR04_TRIG);
		time = chTimeNow();
	    extChannelEnable(&EXTD1, 0);

		/* each two seconds one measurement */
		chThdSleepMilliseconds(2000);

		/* Check, if we received something */
		if (mTrigTime > 0) {
			duration = ST2US(mTrigTime - time);
			DEBUG_PRINT("Duration: %d\r\n", duration);
		}
	}

	return RDY_OK;
}

/******************************************************************************
 * GLOBAL INTERFACE of this module
 ******************************************************************************/

void hcsr04_init(void)
{
	/*
	 * Activates the EXT driver 1.
	 */
	extStart(&EXTD1, &extcfg);

	DEBUG_PRINT("Start distance thread ...");
	  chThdCreateStatic(waDistanceThread, sizeof(waDistanceThread), NORMALPRIO,
	      distanceThread, NULL);
	DEBUG_PRINT(" Done\r\n");
}
