// Application.cc

#include "emu/supervisor/Application.h"
#include "emu/supervisor/RegDumpPreprocessor.h"
#include "emu/base/Stopwatch.h"

#include <exception>
#include <sstream>
#include <set>
#include <map>
#include <cstdlib>  // strtol()
#include <iomanip>
#include <sys/time.h>  // gettimeofday()

#include "xdaq/NamespaceURI.h"
#include "xoap/Method.h"
#include "xoap/MessageFactory.h"  // createMessage()
#include "xoap/SOAPPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPSerializer.h"
#include "xoap/domutils.h"
#include "toolbox/task/WorkLoopFactory.h" // getWorkLoopFactory()

#include "xoap/DOMParser.h"
#include "xoap/DOMParserFactory.h"
#include "xoap/domutils.h"
#include "xdata/soap/Serializer.h"

#include "cgicc/HTMLClasses.h"
#include "xgi/Utils.h"

#include "xcept/tools.h"
#include "xdaq2rc/RcmsStateNotifier.h"
#include "toolbox/fsm/FailedEvent.h"

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationDescriptorImpl.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/exception/Exception.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/XceptSerializer.h"
#include "xdata/Float.h" 
#include "xdata/Double.h" 
#include "xdata/Boolean.h"

#include "emu/supervisor/exception/Exception.h"
#include "emu/supervisor/alarm/Alarm.h"
#include "emu/base/Alarm.h"
#include "emu/soap/ToolBox.h"
#include "emu/soap/Messenger.h"

using namespace std;
using namespace cgicc;

XDAQ_INSTANTIATOR_IMPL(emu::supervisor::Application)

static const string NS_XSI = "http://www.w3.org/2001/XMLSchema-instance";
static const unsigned int N_LOG_MESSAGES = 50;
static const string STATE_UNKNOWN = "unknown";

void emu::supervisor::Application::CalibParam::registerFields(xdata::Bag<CalibParam> *bag)
{
  key_ = "";
  command_ = "";
  loop_ = 1U;
  delay_ = 1U;
  ltc_ = "LTCConfiguration.txt";
  ttcci_ = "TTCciConfiguration_ListenCSCLTC904.txt";
  
  bag->addField("key",     &key_);
  bag->addField("command", &command_);
  bag->addField("loop",    &loop_);
  bag->addField("delay",   &delay_);
  bag->addField("ltc",     &ltc_);
  bag->addField("ttcci",   &ttcci_);
}

void emu::supervisor::Application::RunParameters::registerFields(xdata::Bag<RunParameters> *bag)
{
  key_ = "";
  command_ = "";
  loop_ = 1U;
  delay_ = 1U;
  ci_    = "";
  ci_p_  = "";
  ci_m_  = "";
  ci_tf_ = "";
  pm_ = "";
  pi_ = "";
  
  bag->addField("key",     &key_);
  bag->addField("command", &command_);
  bag->addField("loop",    &loop_);
  bag->addField("delay",   &delay_);
  bag->addField("ci",      &ci_);
  bag->addField("ci_p",    &ci_p_);
  bag->addField("ci_m",    &ci_m_);
  bag->addField("ci_tf",   &ci_tf_);
  bag->addField("pm",      &pm_);
  bag->addField("pi",      &pi_);
}

emu::supervisor::Application::Application(xdaq::ApplicationStub *stub)
  //throw (xcept::Exception) 
  :
  xdaq::WebApplication(stub),
  emu::base::Supervised(stub),
  logger_(Logger::getInstance("emu::supervisor::Application")),
  isInCalibrationSequence_(false),
  run_type_("UNKNOWN"), run_number_(1), runSequenceNumber_(0),
  daq_mode_("UNKNOWN"), ttc_source_(""),
  rcmsStateNotifier_(getApplicationLogger(), getApplicationDescriptor(), getApplicationContext()),
  TFCellOpState_(""), TFCellOpName_("Configuration"), TFCellClass_("Cell"), TFCellInstance_(8), 
  wl_semaphore_(toolbox::BSem::EMPTY), quit_calibration_(false),
  daq_descr_(NULL), tf_descr_(NULL), ttc_descr_(NULL), 
  ci_plus_descr_(NULL), ci_minus_descr_(NULL), ci_tf_descr_(NULL), pm_descr_(NULL),
  pi_plus_descr_(NULL), pi_minus_descr_(NULL), pi_tf_descr_(NULL),
  ci_plus_(NULL), ci_minus_(NULL), ci_tf_(NULL), pm_(NULL),
  pi_plus_(NULL), pi_minus_(NULL), pi_tf_(NULL),
  usePrimaryTCDS_( true ),
  isUsingTCDS_( true ),
  nevents_(-1),
  step_counter_(0),
  error_message_(""), keep_refresh_(false), hide_tts_control_(true),
  controlTFCellOp_(false), // Default MUST be false, i.e., hands off the TF Cell.
  localDAQWriteBadEventsOnly_(false),
  runInfo_(NULL),
  runDbBookingCommand_( "java -jar runnumberbooker.jar" ),
  runDbWritingCommand_( "java -jar runinfowriter.jar" ),
  runDbAddress_       ( "" ),
  runDbUserFile_      ( "" ),
  isBookedRunNumber_  ( false ),
  state_table_(this)
{  
  appDescriptor_ = getApplicationDescriptor();
  
  xdata::InfoSpace *i = getApplicationInfoSpace();
  i->fireItemAvailable("isInCalibrationSequence", &isInCalibrationSequence_);
  i->fireItemAvailable("RunType", &run_type_);
  i->fireItemAvailable("RunNumber", &run_number_);
  
  i->fireItemAvailable("configKeys", &config_keys_);
  i->fireItemAvailable("calibParams", &calib_params_);

  i->fireItemAvailable("runParameters", &runParameters_);
  
  i->fireItemAvailable("DAQMode", &daq_mode_);
  i->fireItemAvailable("TTCSource", &ttc_source_);

  i->fireItemAvailable("usePrimaryTCDS", &usePrimaryTCDS_);
  
  i->fireItemAvailable("TFCellOpState",  &TFCellOpState_);
  i->fireItemAvailable("TFCellOpName",   &TFCellOpName_);
  i->fireItemAvailable("TFCellClass",    &TFCellClass_);
  i->fireItemAvailable("TFCellInstance", &TFCellInstance_);

  i->fireItemAvailable("ttsID", &tts_id_);
  i->fireItemAvailable("ttsBits", &tts_bits_);

  i->fireItemAvailable("controlTFCellOp", &controlTFCellOp_);

  i->fireItemAvailable("localDAQWriteBadEventsOnly", &localDAQWriteBadEventsOnly_);

  // Track Finder Key
  tf_key_ = "310309";   // default key as of 31/03/2009
  i->fireItemAvailable("TrackFinderKey", &tf_key_);  

  i->fireItemAvailable( "runDbBookingCommand", &runDbBookingCommand_ );
  i->fireItemAvailable( "runDbWritingCommand", &runDbWritingCommand_ );
  i->fireItemAvailable( "runDbAddress",        &runDbAddress_        );
  i->fireItemAvailable( "runDbUserFile",       &runDbUserFile_       );
  
  //Interface to Run Control
  i->fireItemAvailable("rcmsStateListener", rcmsStateNotifier_.getRcmsStateListenerParameter());
  i->fireItemAvailable("foundRcmsStateListener", rcmsStateNotifier_.getFoundRcmsStateListenerParameter());
  rcmsStateNotifier_.subscribeToChangesInRcmsStateListener(getApplicationInfoSpace());
	
  xgi::bind(this, &emu::supervisor::Application::webDefault,     "Default");
  xgi::bind(this, &emu::supervisor::Application::webConfigure,   "Configure");
  xgi::bind(this, &emu::supervisor::Application::webStart,       "Start");
  xgi::bind(this, &emu::supervisor::Application::webStop,        "Stop");
  xgi::bind(this, &emu::supervisor::Application::webHalt,        "Halt");
  xgi::bind(this, &emu::supervisor::Application::webReset,       "Reset");
  xgi::bind(this, &emu::supervisor::Application::webSetTTS,      "SetTTS");
  xgi::bind(this, &emu::supervisor::Application::webSwitchTTS,   "SwitchTTS");
  xgi::bind(this, &emu::supervisor::Application::webRunSequence, "RunSequence");
  xgi::bind(this, &emu::supervisor::Application::webResyncViaTCDS,   "ResyncViaTCDS");
  xgi::bind(this, &emu::supervisor::Application::webHardResetViaTCDS,"HardResetViaTCDS");
  xgi::bind(this, &emu::supervisor::Application::webBgoTrainViaTCDS, "BgoTrainViaTCDS");
  
  xoap::bind(this, &emu::supervisor::Application::onConfigure,   "Configure",   XDAQ_NS_URI);
  xoap::bind(this, &emu::supervisor::Application::onStart,       "Start",       XDAQ_NS_URI);
  xoap::bind(this, &emu::supervisor::Application::onStop,        "Stop",        XDAQ_NS_URI);
  xoap::bind(this, &emu::supervisor::Application::onHalt,        "Halt",        XDAQ_NS_URI);
  xoap::bind(this, &emu::supervisor::Application::onReset,       "Reset",       XDAQ_NS_URI);
  xoap::bind(this, &emu::supervisor::Application::onSetTTS,      "SetTTS",      XDAQ_NS_URI);
  xoap::bind(this, &emu::supervisor::Application::onRunSequence, "RunSequence", XDAQ_NS_URI);
  
  calib_wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("CSC SV Calib", "waiting");
  calib_wl_->activate();
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("CSC SV", "waiting");
  wl_->activate();
  configure_signature_   = toolbox::task::bind(this, &emu::supervisor::Application::configureAction,      "configureAction");
  start_signature_       = toolbox::task::bind(this, &emu::supervisor::Application::startAction,          "startAction");
  stop_signature_        = toolbox::task::bind(this, &emu::supervisor::Application::stopAction,           "stopAction");
  halt_signature_        = toolbox::task::bind(this, &emu::supervisor::Application::haltAction,           "haltAction");
  calibration_signature_ = toolbox::task::bind(this, &emu::supervisor::Application::calibrationAction,    "calibrationAction");
  sequencer_signature_   = toolbox::task::bind(this, &emu::supervisor::Application::calibrationSequencer, "calibrationSequencer");
  
  fsm_.addState('H', "Halted",     this, &emu::supervisor::Application::stateChanged);
  fsm_.addState('C', "Configured", this, &emu::supervisor::Application::stateChanged);
  fsm_.addState('E', "Running",    this, &emu::supervisor::Application::stateChanged);
  
  fsm_.setStateName('F', "Error");
  fsm_.setFailedStateTransitionAction(this, &emu::supervisor::Application::transitionFailed);
  fsm_.setFailedStateTransitionChanged(this, &emu::supervisor::Application::stateChanged);
  
  //fsm_.setStateName('F',"Error", this, &emu::supervisor::Application::transitionFailed);
  
  //	fsm_.addState('c', "Configuring", this, &emu::supervisor::Application::stateChanged);
  
  fsm_.addStateTransition('H', 'C', "Configure", this, &emu::supervisor::Application::configureAction);
  fsm_.addStateTransition('C', 'C', "Configure", this, &emu::supervisor::Application::configureAction);
  fsm_.addStateTransition('C', 'E', "Start",     this, &emu::supervisor::Application::startAction);
  fsm_.addStateTransition('E', 'C', "Stop",      this, &emu::supervisor::Application::stopAction);
  fsm_.addStateTransition('C', 'H', "Halt",      this, &emu::supervisor::Application::haltAction);
  fsm_.addStateTransition('E', 'H', "Halt",      this, &emu::supervisor::Application::haltAction);
  fsm_.addStateTransition('H', 'H', "Halt",      this, &emu::supervisor::Application::haltAction);
  fsm_.addStateTransition('E', 'E', "SetTTS",    this, &emu::supervisor::Application::setTTSAction);
  
  // Define invalid transitions, too, so that they can be ignored, or else FSM will be unhappy when one is fired.
  fsm_.addStateTransition('E', 'E', "Configure", this, &emu::supervisor::Application::noAction);
  fsm_.addStateTransition('H', 'H', "Start"    , this, &emu::supervisor::Application::noAction);
  fsm_.addStateTransition('E', 'E', "Start"    , this, &emu::supervisor::Application::noAction);
  fsm_.addStateTransition('H', 'H', "Stop"     , this, &emu::supervisor::Application::noAction);
  fsm_.addStateTransition('C', 'C', "Stop"     , this, &emu::supervisor::Application::noAction);


  fsm_.setInitialState('H');
  fsm_.reset();
  
  state_ = fsm_.getStateName(fsm_.getCurrentState());

  getAppDescriptors();
  
  state_table_.addApplication("emu::fed::Manager");
  state_table_.addApplication("emu::pc::EmuPeripheralCrateManager");
  state_table_.addApplication("emu::daq::manager::Application");
  state_table_.addApplication("ttc::TTCciControl");
  state_table_.addApplication("ttc::LTCControl");
  // state_table_.addApplication("tcds::deadwood::DeadWood");
  state_table_.addApplication("tcds::ici::ICIController");
  state_table_.addApplication("tcds::pi::PIController");
  state_table_.addApplication("tcds::lpm::LPMController");
  // last_log_.size(N_LOG_MESSAGES);
  
  LOG4CPLUS_INFO(logger_, "emu::supervisor::Application constructed for " << state_table_ );
}

xdaq::ApplicationDescriptor* emu::supervisor::Application::findAppDescriptor( const string& klass, const string& service ){
  // Return the first app descriptor with the specified class and service name or NULL if no such app.
  std::set<xdaq::ApplicationDescriptor *> apps = getApplicationContext()->getDefaultZone()->getApplicationDescriptors( klass );
  for ( std::set<xdaq::ApplicationDescriptor *>::iterator ad = apps.begin(); ad != apps.end(); ++ad ){
    if ( (*ad)->getAttribute( "service" ) == service ) return *ad;
  }
  return NULL;
}

void emu::supervisor::Application::getAppDescriptors(){

  try {
    daq_descr_ = getApplicationContext()->getDefaultZone()
      ->getApplicationDescriptor("emu::daq::manager::Application", 0);
  } catch (xdaq::exception::ApplicationDescriptorNotFound& e) {
    LOG4CPLUS_ERROR(logger_, "Failed to get application descriptor for local DAQ Manager. "
		    << xcept::stdformat_exception_history(e));
    stringstream ss;
    ss <<  "Failed to get application descriptor for local DAQ Manager. ";
    XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss.str(), e );
    this->notifyQualified( "error", eObj );
  }

  try {
    ttc_descr_ = getApplicationContext()->getDefaultZone()
      ->getApplicationDescriptor("ttc::TTCciControl", 0);
    isUsingTCDS_ = ( ttc_descr_ == NULL );
    if ( ! isUsingTCDS_ ){
      LOG4CPLUS_WARN(logger_, "Legacy TTC application is found and will be used instead of TCDS.");
    }
  } catch (xdaq::exception::ApplicationDescriptorNotFound& e) {
    LOG4CPLUS_ERROR(logger_, "Failed to get application descriptor for ttc::TTCciControl. "
		    << xcept::stdformat_exception_history(e));
    stringstream ss;
    ss <<  "Failed to get application descriptor for ttc::TTCciControl. ";
    XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss.str(), e );
    this->notifyQualified( "error", eObj );
  }

  try {
    tf_descr_ = getApplicationContext()->getDefaultZone()
      ->getApplicationDescriptor( TFCellClass_.toString(), TFCellInstance_.value_ );
  } catch (xdaq::exception::ApplicationDescriptorNotFound& e) {
    stringstream ss;
    ss << "No Track Finder application \"" << TFCellClass_.toString() 
       << "\" of instance " << TFCellInstance_.value_ << " found.";
    LOG4CPLUS_ERROR(logger_, ss.str() << xcept::stdformat_exception_history(e));
    XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss.str(), e );
    this->notifyQualified( "error", eObj );
  }  
  
  getTCDSAppDescriptors();
}

void emu::supervisor::Application::getTCDSAppDescriptors(){
  //
  // PI apps
  //

  try {
    pi_plus_descr_ = findAppDescriptor("tcds::pi::PIController", "pi-cscp");
    if ( pi_plus_descr_ == NULL ) XCEPT_RAISE( xdaq::exception::ApplicationDescriptorNotFound, "No such application." );
    pi_plus_ = new PIControl( this, pi_plus_descr_, "ME+" );
  } catch (xdaq::exception::ApplicationDescriptorNotFound& e) {
    stringstream ss;
    ss << "Failed to get application descriptor for tcds::pi::PIController, service pi-cscp ";
    LOG4CPLUS_ERROR( logger_, ss.str() << xcept::stdformat_exception_history(e) );
    XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss.str(), e );
    this->notifyQualified( "error", eObj );
  }

  try {
    pi_minus_descr_ = findAppDescriptor("tcds::pi::PIController", "pi-cscm");
    if ( pi_minus_descr_ == NULL )  XCEPT_RAISE( xdaq::exception::ApplicationDescriptorNotFound, "No such application." );
    pi_minus_ = new PIControl( this, pi_minus_descr_, "ME-" );
  } catch (xdaq::exception::ApplicationDescriptorNotFound& e) {
    stringstream ss;
    ss << "Failed to get application descriptor for tcds::pi::PIController, service pi-cscm ";
    LOG4CPLUS_ERROR( logger_, ss.str() << xcept::stdformat_exception_history(e) );
    XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss.str(), e );
    this->notifyQualified( "error", eObj );
  }

  try {
    pi_tf_descr_ = findAppDescriptor("tcds::pi::PIController", "pi-csctf");
    if ( pi_tf_descr_ == NULL )  XCEPT_RAISE( xdaq::exception::ApplicationDescriptorNotFound, "No such application." );
    pi_tf_ = new PIControl( this, pi_tf_descr_, "TF" );
  } catch (xdaq::exception::ApplicationDescriptorNotFound& e) {
    stringstream ss;
    ss << "Failed to get application descriptor for tcds::pi::PIController, service pi-csctf ";
    LOG4CPLUS_ERROR( logger_, ss.str() << xcept::stdformat_exception_history(e) );
    XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss.str(), e );
    this->notifyQualified( "error", eObj );
  }

  //
  // iCI apps
  //

  try {
    ci_plus_descr_ = findAppDescriptor("tcds::ici::ICIController", "ici-cscp");
    if ( ci_plus_descr_ == NULL ) XCEPT_RAISE( xdaq::exception::ApplicationDescriptorNotFound, "No such application." );
    ci_plus_ = new CIControl( this, ci_plus_descr_, "ME+" );
  } catch (xdaq::exception::ApplicationDescriptorNotFound& e) {
    stringstream ss;
    ss << "Failed to get application descriptor for tcds::ici::ICIController, service ici-cscp ";
    LOG4CPLUS_ERROR( logger_, ss.str() << xcept::stdformat_exception_history(e) );
    XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss.str(), e );
    this->notifyQualified( "error", eObj );
  }

  try {
    ci_minus_descr_ = findAppDescriptor("tcds::ici::ICIController", "ici-cscm");
    if ( ci_minus_descr_ == NULL )  XCEPT_RAISE( xdaq::exception::ApplicationDescriptorNotFound, "No such application." );
    ci_minus_ = new CIControl( this, ci_minus_descr_, "ME-" );
  } catch (xdaq::exception::ApplicationDescriptorNotFound& e) {
    stringstream ss;
    ss << "Failed to get application descriptor for tcds::ici::ICIController, service ici-cscm ";
    LOG4CPLUS_ERROR( logger_, ss.str() << xcept::stdformat_exception_history(e) );
    XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss.str(), e );
    this->notifyQualified( "error", eObj );
  }

  try {
    ci_tf_descr_ = findAppDescriptor("tcds::ici::ICIController", "ici-csctf");
    if ( ci_tf_descr_ == NULL )  XCEPT_RAISE( xdaq::exception::ApplicationDescriptorNotFound, "No such application." );
    ci_tf_ = new CIControl( this, ci_tf_descr_, "TF" );
  } catch (xdaq::exception::ApplicationDescriptorNotFound& e) {
    stringstream ss;
    ss << "Failed to get application descriptor for tcds::ici::ICIController, service ici-csctf ";
    LOG4CPLUS_ERROR( logger_, ss.str() << xcept::stdformat_exception_history(e) );
    XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss.str(), e );
    this->notifyQualified( "error", eObj );
  }

  //
  // LPM app
  //

  try {
    pm_descr_ = findAppDescriptor("tcds::lpm::LPMController", "lpm-csc");
    if ( pm_descr_ == NULL ) XCEPT_RAISE( xdaq::exception::ApplicationDescriptorNotFound, "No such application." );
    pm_ = new PMControl( this, pm_descr_, "ME" );
  } catch (xdaq::exception::ApplicationDescriptorNotFound& e) {
    stringstream ss;
    ss << "Failed to get application descriptor for tcds::lpm::LPMController, service lpm-csc ";
    LOG4CPLUS_ERROR( logger_, ss.str() << xcept::stdformat_exception_history(e) );
    XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss.str(), e );
    this->notifyQualified( "error", eObj );
  }

}

xoap::MessageReference emu::supervisor::Application::onConfigure(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  isCommandFromWeb_ = false;
  run_number_ = 1;
  nevents_ = -1;
  
  submit(configure_signature_);
  
  return createReply(message);
}

xoap::MessageReference emu::supervisor::Application::onStart(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  isCommandFromWeb_ = false;
  submit(start_signature_);
  
  return createReply(message);
}

xoap::MessageReference emu::supervisor::Application::onStop(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  isCommandFromWeb_ = false;

  submit(stop_signature_);

  return createReply(message);
}

xoap::MessageReference emu::supervisor::Application::onHalt(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  isCommandFromWeb_ = false;
  quit_calibration_ = true;
  
  submit(halt_signature_);
  
  return createReply(message);
}

xoap::MessageReference emu::supervisor::Application::onReset(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  isCommandFromWeb_ = false;
  resetAction();
  
  return onHalt(message);
}

xoap::MessageReference emu::supervisor::Application::onSetTTS(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  isCommandFromWeb_ = false;
  try{
    fireEvent("SetTTS");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xoap::exception::Exception, "SetTTS failed", e );
  }

  return createReply(message);
}

xoap::MessageReference emu::supervisor::Application::onRunSequence(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  isCommandFromWeb_ = false;
  calib_wl_->submit( sequencer_signature_ );
  return createReply(message);
}


void emu::supervisor::Application::webDefault(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  if (keep_refresh_ || bool(isInCalibrationSequence_)) {
    HTTPResponseHeader &header = out->getHTTPResponseHeader();
    header.addHeader("Refresh", "2");
  }
  
  // Header
  *out << HTMLDoctype(HTMLDoctype::eStrict) << endl;
  *out << html() << endl;
  
  *out << head() << endl;
  *out << title(string("Supervisor ")+state_.toString()) << endl;
  *out << cgicc::link().set("rel", "stylesheet")
    .set("href", "/emu/supervisor/html/emusupervisor.css")
    .set("type", "text/css") << endl;
  *out << head() << endl;
  
  // Body
  *out << body() << endl;
  
  *out << table() << tr();

  *out << td();
  // Config listbox
  *out << form().set("action",
		     "/" + getApplicationDescriptor()->getURN() + "/Configure") << endl;
  
  int n_keys = ( isUsingTCDS_ ? runParameters_.size() : config_keys_.size() );
  
  *out << "Run Type: " << endl;
  *out << cgicc::select().set("name", "runtype") << endl;
  
  int selected_index = keyToIndex(run_type_.toString());
  
  for (int i = 0; i < n_keys; ++i) {
    if (i == selected_index) {
      *out << option()
	.set("value", ( isUsingTCDS_ ? (string)(runParameters_[i].bag.key_) : (string)config_keys_[i] ) )
	.set("selected", "");
    } else {
      *out << option()
	.set("value", ( isUsingTCDS_ ? (string)runParameters_[i].bag.key_ : (string)config_keys_[i] ) );
    }
    *out << ( isUsingTCDS_ ? (string)runParameters_[i].bag.key_ : (string)config_keys_[i] ) << option() << endl;
    // cout << "n_keys " << n_keys
    // 	 << " selected_index " << selected_index
    // 	 << " (string)runParameters_[" << i << "].bag.key_ " << (string)runParameters_[i].bag.key_.toString()
    // 	 << " (string)runParameters_[" << i << "].bag.loop_ " << (string)runParameters_[i].bag.loop_.toString()
    // 	 << " run_type_ " << run_type_.toString()
    // 	 << endl;
  }
  
  *out << cgicc::select() << endl;
  
  *out << input().set("type", "submit")
    .set("name", "command")
    .set("title", "Configure the selected run.")
    .set("value", "Configure") << endl;
  *out << form() << endl;
  *out << td();

  if ( allCalibrationRuns() ){

    *out << td() << "OR" << td();
    
    *out << td();
    *out << form().set("action",
		       "/" + getApplicationDescriptor()->getURN() + "/RunSequence") << endl;
    *out << input().set("type", "submit")
      .set("name", "command")
      .set("title", "Take all calibration runs in an automatic sequence.")
      .set("value", "Run all calibrations") << endl;
    *out << form() << endl;
    *out << td();
  }

  *out << tr() << table();

  /*
   *out << "Run Number: " << endl;
   *out << input().set("type", "text")
   .set("name", "runnumber")
   .set("value", run_number_.toString())
   .set("size", "40") << br() << endl;
   
   *out << "Max # of Events: " << endl;
   *out << input().set("type", "text")
   .set("name", "nevents")
   .set("value", toString(nevents_))
   .set("size", "40") << br() << endl;
  */
  
  // Buttons
  
  *out << table() << tr();

  *out << td() << form().set("action",
			"/" + getApplicationDescriptor()->getURN() + "/Start") << endl;
  *out << input().set("type", "submit")
    .set("title", "Start the configured run.")
    .set("name", "command")
    .set("value", "Start") << endl;
  *out << form() << td() << endl;
  
  *out << td() << form().set("action",
			     "/" + getApplicationDescriptor()->getURN() + "/Stop") << endl;
  *out << input().set("type", "submit")
    .set("title", "Stop the ongoing run.")
    .set("name", "command")
    .set("value", "Stop") << endl;
  *out << form() << td() << endl;
  
  *out << td() << form().set("action",
			     "/" + getApplicationDescriptor()->getURN() + "/Halt") << endl;
  *out << input().set("type", "submit")
    .set("title", "Halt the configured or ongoing run.")
    .set("name", "command")
    .set("value", "Halt") << endl;
  *out << form() << td() << endl;
  
  *out << td() << form().set("action",
			     "/" + getApplicationDescriptor()->getURN() + "/Reset") << endl;
  *out << input().set("type", "submit")
    .set("name", "command")
    .set("value", "Reset") << endl;
    *out << form() << td() << endl;

  *out << tr() << table();
  
  // TTS operation
  if (hide_tts_control_) {
    *out << form().set("action",
		       "/" + getApplicationDescriptor()->getURN() + "/SwitchTTS") << endl;
    *out << input().set("type", "submit")
      .set("name", "command")
      .set("value", "ShowTTSControl") << endl;
    *out << form() << endl;
  } else {
    *out << form().set("action",
		       "/" + getApplicationDescriptor()->getURN() + "/SetTTS") << endl;
    
    *out << small() << "(See, "
	 << a().set("href", "https://twiki.cern.ch/twiki/bin/view/CMS/CSCcommissioningFED#FED_Mapping")
	 << "FED commissioning Wiki page"
	 << a()
	 << " to find out sTTS IDs (FMM-ID).)" << small() << br() << endl;
    *out << "sTTS ID: (750-757,760,830-869)" << endl;
    *out << input().set("type", "text")
      .set("name", "tts_id")
				.set("value", tts_id_.toString())
      .set("size", "10") << br() << endl;
    
    *out << "sTTS value: (0-15)" << endl;
    *out << input().set("type", "text")
      .set("name", "tts_bits")
      .set("value", tts_bits_.toString())
      .set("size", "10") << br() << endl;
    
    *out << input().set("type", "submit")
      .set("name", "command")
      .set("value", "SetTTS") << endl;
    *out << form() << endl;
    
    *out << form().set("action",
		       "/" + getApplicationDescriptor()->getURN() + "/SwitchTTS") << endl;
    *out << input().set("type", "submit")
      .set("name", "command")
      .set("value", "HideTTSControl") << endl;
    *out << form() << endl;
  }
  
  // Error message, if exists.
  if (!error_message_.empty()) {
    *out  << p() << span().set("style", "color: red;")
	  << error_message_ << span() << p() << endl;
    error_message_ = "";
  }
  
  // Configuration parameters
  *out << hr() << endl;

  *out << table(); 
  *out << tr() << td() << "Run type: " << td() << td() << run_type_.toString();
  if ( isUsingTCDS_ ){
    if ( bool(isInCalibrationSequence_) ){
      *out << " (run " << keyToIndex(run_type_.toString())+1 << " in an automatic sequence of "
	   << runParameters_.size() << " calibration runs)";
    }
    *out << td() << tr() << endl;
    if ( isCalibrationMode() ){
      int index = keyToIndex(run_type_);
      *out << tr() 
	   << td() << "Steps completed: " << td() 
	   << td() << step_counter_ << " of " << runParameters_[index].bag.loop_ << td() 
	   << tr() << endl;
    }
  }
  else{ 
    // using legacy TTC
    if ( bool(isInCalibrationSequence_) ){
      *out << " (run " << getCalibParamIndex(run_type_.toString())+1 << " in an automatic sequence of "
	   << calib_params_.size() << " calibration runs)";
    }
    *out << td() << tr() << endl;
    if ( isCalibrationMode() ){
      int index = getCalibParamIndex(run_type_);
      *out << tr() 
	   << td() << "Steps completed: " << td() 
	   << td() << step_counter_ << " of " << calib_params_[index].bag.loop_ << td() 
	   << tr() << endl;
    }
  }
  refreshConfigParameters();
  
  *out << tr() << td() << "TTCci inputs(Clock:Orbit:Trig:BGo): " <<  td() << td() << ttc_source_.toString() << td() << tr() << endl;
  
  *out << tr() << td() << "Mode of DAQManager: " <<  td() << td() << daq_mode_.toString() << td() << tr() << endl;
  string localDAQState = getLocalDAQState();
  *out << tr() << td() << "Local DAQ state: " << td() << td() << span().set("class",localDAQState) << localDAQState << span() << td() << tr() << endl;

  *out << tr() << td() << "State of TF operation " << cite() << TFCellOpName_.toString() << cite() << ": " << td() << td() 
       << span().set("class",TFCellOpState_.toString()) << TFCellOpState_.toString() 
       << span() << td() << tr() << endl;
  *out << table(); 
  
  // Application states
  *out << hr() << endl;
  state_table_.webOutput(out, (string)state_);
  
    // Single TCDS commands, available only with LPM
  *out << hr() << endl;
  if ( pm_ ){
    char state = fsm_.getCurrentState();
    
    *out << table(); 
    *out << tr();
    
    *out << td() << form().set("action", "/" + getApplicationDescriptor()->getURN() + "/ResyncViaTCDS");
    *out << "<input type='submit' name='command' value='Single Resync' title='Attention! This will send a single, unprotected resync.'" 
	 << ( state == 'C' || state == 'E' ? "" : " disabled='disabled'" ) << "/>";
    *out << form() << td();
    
    *out << td() << form().set("action", "/" + getApplicationDescriptor()->getURN() + "/HardResetViaTCDS");
    *out << "<input type='submit' name='command' value='Single HardReset' title='Attention! This will send a single, unprotected hard reset.'" 
	 << ( state == 'C' || state == 'E' ? "" : " disabled='disabled'" ) << "/>";
    *out << form() << td();
    
    *out << td() << form().set("action", "/" + getApplicationDescriptor()->getURN() + "/BgoTrainViaTCDS");
    *out << "<input type='submit' name='command' value='Send Bgo train' title='Send the selected Bgo train. Use this to send Bgos during the run.'" 
	 << ( state == 'C' || state == 'E' ? "" : " disabled='disabled'" ) << "/>";
    *out << "<select name='bgoTrainName' title='Select a Bgo train to send.'" 
	 << ( state == 'C' || state == 'E' ? "" : " disabled='disabled'" ) << ">";
    *out << "<option value='HardReset' title='Disable trigger, hard reset, resync, EC0 and enable trigger.'>Hard Reset</option>";
    *out << "<option value='Resync' title='Disable trigger, resync, EC0 and enable trigger.'>Resync</option>";
    // *out << "<option value='Pause' title='Disable trigger.'>Pause</option>";
    // *out << "<option value='Resume' title='Enable trigger.'>Resume</option>";
    *out << "</select>";
    *out << form() << td();

    *out << tr();
    *out << table(); 
  }  

  // Reason for failure
  if ( reasonForFailure_.toString().length() > 0 ){
    *out << hr()
	 << p()<< "Reason for failure:" << p() 
	 << code() << span().set("style", "color: red;") << withoutString( "<![CDATA[", withoutString( "]]>", reasonForFailure_.toString() ) ) << span() << code() << "\n";
  }

  // Message logs
  *out << hr() << endl;
  // last_log_.webOutput(out);
  
  *out << body() << html() << endl;
}

void emu::supervisor::Application::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  string value;
  isCommandFromWeb_ = true;
  
  value = getCGIParameter(in, "runtype");
  if (value.empty()) { error_message_ += "Please select run type.\n"; }
  run_type_ = value;
  
  run_number_ = 1;
  nevents_ = -1;
  
  if (error_message_.empty()) {
    submit(configure_signature_);
  }
  
  keep_refresh_ = true;
  webRedirect(in, out);
}

void emu::supervisor::Application::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  isCommandFromWeb_ = true;
  // Book run number here to make sure it's done 
  // only when requested by the user from the web page,
  // and not by the FunctionManager via SOAP.
  // Also, no point booking a run number if local DAQ is not available.
  if ( daq_descr_ != NULL ) bookRunNumber();

  try{
    fireEvent("Start");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Start failed", e );
  }
  
  keep_refresh_ = true;
  webRedirect(in, out);
  
  if (!isCalibrationMode()) {
    keep_refresh_ = false;
  }
}

void emu::supervisor::Application::webStop(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  isCommandFromWeb_ = true;

  try{
    fireEvent("Stop");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Stop failed", e );
  }
  
  keep_refresh_ = true;
  webRedirect(in, out);
}

void emu::supervisor::Application::webHalt(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  isCommandFromWeb_ = true;
  quit_calibration_ = true;
  
  try{
    fireEvent("Halt");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Halt failed", e );
  }

  keep_refresh_ = true;
  webRedirect(in, out);
}

void emu::supervisor::Application::webReset(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  isCommandFromWeb_ = true;
  resetAction();
  
  webHalt(in, out);
}

void emu::supervisor::Application::webSetTTS(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  isCommandFromWeb_ = true;
  tts_id_.fromString(getCGIParameter(in, "tts_id"));
  tts_bits_.fromString(getCGIParameter(in, "tts_bits"));
  
  if (error_message_.empty()) {
    try{
      fireEvent("SetTTS");
    }
    catch( toolbox::fsm::exception::Exception& e ){
      XCEPT_RETHROW( xgi::exception::Exception, "SetTTS failed", e );
    }
  }
  
  webRedirect(in, out);
}

void emu::supervisor::Application::webSwitchTTS(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  isCommandFromWeb_ = true;
  hide_tts_control_ = getCGIParameter(in, "command").find("Hide", 0) == 0;
  
  webRedirect(in, out);
}

void emu::supervisor::Application::webRunSequence(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  string value;
  isCommandFromWeb_ = true;

  // No point booking a run number if local DAQ is not available.
  if ( daq_descr_ != NULL ) bookRunNumber();
  
  calib_wl_->submit( sequencer_signature_ );
   
  keep_refresh_ = true;
  webRedirect(in, out);
}

void emu::supervisor::Application::webResyncViaTCDS(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  isCommandFromWeb_ = true;
  
  if ( !pm_ ){
    XCEPT_RAISE( xgi::exception::Exception, "Failed to issue resync via TCDS as no LPM appliation was found." );
  }

  try{
    xdata::String Resync( "Resync" );
    pm_->sendBgo( Resync );
  }
  catch( xcept::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Failed to issue resync via TCDS.", e );
  }

  keep_refresh_ = true;
  webRedirect(in, out);
}

void emu::supervisor::Application::webHardResetViaTCDS(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  isCommandFromWeb_ = true;
  
  if ( !pm_ ){
    XCEPT_RAISE( xgi::exception::Exception, "Failed to issue hard reset via TCDS as no LPM appliation was found." );
  }

  try{
    xdata::String HardReset( "HardReset" );
    pm_->sendBgo( HardReset );
  }
  catch( xcept::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Failed to issue hard reset via TCDS.", e );
  }

  keep_refresh_ = true;
  webRedirect(in, out);
}


void emu::supervisor::Application::webBgoTrainViaTCDS(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  isCommandFromWeb_ = true;
  
  if ( !pm_ ){
    XCEPT_RAISE( xgi::exception::Exception, "Failed to send Bgo train via TCDS as no LPM appliation was found." );
  }

  try{
    xdata::String bgoTrainName( getCGIParameter(in, "bgoTrainName") );
    if ( !bgoTrainName.toString().empty() ) pm_->sendBgoTrain( bgoTrainName );
  }
  catch( xcept::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Failed to send Bgo train via TCDS.", e );
  }

  keep_refresh_ = true;
  webRedirect(in, out);
}


void emu::supervisor::Application::webRedirect(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  string url = in->getenv("PATH_TRANSLATED");
  
  HTTPResponseHeader &header = out->getHTTPResponseHeader();
  
  header.getStatusCode(303);
  header.getReasonPhrase("See Other");
  header.addHeader("Location",
		   url.substr(0, url.find("/" + in->getenv("PATH_INFO"))));
}

bool emu::supervisor::Application::configureAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Configure");
  
  return false;
}

bool emu::supervisor::Application::startAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Start");
  
  return false;
}

bool emu::supervisor::Application::stopAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Stop");
  
  return false;
}

bool emu::supervisor::Application::haltAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Halt");
  
  return false;
}

bool emu::supervisor::Application::calibrationAction(toolbox::task::WorkLoop *wl)
{
  LOG4CPLUS_DEBUG(logger_, "calibrationAction " << "(begin)");
  
  if ( isUsingTCDS_ ){

    unsigned int index = std::max( 0, keyToIndex(run_type_) );

    unsigned int nRuns = ( bool( isInCalibrationSequence_ ) ? runParameters_.size() : 1 );
    unsigned int iRun  = ( bool( isInCalibrationSequence_ ) ? index                : 0 );
    
    LOG4CPLUS_DEBUG( logger_, "Calibration"
		     << "\n   command: " << runParameters_[index].bag.command_.toString()
		     << "\n   loop: "    << runParameters_[index].bag.loop_.toString()
		     << "\n   delay: "   << runParameters_[index].bag.delay_.toString()
		     << "\n   ci:\n"     << runParameters_[index].bag.ci_.toString() 
		     << "\n   ci_p:\n"   << runParameters_[index].bag.ci_p_.toString() 
		     << "\n   ci_m:\n"   << runParameters_[index].bag.ci_m_.toString() 
		     << "\n   ci_tf:\n"  << runParameters_[index].bag.ci_tf_.toString() 
		     << "\n   pm:\n"     << runParameters_[index].bag.pm_.toString() 
		     << "\n   pi:\n"     << runParameters_[index].bag.pi_.toString() );
    
    emu::soap::Messenger m( this );
    
    for (step_counter_ = 0; step_counter_ < runParameters_[index].bag.loop_; ++step_counter_) {
      if (quit_calibration_) { break; }
      LOG4CPLUS_DEBUG(logger_, "calibrationAction: " << step_counter_);
      
      m.sendCommand( "emu::pc::EmuPeripheralCrateManager", runParameters_[index].bag.command_.toString() );

      // xdata::String attributeValue( "Start" );
      // m.sendCommand( "ttc::LTCControl", "Cyclic", emu::soap::Parameters::none, emu::soap::Attributes().add( "Param", &attributeValue ) );
      
      if ( pm_ && !quit_calibration_ ){
	LOG4CPLUS_DEBUG(logger_, "Sending Start Bgo to LPM");
	xdata::String Start( "Start" );
	pm_->initCyclicGenerators().sendBgo( Start );
      }

      if (quit_calibration_) { break; }
      
      sendCalibrationStatus( iRun, nRuns, step_counter_, runParameters_[index].bag.loop_ );
      
      sleep( runParameters_[index].bag.delay_ );

      if ( pm_ ){
	LOG4CPLUS_DEBUG(logger_, "Sending Stop Bgo to LPM");
	xdata::String Stop( "Stop" );
	pm_->sendBgo( Stop );
      }

      if (quit_calibration_) { break; }
    }
  
    sendCalibrationStatus( ( iRun+1 == nRuns ? nRuns : iRun ), nRuns, step_counter_, runParameters_[index].bag.loop_ );
  } // if ( isUsingTCDS_ )
  else{
    unsigned int index = std::max( 0, getCalibParamIndex(run_type_) );
    
    unsigned int nRuns = ( bool( isInCalibrationSequence_ ) ? calib_params_.size() : 1 );
    unsigned int iRun  = ( bool( isInCalibrationSequence_ ) ? index                : 0 );
    
    LOG4CPLUS_DEBUG( logger_, "Calibration" << endl
		     << "command: " << calib_params_[index].bag.command_.toString()       << endl
		     << "loop: "    << calib_params_[index].bag.loop_.toString()	        << endl
		     << "delay: "   << calib_params_[index].bag.delay_.toString()	        << endl
		     << "ltc: "     << endl << calib_params_[index].bag.ltc_.toString()   << endl
		     << "ttcci: "   << endl << calib_params_[index].bag.ttcci_.toString() << endl );
    
    emu::soap::Messenger m( this );
    
    for (step_counter_ = 0; step_counter_ < calib_params_[index].bag.loop_; ++step_counter_) {
      if (quit_calibration_) { break; }
      LOG4CPLUS_DEBUG(logger_, "calibrationAction: " << step_counter_);
      
      m.sendCommand( "emu::pc::EmuPeripheralCrateManager", calib_params_[index].bag.command_.toString() );
      
      xdata::String attributeValue( "Start" );
      m.sendCommand( "ttc::LTCControl", "Cyclic", emu::soap::Parameters::none, emu::soap::Attributes().add( "Param", &attributeValue ) );
      sendCalibrationStatus( iRun, nRuns, step_counter_, calib_params_[index].bag.loop_ );
      
      sleep( calib_params_[index].bag.delay_ );
    }
  
    sendCalibrationStatus( ( iRun+1 == nRuns ? nRuns : iRun ), nRuns, step_counter_, calib_params_[index].bag.loop_ );
  } // if ( isUsingTCDS_ ) else

  if (!quit_calibration_) {
    submit(halt_signature_);
  }

  LOG4CPLUS_DEBUG(logger_, "calibrationAction " << "(end)");

  return false;
}

bool emu::supervisor::Application::calibrationSequencer(toolbox::task::WorkLoop *wl)
{
  // Do all calibrations in one go.
  LOG4CPLUS_DEBUG(logger_, "calibrationSequencer " << "(begin)");
  isInCalibrationSequence_ = true;
  size_t nRunTypes = ( isUsingTCDS_ ? runParameters_.size() : calib_params_.size() );
  for ( size_t i=0; i<nRunTypes && fsm_.getCurrentState() != 'F'; ++i ){
    run_type_ = ( isUsingTCDS_ ? runParameters_[i].bag.key_ : calib_params_[i].bag.key_ );
    if ( !quit_calibration_ ){
      submit(configure_signature_);
      if ( waitForAppsToReach("Configured",true,60) ){
	if ( fsm_.getCurrentState() != 'F' ) submit(start_signature_); // This is supposed to halt itself when done.
      }
      else{
	if ( fsm_.getCurrentState() != 'F' ) submit(halt_signature_);
      }
      waitForAppsToReach("Halted",true);
    }
  }
  isInCalibrationSequence_ = false;
  // Keep refreshing the web page so that it can be seen that the sequence has finished.
  keep_refresh_ = true;
  LOG4CPLUS_DEBUG(logger_, "calibrationSequencer " << "(end)");
  return false;
}

void emu::supervisor::Application::sendCalibrationStatus( unsigned int iRun, unsigned int nRuns, unsigned int iStep, unsigned int nSteps ){
  emu::soap::Messenger m(this);
  
  xdata::UnsignedInteger calibNRuns    ( nRuns  );
  xdata::UnsignedInteger calibNSteps   ( nSteps );
  xdata::UnsignedInteger calibRunIndex ( iRun   );
  xdata::UnsignedInteger calibStepIndex( iStep  );

  try{
    if ( daq_descr_ != NULL ) m.setParameters( daq_descr_, 
					       emu::soap::Parameters()
					       .add( "calibNRuns"    , &calibNRuns     )
					       .add( "calibNSteps"   , &calibNSteps    )
					       .add( "calibRunIndex" , &calibRunIndex  )
					       .add( "calibStepIndex", &calibStepIndex ) );
  }
  catch( xcept::Exception &e ){
    LOG4CPLUS_WARN( logger_, "Failed to send calibration status to emu::daq::manager::Application : " << xcept::stdformat_exception_history(e) );
  }

}

void emu::supervisor::Application::configureAction(toolbox::Event::Reference evt) 
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_DEBUG(logger_, evt->type() << "(begin)");
  LOG4CPLUS_DEBUG(logger_, "runtype: " << run_type_.toString()
		  << " runnumber: " << run_number_ << " nevents: " << nevents_.toString());
  

  rcmsStateNotifier_.findRcmsStateListener();      	
  step_counter_ = 0;

  // Get TCDS app descriptors for primary or secondary TCDS, depending on the usePrimaryTCDS parameter.
  getTCDSAppDescriptors();

  emu::soap::Messenger m( this );

  try {

    //
    // Clean up leftover ops and halt apps
    //
    
    if ( tf_descr_ != NULL && controlTFCellOp_.value_ ){
      TFCellOpState_ = OpGetStateCell();
      if ( TFCellOpState_.toString() != "UNKNOWN" ){
	// Reset csctf-cell operation
	OpResetCell();
	waitForTFCellOpToReach("halted",60);
      }
    }

    try {
      state_table_.refresh();
      if (state_table_.getState("emu::daq::manager::Application", 0) != "Halted") {
	if ( isDAQManagerControlled("Halt") ){
	  m.sendCommand( "emu::daq::manager::Application", 0, "Halt" );
	  waitForDAQToExecute("Halt", 10);
	}
      }
       
      if ( ! isUsingTCDS_ ){
	if (state_table_.getState("ttc::TTCciControl", 0) != "halted") {
	  m.sendCommand( "ttc::TTCciControl", "reset" );
	}
	if (state_table_.getState("ttc::LTCControl", 0) != "halted") {
	  m.sendCommand( "ttc::LTCControl", "reset" );
	  // Allow ttc::LTCControl some time to halt:
	  ::sleep(2);
	}
      }
      else{
	if ( pm_       ) pm_      ->halt();
	if ( ci_plus_  ) ci_plus_ ->halt();
	if ( ci_minus_ ) ci_minus_->halt();
	if ( ci_tf_    ) ci_tf_   ->halt();
	if ( pi_plus_  ) pi_plus_ ->halt();
	if ( pi_minus_ ) pi_minus_->halt();
	if ( pi_tf_    ) pi_tf_   ->halt();
      }

      waitForAppsToReach("Halted",false,30);

    } catch (xcept::Exception ignored) {}
    

    //
    // Configure
    //

    // Tell DAQ Manager the run type, number of events, whether to write bad events only 
    // and whether global DAQ is running the show.
    // Keep the two parameters isGlobalInControl and writeBadEventsOnly separate 
    // as later we may need to write all events even when global DAQ is in control
    // (in centrally started calibration runs).
    xdata::Boolean isGlobalInControl( true );
    if ( isCalibrationMode() || bool( controlTFCellOp_ ) ) isGlobalInControl = false;
    try {
      LOG4CPLUS_INFO( logger_, "Sending to emu::daq::manager::Application : maxNumberOfEvents " << nevents_ .toString() 
		      << ", runType " << ( isCalibrationMode() ? run_type_.toString() : "Monitor" )
		      << ", isGlobalInControl " << isGlobalInControl.toString()
		      << ", writeBadEventsOnly " << localDAQWriteBadEventsOnly_.toString() );
      m.setParameters( "emu::daq::manager::Application", 
		       emu::soap::Parameters()
		       .add( "maxNumberOfEvents" , &nevents_                    )
		       .add( "runType"           , &run_type_                   )
		       .add( "isGlobalInControl" , &isGlobalInControl           )
		       .add( "writeBadEventsOnly", &localDAQWriteBadEventsOnly_ )
		       );
    } catch (xcept::Exception& e) {
      LOG4CPLUS_WARN( logger_, "Failed to send to emu::daq::manager::Application : maxNumberOfEvents " << nevents_ .toString() 
		      << ", runType " << run_type_.toString()
		      << ", isGlobalInControl " << isGlobalInControl.toString() 
		      << ", writeBadEventsOnly " << localDAQWriteBadEventsOnly_.toString() 
		      << ": " << xcept::stdformat_exception_history(e) );
    }

    
    // Configure local DAQ first as its FSM is driven asynchronously,
    // and it will probably finish the transition by the time the others do.
    try {
      if ( isDAQManagerControlled("Configure") ) m.sendCommand( "emu::daq::manager::Application", 0, "Configure" );
    } catch (xcept::Exception ignored) {}
    
    if (!isCalibrationMode()) {
      m.setResponseTimeout( 600 ); // Allow PCrates ample time to be configured.
      m.sendCommand( "emu::pc::EmuPeripheralCrateManager", "Configure" );
      m.resetResponseTimeout(); // Reset response timeout to default value.
    }
       
    // Configure TF Cell operation
    if ( tf_descr_ != NULL && controlTFCellOp_.value_ ){
      if ( waitForTFCellOpToReach("halted",60) ){
	sendCommandCell("configure");
	// Allow more time for 'configure' after key change. With a new key, it may take a couple of minutes.
	waitForTFCellOpToReach("configured",180);
      }
      if ( TFCellOpState_.toString() != "configured" ){
	stringstream ss;
	ss << "TF Cell Operation \"" << TFCellOpName_.toString() 
	   << "\" failed to reach configured state. Aborting.";
	XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss.str() );
	this->notifyQualified( "error", eObj );
	throw eObj;
      } 
    }


    if ( ! isUsingTCDS_ ){
      int index = getCalibParamIndex(run_type_);
      // Configure TTC
      if (index >= 0) {
	m.setParameters( "ttc::TTCciControl" , emu::soap::Parameters().add( "Configuration", &calib_params_[index].bag.ttcci_ ) );
      }
      m.sendCommand( "ttc::TTCciControl", "configure" );    
      // Configure LTC
      if (index >= 0) {
	m.setParameters( "ttc::LTCControl" , emu::soap::Parameters().add( "Configuration", &calib_params_[index].bag.ltc_ ) );
      }
      m.sendCommand( "ttc::LTCControl", "configure" );
    }
    else {
      int index = keyToIndex(run_type_);
      // Configure TCDS
      RegDumpPreprocessor pp;
      ostringstream ppMessages;
      pp.setOptions( RegDumpPreprocessor::expandRanges ).setMessageStream( ppMessages );
      xdata::String pi_conf   ( pp.setTitle("PI"   ).process( runParameters_[index].bag.pi_.toString() ) );
      xdata::String ci_p_conf ( pp.setTitle("CI P" ).process( runParameters_[index].bag.ci_.toString(), runParameters_[index].bag.ci_p_ .toString() ) );
      xdata::String ci_m_conf ( pp.setTitle("CI M" ).process( runParameters_[index].bag.ci_.toString(), runParameters_[index].bag.ci_m_ .toString() ) );
      xdata::String ci_tf_conf( pp.setTitle("CI TF").process( runParameters_[index].bag.ci_.toString(), runParameters_[index].bag.ci_tf_.toString() ) );
      xdata::String pm_conf   ( pp.setTitle("PM"   ).process( runParameters_[index].bag.pm_.toString() ) );
      if ( ppMessages.str().length() > 0 ){
	LOG4CPLUS_WARN( logger_, string( "TCDS register preprocessor says: " ) + ppMessages.str() );
      }
      // PIs
      if ( pi_plus_  ) pi_plus_ ->setRunType( run_type_ ).configure( pi_conf );
      if ( pi_minus_ ) pi_minus_->setRunType( run_type_ ).configure( pi_conf );
      if ( pi_tf_    ) pi_tf_   ->setRunType( run_type_ ).configure( pi_conf );
      if ( pi_plus_  ) pi_plus_ ->waitForState( "Configured", 30 );
      if ( pi_minus_ ) pi_minus_->waitForState( "Configured", 30 );
      if ( pi_tf_    ) pi_tf_   ->waitForState( "Configured", 30 );
      // CIs
      if ( ci_plus_  ) ci_plus_ ->setRunType( run_type_ ).configure( ci_p_conf  );
      if ( ci_minus_ ) ci_minus_->setRunType( run_type_ ).configure( ci_m_conf  );
      if ( ci_tf_    ) ci_tf_   ->setRunType( run_type_ ).configure( ci_tf_conf );
      if ( ci_plus_  ) ci_plus_ ->configureSequence(); // This waits for the state transition to complete.
      if ( ci_minus_ ) ci_minus_->configureSequence(); // This waits for the state transition to complete.
      if ( ci_tf_    ) ci_tf_   ->configureSequence(); // This waits for the state transition to complete.
      // LPM
      if ( pm_       ) pm_      ->setRunType( run_type_ ).configure( pm_conf ).configureSequence(); // This waits for the state transition to complete.
    } // if ( isUsingTCDS_ )

    xdata::String runType( "global" );
    if      ( isCalibrationMode()     ) runType = "calibration";
    else if ( controlTFCellOp_.value_ ) runType = "local";
    m.setParameters( "emu::fed::Manager", emu::soap::Parameters().add( "runType", &runType ) );
    // Configure FED
    m.sendCommand( "emu::fed::Manager", "Configure" );

    if (isCalibrationMode()) {
      m.setResponseTimeout( 600 ); // Allow PCrates ample time to be configured for calibration run.
		if (isAlctCalibrationMode())
		  m.sendCommand( "emu::pc::EmuPeripheralCrateManager", "ConfigCalALCT" );
		else
		  m.sendCommand( "emu::pc::EmuPeripheralCrateManager", "ConfigCalCFEB");
      m.resetResponseTimeout(); // Reset response timeout to default value.
    }   

    // By now the local DAQ must have finished configuring. Checking it is practically only needed
    // in tests when the local DAQ Manager is the only supervised app. We certainly don't need to do it
    // in global runs.
    try{
      if ( runType != "global" ) 
	if ( isDAQManagerControlled("Configure") ) waitForDAQToExecute("Configure", 5, true);
    } catch (xcept::Exception ignored) {}


    if ( !waitForAppsToReach("Configured",false,30) ){
      stringstream ss;
      ss << state_table_;
      XCEPT_RAISE(xcept::Exception,"Applications failed to reach 'Configured' state. Their current state: "+ss.str() );
    }

    // state_table_.refresh();
    // if (!state_table_.isValidState("Configured")) {
    //   stringstream ss;
    //   ss << state_table_;
    //   XCEPT_RAISE(xcept::Exception,
    // 		  "Applications got to unexpected states: "+ss.str() );
    // }
    refreshConfigParameters();
    
  } catch ( xoap::exception::Exception& e) {
    XCEPT_RETHROW( toolbox::fsm::exception::Exception, "Configure transition failed.", e );
  } catch ( xcept::Exception& e ) {
    XCEPT_RETHROW( toolbox::fsm::exception::Exception, "Configure transition failed.", e );
  } catch( std::exception& e ){
    XCEPT_RAISE( toolbox::fsm::exception::Exception, string( "Configure transition failed: " ) + e.what() );
  }
  
  state_table_.refresh();
  LOG4CPLUS_DEBUG(logger_,  "Current state is: [" << fsm_.getStateName (fsm_.getCurrentState()) << "]");
  LOG4CPLUS_DEBUG(logger_, evt->type() << "(end of this function)");
}

void emu::supervisor::Application::startAction(toolbox::Event::Reference evt) 
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_DEBUG(logger_, evt->type() << "(begin)");
  LOG4CPLUS_DEBUG(logger_, "runtype: " << run_type_.toString()
		  << " runnumber: " << run_number_ << " nevents: " << nevents_.toString());
  
  emu::soap::Messenger m( this );

  try {
    state_table_.refresh();

    m.setParameters( "emu::fed::Manager", emu::soap::Parameters().add( "runNumber", &run_number_ ) );
    m.sendCommand( "emu::fed::Manager", "Enable" );
    
    if (!isCalibrationMode()) {
      m.sendCommand( "emu::pc::EmuPeripheralCrateManager", "Enable" );
    }
    
    try {
      if (state_table_.getState("emu::daq::manager::Application", 0) == "Halted" &&
        isDAQManagerControlled("Configure")                                        ) {
    	m.setParameters( "emu::daq::manager::Application", emu::soap::Parameters().add( "maxNumberOfEvents", &nevents_ ) );
    	m.sendCommand( "emu::daq::manager::Application", 0, "Configure" );
    	if ( isCommandFromWeb_ ) waitForDAQToExecute("Configure", 60, true);
    	else                     waitForDAQToExecute("Configure", 2);
      }
      if ( isDAQManagerControlled("Enable") ) {
    	m.setParameters( "emu::daq::manager::Application", emu::soap::Parameters().add( "runNumber", &run_number_ ) );
    	m.sendCommand( "emu::daq::manager::Application", 0, "Enable" );
    	if ( isCommandFromWeb_ ) waitForDAQToExecute("Enable", 60, true);
    	else                     waitForDAQToExecute("Enable", 2);
      }
    } catch (xcept::Exception ignored) {}
    
    state_table_.refresh();
    

    if ( ! isUsingTCDS_ ){
      if (state_table_.getState("ttc::TTCciControl", 0) != "enabled") {
	m.sendCommand( "ttc::TTCciControl", "enable" );
      }
      if (state_table_.getState("ttc::LTCControl", 0) != "enabled") {
	m.sendCommand( "ttc::LTCControl", "enable" );
      }
      xdata::String attributeValue( "Stop" );
      m.sendCommand( "ttc::LTCControl", "Cyclic", emu::soap::Parameters::none, emu::soap::Attributes().add( "Param", &attributeValue ) );
    }

    // Enable TF Cell operation
    if ( tf_descr_ != NULL && controlTFCellOp_.value_ ){
      sendCommandCell("start");
      waitForTFCellOpToReach("running",10);
    }

    if ( isUsingTCDS_ ){
      // Enable TCDS
      // PIs
      if ( pi_plus_  ) pi_plus_ ->enable( run_number_ );
      if ( pi_minus_ ) pi_minus_->enable( run_number_ );
      if ( pi_tf_    ) pi_tf_   ->enable( run_number_ );
      if ( pi_plus_  ) pi_plus_ ->waitForState( "Enabled", 30 );
      if ( pi_minus_ ) pi_minus_->waitForState( "Enabled", 30 );
      if ( pi_tf_    ) pi_tf_   ->waitForState( "Enabled", 30 );
      // CIs
      if ( ci_plus_  ) ci_plus_ ->enable( run_number_ );
      if ( ci_minus_ ) ci_minus_->enable( run_number_ );
      if ( ci_tf_    ) ci_tf_   ->enable( run_number_ );
      if ( ci_plus_  ) ci_plus_ ->enableSequence(); // This waits for the state transition to complete.
      if ( ci_minus_ ) ci_minus_->enableSequence(); // This waits for the state transition to complete.
      if ( ci_tf_    ) ci_tf_   ->enableSequence(); // This waits for the state transition to complete.
      // LPM
      if ( pm_       ) pm_      ->enable( run_number_ ).enableSequence(); // This waits for the state transition to complete.
    }

    refreshConfigParameters();
    
  } catch ( xoap::exception::Exception& e) {
    XCEPT_RETHROW( toolbox::fsm::exception::Exception, "Enable transition failed.", e );
  } catch ( xcept::Exception& e ) {
    XCEPT_RETHROW( toolbox::fsm::exception::Exception, "Enable transition failed.", e );
  } catch( std::exception& e ){
    XCEPT_RAISE( toolbox::fsm::exception::Exception, string( "Enabley transition failed: " ) + e.what() );
  }
  
  if (isCalibrationMode()) {
    quit_calibration_ = false;
    submit(calibration_signature_);
  }
  
  LOG4CPLUS_DEBUG(logger_, evt->type() << "(end)");
}

void emu::supervisor::Application::stopAction(toolbox::Event::Reference evt) 
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_DEBUG(logger_, evt->type() << "(begin)");
  
  emu::soap::Messenger m( this );

  try {
    emu::base::Stopwatch sw;
    sw.start();
    state_table_.refresh();
    cout << "Timing in stopAction: " << endl
	 << "    state table: " << state_table_
	 << "    state_table_.refresh: " << sw.read() << endl;

    // Stop TF Cell operation
    if ( tf_descr_ != NULL && controlTFCellOp_.value_ ){
      sendCommandCell("stop");
      waitForTFCellOpToReach("configured",60);
      cout << "    stop TFCellOp: " << sw.read() << endl;
    }

    if ( ! isUsingTCDS_ ){
      if (state_table_.getState("ttc::LTCControl", 0) != "halted") {
	m.sendCommand( "ttc::LTCControl", "reset" );
	cout << "    Halt (reset) ttc::LTCControl: " << sw.read() << endl;
      }
      if (state_table_.getState("ttc::TTCciControl", 0) != "halted") {
	m.sendCommand( "ttc::TTCciControl", "reset" );
	cout << "    Halt (reset) ttc::TTCciControl: " << sw.read() << endl;
      }
    }
        
    if ( isUsingTCDS_ ){
      // Stop TCDS
      // LPM
      if ( pm_       ) pm_      ->stop().stopSequence(); // This waits for the state transition to complete.
      // CIs
      if ( ci_plus_  ) ci_plus_ ->stop();
      if ( ci_minus_ ) ci_minus_->stop();
      if ( ci_tf_    ) ci_tf_   ->stop();
      if ( ci_plus_  ) ci_plus_ ->waitForState( "Configured", 30 );
      if ( ci_minus_ ) ci_minus_->waitForState( "Configured", 30 );
      if ( ci_tf_    ) ci_tf_   ->waitForState( "Configured", 30 );
      // PIs
      if ( pi_plus_  ) pi_plus_ ->stop();
      if ( pi_minus_ ) pi_minus_->stop();
      if ( pi_tf_    ) pi_tf_   ->stop();
      if ( pi_plus_  ) pi_plus_ ->waitForState( "Configured", 30 );
      if ( pi_minus_ ) pi_minus_->waitForState( "Configured", 30 );
      if ( pi_tf_    ) pi_tf_   ->waitForState( "Configured", 30 );
    }

    try {
      if ( isDAQManagerControlled("Halt") ) m.sendCommand( "emu::daq::manager::Application", 0, "Halt" );
      if ( isCommandFromWeb_ ) waitForDAQToExecute("Halt", 60, true);
      else                     waitForDAQToExecute("Halt", 3);
    } catch (xcept::Exception& ignored) {}
    cout << "    Halt emu::daq::manager::Application: " << sw.read() << endl;

    m.setResponseTimeout( 60 ); // Allow FED ample time to stop.
    m.sendCommand( "emu::fed::Manager", "Disable" );
    m.resetResponseTimeout(); // Reset response timeout to default value.
    cout << "    Disable emu::fed::Manager: " << sw.read() << endl;
    m.sendCommand( "emu::pc::EmuPeripheralCrateManager", "Disable" );
    cout << "    Disable emu::pc::EmuPeripheralCrateManager: " << sw.read() << endl;

    if ( ! isUsingTCDS_ ){
      m.sendCommand( "ttc::TTCciControl", "configure" );
      cout << "    Configure TTCci: " << sw.read() << endl;
      m.sendCommand( "ttc::LTCControl", "configure" );
      cout << "    Configure LTC: " << sw.read() << endl;
    }
    else{
      // The TF FED software will fail on starting if not having just been configured 
      // (i.e., when the previous run was 'stop'-ped) because the TF DDU L1A counter won't be 0. 
      // Let's run the stop sequence now (which will issue a hard reset and a resync) to zero that counter
      // to be ready to be started again.
      // The CSC FEDs don't seem to fail this way, but let's do them, too, to be absolutely sure.
      // Do this through the CIs as the PM is not under our control in global runs.
      if ( ci_plus_  ) ci_plus_ ->stopSequence();
      if ( ci_minus_ ) ci_minus_->stopSequence();
      if ( ci_tf_    ) ci_tf_   ->stopSequence();
    }

    writeRunInfo( isCommandFromWeb_ ); // only write runinfo if Stop was issued from the web interface
    if ( isCommandFromWeb_ ) cout << "    Write run info: " << sw.read() << endl;
  } catch ( xoap::exception::Exception& e) {
    XCEPT_RETHROW( toolbox::fsm::exception::Exception, "Stop transition failed.", e );
  } catch ( xcept::Exception& e ) {
    XCEPT_RETHROW( toolbox::fsm::exception::Exception, "Stop transition failed.", e );
  } catch( std::exception& e ){
    XCEPT_RAISE( toolbox::fsm::exception::Exception, string( "Stop transition failed: " ) + e.what() );
  }
  LOG4CPLUS_DEBUG(logger_, evt->type() << "(end)");
}

void emu::supervisor::Application::haltAction(toolbox::Event::Reference evt) 
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_DEBUG(logger_, evt->type() << "(begin)");
  
  emu::soap::Messenger m( this );

  try {
    emu::base::Stopwatch sw;
    sw.start();
    state_table_.refresh();
    cout << "Timing in haltAction: " << endl
	 << "    state table: " << state_table_
	 << "    state_table_.refresh: " << sw.read() << endl;
    
    // Reset TF Cell operation
    if ( tf_descr_ != NULL && controlTFCellOp_.value_ ){
      OpResetCell();
      cout << "    reset TFCellOp: " << sw.read() << endl;
    }

    if ( ! isUsingTCDS_ ){
      if (state_table_.getState("ttc::LTCControl", 0) != "halted") {
	m.sendCommand( "ttc::LTCControl", "reset" );
	cout << "    Halt (reset) ttc::LTCControl: " << sw.read() << endl;
      }

      if (state_table_.getState("ttc::TTCciControl", 0) != "halted") {
	m.sendCommand( "ttc::TTCciControl", "reset" );
	cout << "    Halt (reset) ttc::TTCciControl: " << sw.read() << endl;
      }
    }
    else{
      // Halt TCDS
      // LPM
      if ( pm_       ) pm_      ->halt().waitForState( "Halted", 30 );
      // CIs
      if ( ci_plus_  ) ci_plus_ ->halt();
      if ( ci_minus_ ) ci_minus_->halt();
      if ( ci_tf_    ) ci_tf_   ->halt();
      if ( ci_plus_  ) ci_plus_ ->waitForState( "Halted", 30 );
      if ( ci_minus_ ) ci_minus_->waitForState( "Halted", 30 );
      if ( ci_tf_    ) ci_tf_   ->waitForState( "Halted", 30 );
      if ( pi_plus_  ) pi_plus_ ->halt();
      if ( pi_minus_ ) pi_minus_->halt();
      if ( pi_tf_    ) pi_tf_   ->halt();
      // PIs
      if ( pi_plus_  ) pi_plus_ ->waitForState( "Halted", 30 );
      if ( pi_minus_ ) pi_minus_->waitForState( "Halted", 30 );
      if ( pi_tf_    ) pi_tf_   ->waitForState( "Halted", 30 );
    }

    m.sendCommand( "emu::fed::Manager", "Halt" );
    cout << "    Halt emu::fed::Manager: " << sw.read() << endl;

    m.sendCommand( "emu::pc::EmuPeripheralCrateManager", "Halt" );
    cout << "    Halt emu::pc::EmuPeripheralCrateManager: " << sw.read() << endl;
    
    try {
      if ( isDAQManagerControlled("Halt") ){
	m.sendCommand( "emu::daq::manager::Application", 0, "Halt" );
	if ( isCommandFromWeb_ ) waitForDAQToExecute("Halt", 60, true);
	else                     waitForDAQToExecute("Halt", 3);
      }
    } catch (xcept::Exception ignored) {}
    cout << "    Halt emu::daq::manager::Application: " << sw.read() << endl;

    // Issue a resync now to make sure L1A is reset to zero in the FEDs in case a global run follows.
    // In a global run, when backpressure is not ignored, this would fail (see http://cmsonline.cern.ch/cms-elog/756961).
    // By resynching through LTC, we make sure it's only done in local runs. 
    // The following command will do nothing if no ttc::LTCControl application is found.
    xdata::String attributeValue( "resync" );
    m.sendCommand( "ttc::LTCControl", "ExecuteSequence", emu::soap::Parameters::none, emu::soap::Attributes().add( "Param", &attributeValue ) );

    writeRunInfo( isCommandFromWeb_ ); // only write runinfo if Halt was issued from the web interface
    if ( isCommandFromWeb_ ) cout << "    Write run info: " << sw.read() << endl;

  } catch ( xoap::exception::Exception& e) {
    XCEPT_RETHROW( toolbox::fsm::exception::Exception, "Halt transition failed.", e );
  } catch ( xcept::Exception& e ) {
    XCEPT_RETHROW( toolbox::fsm::exception::Exception, "Halt transition failed.", e );
  } catch( std::exception& e ){
    XCEPT_RAISE( toolbox::fsm::exception::Exception, string( "Halt transition failed: " ) + e.what() );
  }
  
  LOG4CPLUS_DEBUG(logger_, evt->type() << "(end)");
}

void emu::supervisor::Application::resetAction() throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_DEBUG(logger_, "reset(begin)");
  
  fsm_.reset();
  state_ = fsm_.getStateName(fsm_.getCurrentState());
  
  LOG4CPLUS_DEBUG(logger_, "reset(end)");
}

void emu::supervisor::Application::setTTSAction(toolbox::Event::Reference evt) 
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_DEBUG(logger_, evt->type() << "(begin)");
  
  emu::soap::Messenger m( this );
  
  try {
    m.setParameters( "emu::fed::Manager", emu::soap::Parameters().add( "ttsID", &tts_id_ ).add( "ttsBits", &tts_bits_ ) );
    m.sendCommand( "emu::fed::Manager", 0, "SetTTSBits" );
  } catch (xoap::exception::Exception e) {
    XCEPT_RETHROW(toolbox::fsm::exception::Exception,
		  "SOAP fault was returned", e);
  } catch (xcept::Exception e) {
    XCEPT_RETHROW(toolbox::fsm::exception::Exception,
		  "Failed to send a command", e);
  }
  
  LOG4CPLUS_DEBUG(logger_, evt->type() << "(end)");
}

void emu::supervisor::Application::noAction(toolbox::Event::Reference evt) 
  throw (toolbox::fsm::exception::Exception)
{
  stringstream ss;
  ss << evt->type() 
     << " attempted when in " 
     << fsm_.getStateName(fsm_.getCurrentState())
     << " state. Command ignored.";
  LOG4CPLUS_WARN(logger_, ss.str());
  XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss.str() );
  this->notifyQualified( "warning", eObj );
}

void emu::supervisor::Application::submit(toolbox::task::ActionSignature *signature)
{
  wl_->submit(signature);
}

void emu::supervisor::Application::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
  throw (toolbox::fsm::exception::Exception)
{
  keep_refresh_ = false;
  
  LOG4CPLUS_DEBUG(getApplicationLogger(),"Current state is: [" << fsm.getStateName (fsm.getCurrentState()) << "]");
  // Send notification to Run Control
  state_=fsm.getStateName (fsm.getCurrentState());
  try
    {
      rcmsStateNotifier_.findRcmsStateListener();      	
      LOG4CPLUS_DEBUG(getApplicationLogger(),"Sending state changed notification to Run Control.");
      rcmsStateNotifier_.stateChanged((std::string)state_,"");
    }
  catch(xcept::Exception &e)
    {
      stringstream ss;
      ss << "Failed to notify state change to Run Control. ";
      LOG4CPLUS_ERROR(getApplicationLogger(), ss.str() << xcept::stdformat_exception_history(e));
      XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss.str(), e );
      this->notifyQualified( "error", eObj );
    }
  catch( std::exception& e )
    {
      stringstream ss;
      ss << "Failed to notify state change to Run Control.  std::exception caught: ";
      LOG4CPLUS_ERROR(getApplicationLogger(), ss.str() << e.what() );
      XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss.str() );
      this->notifyQualified( "error", eObj );
    }
  catch(...)
    {
      stringstream ss;
      ss << "Failed to notify state change to Run Control. Unknown exception caught.";
      LOG4CPLUS_ERROR(getApplicationLogger(), ss.str() );
      XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss.str() );
      this->notifyQualified( "error", eObj );
    }
  
  emu::base::Supervised::stateChanged(fsm);
}

void emu::supervisor::Application::transitionFailed(toolbox::Event::Reference event)
  throw (toolbox::fsm::exception::Exception)
{
  keep_refresh_ = false;
  toolbox::fsm::FailedEvent &failed = dynamic_cast<toolbox::fsm::FailedEvent&>(*event);
  
  stringstream reason;
  reason << "<![CDATA[" 
	 << endl
	 << "Failure occurred when performing transition"
	 << " from "        << failed.getFromState()
	 << " to "          << failed.getToState()
	 << ". Exception: " << xcept::stdformat_exception_history( failed.getException() )  
	 << endl
	 << "]]>";
  
  reasonForFailure_ = reason.str();
  LOG4CPLUS_ERROR(getApplicationLogger(), reason.str());

  // Send notification to Run Control
  try {
    rcmsStateNotifier_.findRcmsStateListener();      	
    LOG4CPLUS_DEBUG(getApplicationLogger(),"Sending state changed notification to Run Control.");
    rcmsStateNotifier_.stateChanged("Error",xcept::stdformat_exception_history(failed.getException()));
  } catch(xcept::Exception &e) {
    LOG4CPLUS_ERROR(getApplicationLogger(), "Failed to notify state change to Run Control : "
		    << xcept::stdformat_exception_history(e));
    stringstream ss3;
    ss3 << "Failed to notify state change to Run Control : ";
    XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss3.str(), e );
    this->notifyQualified( "error", eObj );
  }
  
}

//////////////////////////////////////////////////////////////////////


void emu::supervisor::Application::sendCommandCell(string command){
  if ( tf_descr_ == NULL ) return;
  emu::soap::Messenger m( this );

  xdata::String async( "false" ); 
  xdata::String cid( "10" );
  xdata::String sid( "73" ); 
  xdata::String paramName( "KEY" );
  xdata::String commandName( command );

  try{
    emu::soap::Attributes paramAttr;
    paramAttr.setUsePrefixOfParent( false ).add( "name", &paramName );
    m.sendCommand( tf_descr_, 
		   emu::soap::QualifiedName( "OpSendCommand", "urn:ts-soap:3.0", "ts-soap" ),
		   emu::soap::Parameters()
		   .add( "operation", &TFCellOpName_ )
		   .add( "command"  , &commandName   )
		   .add( "param"    , &tf_key_       , &paramAttr ),
		   emu::soap::Attributes()
		   .setUsePrefixOfParent( false )
		   .add( "async", &async )
		   .add( "cid"  , &cid   )
		   .add( "sid"  , &sid   )
		   );
  } 
  catch( xcept::Exception& e ){
    LOG4CPLUS_ERROR( getApplicationLogger(), "Failed to send command '" << command << "' to TF Cell " << xcept::stdformat_exception_history(e) );
  }
}
 

std::string emu::supervisor::Application::OpGetStateCell(){
  if ( tf_descr_ == NULL ) return "";

  emu::soap::Messenger m( this );

  xdata::String async( "false" ); 
  xdata::String cid( "10" );
  xdata::String sid( "73" ); 

  try{
    xdata::String state;
    emu::soap::extractParameters( m.sendCommand( tf_descr_, 
						 emu::soap::QualifiedName( "OpGetState", "urn:ts-soap:3.0", "ts-soap" ),
						 emu::soap::Parameters()
						 .add( "operation", &TFCellOpName_ ),
						 emu::soap::Attributes()
						 .setUsePrefixOfParent( false )
						 .add( "async", &async )
						 .add( "cid"  , &cid   )
						 .add( "sid"  , &sid   )
						 ),
				  emu::soap::Parameters().add( emu::soap::QualifiedName( "payload", "urn:ts-soap:3.0" ), &state )
				  );
    return ( state == "" ? "UNKNOWN" : state );
  } 
  catch( xcept::Exception& e ){
    LOG4CPLUS_ERROR( getApplicationLogger(), "Failed to get state TF Cell Operation " << xcept::stdformat_exception_history(e) );
  }
  return "UNKNOWN";
}


void emu::supervisor::Application::OpResetCell(){
  if ( tf_descr_ == NULL ) return;

  emu::soap::Messenger m( this );

  xdata::String async( "false" ); 
  xdata::String cid( "10" );
  xdata::String sid( "73" );

  try{
    m.sendCommand( tf_descr_, 
		   emu::soap::QualifiedName( "OpReset", "urn:ts-soap:3.0", "ts-soap" ),
		   emu::soap::Parameters()
		   .add( "operation", &TFCellOpName_ ),
		   emu::soap::Attributes()
		   .setUsePrefixOfParent( false )
		   .add( "async", &async )
		   .add( "cid"  , &cid   )
		   .add( "sid"  , &sid   )
		   );
  } 
  catch( xcept::Exception& e ){
    LOG4CPLUS_ERROR( getApplicationLogger(), "Failed to reset TF Cell Operation " << xcept::stdformat_exception_history(e) );
  }
}

//////////////////////////////////////////////////////////////////////


bool emu::supervisor::Application::waitForTFCellOpToReach( const string targetState, const unsigned int seconds ){
  if ( tf_descr_ == NULL ) return false;

  // Poll, and return TRUE if and only if TF Cell Operation gets into the expected state before timeout.
  for ( unsigned int i=0; i<=seconds; ++i ){
    TFCellOpState_ = OpGetStateCell();
    if ( TFCellOpState_.toString() == targetState ){ return true; }
    LOG4CPLUS_INFO( logger_, "Waited " << i << " sec so far for TF Cell Operation " 
		    << TFCellOpName_.toString() << " to get " << targetState 
		    << ". It is still in " << TFCellOpState_.toString() << " state." );
    ::sleep(1);
  }

  LOG4CPLUS_ERROR( logger_, "Timeout after waiting " << seconds << " sec so far for TF Cell Operation " 
		   << TFCellOpName_.toString() << " to get " << targetState 
		   << ". It is still in " << TFCellOpState_.toString() << " state." );

  stringstream ss10;
  ss10 <<  "Timeout after waiting " << seconds << " sec so far for TF Cell Operation "
       << TFCellOpName_.toString() << " to get "<< targetState 
       << ". It is still in " << TFCellOpState_.toString() << " state.";
  XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss10.str() );
  this->notifyQualified( "error", eObj );
  return false;
}


void emu::supervisor::Application::refreshConfigParameters()
{
	daq_mode_ = getDAQMode();
	TFCellOpState_ = OpGetStateCell();
	ttc_source_ = getTTCciSource();
}

string emu::supervisor::Application::getCGIParameter(xgi::Input *in, string name)
{
	cgicc::Cgicc cgi(in);
	string value;

	form_iterator i = cgi.getElement(name);
	if (i != cgi.getElements().end()) {
		value = (*i).getValue();
	}

	return value;
}

bool emu::supervisor::Application::allCalibrationRuns()
{
  if ( isUsingTCDS_ ){
    for (size_t i = 0; i < runParameters_.size(); ++i) if ( runParameters_[i].bag.key_.toString().substr( 0, 5 ) != "Calib" ) return false;
  }
  else{
    for (size_t i = 0; i <  calib_params_.size(); ++i) if (  calib_params_[i].bag.key_.toString().substr( 0, 5 ) != "Calib" ) return false;
  }
  return true;
}

bool emu::supervisor::Application::isCalibrationMode()
{
  return ( run_type_.toString().substr( 0, 5 ) == "Calib" );
}

bool emu::supervisor::Application::isAlctCalibrationMode()
{
	// std::cout << "isAlctCalibMode: runtype: " << run_type_.toString() << "index" << keyToIndex(run_type_);
	bool res = run_type_.toString().find("ALCT") != string::npos;
	std::cout << "isAlctCalibMode result: " << res << std::endl;
	return res;
}

int emu::supervisor::Application::getCalibParamIndex(const string name)
{
	int result = -1;

	for (size_t i = 0; i < calib_params_.size(); ++i) {
		if (calib_params_[i].bag.key_ == name) {
			result = i;
			break;
		}
	}

	return result;
}

int emu::supervisor::Application::keyToIndex(const string name)
{
	int result = -1;

	for (size_t i = 0; i < runParameters_.size(); ++i) {
		if (runParameters_[i].bag.key_ == name) {
			result = i;
			break;
		}
	}

	return result;
}


string emu::supervisor::Application::getDAQMode(){
  string result("");

  emu::soap::Messenger m( this );

  xdata::Boolean daqMode( false );
  try{
    m.getParameters( "emu::daq::manager::Application", 0, emu::soap::Parameters().add( "supervisedMode", &daqMode ) );
    result = ( bool( daqMode ) ? "supervised" : "unsupervised" );
    REVOKE_ALARM( "noLocalDAQ", NULL );
  } catch (xcept::Exception e) {
    LOG4CPLUS_INFO(logger_, "Failed to get local DAQ state. "
		   << xcept::stdformat_exception_history(e));
    RAISE_ALARM( emu::supervisor::alarm::NoLocalDAQ, "noLocalDAQ", "warn", "Local DAQ is in down or inaccessible.", "", &logger_ );
    result = "UNKNOWN";
  }

  return result;
}

string emu::supervisor::Application::getLocalDAQState(){

  emu::soap::Messenger m( this );

  xdata::String daqState( "UNKNOWN" );
  try{
    m.getParameters( "emu::daq::manager::Application", 0, emu::soap::Parameters().add( "daqState", &daqState ) );
    REVOKE_ALARM( "noLocalDAQ", NULL );
  } catch (xcept::Exception e) {
    LOG4CPLUS_INFO(logger_, "Failed to get local DAQ state. "
		   << xcept::stdformat_exception_history(e));
    RAISE_ALARM( emu::supervisor::alarm::NoLocalDAQ, "noLocalDAQ", "warn", "Local DAQ is in down or inaccessible.", "", &logger_ );
  }

  return daqState.toString();
}

string emu::supervisor::Application::getTTCciSource(){
  string result( "" );
  if ( ttc_descr_ == NULL ) return result;

  emu::soap::Messenger m( this );

  xdata::String ClockSource   = "UNKNOWN";
  xdata::String OrbitSource   = "UNKNOWN";
  xdata::String TriggerSource = "UNKNOWN";
  xdata::String BGOSource     = "UNKNOWN";

  try{
    m.getParameters( ttc_descr_,
		     emu::soap::Parameters()
		     .add( "ClockSource"  , &ClockSource   ) 
		     .add( "OrbitSource"  , &OrbitSource   ) 
		     .add( "TriggerSource", &TriggerSource ) 
		     .add( "BGOSource"    , &BGOSource     ) );
    result  =       ClockSource.toString();
    result += ":" + OrbitSource.toString();
    result += ":" + TriggerSource.toString();
    result += ":" + BGOSource.toString();
  }
  catch(xcept::Exception e){
    result = "UNKNOWN";
  }

  return result;
}


bool emu::supervisor::Application::waitForDAQToExecute( const string command, const unsigned int seconds, const bool poll ){
  string expectedState;
  if      ( command == "Configure" ){ expectedState = "Ready";   }
  else if ( command == "Enable"    ){ expectedState = "Enabled"; }
  else if ( command == "Halt"      ){ expectedState = "Halted";  }
  else                              { return true; }

  // If not polling (default), just wait and return TRUE:
  if ( !poll ){
    ::sleep( seconds );
    return true;
  }

  // Poll, and return TRUE if and only if DAQ gets into the expected state before timeout.
  emu::soap::Messenger m( this );
  xdata::String  daqState;
  for ( unsigned int i=0; i<=seconds; ++i ){
    m.getParameters( "emu::daq::manager::Application", 0, emu::soap::Parameters().add( "daqState", &daqState ) );
    if ( daqState.toString() != "Halted"  && daqState.toString() != "Ready" && 
	 daqState.toString() != "Enabled" && daqState.toString() != "INDEFINITE" ){
      LOG4CPLUS_ERROR( logger_, "Local DAQ is in " << daqState.toString() << " state. Please destroy and recreate local DAQ." );
      stringstream ss9;
      ss9 <<  "Local DAQ is in " << daqState.toString() << " state. Please destroy and recreate local DAQ." ;
      XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss9.str() );
      this->notifyQualified( "error", eObj );
      return false;
    }
    if ( daqState.toString() == expectedState ){ return true; }
    LOG4CPLUS_INFO( logger_, "Waited " << i << " sec so far for local DAQ to get " 
		    << expectedState << ". It is still in " << daqState.toString() << " state." );
    ::sleep(1);
  }

  LOG4CPLUS_ERROR( logger_, "Timeout after waiting " << seconds << " sec for local DAQ to get " << expectedState 
		   << ". It is in " << daqState.toString() << " state." );
  stringstream ss10;
  ss10 <<  "Timeout after waiting " << seconds << " sec for local DAQ to get " << expectedState 
		   << ". It is in " << daqState.toString() << " state." ;
  XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss10.str() );
  this->notifyQualified( "error", eObj );
  return false;
}

bool emu::supervisor::Application::isDAQManagerControlled(string command)
{
  emu::soap::Messenger m( this );
  xdata::Boolean supervisedMode;
  // xdata::Boolean configuredInSupervisedMode;
  xdata::String  daqState;
  try {
    m.getParameters( "emu::daq::manager::Application", 0,
		     emu::soap::Parameters()
		     .add( "supervisedMode"            , &supervisedMode             )
		     // .add( "configuredInSupervisedMode", &configuredInSupervisedMode )
		     .add( "daqState"                  , &daqState                   ) );
  }
  catch (xcept::Exception &ignored){
    return false;
  }

  // No point in sending any command when DAQ is in an irregular state (failed, indefinite, ...)
  if ( daqState.toString() != "Halted"  && daqState.toString() != "Ready" && 
       daqState.toString() != "Enabled" && daqState.toString() != "INDEFINITE" ){
    LOG4CPLUS_WARN( logger_, "No command \"" << command << "\" sent to emu::daq::manager::Application because local DAQ is in " 
		    << daqState.toString() << " state. Please destroy and recreate local DAQ." );
    stringstream ss11;
    ss11 <<  "No command \"" << command << "\" sent to emu::daq::manager::Application because local DAQ is in " 
	 << daqState.toString() << " state. Please destroy and recreate local DAQ." ;
    XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss11.str() );
    this->notifyQualified( "warn", eObj );
    return false;
  }

  // Don't send any other command when DAQ is in unsupervised mode.
  if ( ! bool( supervisedMode ) ) { return false; }
  
  // And don't send any other command when DAQ was configured in unsupervised mode, either.
  // if ( command != "Configure" && !bool( configuredInSupervisedMode ) ) { return false; }

  return true;

}

bool emu::supervisor::Application::waitForAppsToReach( const string targetState, bool includingSupervisor, const int seconds ){
  // If seconds is negative, no timeout.
  for ( int i=0; i<=seconds || seconds<0; ++i ){
    state_table_.refresh( false ); // Do not force refresh, we're not in a hurry. We'll refresh soon anyway.
    // Both the supervised apps and the Supervisor app itself should be in the target state if includingSupervisor is true:
    if ( state_table_.isValidState( targetState )
	 &&
	 ( !includingSupervisor || fsm_.getStateName(fsm_.getCurrentState()) == targetState ) ) return true;
    LOG4CPLUS_DEBUG( logger_, "Waited " << i << " sec so far for applications to get '" << targetState
		    << "'. Their current states are:" << state_table_ );
    if ( fsm_.getCurrentState() == 'F' ) return false; // Abort if in Failed state.
    ::sleep(1);
  }
  LOG4CPLUS_ERROR( logger_, "Timeout after waiting " << seconds << " sec for applications to get " << targetState );
  stringstream ss;
  ss <<  "Timeout after waiting " << seconds << " sec for applications to get " << targetState;
  XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss.str() );
  this->notifyQualified( "error", eObj );
  return false;
}

void emu::supervisor::Application::onException( xcept::Exception& e ){ // callback for toolbox::exception::Listener
  LOG4CPLUS_ERROR( logger_, "Exception caught in TCDS hardware lease renewal thread: " << xcept::stdformat_exception_history(e) );
}

emu::supervisor::Application::StateTable::StateTable(emu::supervisor::Application *sv) 
  : app_(sv)
  , bSem_( toolbox::BSem::FULL )
  , lastRefreshTime_(0){}

void emu::supervisor::Application::StateTable::addApplication(string klass)
{

	// find applications
	std::set<xdaq::ApplicationDescriptor *> apps;
	try {
		apps = app_->getApplicationContext()->getDefaultZone()
				->getApplicationDescriptors(klass);
	} catch (xdaq::exception::ApplicationDescriptorNotFound e) {
                bSem_.give();
		return; // Do nothing if the target doesn't exist
	}

	// add to the table
        bSem_.take();
	std::set<xdaq::ApplicationDescriptor *>::iterator i = apps.begin();
	for (; i != apps.end(); ++i) {
		table_.push_back(
				pair<xdaq::ApplicationDescriptor *, string>(*i, "NULL"));
	}
        bSem_.give();
}

void emu::supervisor::Application::StateTable::refresh( bool forceRefresh )
{
        // Limit refresh rate to 1/2 Hz if forceRefresh is false. (forceRefresh is true by default).
        time_t timeNow;
	time( &timeNow );
	if ( timeNow < lastRefreshTime_ + 2 && !forceRefresh ) return;

	string klass = "";
	int instance = -1;
	string service = "";

	emu::soap::Messenger m( app_ );

	vector<pair<xdaq::ApplicationDescriptor *, string> >::iterator i =
			table_.begin();
	for (; i != table_.end(); ++i) {

	        klass    = i->first->getClassName();
		instance = i->first->getInstance();
		service  = i->first->getAttribute( "service" );

		try {
			xdata::String state;
			if ( klass == "tcds::pi::PIController" ){
			  if ( service == "pi-cscp"  && app_->pi_plus_   ) state = app_->pi_plus_ ->getSteadyState();
			  if ( service == "pi-cscm"  && app_->pi_minus_  ) state = app_->pi_minus_->getSteadyState();
			  if ( service == "pi-csctf" && app_->pi_tf_     ) state = app_->pi_tf_   ->getSteadyState();
			}
			else if ( klass == "tcds::ici::ICIController" ){
			  if ( service == "ici-cscp"  && app_->ci_plus_  ) state = app_->ci_plus_ ->getSteadyState();
			  if ( service == "ici-cscm"  && app_->ci_minus_ ) state = app_->ci_minus_->getSteadyState();
			  if ( service == "ici-csctf" && app_->ci_tf_    ) state = app_->ci_tf_   ->getSteadyState();
			}
			else if ( klass == "tcds::lpm::LPMController" ){
			  if ( service == "lpm-csc"   && app_->pm_       ) state = app_->pm_      ->getSteadyState();
			}
			else{
			  m.getParameters( i->first, emu::soap::Parameters().add( "stateName", &state ) );
			}
                        bSem_.take();
			i->second = state.toString();
			lastRefreshTime_ = timeNow;
                        bSem_.give();
		} catch (xcept::Exception &e) {
			i->second = STATE_UNKNOWN;
			LOG4CPLUS_ERROR(app_->logger_, "Exception when trying to get state of "
					<< klass << ": " << xcept::stdformat_exception_history(e));
			stringstream ss12;
			ss12 << "Exception when trying to get state of "
			     << klass << ": " ;
			XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss12.str(), e );
			app_->notifyQualified( "error", eObj );
                        bSem_.give();
		} catch (...) {
			LOG4CPLUS_ERROR(app_->logger_, "Unknown exception when trying to get state of " << klass);
			stringstream ss13;
			ss13 << "Unknown exception when trying to get state of " << klass;
			XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss13.str() );
			app_->notifyQualified( "error", eObj );
			i->second = STATE_UNKNOWN;
                        bSem_.give();
		}

		if (klass == "emu::daq::manager::Application" && i->second == STATE_UNKNOWN) {
			LOG4CPLUS_WARN(app_->logger_, "State of emu::daq::manager::Application will be unknown.");
			stringstream ss14;
			ss14 << "State of emu::daq::manager::Application will be unknown.";
			XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss14.str() );
			app_->notifyQualified( "warn", eObj );
		}
	}

}

string emu::supervisor::Application::StateTable::getState(string klass, unsigned int instance) const
{
	string state = "";

	vector<pair<xdaq::ApplicationDescriptor *, string> >::const_iterator i =
			table_.begin();
	for (; i != table_.end(); ++i) {
		if (klass == i->first->getClassName()
				&& instance == i->first->getInstance()) {
			state = i->second;
			break;
		}
	}

	return state;
}

bool emu::supervisor::Application::StateTable::isValidState(string expected) const
{
	bool is_valid = true;

	vector<pair<xdaq::ApplicationDescriptor *, string> >::const_iterator i =
			table_.begin();
	for (; i != table_.end(); ++i) {
		string checked = expected;
		string klass = i->first->getClassName();

		// Ignore emu::daq::manager::Application in global runs.
		if ( klass == "emu::daq::manager::Application" 
		     && app_->run_type_ == "Global" ) continue;

		// TTC/LTC have their own peculiar state names. Translate them:
		if (klass == "ttc::TTCciControl" || klass == "ttc::LTCControl") {
			if (expected == "Halted"    ) { checked = "halted";     }
			if (expected == "Configured") { checked = "configured"; }
			if (expected == "Enabled"   ) { checked = "enabled";    }
		}

		if (i->second != checked){
		  is_valid = false;
		  break;
		}

	}

	return is_valid;
}

void emu::supervisor::Application::StateTable::webOutput(xgi::Output *out, string sv_state)
		throw (xgi::exception::Exception)
{
	refresh();
	*out << table() << endl;

	// My state
	*out << tr();
	*out << td() << "Application" << "(" << "0" << ")" << td();
	*out << td().set("class", sv_state) << sv_state << td();
	*out << tr() << endl;

	// Applications
	vector<pair<xdaq::ApplicationDescriptor *, string> >::iterator i =
			table_.begin();
	for (; i != table_.end(); ++i) {
		string klass = i->first->getClassName();
		int instance = i->first->getInstance();
		string state = i->second;
		string service( i->first->getAttribute( "service" ) );

		*out << tr();
		*out << td() << klass << " (" << instance << ",'" << service << "')" << td();
		*out << td().set("class", state) << state << td();
		*out << tr() << endl;
	}

	*out << table() << endl;
}

ostream& emu::supervisor::operator<<( ostream& os, const emu::supervisor::Application::StateTable& st ){
  os << endl << "emu::supervisor::Application(0) " << st.getApplication()->getFSM()->getCurrentState() << endl;
  for (vector<pair<xdaq::ApplicationDescriptor *, string> >::const_iterator i = st.getTable()->begin(); i != st.getTable()->end(); ++i) {
    os << i->first->getClassName() << " (" << i->first->getInstance() << ",'" << i->first->getAttribute( "service" ) << "') " << i->second << endl;
  }
  return os;
}

void emu::supervisor::Application::LastLog::size(unsigned int size)
{
	size_ = size;
}

unsigned int emu::supervisor::Application::LastLog::size() const
{
	return size_;
}

void emu::supervisor::Application::LastLog::add(string message)
{
	messages_.push_back(getTime() + " " + message);

	if (messages_.size() > size_) { messages_.pop_front(); }
}

string emu::supervisor::Application::LastLog::getTime() const
{
	timeval tv;
	gettimeofday(&tv, 0);

	ostringstream line;
	char s[100];

	strftime(s, 100, "%Y-%m-%dT%H:%M:%S.", localtime(&tv.tv_sec));
	line << s;
	line << tv.tv_usec / 1000;
	strftime(s, 100, "%z", localtime(&tv.tv_sec));
	line << s;

	return line.str();
}

void emu::supervisor::Application::LastLog::webOutput(xgi::Output *out)
		throw (xgi::exception::Exception)
{
	*out << "Last " << messages_.size() << " log messages:" << br() << endl;
	*out << textarea().set("cols", "120").set("rows", "20")
			.set("readonly").set("class", "log") << endl;

	deque<string>::iterator i = messages_.begin();
	for (; i != messages_.end(); ++i) {
		*out << *i << endl;
	}

	*out << textarea() << endl;
}


void emu::supervisor::Application::bookRunNumber(){

  isBookedRunNumber_ = false;

  // Don't book debug runs:
  if ( run_type_.toString() == "Debug" ) return;
  
  // Just in case it's left over from the previuos run:
  if ( runInfo_ ) {
    delete runInfo_; 
    runInfo_ = 0;
  }

  try
    {
      runInfo_ = emu::supervisor::RunInfo::Instance( runDbBookingCommand_.toString(),
					     runDbWritingCommand_.toString(),
					     runDbUserFile_.toString(),
					     runDbAddress_.toString() );
    }
  catch( string e )
    {
      LOG4CPLUS_ERROR(logger_, e);
      stringstream ss20;
      ss20 <<  e;
      XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss20.str() );
      this->notifyQualified( "error", eObj );
    }

  if ( runInfo_ ){

    const string sequence = "CMS.CSC";
    
    LOG4CPLUS_INFO(logger_, "Booking run number with " <<
		   runDbBookingCommand_.toString() << " at " <<
		   runDbAddress_.toString()  << " for " << sequence );
    
    bool success = runInfo_->bookRunNumber( sequence );
    
    if ( success ){
      isBookedRunNumber_ = true;
      run_number_        = runInfo_->runNumber();
      runSequenceNumber_ = runInfo_->runSequenceNumber();
      LOG4CPLUS_INFO(logger_, "Booked run rumber " << run_number_.toString() <<
		     " (" << sequence << " " << runSequenceNumber_.toString() << ")");
    }
    else{
      LOG4CPLUS_ERROR(logger_,
		      "Failed to book run number: " 
		      <<  runInfo_->errorMessage()
		      << " ==> Falling back to run number " << run_number_.toString() 
		      << " specified by user." );
      stringstream ss21;
      ss21 << 
		      "Failed to book run number: " 
		      <<  runInfo_->errorMessage()
		      << " ==> Falling back to run number " << run_number_.toString() 
		      << " specified by user." ;
      XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss21.str() );
      this->notifyQualified( "error", eObj );
    }
  } // if ( runInfo_ ){

}

void emu::supervisor::Application::writeRunInfo( bool toDatabase ){
  // Update run info db

  if ( !toDatabase ) return;

  // Don't write about debug runs:
  if ( run_type_.toString() == "Debug" ) return;

  // If DAQ Manager is absent, we have nothing to do here:
  if ( ! isDAQManagerControlled("Halt") ) return;

  // emu::daq::manager::Application's FSM is asynchronous. Wait for it.
  if ( ! waitForDAQToExecute("Halt", 10, true ) ){
    LOG4CPLUS_WARN(logger_, "Nothing written to run database as local DAQ has not stopped.");
    stringstream ss22;
    ss22 <<  "Nothing written to run database as local DAQ has not stopped.";
    XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss22.str() );
    this->notifyQualified( "warn", eObj );
    return;
  }

  // If it's not a debug run, it should normally have been booked. If not, inform the user that it somehow wasn't.
  if ( toDatabase && !isBookedRunNumber_ ){
    LOG4CPLUS_WARN(logger_, "Nothing written to run database as no run number was booked.");
    stringstream ss23;
    ss23 <<  "Nothing written to run database as no run number was booked.";
    XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss23.str() );
    this->notifyQualified( "warn", eObj );
  }

    bool success = false;
    const string nameSpace = "CMS.CSC";
    string name, value;

    //
    // Deserialize reply to run summary query
    //

    emu::soap::Messenger m( this );

    // Start and end times
    xdata::String start_time( "UNKNOWN" );
    xdata::String stop_time( "UNKNOWN" );
    // FU event count
    xdata::String built_events( "0" );
    // RUI event counts and instances
    xdata::Vector<xdata::String> rui_counts;
    xdata::Vector<xdata::String> rui_instances;

    try{
      emu::soap::extractParameters( m.sendCommand( "emu::daq::manager::Application", 0, "QueryRunSummary" ),
				    emu::soap::Parameters()                  
				    .add( "start_time"   , &start_time     ) 
				    .add( "stop_time"    , &stop_time      ) 
				    .add( "built_events" , &built_events   ) 
				    .add( "rui_counts"   , &rui_counts     ) 
				    .add( "rui_instances", &rui_instances  ) );
    }
    catch( xcept::Exception e ){
      LOG4CPLUS_WARN( logger_, "Run summary unknown: " << xcept::stdformat_exception_history(e) );
      stringstream ss25;
      ss25 <<  "Run summary unknown: "  ;
      XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss25.str(), e );
      this->notifyQualified( "warn", eObj );
    }
    
    //
    // run type
    //
    name      = "run_type";
    value     = run_type_.toString();
    if ( toDatabase && isBookedRunNumber_ ){
      success = runInfo_->writeRunInfo( name, value, nameSpace );
      if ( success ){
	LOG4CPLUS_INFO(logger_, "Wrote to run database: " << 
		       nameSpace << ":" << name << " = " << value );
      }
      else{
	LOG4CPLUS_ERROR(logger_,
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString() <<
			" : " << runInfo_->errorMessage() );
	stringstream ss26;
	ss26 << 
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString() <<
			" : " << runInfo_->errorMessage() ;
	XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss26.str() );
	this->notifyQualified( "error", eObj );
      }
    }

    //
    // start time and stop time
    //
    name      = "start_time";
    value     = start_time.toString();
    if ( toDatabase && isBookedRunNumber_ ){
      success = runInfo_->writeRunInfo( name, value, nameSpace );
      if ( success ){
	LOG4CPLUS_INFO(logger_, "Wrote to run database: " << 
				     nameSpace << ":" << name << " = " << value );
      }
      else{
	LOG4CPLUS_ERROR(logger_,
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString()  <<
			" : " << runInfo_->errorMessage() );
	stringstream ss27;
	ss27 << 
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString()  <<
			" : " << runInfo_->errorMessage() ;
	XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss27.str() );
	this->notifyQualified( "error", eObj );
      }
    }
    name      = "stop_time";
    value     = stop_time.toString();
    if ( toDatabase && isBookedRunNumber_ ){
      success = runInfo_->writeRunInfo( name, value, nameSpace );
      if ( success ){
	LOG4CPLUS_INFO(logger_, "Wrote to run database: " << 
		       nameSpace << ":" << name << " = " << value );
      }
      else{
	LOG4CPLUS_ERROR(logger_,
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString()  <<
			" : " << runInfo_->errorMessage() );
	stringstream ss28;
	ss28 << 
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString()  <<
			" : " << runInfo_->errorMessage() ;
	XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss28.str() );
	this->notifyQualified( "error", eObj );
      }
    }

    //
    // trigger sources
    //
    xdata::String ClockSource   = "UNKNOWN";
    xdata::String OrbitSource   = "UNKNOWN";
    xdata::String TriggerSource = "UNKNOWN";
    xdata::String BGOSource     = "UNKNOWN";
    try{
      m.getParameters( "ttc::TTCciControl", 2,
		       emu::soap::Parameters()
		       .add( "ClockSource"  , &ClockSource   ) 
		       .add( "OrbitSource"  , &OrbitSource   ) 
		       .add( "TriggerSource", &TriggerSource ) 
		       .add( "BGOSource"    , &BGOSource     ) );
    }
    catch(xcept::Exception e){
      LOG4CPLUS_ERROR(logger_,"Failed to get trigger sources from ttc::TTCciControl: " << 
		      xcept::stdformat_exception_history(e) );
      stringstream ss30;
      ss30 << "Failed to get trigger sources from ttc::TTCciControl: ";
      XCEPT_DECLARE_NESTED( emu::supervisor::exception::Exception, eObj, ss30.str(), e );
      this->notifyQualified( "error", eObj );
    }
    name  = "clock_source";
    value = ClockSource.toString();
    if ( toDatabase && isBookedRunNumber_ ){
      success = runInfo_->writeRunInfo( name, value, nameSpace );
      if ( success ){
	LOG4CPLUS_INFO(logger_, "Wrote to run database: " << 
				     nameSpace << ":" << name << " = " << value );
      }
      else{
	LOG4CPLUS_ERROR(logger_,
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString() <<
			" : " << runInfo_->errorMessage() );
	stringstream ss31;
	ss31 << 
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString() <<
			" : " << runInfo_->errorMessage() ;
	XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss31.str() );
	this->notifyQualified( "error", eObj );
      }
    }
    name  = "orbit_source";
    value = OrbitSource.toString();
    if ( toDatabase && isBookedRunNumber_ ){
      success = runInfo_->writeRunInfo( name, value, nameSpace );
      if ( success ){
	LOG4CPLUS_INFO(logger_, "Wrote to run database: " << 
				     nameSpace << ":" << name << " = " << value );
      }
      else{
	LOG4CPLUS_ERROR(logger_,
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString() <<
			" : " << runInfo_->errorMessage() );
	stringstream ss32;
	ss32 << 
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString() <<
			" : " << runInfo_->errorMessage() ;
	XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss32.str() );
	this->notifyQualified( "error", eObj );
      }
    }
    name  = "trigger_source";
    value = TriggerSource.toString();
    if ( toDatabase && isBookedRunNumber_ ){
      success = runInfo_->writeRunInfo( name, value, nameSpace );
      if ( success ){
	LOG4CPLUS_INFO(logger_, "Wrote to run database: " << 
		       nameSpace << ":" << name << " = " << value );
      }
      else{
	LOG4CPLUS_ERROR(logger_,
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString() );
	stringstream ss33;
	ss33 << 
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString() ;
	XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss33.str() );
	this->notifyQualified( "error", eObj );
      }
    }
    name  = "BGO_source";
    value = BGOSource.toString();
    if ( toDatabase && isBookedRunNumber_ ){
      success = runInfo_->writeRunInfo( name, value, nameSpace );
      if ( success ){
	LOG4CPLUS_INFO(logger_, "Wrote to run database: " << 
		       nameSpace << ":" << name << " = " << value );
      }
      else{
	LOG4CPLUS_ERROR(logger_,
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString() <<
			" : " << runInfo_->errorMessage() );
	stringstream ss34;
	ss34 << 
			"Failed to write " << nameSpace << ":" << name << 
			" to run database " << runDbAddress_.toString() <<
			" : " << runInfo_->errorMessage() ;
	XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss34.str() );
	this->notifyQualified( "error", eObj );
      }
    }

    //
    // EmuFU event count
    //
      name  = "built_events";
      value = built_events.toString();
      if ( toDatabase && isBookedRunNumber_ ){
	success = runInfo_->writeRunInfo( name, value, nameSpace );
	if ( success ){
	  LOG4CPLUS_INFO(logger_, "Wrote to run database: " << 
			 nameSpace << ":" << name << " = " << value );
	}
	else{
	  LOG4CPLUS_ERROR(logger_,
			  "Failed to write " << nameSpace << ":" << name << 
			  " to run database " << runDbAddress_.toString() <<
			  " : " << runInfo_->errorMessage() );
	  stringstream ss35;
	  ss35 << 
			  "Failed to write " << nameSpace << ":" << name << 
			  " to run database " << runDbAddress_.toString() <<
			  " : " << runInfo_->errorMessage() ;
	  XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss35.str() );
	  this->notifyQualified( "error", eObj );
	}
      }

    //
    // EmuRUI event counts
    //
    for ( unsigned int i = 0; i < rui_counts.elements(); ++i ){
      name  = "EmuRUI" + (dynamic_cast<xdata::String*>(rui_instances.elementAt(i)))->toString();
      value = (dynamic_cast<xdata::String*>(rui_counts.elementAt(i)))->toString();
      if ( toDatabase && isBookedRunNumber_ ){
	success = runInfo_->writeRunInfo( name, value, nameSpace );
	if ( success ){
	  LOG4CPLUS_INFO(logger_, "Wrote to run database: " << 
				       nameSpace << ":" << name << " = " << value );
	}
	else{
	  LOG4CPLUS_ERROR(logger_,
			  "Failed to write " << nameSpace << ":" << name << 
			  " to run database " << runDbAddress_.toString() <<
			  " : " << runInfo_->errorMessage() );
	  stringstream ss36;
	  ss36 << 
			  "Failed to write " << nameSpace << ":" << name << 
			  " to run database " << runDbAddress_.toString() <<
			  " : " << runInfo_->errorMessage() ;
	  XCEPT_DECLARE( emu::supervisor::exception::Exception, eObj, ss36.str() );
	  this->notifyQualified( "error", eObj );
	}
      }
    }
}

string emu::supervisor::Application::withoutString( const string& toRemove, const string& str ){
  // Remove all occurences of a string from str.
  string s( str );
  size_t pos = 0;
  while( ( pos = s.find( toRemove, pos ) ) != string::npos ) s.erase( pos, toRemove.length() );
  return s;
}

// End of file
// vim: set sw=4 ts=4:
// End of file
// vim: set sw=4 ts=4:
