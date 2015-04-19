/*
 * boblight.c
 *
 *  Created on: Apr 4, 2015
 *      Author: c3ma
 */

#include "boblight.h"

#include "usbcdc/usbcdc.h"
#include "ledstripe/ledstripe.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#define	STARTOFFSET				2
#define HEX_SIZE				2
#define TEXTLINE_MAX_LENGTH 	1024

#define DEBUG_PRINT( ... )	chprintf((BaseSequentialStream *) &SD6, __VA_ARGS__);/**< Uart print */

/******************************************************************************
 * LOCAL VARIABLES for this module
 ******************************************************************************/
static WORKING_AREA(waBoblightThread, 4096);

static int channelSize = 0;

static int ledOffset = 0;

/******************************************************************************
 * LOCAL FUNCTIONS for this module
 ******************************************************************************/

static void readDirectWS2812cmd(char *textbuffer)
{
	int i=0;
	int length = usbcdc_readAll(textbuffer, TEXTLINE_MAX_LENGTH);
	if(length >= 2 && (textbuffer[i] == 'A' && textbuffer[i+1] == 'd' && textbuffer[i+2] == 'a'))
	{
		ledOffset=0;
		channelSize = textbuffer[i+3] * 256 + textbuffer[i+4];
		//DEBUG_PRINT("%3d [%2d chan. CRC: %2X]\r\n", i, channelSize, textbuffer[i+5]);

		for(i=6; i < length; i+=3)
		{
			if (textbuffer[i] == 'A' && textbuffer[i+1] == 'd' && textbuffer[i+2] == 'a')
			{
				channelSize = textbuffer[i+3] * 256 + textbuffer[i+4];
				//DEBUG_PRINT("\r\n%3d [%2d chan. CRC: %2X] REOPEND", i, channelSize, textbuffer[i+5]);
				i+=6;
				ledOffset=0;
			}

			ledstripe_framebuffer[ledOffset].red = 	(uint8_t) textbuffer[i+0];
			ledstripe_framebuffer[ledOffset].green = 	(uint8_t) textbuffer[i+1];
			ledstripe_framebuffer[ledOffset].blue = 	(uint8_t) textbuffer[i+2];
			ledOffset++;
			/*DEBUG_PRINT("%.2X%.2X%.2X ", textbuffer[i+0], textbuffer[i+1], textbuffer[i+2]); */
		}
		/* DEBUG_PRINT("\r\n"); */
	}
	else if (length > 0)
	{
		for(i=0; i < (length - 3); i+=3)
		{
			ledstripe_framebuffer[ledOffset].red = 	(uint8_t) textbuffer[i+0];
			ledstripe_framebuffer[ledOffset].green =(uint8_t) textbuffer[i+1];
			ledstripe_framebuffer[ledOffset].blue = (uint8_t) textbuffer[i+2];
			ledOffset++;
		}
	}
}


static msg_t
boblightThread(void *arg)
{
	(void) arg;
	systime_t time = chTimeNow();
	char textbuffer[TEXTLINE_MAX_LENGTH];

	chRegSetThreadName("boblight");

	/*
	 * Initialize USB serial console
	 */
	usbcdc_init(NULL);

	/* Say hello to the host */
	usbcdc_print("Ada\n");
	time = chTimeNow();

	DEBUG_PRINT("Start listening...\r\n");
	  while (1)
	  {
		  usbcdc_process();
		  memset(textbuffer, 0, TEXTLINE_MAX_LENGTH);
		  readDirectWS2812cmd(textbuffer);
		  chThdSleepMilliseconds(5);

			/* Send ACK to host each second */
		  if (time + MS2ST(1000) < chTimeNow())
		  {
			usbcdc_putMemory((uint8_t *) "Ada\n", 4);
			time = chTimeNow();
			DEBUG_PRINT("Still alive, channel size %4d, actual offset %4d,\r\n", channelSize, ledOffset);
		  }
	  }

	  return RDY_OK;
}

void boblight_init(void)
{
	DEBUG_PRINT("Start boblight thread ...");
	  chThdCreateStatic(waBoblightThread, sizeof(waBoblightThread), NORMALPRIO,
	      boblightThread, NULL);
	DEBUG_PRINT(" Done\r\n");
}
