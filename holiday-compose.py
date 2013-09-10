#!/usr/bin/python
#
"""
Holiday class implementation for Holiday by Moorescloud

Homepage and documentation: http://dev.moorescloud.com/

Copyright (c) 2013, Mark Pesce.
License: MIT (see LICENSE for details)
"""

__author__ = 'Mark Pesce'
__version__ = '0.01-dev'
__license__ = 'MIT'

import os

class Holiday:

	remote = False
	addr = ''
	NUM_GLOBES = 50
	pipe_fd = -1
	pid = os.getpid()

	# To talk with the compositor
	NUM_LINES = 52

	# Storage for all 50 globe values
	# 
	globes = [ [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00],
	[ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00],
	[ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00],
	[ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00],
	[ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00],
	[ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00],
	[ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00],
	[ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00],
	[ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00],
	[ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00], [ 0x00, 0x00, 0x00] ]

	def __init__(self, remote=False, addr=''):
		"""If remote, you better supply a valid address.  
		We'll throw an exception when you don't do this."""
		#print "init init init"
		self.remote = remote
		if (self.remote == True):
			#if len(addr) == 0:
			#	throw self.KeyboardInterrupt # Kinda vague, eh?
			self.addr = addr
		else:
			self.open_pipe()
			if self.pipe_fd == -1:
				"We could not open a pipe, this is not good."


	def setglobe(self, globenum, r, g, b):
		"""Set a globe"""
		if (globenum < 0) or (globenum >= self.NUM_GLOBES):
			return
		self.globes[globenum][0] = r
		self.globes[globenum][1] = g
		self.globes[globenum][2] = b

	def getglobe(self, globenum):
		"""Return a tuple representing a globe's RGB color value"""
		if (globenum < 0) or (globenum >= self.NUM_GLOBES):
			return False
		return (self.globes[globenum][0], self.globes[globenum][1], self.globes[globenum][2])

	def chase(self, direction=True):
		"""Rotate all of the globes around - up if TRUE, down if FALSE"""
		if direction == True:
			#print "Chasing"
			k = self.globes[self.NUM_GLOBES-1]	# Get the topmost thingy
			i = self.NUM_GLOBES-1
			while i > 0:
				self.globes[i] = self.globes[i-1]
				i = i - 1
			self.globes[0] = k
		else:
			k = self.globes[0]
			i = 0
			while i < self.NUM_GLOBES-1:
				self.globes[i] = self.globes[i+1]
				i = i + 1
			self.globes[self.NUM_GLOBES-1] = k
		return

	def rotate(self, newr, newg, newb, direction="True"):
		"""Rotate all of the globes up if TRUE, down if FALSE
		   Set the new start of the string to the color values"""
		return

	def open_pipe(self):
		"""Open the pipe"""
		pipename = "/run/compose.fifo"
		try:
			self.pipe_fd = open(pipename, 'wb')
			print("Successfully opened %s" % pipename)
			return
		except:
			print "Have an exception, because it doesn't exist?"
			self.pipe_fd = -1
		return False	# Couldn't open any, so we failed

	def close_pipe(self):
		self.pipe_fd.close()

	def render(self):
		"""The render routine varies depending on where it's running.
		If it's running locally on a Holiday then it uses pipeligths/compositor
		And sends the globe values out directly.
		If it's running on a remote machine, it has to use IoTAS (and requests) to send it"""

		if (self.remote == True):
			print "REMOTE NOT IMPLEMENTED!!!"

		else:
			"""Render the LED array to the Holiday
			This is done by composing a text string in memory
			Which is then written out to the compositor FIFO pipe in a single go, 
			So it should be reasonably fast."""
			compositor_str = "0x010203\n"
			compositor_str = compositor_str + "HolidayR\n"		# First two lines are placeholders for now, will be meaningful
			ln = 0
			while (ln < self.NUM_GLOBES):
				tripval = (self.globes[ln][0] * 65536) + (self.globes[ln][1] * 256) + self.globes[ln][2]
				compositor_str = compositor_str + "0x%06X\n" % tripval
				ln = ln+1
			#print compositor_str
			self.pipe_fd.write(compositor_str)
			self.pipe_fd.flush()
			return

if __name__ == '__main__':
	import time, random
	hol = Holiday(remote=False)

	# We're going to chase a light around, so set it up
	hol.setglobe(0, 0x03, 0x03, 0x03)
	hol.setglobe(1, 0xbf, 0xbf, 0xbf)
	hol.setglobe(2, 0x02, 0x02, 0x02)
	hol.render()

	while True:
		hol.chase(direction=True)
		hol.render()
		time.sleep(.1)
	hol.close_pipe()

