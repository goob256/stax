# Very lazy makefile
# FIXME: remove debugging flags

help:
	@echo Type \"make linux\" or \"make windows\"

debug:
	g++ -g -DUNIX *.cpp -o stax `allegro-config --libs`
	exedat stax stax.dat

linux:
	g++ -DUNIX *.cpp -o stax `allegro-config --libs`
	strip stax
	exedat stax stax.dat

windows:
	g++ -mwindows *.cpp icon.res -o stax.exe -lalleg -lwinmm
	exedat stax.exe stax.dat

clean:
	rm -f stax stax.exe
	rm -f stax.hs
	rm -f stax.cfg
