Compositor - multi-process light mixing for Holiday by MooresCloud

The compositor is an always-running background process that opens four named pipes
(/run/pipelights.0.fifo through /run/pipelights.3.fifo) and scans them continuously
looking for data to render to the Holiday's bulbs.

By convention the pipes are allocated to the following processes

Pipe 0 - System processes (warnings, exceptions, failures)
Pipe 1 - IoTAS
Pipe 2 - Python processes
Pipe 3 - Native processes

At this time there's no pooling mechanism, so it is possible for two processes to be writing to the same FIFO.
That could produce - ahem - interesting results.

The data is in the format of 52 lines, each 9 bytes in length (8 characters plus newline)

Flags - 24 bits of ASCII hex in format 0xHHHHHH

NEW_DATA_FLAG   0b0000000000000001		// True if this data is freshly received from pipe
REPLACE_FLAG	0b0000000000000010		// if set replace buffer, otherwise mix
MASK_FLAG		0b0000000000000100		// if set non-zero bytes are masked in
HAS_DATA_FLAG	0b0000000000001000		// Set if there is data to render
BRIGHTNESS_FLAG 0b1000000000000000		// Set the brightness in the following 7 bits
BRIGHTNESS_MASK 0b0111111100000000		// These are the bits used to determine brightness

App Name  - 8 characters of descriptive information: 'IoTASRVC', 'RAINBOWC', etc
Bulb RGB value - 24 bits of ASCII hex in format 0xRRGGBB, repeated 50 times

