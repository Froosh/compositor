# Simple Makefile to make pipelights
all: randcomp compositor rainbowcomp compose

compose: compose.c compose.h spi.h gpio-mmap.h
	gcc compose.c -o compose -DIMX 
	sudo chown root:root compose
	sudo chmod 6711 compose
	
compositor: compositor.c compositor.h spi.h gpio-mmap.h
	gcc compositor.c -o compositor -DIMX 
	sudo chown root:root compositor
	sudo chmod 6711 compositor

rainbowcomp: singbowcomp.c rainbow.c 
	gcc -o rainbowcomp singbowcomp.c -lrt -DIMX
	sudo chown root:root rainbowcomp
	sudo chmod 6711 rainbowcomp

randcomp: singcomp.c rand.c 
	gcc -o randcomp singcomp.c -lrt -DIMX 
	sudo chown root:root randcomp
	sudo chmod 6711 randcomp
