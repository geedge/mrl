.PHONY: all clean install
all:
	cd src && ${MAKE}
clean:
	cd src && ${MAKE} clean
install:
	cd src && ${MAKE} install