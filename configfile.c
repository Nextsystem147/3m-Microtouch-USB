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

#include "opengalax2.h"

#define CONFIG_FILE "/etc/opengalax2.conf"
#define MAXLEN 1024

static const conf_data default_config = {
	/* uinput_device */ "/dev/uinput",
	/* rightclick_enable */	0,
	/* rightclick_duration */ 350,
	/* rightclick_range */ 10,
	/* direction */ 0,
	/* screen width */ 800,
	/* screen height */ 480,
};

int create_config_file (char* file) {
	FILE* fd;

	fd = fopen(file, "w");
	if (fd == NULL)
		return 0;

	fprintf(fd, "# opengalax2 configuration file\n");
	fprintf(fd, "\n#### config data:\n");
	fprintf(fd, "uinput_device=%s\n", default_config.uinput_device);
	fprintf(fd, "rightclick_enable=%d\n", default_config.rightclick_enable);
	fprintf(fd, "rightclick_duration=%d\n", default_config.rightclick_duration);
	fprintf(fd, "rightclick_range=%d\n", default_config.rightclick_range);
	fprintf(fd, "# direction: 0 = normal, 1 = invert X, 2 = invert Y, 4 = swap X with Y\n");
	fprintf(fd, "direction=%d\n", default_config.direction);
	fprintf(fd, "screen_width=%d\n", default_config.screen_width);
	fprintf(fd, "screen_height=%d\n", default_config.screen_height);
	fprintf(fd, "\n");

	fclose(fd);
	return 1;
}

conf_data config_parse (void) {

	char file[MAXLEN];
	char input[MAXLEN], temp[MAXLEN];
	FILE *fd;
	size_t len;
	conf_data config = default_config;

	sprintf( file, "%s", CONFIG_FILE);
	if (!file_exists(file)) {
		if (!running_as_root())
			return config;
		if (!create_config_file(file)) {
			fprintf (stderr,"Failed to create default config file: %s\n", file);
			exit (1);
		}
	}

	fd = fopen (file, "r");
	if (fd == NULL) {
		fprintf (stderr,"Could not open configuration file: %s\n", file);
		exit (1);
	}

	while ((fgets (input, sizeof (input), fd)) != NULL) {

		if ((strncmp ("uinput_device=", input, 14)) == 0) {
			strncpy (temp, input + 14,MAXLEN-1);
			len=strlen(temp);
			temp[len-1]='\0';
			sprintf(config.uinput_device, "%s", temp);
			memset (temp, 0, sizeof (temp));
		}

		if ((strncmp ("rightclick_enable=", input, 18)) == 0) {
			strncpy (temp, input + 18,MAXLEN-1);
			len=strlen(temp);
			temp[len+1]='\0';
			config.rightclick_enable = atoi(temp);
		}

		if ((strncmp ("rightclick_duration=", input, 20)) == 0) {
			strncpy (temp, input + 20,MAXLEN-1);
			len=strlen(temp);
			temp[len+1]='\0';
			config.rightclick_duration = atoi(temp);
		}

		if ((strncmp ("rightclick_range=", input, 17)) == 0) {
			strncpy (temp, input + 17,MAXLEN-1);
			len=strlen(temp);
			temp[len+1]='\0';
			config.rightclick_range = atoi(temp);
		}

		if ((strncmp ("direction=", input, 10)) == 0) {
			strncpy (temp, input + 10,MAXLEN-1);
			len=strlen(temp);
			temp[len+1]='\0';
			config.direction = atoi(temp);
		}

		if ((strncmp ("screen_width=", input, 13)) == 0) {
			strncpy (temp, input + 13, MAXLEN-1);
			len=strlen(temp);
			temp[len+1]='\0';
			config.screen_width = atoi(temp);
		}

		if ((strncmp ("screen_height=", input, 14)) == 0) {
			strncpy (temp, input + 14, MAXLEN-1);
			len=strlen(temp);
			temp[len+1]='\0';
			config.screen_height = atoi(temp);
		}
	}

	fclose(fd);
	return config;
}
