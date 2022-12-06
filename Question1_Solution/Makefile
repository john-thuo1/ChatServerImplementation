all : ProcessVersion1 ProcessVersion2

ProcessVersion1: version1.c

	gcc version1.c -o  ProcessVersion1

ProcessVersion2: version2.c

	gcc version2.c -o  ProcessVersion2

clean:
    
	-rm -f *.o ProcessVersion1
	-rm -f *.o ProcessVersion2