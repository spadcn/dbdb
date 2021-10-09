CC=gcc
WORKDIR=$(PWD)

CMAKE3 := $(shell cmake3 --version 2> /dev/null)

ifdef CMAKE3
    CMAKE=cmake3
else
    CMAKE=cmake
endif

ADDRESS_SANITIZER=-fsanitize=address -static-libasan -Wpadded
PROJECT_INCLUDE=-I${WORKDIR}/include
PROJECT=-L${WORKDIR}/lib -I${WORKDIR}/include

all:
	mkdir -p build
	rm -rf build/* && cd build && $(CMAKE) -DCMAKE_INSTALL_PREFIX=/usr .. && make && cd -
clean:
	mkdir -p build
	rm -rf build/*
debug:
	mkdir -p build
	rm -rf build/* && cd build && $(CMAKE) -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug .. && make && cd -
release:
	mkdir -p build
	rm -rf build/* && cd build && $(CMAKE) -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release .. && make && cd -
install: release
	cd build/ && sudo make install && sudo ldconfig
debuginstall: debug
	cd build/ && sudo make install && sudo ldconfig
uninstall:
	sh ./UNINSTALL.sh 2> /dev/null || echo

run:
	./build/${prog} $(args)
