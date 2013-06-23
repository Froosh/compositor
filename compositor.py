#!/usr/bin/python
#
"""
Compositor module for Holiday by MooresCloud

Homepage and documentation: http://dev.moorescloud.com/

Copyright (c) 2013, Mark Pesce.
License: MIT (see LICENSE for details)
"""

__author__ = 'Mark Pesce'
__version__ = '0.01-dev'
__license__ = 'MIT'

NUM_PIPES = 4			# Must be in sync with the value defined in compositor.c
NUM_LINES = 52

def open_free_pipe():
	"""Walk through the pool of pipes and open the first available pipe"""
	global NUM_PIPES
	j = 0
	while (j < NUM_PIPES):
		pipename = "/run/pipelights.%0d.fifo" % j;
		try:
			fd = open(pipename, 'wb')
			print("Successfully opened %s" % pipename)
			return fd
		except:
			print "Have an exception, because it doesn't exist?"
			j = j + 1 # Couldn't open this one, try the next one
	return False	# Couldn't open any, so we failed

def close_pipe(pipefd):
	pipefd.close()

if __name__ == '__main__':

	# Testing stuffs here
	a_pipe = open_free_pipe()
	i = 0
	pipestr = ''
	while i < NUM_LINES:
		pipestr = pipestr + "0xFFFFFF\n"
		i = i + 1
	a_pipe.write(pipestr)
	if (a_pipe != False):
		close_pipe(a_pipe)
