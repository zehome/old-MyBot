include Makefile.common

MODULES = lydiaModule.so pingModule.so listModule.so operatorModule.so

all: test mybot $(MODULES)

mybot: test
	cd src; $(MAKE) mybot

$(MODULES):
	cd src; $(MAKE) $@; cd ..

test:
	@if test ! -f src/Makefile; then echo "Please run ./compile.sh"; exit 1; fi

clean:
	rm -f mod/*.so src/*.o src/Makefile mybot
