/** @file main.c
 * @brief Entrypoint of the custom project
 * @author Ollo
 *
 * @date 28.06.2014
 * @defgroup WookieController
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "usbcdc/usbcdc.h"
#include "cmd/cmd.h"
#include "ledstripe/ledstripe.h"

/******************************************************************************
 * DEFINITIONS
 ******************************************************************************/

#define PRINT( ... )	chprintf((BaseSequentialStream *) &SD6, __VA_ARGS__);/**< Uart print */ \
                        usbcdc_print(__VA_ARGS__);

static WORKING_AREA(waLEDstripBlink, 128);
static msg_t
ledThread(void *arg);

#define LED_END_POSTION		0

/*===========================================================================*/
/* Command line related. */
/*===========================================================================*/
void
cmd_ledctrl(BaseSequentialStream *chp, int argc, char *argv[])
{
  int i;
  if (argc >= 1 && strcmp(argv[0], "test1") == 0)
    {
      chprintf(chp, "Red ...\r\n");
      for (i = 0; i < LEDSTRIPE_FRAMEBUFFER_SIZE; i++)
        {
          ledstripe_framebuffer[i].red = 255;
          ledstripe_framebuffer[i].green = 0;
          ledstripe_framebuffer[i].blue = 0;
        }
      chThdSleepMilliseconds(5000);
      chprintf(chp, "Green ...\r\n");
      for (i = 0; i < LEDSTRIPE_FRAMEBUFFER_SIZE; i++)
        {
          ledstripe_framebuffer[i].green = 255;
          ledstripe_framebuffer[i].red = 0;
          ledstripe_framebuffer[i].blue = 0;
        }
      chThdSleepMilliseconds(5000);
      chprintf(chp, "Blue ...\r\n");
      for (i = 0; i < LEDSTRIPE_FRAMEBUFFER_SIZE; i++)
        {
          ledstripe_framebuffer[i].blue = 255;
          ledstripe_framebuffer[i].red = 0;
          ledstripe_framebuffer[i].green = 0;
        }
      chThdSleepMilliseconds(5000);
    }
  else if (argc >= 1 && strcmp(argv[0], "on") == 0)
    {
      for (i = 0; i < LEDSTRIPE_FRAMEBUFFER_SIZE; i++)
        {
          ledstripe_framebuffer[i].red = 255;
          ledstripe_framebuffer[i].green = 255;
          ledstripe_framebuffer[i].blue = 255;
        }
    }
  else if (argc >= 1 && strcmp(argv[0], "off") == 0)
    {
      for (i = 0; i < LEDSTRIPE_FRAMEBUFFER_SIZE; i++)
        {
          ledstripe_framebuffer[i].red = 0;
          ledstripe_framebuffer[i].green = 0;
          ledstripe_framebuffer[i].blue = 0;
        }
    }
  else if (argc >= 4 && strcmp(argv[0], "end") == 0)
    {
      int red = atoi(argv[1]);
      int green = atoi(argv[2]);
      int blue = atoi(argv[3]);
      for (i = 0; i < LEDSTRIPE_FRAMEBUFFER_SIZE; i++)
        {
          ledstripe_framebuffer[i].red = 0;
          ledstripe_framebuffer[i].green = 0;
          ledstripe_framebuffer[i].blue = 0;
        }
      for (i = LED_END_POSTION; i < LEDSTRIPE_FRAMEBUFFER_SIZE; i++)
        {
          ledstripe_framebuffer[i].red = red;
          ledstripe_framebuffer[i].green = green;
          ledstripe_framebuffer[i].blue = blue;
        }
    }
  else if (argc >= 1 && strcmp(argv[0], "start") == 0)
    {
      chprintf(chp, "Start led thread ...");
      chThdCreateStatic(waLEDstripBlink, sizeof(waLEDstripBlink), NORMALPRIO,
          ledThread, NULL);
      chprintf(chp, " Done\r\n");
    }
  else if (argc >= 1 && strcmp(argv[0], "show") == 0)
    {
      chprintf(chp, "\r\n");
      for (i = 0; i < LEDSTRIPE_FRAMEBUFFER_SIZE; i++)
        {
          chprintf(chp, "%.2X%.2X%.2X ",
              ledstripe_framebuffer[i].red,
              ledstripe_framebuffer[i].green,
              ledstripe_framebuffer[i].blue);
        }
        chprintf(chp, "\r\n");
    }
  else if (argc >= 1)  /* Update the LEDs directly */
  {
	  int i,j= 0, color = 0;
	  long int number = 0;
	  int length = strlen(argv[0]);
	  char pEnd[2] = { '0', '0' };
	  for(i=0; i < length; i+=2){
		  memcpy(pEnd, argv[0] +i, 2);
		  number = strtol(pEnd, NULL, 16);
		  chprintf(chp, "%d %d\r\n", j, number);
		  switch(color)
		  {
		  case 0:
			  ledstripe_framebuffer[j].red = number;
			  break;
		  case 1:
			  ledstripe_framebuffer[j].green = number;
			  break;
		  case 2:
			  ledstripe_framebuffer[j].blue = number;
			  j++;
			  color=-1;
			  break;
		  }
		  color++;
	  }

  }
  else /* Usage */
    {
      chprintf(chp, "possible arguments are:\r\n"
          "-test1\r\n"
          "-start\r\n"
          "-end (red) (green) (blue)\tSet the last box\r\n"
          "-on\r\n"
          "-off\r\n"
          "-show\r\n");
    }
}

static const ShellCommand commands[] =
  {
    { "led", cmd_ledctrl },
    { "mem", cmd_mem },
    { "threads", cmd_threads },
    { NULL, NULL } };

static const ShellConfig shell_cfg1 =
  { (BaseSequentialStream *) &SD6, commands };

/*===========================================================================*/
/* Main and generic code.                                                    */
/*===========================================================================*/

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static WORKING_AREA(waThreadBlink, 128);
static msg_t
blinkerThread(void *arg)
{

  (void) arg;
  chRegSetThreadName("blinker");
  while (TRUE)
    {
      palSetPad(GPIOD, GPIOD_LED4); /* Green.  */
      chThdSleepMilliseconds(500);palClearPad(GPIOD, GPIOD_LED4); /* Green.  */
      chThdSleepMilliseconds(500);
    }
  return RDY_OK;
}

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static msg_t
ledThread(void *arg)
{
  int pos_r = 0;
  int pos_g = 0;
  int pos_b = 0;

  (void) arg;

  while (TRUE)
    {
      //code for running light
      pos_r %= LEDSTRIPE_FRAMEBUFFER_SIZE;
      pos_g %= LEDSTRIPE_FRAMEBUFFER_SIZE;
      pos_b %= LEDSTRIPE_FRAMEBUFFER_SIZE;
      ledstripe_framebuffer[pos_r].red = 255;
      if (pos_r > 0)
        ledstripe_framebuffer[pos_r - 1].red = 64;
      if (pos_r > 1)
        ledstripe_framebuffer[pos_r - 2].red = 10;
      ledstripe_framebuffer[pos_g].green = 0;
      if (pos_g > 0)
        ledstripe_framebuffer[pos_g - 1].green = 0;
      if (pos_g > 1)
        ledstripe_framebuffer[pos_g - 2].green = 0;
      ledstripe_framebuffer[pos_b].blue = 255;
      ;
      if (pos_b > 0)
        ledstripe_framebuffer[pos_b - 1].blue = 64;
      if (pos_b > 1)
        ledstripe_framebuffer[pos_b - 2].blue = 10;
      chThdSleepMilliseconds(10);
      ledstripe_framebuffer[pos_r].red = 0;
      if (pos_r > 0)
        ledstripe_framebuffer[pos_r - 1].red = 0;
      if (pos_r > 1)
        ledstripe_framebuffer[pos_r - 2].red = 0;
      pos_r++;
      ledstripe_framebuffer[pos_g].green = 0;
      if (pos_g > 0)
        ledstripe_framebuffer[pos_g - 1].green = 0;
      if (pos_g > 1)
        ledstripe_framebuffer[pos_g - 2].green = 0;
      pos_g++;
      ledstripe_framebuffer[pos_b].blue = 0;
      if (pos_b > 0)
        ledstripe_framebuffer[pos_b - 1].blue = 0;
      if (pos_b > 1)
        ledstripe_framebuffer[pos_b - 2].blue = 0;
      pos_b++;
    }

  return RDY_OK;
}

/*
 * Application entry point.
 */
int
main(void)
{
  int i, red, green, blue;
  int offset = 0;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Initialize USB serial console
   */
  usbcdc_init(commands);

  /*
   * Shell manager initialization.
   */
  shellInit();

  /*
   * Activates the serial driver 6 and SDC driver 1 using default
   * configuration.
   */
  sdStart(&SD6, NULL);

  chThdSleep(MS2ST(100));

  /*
   * Initialize LedDriver
   */
  ledstripe_init(GPIOB, GPIOB_PIN0);

  PRINT("\x1b[1J\x1b[0;0HStarting ChibiOS\r\n");
  PRINT("Start blinker thread ...");
  chThdCreateStatic(waThreadBlink, sizeof(waThreadBlink), NORMALPRIO,
      blinkerThread, NULL);
  PRINT(" Done\r\n");

  shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state, when the button is
   * pressed the test procedure is launched with output on the serial
   * driver 2.
   */
  while (TRUE)
    {
      usbcdc_process();
      if (palReadPad(GPIOA, GPIOA_BUTTON))
        {
          if (ledstripe_framebuffer[offset].red > 0
              && ledstripe_framebuffer[offset].green > 0
              && ledstripe_framebuffer[offset].blue > 0)
            {
              red = 255;
              green = 0;
              blue = 0;
            }
          else if (ledstripe_framebuffer[offset].red > 0)
            {
              red = 0;
              green = 255;
              blue = 0;
            }
          else if (ledstripe_framebuffer[offset].green > 0)
            {
              red = 0;
              green = 0;
              blue = 255;
            }
          else if (ledstripe_framebuffer[offset].blue > 0)
            {
              red = 0;
              green = 0;
              blue = 0;
            }
          else
            {
              red = green = blue = 255;
            }
          PRINT("Set %.2X%.2X%.2X (RRGGBB)\r\n", red, green, blue);

          /* Update the end of the stripe */
          for (i = offset; i < LEDSTRIPE_FRAMEBUFFER_SIZE; i++)
            {
              ledstripe_framebuffer[i].red = red;
              ledstripe_framebuffer[i].green = green;
              ledstripe_framebuffer[i].blue = blue;
            }
        }

      /* Wait some time, to make the scheduler running tasks with lower prio */
      chThdSleep(MS2ST(500));
    }
}

