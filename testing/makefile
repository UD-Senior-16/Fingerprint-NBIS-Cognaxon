all: estimatethreshold-png estimatethreshold-tif

estimatethreshold-png: estimatethreshold-png.cpp
	g++ -std=c++11 -o estimatethreshold-png estimatethreshold-png.cpp

estimatethreshold-tif: estimatethreshold-tif.cpp
	g++ -std=c++11 -o estimatethreshold-tif estimatethreshold-tif.cpp

clean:
	rm -f estimatethreshold-png estimatethreshold-tif
