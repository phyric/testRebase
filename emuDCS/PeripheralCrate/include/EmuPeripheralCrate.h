// $Id: EmuPeripheralCrate.h,v 2.32 2006/04/27 18:46:03 mey Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _EmuPeripheralCrate_h_
#define _EmuPeripheralCrate_h_

//#define STANDALONE

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include<unistd.h> // for sleep()
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <time.h>

#include "xdaq/Application.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/exception/Exception.h"

#include "xgi/Utils.h"
#include "xgi/Method.h"
#include "xdata/UnsignedLong.h"
#include "xdata/String.h"

#include "xdaq/NamespaceURI.h"
#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"

#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

// My Stuff

#include "PeripheralCrateParser.h"
#include "EmuController.h"
#include "Crate.h"
#include "DAQMB.h"
#include "TMB.h"
#include "CrateUtils.h"
#include "TMBTester.h"
#include "CCB.h"
#include "MPC.h"
#include "DDU.h"
#include "DAQMBTester.h"
#include "TMBTester.h"
#include "ALCTController.h"
#include "RAT.h"
#include "CrateSelector.h"
#include "ChamberUtilities.h"
//#include "geom.h"
#include "InfoSpace.h"
#include "CrateUtilities.h"
#include "CalibDAQ.h"
//
#ifdef STANDALONE
#else
#include "EmuApplication.h"
#endif

using namespace cgicc;
using namespace std;

#ifdef STANDALONE
class EmuPeripheralCrate: public xdaq::Application 
#else
class EmuPeripheralCrate: public EmuApplication
#endif
{
  //
protected:
  //
  xdata::InfoSpace *appInfoSpace_;
  vector< pair<string, xdata::Serializable *> > stdConfigParams_;
  //
  xdata::UnsignedLong triggerSourceId_;
  //
  // EMu-specific stuff
  //
  xdata::UnsignedLong runNumber_;
  xdata::UnsignedLong maxNumTriggers_;
  //
  xdata::String xmlFile_;
  xdata::String TMBFirmware_;
  xdata::String TestLogFile_;
  //
  std::string xmlFile;
  xdata::UnsignedLong myParameter_;
  EmuController * MyController;
  //TMB * thisTMB ;
  //DAQMB* thisDMB ;
  CCB* thisCCB ;
  ALCTController *alct ;
  RAT * rat;
  MPC * thisMPC;
  ChamberUtilities MyTest[9];
  ostringstream CrateTestsOutput[9];
  ostringstream OutputStringDMBStatus[9];
  ostringstream OutputStringTMBStatus[9];
  ostringstream OutputDMBTests[9];
  ostringstream OutputTMBTests[9];
  std::vector <float> ChartData[100];
  //
  int TMBRegisterValue_;
  int CCBRegisterValue_;
  vector<TMB*>   tmbVector;
  vector<TMBTester>   tmbTestVector;
  vector<DAQMB*> dmbVector;
  Crate *thisCrate;
  std::string Operator_;
  std::string RunNumber_;
  std::string MPCBoardID_;
  std::string CCBBoardID_;
  std::string DMBBoardID_[9];
  std::string TMBBoardID_[9];
  std::string RATBoardID_[9];
  int TMB_, DMB_,RAT_;
  int Counter_;
  bool AutoRefreshTMBCounters_;
  //
public:
  //
  XDAQ_INSTANTIATOR();
  //
#ifdef STANDALONE
  EmuPeripheralCrate(xdaq::ApplicationStub * s): xdaq::Application(s) 
#else
  EmuPeripheralCrate(xdaq::ApplicationStub * s): EmuApplication(s)
#endif
  {	
    //
    MyController = 0;
    //thisTMB = 0;
    //thisDMB = 0;
    thisCCB = 0;
    thisMPC = 0;
    rat = 0;
    alct = 0;
    //
    xgi::bind(this,&EmuPeripheralCrate::Default, "Default");
    xgi::bind(this,&EmuPeripheralCrate::setConfFile, "setConfFile");
    xgi::bind(this,&EmuPeripheralCrate::getTestLogFile, "getTestLogFile");
    xgi::bind(this,&EmuPeripheralCrate::getTestLogFileUpload, "getTestLogFileUpload");
    xgi::bind(this,&EmuPeripheralCrate::TmbMPCTest, "TmbMPCTest");
    xgi::bind(this,&EmuPeripheralCrate::InitSystem, "InitSystem");
    xgi::bind(this,&EmuPeripheralCrate::InitChamber, "InitChamber");
    xgi::bind(this,&EmuPeripheralCrate::setRawConfFile, "setRawConfFile");
    xgi::bind(this,&EmuPeripheralCrate::UploadConfFile, "UploadConfFile");
    xgi::bind(this,&EmuPeripheralCrate::TMBStatus, "TMBStatus");
    xgi::bind(this,&EmuPeripheralCrate::DefineConfiguration, "DefineConfiguration");
    //
    xgi::bind(this,&EmuPeripheralCrate::getData0, "getData0");
    xgi::bind(this,&EmuPeripheralCrate::getData1, "getData1");
    xgi::bind(this,&EmuPeripheralCrate::getData2, "getData2");
    xgi::bind(this,&EmuPeripheralCrate::getData3, "getData3");
    xgi::bind(this,&EmuPeripheralCrate::getData4, "getData4");
    xgi::bind(this,&EmuPeripheralCrate::getData5, "getData5");
    xgi::bind(this,&EmuPeripheralCrate::getData6, "getData6");
    xgi::bind(this,&EmuPeripheralCrate::getData7, "getData7");
    xgi::bind(this,&EmuPeripheralCrate::getData8, "getData8");
    xgi::bind(this,&EmuPeripheralCrate::getData9, "getData9");
    xgi::bind(this,&EmuPeripheralCrate::getData10, "getData10");
    xgi::bind(this,&EmuPeripheralCrate::getData11, "getData11");
    xgi::bind(this,&EmuPeripheralCrate::getData12, "getData12");
    xgi::bind(this,&EmuPeripheralCrate::getData13, "getData13");
    xgi::bind(this,&EmuPeripheralCrate::getData14, "getData14");
    xgi::bind(this,&EmuPeripheralCrate::getData15, "getData15");
    xgi::bind(this,&EmuPeripheralCrate::getData16, "getData16");
    xgi::bind(this,&EmuPeripheralCrate::getData17, "getData17");
    xgi::bind(this,&EmuPeripheralCrate::getData18, "getData18");
    xgi::bind(this,&EmuPeripheralCrate::getData19, "getData19");
    xgi::bind(this,&EmuPeripheralCrate::getData20, "getData20");
    xgi::bind(this,&EmuPeripheralCrate::getData21, "getData21");
    //
    xgi::bind(this,&EmuPeripheralCrate::LoadTMBFirmware, "LoadTMBFirmware");
    xgi::bind(this,&EmuPeripheralCrate::LoadALCTFirmware, "LoadALCTFirmware");
    xgi::bind(this,&EmuPeripheralCrate::ReadTMBRegister, "ReadTMBRegister");
    xgi::bind(this,&EmuPeripheralCrate::ReadCCBRegister, "ReadCCBRegister");
    xgi::bind(this,&EmuPeripheralCrate::HardReset, "HardReset");
    xgi::bind(this,&EmuPeripheralCrate::testTMB, "testTMB");
    xgi::bind(this,&EmuPeripheralCrate::Automatic, "Automatic");
    xgi::bind(this,&EmuPeripheralCrate::TMBTests,  "TMBTests");
    xgi::bind(this,&EmuPeripheralCrate::TMBUtils,  "TMBUtils");
    xgi::bind(this,&EmuPeripheralCrate::DMBStatus, "DMBStatus");
    xgi::bind(this,&EmuPeripheralCrate::DMBBoardID, "DMBBoardID");
    xgi::bind(this,&EmuPeripheralCrate::TMBBoardID, "TMBBoardID");
    xgi::bind(this,&EmuPeripheralCrate::RATBoardID, "RATBoardID");
    xgi::bind(this,&EmuPeripheralCrate::CCBStatus, "CCBStatus");
    xgi::bind(this,&EmuPeripheralCrate::CCBUtils, "CCBUtils");
    xgi::bind(this,&EmuPeripheralCrate::MPCStatus, "MPCStatus");
    xgi::bind(this,&EmuPeripheralCrate::DMBTests, "DMBTests");
    xgi::bind(this,&EmuPeripheralCrate::DMBUtils, "DMBUtils");
    xgi::bind(this,&EmuPeripheralCrate::DMBLoadFirmware, "DMBLoadFirmware");
    xgi::bind(this,&EmuPeripheralCrate::CFEBLoadFirmware, "CFEBLoadFirmware");
    xgi::bind(this,&EmuPeripheralCrate::CFEBStatus, "CFEBStatus");
    xgi::bind(this,&EmuPeripheralCrate::ALCTStatus, "ALCTStatus");
    xgi::bind(this,&EmuPeripheralCrate::CrateConfiguration, "CrateConfiguration");
    xgi::bind(this,&EmuPeripheralCrate::RATStatus, "RATStatus");
    xgi::bind(this,&EmuPeripheralCrate::CrateTests, "CrateTests");
    xgi::bind(this,&EmuPeripheralCrate::ChamberTests, "ChamberTests");
    xgi::bind(this,&EmuPeripheralCrate::DMBTurnOff, "DMBTurnOff");
    xgi::bind(this,&EmuPeripheralCrate::DMBTurnOn, "DMBTurnOn");
    xgi::bind(this,&EmuPeripheralCrate::TMBPrintCounters, "TMBPrintCounters");
    xgi::bind(this,&EmuPeripheralCrate::TMBResetCounters, "TMBResetCounters");
    xgi::bind(this,&EmuPeripheralCrate::DMBPrintCounters, "DMBPrintCounters");
    xgi::bind(this,&EmuPeripheralCrate::ALCTTiming, "ALCTTiming");
    xgi::bind(this,&EmuPeripheralCrate::ALCTScan, "ALCTScan");
    xgi::bind(this,&EmuPeripheralCrate::FindLv1aDelayComparator, "FindLv1aDelayComparator");
    xgi::bind(this,&EmuPeripheralCrate::CFEBTiming, "CFEBTiming");
    xgi::bind(this,&EmuPeripheralCrate::CFEBScan, "CFEBScan");
    xgi::bind(this,&EmuPeripheralCrate::CalibrationRuns, "CalibrationRuns");
    xgi::bind(this,&EmuPeripheralCrate::TMBStartTrigger, "TMBStartTrigger");
    xgi::bind(this,&EmuPeripheralCrate::EnableL1aRequest, "EnableL1aRequest");
    xgi::bind(this,&EmuPeripheralCrate::TMBL1aTiming, "TMBL1aTiming");
    xgi::bind(this,&EmuPeripheralCrate::ALCTL1aTiming, "ALCTL1aTiming");
    xgi::bind(this,&EmuPeripheralCrate::ALCTvpf,"ALCTvpf");
    xgi::bind(this,&EmuPeripheralCrate::DMBTestAll, "DMBTestAll");
    xgi::bind(this,&EmuPeripheralCrate::DMBTest3, "DMBTest3");
    xgi::bind(this,&EmuPeripheralCrate::DMBTest4, "DMBTest4");
    xgi::bind(this,&EmuPeripheralCrate::DMBTest5, "DMBTest5");
    xgi::bind(this,&EmuPeripheralCrate::DMBTest6, "DMBTest6");
    xgi::bind(this,&EmuPeripheralCrate::DMBTest8, "DMBTest8");
    xgi::bind(this,&EmuPeripheralCrate::DMBTest9, "DMBTest9");
    xgi::bind(this,&EmuPeripheralCrate::DMBTest10, "DMBTest10");
    xgi::bind(this,&EmuPeripheralCrate::DMBTest11, "DMBTest11");
    xgi::bind(this,&EmuPeripheralCrate::TriggerTestInjectALCT, "TriggerTestInjectALCT");
    xgi::bind(this,&EmuPeripheralCrate::TriggerTestInjectCLCT, "TriggerTestInjectCLCT");
    xgi::bind(this,&EmuPeripheralCrate::PowerUp,  "PowerUp");
    xgi::bind(this,&EmuPeripheralCrate::Operator, "Operator");
    xgi::bind(this,&EmuPeripheralCrate::RunNumber, "RunNumber");
    xgi::bind(this,&EmuPeripheralCrate::MPCBoardID, "MPCBoardID");
    xgi::bind(this,&EmuPeripheralCrate::CCBBoardID, "CCBBoardID");
    xgi::bind(this,&EmuPeripheralCrate::LogDMBTestsOutput, "LogDMBTestsOutput");
    xgi::bind(this,&EmuPeripheralCrate::LogOutput, "LogOutput");
    xgi::bind(this,&EmuPeripheralCrate::LogTestSummary, "LogTestSummary");
    xgi::bind(this,&EmuPeripheralCrate::LogTMBTestsOutput, "LogTMBTestsOutput");
    xgi::bind(this,&EmuPeripheralCrate::FindWinner, "FindWinner");
    xgi::bind(this,&EmuPeripheralCrate::CalibrationCFEBTime, "CalibrationCFEBTime");
    xgi::bind(this,&EmuPeripheralCrate::CalibrationCFEBCharge, "CalibrationCFEBCharge");
    xgi::bind(this,&EmuPeripheralCrate::CalibrationCFEBPedestal, "CalibrationCFEBPedestal");
    xgi::bind(this,&EmuPeripheralCrate::CalibrationComparatorPulse, "CalibrationComparatorPulse");
    xgi::bind(this,&EmuPeripheralCrate::CalibrationALCT, "CalibrationALCT");
    xgi::bind(this,&EmuPeripheralCrate::LaunchMonitor, "LaunchMonitor");
    xgi::bind(this,&EmuPeripheralCrate::CreateMonitorUnit, "CreateMonitorUnit");
    xgi::bind(this,&EmuPeripheralCrate::MonitorFrameLeft, "MonitorFrameLeft");
    xgi::bind(this,&EmuPeripheralCrate::MonitorFrameRight, "MonitorFrameRight");
    xgi::bind(this,&EmuPeripheralCrate::ResetAllCounters, "ResetAllCounters");
    xgi::bind(this,&EmuPeripheralCrate::CalibrationRandomWiresALCT, "CalibrationRandomWiresALCT");
    //
#ifndef STANDALONE
    //
    // State machine definition
    //

    // SOAP call-back functions, which relays to *Action method.
    xoap::bind(this, &EmuPeripheralCrate::onConfigure, "Configure", XDAQ_NS_URI);
    xoap::bind(this, &EmuPeripheralCrate::onEnable,    "Enable",    XDAQ_NS_URI);
    xoap::bind(this, &EmuPeripheralCrate::onDisable,   "Disable",   XDAQ_NS_URI);
    xoap::bind(this, &EmuPeripheralCrate::onHalt,      "Halt",      XDAQ_NS_URI);
    //
    // fsm_ is defined in EmuApplication
    fsm_.addState('H', "Halted",     this, &EmuPeripheralCrate::stateChanged);
    fsm_.addState('C', "Configured", this, &EmuPeripheralCrate::stateChanged);
    fsm_.addState('E', "Enabled",    this, &EmuPeripheralCrate::stateChanged);
    //
    fsm_.addStateTransition(
      'H', 'C', "Configure", this, &EmuPeripheralCrate::configureAction);
    fsm_.addStateTransition(
      'C', 'C', "Configure", this, &EmuPeripheralCrate::configureAction);
    fsm_.addStateTransition(
      'C', 'E', "Enable",    this, &EmuPeripheralCrate::enableAction);
    fsm_.addStateTransition(
      'E', 'C', "Disable",   this, &EmuPeripheralCrate::disableAction);
    fsm_.addStateTransition(
      'C', 'H', "Halt",      this, &EmuPeripheralCrate::haltAction);
    fsm_.addStateTransition(
      'E', 'H', "Halt",      this, &EmuPeripheralCrate::haltAction);
    fsm_.addStateTransition(
      'H', 'H', "Halt",      this, &EmuPeripheralCrate::haltAction);

    fsm_.setInitialState('H');
    fsm_.reset();

    //
    // state_ is defined in EmuApplication
    state_ = fsm_.getStateName(fsm_.getCurrentState());
    //

#endif // ! STANDALONE    
    //
    //xoap::bind(this, &EmuPeripheralCrate::onMessage, "onMessage", XDAQ_NS_URI );    
    //xoap::bind(this, &EmuPeripheralCrate::Configure, "Configure", XDAQ_NS_URI );    
    //xoap::bind(this, &EmuPeripheralCrate::Init, "Init", XDAQ_NS_URI );    
    //xoap::bind(this, &EmuPeripheralCrate::Enable, "Enable", XDAQ_NS_URI );    
    //xoap::bind(this, &EmuPeripheralCrate::Disable, "Disable", XDAQ_NS_URI );    
    //
    myParameter_ =  0;
    //
    xmlFile_     = 
      "config.xml" ;
    //
    TMBFirmware_ = 
      "../svf/tmb2005e.svf";
    //
    TMBRegisterValue_ = -1;
    CCBRegisterValue_ = -1;
    Operator_ = "Name...";
    RunNumber_= "0";
    MPCBoardID_ = "-2";
    CCBBoardID_ = "-2";
    for (int i=0; i<9; i++) { DMBBoardID_[i] = "-2" ; TMBBoardID_[i] = "-2" ; RATBoardID_[i] = "-2" ;}
    //
    for(int i=0; i<9;i++) {
      OutputStringDMBStatus[i] << "Output..." << std::endl;
      OutputStringTMBStatus[i] << "Output..." << std::endl;
      OutputDMBTests[i]        << "Output..." << std::endl;
      OutputTMBTests[i]        << "Output..." << std::endl;
      CrateTestsOutput[i]      << "Output..." << std::endl;
    }
    //
    AutoRefreshTMBCounters_ = true ;
    //
    this->getApplicationInfoSpace()->fireItemAvailable("runNumber", &runNumber_);
    this->getApplicationInfoSpace()->fireItemAvailable("xmlFileName", &xmlFile_);
    //
  }
  //
  void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    std::string LoggerName = getApplicationLogger().getName() ;
    std::cout << "Name of Logger is " <<  LoggerName <<std::endl;
    //
    //if (getApplicationLogger().exists(getApplicationLogger().getName())) {
      LOG4CPLUS_INFO(getApplicationLogger(), "EmuPeripheralCrate");
      //}
    //
    if ( MyController == 0 ) MyController = new EmuController();
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    //
    *out << "<a href=\"/\"><img border=\"0\" src=\"/daq/xgi/images/XDAQLogo.gif\" title=\"XDAQ\" alt=\"\" style=\"width: 145px; height: 89px;\"></a>" << std::endl;
    //
    *out << cgicc::title("EmuPeripheralCrate") << std::endl;
    //
    *out << cgicc::h1("EmuPeripheralCrate");
    *out << cgicc::br();
    //
    //std::cout << "The xmlfile is " << xmlFile_.toString() << std::endl;
    //
    //if (tmbVector.size()==0 && dmbVector.size()==0) {
    //
    *out << cgicc::span().set("style","color:blue");
    *out << cgicc::h3(cgicc::b(cgicc::i("Current configuration filename : "))) ;
    *out << cgicc::span();
    //
    *out << xmlFile_.toString() << std::endl ;
    //
    *out << cgicc::br();
    //
    std::string DefineConfiguration =
      toolbox::toString("/%s/DefineConfiguration",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DefineConfiguration) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Define Configuration") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    if (tmbVector.size()>0 || dmbVector.size()>0) {
      //
      if ( Operator_.find("NameOld",0) == string::npos ) {
	//
	// Crate Configuration
	//
	std::string CrateConfiguration =
	  toolbox::toString("/%s/CrateConfiguration",getApplicationDescriptor()->getURN().c_str());
	//
	*out << cgicc::a("[Crate Configuration]").set("href",CrateConfiguration) << endl;
	//
	std::string CrateTests =
	  toolbox::toString("/%s/CrateTests",getApplicationDescriptor()->getURN().c_str());
	//
	*out << cgicc::a("[Crate Tests]").set("href",CrateTests) << endl;
	//
	std::string CalibrationRuns =
	  toolbox::toString("/%s/CalibrationRuns",getApplicationDescriptor()->getURN().c_str());
	//
	*out << cgicc::a("[Calibration Runs]").set("href",CalibrationRuns) << endl;
	//
      }
      //
      *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial; background-color:yellow");
      *out << std::endl;
      *out << cgicc::legend((("Initialisation"))).set("style","color:blue") ;
      //
      std::string InitSystem =
	toolbox::toString("/%s/InitSystem",getApplicationDescriptor()->getURN().c_str());
      //
      *out << cgicc::form().set("method","GET").set("action",InitSystem) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Init System") << std::endl ;
      *out << cgicc::form() << std::endl ;
      //
      std::string LaunchMonitor =
	toolbox::toString("/%s/LaunchMonitor",getApplicationDescriptor()->getURN().c_str());
      //
      *out << cgicc::form().set("method","GET").set("action",LaunchMonitor).set("target","_blank") << std::endl ;
      *out << cgicc::input().set("type","submit")
	.set("value","Launch Monitor").set("name","LaunchMonitor") << std::endl ;
      *out << cgicc::form() << std::endl ;
      //
      std::string Operator =
	toolbox::toString("/%s/Operator",getApplicationDescriptor()->getURN().c_str());
      //
      *out << cgicc::form().set("method","GET").set("action",Operator) << std::endl ;
      *out << cgicc::input().set("type","text").set("name","Operator")
	.set("size","20").set("value",Operator_) << std::endl ;
      *out << "Operator" << std::endl;
      *out << cgicc::form() << std::endl ;
      //
      std::string RunNumber =
	toolbox::toString("/%s/RunNumber",getApplicationDescriptor()->getURN().c_str());
      //
      *out << cgicc::form().set("method","GET").set("action",RunNumber) << std::endl ;
      *out << cgicc::input().set("type","text").set("name","RunNumber")
	.set("size","20").set("value",RunNumber_) << std::endl ;
      *out << "RunNumber" << std::endl;
      *out << cgicc::form() << std::endl ;
      //
      std::string LogOutput =
	toolbox::toString("/%s/LogOutput",getApplicationDescriptor()->getURN().c_str());
      //
      *out << cgicc::form().set("method","GET").set("action",LogOutput) << std::endl ;
      *out << cgicc::input().set("type","submit")
	.set("value","Log all output").set("name","LogOutput") << std::endl ;
      *out << cgicc::form() << std::endl ;
      //
      std::string LogTestSummary =
	toolbox::toString("/%s/LogTestSummary",getApplicationDescriptor()->getURN().c_str());
      //
      *out << cgicc::form().set("method","GET").set("action",LogTestSummary) << std::endl ;
      *out << cgicc::input().set("type","submit")
	.set("value","Log Test Summary").set("name","LogTestSummary") << std::endl ;
      *out << cgicc::form() << std::endl ;
      //
      //
      std::string PowerUp =
	toolbox::toString("/%s/PowerUp",getApplicationDescriptor()->getURN().c_str());
      //
      *out << cgicc::form().set("method","GET").set("action","http://emuslice03:1973/urn:xdaq-application:lid=30/")
	.set("target","_blank") << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Power Up") << std::endl ;
      *out << cgicc::form() << std::endl ;
      //
      //
      *out << cgicc::fieldset();
      //
      //
      //
      *out << std::endl;
      //
      //
    }
    //
    //cout << "Here4" << endl ;
    //
  }
  //
  //
  // SOAP Callback  
  //
private:
  //
#ifndef STANDALONE
  //
  // SOAP Callback  
  //
  //
  xoap::MessageReference EmuPeripheralCrate::onConfigure (xoap::MessageReference message) throw (xoap::exception::Exception)
  {
    fireEvent("Configure");

    return createReply(message);
  }

  //
  xoap::MessageReference EmuPeripheralCrate::onEnable (xoap::MessageReference message) throw (xoap::exception::Exception)
  {
    fireEvent("Enable");

    return createReply(message);
  }

  //
  xoap::MessageReference EmuPeripheralCrate::onDisable (xoap::MessageReference message) throw (xoap::exception::Exception)
  {
    fireEvent("Disable");

    return createReply(message);
  }

  //
  xoap::MessageReference EmuPeripheralCrate::onHalt (xoap::MessageReference message) throw (xoap::exception::Exception)
  {
    fireEvent("Halt");

    return createReply(message);
  }

  void EmuPeripheralCrate::configureAction(toolbox::Event::Reference e) 
    throw (toolbox::fsm::exception::Exception)
  {
    //
    //if ( MyController != 0 ) {
    //delete MyController ;
    //}
    //
    //MyController = new EmuController();
    //
    //MyController->SetConfFile(xmlFile_);
    //
    Configuring();
    //
    //MyController->init(); // For CSCSupervisor
    //
    //MyController->configure();
    //
    std::cout << "Configure" << std::endl ;
    std::cout << xmlFile_.toString() << std::endl;
    //
    //sleep(3);
    //
    // reply to caller
    //
    std::cout << "Received Message Configure" << std::endl ;
  }

  //
  void EmuPeripheralCrate::enableAction(toolbox::Event::Reference e) 
    throw (toolbox::fsm::exception::Exception)
  {
    //
    MyController->init();
    //
    MyController->configure();
    //
    std::cout << "Received Message Enable" << std::endl ;
  }

  //
  void EmuPeripheralCrate::disableAction(toolbox::Event::Reference e) 
    throw (toolbox::fsm::exception::Exception)
  {
    // do nothing
    std::cout << "Received Message Disable" << std::endl ;
  }

  //
  void EmuPeripheralCrate::haltAction(toolbox::Event::Reference e) 
    throw (toolbox::fsm::exception::Exception)
  {
    // do nothing
    std::cout << "Received Message Halt" << std::endl ;
  }

  //
  void EmuPeripheralCrate::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
    throw (toolbox::fsm::exception::Exception)
  {
    EmuApplication::stateChanged(fsm);
  }

#endif // ! STANDALONE
  //
  xoap::MessageReference Configure (xoap::MessageReference msg) throw (xoap::exception::Exception)
  {
    //
    if ( MyController != 0 ) {
      delete MyController ;
    }
    //
    MyController = new EmuController();
    //
    MyController->SetConfFile(xmlFile_);
    //
    //MyController->init(); // For CSCSupervisor
    //
    //MyController->configure();
    //
    std::cout << "Configure" << std::endl ;
    //
    //sleep(3);
    //
    // reply to caller
    //
    std::cout << "Received Message Configure" << std::endl ;
    //
    //fireEvent("Configure");
    //
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = envelope.createName( "onMessageResponse", "xdaq", XDAQ_NS_URI);
    //xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
    return reply;    
  }
  //
  xoap::MessageReference Init (xoap::MessageReference msg) throw (xoap::exception::Exception)
  {
    //
    if ( MyController != 0 ) {
      delete MyController ;
    }
    //
    MyController = new EmuController();
    //
    MyController->SetConfFile(xmlFile_);
    //
    MyController->init();
    //
    // reply to caller
    //
    std::cout << "Received Message Init" << std::endl ;
    //
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = envelope.createName( "onMessageResponse", "xdaq", XDAQ_NS_URI);
    //xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
    return reply;
    
  }
  //
  xoap::MessageReference Enable (xoap::MessageReference msg) throw (xoap::exception::Exception)
  {
    //
    //enable();
    //
    std::cout << "Enable" << std::endl ;
    //
    //sleep(3);
    //
    // reply to caller
    //
    std::cout << "Received Message Enable" << std::endl ;
    //
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = envelope.createName( "onMessageResponse", "xdaq", XDAQ_NS_URI);
    //xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
    return reply;    
  }
  //
  xoap::MessageReference Disable (xoap::MessageReference msg) throw (xoap::exception::Exception)
  {
    //
    //disable();
    //
    std::cout << "Disable" << std::endl ;
    //
    ::sleep(3);
    //
    // reply to caller
    //
    std::cout << "Received Message Disable" << std::endl ;
    //
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = envelope.createName( "onMessageResponse", "xdaq", XDAQ_NS_URI);
    //xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
    return reply;    
  }
  //
  void EmuPeripheralCrate::CrateTests(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << cgicc::legend("Crate Tests").set("style","color:blue") ;
    //
    std::string TmbMPCTest =
      toolbox::toString("/%s/TmbMPCTest",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",TmbMPCTest) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Crate TMB/MPC test") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::fieldset();
    //
  }
  //
  void EmuPeripheralCrate::DefineConfiguration(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    //
    *out << "<a href=\"/\"><img border=\"0\" src=\"/daq/xgi/images/XDAQLogo.gif\" title=\"XDAQ\" alt=\"\" style=\"width: 145px; height: 89px;\"></a>" << std::endl;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << std::endl;
    //
    *out << cgicc::legend("Upload Configuration...").set("style","color:blue") 
	 << cgicc::p() << std::endl ;
    //
    std::string method =
      toolbox::toString("/%s/setConfFile",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","POST").set("action",method) << std::endl ;
    *out << cgicc::input().set("type","text")
      .set("name","xmlFilename")
      .set("size","90")
      .set("ENCTYPE","multipart/form-data")
      .set("value",xmlFile_);
    //
    *out << std::endl;
    //
    *out << cgicc::input().set("type","submit")
      .set("value","Set configuration file local") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    //
    //
    // Upload file...
    //
    std::string methodUpload =
      toolbox::toString("/%s/UploadConfFile",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","POST")
      .set("enctype","multipart/form-data")
      .set("action",methodUpload) << std::endl ;
    //
    *out << cgicc::input().set("type","file")
      .set("name","xmlFilenameUpload")
      .set("size","90") ;
    //
    *out << std::endl;
    //
    *out << cgicc::input().set("type","submit").set("value","Send") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    *out << std::endl;
    //
    std::string methodRaw =
      toolbox::toString("/%s/setRawConfFile",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","POST").set("action",methodRaw) << std::endl ;
    *out << cgicc::textarea().set("name","Text")
      .set("WRAP","OFF")
      .set("rows","10").set("cols","90");
    *out << "Paste configuration..." << endl ;
    *out << cgicc::textarea();
    *out << cgicc::input().set("type","submit").set("value","Send");
    *out << cgicc::form() << std::endl ;    
    //
    *out << std::endl;
    //    
    std::string getTestLogFile =
      toolbox::toString("/%s/getTestLogFile",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","POST").set("action",getTestLogFile) << std::endl ;
    *out << cgicc::input().set("type","text")
      .set("name","TestLogFile")
      .set("size","90")
      .set("ENCTYPE","multipart/form-data")
      .set("value",TestLogFile_);
    //
    *out << cgicc::input().set("type","submit")
      .set("value","Restore Test configuration") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    std::string getTestLogFileUpload =
      toolbox::toString("/%s/getTestLogFileUpload",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","POST")
      .set("enctype","multipart/form-data")
      .set("action",getTestLogFileUpload) << std::endl ;
    //
    *out << cgicc::input().set("type","file")
      .set("name","TestFileUpload")
      .set("size","90") ;
    //
    *out << std::endl;
    //
    *out << cgicc::input().set("type","submit").set("value","Send") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    *out << std::endl;
    //
    *out << cgicc::fieldset();
    *out << std::endl;
    //
  }
  //
  void EmuPeripheralCrate::CalibrationRuns(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << cgicc::legend("Calibration Runs").set("style","color:blue") ;
    //
    std::string CalibrationCFEBTime =
      toolbox::toString("/%s/CalibrationCFEBTime",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",CalibrationCFEBTime) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Calibration run CFEB Time") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    std::string CalibrationComparatorPulse =
      toolbox::toString("/%s/CalibrationComparatorPulse",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",CalibrationComparatorPulse) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Calibration Comparator Pulse") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    std::string FindLv1aDelayComparator =
      toolbox::toString("/%s/FindLv1aDelayComparator",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",FindLv1aDelayComparator) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Find l1a delay for Comparator") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    std::string CalibrationCFEBCharge =
      toolbox::toString("/%s/CalibrationCFEBCharge",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",CalibrationCFEBCharge) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Calibration run CFEB Charge") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    std::string CalibrationCFEBPedestal =
      toolbox::toString("/%s/CalibrationCFEBPedestal",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",CalibrationCFEBPedestal) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Calibration run CFEB Pedestal") << std::endl ;
      *out << cgicc::form() << std::endl ;
      //
      std::string CalibrationALCT =
	toolbox::toString("/%s/CalibrationALCT",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",CalibrationALCT) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Calibration run all Wires ALCT") << std::endl ;
      *out << cgicc::form() << std::endl ;
      //
      std::string CalibrationRandomWiresALCT =
	toolbox::toString("/%s/CalibrationRandomWiresALCT",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",CalibrationRandomWiresALCT) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Calibration random Wires ALCT") << std::endl ;
      *out << cgicc::form() << std::endl ;
      //
      *out << cgicc::fieldset();
      //
  }
  //
  void EmuPeripheralCrate::CrateConfiguration(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    //
    *out << "<a href=\"/\"><img border=\"0\" src=\"/daq/xgi/images/XDAQLogo.gif\" title=\"XDAQ\" alt=\"\" style=\"width: 145px; height: 89px;\"></a>" << std::endl;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial; background-color:#00FF00");
    //*out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial");
    *out << std::endl;
    //
    *out << cgicc::legend("Crate Configuration...").set("style","color:blue") << 
      cgicc::p() << std::endl ;
    //
    //*out << cgicc::body().set("bgcolor=yellow");
    //
    for(int ii=1; ii<22; ii++) {
      //
      *out << cgicc::table().set("border","1");
      //
      *out << cgicc::td();
	  //
      *out << "Slot " << setfill('0') << setw(2) << dec << ii << endl;
      //
      *out << cgicc::td();
      //
      char Name[50] ;
      std::string CCBStatus =
	toolbox::toString("/%s/CCBStatus",getApplicationDescriptor()->getURN().c_str());
      std::string CCBUtils =
	toolbox::toString("/%s/CCBUtils",getApplicationDescriptor()->getURN().c_str());
      std::string CCBBoardID =
	toolbox::toString("/%s/CCBBoardID",getApplicationDescriptor()->getURN().c_str());
      int slot = thisCrate->ccb()->slot() ;
      sprintf(Name,"CCB Status slot=%d",slot);
      if(slot == ii) {
	*out << cgicc::td();
	*out << "CCB Board ID" ;
	*out << cgicc::form().set("method","GET").set("action",CCBBoardID) << std::endl ;
	*out << cgicc::input().set("type","text").set("name","CCBBoardID")
	  .set("value",CCBBoardID_) << std::endl ;
	*out << cgicc::form() << std::endl ;
	*out << cgicc::td();
	//
	*out << cgicc::td();
	//
	if ( CCBBoardID_.find("-1") == string::npos ) {
	  //
	  //*out << cgicc::form().set("method","GET").set("action",CCBStatus)
	  //.set("target","_blank") << std::endl ;
	  //*out << cgicc::input().set("type","submit").set("value",Name) << std::endl ;
	  //char buf[20];
	  //sprintf(buf,"%d",ii);
	  //*out << cgicc::input().set("type","hidden").set("value",buf).set("name","ccb");
	  //*out << cgicc::form() << std::endl ;
	  //
	  std::string CCBStatus =
	    toolbox::toString("/%s/CCBStatus?ccb=%d",getApplicationDescriptor()->getURN().c_str(),ii);
	  //
	  *out << cgicc::a("CCB Status").set("href",CCBStatus) << endl;
	  //
	}
	*out << cgicc::td();
	//
	*out << cgicc::td();
	//
	if ( CCBBoardID_.find("-1") == string::npos ) {
	  //
	  sprintf(Name,"CCB Utils slot=%d",slot);
	  //
	  //*out << cgicc::form().set("method","GET").set("action",CCBUtils)
	  //.set("target","_blank") << std::endl ;
	  //*out << cgicc::input().set("type","submit").set("value",Name) << std::endl ;
	  //char buf[20];
	  //sprintf(buf,"%d",ii);
	  //*out << cgicc::input().set("type","hidden").set("value",buf).set("name","ccb");
	  //*out << cgicc::form() << std::endl ;
	  //
	  std::string CCBUtils =
	    toolbox::toString("/%s/CCBUtils?ccb=%d",getApplicationDescriptor()->getURN().c_str(),ii);
	  //
	  *out << cgicc::a("CCB Utils").set("href",CCBUtils) << endl;
	  //
	}
	*out << cgicc::td();
	//
      }
      //
      std::string MPCStatus =
	toolbox::toString("/%s/MPCStatus",getApplicationDescriptor()->getURN().c_str());
      std::string MPCBoardID =
	toolbox::toString("/%s/MPCBoardID",getApplicationDescriptor()->getURN().c_str());
      slot = -1;
      if ( thisMPC ) slot = thisCrate->mpc()->slot() ;
      sprintf(Name,"MPC Status slot=%d",slot);
      if(slot == ii) {
	//
	*out << cgicc::td();
	*out << "MPC Board ID" ;
	*out << cgicc::form().set("method","GET").set("action",MPCBoardID) << std::endl ;
	*out << cgicc::input().set("type","text").set("name","MPCBoardID")
	  .set("value",MPCBoardID_) << std::endl ;
	*out << cgicc::form() << std::endl ;
	*out << cgicc::td();
	//
	*out << cgicc::td();
	if ( MPCBoardID_.find("-1") == string::npos && thisMPC ) {
	  //
	  //*out << cgicc::form().set("method","GET").set("action",MPCStatus)
	  //.set("target","_blank") << std::endl ;
	  //*out << cgicc::input().set("type","submit").set("value",Name) << std::endl ;
	  //char buf[20];
	  //sprintf(buf,"%d",ii);
	  //*out << cgicc::input().set("type","hidden").set("value",buf).set("name","mpc");
	  //*out << cgicc::form() << std::endl ;
	  //
	  std::string MPCStatus =
	    toolbox::toString("/%s/MPCStatus?mpc=%d",getApplicationDescriptor()->getURN().c_str(),ii);
	  //
	  *out << cgicc::a("MPC Status").set("href",MPCStatus) << endl;
	  //
	}
	*out << cgicc::td();
      }
      //
      std::string TMBStatus ;
      std::string TMBTests ;
      std::string TMBUtils ;
      std::string TMBBoardID ;
      std::string RATBoardID ;
      //
      TMBStatus  =
	toolbox::toString("/%s/TMBStatus",getApplicationDescriptor()->getURN().c_str());
      TMBBoardID =
	toolbox::toString("/%s/TMBBoardID",getApplicationDescriptor()->getURN().c_str());
      RATBoardID =
	toolbox::toString("/%s/RATBoardID",getApplicationDescriptor()->getURN().c_str());
      TMBTests   =
	toolbox::toString("/%s/TMBTests",getApplicationDescriptor()->getURN().c_str());
      TMBUtils   =
	toolbox::toString("/%s/TMBUtils",getApplicationDescriptor()->getURN().c_str());
      //
      for (unsigned int i=0; i<tmbVector.size(); i++) {
	//
	int slot = tmbVector[i]->slot();
	if(slot == ii) {
	  //
	  *out << cgicc::td();
	  *out << "TMB Board ID" ;
	  *out << cgicc::form().set("method","GET").set("action",TMBBoardID) << std::endl ;
	  char buf[20];
	  sprintf(buf,"TMBBoardID_%d",i);
	  *out << cgicc::input().set("type","text").set("name",buf)
	    .set("value",TMBBoardID_[i]) << std::endl ;
	  sprintf(buf,"%d",i);
	  *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
	  *out << cgicc::form() << std::endl ;
	  *out << cgicc::td();
	  //
	  *out << cgicc::td();
	  *out << "RAT Board ID" ;
	  *out << cgicc::form().set("method","GET").set("action",RATBoardID) << std::endl ;
	  //
	  sprintf(buf,"RATBoardID_%d",i);
	  *out << cgicc::input().set("type","text").set("name",buf)
	    .set("value",RATBoardID_[i]) << std::endl ;
	  sprintf(buf,"%d",i);
	  *out << cgicc::input().set("type","hidden").set("value",buf).set("name","rat");
	  *out << cgicc::form() << std::endl ;
	  *out << cgicc::td();
	  //
	  sprintf(Name,"TMB Status slot=%d",tmbVector[i]->slot());	
	  *out << cgicc::td();
	  if ( TMBBoardID_[i].find("-1") == string::npos ) {
	    //
	    //*out << cgicc::form().set("method","GET").set("action",TMBStatus)
	    //.set("target","_blank") << std::endl ;
	    //*out << cgicc::input().set("type","submit").set("value",Name) << std::endl ;
	    //sprintf(buf,"%d",i);
	    //*out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
	    //*out << cgicc::form() << std::endl ;
	    //
	    std::string TMBStatus =
	      toolbox::toString("/%s/TMBStatus?tmb=%d",getApplicationDescriptor()->getURN().c_str(),i);
	    //
	    *out << cgicc::a("TMB Status").set("href",TMBStatus) << endl;
	    //
	  }
	  *out << cgicc::td();
	  //
	  sprintf(Name,"TMB Tests slot=%d",tmbVector[i]->slot());	  
	  *out << cgicc::td();
	  if ( TMBBoardID_[i].find("-1") == string::npos ) {
	    //
	    //*out << cgicc::form().set("method","GET").set("action",TMBTests)
	    //.set("target","_blank") << std::endl ;
	    //*out << cgicc::input().set("type","submit").set("value",Name) << std::endl ;
	    //sprintf(buf,"%d",i);
	    //*out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
	    //*out << cgicc::form() << std::endl ;
	    //
	    std::string TMBTests =
	      toolbox::toString("/%s/TMBTests?tmb=%d",getApplicationDescriptor()->getURN().c_str(),i);
	    //
	    *out << cgicc::a("TMB Tests").set("href",TMBTests) << endl;
	    //
	  }
	  *out << cgicc::td();
	  //
	  sprintf(Name,"TMB Utils slot=%d",tmbVector[i]->slot());	  
	  *out << cgicc::td();
	  if ( TMBBoardID_[i].find("-1") == string::npos ) {
	    //
	    //*out << cgicc::form().set("method","GET").set("action",TMBUtils)
	    //.set("target","_blank") << std::endl ;
	    //*out << cgicc::input().set("type","submit").set("value",Name) << std::endl ;
	    //sprintf(buf,"%d",i);
	    //*out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
	    //*out << cgicc::form() << std::endl ;	    
	    //
	    std::string TMBUtils =
	      toolbox::toString("/%s/TMBUtils?tmb=%d",getApplicationDescriptor()->getURN().c_str(),i);
	    //
	    *out << cgicc::a("TMB Utils").set("href",TMBUtils) << endl;
	    //
	  }
	  *out << cgicc::td();
	  //
	  //Found TMB...look for DMB...
	  //
	  for (unsigned int iii=0; iii<dmbVector.size(); iii++) {
	    int dmbslot = dmbVector[iii]->slot();
	    std::string ChamberTests =
	      toolbox::toString("/%s/ChamberTests",getApplicationDescriptor()->getURN().c_str());    
	    sprintf(Name,"Chamber tests TMBslot=%d DMBslot=%d",slot,dmbslot);
	    //
	    if ( dmbslot == slot+1 ) {
	      *out << cgicc::td();
	      if ( TMBBoardID_[i].find("-1") == string::npos ) {
		//
		//*out << cgicc::form().set("method","GET").set("action",ChamberTests)
		//.set("target","_blank") << std::endl ;
		//*out << cgicc::input().set("type","submit").set("value",Name) << std::endl ;
		//char buf[20];
		//sprintf(buf,"%d",i);
		//*out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
		//sprintf(buf,"%d",iii);
		//*out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
		//*out << cgicc::form() << std::endl ;
		//
		std::string ChamberTests =
		  toolbox::toString("/%s/ChamberTests?tmb=%d&dmb=%d",getApplicationDescriptor()->getURN().c_str(),i,iii);
		//
		*out << cgicc::a(Name).set("href",ChamberTests) << endl;
		//
		MyTest[i].SetTMB(tmbVector[i]);
		MyTest[i].SetDMB(dmbVector[iii]);
		MyTest[i].SetCCB(thisCCB);
		MyTest[i].SetMPC(thisMPC);
		//
	      }
	      *out << cgicc::td();
	    }
	      }
	  //
	}
      }
	  //
      std::string DMBStatus;
      std::string DMBTests;
      std::string DMBUtils;
      std::string DMBBoardID;
      //
      for (unsigned int i=0; i<dmbVector.size(); i++) {
	    DMBStatus =
	      toolbox::toString("/%s/DMBStatus",getApplicationDescriptor()->getURN().c_str());
	    DMBTests =
	      toolbox::toString("/%s/DMBTests",getApplicationDescriptor()->getURN().c_str());
	    DMBUtils =
	      toolbox::toString("/%s/DMBUtils",getApplicationDescriptor()->getURN().c_str());
	    DMBBoardID =
	      toolbox::toString("/%s/DMBBoardID",getApplicationDescriptor()->getURN().c_str());
	    int slot = dmbVector[i]->slot();
	    if(slot == ii ) {
	      //
	      *out << cgicc::td();
	      *out << "DMB Board ID" ;
	      *out << cgicc::form().set("method","GET").set("action",DMBBoardID) << std::endl ;
	      char buf[20];
	      sprintf(buf,"DMBBoardID_%d",i);
	      *out << cgicc::input().set("type","text").set("name",buf)
		.set("value",DMBBoardID_[i]) << std::endl ;
	      sprintf(buf,"%d",i);
	      *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
	      *out << cgicc::form() << std::endl ;
	      *out << cgicc::td();
	      //
	      sprintf(Name,"DMB Status slot=%d",dmbVector[i]->slot());
	      *out << cgicc::td();
	      if ( DMBBoardID_[i].find("-1",0) == string::npos ) {
		//
		//*out << cgicc::form().set("method","GET").set("action",DMBStatus)
		//.set("target","_blank") << std::endl ;
		//*out << cgicc::input().set("type","submit").set("value",Name) << std::endl ;
		//sprintf(buf,"%d",i);
		//*out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
		//*out << cgicc::form() << std::endl ;
		//
		std::string DMBStatus =
		  toolbox::toString("/%s/DMBStatus?dmb=%d",getApplicationDescriptor()->getURN().c_str(),i);
		//
		*out << cgicc::a("DMB Status").set("href",DMBStatus) << endl;
		//
	      }
	      *out << cgicc::td();
	      //
	      sprintf(Name,"DMB Tests  slot=%d",dmbVector[i]->slot());
	      //
	      *out << cgicc::td();
	      if ( DMBBoardID_[i].find("-1",0) == string::npos ) {
		//
		//*out << cgicc::form().set("method","GET").set("action",DMBTests)
		//.set("target","_blank") << std::endl ;
		//*out << cgicc::input().set("type","submit").set("value",Name) << std::endl ;
		//sprintf(buf,"%d",i);
		//*out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
		//*out << cgicc::form() << std::endl ;
		//
		std::string DMBTests =
		  toolbox::toString("/%s/DMBTests?dmb=%d",getApplicationDescriptor()->getURN().c_str(),i);
		//
		*out << cgicc::a("DMB Tests").set("href",DMBTests) << endl;
		//
	      }
	      *out << cgicc::td();
	      //
	      sprintf(Name,"DMB Utils  slot=%d",dmbVector[i]->slot());
	      //
	      *out << cgicc::td();
	      if ( DMBBoardID_[i].find("-1",0) == string::npos ) {
		//*out << cgicc::form().set("method","GET").set("action",DMBUtils)
		//.set("target","_blank") << std::endl ;
		//*out << cgicc::input().set("type","submit").set("value",Name) << std::endl ;
		//sprintf(buf,"%d",i);
		//*out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
		//*out << cgicc::form() << std::endl ;
		//
		std::string DMBUtils =
		  toolbox::toString("/%s/DMBUtils?dmb=%d",getApplicationDescriptor()->getURN().c_str(),i);
		//
		*out << cgicc::a("DMB Utils").set("href",DMBUtils) << endl;
		//
	      }
	      *out << cgicc::td();
	    //
	    }
	  }
	//
	//*out<< cgicc::br() ;
	//
	  *out << cgicc::table();
	  //
	}
	//
	//*out << cgicc::body();
	*out << cgicc::fieldset();
	//
  }
  //
  void EmuPeripheralCrate::CalibrationALCT(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    CalibDAQ calib;
    int npulses = 100;
    for (int ii=0; ii<npulses; ii++) {
      //
      calib.pulseAllWires();
      //
      cout << npulses << endl;
      std::cout << "Pulse..." << std::endl;
      std::cout << std::endl;
      //
      this->Default(in,out);
      //
    }
  }
  //
  void EmuPeripheralCrate::FindLv1aDelayComparator(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    CalibDAQ calib;
    //
    calib.FindL1aDelayComparator();
    //
    this->Default(in,out);
      //
  }
  //
  void EmuPeripheralCrate::CalibrationRandomWiresALCT(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    CrateSelector theSelector;
    std::vector<Crate*> myCrates = theSelector.crates();
    //
    for(unsigned j = 0; j < myCrates.size(); ++j) {
      //
      (myCrates[j]->chamberUtilsMatch())[0].CCBStartTrigger();
      //
      ::sleep(1);
      //
    }
    //
    CalibDAQ calib;
    int npulses = 100;
    for (int ii=0; ii<npulses; ii++) {
      //
      calib.pulseRandomWires();
      //
      cout << npulses << endl;
      std::cout << "Pulse..." << std::endl;
      std::cout << std::endl;
      //
      this->Default(in,out);
      //
    }
    //
  }
  //
  void EmuPeripheralCrate::CalibrationCFEBCharge(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    CalibDAQ calib;
    //calib.loadConstants();
    //
    //int nsleep, nstrip, tries, counter =0;
    float dac;
    //int nsleep = 100000;  
    dac = 1.0;
    /*
    for (int i=0;i<16;i++) {  
      for (int ntim=0;ntim<20;ntim++) {
	calib.pulseAllDMBs(ntim, i, dac, nsleep);  
	counter++;
	std::cout << "dac = " << dac <<
	  "  strip = " << i <<
	  "  ntim = " << ntim <<
	  "  event  = " << counter << std::endl;
      }
    }
    */
    //
    calib.rateTest();
    //
    this->Default(in,out);
    //
  }
  //
  void EmuPeripheralCrate::CalibrationCFEBTime(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    CalibDAQ calib;
    //calib.loadConstants();
    //
    //int nsleep, nstrip, tries, counter =0;
    float dac;
    int counter=0;
    int nsleep = 100;  
    dac = 1.0;
    //
    for (int i=0;i<16;i++) {  
      for (int ntim=0;ntim<20;ntim++) {
	calib.pulseAllDMBs(ntim, i, dac, nsleep);  
	counter++;
	std::cout << "dac = " << dac <<
	  "  strip = " << i <<
	  "  ntim = " << ntim <<
	  "  event  = " << counter << std::endl;
      }
    }
    //
    this->Default(in,out);
    //
  }
  //
  void EmuPeripheralCrate::CalibrationComparatorPulse(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    CalibDAQ calib;
    //
    calib.pulseComparatorPulse();
    //
    this->Default(in,out);
    //
  }
  //
  void EmuPeripheralCrate::CalibrationCFEBPedestal(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    CalibDAQ calib;
    calib.loadConstants();
    //
    //int nsleep, nstrip, tries;
    //float dac;
    //
    calib.pedestalCFEB();
    //
    this->Default(in,out);
    //
  }
  //
  void EmuPeripheralCrate::LaunchMonitor(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eFrames) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    //
    std::string MonitorFrameLeft =
      toolbox::toString("/%s/MonitorFrameLeft",getApplicationDescriptor()->getURN().c_str());
    std::string MonitorFrameRight =
      toolbox::toString("/%s/MonitorFrameRight",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::frameset().set("cols","200,*");
    *out << cgicc::frame().set("src",MonitorFrameLeft);
    *out << cgicc::frame().set("src",MonitorFrameRight);
    *out << cgicc::frameset() ;
    //
    //
    //std::string ResetAllCounters =
    //toolbox::toString("/%s/ResetAllCounters",getApplicationDescriptor()->getURN().c_str());
    //
    //*out << cgicc::form().set("method","GET").set("action",ResetAllCounters) << std::endl ;
    //*out << cgicc::input().set("type","submit").set("value","Reset All Counters") << std::endl ;
    //*out << cgicc::form() << std::endl ;
    //
    //::sleep(1);
    //
    //this->LaunchMonitor(in,out);
    //
  }
  //
  void EmuPeripheralCrate::MonitorFrameRight(xgi::Input * in, xgi::Output * out) 
    throw (xgi::exception::Exception){
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    //
    for(int counter=0; counter<22; counter++) {
      //
      Counter_ = counter;
      //
      std::string CreateMonitorUnit =
	toolbox::toString("/%s/CreateMonitorUnit?counter=%d",getApplicationDescriptor()->getURN().c_str(),counter);
      //
      //*out << cgicc::frame().set("src",CreateMonitorUnit)<<std::endl;
      //
      //
      this->CreateMonitorUnit(in,out);
      //
    }
    //
  }
  //
  void EmuPeripheralCrate::MonitorFrameLeft(xgi::Input * in, xgi::Output * out) 
    throw (xgi::exception::Exception){
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    //
    std::string ResetAllCounters =
      toolbox::toString("/%s/ResetAllCounters",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",ResetAllCounters) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Reset All Counters") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    this->LaunchMonitor(in,out);
    //
  }
  //
  void EmuPeripheralCrate::CreateMonitorUnit(xgi::Input * in, xgi::Output * out) 
    throw (xgi::exception::Exception){
    //
      *out << cgicc::fieldset().set("style","font-size: 8pt; font-family: arial;");
      //
      //
      cgicc::Cgicc cgi(in);
      cgicc::form_iterator name = cgi.getElement("counter");
      name = cgi.getElement("counter");
      //
      if(name != cgi.getElements().end()) {
	Counter_ = cgi["counter"]->getIntegerValue();
      } else {
	cout << "No counter" << endl;
      }
      //
      ChartData[Counter_].clear();
      //
      *out << "Counter= " << Counter_ << std::endl;
      *out << cgicc::br();
      //
      for(unsigned int i=0; i<tmbVector.size(); i++) {
	//
	tmbVector[i]->RedirectOutput(out);
	tmbVector[i]->GetCounters();
	//
	if ( tmbVector[i]->GetCounter(4)>0) {
	  ChartData[Counter_].push_back((float)(tmbVector[i]->GetCounter(Counter_))/(tmbVector[i]->GetCounter(4)));
	} 	  
	//
	if ( Counter_ == 0 ) {
	  if ( tmbVector[i]->GetCounter(0) > 0 ) {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	  //
	} else if ( Counter_ == 1 ) {
	  if ( tmbVector[i]->GetCounter(1) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 2 ) {
	  if ( tmbVector[i]->GetCounter(2) > 0 ) {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 3 ) {
	  if ( tmbVector[i]->GetCounter(3) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 4 ) {
	  if ( tmbVector[i]->GetCounter(4) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 5 ) {
	  if ( tmbVector[i]->GetCounter(5) > 0 ) {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 6 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 7 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 8 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 9 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 10 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 11 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 12 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 13 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 14 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 15 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 16 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 17 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 18 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 19 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 20 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else if ( Counter_ == 21 ) {
	  if ( tmbVector[i]->GetCounter(Counter_) > 0 ) {
	    *out << cgicc::span().set("style","color:green");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  } else {
	    *out << cgicc::span().set("style","color:red");
	    tmbVector[i]->PrintCounters(Counter_);
	    *out << cgicc::span();
	    *out << cgicc::br();
	  }
	} else {
	  //
	  *out << cgicc::pre() ;
	  tmbVector[i]->PrintCounters(Counter_);
	  *out << cgicc::pre() ;
	  //
	}
	//
	tmbVector[i]->RedirectOutput(&std::cout);
	//
	//cgicc::br();
	//
      }
      //
      this->Display(in,out,Counter_);
      //
      *out << cgicc::fieldset();    
    //
  }
  //
  void EmuPeripheralCrate::Display(xgi::Input * in, xgi::Output * out, int counter ) 
    throw (xgi::exception::Exception)
  {
    //
    *out << "<HTML>" <<std::endl;
    *out << "<BODY bgcolor=\"#FFFFFF\">" <<std::endl;
    *out << "<OBJECT classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,0,0\" WIDTH=\"565\" HEIGHT=\"420\" id=\"FC_2_3_Bar2D\">" <<std::endl;
    *out << "<PARAM NAME=movie VALUE=\"/daq/extern/FusionCharts/Charts/FC_2_3_Bar2D.swf\">" <<std::endl;
    //
    //*out << "<PARAM NAME=\"FlashVars\" VALUE=\"&dataURL=getData\">"<<std::endl;
    ostringstream output;
    output << "<PARAM NAME=\"FlashVars\" VALUE=\"&dataURL=getData" << counter <<"\">"<<std::endl;
    //
    std::cout << output.str() << std::endl;
    *out << output.str() << std::endl ;
    *out << "<PARAM NAME=quality VALUE=high>" << std::endl ;
    *out << "<PARAM NAME=bgcolor VALUE=#FFFFFF>" << std::endl ;
    //*out << "<EMBED src=\"/daq/extern/FusionCharts/Charts/FC_2_3_Bar2D.swf\" FlashVars=\"&dataURL=getData\" quality=high bgcolor=#FFFFFF WIDTH=\"565\" HEIGHT=\"420\" NAME=\"FC_2_3_Bar2D\" TYPE=\"application/x-shockwave-flash\" PLUGINSPAGE=\"http://www.macromedia.com/go/getflashplayer\"></EMBED>" << std::endl;
    //
    ostringstream output2;
    output2 << "<EMBED src=\"/daq/extern/FusionCharts/Charts/FC_2_3_Bar2D.swf\" FlashVars=\"&dataURL=getData"<< counter <<"\" quality=high bgcolor=#FFFFFF WIDTH=\"565\" HEIGHT=\"420\" NAME=\"FC_2_3_Bar2D\" TYPE=\"application/x-shockwave-flash\" PLUGINSPAGE=\"http://www.macromedia.com/go/getflashplayer\"></EMBED>" << std::endl;
    //
    *out << output2.str() << std::endl;
    //
    *out << "</OBJECT>" << std::endl;
    *out << "</BODY>" << std::endl;
    *out << "</HTML>" << std::endl;
    //
  }
  //
  void EmuPeripheralCrate::getData0(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='ALCT: CRC error' subcaption='Normalized' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1' animation='0'>" << std::endl;
    //
    std::cout << ChartData[0].size() << endl ;
    for(unsigned int i=0;i<ChartData[0].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[0][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData1(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='ALCT: LCT sent to TMB' subcaption='Normalized' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[1].size() << endl ;
    for(unsigned int i=0;i<ChartData[1].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[1][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData2(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='ALCT: LCT error' subcaption='Normalized' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[2].size() << endl ;
    for(unsigned int i=0;i<ChartData[2].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[2][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData3(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='ALCT: L1A readout' subcaption='Normalized' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[3].size() << endl ;
    for(unsigned int i=0;i<ChartData[3].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[3][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData4(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='CLCT Pretrigger' subcaption='Normalized' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[4].size() << endl ;
    for(unsigned int i=0;i<ChartData[4].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[4][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData5(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='CLCT Pretrig but no wbuf' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[5].size() << endl ;
    for(unsigned int i=0;i<ChartData[5].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[5][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData6(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='CLCT: Invalid pattern after drift 0' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[6].size() << endl ;
    for(unsigned int i=0;i<ChartData[6].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[6][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData7(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='TMB matching rejected' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[7].size() << endl ;
    for(unsigned int i=0;i<ChartData[7].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[7][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData8(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='CLCT or ALCT or both triggered' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[8].size() << endl ;
    for(unsigned int i=0;i<ChartData[8].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[8][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData9(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='TMB: CLCT or ALCT or both trigger xmit MPC' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[9].size() << endl ;
    for(unsigned int i=0;i<ChartData[9].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[9][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData10(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='TMB: CLCT and ALCT matched in time' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[10].size() << endl ;
    for(unsigned int i=0;i<ChartData[10].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[10][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData11(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='TMB: ALCT-only trigger' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[11].size() << endl ;
    for(unsigned int i=0;i<ChartData[11].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[11][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData12(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='CLCT-only trigger' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[12].size() << endl ;
    for(unsigned int i=0;i<ChartData[12].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[12][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData13(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='TMB: No trig pulse response' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    std::cout << ChartData[13].size() << endl ;
    for(unsigned int i=0;i<ChartData[13].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[13][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData14(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='TMB: No MPC transmission' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    for(unsigned int i=0;i<ChartData[14].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[14][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData15(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='No MPC response FF pulse' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    for(unsigned int i=0;i<ChartData[15].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[15][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData16(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='MPC accepted LCT0' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    for(unsigned int i=0;i<ChartData[16].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[16][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData17(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='MPC accepted LCT1' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    for(unsigned int i=0;i<ChartData[17].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[17][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
      std::cout << output.str() << std::endl;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData18(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='L1A received' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    for(unsigned int i=0;i<ChartData[18].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[18][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData19(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='L1A: TMB triggered, TMB in L1A window' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    for(unsigned int i=0;i<ChartData[19].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[19][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData20(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='L1A: L1A received, no TMB in window' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    for(unsigned int i=0;i<ChartData[20].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[20][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::getData21(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    *out << "<graph caption='CLCT pre-trigger' subcaption='Default' xAxisName='Board' yAxisName='Rate' numberPrefix='' showNames='1'>" << std::endl;
    //
    for(unsigned int i=0;i<ChartData[21].size();i++) {
      ostringstream output;
      output << "<set name='" << i <<"'"<< " value='" << ChartData[21][i] << "'" << " />" << std::endl;
      *out << output.str() << std::endl ;
    }
    *out << "</graph>" << std::endl;    
  }
  //
  void EmuPeripheralCrate::ResetAllCounters(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    for(unsigned int i=0; i<tmbVector.size(); i++) {
      //
      //*out << cgicc::fieldset().set("style","font-size: 8pt; font-family: arial;");
      //
      tmbVector[i]->ResetCounters();
      //
      //*out << cgicc::fieldset();    
      //
    }
    //
    this->MonitorFrameLeft(in,out);
    //
  }
  //
  void EmuPeripheralCrate::InitSystem(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    cout << "Init System" << endl ;
    //
    MyController->configure();          // Init system
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("navigator");
    //
    int navigator;
    if(name != cgi.getElements().end()) {
      navigator = cgi["navigator"]->getIntegerValue();
      cout << "Navigator " << navigator << endl;
      if ( navigator == 1 ) {
	thisCCB->setCCBMode(CCB::VMEFPGA);      // It needs to be in FPGA mode to work.
	this->ChamberTests(in,out);
      }
    } else {
      cout << "No navigator" << endl;
      this->Default(in,out);
    }
  }
  //
  void EmuPeripheralCrate::InitChamber(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "Init Chamber" << endl ;
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
      dmb = DMB_;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
      tmb = TMB_;
    }
    //
    //DAQMB * thisDMB = dmbVector[dmb];
    //TMB * thisTMB = tmbVector[tmb];
    //
    //MyTest.SetTMB(thisTMB);
    //MyTest.SetDMB(thisDMB);
    //MyTest.SetCCB(thisCCB);
    //
    MyTest[tmb].InitSystem();          // Init chamber
    //
    thisCCB->setCCBMode(CCB::VMEFPGA);      // It needs to be in FPGA mode to work.
    //
    this->ChamberTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::CCBBoardID(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    CCBBoardID_= cgi["CCBBoardID"]->getValue() ;
    //
    this->Default(in,out);
    //
  }
  //
  void EmuPeripheralCrate::TmbMPCTest(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    CrateUtilities myCrateTest;
    myCrateTest.SetCrate(thisCrate);
    myCrateTest.MpcTMBTest(1000);
    //
    this->Default(in,out);
    //
  }
  //
  void EmuPeripheralCrate::MPCBoardID(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    MPCBoardID_= cgi["MPCBoardID"]->getValue() ;
    //
    this->Default(in,out);
    //
  }
  //
  void EmuPeripheralCrate::PowerUp(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    *out << h1("Run Valery's program");
    *out << cgicc::a("Voltages").set("href","http://emuslice03:1973/urn:xdaq-application:lid=30/") << endl;
    //
  }
  //
  void EmuPeripheralCrate::Operator(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    cgicc::Cgicc cgi(in);
    //
    Operator_= cgi["Operator"]->getValue() ;
    //
    this->Default(in,out);
  }
  //
  void EmuPeripheralCrate::RunNumber(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    cgicc::Cgicc cgi(in);
    //
    RunNumber_= cgi["RunNumber"]->getValue() ;
    std::cout << "RunNumber " << RunNumber_ << std::endl ;
    //
    this->Default(in,out);
  }
  //
  void EmuPeripheralCrate::DMBTestAll(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    OutputDMBTests[dmb] << "DMB Test3" << endl ;
    //
    thisDMB->RedirectOutput(&OutputDMBTests[dmb]);
    //thisDMB->test3();
    thisDMB->test4();
    thisDMB->test5();
    thisDMB->test6();
    thisDMB->test8();
    thisDMB->test9();
    thisDMB->test10();
    thisDMB->RedirectOutput(&std::cout);
    //
    this->DMBTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::DMBTest3(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    OutputDMBTests[dmb] << "DMB Test3" << endl ;
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    thisDMB->RedirectOutput(&std::cout);
    thisDMB->test3();
    thisDMB->RedirectOutput(&std::cout);
    //
    this->DMBTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::DMBTest4(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    OutputDMBTests[dmb] << "DMB Test4" << endl ;
    //
    thisDMB->RedirectOutput(&OutputDMBTests[dmb]);
    thisDMB->test4();
    thisDMB->RedirectOutput(&std::cout);
    //
    this->DMBTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::DMBTest5(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    OutputDMBTests[dmb] << "DMB Test5" << endl ;
    //
    thisDMB->RedirectOutput(&OutputDMBTests[dmb]);
    thisDMB->test5();
    thisDMB->RedirectOutput(&std::cout);
    //
    this->DMBTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::DMBTest6(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    OutputDMBTests[dmb] << "DMB Test6" << endl ;
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    thisDMB->RedirectOutput(&OutputDMBTests[dmb]);
    thisDMB->test6();
    thisDMB->RedirectOutput(&std::cout);
    //
    this->DMBTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::DMBTest8(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    OutputDMBTests[dmb] << "DMB Test8" << endl ;
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    thisDMB->RedirectOutput(&OutputDMBTests[dmb]);
    thisDMB->test8();
    thisDMB->RedirectOutput(&std::cout);
    //
    this->DMBTests(in,out);
  }
  //
  void EmuPeripheralCrate::DMBTest9(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    OutputDMBTests[dmb] << "DMB Test9" << endl ;
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    thisDMB->RedirectOutput(&OutputDMBTests[dmb]);
    thisDMB->test9();
    thisDMB->RedirectOutput(&std::cout);
    //
    this->DMBTests(in,out);
  }
  //
  void EmuPeripheralCrate::DMBTest10(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    OutputDMBTests[dmb] << "DMB Test10" << endl ;
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    thisDMB->RedirectOutput(&OutputDMBTests[dmb]);
    thisDMB->test10();
    thisDMB->RedirectOutput(&std::cout);
    //
    this->DMBTests(in,out);
  }
  //
  void EmuPeripheralCrate::DMBTest11(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    OutputDMBTests[dmb] << "DMB Test11" << endl ;
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    thisDMB->RedirectOutput(&std::cout);
    thisDMB->test11();
    thisDMB->RedirectOutput(&std::cout);
    //
    this->DMBTests(in,out);
  }
  //
  void EmuPeripheralCrate::DMBPrintCounters(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    thisDMB->RedirectOutput(&std::cout);
    thisDMB->PrintCounters(1);
    thisDMB->RedirectOutput(&std::cout);
    //
    this->DMBUtils(in,out);
  }
  //
  void EmuPeripheralCrate::ChamberTests(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    //*out << cgicc::HTTPHTMLHeader();
    //
    cgicc::Cgicc cgi(in);
    //
    const CgiEnvironment& env = cgi.getEnvironment();
    //
    std::string crateStr = env.getQueryString() ;
    //
    cout << crateStr << endl ;
    //
    int tmb, dmb;
    //
    //sscanf(crateStr.c_str(),"tmb=%d,dmb=%d",&tmb,&dmb);
    //
    //cout << tmb << " " << dmb << endl;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
      dmb = DMB_;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
      tmb = TMB_;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    DAQMB * thisDMB = dmbVector[dmb];
    //
    char Name[50];
    sprintf(Name,"Chamber tests TMBslot=%d DMBslot=%d",thisTMB->slot(),thisDMB->slot());
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    *out << cgicc::title(Name) << std::endl;
    //
    *out << cgicc::h1(Name);
    *out << cgicc::br();
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << std::endl;
    //
    *out << cgicc::legend("Crate Tests").set("style","color:blue") << cgicc::p() << std::endl ;
    //
    char buf[20];
    //
    std::string Automatic =
      toolbox::toString("/%s/Automatic",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",Automatic) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Automatic") << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    std::string InitChamber =
      toolbox::toString("/%s/InitChamber",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",InitChamber) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Init Chamber") << std::endl ;
    *out << cgicc::input().set("type","hidden").set("value","1").set("name","navigator");
    *out << cgicc::form() << std::endl ;
    //
    std::string TMBStartTrigger =
      toolbox::toString("/%s/TMBStartTrigger",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",TMBStartTrigger) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","TMBStartTrigger") << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    std::string EnableL1aRequest =
      toolbox::toString("/%s/EnableL1aRequest",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",EnableL1aRequest) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","EnableL1aRequest") << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    std::string ALCTTiming =
      toolbox::toString("/%s/ALCTTiming",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",ALCTTiming) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","ALCT Timing") << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::pre();
    *out << "ALCT rx Phase = " << MyTest[tmb].GetALCTrxPhaseTest() <<  " (" << MyTest[tmb].GetALCTrxPhase() 
	 << ") " << std::endl;
    //
    *out << "ALCT tx Phase = " << MyTest[tmb].GetALCTtxPhaseTest() <<  " (" << MyTest[tmb].GetALCTtxPhase()
	 << ") " << std::endl;
    *out << cgicc::pre();
    //
    std::string CFEBTiming =
      toolbox::toString("/%s/CFEBTiming",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",CFEBTiming) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","CFEB Timing") << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::pre();
    for(int i=0;i<5;i++) *out << "CFEB " << i << " rx Phase = " << 
			   MyTest[tmb].GetCFEBrxPhaseTest(i) << " ("  << 
			   MyTest[tmb].GetCFEBrxPhase(i)     << ") " <<
			   std::endl;
    *out << cgicc::pre();
    //
    alct = thisTMB->alctController();
    //
    if ( alct ) {
      //
      std::string ALCTScan =
	toolbox::toString("/%s/ALCTScan",getApplicationDescriptor()->getURN().c_str());
      //
      *out << cgicc::form().set("method","GET").set("action",ALCTScan) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","ALCT Scan") << std::endl ;
      char buf[20];
      sprintf(buf,"%d",tmb);
      *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
      sprintf(buf,"%d",dmb);
      *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
      *out << cgicc::form() << std::endl ;
      //
      cgicc::pre();
      for (int Wire = 0; Wire<(alct->GetWGNumber())/6; Wire++) {
	*out << MyTest[tmb].GetALCTWireScan(Wire) ;
      }
      *out << cgicc::br();
      *out << std::endl;
      //
      cgicc::pre();
      //
    }
    //
    std::string CFEBScan =
      toolbox::toString("/%s/CFEBScan",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",CFEBScan) << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::input().set("type","submit").set("value","CFEB Scan") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    cgicc::pre();
    for (int CFEBs = 0; CFEBs<5; CFEBs++) {
      *out << "CFEB Id="<<CFEBs<< " " ;
      for (int HalfStrip = 0; HalfStrip<32; HalfStrip++) {
	*out << MyTest[tmb].GetCFEBStripScan(CFEBs,HalfStrip) ;
      }
      *out << cgicc::br();
      *out << std::endl;
    }
    cgicc::pre();
    //
    std::string FindWinner =
      toolbox::toString("/%s/FindWinner",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",FindWinner) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Find Winner bits") << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    std::string TMBL1aTiming =
      toolbox::toString("/%s/TMBL1aTiming",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",TMBL1aTiming) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","TMB L1a Timing") << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << MyTest[tmb].GetTMBL1aTiming() ;
    //
    std::string ALCTL1aTiming =
      toolbox::toString("/%s/ALCTL1aTiming",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",ALCTL1aTiming) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","ALCT L1a Timing") << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << MyTest[tmb].GetALCTL1aDelay() ;
    //
    std::string ALCTvpf =
      toolbox::toString("/%s/ALCTvpf",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",ALCTvpf) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Find ALCT vpf") << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << MyTest[tmb].GetALCTvpf() ;
    //
    *out << cgicc::br();
    //
    *out << cgicc::fieldset();
    *out << std::endl;
    //
    *out << cgicc::form().set("method","GET") << std::endl ;
    *out << cgicc::textarea().set("name","CrateTestOutput")
      .set("WRAP","OFF")
      .set("rows","20").set("cols","60");
    *out << CrateTestsOutput[tmb].str() << endl ;
    *out << cgicc::textarea();
    *out << cgicc::form() << std::endl ;
    //
  }
  //
  void EmuPeripheralCrate::TMBStartTrigger(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    //
    thisTMB->StartTTC();
    //
    this->ChamberTests(in,out);
    //
  }
  //
    void EmuPeripheralCrate::EnableL1aRequest(xgi::Input * in, xgi::Output * out ) 
      throw (xgi::exception::Exception)
  {
    //
    cout << "Enable L1a request" << endl ;
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    TMB * thisTMB = tmbVector[tmb];
    //
    std::cout << "DMB slot " << thisDMB->slot() << " TMB slot " << thisTMB->slot() << std::endl;
    //
    thisTMB->EnableL1aRequest();
    thisCCB->setCCBMode(CCB::VMEFPGA);
    //
    this->ChamberTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::ALCTTiming(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "ALCTTiming" << endl;
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
      dmb = DMB_;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
      tmb = TMB_;
    }
    //
    //DAQMB * thisDMB = dmbVector[dmb];
    //TMB * thisTMB = tmbVector[tmb];
    //
    //MyTest.SetTMB(thisTMB);
    //MyTest.SetCCB(thisCCB);
    //
    //MyTest[tmb].RedirectOutput(&CrateTestsOutput[tmb]);
    MyTest[tmb].RedirectOutput(&std::cout);
    MyTest[tmb].ALCTTiming();
    MyTest[tmb].RedirectOutput(&std::cout);
    //
    this->ChamberTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::Automatic(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "Automatic" << endl;
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
      dmb = DMB_;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
      tmb = TMB_;
    }
    //
    //DAQMB * thisDMB = dmbVector[dmb];
    //TMB * thisTMB = tmbVector[tmb];
    //
    //MyTest.SetTMB(thisTMB);
    //MyTest.SetDMB(thisDMB);
    //MyTest.SetCCB(thisCCB);
    //
    //int RXphase, TXphase;
    //
    MyTest[tmb].Automatic();
    //
    this->ChamberTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::CFEBTiming(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "CFEBTiming" << endl;
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
      dmb = DMB_;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
      tmb = TMB_;
    }
    //
    //DAQMB * thisDMB = dmbVector[dmb];
    //TMB * thisTMB = tmbVector[tmb];
    //
    //MyTest.SetTMB(thisTMB);
    //MyTest.SetDMB(thisDMB);
    //MyTest.SetCCB(thisCCB);
    //
    MyTest[tmb].CFEBTiming();
    //
    this->ChamberTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::TMBL1aTiming(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "TMBL1aTiming" << endl;
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
      dmb = DMB_;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
      tmb = TMB_;
    }
    //
    //DAQMB * thisDMB = dmbVector[dmb];
    //TMB * thisTMB = tmbVector[tmb];
    //
    //MyTest.SetTMB(thisTMB);
    //MyTest.SetDMB(thisDMB);
    //MyTest.SetCCB(thisCCB);
    //
    MyTest[tmb].FindTMB_L1A_delay(50,200);
    //
    this->ChamberTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::ALCTL1aTiming(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "ALCTL1aTiming" << endl;
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
      dmb = DMB_;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
      tmb = TMB_;
    }
    //
    //DAQMB * thisDMB = dmbVector[dmb];
    //TMB * thisTMB = tmbVector[tmb];
    //
    //MyTest.SetTMB(thisTMB);
    //MyTest.SetDMB(thisDMB);
    //MyTest.SetCCB(thisCCB);
    //
    MyTest[tmb].FindALCT_L1A_delay(140,160);
    //
    this->ChamberTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::ALCTvpf(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "ALCTvpf" << endl;
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
    }
    //
    //DAQMB * thisDMB = dmbVector[dmb];
    //TMB * thisTMB = tmbVector[tmb];
    //
    //MyTest.SetTMB(thisTMB);
    //MyTest.SetDMB(thisDMB);
    //MyTest.SetCCB(thisCCB);
    //
    MyTest[tmb].FindALCTvpf();
    //
    this->ChamberTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::ALCTScan(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "ALCTScan" << endl;
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
    }
    //
    //DAQMB * thisDMB = dmbVector[dmb];
    //TMB * thisTMB = tmbVector[tmb];
    //
    //MyTest.SetTMB(thisTMB);
    //MyTest.SetCCB(thisCCB);
    //
    MyTest[tmb].ALCTChamberScan();
    //
    this->ChamberTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::CFEBScan(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "CFEBScan" << endl;
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
    }
    //
    //DAQMB * thisDMB = dmbVector[dmb];
    //TMB * thisTMB = tmbVector[tmb];
    //
    //MyTest.SetTMB(thisTMB);
    //MyTest.SetCCB(thisCCB);
    //MyTest.SetDMB(thisDMB);
    //
    MyTest[tmb].CFEBChamberScan();
    //
    this->ChamberTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::FindWinner(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "FindWinner" << endl;
    //
    cgicc::Cgicc cgi(in);
    //
    int tmb, dmb;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "No dmb" << endl;
    }
    //
    name = cgi.getElement("tmb");
    //
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "No tmb" << endl;
    }
    //
    //DAQMB * thisDMB = dmbVector[dmb];
    //TMB * thisTMB = tmbVector[tmb];
    //
    //MyTest.SetTMB(thisTMB);
    //MyTest.SetCCB(thisCCB);
    //MyTest.SetDMB(thisDMB);
    //MyTest.SetMPC(thisMPC);
    //
    MyTest[tmb].FindWinner(2);
    //
    this->ChamberTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::MakeReference(xgi::Input * in , xgi::Output * out )
  {
    //
    cgicc::Cgicc cgi(in);
    //
    const CgiEnvironment& env = cgi.getEnvironment();
    //
    std::string dmbStr = env.getQueryString() ;
    //int dmb = atoi(dmbStr.c_str());
    //
    std::string test =  env.getReferrer() ;
    cout << test << endl ;
    //
    *out << cgicc::a("Back").set("href",test) << endl;
  }
  //
  void EmuPeripheralCrate::DMBTurnOff(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    cout << "DMBTurnOff" << endl;
    //
    if (thisDMB) {
      thisDMB->lowv_onoff(0x0);
    }
    //
    this->DMBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::DMBLoadFirmware(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    cout << "DMBLoadFirmware" << endl;
    //
    thisCCB->hardReset();
    //
    ::sleep(2);
    //
    if (thisDMB) {
      //
      char *out;
      thisDMB->epromload(MPROM,"../svf/dmb6cntl_v18_r2.svf",1,out);  // load mprom
      //thisDMB->epromload(MPROM,"../svf/dmb6cntl_me11.svf",1,out);  // load mprom
    }
    //
    ::sleep(2);
    //
    thisCCB->hardReset();
    //
    this->DMBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::CFEBLoadFirmware(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    int dmbNumber = -1;
    //
    cgicc::form_iterator name2 = cgi.getElement("DMBNumber");
    //int registerValue = -1;
    if(name2 != cgi.getElements().end()) {
      dmbNumber = cgi["DMBNumber"]->getIntegerValue();
      //
    }
    //
    std::cout << "Loading DMBNumber " <<dmbNumber << std::endl ;
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    cout << "CFEBLoadFirmware" << endl;
    //
    thisCCB->hardReset();
    //
    if (thisDMB) {
      //
      char *out;
      //
      vector<CFEB> thisCFEBs = thisDMB->cfebs();
      if (dmbNumber == -1 ) {
	for (unsigned int i=0; i<thisCFEBs.size(); i++) {
	  thisCCB->hardReset();
	  ::sleep(1);
	  thisDMB->febpromuser(thisCFEBs[i]);
	  thisDMB->epromload(thisCFEBs[i].promDevice(),"../svf/cfeb_v4_r2.svf",1,out);  // load mprom
	  ::sleep(1);
	  thisCCB->hardReset();
	}
      } else {
	thisCCB->hardReset();
	::sleep(1);
	thisDMB->febpromuser(thisCFEBs[dmbNumber]);
	thisDMB->epromload(thisCFEBs[dmbNumber].promDevice(),"../svf/cfeb_v4_r2.svf",1,out);  // load mprom
	::sleep(1);
	thisCCB->hardReset();
      }
      //
    }
    //
    thisCCB->hardReset();
    //
    this->DMBUtils(in,out);
    //
  }
  //

  void EmuPeripheralCrate::DMBTurnOn(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    cout << "DMBTurnOn" << endl;
    //
    if (thisDMB) {
      thisDMB->lowv_onoff(0x3f);
    }
    //
    this->DMBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::TMBPrintCounters(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "TMBPrintCounters "<< endl;
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    //
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    }
    //
    name = cgi.getElement("AutoRefreshOn");
    //
    if(name != cgi.getElements().end()) {
      std::cout << "auto refresh on" <<std::endl ;
      AutoRefreshTMBCounters_ = true;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    //
    LOG4CPLUS_INFO(getApplicationLogger(), "Start PrintCounters");
    thisTMB->RedirectOutput(&OutputStringTMBStatus[tmb]);
    thisTMB->GetCounters();
    thisTMB->PrintCounters();
    thisTMB->RedirectOutput(&std::cout);
    LOG4CPLUS_INFO(getApplicationLogger(), "Done PrintCounters");
    //
    this->TMBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::TMBResetCounters(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    //
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    thisTMB->ResetCounters();
    //
    this->TMBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::TriggerTestInjectALCT(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    //
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    thisTMB->RedirectOutput(&OutputStringTMBStatus[tmb]);
    thisTMB->TriggerTestInjectALCT();
    thisTMB->RedirectOutput(&std::cout);
    //
    this->TMBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::TriggerTestInjectCLCT(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    //
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    thisTMB->RedirectOutput(&OutputStringTMBStatus[tmb]);
    thisTMB->TriggerTestInjectCLCT();
    thisTMB->RedirectOutput(&std::cout);
    //
    this->TMBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::ALCTStatus(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "Not tmb" << endl ;
      tmb = TMB_;
    }
    //
    ALCTController * alct = tmbVector[tmb]->alctController();
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    *out << cgicc::title("Simple Web Form") << std::endl;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << std::endl;
    //
    *out << cgicc::legend("ALCT Status").set("style","color:blue") << cgicc::p() << std::endl ;
    //
    ALCTIDRegister sc_id ;
    int err = alct->alct_read_slowcontrol_id(&sc_id);
    *out <<  " ALCT Slowcontrol ID " << sc_id << std::endl;
    //
    *out << cgicc::br();
    //
    err = alct->alct_fast_read_id(sc_id);
    *out <<  " ALCT Fastcontrol ID " << sc_id << std::endl;
    //
    *out << cgicc::fieldset();
    //
  }
  //
  void EmuPeripheralCrate::RATStatus(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "Not tmb" << endl ;
      tmb = TMB_;
    }
    //
    RAT * rat = tmbVector[tmb]->getRAT();
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    *out << cgicc::title("Simple Web Form") << std::endl;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << std::endl;
    //
    *out << cgicc::legend("RAT Status").set("style","color:blue") << cgicc::p() << std::endl ;
    //
    *out << cgicc::br();
    //
    *out << cgicc::pre();
    rat->RedirectOutput(out);
    rat->ReadRatUser1();
    rat->decodeRATUser1();
    rat->RedirectOutput(&std::cout);
    *out << cgicc::pre();
    //
    *out << cgicc::fieldset();
    //
  }
  //
  void EmuPeripheralCrate::CFEBStatus(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "Not dmb" << endl ;
      dmb = DMB_;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    *out << cgicc::title("Simple Web Form") << std::endl;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << std::endl;
    //
    *out << cgicc::legend("CFEB Status").set("style","color:blue") << cgicc::p() << std::endl ;
    //
    char buf[200];
    //
    std::vector<CFEB> cfebs = thisDMB->cfebs() ;
    //
    typedef std::vector<CFEB>::iterator CFEBItr;
    //
    for(CFEBItr cfebItr = cfebs.begin(); cfebItr != cfebs.end(); ++cfebItr) {
      sprintf(buf,"CFEB %d : ",(*cfebItr).number());
      *out << buf;
      //
      //*out << cgicc::br();
      //
      sprintf(buf,"CFEB prom user id : %08x CFEB fpga user id : %08x ",
	      (int)thisDMB->febpromuser(*cfebItr),
	      (int)thisDMB->febfpgauser(*cfebItr));
      //
      if ( thisDMB->febfpgauser(*cfebItr) == 0xcfeba042 ) {
	*out << cgicc::span().set("style","color:green");
	*out << buf;
	*out << cgicc::span();
      } else {
	*out << cgicc::span().set("style","color:red");
	*out << buf;
	*out << " (Should be 0xcfeba042) ";
	*out << cgicc::span();
      }
      //
      *out << cgicc::br();
      //
    }
    //
    *out << cgicc::fieldset();
    *out << std::endl;
    //
  }
  //
  void EmuPeripheralCrate::CCBStatus(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    char Name[50] ;
    sprintf(Name,"CCB Status slot=%d",thisCCB->slot());
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    *out << cgicc::title(Name) << std::endl;
    //
    *out << cgicc::h1(Name);
    *out << cgicc::br();
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << std::endl;
    //
    *out << cgicc::legend("CCB Info").set("style","color:blue") << cgicc::p() << std::endl ;
    //
    thisCCB->RedirectOutput(out);
    thisCCB->firmwareVersion();
    thisCCB->RedirectOutput(&std::cout);
    //
    *out << cgicc::fieldset();
    //
  }
  //
  void EmuPeripheralCrate::CCBUtils(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    char Name[50] ;
    sprintf(Name,"CCB Utils slot=%d",thisCCB->slot());
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    *out << cgicc::title(Name) << std::endl;
    //
    *out << cgicc::h1(Name);
    *out << cgicc::br();
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << std::endl;
    //
    *out << cgicc::legend("CCB Utils").set("style","color:blue") << cgicc::p() << std::endl ;
    //
    std::string ReadCCBRegister =
      toolbox::toString("/%s/ReadCCBRegister",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",ReadCCBRegister) << std::endl ;
    *out << "Read Register (int)..." << std:: endl;
    *out << cgicc::input().set("type","text").set("value","0")
      .set("name","CCBRegister") << std::endl ;
    *out << "Register value : (hex) " << std::hex << CCBRegisterValue_ << std::endl;
    *out << cgicc::form() << std::endl ;
    //
    std::string HardReset =
      toolbox::toString("/%s/HardReset",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",HardReset) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","HardReset");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::fieldset();
    //
  }
  //
  void EmuPeripheralCrate::MPCStatus(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    char Name[50] ;
    sprintf(Name,"MPC Status slot=%d",thisMPC->slot());
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    *out << cgicc::title(Name) << std::endl;
    //
    *out << cgicc::h1(Name);
    *out << cgicc::br();
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << std::endl;
    //
    *out << cgicc::legend("MPC Info").set("style","color:blue") << cgicc::p() << std::endl ;
    //
    thisMPC->RedirectOutput(out);
    thisMPC->firmwareVersion();
    thisMPC->RedirectOutput(&std::cout);
    //
    *out << cgicc::fieldset();
    //
  }
  //
  void EmuPeripheralCrate::TMBTests(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "Not tmb" << endl ;
      tmb = TMB_;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    //
    char Name[50];
    sprintf(Name,"TMB Tests slot=%d",thisTMB->slot());	  
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    //
    *out << "<a href=\"/\"><img border=\"0\" src=\"/daq/xgi/images/XDAQLogo.gif\" title=\"XDAQ\" alt=\"\" style=\"width: 145px; height: 89px;\"></a>" << std::endl;
    //
    *out << cgicc::title(Name) << std::endl;
    //
    *out << cgicc::h1(Name);
    *out << cgicc::br();
    //
    alct = thisTMB->alctController();
    //
    char buf[20];
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << endl ;
    //
    std::string RunAllTests =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",RunAllTests)
	 << std::endl ;
    *out << cgicc::input().set("type","submit")
      .set("value","Run All TMB tests") 
      .set("style","color:blue") 
	 << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden")
      .set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","0").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::table().set("border","1");
    *out << cgicc::td().set("ALIGN","center");
    //
    std::string testBootRegister =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testBootRegister)
	 << std::endl ;
    if ( tmbTestVector[tmb].GetResultTestBootRegister() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test Boot register") 
	.set("style","color:blue") 
	   << std::endl ;
    }
    else if ( tmbTestVector[tmb].GetResultTestBootRegister() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test Boot register") 
	.set("style","color:green") 
	   << std::endl ;
    }
    else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test Boot register") 
	.set("style","color:red") 
	   << std::endl ;
    }
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","1").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td().set("ALIGN","center");
    //
    //std::string testVMEfpgaDataRegister =
    //toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    //*out << cgicc::form().set("method","GET").set("action",testVMEfpgaDataRegister)
    // << std::endl ;
    //
    //if ( tmbTestVector[tmb].GetResultTestVMEfpgaDataRegister() == -1 ) {
    //*out << cgicc::input().set("type","submit")
    //.set("value","TMB test VME fpga data register") 
    //.set("style","color:blue") 
    //   << std::endl ;
    //} else if ( tmbTestVector[tmb].GetResultTestVMEfpgaDataRegister() > 0 ) {
    //*out << cgicc::input().set("type","submit")
    //.set("value","TMB test VME fpga data register") 
    //.set("style","color:green") 
    //   << std::endl ;
    //} else {
    //*out << cgicc::input().set("type","submit")
    //.set("value","TMB test VME fpga data register") 
    //.set("style","color:red") 
    //   << std::endl ;
    //}
    //
    //sprintf(buf,"%d",tmb);
    //*out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    //*out << cgicc::input().set("type","hidden")
    //.set("value","2").set("name","tmbTestid");
    //*out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td().set("ALIGN","center");
    //    
    std::string testFirmwareDate =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testFirmwareDate)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestFirmwareDate() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test firmware date") 
	.set("style","color:blue") 
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestFirmwareDate() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test firmware date") 
	.set("style","color:green") 
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test firmware date") 
	.set("style","color:red") 
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","3").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::tr().set("ALIGN","center");
    *out << cgicc::td();
    //
    std::string testFirmwareType =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testFirmwareType)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestFirmwareType() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test firmware type") 
	.set("style","color:blue") 
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestFirmwareType() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test firmware type") 
	.set("style","color:green") 
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test firmware type") 
	.set("style","color:red") 
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","4").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    std::string testFirmwareVersion =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testFirmwareVersion)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestFirmwareVersion() == -1 ) {
    *out << cgicc::input().set("type","submit")
      .set("value","TMB test firmware version") 
      .set("style","color:blue") 
	 << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestFirmwareVersion() > 0 ) {
    *out << cgicc::input().set("type","submit")
      .set("value","TMB test firmware version") 
      .set("style","color:green") 
	 << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test firmware version") 
	.set("style","color:red") 
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","5").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    std::string testFirmwareRevCode =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testFirmwareRevCode)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestFirmwareRevCode() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test firmware RevCode Id") 
	.set("style","color:blue") 
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestFirmwareRevCode() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test firmware RevCode Id") 
	.set("style","color:green") 
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test firmware RevCode Id") 
	.set("style","color:red") 
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","6").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    *out << cgicc::tr();
    //
    *out << cgicc::tr().set("ALIGN","center");
    *out << cgicc::td();
    //
    std::string testMezzId =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testMezzId)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestMezzId() == -1 ) {
    *out << cgicc::input().set("type","submit")
      .set("value","TMB test mezzanine Id") 
      .set("style","color:blue") 
	 << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestMezzId() > 0 ) {
    *out << cgicc::input().set("type","submit")
      .set("value","TMB test mezzanine Id") 
      .set("style","color:green") 
	 << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
      .set("value","TMB test mezzanine Id") 
      .set("style","color:red") 
	 << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","7").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    std::string testPromId =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testPromId)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestPromId() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test PROM Id") 
	.set("style","color:blue") 
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestPromId() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test PROM Id") 
	.set("style","color:green") 
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test PROM Id") 
	.set("style","color:red") 
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","8").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    std::string testPROMPath =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testPROMPath)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestPROMPath() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test PROM path") 
	.set("style","color:blue") 
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestPROMPath() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test PROM path") 
	.set("style","color:green") 
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test PROM path") 
	.set("style","color:red") 
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","9").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    *out << cgicc::tr();
    //
    *out << cgicc::tr().set("ALIGN","center");
    *out << cgicc::td();
    //
    std::string testDSN =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testDSN)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestDSN() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test DSN")
	.set("style","color:blue")  
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestDSN() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test DSN") 
	.set("style","color:green") 
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test DSN") 
	.set("style","color:red") 
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","10").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    std::string testADC =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testADC)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestADC() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test ADC")
	.set("style","color:blue")  
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestADC() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test ADC")
	.set("style","color:green")  
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test ADC")
	.set("style","color:red")  
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","11").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    ///////////////////////////////////////////////////////////////////////
    //
    std::string test3d3444 =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",test3d3444)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTest3d3444() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test 3d3444") 
	.set("style","color:blue")  
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTest3d3444() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test 3d3444") 
	.set("style","color:green")  
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test 3d3444") 
	.set("style","color:red")  
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","12").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    *out << cgicc::tr();
    //
    *out << cgicc::tr().set("ALIGN","center");
    *out << cgicc::td();
    //
    ///////////////////////////////////////////////////////////
    //
    std::string testRATtemper =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testRATtemper)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestRATtemper() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test RAT temperature") 
	.set("style","color:blue")  
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestRATtemper() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test RAT temperature") 
	.set("style","color:green")  
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test RAT temperature") 
	.set("style","color:red")  
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","13").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    ////////////////////////////////////////////////////////////////
    //
    std::string testRATidCodes =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testRATidCodes)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestRATidCodes() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test RAT Id Codes") 
	.set("style","color:blue")  
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestRATidCodes() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test RAT Id Codes") 
	.set("style","color:green")  
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test RAT Id Codes") 
	.set("style","color:red")  
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","14").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    ////////////////////////////////////////////////////////////////////
    //
    std::string testRATuserCodes =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testRATtemper)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestRATuserCodes() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test RAT User Codes") 
	.set("style","color:blue")  
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestRATuserCodes() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test RAT User Codes") 
	.set("style","color:green")  
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test RAT User Codes") 
	.set("style","color:red")  
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","15").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    *out << cgicc::tr();
    //
    *out << cgicc::tr().set("ALIGN","center");
    *out << cgicc::td().set("VALIGN","bottom");
    //
    /////////////////////////////////////////////////////////////////////
    //
    std::string testU760K =
      toolbox::toString("/%s/testTMB",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",testU760K)
	 << std::endl ;
    //
    if ( tmbTestVector[tmb].GetResultTestU76chip() == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test U760K") 
	.set("style","color:blue")  
	   << std::endl ;
    } else if ( tmbTestVector[tmb].GetResultTestU76chip() > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test U760K") 
	.set("style","color:green")  
	   << std::endl ;
    } else {
      *out << cgicc::input().set("type","submit")
	.set("value","TMB test U760K") 
	.set("style","color:red")  
	   << std::endl ;
    }
    //
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","hidden")
      .set("value","16").set("name","tmbTestid");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    *out << cgicc::table();
    //
    /////////////////////////////////////////////////////////////////////
    //
    *out << cgicc::textarea().set("name","TMBTestOutput")
      .set("WRAP","OFF")
      .set("rows","20").set("cols","100");
    *out << OutputTMBTests[tmb].str() << endl ;
    *out << cgicc::textarea();
    //    
    std::string LogTMBTestsOutput =
      toolbox::toString("/%s/LogTMBTestsOutput",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",LogTMBTestsOutput) << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","submit")
      .set("value","Log output").set("name","LogTMBTestsOutput") << std::endl ;
    *out << cgicc::input().set("type","submit")
      .set("value","Clear")
      .set("name","ClearTMBTestsOutput") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::fieldset();
    //
  }
  //
  void EmuPeripheralCrate::testTMB(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    //
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    }
    //
    name = cgi.getElement("tmbTestid");
    //
    int tmbTestid;
    if(name != cgi.getElements().end()) {
      tmbTestid = cgi["tmbTestid"]->getIntegerValue();
      cout << "tmbTestid " << tmbTestid << endl;
    }
    //
    tmbTestVector[tmb].RedirectOutput(&OutputTMBTests[tmb]);
    if ( tmbTestid == 1 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testBootRegister();
      ::sleep(1);
      printf("Test1");
    }
    if ( tmbTestid == 2 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testVMEfpgaDataRegister();
      printf("Test2");
    }
    if ( tmbTestid == 3 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testFirmwareDate();
      printf("Test3");
    }
    if ( tmbTestid == 4 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testFirmwareType();
      printf("Test4");
    }
    if ( tmbTestid == 5 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testFirmwareVersion();
      printf("Test5");
    }
    if ( tmbTestid == 6 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testFirmwareRevCode();
      printf("Test6");
    }
    if ( tmbTestid == 7 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testMezzId();
      printf("Test7");
    }
    if ( tmbTestid == 8 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testPROMid();
      printf("Test8");
    }
    if ( tmbTestid == 9 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testPROMpath();
      printf("Test9");
    }
    if ( tmbTestid == 10 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testDSN();
      printf("Test10");
    }
    if ( tmbTestid == 11 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testADC();
      printf("Test11");
    }
    if ( tmbTestid == 12 || tmbTestid == 0 ) {
      tmbTestVector[tmb].test3d3444();
      printf("Test12");
    }
    if ( tmbTestid == 13 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testRATtemper();
      printf("Test13");
    }
    if ( tmbTestid == 14 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testRATidCodes();
      printf("Test14");
    }
    if ( tmbTestid == 15 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testRATuserCodes();
      printf("Test15");
    }
    if ( tmbTestid == 16 || tmbTestid == 0 ) {
      tmbTestVector[tmb].testU76chip();
      printf("Test16");
    }
    //
    tmbTestVector[tmb].RedirectOutput(&std::cout);
    //
    //std::cout << "Done" << std::endl ;
    //
    this->TMBTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::TMBStatus(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "Not tmb" << endl ;
      tmb = TMB_;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    //
    char Name[50];
    sprintf(Name,"TMB Status slot=%d",thisTMB->slot());	
    //
    alct = thisTMB->alctController();
    rat  = thisTMB->getRAT();
    //
    if (alct) {
      std::string ALCTStatus =
	toolbox::toString("/%s/ALCTStatus?tmb=%d",getApplicationDescriptor()->getURN().c_str(),tmb);
      //
      *out << cgicc::a("ALCT Status").set("href",ALCTStatus) << endl;
      //
    }
    //
    if (rat) {
      std::string RATStatus =
	toolbox::toString("/%s/RATStatus?tmb=%d",getApplicationDescriptor()->getURN().c_str(),tmb);
      //
      *out << cgicc::a("RAT Status").set("href",RATStatus) << endl;
      //
    }
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    *out << cgicc::title(Name) << std::endl;
    //
    *out << cgicc::h1(Name);
    *out << cgicc::br();
    //
    char buf[200] ;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << std::endl;
    //
    *out << cgicc::legend("TMB Info").set("style","color:blue") << cgicc::p() << std::endl ;
    //
    *out << cgicc::pre();
    //
    sprintf(buf,"TMB Firmware date         : %02x/%02x/%04x ",((thisTMB->FirmwareDate()>>8)&0xff), 
	    (thisTMB->FirmwareDate()&0xff), 
	    (thisTMB->FirmwareYear()&0xffff)  );
    //
    if ( ((thisTMB->FirmwareDate()>>8)&0xff) ==  0x11 &&
	 ((thisTMB->FirmwareDate()&0xff))    ==  0x15 &&
	 ((thisTMB->FirmwareYear()&0xffff))  == 0x2004 ) {
      *out << cgicc::span().set("style","color:green");
      *out << buf;
      *out << cgicc::span();
    } else {
      *out << cgicc::span().set("style","color:red");
      *out << buf;
      *out << cgicc::span();
    }
    //
    *out << cgicc::br();
    //
    sprintf(buf,"Firmware Type             : %01x ",(thisTMB->FirmwareVersion()&0xf));       
    //
    if ( (thisTMB->FirmwareVersion()&0xf) == 0xc ) {
      *out << cgicc::span().set("style","color:green");
      *out << buf;
      *out << cgicc::span();
    } else {
      *out << cgicc::span().set("style","color:red");
      *out << buf;
      *out << cgicc::span();
    }
    //
    *out << cgicc::br();
    //
    sprintf(buf,"Firmware Version Code     : %01x ",((thisTMB->FirmwareVersion()>>4)&0xf));       
    //
    if ( ((thisTMB->FirmwareVersion()>>4)&0xf) == 0xd ){
      *out << cgicc::span().set("style","color:green");
      *out << buf ;
      *out << cgicc::span();
    } else {
      *out << cgicc::span().set("style","color:red");
      *out << buf ;
      *out << cgicc::span();
    }
    //
    *out << cgicc::br();
    //
    sprintf(buf,"Geographic Address        : %02d ",((thisTMB->FirmwareVersion()>>8)&0x1f));       
    if ( ((thisTMB->FirmwareVersion()>>8)&0x1f) == thisTMB->slot() ){
      *out << cgicc::span().set("style","color:green");
      *out << buf ;
      *out << cgicc::span();
    } else {
      *out << cgicc::span().set("style","color:red");
      *out << buf ;
      *out << cgicc::span();
      //
    }
    //
    *out << cgicc::br();
    //
    sprintf(buf,"Firmware Revision Code    : %04x ",((thisTMB->FirmwareRevCode())&0xffff));       
    *out << buf ;
    *out << cgicc::br();
    //
    sprintf(buf,"Power Comparator          : %02x ",((thisTMB->PowerComparator())&0x1f));       
    if ( (thisTMB->PowerComparator())&0x1f == 0x1f ) {
      *out << cgicc::span().set("style","color:green");
      *out << buf ;
      *out << cgicc::span();
    } else {
      *out << cgicc::span().set("style","color:red");
      *out << buf ;
      *out << cgicc::span();
    }
    //
    *out << cgicc::br();
    //
    *out << cgicc::pre();
    //
    *out << cgicc::fieldset();
    //
    *out << cgicc::fieldset();
    *out << cgicc::legend("CLCT Info").set("style","color:blue") << cgicc::p() << std::endl ;
    *out << cgicc::pre();
    thisTMB->RedirectOutput(out);
    thisTMB->DecodeCLCT();
    thisTMB->RedirectOutput(&std::cout);
    *out << cgicc::pre();
    *out << cgicc::fieldset();
    //
    if (alct) {
      *out << cgicc::fieldset();
      *out << cgicc::legend("ALCT Info").set("style","color:blue") << cgicc::p() << std::endl ;
      *out << cgicc::pre();
      thisTMB->RedirectOutput(out);
      thisTMB->DecodeALCT();
      thisTMB->RedirectOutput(&std::cout);
      *out << cgicc::pre();
      *out << cgicc::fieldset();
    }
    //
    *out << std::endl;    
    //
  }
  //
  void EmuPeripheralCrate::DMBBoardID(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception){
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "Not dmb" << endl ;
      dmb = DMB_;
    }
    //
    char buf[20];
    sprintf(buf,"DMBBoardID_%d",dmb);
    DMBBoardID_[dmb] = cgi[buf]->getValue();
    //
    this->Default(in,out);
    //
    }
  //
  void EmuPeripheralCrate::TMBBoardID(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception){
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "Not tmb" << endl ;
      tmb = TMB_;
    }
    //
    char buf[20];
    sprintf(buf,"TMBBoardID_%d",tmb);
    TMBBoardID_[tmb] = cgi[buf]->getValue();
    //
    this->Default(in,out);
    //
    }
  //
  void EmuPeripheralCrate::RATBoardID(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception){
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("rat");
    int rat;
    if(name != cgi.getElements().end()) {
      rat = cgi["rat"]->getIntegerValue();
      cout << "RAT " << rat << endl;
      RAT_ = rat;
    } else {
      cout << "Not rat" << endl ;
      rat = RAT_;
    }
    //
    char buf[20];
    sprintf(buf,"RATBoardID_%d",rat);
    RATBoardID_[rat] = cgi[buf]->getValue();
    //
    this->Default(in,out);
    //
    }
  //
  void EmuPeripheralCrate::DMBStatus(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "Not dmb" << endl ;
      dmb = DMB_;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    char Name[50];
    sprintf(Name,"DMB Status slot=%d",thisDMB->slot());	
    //
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    *out << cgicc::title(Name) << std::endl;
    //
    *out << cgicc::h1(Name);
    *out << cgicc::br();
    //
    char buf[200] ;
    //
    if( thisDMB->cfebs().size() > 0 ) {
      std::string CFEBStatus =
	toolbox::toString("/%s/CFEBStatus?dmb=%d",getApplicationDescriptor()->getURN().c_str(),dmb);
      *out << cgicc::a("CFEB Status").set("href",CFEBStatus) << endl;
    }
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << std::endl;
    //
    *out << cgicc::legend("DMB IDs").set("style","color:blue") << cgicc::p() << std::endl ;
    //
    *out << cgicc::pre();
    //
    sprintf(buf,"DMB prom VME->Motherboard          : %08x ",(int)thisDMB->mbpromuser(0));
    *out << buf ;
    *out << cgicc::br();
    //
    sprintf(buf,"DMB prom Motherboard Controller    : %08x ",(int)thisDMB->mbpromuser(1));
    *out << buf  ;
    *out << cgicc::br();
    //
    sprintf(buf,"DMB fpga id                        : %08x ",(int)thisDMB->mbfpgaid());
    *out << buf  ;
    *out << cgicc::br();
      //
    sprintf(buf,"DMB prom VME->Motherboard ID       : %08x ",(int)thisDMB->mbpromid(0));
    *out << buf  ;
    *out << cgicc::br();
    //
    sprintf(buf,"DMB prom Motherboard Controller ID : %08x ",(int) thisDMB->mbpromid(1));
    *out << buf  ;
    *out << cgicc::br();
      //
    sprintf(buf,"DMB fpga user id                   : %x ", (int) thisDMB->mbfpgauser());
    if ( thisDMB->mbfpgauser() == 0x48547182 ) {
	*out << cgicc::span().set("style","color:green");
	*out << buf;
	*out << cgicc::span();
    } else {
      *out << cgicc::span().set("style","color:red");
      *out << buf;
      *out << cgicc::span();
    }
    
    //
    *out << cgicc::pre();
    //
    *out << cgicc::br();
    //
    //thisDMB->lowv_dump();
    //thisDMB->daqmb_adc_dump();
    //thisDMB->daqmb_promfpga_dump();
    //
    *out << cgicc::fieldset();
    *out << endl ;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << endl ;
    //
    *out << cgicc::legend("Voltages & Temperatures").set("style","color:blue") 
	 << cgicc::p() << std::endl ;
    //
    *out << cgicc::table().set("border","1");
    //
    float value;
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB1 3.3V = %3.2f ",(value=thisDMB->lowv_adc(3,3))/1000.);
    if ( value/1000. < 3.3*(0.95) ||
	 value/1000. > 3.3*(1.05) ) {	 
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB1 5.0V = %3.2f ",(value=thisDMB->lowv_adc(3,4))/1000.);
    if ( value/1000. < 5.0*0.95 ||
	 value/1000. > 5.0*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB1 6.0V = %3.2f ",(value=thisDMB->lowv_adc(3,5))/1000.);
    if ( value/1000. < 6.0*0.95 ||
	 value/1000. > 6.0*1.05  ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::table();
    //
    *out << cgicc::table().set("border","1");
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB2 3.3V = %3.2f ",(value=thisDMB->lowv_adc(3,6))/1000.);
    if ( value/1000. < 3.3*0.95 ||
	 value/1000. > 3.3*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");      
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB2 5.0V = %3.2f ",(value=thisDMB->lowv_adc(3,7))/1000.);
    if ( value/1000. < 5.0*0.95 ||
	 value/1000. > 5.0*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");      
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB2 6.0V = %3.2f ",(value=thisDMB->lowv_adc(4,0))/1000.);
    if ( value/1000. < 6.0*0.95 ||
	 value/1000. > 6.0*1.95 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");      
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::table();
    //
    *out << cgicc::table().set("border","1");
    *out << cgicc::td();
    sprintf(buf,"CFEB3 3.3V = %3.2f ",(value=thisDMB->lowv_adc(4,1))/1000.);
    if ( value/1000. < 3.3*0.95 ||
	 value/1000. > 3.3*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");      
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
      //
    *out << cgicc::td();
    sprintf(buf,"CFEB3 5.0V = %3.2f ",(value=thisDMB->lowv_adc(4,2))/1000.);
    if (  value/1000. < 5.0*0.95 ||
	  value/1000. > 5.0*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");      
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB3 6.0V = %3.2f ",(value=thisDMB->lowv_adc(4,3))/1000.);
    if ( value/1000. < 6.0*0.95 ||
	 value/1000. > 6.0*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");      
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::table();
    //
    *out << cgicc::table().set("border","1");
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB4 3.3V = %3.2f ",(value=thisDMB->lowv_adc(4,4))/1000.);
    if ( value/1000. < 3.3*0.95 ||
	 value/1000. > 3.3*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");      
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB4 5.0V = %3.2f ",(value=thisDMB->lowv_adc(4,5))/1000.);
    if ( value/1000. < 5.0*0.95 ||
	 value/1000. > 5.0*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");  
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB4 6.0V = %3.2f ",(value=thisDMB->lowv_adc(4,6))/1000.);
    if ( value/1000. < 6.0*0.95 ||
	 value/1000. > 6.0*1.05 ){
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");  
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::table();
    //
    *out << cgicc::table().set("border","1");
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB5 3.3V = %3.2f ",(value=thisDMB->lowv_adc(4,7))/1000.);
    if ( value/1000. < 3.3*0.95 ||
	 value/1000. > 3.3*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");  
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB5 5.0V = %3.2f ",(value=thisDMB->lowv_adc(5,0))/1000.);
    if ( value/1000. < 5.0*0.95 ||
	 value/1000. > 5.0*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");  
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"CFEB5 6.0V = %3.2f ",(value=thisDMB->lowv_adc(5,1))/1000.);
    if ( value/1000. < 6.0*0.95 ||
	 value/1000. > 6.0*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");  
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::table();
    //
    *out << cgicc::br();
    //
    *out << cgicc::table().set("border","1");
    //
    *out << cgicc::td();
    sprintf(buf,"ALCT  3.3V = %3.2f ",(value=thisDMB->lowv_adc(5,2))/1000.);
    if ( value/1000. < 3.3*0.95 ||
	 value/1000. > 3.3*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");  
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"ALCT  1.8V = %3.2f ",(value=thisDMB->lowv_adc(5,3))/1000.);
    if ( value/1000. < 1.8*0.95 ||
	 value/1000. > 1.8*1.95 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");  
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"ALCT  5.5V B = %3.2f ",(value=thisDMB->lowv_adc(5,4))/1000.);
    if ( value/1000. < 5.5*0.95 ||
	 value/1000. > 5.5*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");  
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"ALCT  5.5V A = %3.2f ",(value=thisDMB->lowv_adc(5,5))/1000.);
    if ( value/1000. < 5.5*0.95 ||
	 value/1000. > 5.5*1.05 ) {
      *out << cgicc::span().set("style","color:red");
    } else {
      *out << cgicc::span().set("style","color:green");  
    }
    *out << buf ;
    *out << cgicc::span();
    *out << cgicc::td();
    //
    *out << cgicc::table();
    //
    *out << cgicc::br();
    //
    *out << cgicc::table().set("border","1");;
    //
    *out << cgicc::td();
    sprintf(buf,"DMB temperature = %3.1f ",(value=thisDMB->readthermx(0)));
    if ( value > 50 && value < 95 ) {
      *out << cgicc::span().set("style","color:green");
    } else {
      *out << cgicc::span().set("style","color:red");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"FEB1 temperature = %3.1f ",(value=thisDMB->readthermx(1)));
    if ( value > 50 && value < 95 ) {
      *out << cgicc::span().set("style","color:green");
    } else {
      *out << cgicc::span().set("style","color:red");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
      //
    *out << cgicc::td();
    sprintf(buf,"FEB2 temperature = %3.1f ",(value=thisDMB->readthermx(2)));
    if ( value > 50 && value < 95 ) {
      *out << cgicc::span().set("style","color:green");
    } else {
      *out << cgicc::span().set("style","color:red");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::tr();
    //
    *out << cgicc::td();
    sprintf(buf,"FEB3 temperature = %3.1f ",(value=thisDMB->readthermx(3)));
    if ( value > 50 && value < 95 ) {
      *out << cgicc::span().set("style","color:green");
    } else {
      *out << cgicc::span().set("style","color:red");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"FEB4 temperature = %3.1f ",(value=thisDMB->readthermx(4)));
    if ( value > 50 && value < 95 ) {
      *out << cgicc::span().set("style","color:green");
    } else {
      *out << cgicc::span().set("style","color:red");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::td();
    sprintf(buf,"FEB5 temperature = %3.1f ",(value=thisDMB->readthermx(5)));
    if ( value > 50 && value < 95 ) {
      *out << cgicc::span().set("style","color:green");
    } else {
      *out << cgicc::span().set("style","color:red");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::tr();
    //
    *out << cgicc::table();
    //
    *out << cgicc::br();
    //
    *out << cgicc::table().set("border","1");;
    //
    *out << cgicc::td();
    float readout = thisDMB->adcplus(2,0) ;
    sprintf(buf,"DMB DAC1 = %3.1f ",readout);
    //
    if ( readout > 3400 && readout < 3600 ) {
      *out << cgicc::span().set("style","color:black");
    } else {
      *out << cgicc::span().set("style","color:black");
    }
    //
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::td();
    readout = thisDMB->adcplus(2,1) ;
    sprintf(buf,"DMB DAC2 = %3.1f ",readout);
    if ( readout > 3400 && readout < 3600 ) {
      *out << cgicc::span().set("style","color:black");
    } else {
      *out << cgicc::span().set("style","color:black");
    }
    //
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::td();
    readout = thisDMB->adcplus(2,2) ;
    sprintf(buf,"DMB DAC3 = %3.1f ",readout);
    if ( readout > 3400 && readout < 3600 ) {
      *out << cgicc::span().set("style","color:black");
    } else {
      *out << cgicc::span().set("style","color:black");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::td();
    readout = thisDMB->adcplus(2,3) ;
    sprintf(buf,"DMB DAC4 = %3.1f ",readout);
    if ( readout > 3400 && readout < 3600 ) {
      *out << cgicc::span().set("style","color:black");
    } else {
      *out << cgicc::span().set("style","color:black");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::td();
    readout = thisDMB->adcplus(2,4) ;
    sprintf(buf,"DMB DAC5 = %3.1f ",readout);
    if ( readout > 3400 && readout < 3600 ) {
      *out << cgicc::span().set("style","color:black");
    } else {
      *out << cgicc::span().set("style","color:black");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::table();
    //
    *out << cgicc::table().set("border","1");;
    //
    *out << cgicc::td();
    readout = thisDMB->adcplus(1,6) ;
    sprintf(buf,"1.8V Chip1 = %3.1f ",readout);
    if ( readout > 1700 && readout < 1900 ) {
      *out << cgicc::span().set("style","color:green");
    } else {
      *out << cgicc::span().set("style","color:red");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::td();
    readout = thisDMB->adcplus(2,6) ;
    sprintf(buf,"1.8V Chip2 = %3.1f ",readout);
    if ( readout > 1700 && readout < 1900 ) {
      *out << cgicc::span().set("style","color:green");
    } else {
      *out << cgicc::span().set("style","color:red");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::td();
    readout = thisDMB->adcplus(3,6) ;
    sprintf(buf,"1.8V Chip3 = %3.1f ",readout);
    if ( readout > 1700 && readout < 1900 ) {
      *out << cgicc::span().set("style","color:green");
    } else {
      *out << cgicc::span().set("style","color:red");
    }
    *out << buf ;
    *out << cgicc::span() ;
    *out << cgicc::td();
    //
    *out << cgicc::table();
    //
    *out << cgicc::fieldset();
    *out << std::endl;
    //
  }
  //
  void EmuPeripheralCrate::TMBUtils(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "Not tmb" << endl ;
      tmb = TMB_;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    //
    char Name[50];
    sprintf(Name,"TMB Utils slot=%d",thisTMB->slot());
    //
    alct = thisTMB->alctController();
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    *out << cgicc::title(Name) << std::endl;
    //
    *out << "<a href=\"/\"><img border=\"0\" src=\"/daq/xgi/images/XDAQLogo.gif\" title=\"XDAQ\" alt=\"\" style=\"width: 145px; height: 89px;\"></a>" << std::endl;
    //
    *out << cgicc::h1(Name);
    *out << cgicc::br();
    //
    char buf[200] ;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << endl ;
    //
    *out << cgicc::legend("TMB Utils").set("style","color:blue") ;
    //
    std::string LoadTMBFirmware =
      toolbox::toString("/%s/LoadTMBFirmware",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",LoadTMBFirmware) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Load TMB Firmware") << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::input().set("type","text")
      .set("name","TMBFirmware")
      .set("size","80")
      .set("ENCTYPE","multipart/form-data")
      .set("value",TMBFirmware_);
    *out << cgicc::form() << std::endl ;
    //
    if (alct) {
      //
      std::string LoadALCTFirmware =
	toolbox::toString("/%s/LoadALCTFirmware",getApplicationDescriptor()->getURN().c_str());
      //
      *out << cgicc::form().set("method","GET").set("action",LoadALCTFirmware) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Load ALCT Firmware") << std::endl ;
      sprintf(buf,"%d",tmb);
      *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
      *out << cgicc::form() << std::endl ;
      //
    }
    //
    std::string ReadTMBRegister =
      toolbox::toString("/%s/ReadTMBRegister",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",ReadTMBRegister) << std::endl ;
    *out << "Read Register (int)..." << std:: endl;
    *out << cgicc::input().set("type","text").set("value","0")
      .set("name","TMBRegister") << std::endl ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << "Register value : (hex) " << std::hex << TMBRegisterValue_ << std::endl;
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::fieldset();
    //
    std::string TMBPrintCounters =
      toolbox::toString("/%s/TMBPrintCounters",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",TMBPrintCounters) ;
    *out << cgicc::input().set("type","submit").set("value","TMB Print Counters") ;
    //*out << cgicc::input().set("type","submit").set("value","AutoRefresh On").set("name","AutoRefreshOn") ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::form() ;
    //
    std::string TMBResetCounters =
      toolbox::toString("/%s/TMBResetCounters",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",TMBResetCounters) ;
    *out << cgicc::input().set("type","submit").set("value","TMB Reset Counters") ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::form() << std::endl ;
    //
    std::string TriggerTestInjectALCT =
      toolbox::toString("/%s/TriggerTestInjectALCT",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",TriggerTestInjectALCT) ;
    *out << cgicc::input().set("type","submit").set("value","TriggerTest : InjectALCT") ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::form() << std::endl ;
    //
    std::string TriggerTestInjectCLCT =
      toolbox::toString("/%s/TriggerTestInjectCLCT",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",TriggerTestInjectCLCT) ;
    *out << cgicc::input().set("type","submit").set("value","TriggerTest : InjectCLCT") ;
    sprintf(buf,"%d",tmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
    *out << cgicc::form() << std::endl ;
    //
    // Output area
    //
    *out << cgicc::form().set("method","GET") << std::endl ;
    *out << cgicc::pre();
    *out << cgicc::textarea().set("name","CrateTestTMBOutput")
      .set("rows","50")
      .set("cols","100")
      .set("WRAP","OFF");
    *out << OutputStringTMBStatus[tmb].str() << endl ;
    *out << cgicc::textarea();
    OutputStringTMBStatus[tmb].str("");
    *out << cgicc::pre();
    *out << cgicc::form() << std::endl ;
    //
  }
  //
  void EmuPeripheralCrate::LoadTMBFirmware(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    //const CgiEnvironment& env = cgi.getEnvironment();
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "Not tmb" << endl ;
      tmb = TMB_;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    //
    thisCCB->hardReset();
    //
    int debugMode(0);
    int jch(5);
    string chamberType("ME21");
    ALCTController *alct = new ALCTController(thisTMB,chamberType);
    //
    thisTMB->disableAllClocks();
    printf("Programming...");
    int status = alct->SVFLoad(&jch,TMBFirmware_.toString().c_str(),debugMode);
    thisTMB->enableAllClocks();
    //
    if (status >= 0){
      cout << "=== Programming finished"<< endl;
      cout << "=== " << status << " Verify Errors  occured" << endl;
    }
    else{
      cout << "=== Fatal Error. Exiting with " <<  status << endl;
    }
    //
    thisCCB->hardReset();
    //
    this->TMBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::LoadALCTFirmware(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    //const CgiEnvironment& env = cgi.getEnvironment();
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "Not tmb" << endl ;
      tmb = TMB_;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    //
    thisCCB->hardReset();
    //
    int debugMode(0);
    int jch(3);
    //
    ALCTIDRegister sc_id, chipID ;
    //
    printf("Reading IDs...") ;
    //
    alct->alct_read_slowcontrol_id(&sc_id) ;
    std::cout <<  " ALCT Slowcontrol ID " << sc_id << std::endl;
    alct->alct_fast_read_id(chipID);
    std::cout << " ALCT Fastcontrol ID " << chipID << std::endl;
    //
    thisTMB->disableAllClocks();
    printf("Programming...");
    int status = alct->SVFLoad(&jch,"../svf/alct384rl.svf",debugMode);
    thisTMB->enableAllClocks();
    //
    if (status >= 0){
      cout << "=== Programming finished"<< endl;
      cout << "=== " << status << " Verify Errors  occured" << endl;
    }
    else{
      cout << "=== Fatal Error. Exiting with " <<  status << endl;
    }
    //
    thisCCB->hardReset();
    //
    this->TMBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::ReadTMBRegister(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    //const CgiEnvironment& env = cgi.getEnvironment();
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "Not tmb" << endl ;
      tmb = TMB_;
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    //
    cgicc::form_iterator name2 = cgi.getElement("TMBRegister");
    int registerValue = -1;
    if(name2 != cgi.getElements().end()) {
      registerValue = cgi["TMBregister"]->getIntegerValue();
      cout << "Register " << registerValue << endl;
      //
      TMBRegisterValue_ = thisTMB->ReadRegister(registerValue);
      //
    }
    //
    this->TMBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::ReadCCBRegister(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name2 = cgi.getElement("CCBRegister");
    int registerValue = -1;
    if(name2 != cgi.getElements().end()) {
      registerValue = cgi["CCBregister"]->getIntegerValue();
      cout << "Register " << registerValue << endl;
      //
      CCBRegisterValue_ = thisCCB->ReadRegister(registerValue);
      //
    }
    //
    this->CCBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::HardReset(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    std::cout << "hardReset" << std::endl;
    //
    thisCCB->hardReset();
    //
    this->CCBUtils(in,out);
    //
  }
  //
  void EmuPeripheralCrate::DMBUtils(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "Not dmb" << endl ;
      dmb = DMB_;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    char Name[50];
    sprintf(Name,"DMB Utils slot=%d",thisDMB->slot());
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    *out << cgicc::title(Name) << std::endl;
    //
    *out << cgicc::h1(Name);
    *out << cgicc::br();
    //
    char buf[200] ;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << endl ;
    //
    *out << cgicc::legend("DMB Utils").set("style","color:blue") ;
    //
    *out << cgicc::table().set("border","1");
    //
    *out << cgicc::td();
    //
    std::string DMBTurnOff =
      toolbox::toString("/%s/DMBTurnOff",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBTurnOff) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","DMB Turn Off LV") << std::endl ;
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    std::string DMBTurnOn =
      toolbox::toString("/%s/DMBTurnOn",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBTurnOn) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","DMB Turn On LV") << std::endl ;
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::table();
    //
    std::string DMBPrintCounters =
      toolbox::toString("/%s/DMBPrintCounters",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBPrintCounters)
	 << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","DMB Print Counters") << std::endl ;
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    std::string DMBLoadFirmware =
      toolbox::toString("/%s/DMBLoadFirmware",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBLoadFirmware)
	 << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","DMB Load Firmware") << std::endl ;
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    std::string CFEBLoadFirmware =
      toolbox::toString("/%s/CFEBLoadFirmware",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",CFEBLoadFirmware)
	 << std::endl ;
    *out << "CFEB to download (0-4), (-1 == all) : ";
    *out << cgicc::input().set("type","text").set("value","-1")
      .set("name","DMBNumber") << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","CFEB Load Firmware") << std::endl ;
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::fieldset();
    //
  }
  //
  void EmuPeripheralCrate::DMBTests(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "Not dmb" << endl ;
      dmb = DMB_;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    char Name[50];
    sprintf(Name,"DMB Tests slot=%d",thisDMB->slot());
    //
    *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
    //
    *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
    //
    *out << "<a href=\"/\"><img border=\"0\" src=\"/daq/xgi/images/XDAQLogo.gif\" title=\"XDAQ\" alt=\"\" style=\"width: 145px; height: 89px;\"></a>" << std::endl;
    //
    *out << cgicc::title(Name) << std::endl;
    //
    *out << cgicc::h1(Name);
    *out << cgicc::br();
    //
    char buf[200] ;
    //
    *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;");
    *out << endl ;
    //
    *out << cgicc::legend("DMB Tests").set("style","color:blue") ;
    //
    std::string DMBTestAll =
      toolbox::toString("/%s/DMBTestAll",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBTestAll)
	 << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","DMB Test All ") << std::endl ;
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    /*
    std::string DMBTest3 =
      toolbox::toString("/%s/DMBTest3",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBTest3)
	 << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","DMB Test3 (Check DMB Fifos)") << std::endl ;
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    */
    //
    *out << cgicc::table().set("border","0");
    *out << cgicc::td();
    //
    std::string DMBTest4 =
      toolbox::toString("/%s/DMBTest4",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBTest4)
	 << std::endl ;
    if( thisDMB->GetTestStatus(4) == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test4 (Check voltages)")
	.set("style","color:blue") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(4) > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test4 (Check voltages)")
	.set("style","color:red") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(4) ==0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test4 (Check voltages)")
	.set("style","color:green") 
	   << std::endl ;
    }
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    std::string DMBTest5 =
      toolbox::toString("/%s/DMBTest5",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBTest5)
	 << std::endl ;
    if( thisDMB->GetTestStatus(5) == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test5 (Check Power Register)")
	.set("style","color:blue") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(5) > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test5 (Check Power Register)")
	.set("style","color:red") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(5) ==0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test5 (Check PowerRegister)")
	.set("style","color:green") 
	   << std::endl ;
    }
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    std::string DMBTest6 =
      toolbox::toString("/%s/DMBTest6",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBTest6)
	 << std::endl ;
    if( thisDMB->GetTestStatus(6) == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test6 (Check FPGA IDs)")
	.set("style","color:blue") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(6) > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test6 (Check FPGA IDs)")
	.set("style","color:red") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(6) ==0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test6 (Check FPGA IDs)")
	.set("style","color:green") 
	   << std::endl ;
    }
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::tr();
    *out << cgicc::td();
    //
    std::string DMBTest8 =
      toolbox::toString("/%s/DMBTest8",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBTest8)
	 << std::endl ;
    if( thisDMB->GetTestStatus(8) == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test8 (Check Comparator DAC/ADC)")
	.set("style","color:blue") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(8) > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test8 (Check Comparator DAC/ADC)")
	.set("style","color:red") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(8) ==0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test8 (Check Comparator DAC/ADC)")
	.set("style","color:green") 
	   << std::endl ;
    }
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    std::string DMBTest9 =
      toolbox::toString("/%s/DMBTest9",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBTest9)
	 << std::endl ;
    if( thisDMB->GetTestStatus(9) == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test9 (Check CDAC)")
	.set("style","color:blue") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(9) > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test9 (Check CDAC)")
	.set("style","color:red") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(9) ==0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test9 (Check CDAC)")
	.set("style","color:green") 
	   << std::endl ;
    }
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::td();
    //
    std::string DMBTest10 =
      toolbox::toString("/%s/DMBTest10",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBTest10)
	 << std::endl ;
    if( thisDMB->GetTestStatus(10) == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test10 (Check SFM)")
	.set("style","color:blue") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(10) > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test10 (Check SFM)")
	.set("style","color:red") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(10) ==0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test10 (Check SFM)")
	.set("style","color:green") 
	   << std::endl ;
    }
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::tr();
    //
    *out << cgicc::tr();
    *out << cgicc::td();
    //
    std::string DMBTest11 =
      toolbox::toString("/%s/DMBTest11",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",DMBTest11)
	 << std::endl ;
    if( thisDMB->GetTestStatus(11) == -1 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test11 (Check Comparator DAC/ADC)")
	.set("style","color:blue") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(11) > 0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test11 (Check Comparator DAC/ADC)")
	.set("style","color:red") 
	   << std::endl ;
    }
    if( thisDMB->GetTestStatus(11) ==0 ) {
      *out << cgicc::input().set("type","submit")
	.set("value","DMB Test11 (Check Comparator DAC/ADC)")
	.set("style","color:green") 
	   << std::endl ;
    }
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::form() << std::endl ;
    //
    *out << cgicc::td();
    //
    *out << cgicc::table();
    //
    *out << cgicc::fieldset() << std::endl;
    //
    *out << cgicc::form().set("method","GET") << std::endl ;
    *out << cgicc::textarea().set("name","CrateTestDMBOutput")
      .set("WRAP","OFF")
      .set("rows","20").set("cols","60");
    *out << OutputDMBTests[dmb].str() << endl ;
    *out << cgicc::textarea();
    *out << cgicc::form();
    //
    std::string method =
      toolbox::toString("/%s/LogDMBTestsOutput",getApplicationDescriptor()->getURN().c_str());
    //
    *out << cgicc::form().set("method","GET").set("action",method) << std::endl ;
    sprintf(buf,"%d",dmb);
    *out << cgicc::input().set("type","hidden").set("value",buf).set("name","dmb");
    *out << cgicc::input().set("type","submit")
      .set("value","Log output").set("name","LogDMBTestsOutput") << std::endl ;
    *out << cgicc::input().set("type","submit")
      .set("value","Clear")
      .set("name","ClearDMBTestsOutput") << std::endl ;
    *out << cgicc::form() << std::endl ;
    //
    std::cout << "Done" << std::endl;
    //
  }
  //
  void EmuPeripheralCrate::LogDMBTestsOutput(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "LogDMBTestsOutput" << std::endl;
    //
    cgicc::Cgicc cgi(in);
    //
    //
    cgicc::form_iterator name = cgi.getElement("dmb");
    //
    //
    int dmb;
    if(name != cgi.getElements().end()) {
      dmb = cgi["dmb"]->getIntegerValue();
      cout << "DMB " << dmb << endl;
      DMB_ = dmb;
    } else {
      cout << "Not dmb" << endl ;
      dmb = DMB_;
    }
    //
    cgicc::form_iterator name2 = cgi.getElement("ClearDMBTestsOutput");
    //
    if(name2 != cgi.getElements().end()) {
      cout << "Clear..." << endl;
      cout << cgi["ClearDMBTestsOutput"]->getValue() << std::endl ;
      OutputDMBTests[dmb].str("");
      //
    this->DMBTests(in,out);
    return;
    }
    //
    DAQMB * thisDMB = dmbVector[dmb];
    //
    cout << DMBBoardID_[dmb] << endl ;
    //
    char buf[20];
    sprintf(buf,"DMBTestsLogFile_%d_%s.log",thisDMB->slot(),DMBBoardID_[dmb].c_str());
    //
    ofstream DMBTestsLogFile;
    DMBTestsLogFile.open(buf);
    DMBTestsLogFile << OutputDMBTests[dmb].str() ;
    DMBTestsLogFile.close();
    //
    OutputDMBTests[dmb].str("");
    //
    this->DMBTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::LogTestSummary(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception){
    //
    time_t rawtime;
    time(&rawtime);
    //
    std::string buf;
    std::string time_dump = ctime(&rawtime);
    std::string time = time_dump.substr(0,time_dump.length()-1);
    //
    while( time.find(" ",0) != string::npos ) {
      //
      int thispos = time.find(" ",0); 
      time.replace(thispos,1,"_");
      //
    }
    //
    buf = "EmuPeripheralCrateTestSummary_"+RunNumber_+"_"+Operator_+time+".log";
    //
    ofstream LogFile;
    LogFile.open(buf.c_str());
    //
    LogFile << " *** Output : Test Summary *** " << std::endl ;
    //
    LogFile << std::endl;
    LogFile << "Operator : " << Operator_ << std::endl ;
    LogFile << "Time     : " << ctime(&rawtime) << std::endl ;
    LogFile << "XML File : " << xmlFile_.toString() << std::endl ;
    //
    for (unsigned int i=0; i<tmbVector.size(); i++) {
      //
      LogFile << "TMB " << std::setw(5) << tmbVector[i]->slot() << std::setw(5) <<
	TMBBoardID_[i] << std::setw(5) <<
	tmbTestVector[i].GetResultTestBootRegister() << std::setw(5) <<
	tmbTestVector[i].GetResultTestVMEfpgaDataRegister() << std::setw(5) <<
	tmbTestVector[i].GetResultTestFirmwareDate() << std::setw(5) <<
	tmbTestVector[i].GetResultTestFirmwareType() << std::setw(5) <<
	tmbTestVector[i].GetResultTestFirmwareVersion() << std::setw(5) <<
	tmbTestVector[i].GetResultTestFirmwareRevCode() << std::setw(5) <<
	tmbTestVector[i].GetResultTestMezzId() << std::setw(5) <<
	tmbTestVector[i].GetResultTestPromId() << std::setw(5) <<
	tmbTestVector[i].GetResultTestPROMPath() << std::setw(5) <<
	tmbTestVector[i].GetResultTestDSN() << std::setw(5) <<
	tmbTestVector[i].GetResultTestADC() << std::setw(5) <<
	tmbTestVector[i].GetResultTest3d3444() << std::setw(5)
	      << std::endl ;
      //
    }
    //
    for(int i=0; i<20; i++) LogFile << "+";
    LogFile << std::endl ;
    //
    for (unsigned int i=0; i<dmbVector.size(); i++) {
      //
      LogFile << "DMB " << std::setw(5) << dmbVector[i]->slot() << std::setw(5) <<
	DMBBoardID_[i] ;
	for (int j=0; j<20; j++) LogFile << std::setw(5) << dmbVector[i]->GetTestStatus(j) ;
      LogFile << std::endl ;
      //
    }
    //
    LogFile << std::endl;
    //
    LogFile.close();
    //
    this->Default(in,out);
    //
  }
  //
  void EmuPeripheralCrate::LogOutput(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception){
    //
    std::cout << "LogOuput" << std::endl;
    //
    //
    std::string buf;
    //int test = 1;
    buf = "EmuPeripheralCrateLogFile_"+RunNumber_+"_"+Operator_+".log";
    //
    std::cout << "File to write to" << buf << std::endl ;
    //
    ifstream TextFile ;
    TextFile.open(xmlFile_.toString().c_str());
    //
    ofstream LogFile;
    LogFile.open(buf.c_str());
    while(TextFile.good()) LogFile << (char) TextFile.get() ;
    TextFile.close();
    for (unsigned int i=0; i<tmbVector.size(); i++) {
      LogFile << OutputTMBTests[i].str() ;
    }
    for (unsigned int i=0; i<tmbVector.size(); i++) {
      LogFile << OutputDMBTests[i].str() ;
    }
    LogFile.close();    
    //
    this->Default(in,out);
    //
  }
  //
  void EmuPeripheralCrate::LogTMBTestsOutput(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    //
    cout << "LogTMBTestsOutput" << std::endl;
    //
    cgicc::Cgicc cgi(in);
    //
    cgicc::form_iterator name = cgi.getElement("tmb");
    //
    int tmb;
    if(name != cgi.getElements().end()) {
      tmb = cgi["tmb"]->getIntegerValue();
      cout << "TMB " << tmb << endl;
      TMB_ = tmb;
    } else {
      cout << "Not tmb" << endl ;
      tmb = TMB_;
    }
    //
    cgicc::form_iterator name2 = cgi.getElement("ClearTMBTestsOutput");
    //
    if(name2 != cgi.getElements().end()) {
      cout << "Clear..." << endl;
      cout << cgi["ClearTMBTestsOutput"]->getValue() << std::endl ;
      OutputTMBTests[tmb].str("");
      //
      this->TMBTests(in,out);
      return ;
      //
    }
    //
    TMB * thisTMB = tmbVector[tmb];
    //
    cout << TMBBoardID_[tmb] << endl ;
    //
    char buf[20];
    sprintf(buf,"TMBTestsLogFile_%d_%s.log",thisTMB->slot(),TMBBoardID_[tmb].c_str());
    //
    ofstream TMBTestsLogFile;
    TMBTestsLogFile.open(buf);
    TMBTestsLogFile << OutputTMBTests[tmb].str() ;
    TMBTestsLogFile.close();
    //
    OutputTMBTests[tmb].str("");
    //
    this->TMBTests(in,out);
    //
  }
  //
  void EmuPeripheralCrate::setRawConfFile(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    try
      {
	cout << "setRawConfFile" << endl ;
	//
	cgicc::Cgicc cgi(in);
	//
	ofstream TextFile ;
	TextFile.open("MyTextConfigurationFile.xml");
	TextFile << (cgi["Text"]->getValue()) ; 
	TextFile.close();
	//
	xmlFile_ = "MyTextConfigurationFile.xml" ;
	//
	//Configuring();
	//
	cout << "Out setRawConfFile" << endl ;
	//
	this->Default(in,out);
      }
    catch (const std::exception & e )
      {
	//XECPT_RAISE(xgi::exception::Exception, e.what());
      }
  }
  //
  void EmuPeripheralCrate::Configuring(){
    //
    std::cout << "Configuring " << std::endl ;
    //
    //-- parse XML file
    //
    //cout << "---- XML parser ----" << endl;
    //cout << " Here parser " << endl;
    //PeripheralCrateParser parser;
    //cout << " Using file " << xmlFile_.toString() << endl ;
    //
    //parser.parseFile(xmlFile_.toString().c_str());
    //
    if ( MyController != 0 ) {
      delete MyController ;
    }
    //
    MyController = new EmuController();
    //
    MyController->SetConfFile(xmlFile_.toString().c_str());
    MyController->init();
    //
    //-- Make sure that only one TMB in one crate is configured
    //
    CrateSelector selector = MyController->selector();
    vector<Crate*> crateVector = selector.crates();
    //
    if (crateVector.size() > 1 ) std::cout << "Warning...this configuration file has more than one crate" 
					   << std::endl;
    //
    //if (crateVector.size() > 1){
    //cerr << "Error: only one PeripheralCrate allowed" << endl;
    //exit(1);
    //}
    //
    tmbVector = selector.tmbs(crateVector[0]);
    //
    CrateUtils MyCrateUtils;
    MyCrateUtils.SetCrate(crateVector[0]);
    //
    tmbTestVector = MyCrateUtils.TMBTests();
    //
    //if (tmbVector.size() > 1){
    //cerr << "Error: only one TMB in xml file allowed" << endl ;
    //exit(1);
    //}
    //
    dmbVector = selector.daqmbs(crateVector[0]);
    //
    //chamberVector = selector.chambers(crateVector[0]);
    //
    //if (dmbVector.size() > 1){
    //cerr << "Error: only one DMB in xml file allowed" << endl;
    //exit(1);
    //} 
    //
    //-- get pointers to CCB, TMB and DMB
    //
    thisCrate = crateVector[0];
    thisCCB = thisCrate->ccb();
    thisMPC = thisCrate->mpc();
    //
    //TMB * thisTMB = tmbVector[0];
    //DAQMB * thisDMB = dmbVector[0];
    //
    //DDU * thisDDU = thisCrate->ddu();
    //if(thisTMB) alct = thisTMB->alctController();
    //
    std::cout << "Done" << std::endl ;
  }
  //

  void EmuPeripheralCrate::setConfFile(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    try
      {
	//
	cgicc::Cgicc cgi(in);
	//
	const_file_iterator file;
	file = cgi.getFile("xmlFileName");
	//
	cout << "GetFiles string" << endl ;
	//
	if(file != cgi.getFiles().end()) (*file).writeToStream(cout);
	//
	string XMLname = cgi["xmlFileName"]->getValue() ; 
	//
	cout << XMLname  << endl ;
	//
	xmlFile_ = XMLname ;
	//
	Configuring();
	//
	this->Default(in,out);
	//
      }
    catch (const std::exception & e )
      {
	//XECPT_RAISE(xgi::exception::Exception, e.what());
      }
  }
  //
  //
  void EmuPeripheralCrate::getTestLogFile(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    try
      {
	//
	cgicc::Cgicc cgi(in);
	//
	const_file_iterator file;
	file = cgi.getFile("TestLogFile");
	//
	cout << "GetFiles string" << endl ;
	//
	if(file != cgi.getFiles().end()) (*file).writeToStream(cout);
	//
	string TestLogFile = cgi["TestLogFile"]->getValue() ; 
	//
	cout << TestLogFile  << endl ;
	//
	TestLogFile_ = TestLogFile;
	//
	ParseTestLogFile(TestLogFile);
	//
	this->Default(in,out);
	//
      }
    catch (const std::exception & e )
      {
	//XECPT_RAISE(xgi::exception::Exception, e.what());
      }
  }

  void EmuPeripheralCrate::ParseTestLogFile(xdata::String logFile) 
    throw (xgi::exception::Exception){
    //
    ifstream TextFile ;
    std:: cout << "Opening file " << logFile.toString().c_str() << std::endl ;
    TextFile.open(logFile.toString().c_str());    
    //
    //while(TextFile.good()) std::cout << (char) TextFile.get() ;
    //
    bool FoundXML = false ;
    int nTMB = 0;
    int nDMB = 0;
    //
    std::string line, line0, line1, line2;
    while(TextFile.good()) {
      //std::cout << "Line" <<std::endl ;
      getline(TextFile,line);
      //std::cout << line << std::endl ;
      if ( line.find("XML File") != string::npos ) {
	std::cout << "Found" << std::endl ;
	int start = line.find("/");
	int end   = line.length();
	xmlFile_ = line.substr(start,end-start) ;
	//
	Configuring();
	FoundXML = true ;
	//
      }
      //
      //
      if ( FoundXML ) { // Processed XML File
	if ( line.find("TMB") != string::npos ) {	  
	  //
	  int slot, boardid, testResult[20];
	  istringstream instring(line);
	  //
	  instring >> line0 >> slot >> boardid 
		   >> testResult[0] 
		   >> testResult[1] 
		   >> testResult[2] 
		   >> testResult[3] 
		   >> testResult[4] 
		   >> testResult[5] 
		   >> testResult[6] 
		   >> testResult[7] 
		   >> testResult[8] 
		   >> testResult[9] 
		   >> testResult[10] 
		   >> testResult[11] ;
	  //
	  std::cout << "TMB.Setting " << nTMB << " " << boardid << " " << testResult[0] <<  " " << testResult[1] << std::endl ;
	  //
	  char buf[20];
	  sprintf(buf,"%d",boardid);
	  TMBBoardID_[nTMB] = buf ;
	  //
	  tmbTestVector[nTMB].SetResultTestBootRegister(testResult[0]);
	  tmbTestVector[nTMB].SetResultTestVMEfpgaDataRegister(testResult[1]);
	  tmbTestVector[nTMB].SetResultTestFirmwareDate(testResult[2]); 
	  tmbTestVector[nTMB].SetResultTestFirmwareType(testResult[3]); 
	  tmbTestVector[nTMB].SetResultTestFirmwareVersion(testResult[4]); 
	  tmbTestVector[nTMB].SetResultTestFirmwareRevCode(testResult[5]);
	  tmbTestVector[nTMB].SetResultTestMezzId(testResult[6]);
	  tmbTestVector[nTMB].SetResultTestPromId(testResult[7]);
	  tmbTestVector[nTMB].SetResultTestPROMPath(testResult[8]);
	  tmbTestVector[nTMB].SetResultTestDSN(testResult[9]);
	  tmbTestVector[nTMB].SetResultTestADC(testResult[10]);
	  tmbTestVector[nTMB].SetResultTest3d3444(testResult[11]);
	  //
	  nTMB++ ;
	  //	  
	}
	//
	if ( line.find("DMB") != string::npos ) {	  
	  //
	  int slot, boardid, testResult[20];
	  istringstream instring(line);
	  //
	  instring >> line0 >> slot >> boardid 
		   >> testResult[0] 
		   >> testResult[1] 
		   >> testResult[2] 
		   >> testResult[3] 
		   >> testResult[4] 
		   >> testResult[5] 
		   >> testResult[6] 
		   >> testResult[7] 
		   >> testResult[8] 
		   >> testResult[9] 
		   >> testResult[10] 
		   >> testResult[11] 
		   >> testResult[12] 
		   >> testResult[13] 
		   >> testResult[14] 
		   >> testResult[15] 
		   >> testResult[16] 
		   >> testResult[17] 
		   >> testResult[18] 
		   >> testResult[19] ;
	  //
	  std::cout << "DMB.Setting " << nDMB << " " << boardid << " " << testResult[0] <<  " " << testResult[1] << std::endl ;
	  //
	  char buf[20];
	  sprintf(buf,"%d",boardid);
	  DMBBoardID_[nDMB] = buf;
	  //
	  if ( dmbVector[nDMB]->slot() == slot ) {
	    for(int i=0; i<20; i++) dmbVector[nDMB]->SetTestStatus(i,testResult[i]);
	  }
	  //
	  nDMB++;
	  //
	}
	//
      }
    }
    //
    std::cout << std::endl ;
    //
    TextFile.close();
    //
    
  }
  //
  void EmuPeripheralCrate::getTestLogFileUpload(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    try
      {
	//
	cgicc::Cgicc cgi(in);
	//
	const_file_iterator file;
	file = cgi.getFile("TestFileUpload");
	//
	cout << "GetFiles" << endl ;
	//
	if(file != cgi.getFiles().end()) {
	  ofstream TextFile ;
	  TextFile.open("MyTestLogFile.xml");
	  (*file).writeToStream(TextFile);
	  TextFile.close();
	}
	//
	TestLogFile_ = "MyTestLogFile.xml" ;
	//
	ParseTestLogFile(TestLogFile_);
	//
	//cout << "UploadConfFile done" << endl ;
	//
	this->Default(in,out);
	//
      }
    catch (const std::exception & e )
      {
	//XECPT_RAISE(xgi::exception::Exception, e.what());
      }
  }
  //
  void EmuPeripheralCrate::UploadConfFile(xgi::Input * in, xgi::Output * out ) 
    throw (xgi::exception::Exception)
  {
    try
      {
	//
	cout << "UploadConfFileUpload" << endl ;
	//
	cgicc::Cgicc cgi(in);
	//
	const_file_iterator file;
	file = cgi.getFile("xmlFileNameUpload");
	//
	cout << "GetFiles" << endl ;
	//
	if(file != cgi.getFiles().end()) {
	  ofstream TextFile ;
	  TextFile.open("MyTextConfigurationFile.xml");
	  (*file).writeToStream(TextFile);
	  TextFile.close();
	}
	//
	xmlFile_ = "MyTextConfigurationFile.xml" ;
	//
	Configuring();
	//
	cout << "UploadConfFile done" << endl ;
	//
	this->Default(in,out);
	//
      }
    catch (const std::exception & e )
      {
	//XECPT_RAISE(xgi::exception::Exception, e.what());
      }
  }

  
};

#endif
