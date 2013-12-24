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

#define DEFAULT_PID_FILE "/var/run/opengalax2.pid"
#define MAX_KEYWORD_LEN 256
#define MAX_PARAM_LEN 48
#define GALAX_DEVICE_NAME "N: Name=\"eGalax Inc. USB TouchController\""

#include "opengalax2.h"

int running_as_root (void) {
	uid_t uid, euid;	
	uid = getuid();
	euid = geteuid();
	if (uid != 0)
		return 0;
	if (euid != 0)
		return 0;
	return 1;
}

int time_elapsed_ms (struct timeval *start, struct timeval *end, int ms) {
	int difference = (end->tv_usec + end->tv_sec * 1000000) - (start->tv_usec + start->tv_sec * 1000000);
	if (difference > ms * 1000)
		return 1;
	return 0;
}

int setup_uinput_dev (int screen_width, int screen_height) {
	fd_uinput = open (conf.uinput_device, O_WRONLY | O_NONBLOCK);
	if (fd_uinput < 0)
		die ("error: uinput");

	if (ioctl (fd_uinput, UI_SET_EVBIT, EV_KEY) < 0)
		die ("error: ioctl");

	if (ioctl (fd_uinput, UI_SET_KEYBIT, BTN_LEFT) < 0)
		die ("error: ioctl");

	if (ioctl (fd_uinput, UI_SET_KEYBIT, BTN_RIGHT) < 0)
		die ("error: ioctl");

	if (ioctl (fd_uinput, UI_SET_EVBIT, EV_ABS) < 0)
		die ("error: ioctl");

	if (ioctl (fd_uinput, UI_SET_ABSBIT, ABS_X) < 0)
		die ("error: ioctl");

	if (ioctl (fd_uinput, UI_SET_ABSBIT, ABS_Y) < 0)
		die ("error: ioctl");

	memset (&uidev, 0, sizeof (uidev));
	snprintf (uidev.name, UINPUT_MAX_NAME_SIZE, "opengalax");
	uidev.id.bustype = BUS_VIRTUAL;
	uidev.id.vendor = 0xeef;
	uidev.id.product = 0x1;
	uidev.id.version = 1;

	uidev.absmin[ABS_X] = 0;
	uidev.absmax[ABS_X] = screen_width - 1;
	uidev.absmin[ABS_Y] = 0;
	uidev.absmax[ABS_Y] = screen_height - 1;

	if (write (fd_uinput, &uidev, sizeof (uidev)) < 0)
		die ("error: write");

	if (ioctl (fd_uinput, UI_DEV_CREATE) < 0)
		die ("error: ioctl");

	return 0;
}

void signal_handler (int sig) {
	(void) sig;
	remove_pid_file();

	if (ioctl (fd_uinput, UI_DEV_DESTROY) < 0)
		die ("error: ioctl");

	close (fd_uinput);
	exit(1);
}

void bindToGalax (void) {
	FILE *fp = NULL;
	char keyword[MAX_KEYWORD_LEN];
	char param[MAX_PARAM_LEN];
	unsigned char thisDev = 0;
	int ch, devFound = 0;
	unsigned int i = 0;

	//if ( ts )
	//	return;

	fp = fopen("/proc/bus/input/devices", "r");
    
	if (!fp) {
		int err = errno;
		fprintf(stderr, "error: Unable to open file /proc/bus/input/devices.\r\nError: %s\n", strerror(err));
		exit(-1);
	}

	while ( !feof(fp) ) {

		ch = fgetc(fp);
		if ( ch == 'N' ) {
			memset(keyword, 0, sizeof(keyword));
			i = 0;
			while (( !feof(fp)) && ( ch != '\n' )) {
				if ( i < sizeof(keyword) - 2 )
					keyword[i++] = ch;
				ch = fgetc(fp);
			}
                
			if ( strcmp(keyword, GALAX_DEVICE_NAME) == 0 ) {
				devFound = 1;
				thisDev = 1;
			} else thisDev = 0;
		}

		if (( ch == 'H' ) && ( thisDev )) {
			memset(keyword, 0, sizeof(keyword));
			i = 0;
			while (( !feof(fp)) && ( ch != '\n')) {
				if ( i < sizeof(keyword) - 2 )
					keyword[i++] = ch;
				ch = fgetc(fp);
			}

			if ( strncmp(keyword, "H: Handlers=", 12) == 0 ) {
				int lastParam = 0;
				int paramEnd, paramBegin = 12;

				while ( !lastParam ) {
					memset(param, 0, sizeof(param));
					paramEnd = 0;

					while (( keyword[paramBegin + paramEnd] != ' ') && ( keyword[paramBegin + paramEnd] != 0 ))
						paramEnd++;

					if (( keyword[paramBegin + paramEnd] == 0 ) || ( keyword[paramBegin + paramEnd + 1] == 0 ))
						lastParam = 1;

					strncpy(param, keyword+paramBegin, paramEnd);
					if ( strncmp(param, "event", 5) == 0 ) {
						fclose(fp);
						char eventPath[16 + paramEnd];
						sprintf(eventPath, "/dev/input/%s", param);
						ts = ts_open(eventPath, 0);

						printf("Binding tslib to %s\n", eventPath);
						return;
					}
					paramBegin += paramEnd + 1;
				}
			}
		}
		while (( !feof(fp)) && ( ch != '\n' ))
			ch = fgetc(fp);
	}

	fclose(fp);

	if ( devFound )
		fprintf(stderr, "error: Unable to parse event handler for eGalax Touchscreen from /proc/bus/input/devices.\nEvent handler missing, or file format cannot be parsed.\n");
	else
		fprintf(stderr, "error: Unable find eGalax Touchscreen from /proc/bus/input/devices.\nMissing %s\n", GALAX_DEVICE_NAME);
	exit(-1);
}

void initialize_panel (int sig) {
	(void) sig;
	bindToGalax();

	if (!ts) {
		fprintf(stderr, "error: ts_open\n");
		remove_pid_file();

		if (ioctl (fd_uinput, UI_DEV_DESTROY) < 0)
			die ("error: ioctl");

		close (fd_uinput);
		exit(-1);
	}

	if (ts_config(ts)) {
		fprintf(stderr, "error: ts_config\n");
		remove_pid_file();

		if (ioctl (fd_uinput, UI_DEV_DESTROY) < 0)
			die ("error: ioctl");

		close (fd_uinput);
		exit(-1);
	}
}

void signal_installer (void) {
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGCHLD, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGUSR1, initialize_panel);
}

int file_exists (char *file) {
	struct stat buf;
	if (stat(file, &buf) == 0)
		return 1;
	return 0;
}

char* default_pid_file (void) {
	char* file = malloc(strlen(DEFAULT_PID_FILE)+2);
	sprintf(file, "%s", DEFAULT_PID_FILE);
	return file;
}

int create_pid_file (void) {
	int fd;
	char *pidfile;
	char buf[100];
	ssize_t cnt;
	char* procpid = malloc( sizeof(buf) + 15 );

	pidfile = default_pid_file();

	if (file_exists(pidfile)) {

		// check if /proc/{pid}/cmdline exists and contains opengalax
		// if it does, means opengalax is already running, so we exit cowardly
		// if it does not contain opengalax, then we remove the old pid file and continue

		fd = open(pidfile, O_RDONLY);
		if (fd < 0) {
			fprintf (stderr,"Could not open pid file: %s\n", pidfile);
			return 0;
		}
		cnt=read(fd, buf, sizeof(buf)-1);
		buf[cnt]='\0';
		
		close(fd);

		strcpy(procpid, "");
		strcat(procpid, "/proc/");
		strcat(procpid, buf);
		strcat(procpid, "/cmdline");

		if (file_exists(procpid)) {
			fd = open(procpid, O_RDONLY);
			if (fd < 0) {
				fprintf (stderr,"Could not open file: %s\n", procpid);
				return 0;
			}

			cnt=read(fd, buf, sizeof(buf)-1);
			buf[cnt]='\0';
			
			close(fd);

			if (strstr(buf,"opengalax") != NULL) {
				fprintf (stderr,"Refusing to start as another instance is already running\n");
				return 0;
			} else {
				if (!remove_pid_file()) 
					return 0;
			}
		}
	}

	fd = open(pidfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0 ) {
		fprintf(stderr,"Could not write pid file: %s\n", pidfile);
		return 0;
	}

	sprintf( buf, "%d", getpid() );
	if (write(fd, buf, strlen(buf)) < 1) {
		perror("Something wrong happening while writing pid file");
		close(fd);
		return 0;
	}
	close(fd);

	free(procpid);

	return 1;
}

int remove_pid_file (void) {

	char *pidfile;

	pidfile = default_pid_file();

	if (!file_exists(pidfile)) {
		fprintf (stderr,"pid file does not exist: %s\n", pidfile);
		return 1;
	}

	if (unlink(pidfile) != 0) {
		fprintf (stderr,"Could not delete pid file: %s\n", pidfile);
		return 0;
	}
	return 1;
}

char *file_basename(char *path)
{
    char *base = strrchr(path, '/');
    return base ? base+1 : path;
}
