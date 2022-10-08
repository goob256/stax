FLAGS=-Wall

OBJS=clobber.o config.o coop.o input.o main.o puzzle.o shared.o shifty.o sucka.o

help:
	@echo
	@echo "Type 'make <platform>' where platform is linux or dos"
	@echo

linux: $(OBJS)
	gcc -o stax $(OBJS) `allegro-config --libs`

dos: $(OBJS)
	gcc -o stax.exe $(OBJS) -lalleg

$(OBJS): stax.h
.c.o:
	gcc -c $(FLAGS) $<

clean:
	@rm -f *~ *.o stax stax.exe
