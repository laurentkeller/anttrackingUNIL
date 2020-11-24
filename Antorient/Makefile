#CPP = g++
CXX = g++
#CXX = $(shell wx-config --cxx)

# Sous Windows, l'executable finit en .exe (pour strip et clean)
ifeq ($(OS),Windows_NT)
PROGRAM = antorient.exe
DATCORR_PROGRAM = datcorr.exe
ICONFILE = winicon.cof
else
DATCORR_PROGRAM = datcorr
PROGRAM = antorient
ICONFILE =
endif

ifndef OSTYPE
OSTYPE = $(shell uname)
endif

# Sous MacOS on veut utiliser la version "locale" (plus recente) de wxWidgets
ifneq (,$(findstring ${OSTYPE},Darwin))
WXCONFIG = /usr/local/bin/wx-config
#ARCH_FLAGS = -arch i386
ARCH_FLAGS = 
LIBS =
else
WXCONFIG = wx-config
ARCH_FLAGS =
LIBS =
endif

OBJECTS = antorient.o mainwin.o main.o tags3.o utils.o exception.o datfile.o wx_datcorr.o datcorrwin.o datcorrthread.o datcorr.o wx_progressbar.o progressbarwin.o

.SUFFIXES:      .o .cpp

.cpp.o :
	$(CXX) -c $(ARCH_FLAGS) -O3 `$(WXCONFIG) --cxxflags` -o $@ $<
#%:
#	$(CXX) $(CPPFLAGS) -o $@ $^$(LIBS)

# Sous MacOS on a besoin d'une application...

ifneq (,$(findstring ${OSTYPE},Darwin))

#CPPFLAGS = -Wall -O3 -arch i386

all:    $(PROGRAM).app $(DATCORR_PROGRAM)

$(PROGRAM).app:   $(PROGRAM)
#	`$(WXCONFIG) --rescomp` $(PROGRAM)
	mkdir -p $(PROGRAM).app
	mkdir -p $(PROGRAM).app/Contents
	mkdir -p $(PROGRAM).app/Contents/MacOS
	mkdir -p $(PROGRAM).app/Contents/Resources
	echo -n "APPL????" > $(PROGRAM).app/Contents/PkgInfo
	cp $(PROGRAM) $(PROGRAM).app/Contents/MacOS
	cp Info.plist $(PROGRAM).app/Contents
	cp app.icns $(PROGRAM).app/Contents/Resources
	rm -f $(PROGRAM)

else

all:  $(PROGRAM) $(DATCORR_PROGRAM)

endif

$(PROGRAM):     $(OBJECTS)
	$(CXX) $(ARCH_FLAGS) -o $(PROGRAM) $(OBJECTS) $(ICONFILE) `$(WXCONFIG) --libs` $(LIBS)
	strip $(PROGRAM)

$(DATCORR_PROGRAM): terminal_datcorr.o datcorr.o tags3.o utils.o datfile.o exception.o
	$(CXX) $(ARCH_FLAGS) $(CPPFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o $(PROGRAM) $(DATCORR_PROGRAM)
	rm -rf $(PROGRAM).app

.PHONY: all clean
