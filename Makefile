all: compile link

compile:
	g++ -I src/include -c maze.cpp

link: 
	g++ maze.o -o maze -L src/lib -l sfml-graphics -l sfml-window -l sfml-system