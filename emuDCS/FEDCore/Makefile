##
#
# This is the TriDAS/emu/emuDCS/FEDCore Package Makefile
#
##

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)
include $(BUILD_HOME)/emu/rpm_version

# CXX=g++4
# LD=g++4

#
# Packages to be built
#
Project=emu
Package=emuDCS/FEDCore
PackageName=FEDCore
Description="Core libraries for controlling Front End Device (FED) crates for the CMS Emu (CSC) subdetector"
Summary="Emu (CSC) FED Core Libraries"
Authors="Phillip Killewald, Stan Durkin, Jason Gilmore, Jianhui Gu"
Link="http://physics.ohio-state.edu/~cms"

Sources=\
	CoreVersion.cc \
	VMELock.cc\
	VMEModule.cc \
	Fiber.cc \
	DDU.cc \
	FIFO.cc \
	DCC.cc \
	VMEController.cc \
	Crate.cc

IncludeDirs = \
	$(XDAQ_ROOT)/include \
	$(XDAQ_ROOT)/include/xdaq

LibraryDirs = \
	$(XDAQ_ROOT)/lib

UserCFlags =
UserCCFlags = -pipe -DLINUX -g -Wall -pedantic-errors -Wno-long-long -Wno-variadic-macros -isystem $(XDAQ_ROOT)/include -O2
UserStaticLinkFlags =
UserExecutableLinkFlags = -D_REENTRAN -pthread -pedantic-errors

# These libraries can be platform specific and
# potentially need conditional processing
#
Libraries =
ExternalObjects =

#
# Compile the source files and create a shared library
#
DynamicLibrary = EmuFEDCore
StaticLibrary =

Executables =
TestExecutables =

include $(XDAQ_ROOT)/config/Makefile.rules
include $(XDAQ_ROOT)/config/mfRPM.rules
