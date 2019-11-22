/*
 * Copyright 2010 Michael Ossmann
 *
 * This file is part of Project Ubertooth.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <getopt.h>
#include <stdlib.h>
#include "ubertooth.h"
#include <time.h>

uint8_t debug;
int rssi_offset = -54, isOk=0, rssi100=0;
double first=0;
long second=0, diff2;
int j=0;
struct timeval startTime, endTime, gepTime;

void cb_specan(ubertooth_t* ut __attribute__((unused)), void* args)
{
	uint16_t high_freq = (((uint8_t*)args)[0]) |
	                     (((uint8_t*)args)[1] << 8);
	uint8_t output_mode = ((uint8_t*)args)[2];

	usb_pkt_rx rx = fifo_pop(ut->fifo);
	int r;
	uint16_t frequency;
	int8_t rssi, rssi_result;
	double diff, time_present;
	
	gettimeofday( &startTime, NULL );
	/* process each received block */
	// j = 0 2439
	// j = 3 2442


    if(j == 0) j = 3;
    else j = 0;

    frequency = (rx.data[j] << 8) | rx.data[j + 1];
    rssi = (int8_t)rx.data[j + 2];
    rssi_result = rssi + rssi_offset;

    if(rssi100 > 0){
        if(rssi100 == 100) {
            rssi100 = 0;
            isOk = 0;
            printf("-\n");
        }
        else isOk = 1;
    }
    else{
        if(rssi_result > -75) isOk = 1;
    }
    if (isOk > 0){
        time_present = (double)rx.clk100ns/10000000;

        //printf("Present: %f\n", time_present);
        diff = time_present-first;
        first = time_present;

        //printf("The number of rssi: %d\n", rssi100);
        //printf("%f, %d, %d\n\n", diff, frequency, rssi_result);
        printf("%f, %d, %d\n", time_present, frequency, rssi_result);
        ++rssi100;
	}
	gettimeofday( &endTime, NULL );
	gepTime.tv_sec = endTime.tv_sec - startTime.tv_sec;
	gepTime.tv_usec = endTime.tv_usec - startTime.tv_usec;

	if ( gepTime.tv_usec < 0 ) {
		gepTime.tv_sec = gepTime.tv_sec - 1;
		gepTime.tv_usec = gepTime.tv_usec + 1000000;
	}
	//printf("ellapsed time [%ld] second\n", gepTime.tv_usec);

	fflush(stderr);
}

static void usage(FILE *file)
{
	fprintf(file, "ubertooth-specan - output a continuous stream of signal strengths\n");
	fprintf(file, "\n");
	fprintf(file, "!!!!!\n");
	fprintf(file, "NOTE: you probably want ubertooth-specan-ui\n");
	fprintf(file, "!!!!!\n");
	fprintf(file, "\n");
	fprintf(file, "Usage:\n");
	fprintf(file, "\t-h this help\n");
	fprintf(file, "\t-v verbose (print debug information to stderr)\n");
	fprintf(file, "\t-g output suitable for feedgnuplot\n");
	fprintf(file, "\t-G output suitable for 3D feedgnuplot\n");
	fprintf(file, "\t-d <filename> output to file\n");
	fprintf(file, "\t-l lower frequency (default 2402)\n");
	fprintf(file, "\t-u upper frequency (default 2480)\n");
	fprintf(file, "\t-U<0-7> set ubertooth device to use\n");
}

int main(int argc, char *argv[])
{
	int opt, r = 0, output_mode = SPECAN_STDOUT;
	int lower= 2402, upper= 2480;
	int ubertooth_device = -1;

	ubertooth_t* ut = NULL;

	while ((opt=getopt(argc,argv,"vhgGd:l::u::U:")) != EOF) {
		switch(opt) {
		case 'v':
			debug++;
			break;
		case 'g':
			output_mode = SPECAN_GNUPLOT_NORMAL;
			break;
		case 'G':
			output_mode = SPECAN_GNUPLOT_3D;
			break;
		case 'd':
			output_mode = SPECAN_FILE;
			if(*optarg == '-') {
				dumpfile = stdout;
			} else {
				dumpfile = fopen(optarg, "w");
				if (dumpfile == NULL) {
					perror(optarg);
					return 1;
				}
			}
			break;
		case 'l':
			if (optarg)
				lower= atoi(optarg);
			else
				printf("lower: %d\n", lower);
			break;
		case 'u':
			if (optarg)
				upper= atoi(optarg);
			else
				printf("upper: %d\n", upper);
			break;
		case 'U':
			ubertooth_device = atoi(optarg);
			break;
		case 'h':
			usage(stdout);
			return 0;
		default:
			usage(stderr);
			return 1;
		}
	}

	ut = ubertooth_start(ubertooth_device);

	if (ut == NULL) {
		usage(stderr);
		return 1;
	}

	r = ubertooth_check_api(ut);
	if (r < 0)
		return 1;

	/* Clean up on exit. */
	register_cleanup_handler(ut, 0);

	uint8_t specan_args[] = {
		(uint8_t)(upper & 0xff),
		(uint8_t)(upper >> 8),
		output_mode
	};

	// init USB transfer
	r = ubertooth_bulk_init(ut);
	if (r < 0)
		return r;

	r = ubertooth_bulk_thread_start();
	if (r < 0)
		return r;

	// tell ubertooth to start specan and send packets
	r = cmd_specan(ut->devh, 2439, 2442);
	if (r < 0)
		return r;

	// receive and process each packet
	while(!ut->stop_ubertooth) {
		ubertooth_bulk_receive(ut, cb_specan, specan_args);
	}

	ubertooth_bulk_thread_stop();

	ubertooth_stop(ut);
	fprintf(stderr, "Ubertooth stopped\n");
	return r;
}
