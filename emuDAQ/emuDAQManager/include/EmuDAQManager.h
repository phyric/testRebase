#ifndef __EmuDAQManager_h__
#define __EmuDAQManager_h__

#include "emuDAQManager/exception/Exception.h"
#include "i2o/i2oDdmLib.h"
#include "i2o/utils/AddressMap.h"
#include "toolbox/mem/MemoryPoolFactory.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/WebApplication.h"
#include "xdata/Boolean.h"
#include "xdata/InfoSpace.h"
#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/Integer.h"
#include "xdata/Vector.h"
#include "emu/cscSV/include/EmuRunInfo.h"
#include "emu/cscSV/include/EmuELog.h"
#include "emu/cscSV/include/EmuApplication.h"
#include "xdata/ItemEvent.h"

#include "toolbox/task/WorkLoop.h"
#include "toolbox/BSem.h"

#include <string>


using namespace std;


/**
 * Tests the RU builder applications.
 */
class EmuDAQManager :
// public xdaq::WebApplication,
public EmuApplication,
public xdata::ActionListener
{

public:

    /**
     * Define factory method for the instantion of EmuDAQManager applications.
     */
    XDAQ_INSTANTIATOR();

    /**
     * Constructor.
     */
    EmuDAQManager(xdaq::ApplicationStub *s)
    throw (xdaq::exception::Exception);

    /**
     * Invoked when an exception has been received from the sentinel.
     */
    void onException(xcept::Exception &e);

private:

    /**
     * The logger of this application.
     */
    Logger logger_;

    /**
     * Used to access the I2O address map without a function call.
     */
    i2o::utils::AddressMap *i2oAddressMap_;

    /**
     * Used to access the memory pool factory without a function call.
     */
    toolbox::mem::MemoryPoolFactory *poolFactory_;

    /**
     * Used to access the application's info space without a function call.
     */
    xdata::InfoSpace *appInfoSpace_;

    /**
     * Used to access the application's descriptor without a function call.
     */
    xdaq::ApplicationDescriptor *appDescriptor_;

    /**
     * Used to access the application's context without a function call.
     */
    xdaq::ApplicationContext *appContext_;

    /**
     * Used to access the application's zone without a function call.
     */
    xdaq::Zone *zone_;

    /**
     * The XML class name of the application.
     */
    string xmlClass_;

    /**
     * The instance number of the application.
     */
    unsigned long instance_;

    /**
     * The application's URN.
     */
    string urn_;

    /**
     * The application descriptors of all the EVMs (there should only be 1
     * evm!).
     */
    vector< xdaq::ApplicationDescriptor* > evmDescriptors_;

    /**
     * The application descriptors of all the BUs.
     */
    vector< xdaq::ApplicationDescriptor* > buDescriptors_;

    /**
     * The application descriptors of all of the RUs.
     */
    vector< xdaq::ApplicationDescriptor* > ruDescriptors_;

    /**
     * The application descriptors of all of the TAs (there should only be 1
     * TA!).
     */
    vector< xdaq::ApplicationDescriptor* > taDescriptors_;

    /**
     * The application descriptors of all of the RUIs.
     */
    vector< xdaq::ApplicationDescriptor* > ruiDescriptors_;

    /**
     * The application descriptors of all of the FUs.
     */
    vector< xdaq::ApplicationDescriptor* > fuDescriptors_;

    /**
     * True if the test of the RU builder applications has been started, else
     * false.
     */
    bool testConfigured_;
    bool testStarted_;


    /////////////////////////////////////////////////////
    // Beginning of exported parameters for monitoring //
    /////////////////////////////////////////////////////

    /**
     * Exported read-only parameter specifying the current state of the
     * application.
     */
    // (Moved to EmuApplication)   xdata::String stateName_;

    /////////////////////////////////////////////////////
    // End of exported parameters for monitoring       //
    /////////////////////////////////////////////////////


    /**
     * Returns the name to be given to the logger of this application.
     */
    string generateLoggerName();

    /**
     * Get the descriptors of all the applications the EmuDAQManager will
     * need to control.
     */
    void getAllAppDescriptors();

    /**
     * Returns a vector of application descriptors order by instance number.
     */
    vector< xdaq::ApplicationDescriptor* > getAppDescriptors
    (
     xdaq::Zone             *zone,
     const string           appClass
    )
    throw (emuDAQManager::exception::Exception);

    /**
     * Creates the CSS file for this application.
     */
    void css
    (
        xgi::Input  *in,
        xgi::Output *out
    )
    throw (xgi::exception::Exception);

    /**
     * The default web page of the application.
     */
    void defaultWebPage(xgi::Input *in, xgi::Output *out)
    throw (xgi::exception::Exception);

    /**
     * Web page used to control the RU builder.
     */
    void controlWebPage(xgi::Input *in, xgi::Output *out)
    throw (xgi::exception::Exception);

  void governorForm(xgi::Input *in, xgi::Output *out)
    throw (xgi::exception::Exception);
  void processGovernorForm(xgi::Input *in, xgi::Output *out)
    throw (xgi::exception::Exception);
  void commentWebPage(xgi::Input *in, xgi::Output *out)
    throw (xgi::exception::Exception);
  void processCommentForm(xgi::Input *in)
    throw (xgi::exception::Exception);
  void commandWebPage(xgi::Input *in, xgi::Output *out)
    throw (xgi::exception::Exception);
  void printUserComments( xgi::Output *out );
  void stringReplace( string& inThis, const string& This, const string& withThis );
  string textToHtml( const string text );

  xdata::Boolean globalMode_; // true if we obey central run control
  xdata::Boolean configuredInGlobalMode_; // true if this run was configured on central run control's command

  string comments_;        // The comments entered by the user.
  string globalRunNumber_; // The global run number entered by the user.
  bool   badRun_;          // User's judgement on the run.

  xdata::Boolean postToELog_;         // whether or not to post to e-log
  xdata::String curlHost_;            // host on which to execute the curl command
  xdata::String curlCommand_;         // the curl command's full path
  xdata::String curlCookies_;         // file for cookies
  xdata::String CMSUserFile_;         // file that contains the username:password for CMS user
  xdata::String eLogUserFile_;        // file that contains the username:password:author for eLog user
  xdata::String eLogURL_;             // eLog's URL 

  EmuRunInfo *runInfo_; // communicates with run database

  xdata::String runDbBookingCommand_; // e.g. "java -jar runnumberbooker.jar"
  xdata::String runDbWritingCommand_; // e.g. "java -jar runinfowriter.jar"
  xdata::String runDbAddress_;        // e.g. "dbc:oracle:thin:@oracms.cern.ch:10121:omds"
  xdata::String runDbUserFile_;       // file that contains the username:password for run db user
  void bookRunNumber();
  void writeRunInfo( bool toDatabase, bool toELog );
  void postToELog( string subject, string body, vector<string> *attachments=0 );
  bool isBookedRunNumber_;

  xdata::UnsignedLong runNumber_;
  xdata::UnsignedLong runSequenceNumber_;
  xdata::Integer maxNumberOfEvents_;
  xdata::Vector<xdata::String> runTypes_; // all possible run types
  xdata::String runType_; // the current run type
  xdata::Boolean buildEvents_;
  xdata::String daqState_; // the combined state of the DAQ applications
  int stringToInt( const string* const s );
  int purgeIntNumberString( string* s );

  xdata::String hardwareMapping_; // file of the hardware mapping (path relative to XDAQ_ROOT

  vector< map< string,string > > getRUIEventCounts();
  vector< map< string,string > > getFUEventCounts();
  void printEventCountsTable( xgi::Output              *out,
			      string                    title,
			      vector< map< string,string > > counts );
  string getDateTime();
  string ageOfPageClock();
  void   getRunInfoFromTA( string* runnum, string* maxevents, string* starttime, string* stoptime );
  string reformatTime( string time );
  vector< pair<xdaq::ApplicationDescriptor*, string> > daqAppStates_;
  set<string> daqContexts_; // all different DAQ contexts with apps controlled by EmuDAQManager
  void   createAllAppStatesVector();
  void   queryAppStates( vector< pair<xdaq::ApplicationDescriptor*, string> > &appStates );
  string getDAQState();
  void   printStatesTable( xgi::Output *out,
			   string title,
			   set<string> &contexts,
			   vector< pair<xdaq::ApplicationDescriptor*, string> > &appStates )
    throw (xgi::exception::Exception);
  void getMnemonicNames();
  map<int,string> hardwareMnemonics_; // hardwareMnemonics[EmuRUI_instance]
  void printDAQState( xgi::Output *out, string state );

  xoap::MessageReference makeTAGenerateRunStopTime();

  vector< xdaq::ApplicationDescriptor* > dqmMonitorDescriptors_;
  set<string> dqmContexts_; // all different DQM contexts with apps controlled by EmuDAQManager
  vector< pair<xdaq::ApplicationDescriptor*, string> > dqmAppStates_;
  xdata::Boolean controlDQM_;
  void controlDQM( const string action )
    throw (emuDAQManager::exception::Exception);

  virtual void actionPerformed(xdata::Event & received ); // inherited from xdata::ActionListener

  // For driving the FSM by asynchronous SOAP.
  toolbox::task::WorkLoop *workLoop_;
//   toolbox::BSem bsem_;
  toolbox::task::ActionSignature *configureSignature_;
  toolbox::task::ActionSignature *enableSignature_;
  toolbox::task::ActionSignature *haltSignature_;

  // Parameters to obtain from TTCciControl
  xdata::String TTCci_ClockSource_;
  xdata::String TTCci_OrbitSource_;
  xdata::String TTCci_TriggerSource_;
  xdata::String TTCci_BGOSource_;
  // Parameters to obtain from TF_hyperDAQ
  xdata::String TF_triggerMode_;
  void getTriggerSources();
  void getTriggerMode();

  void setParametersForGlobalMode();

  string warningsToDisplay_;

  // STEP-specific stuff
  xdata::Boolean STEPFinished_; // set to TRUE when all DDUs' all live and unmasked inputs have produced the requested number of events
  bool printSTEPCountsTable( stringstream& out, bool control );
  bool isSTEPFinished();
  xoap::MessageReference querySTEP( xdaq::ApplicationDescriptor* ruiDescriptor )
    throw (emuDAQManager::exception::Exception);
  void maskDDUInputs( const bool in, const std::vector<cgicc::FormEntry>& fev );
  void sendDDUInputMask( const bool in, const unsigned int ruiInstance, const std::set<unsigned int>& inputs );


  // FM-specific stuff
  xdata::String TF_FM_URL_; /// (base) URL (http://host:port) of Track Finder Funtion Manager
  xdata::String CSC_FM_URL_; /// (base) URL (http://host:port) of CSC Funtion Manager
  xdata::String RegexMatchingTFConfigName_; /// regular expression matching the name of the Track Finder configuration
  xdata::String RegexMatchingCSCConfigName_; /// regular expression matching the name of the CSC configuration
  xdata::UnsignedLong TFConfigId_; /// unique id of the Track Finder configuration
  xdata::UnsignedLong CSCConfigId_; /// unique id of the CSC configuration
  xoap::MessageReference postSOAP( xoap::MessageReference message,
				   const string& URL,
				   const string& SOAPAction )
    throw (xdaq::exception::Exception);
  vector<string> parseRunningConfigurationsReplyFromFM( xoap::MessageReference reply );
  vector<string> getRunningConfigurationsFromFM( const string& URL )
    throw (emuDAQManager::exception::Exception);
  string parseConfigParameterReplyFromFM( xoap::MessageReference reply )
    throw(emuDAQManager::exception::Exception);
  string getConfigParameterFromFM( const string& configurationURL,
				   const string& parameterName )
  throw (emuDAQManager::exception::Exception);
  void getIdsOfRunningConfigurationsFromFM();



    /**
     * Processes the form sent from the control web page.
     */
    void processCommandForm(xgi::Input *in)
    throw (xgi::exception::Exception);

    /**
     * Prints hyper-text links to the specified applications.
     */
    void printAppInstanceLinks
    (
        xgi::Output                            *out,
        vector< xdaq::ApplicationDescriptor* > &appDescriptors
    );

    /**
     * Prints a row of HTML tables, where each table displays parameters
     * gathered from a single RU builder application.
     */
    void printParamsTables
    (
        xgi::Input                              *in,
        xgi::Output                             *out,
        vector< xdaq::ApplicationDescriptor* >  &appDescriptors,
        vector< vector< pair<string,string> > > paramsOfApps
    )
    throw (xgi::exception::Exception);

    /**
     * Prints an HTML table displaying parameters gathered from a RU builder
     * application.
     */
    void printParamsTable
    (
        xgi::Input                    *in,
        xgi::Output                   *out,
        xdaq::ApplicationDescriptor   *appDescriptor,
        vector< pair<string,string> > params
    )
    throw (xgi::exception::Exception);

    /**
     * Returns the hyper-text reference of the specified application.
     */
    string getHref(xdaq::ApplicationDescriptor *appDescriptor);

    /**
     * Gets and returns the current event number from the EVM.
     */
    vector< pair<string,string> > getEventNbFromEVM
    (
        xdaq::ApplicationDescriptor *evmDescriptor
    )
    throw (emuDAQManager::exception::Exception);

    /**
     * Gets and returns the values of statistics parameters exported by the
     * specified applications.
     */
    vector< vector< pair<string,string> > > getStats
    (
        vector< xdaq::ApplicationDescriptor* > &appDescriptors
    );

    /**
     * Get and returns the values of statistics parameters exported by the
     * specified application.
     */
    vector< pair<string,string> > getStats
    (
        xdaq::ApplicationDescriptor *appDescriptor
    );

    /**
     * Displays a web page of monitoring information that can be easily
     * parsed by software.
     */
    void machineReadableWebPage(xgi::Input *in, xgi::Output *out)
    throw (xgi::exception::Exception);

    /**
     * Exports the parameters.
     */
    void exportParams(xdata::InfoSpace *s);

    /**
     * Starts the DAQ.
     */
    void configureDAQ()
    throw (emuDAQManager::exception::Exception);
    void startDAQ()
    throw (emuDAQManager::exception::Exception);

    /**
     * Checks that there is a minimum set of applications to make a RU builder.
     */
    void checkThereIsARuBuilder()
    throw (emuDAQManager::exception::Exception);

    /**
     * Tell the EVM whether or not it should generate dummy triggers.
     */
    void setEVMGenerateDummyTriggers(const bool value)
    throw (emuDAQManager::exception::Exception);

    /**
     * Tell the RUs whether or not they should generate dummy super-fragments.
     */
    void setRUsGenerateDummySuperFrags(const bool vale)
    throw (emuDAQManager::exception::Exception);

    /**
     * Tells the BUs whether or not to drop events.
     */
    void setBUsDropEvents(const bool value)
    throw (emuDAQManager::exception::Exception);

    /**
     * Starts the imaginary trigger, i.e. the TA.
     */
    void configureTrigger()
    throw (emuDAQManager::exception::Exception);
    void startTrigger()
    throw (emuDAQManager::exception::Exception);

    /**
     * Starts just the RU builder, i.e. the BUs, EVM and RUs.
     */
    void configureRuBuilder()
      throw (emuDAQManager::exception::Exception);
    void startRuBuilder()
    throw (emuDAQManager::exception::Exception);

    /**
     * Starts the imaginary FED builder, i.e. the RUIs.
     */
    void configureFedBuilder()
    throw (emuDAQManager::exception::Exception);
    void startFedBuilder()
    throw (emuDAQManager::exception::Exception);

    /**
     * Starts the imaginary filter farm, i.e. the FUs.
     */
    void configureFilterFarm()
      throw (xcept::Exception);
//     throw (emuDAQManager::exception::Exception);
    void startFilterFarm()
    throw (emuDAQManager::exception::Exception);

    /**
     * Stops the test.
     */
    void stopDAQ()
    throw (emuDAQManager::exception::Exception);

    /**
     * Stops the imaginary FED builder, i.e. the RUIs.
     */
    void stopFedBuilder()
    throw (emuDAQManager::exception::Exception);

    /**
     * Stops just the RU builder, i.e. BUs, EVM and RUs.
     */
    void stopRuBuilder()
    throw (emuDAQManager::exception::Exception);

    /**
     * Stops the imaginary trigger, i.e. the TA.
     */
    void stopTrigger()
    throw (emuDAQManager::exception::Exception);

    /**
     * Stops the imaginary filter farm, i.e. the FUs.
     */
    void stopFilterFarm()
    throw (emuDAQManager::exception::Exception);

    /**
     * Resets applications.
     */
    void resetApps( vector< xdaq::ApplicationDescriptor* > apps )
    throw (emuDAQManager::exception::Exception);

    /**
     * Resets EmuRUIs and EmuFUs.
     */
    void resetDAQ()
    throw (emuDAQManager::exception::Exception);

    /**
     * Sends the specified FSM event as a SOAP message to the specified
     * application.  An exception is raised if the application does not reply
     * successfully with a SOAP response.
     */
    void sendFSMEventToApp
    (
        const string                 eventName,
        xdaq::ApplicationDescriptor* appDescriptor
    )
    throw (emuDAQManager::exception::Exception);

    /**
     * Creates a simple SOAP message representing a command with no
     * parameters.
     */
    xoap::MessageReference createSimpleSOAPCmdMsg(const string cmdName)
    throw (emuDAQManager::exception::Exception);

    /**
     * Gets and returns the value of the specified parameter from the specified
     * application.
     */
    string getScalarParam
    (
        xdaq::ApplicationDescriptor* appDescriptor,
        const string                 paramName,
        const string                 paramType
    )
    throw (emuDAQManager::exception::Exception);

  map<string,string> getScalarParams
  (
   xdaq::ApplicationDescriptor* appDescriptor,
   const map<string,string>     paramNamesAndTypes
   )
    throw (emuDAQManager::exception::Exception);
    
    /**
     * Sets the specified parameter of the specified application to the
     * specified value.
     */
    void setScalarParam
    (
        xdaq::ApplicationDescriptor* appDescriptor,
        const string                 paramName,
        const string                 paramType,
        const string                 paramValue
    )
    throw (emuDAQManager::exception::Exception);

    /**
     * Creates a ParameterGet SOAP message.
     */
    xoap::MessageReference createParameterGetSOAPMsg
    (
        const string appClass,
        const string paramName,
        const string paramType
    )
    throw (emuDAQManager::exception::Exception);

  xoap::MessageReference createParametersGetSOAPMsg
  (
   const string             appClass,
   const map<string,string> paramNamesAndTypes
   )
    throw (emuDAQManager::exception::Exception);

    /**
     * Creates a ParameterSet SOAP message.
     */
    xoap::MessageReference createParameterSetSOAPMsg
    (
        const string appClass,
        const string paramName,
        const string paramType,
        const string paramValue
    )
    throw (emuDAQManager::exception::Exception);

    /**
     * Returns the value of the specified parameter from the specified SOAP
     * message.
     */
    string extractScalarParameterValueFromSoapMsg
    (
        xoap::MessageReference msg,
        const string           paramName
    )
    throw (emuDAQManager::exception::Exception);

  map<string,string> extractScalarParameterValuesFromSoapMsg
  (
   xoap::MessageReference   msg,
   const map<string,string> paramNamesAndTypes
   )
    throw (emuDAQManager::exception::Exception);
  
    /**
     * Retruns the node with the specified local name from the specified list
     * of node.  An exception is thrown if the node is not found.
     */
    DOMNode *findNode
    (
        DOMNodeList *nodeList,
        const string nodeLocalName
    )
    throw (emuDAQManager::exception::Exception);

    /**
     * Prints the specified soap message to standard out.
     */
    void printSoapMsgToStdOut(xoap::MessageReference message);

  // Supervisor-specific stuff:
public:
  // SOAP interface
  xoap::MessageReference onConfigure(xoap::MessageReference message)
    throw (xoap::exception::Exception);
  xoap::MessageReference onEnable(xoap::MessageReference message)
    throw (xoap::exception::Exception);
  xoap::MessageReference onDisable(xoap::MessageReference message)
    throw (xoap::exception::Exception);
  xoap::MessageReference onHalt(xoap::MessageReference message)
    throw (xoap::exception::Exception);
  xoap::MessageReference onReset(xoap::MessageReference message)
    throw (xoap::exception::Exception);
  xoap::MessageReference onQueryDAQState(xoap::MessageReference message)
    throw (xoap::exception::Exception);
  xoap::MessageReference onQueryRunSummary(xoap::MessageReference message)
    throw (xoap::exception::Exception);

  // State transitions
  void configureAction(toolbox::Event::Reference e)
    throw (toolbox::fsm::exception::Exception);
  void enableAction(toolbox::Event::Reference e)
    throw (toolbox::fsm::exception::Exception);
  void disableAction(toolbox::Event::Reference e)
    throw (toolbox::fsm::exception::Exception);
  void haltAction(toolbox::Event::Reference e)
    throw (toolbox::fsm::exception::Exception);
  
  void reConfigureAction(toolbox::Event::Reference e)
    throw (toolbox::fsm::exception::Exception);
  void noAction(toolbox::Event::Reference e)
    throw (toolbox::fsm::exception::Exception);
  void resetAction()
    throw (toolbox::fsm::exception::Exception);

  bool configureActionInWorkLoop(toolbox::task::WorkLoop *wl);
  bool enableActionInWorkLoop(toolbox::task::WorkLoop *wl);
  bool haltActionInWorkLoop(toolbox::task::WorkLoop *wl);

private:
  void stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
    throw (toolbox::fsm::exception::Exception);

};


#endif
