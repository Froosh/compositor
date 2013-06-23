/*
 * Sing is the Processing-like framework for CloudLight procedural animation
 * After the Processing language by Tom Igoe
 * 
 * For the moment, the user procedural file is simply included
 * And everything is connected together at compile time
 * 
 * The user procedural file should define:
 *
 * void setup(void*)
 * void loop(void*)
 *
 * And leave everything else to this framework. Hopefully.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <time.h>

#define NUM_GLOBES 50

uint8_t txbuf[NUM_GLOBES*3];			// Global frame buffer
long sync_time = 0;
int pipe_fd;				// the global file descriptor for the FIFO

void synchronize(long since) {
struct timespec ts;
int retval;
long curr;
int waiting = 1;

	// synchronize will exit at least since nanoseconds after the last time sync exited
	// will try to be as close as possible, given the vagaries of multitasking
	// The first time through, it will exit immediately as it has not been initialized
	// So call synchronize(1) during initialization to set things up appropriately
	//
	while (1) {
		retval = clock_gettime(CLOCK_REALTIME, &ts);
		curr = ts.tv_nsec;
		if (ts.tv_nsec < sync_time) {		// Handle wraps gracefully
			curr += 1000000000;
		}
		//printf("curr %d sync_time %d diff %d\n", curr, sync_time, curr-sync_time);  
		if ((curr - sync_time) >= since) {
			sync_time = ts.tv_nsec;
			return;
		} else {
			usleep(100);		// Sleep for 200 microseconds
		} 			
	}
}

long started = 0;

long since_start(void) {
struct timespec ts;
int retval;
long curr;

	// Return the number of milliseconds since the process began execution
	// First call returns zero, but initializes values
	//
	retval = clock_gettime(CLOCK_REALTIME, &ts);
	curr = ts.tv_nsec / 1000000;	// Convert to msec
	curr = (ts.tv_sec * 1000) + curr;
	if (started == 0) {
		started = curr;
		return 0;
	} 
	return curr - started;
} 

#define LINE_SIZE 9			// 8 characters plus newline
#define NUM_LINES 52		// 52 lines in total

// Implementation of send_frame to be used with compositor
static void send_frame(int fd, unsigned long flags) {
char message[NUM_LINES * LINE_SIZE];		// Total buffer size
int k = 0;

	// Use a ginormous snprintf call to format the data for the compositor
	// First line is some flags -- unimplemented at present
	// And the next line is the name of the app sending data
	// And the next fifty lines are buffer data
	snprintf(message, NUM_LINES*LINE_SIZE, 
		"0x%06x\n%s\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n0x%02x%02x%02x\n",
		flags,		// All the flags
		"RANDCOMP",
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++],
		txbuf[k++], txbuf[k++], txbuf[k++] 
	);

	// And write the message out in one go
	write(fd, message, NUM_LINES*LINE_SIZE);
	//printf("%sEOM\n\n", message);
}

void set_pixel(int pixnum, uint8_t r, uint8_t g, uint8_t b) {
	int offset;

#ifdef PREPROTO
	// Make sure everything stays within the appropriate limits
	r = r >> 1;
	g = g >> 1;
	b = b >> 1;
#endif
	
	// Need to check for out-of-bounds here, but meh.
	//printf("col %d row %d offset %d\n", col, row, (*climap + col)[row]);
	// Calculate the offset into the buffer
	offset = pixnum * 3;		// Get the correct offset from the map
#ifdef PROTO
	txbuf[offset] = g;
	txbuf[offset+1] = r;		// GGRRBB
#else
	txbuf[offset] = r;
	txbuf[offset+1] = g;
#endif
	txbuf[offset+2] = b;

}
#include "rand.c"

int main(int argc, char *argv[])
{
	
#ifdef OLIMEX
	int retuid;
	retuid = setuid(0);     // root me!
    if (retuid == -1) {
        exit(-10);
    }
#endif

	// Open the pipe, write the data, close the pipe.
	// By convention, native code uses pipe 3
	pipe_fd = open("/run/pipelights.3.fifo", O_WRONLY);
	if (pipe_fd == -1) {
		printf("Failed to open pipe!\n");
		exit(-120);
	}

	since_start();	// initialize the count of when we started execution

	// Ok, we're ready to start, so call the setup
	setup();
	synchronize(1);	// And setup the sync function

	while (1) {
		//memset((void*) txbuf, (int) 0x80, (size_t) sp);  // Clear the string quickly
		loop(since_start());
		synchronize(800000000);					// 50 fps or 20 msec/frame
		send_frame(pipe_fd, 0L);				// Try to transmit the frame
	}
	close(pipe_fd);

	return 0;
}
