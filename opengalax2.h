/*
 *   opengalax2 touchscreen daemon utilizing tslib
 *   Copyright 2013 Oskari Rauta <oskari.rauta@gmail.com>
 *
 *   Fork of opengalax driver by Pau Oliva Fora form:
 *   https://github.com/poliva/opengalax
 *
 *   Original header:
 *   opengalax touchscreen daemon
 *   Copyright 2012 Pau Oliva Fora <pof@eslack.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 *
 */
#ifndef __OPENGALAX2_H
#define __OPENGALAX2_H

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <linux/uinput.h>
#include <sys/stat.h>
#include <tslib.h>

#define XA_MAX 0xF
#define YA_MAX 0xF

#define XB_MAX	0x7F
#define YB_MAX	0x7F

#define X_AXIS_MAX (XA_MAX+1)*(XB_MAX+1)
#define Y_AXIS_MAX (YA_MAX+1)*(YB_MAX+1)

#define CMD_OK 0xFA
#define CMD_ERR 0xFE

#define PRESS 0x81
#define RELEASE 0x80

#define BTN1_RELEASE 0
#define BTN1_PRESS 1
#define BTN2_RELEASE 2
#define BTN2_PRESS 3
#define BTN1_TOUCH 4

#define DEBUG 0

#define die(str, args...) do { \
	perror(str); \
	exit(EXIT_FAILURE); \
} while(0)

/* config file */
typedef struct {
	char uinput_device[1024];
	int rightclick_enable;
	int rightclick_duration;
	int rightclick_range;
	int direction;
	int screen_width;
	int screen_height;
} conf_data;

extern struct tsdev *ts;
extern int fd_uinput;
extern  struct uinput_user_dev uidev;
extern  conf_data conf;
extern  struct timeval tv;

/* configfile.c */
	int create_config_file (char* file);
	conf_data config_parse (void);

/* functions.c */
	int running_as_root (void);
	int time_elapsed_ms (struct timeval *start, struct timeval *end, int ms);
	int setup_uinput_dev (int screen_width, int screen_height);
	void signal_handler (int sig);
	void bindToGalax (void);
	void initialize_panel (int sig);
	void signal_installer (void);
	int file_exists (char *file);
	char* default_pid_file (void); 
	int create_pid_file (void); 
	int remove_pid_file (void);
	char *file_basename(char *path);

/*

init sequence:
 	send: f5 f3 0a f3 64 f3 c8 f4
 	read: fa fa fa fa fa fa fa fa

return values:
	0xFA == OK
	0xFE == ERROR

Example PDUs:
	81 0F 3D 05 06
	80 0F 3D 05 06

byte 0:
	0x80 == PRESS
	0x81 == RELEASE

byte 1:
	X axis value, from 0 to 0x0F
byte 2:
	X axis value, from 0 to 0x7F

byte 3:
	Y axis value, from 0 to 0x0F
byte 4:
	Y axis value, from 0 to 0x7F

*/


#endif  /* __OPENGALAX2_H */
