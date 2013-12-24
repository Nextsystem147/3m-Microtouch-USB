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
 *   inspired on: 
 *     http://github.com/superatrain/SPF-Panel-Driver
 *     http://thiemonge.org/getting-started-with-uinput
 *
 */

#include "opengalax2.h"

int main (int argc, char *argv[]) {

	unsigned char click;

	int x, y;
	int prev_x = 0;
	int prev_y = 0;

	int btn1_state = BTN1_RELEASE;
	int btn2_state = BTN2_RELEASE;

	int old_btn1_state = BTN1_RELEASE;
	int old_btn2_state = BTN2_RELEASE;
	int first_click = 0;

	int opt;
	int screen_width, screen_height;

	pid_t pid;
	//ssize_t res;

	struct input_event ev[2];
	struct input_event ev_button[4];
	struct input_event ev_sync;

	struct timeval tv_start_click;
	struct timeval tv_btn2_click;
	struct timeval tv_current;

	char foreground = 0;
	char displayVer = 0;
	char displayOpts = 0;

	conf = config_parse();
	screen_width = conf.screen_width;
	screen_height = conf.screen_height;

	while ((opt = getopt(argc, argv, "x:y:u:dfvh?")) != EOF) {
		switch (opt) {
			case 'v':
				displayVer = 1;
				break;
			case 'x':
				conf.screen_width=atoi(optarg);
				break;
			case 'y':
				conf.screen_height=atoi(optarg);
				break;
			case 'u':
				sprintf(conf.uinput_device, "%s", optarg);
				break;
			case 'd':
				displayOpts = 1;
				break;
			case 'f':
				foreground = 1;
				break;
			default:
				printf("%s v%s%s - %s\n", APPNAME, VERSION, EXTRAVERSION, AUTHOR);
				printf("Usage: %s [options]\n", file_basename(argv[0]));
				printf("        -x number            : override display width from configuration\n");
				printf("        -y number            : override display height from configuration\n");
				printf("        -u <uinput-device>   : default=%s\n", conf.uinput_device);
				printf("	-v                   : display version and exit\n");
				printf("        -d                   : display configuration\n");
				printf("        -f                   : run in foreground (do not daemonize)\n\n");
				exit(1);
                                break;
		}
	}

	if (( !displayVer ) && ( !displayOpts ) && ( !foreground )) {
		printf("%s v%s ", APPNAME, VERSION);
		fflush(stdout);
	} else printf("%s v%s%s - %s\n", APPNAME, VERSION, EXTRAVERSION, AUTHOR);

	if ( displayVer )
		exit(1);
		
	if (( displayOpts ) || ( foreground )) {
		printf ("\nConfiguration data:\n");
		printf ("\tuinput_device=%s\n",conf.uinput_device);
		printf ("\trightclick_enable=%d\n",conf.rightclick_enable);
		printf ("\trightclick_duration=%d\n",conf.rightclick_duration);
		printf ("\trightclick_range=%d\n",conf.rightclick_range);
		printf ("\tdirection=%d\n",conf.direction);
		printf ("\tscreen_width=%d\n", conf.screen_width);
		printf ("\tscreen_height=%d\n\n", conf.screen_height);
	}

	if (!running_as_root()) {
		fprintf(stderr,"this program must be run as root user\n");
		exit (-1);
	}

	if (!foreground) {
		if ((pid = fork()) < 0)
			exit(1);
		else
			if (pid != 0)
				exit(0);

		/* daemon running here */
		setsid();
		if (chdir("/") != 0) 
			die("Could not chdir");
		umask(0);
		printf("forked into background\n");
	} else
		printf("\n");

	/* create pid file */
	if (!create_pid_file())
		exit(-1);

	initialize_panel(0);

	// configure uinput
	setup_uinput_dev(screen_width, screen_height);

	// handle signals
	signal_installer();

	// input sync signal:
	memset (&ev_sync, 0, sizeof (struct input_event));
	ev_sync.type = EV_SYN;
	ev_sync.code = 0;
	ev_sync.value = 0;

	// button press signals:
	memset (&ev_button, 0, sizeof (ev_button));
	ev_button[BTN1_RELEASE].type = EV_KEY;
	ev_button[BTN1_RELEASE].code = BTN_LEFT;
	ev_button[BTN1_RELEASE].value = 0;
	ev_button[BTN1_PRESS].type = EV_KEY;
	ev_button[BTN1_PRESS].code = BTN_LEFT;
	ev_button[BTN1_PRESS].value = 1;
	ev_button[BTN2_RELEASE].type = EV_KEY;
	ev_button[BTN2_RELEASE].code = BTN_RIGHT;
	ev_button[BTN2_RELEASE].value = 0;
	ev_button[BTN2_PRESS].type = EV_KEY;
	ev_button[BTN2_PRESS].code = BTN_RIGHT;
	ev_button[BTN2_PRESS].value = 1;

	if (foreground)
		printf("panel initialized\n");

	// main bucle
	while (1) {

		struct ts_sample samp;
		int ret = ts_read(ts, &samp, 1);

		if (ret < 0) {                        
			perror("ts_read");
			signal_handler(0);
		}

		if (ret != 1)                        
			continue;      

		// Should have select timeout, because finger down garantees many results..
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		click = samp.pressure == 0 ? RELEASE : PRESS;
		x = samp.x;
		y = samp.y;

		old_btn1_state = btn1_state;
		old_btn2_state = btn2_state;

		switch (click) {
			case PRESS:
				if (old_btn1_state == BTN1_RELEASE && old_btn2_state == BTN2_RELEASE) {
					btn1_state = BTN1_PRESS;
					btn2_state = BTN2_RELEASE;
				}
				break;
			case RELEASE:
				btn1_state = BTN1_RELEASE;
				btn2_state = BTN2_RELEASE;
				break;
		}

		// If this is the first panel event, track time for no-drag timer
		first_click = 0;
		if (old_btn1_state == BTN1_RELEASE && btn1_state == BTN1_PRESS) {
			first_click = 1;
			gettimeofday (&tv_start_click, NULL);
			gettimeofday (&tv_btn2_click, NULL);
		}

		// load X,Y into input_events
		memset (ev, 0, sizeof (ev));
		ev[0].type = EV_ABS;
		ev[0].code = ABS_X;
		ev[0].value = x;
		ev[1].type = EV_ABS;
		ev[1].code = ABS_Y;
		ev[1].value = y;

		gettimeofday (&tv_current, NULL);

		// Only move to posision of click for first while - prevents accidental dragging.
		if (time_elapsed_ms (&tv_start_click, &tv_current, 200) || first_click) {
			// send X,Y
			if (write (fd_uinput, &ev[0], sizeof (struct input_event)) < 0)
				die ("error: write");
			if (write (fd_uinput, &ev[1], sizeof (struct input_event)) < 0)
				die ("error: write");
		} else {
			// store position for right click management
			prev_x = x;
			prev_y = y;
		}

		if (conf.rightclick_enable) {

			// emulate right click by press and hold
			if (time_elapsed_ms (&tv_btn2_click, &tv_current, conf.rightclick_duration)) {
				if (( x-(conf.rightclick_range/2) < prev_x && prev_x < x+(conf.rightclick_range/2) ) &&
					( y-(conf.rightclick_range/2) < prev_y && prev_y < y+(conf.rightclick_range/2) ) ) {
					btn2_state=BTN2_PRESS;
					btn1_state=BTN1_RELEASE;
				}
			}

			// reset the start click counter and store position (allows select text + rightclick)
			if (time_elapsed_ms (&tv_btn2_click, &tv_current, conf.rightclick_duration*2) && btn2_state == BTN2_RELEASE) {
				gettimeofday (&tv_btn2_click, NULL);
				prev_x = x;
				prev_y = y;
			}

			// force button2 transition
			if (old_btn2_state == BTN2_RELEASE && btn2_state == BTN2_PRESS) {
				if (write(fd_uinput, &ev_button[BTN1_RELEASE], sizeof (struct input_event)) < 0)
					die ("error: write");
				if (write(fd_uinput, &ev_button[BTN2_RELEASE], sizeof (struct input_event)) < 0)
					die ("error: write");
				if (write (fd_uinput, &ev_sync, sizeof (struct input_event)) < 0)
					die ("error: write");
				if (foreground)
					printf ("X: %d Y: %d BTN1: OFF BTN2: OFF FIRST: %s\n", x, y,
					first_click == 0 ? "No" : first_click == 1 ? "Yes" : "Unknown");

				usleep (10000);

				if (write(fd_uinput, &ev_button[BTN1_RELEASE], sizeof (struct input_event)) < 0)
					die ("error: write");
				if (write(fd_uinput, &ev_button[BTN2_PRESS], sizeof (struct input_event)) < 0)
					die ("error: write");
				if (write (fd_uinput, &ev_sync, sizeof (struct input_event)) < 0)
					die ("error: write");
				if (foreground)
					printf ("X: %d Y: %d BTN1: OFF BTN2: ON  FIRST: %s\n", x, y,
					first_click == 0 ? "No" : first_click == 1 ? "Yes" : "Unknown");
			}

			// clicking button2
			if (write(fd_uinput, &ev_button[btn2_state], sizeof (struct input_event)) < 0)
				die ("error: write");
		}

		// clicking button1
		if (write(fd_uinput, &ev_button[btn1_state], sizeof (struct input_event)) < 0)
			die ("error: write");

		// Sync
		if (write (fd_uinput, &ev_sync, sizeof (struct input_event)) < 0)
			die ("error: write");

		if (foreground)
			printf ("X: %d Y: %d BTN1: %s BTN2: %s FIRST: %s\n", x, y,
				btn1_state == BTN1_RELEASE ? "OFF" : btn1_state == BTN1_PRESS ? "ON " : "Unknown",
				btn2_state == BTN2_RELEASE ? "OFF" : btn2_state == BTN2_PRESS ? "ON " : "Unknown",
				first_click == 0 ? "No" : first_click == 1 ? "Yes" : "Unknown");
	}

	return 0;
}
