# List of all the board related files.
APPSRC = src/main.c \
		src/usbcdc/usbcdc.c \
		src/ledstripe/ledstripe.c \
		src/boblight/crc32.c \
		src/boblight/boblight.c \
		src/cmd/cmd_threads.c \
		src/cmd/cmd_ledstripe.c \
		src/cmd/cmd_reboot.c \
		src/cmd/cmd_mem.c 


# Required include directories
APPINC = ${APP}

# List all user C define here
APPDEFS =

