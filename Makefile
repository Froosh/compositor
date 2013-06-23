# Simple Makefile to make pipelights
compositor: compositor.c compositor.h spi.h gpio-mmap.h
	gcc compositor.c -o compositor -DOLIMEX -DPREPROTO
	sudo chown root:root compositor
	sudo chmod 6711 compositor

rainbowcomp: singbowcomp.c rainbow.c 
	gcc -o rainbowcomp singbowcomp.c -lrt -DOLIMEX -DPREPROTO
	sudo chown root:root rainbowcomp
	sudo chmod 6711 rainbowcomp

randcomp: singcomp.c rand.c 
	gcc -o randcomp singcomp.c -lrt -DOLIMEX -DPREPROTO
	sudo chown root:root randcomp
	sudo chmod 6711 randcomp
