##
#
# This is the TriDAS/emu/emuDCS/FEDUtils Package Makefile
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
Package=emuDCS/FEDUtils
PackageName=FEDUtils
Description="Utility libraries used by the Front End Driver (FED) crate XDAQ applications (FEDApps) for the CMS EMu (CSC) subdetector"
Summary="Emu (CSC) FED Utility Libraries"
Authors="Phillip Killewald, Angela Brett, Stan Durkin, Jason Gilmore, Jianhui Gu"
Link="http://physics.ohio-state.edu/~cms"

Sources=\
	UtilsVersion.cc \
	Parser.cc \
	FiberParser.cc \
	DDUParser.cc \
	FIFOParser.cc \
	DCCParser.cc \
	VMEControllerParser.cc \
	CrateParser.cc \
	DBAgent.cc \
	SystemDBAgent.cc \
	CrateDBAgent.cc \
	VMEControllerDBAgent.cc \
	DDUDBAgent.cc \
	DCCDBAgent.cc \
	FiberDBAgent.cc \
	FIFODBAgent.cc \
	DBConfigurator.cc \
	XMLConfigurator.cc \
	AutoConfigurator.cc \
	DDUDebugger.cc \
	DCCDebugger.cc \
	DataTable.cc \
	JSONSpiritWriter.cc

IncludeDirs = \
	$(BUILD_HOME)/emu/emuDCS/FEDCore/include \
	$(BUILD_HOME)/emu/emuDCS/OnlineDB/include \
	$(BUILD_HOME)/emu/base/include \
	$(XDAQ_ROOT)/include \
	$(XDAQ_ROOT)/include/xdaq

LibraryDirs = \
	$(XDAQ_ROOT)/lib

UserCFlags =
UserCCFlags = -pipe -DLINUX -g -Wall -pedantic-errors -Wno-long-long  -Wno-variadic-macros -isystem $(XDAQ_ROOT)/include -O2
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
DynamicLibrary = EmuFEDUtils
StaticLibrary =

Executables =
TestExecutables =

include $(XDAQ_ROOT)/config/Makefile.rules
include $(XDAQ_ROOT)/config/mfRPM.rules
