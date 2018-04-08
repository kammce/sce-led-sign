sce_sign : src/main.cpp
	g++ -o sce_sign -I"build/" -I"src/" src/main.cpp src/librgbmatrix.a -lpthread -lrt -lm