all:
	g++ generator.cpp -o generator -Wno-deprecated -L/opt/X11/lib -lglut -lGLU -lGL -lGLEW -std=c++11
	g++ engine.cpp -o engine -Wno-deprecated -L/opt/X11/lib -lglut -lGLU -lGL -lGLEW -lIL -std=c++11

gen:
	g++ generator.cpp -o generator -Wno-deprecated -L/opt/X11/lib -lglut -lGLU -lGL -lGLEW -std=c++11

eng:
	g++ engine.cpp -o engine -Wno-deprecated -L/opt/X11/lib -lglut -lGLU -lGL -lGLEW -lIL -std=c++11

clean:
	rm engine generator
