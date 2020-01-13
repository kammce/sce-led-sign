MATRIX_LIBRARY=src/third_party/rpi-rgb-led-matrix/lib/librgbmatrix.a
FINAL_EXECUTABLE=sce_sign.exe

default: sce_sign.exe

$(MATRIX_LIBRARY):
	cd src/third_party/rpi-rgb-led-matrix && make

$(FINAL_EXECUTABLE) : src/main.cpp $(MATRIX_LIBRARY)
	g++ -o $@ $? \
	-I"build/" -I"src/" -I"src/third_party/rpi-rgb-led-matrix/include" \
	-lpthread -lrt -lm -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4 -mtune=cortex-a7 -O3 \

clean:
	rm $(FINAL_EXECUTABLE) $(MATRIX_LIBRARY)
