PROGRAM=seriallog
$(PROGRAM) : $(PROGRAM).c
	gcc -O2 -Wall -s $(PROGRAM).c -o $(PROGRAM) -lpthread

