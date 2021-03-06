#***************************************************************************
## name of the operating system
OS := $(shell uname)

## Directory structure
BDIR = $(HOME)/bin
HDIR = $(DAQHOME)/csc_display
SDIR = $(HDIR)/src
ODIR = $(HDIR)/obj
IDIR = $(HDIR)/include
LDIR = $(HDIR)/lib
DAQCOMMON=$(DAQHOME)/daq_common
CMSSW=$(HOME)/CMSSW/src

#***************************************************************************
## names for commands
SHELL      = /bin/sh

ifeq ($(OS),Linux)
CC         = gcc
F77        = g77
CXX = g++
endif

#***************************************************************************
## options for compiling

# Uncomment the next line when debugging
#DEBUG      =  -g -Wall -DDEBUG -DFAKEDATA
#DEBUG      =  -g -Wall -DDEBUG -DCABLE_MAP_TEST
DEBUG      =  -ggdb -W -DDEBUG

# Uncomment the next line when NOT debugging
#DEBUG      = -W -O

ifeq ($(OS),Linux)
CCOPT      = $(DEBUG) -DLOCALDAQ
F77OPT     = $(DEBUG) -ff90 -fno-second-underscore
endif

CCOPT += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE `root-config --cflags` -DCSC_DISPLAY

#***************************************************************************
## location of CERN stuff

ifeq ($(OS),Linux)

CERNLIB  = `$(CERNDIR)/bin/cernlib -v pro graflib packlib grafX11 mathlib kernlib`
endif

#***************************************************************************
## includes and linking X libraries

ifeq ($(OS),Linux)
XINC       = -I/usr/include/X11 -I/usr/X11R6/include -I/usr/include
XLIB       = -L/usr/X11R6/lib -L/usr/lib64 -lXpm -lX11
#EXTLIB	   = -L$(XDAQ_ROOT)/lib -L$(BUILD_HOME)/emu/daq/reader/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) -L$(BUILD_HOME)/emu/dqm/common/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) -L$(BUILD_HOME)/emu/dqm/cmssw/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) -lemudaqreader_sa  -lemudqmcommon_sa -lCSCRawToDigi -lCSCDigi  -llog4cplus 
EXTLIB	   = -L$(BUILD_HOME)/$(XDAQ_PLATFORM)/lib -lemudaqreader -lemudqmcommon -L$(BUILD_HOME)/emu/dqm/cmssw/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) -lCSCRawToDigi -lCSCDigi -L$(XDAQ_ROOT)/lib -llog4cplus -lxerces-c -lxoap -lxcept -ltoolbox -lasyncresolv -lxdaq -lxdata -lcgicc -lxgi -lconfig -lpeer -llogudpappender -llogxmlappender -lmimetic -luuid
endif

#***************************************************************************
## other includes and libs
Project = emu
LOCINC     = -I$(IDIR) -I$(DAQCOMMON)/include -I$(BUILD_HOME)/$(Project)/daq/reader/include  -I$(ROOTSYS)/include -I$(BUILD_HOME)/emu/dqm/cmssw/include/emu/dqm/cmssw -I$(CMSSW) -I$(BUILD_HOME)/emu/dqm/common/include -I$(XDAQ_ROOT)/include

ifeq ($(OS),Linux)
  LINUX_VERSION := $(shell uname -r)
  ifeq ($(LINUX_VERSION),2.0.36)
    LIB        = -lm -ldl -lc -lfl \
             /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.29/libf2c.a
  else
    LIB        = -L$(LDIR)  -lm -ldl -lc -lfl `root-config --glibs`
# IMPORTANT: libemudaqreader_sa must be compiled like this: make Standalone='yes'.
# copy the resulting .so file to this project lib directory
  endif
endif

J_DISPLAYDICT	=rootcint -f src/J_DisplayDict.cc -c -p $(LOCINC) $(IDIR)/J_Display.h 

$(BDIR)/csc_display :  \
	$(ODIR)/application.o \
	$(ODIR)/J_Display.o \
	$(ODIR)/J_DisplayDict.o \
	$(ODIR)/unpack_data.o \
	$(DAQCOMMON)/obj/daq_ipc.o \
	$(DAQCOMMON)/obj/get_next_event.o \
	$(DAQCOMMON)/obj/lex_cable_map.o \
	$(DAQCOMMON)/obj/crcdivide.o 
	$(CXX) -o $(LDIR)/csc_display  \
		$(ODIR)/application.o \
		$(ODIR)/J_Display.o \
		$(ODIR)/J_DisplayDict.o \
		$(ODIR)/unpack_data.o \
		$(DAQCOMMON)/obj/daq_ipc.o \
		$(DAQCOMMON)/obj/get_next_event.o \
		$(DAQCOMMON)/obj/lex_cable_map.o \
		$(DAQCOMMON)/obj/crcdivide.o \
		$(LIB) $(XLIB) $(EXTLIB)


$(ODIR)/J_Display.o : $(SDIR)/J_Display.cc $(DAQCOMMON)/include/csc_event.h \
                                           $(DAQCOMMON)/include/daq_conf.h \
                                           $(DAQCOMMON)/include/j_common_data.h \
					   $(IDIR)/J_Display.h
	$(CC) $(CCOPT) $(LOCINC) $(XINC) -o $(ODIR)/J_Display.o \
	 -c $(SDIR)/J_Display.cc
	$(J_DISPLAYDICT)

$(ODIR)/J_DisplayDict.o : $(SDIR)/J_DisplayDict.cc $(IDIR)/J_Display.h
	$(J_DISPLAYDICT)
	$(CC) $(CCOPT) -I. $(LOCINC) $(XINC) -o $(ODIR)/J_DisplayDict.o \
	-c $(SDIR)/J_DisplayDict.cc

$(ODIR)/unpack_data.o : $(SDIR)/unpack_data.cc \
                                           $(DAQCOMMON)/include/csc_event.h \
                                           $(DAQCOMMON)/include/daq_conf.h \
                                           $(DAQCOMMON)/include/j_common_data.h
	$(CC) $(CCOPT) $(LOCINC) $(XINC) -o $(ODIR)/unpack_data.o \
          -c $(SDIR)/unpack_data.cc 

$(ODIR)/application.o : $(SDIR)/application.cc $(IDIR)/application.h \
                        $(DAQCOMMON)/include/daq_module.h \
                        $(DAQCOMMON)/include/daq_ipc.h \
                        $(DAQCOMMON)/include/daq_conf.h
	$(CC) $(CCOPT) $(LOCINC) $(XINC) -o $(ODIR)/application.o \
          -c $(SDIR)/application.cc

$(DAQCOMMON)/obj/daq_ipc.o :
	cd $(DAQCOMMON) ; make $(DAQCOMMON)/obj/daq_ipc.o

$(DAQCOMMON)/obj/get_next_event.o :
	cd $(DAQCOMMON) ; make $(DAQCOMMON)/obj/get_next_event.o

$(DAQCOMMON)/obj/crcdivide.o :
	cd $(DAQCOMMON) ; make $(DAQCOMMON)/obj/crcdivide.o

$(DAQCOMMON)/obj/lex_cable_map.o :
	cd $(DAQCOMMON) ; make $(DAQCOMMON)/obj/lex_cable_map.o

clean : 
	cd $(ODIR) ; rm -f *.o 
	find . -name "*~" -exec rm {} \;
	rm -f $(LDIR)/csc_display

install : 
	make
	cp $(LDIR)/csc_display $(BDIR)/
