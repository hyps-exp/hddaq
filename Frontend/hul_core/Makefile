### Makefile ###

LIBDIR		= ../lib

CXXFLAGS	+= -Wall -g -MMD -std=c++1y

SRC		= $(wildcard *.cc)
OBJ		= $(SRC:.cc=.o)
DEPENDS		= $(SRC:.cc=.d)

echo		:= /bin/echo -e
mkdir		:= mkdir -p

libhul.a: $(OBJ)
	@if [ ! -d $(LIBDIR) ]; then \
	  $(echo) "mkdir -p $(LIBDIR)"; $(mkdir) $(LIBDIR); \
	fi
	ar cr $(LIBDIR)/$@ $(OBJ)

clean:
	rm -f $(LIBDIR)/* *.d *.o *~

-include $(DEPENDS)

