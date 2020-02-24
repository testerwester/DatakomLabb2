prog : jim_corrected.o Stud.o
	$(CC) jim_corrected.o Stud.o -o prog

jim_corrected.o : jim_corrected.c Stud.h
	$(CC) -c jim_corrected.c

Stud.o : Stud.c Stud.h
	$(CC) -c Stud.c

rm :
	rm -f jim_corrected.o Stud.o prog



#main.o: main.cpp functions.h
#  $(CC) -c main.cpp
#
#factorial.o: factorial.cpp functions.h
#   $(CC) -c factorial.cpp
#
#hello.o: hello.cpp functions.h
#   $(CC) -c hello.cpp