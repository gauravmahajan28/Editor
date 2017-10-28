a.out: controlTerminal.o fileHandling.o keyboardInput.o defaultMode.o commandMode.o insertMode.o OS_Assign1_20172045.o
	gcc  controlTerminal.o fileHandling.o keyboardInput.o defaultMode.o commandMode.o insertMode.o OS_Assign1_20172045.o

controlTerminal.o: controlTerminal.c controlTerminal.h
	cc -c -Wall controlTerminal.c

fileHandling.o: fileHandling.c fileHandling.h
	cc -c -Wall fileHandling.c

keyboardInput.o: keyboardInput.c keyboardInput.h
	cc -c -Wall keyboardInput.c

defaultMode.o: defaultMode.c modes.h keys.h
	cc -c -Wall defaultMode.c

commandMode.o: commandMode.c modes.h keys.h
	cc -c -Wall commandMode.c

insertMode.o: insertMode.c modes.h keys.h
	cc -c -Wall insertMode.c

OS_Assign1_20172045.o: OS_Assign1_20172045.c
	cc -c -Wall OS_Assign1_20172045.c
