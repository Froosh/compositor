/*
 * The compositor opens a series of pipes 
 * These can be used by processes that wish to write to the Holiday bulbs
 * The processes can send a message that indicates how to write to the bulbs
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/types.h>
#include <time.h>
#include <string.h>

#include "spi.h"
#include "compositor.h"

#define NUM_PIPES 4
#define LINE_SIZE 9			// 8 characters plus newline
#define MESSAGE_SIZE ((NUM_BULBS + 2) * LINE_SIZE)

// Bit flags for pipe_data.flags
#define NEW_DATA_FLAG   0b0000000000000001		// True if this data is freshly received from pipe
#define REPLACE_FLAG	0b0000000000000010		// if set replace buffer, otherwise mix
#define MASK_FLAG		0b0000000000000100		// if set non-zero bytes are masked in
#define HAS_DATA_FLAG	0b0000000000001000		// Set if there is data to render
#define BRIGHTNESS_FLAG 0b1000000000000000		// Set the brightness in the following 7 bits
#define BRIGHTNESS_MASK 0b0111111100000000		// These are the bits used to determine brightness


uint8_t txbuf[NUM_BULBS * 3];	// Global frame buffer, allocate once
int string_size = NUM_BULBS;	

int pipe_fd[NUM_PIPES];				// Array of pipe file descriptors

struct pipe_data {
	unsigned long flags;			// Various flags to control things and stuff
	char name[LINE_SIZE];			// Name of the process or app sending this data
	uint8_t rgbbuf[(NUM_BULBS*3)];	// Storage for the bulb RGB values
};

struct pipe_data pipebuf[NUM_PIPES];	// Storage for the pipe buffers

// Open the named pipe as given, return file descriptor to it
// If the returned value is less than zero, it failed
int open_pipe(int index) {
char name[255];
int fd, err;

	// Make the name from the index
	sprintf(name, "/run/pipelights.%0d.fifo", index);

	// Create the named pipe
	err = mkfifo(name, 0666);
	if (err != 0) {
		return(-100);
	}

	// Set permissions on named pipe so everyone can write to it
	err = chmod(name, 0666);
	if (err != 0) {
		return(-101);
	}

	// Open the pipe in a non-blocking way and return a descriptor thingy
	fd = open(name, O_RDONLY | O_NONBLOCK);
	return fd; 			// return the file descriptor
}

int check_pipe(int index) {
int avail;
int err;

	// Does a pipe have data available?
	// Is there some way we can know this?
	// I sure hope so or this ain't gonna work
	err = ioctl(pipe_fd[index], FIONREAD, &avail);
	if (err == -1) {
		return -1;
	}
	//printf("Pipe %d has %d bytes to read\n", index, avail);
	return avail;
}

int read_pipe(int index) {
int j, k;
char read_message[MESSAGE_SIZE];
ssize_t bytes_read;
unsigned long rawcolor;
uint8_t r, g, b;
int ln = 0;

	// Read an entire message in from the pipe and store it in the appropriate array thingy
	// It should be in 52 lines. We hope.
	bytes_read = read(pipe_fd[index], read_message, (size_t) MESSAGE_SIZE);
	if (bytes_read < MESSAGE_SIZE) {
		printf("Message too short!\n");
	}
	//printf("%sEOM\n", read_message);

	// The first line is flags, we're just kind of glossing this for now.
	//bytes_read = read(pipe_fd[index], read_string, (size_t) LINE_SIZE);
	//read_string[bytes_read] = 0x00;		// Null terminate end of string
	pipebuf[index].flags = strtoll(&read_message[(ln++ * LINE_SIZE)], NULL, 16);
	pipebuf[index].flags |= (NEW_DATA_FLAG + HAS_DATA_FLAG);			// Set the new data flag
	
	// The next line is the name of the sending process, store it too
	strncpy(pipebuf[index].name, &read_message[(ln++ * LINE_SIZE)], LINE_SIZE-1);
	pipebuf[index].name[LINE_SIZE] = 0x00;		// Null terminate string	

	// And the next fifty lines are bulb values.  No, really.
	for(j = 0, k = 0; j < NUM_BULBS; j++) {

		rawcolor = strtol(&read_message[(ln++ * LINE_SIZE)], NULL, 16);
		r = (rawcolor >> 16) & 0xFF;
 		g = (rawcolor >> 8) & 0xFF;
 		b = rawcolor & 0xFF;

		// Here we'll do something with the bytes we've read in.
		pipebuf[index].rgbbuf[k++] = r;
		pipebuf[index].rgbbuf[k++] = g;
		pipebuf[index].rgbbuf[k++] = b;
		//printf("Bulb %d has values %02X %02X %02X\n", j, r, g, b);
	}
	pipebuf[index].rgbbuf[k++] = 0x00;		// And null last 3 bytes
	pipebuf[index].rgbbuf[k++] = 0x00;
	pipebuf[index].rgbbuf[k++] = 0x00;

	//printf("Everything read in\n");
}

// The renderer is the heart of the compositor.
// It will mix the buffers together IF the flags are set correctly
// Otherwise it might do other sorts of things. Or something.
void render() {
int j, q, k;
unsigned int ra[NUM_BULBS][3];		// The mixer array thingy
int mixed = 0;						// Number mixed

	// First, zero the mixed array
	for (j = 0; j < NUM_BULBS; j++) {
		ra[j][0] = ra[j][1] = ra[j][2] = 0;
	}

	// Now add in each of the arrays
	// For the moment we're not doing any masking, just adding
	for (j = 0; j < NUM_PIPES; j++) {
		if (pipebuf[j].flags & HAS_DATA_FLAG) {	// buffer has data?

			// Add the data into the render array
			for (q = 0, k = 0; q < NUM_BULBS; q++) {
				ra[q][0] += pipebuf[j].rgbbuf[k++];
				ra[q][1] += pipebuf[j].rgbbuf[k++];
				ra[q][2] += pipebuf[j].rgbbuf[k++];
			}

			if (pipebuf[j].flags & NEW_DATA_FLAG) {	
				pipebuf[j].flags ^= NEW_DATA_FLAG;		// Unset the flag
			}
			mixed++;		// Increment the number of buffers mixed together
		}
	}

	// At this point all the buffers should be mixed.
	// Now we need to divide them by the number we mixed together
	// This is only computationally expensive if we have 3 buffers
	switch (mixed) {
		case 4:
			for (j = 0, k = 0; j < NUM_BULBS; j++) {
				txbuf[k++] = ra[j][0] >> 2;		// Divide by 4
				txbuf[k++] = ra[j][1] >> 2;
				txbuf[k++] = ra[j][2] >> 2;
			}
			break;

		case 3:
			for (j = 0, k = 0; j < NUM_BULBS; j++) {
				txbuf[k++] = ra[j][0] / 3;		// Divide by 3
				txbuf[k++] = ra[j][1] / 3;
				txbuf[k++] = ra[j][2] / 3;
			}
			break;

		case 2:
			for (j = 0, k = 0; j < NUM_BULBS; j++) {
				txbuf[k++] = ra[j][0] >> 1;		// Divide by 2
				txbuf[k++] = ra[j][1] >> 1;
				txbuf[k++] = ra[j][2] >> 1;
			}
			break;

		case 1:
			for (j = 0, k = 0; j < NUM_BULBS; j++) {
				txbuf[k++] = ra[j][0];		// Straight copy
				txbuf[k++] = ra[j][1];
				txbuf[k++] = ra[j][2];
			}
			break;

		case 0:
		default:
			for (j = 0, k = 0; j < NUM_BULBS; j++) {
				txbuf[k++] = 0;		// Clear output array
				txbuf[k++] = 0;
				txbuf[k++] = 0;
			}
			break;
	}

	// And send it on its merry way
	spi_send(txbuf, (NUM_BULBS*3));		// Just for testing purposes
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

int main(int argc, char *argv[])
{
	int j;
	int retuid;
	int doing = 1;			// While running, set true
	int avail = 0;			// Bytes available to read

	// Try to get some privleges here
	retuid = setuid(0);		// root me!
	if (retuid == -1) {
		perror("Could not setuid");
		exit(-10);
	}

	spi_open();

	// Open ALL THE PIPES
	for (j =0; j < NUM_PIPES; j++) {
		printf("Opening pipe %d\n", j);
		pipe_fd[j] = open_pipe(j);
		if (pipe_fd[j] < 0) {
			printf("Failed, exiting with %d\n", pipe_fd[j]);
			spi_close();
			exit(pipe_fd[j]);
		}
	}

	// Any given pipe has to have at least 9 * 52 bytes to read (8 chars plus newline by 52 lines)
	// So let's just scan to see what's going on here
	while (doing) {
		for (j = 0; j < NUM_PIPES; j++) {
			avail = check_pipe(j);
			if (avail >= MESSAGE_SIZE) {
				//printf("Pipe %d has %d bytes to read\n", j, avail);
				read_pipe(j);
			}
			usleep(1000);
		}
		render();	// Just for testing purposes
	}

	spi_close();

	return 0;
}
