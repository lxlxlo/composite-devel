DEST=$(HOME)/.lv2/composite_sampler.lv2
CXXFLAGS=-ggdb -Wall -I../Tritium `pkg-config --cflags QtCore`
LIBS=-L$(LD_LIBRARY_PATH)

all: composite_sampler.so

install: all composite_sampler.ttl
	if [ ! -d $(DEST) ] ; then mkdir $(DEST) ; fi
	cp -f manifest.ttl $(DEST)
	cp -f composite_sampler.so $(DEST)
	cp -f composite_sampler.ttl $(DEST)

composite_sampler.so: EngineLv2.o
	$(CXX) -o $@ $< -shared $(LIBS) -lTritium

clean:
	rm -f *.o composite_sampler.so *~
