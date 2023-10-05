.PHONY : clean_o

all: 5RC_Stint_Time_Util clean_o

cc:=g++
execpath:=./elf/
sourcefiles:=5RC_Stint_Time_Util.o DiskServer.o RegisteredVars.o IbtHandler.o

%.o: %.cpp
	$(cc) -g -c -std=c++17 $?

$(execpath): 
	mkdir $(execpath)

clean_o:
	rm $(wildcard *.o)

5RC_Stint_Time_Util: $(sourcefiles) | $(execpath)
	$(cc) -g -std=c++17 -o $(execpath)/$@ $? 
	
