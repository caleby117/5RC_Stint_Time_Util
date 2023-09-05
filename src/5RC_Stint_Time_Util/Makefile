.PHONY : clean_o

all: 5RC_Stint_Time_Util clean_o

cc:=g++
execpath:=./elf/
sourcefiles:=5RC_Stint_Time_Util.o StintVars.o

%.o: %.cpp
	$(cc) -c $?

$(execpath): 
	mkdir $(execpath)

clean_o:
	rm $(wildcard *.o)

5RC_Stint_Time_Util: $(sourcefiles) | $(execpath)
	$(cc) $? -o $(execpath)/$@
	
