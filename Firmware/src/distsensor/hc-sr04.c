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


/** (Backported from Chibios 3.0)
 * @brief   System ticks to microseconds.
 * @details Converts from system ticks number to microseconds.
 * @note    The result is rounded up to the next microsecond boundary.
 *
 * @param[in] n         number of system ticks
 * @return              The number of microseconds.
 *
 * @api
 */
#define ST2US(n) ((((n) - 1UL) / (CH_FREQUENCY / 1000000UL)) + 1UL)

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
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_RISING_EDGE | EXT_MODE_GPIOA, extcb1},
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

	mTrigTime = chTimeNow();
}

static msg_t
distanceThread(void *arg)
{
	systime_t time = chTimeNow();
	unsigned int duration;
	(void) arg;
	chRegSetThreadName("distsensor");


    extChannelEnable(&EXTD1, 3);
	while (TRUE) {
		/* According to the datasheet a high level of 10us is required */
		palClearPad(GPIOA, GPIOA_HCSR04_TRIG);
		chThdSleepMilliseconds(10);
		time = chTimeNow();
		mTrigTime=0U;
		palSetPad(GPIOA, GPIOA_HCSR04_TRIG);

		/* one measurements per second */
		chThdSleepMilliseconds(1000);

		/* Check, if we received something */
		if (mTrigTime > 0) {
			duration = ST2US(mTrigTime - time);
			DEBUG_PRINT("Duration: %5d us Distance %5d cm\r\n", duration, (duration/58));
		}
	}

	extChannelDisable(&EXTD1, 3);
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

	palClearPad(GPIOA, GPIOA_HCSR04_TRIG);

	DEBUG_PRINT("Start distance thread ...");
	  chThdCreateStatic(waDistanceThread, sizeof(waDistanceThread), NORMALPRIO,
	      distanceThread, NULL);
	DEBUG_PRINT(" Done\r\n");
}
