/*****************************************************************************\
* $Id: ConfigurationEditor.cc,v 1.10 2009/12/10 16:55:02 paste Exp $
\*****************************************************************************/
#include "emu/fed/ConfigurationEditor.h"

#include <vector>
#include <sstream>
#include <fstream>

#include "xgi/Method.h"
#include "cgicc/HTMLClasses.h"
#include "xdaq2rc/RcmsStateNotifier.h"
#include "xdata/TimeVal.h"
#include "toolbox/TimeVal.h"
#include "toolbox/string.h"
#include "emu/base/Alarm.h"
#include "emu/fed/JSONSpiritWriter.h"
#include "emu/fed/XMLConfigurator.h"
#include "emu/fed/DBConfigurator.h"
#include "emu/fed/Crate.h"
#include "emu/fed/VMEController.h"
#include "emu/fed/DDU.h"
#include "emu/fed/DCC.h"
#include "emu/fed/Fiber.h"
#include "emu/fed/FIFO.h"
#include "emu/fed/SystemDBAgent.h"

XDAQ_INSTANTIATOR_IMPL(emu::fed::ConfigurationEditor)

emu::fed::ConfigurationEditor::ConfigurationEditor(xdaq::ApplicationStub *stub):
xdaq::WebApplication(stub),
emu::fed::Application(stub),
systemName_(""),
dbKey_(0)
{
	
	// Variables that are to be made available to other applications
	xdata::InfoSpace *infoSpace = getApplicationInfoSpace();
	infoSpace->fireItemAvailable("dbUsername", &dbUsername_);
	infoSpace->fireItemAvailable("dbPassword", &dbPassword_);
	
	// HyperDAQ pages
	xgi::bind(this, &emu::fed::ConfigurationEditor::webDefault, "Default");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webUploadFile, "UploadFile");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webGetDBKeys, "GetDBKeys");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webLoadFromDB, "LoadFromDB");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webCreateNew, "CreateNew");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webWriteXML, "WriteXML");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webBuildSystem, "BuildSystem");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webBuildCrate, "BuildCrate");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webBuildController, "BuildController");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webBuildDDU, "BuildDDU");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webBuildFiber, "BuildFiber");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webBuildDCC, "BuildDCC");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webBuildFIFO, "BuildFIFO");
	xgi::bind(this, &emu::fed::ConfigurationEditor::webUploadToDB, "UploadToDB");
	
	timeStamp_ = time(NULL);
}


// HyperDAQ pages
void emu::fed::ConfigurationEditor::webDefault(xgi::Input *in, xgi::Output *out)
{
	
	std::vector<std::string> jsFileNames;
	jsFileNames.push_back("definitions.js");
	jsFileNames.push_back("configurationEditor.js");
	jsFileNames.push_back("common.js");
	*out << Header("FED Crate Configuration Editor", jsFileNames);
	
	*out << cgicc::div()
		.set("class", "titlebar default_width")
		.set("id", "FED_Configuration_Selection_titlebar") << std::endl;
	*out << cgicc::div("Configuration Selection")
		.set("class", "titletext") << std::endl;
	*out << cgicc::div() << std::endl;
	
	*out << cgicc::fieldset()
		.set("class", "dialog default_width")
		.set("id", "FED_Configuration_Selection_dialog") << std::endl;
		
	*out << cgicc::div("Upload XML file")
		.set("class", "category tier0") << std::endl;
	std::ostringstream formAction;
	formAction << getApplicationDescriptor()->getContextDescriptor()->getURL() << "/" << getApplicationDescriptor()->getURN() << "/UploadFile";
	*out << cgicc::form()
		.set("name", "xmlFileForm")
		.set("id", "xml_file_form")
		.set("style", "display: inline;")
		.set("method", "POST")
		.set("enctype", "multipart/form-data")
		.set("action", formAction.str())
		.set("target", "xml_file_frame") << std::endl;
	*out << cgicc::input()
		.set("class", "tier1")
		.set("name", "xmlFile")
		.set("id", "xml_file_upload")
		.set("type", "file") << std::endl;
	*out << cgicc::form() << std::endl;
	*out << cgicc::button("Upload")
		.set("name", "xmlFileButton")
		.set("id", "xml_file_button") << std::endl;
	*out << cgicc::iframe()
		.set("name", "xml_file_frame")
		.set("id", "xml_file_frame")
		.set("style", "display: none;") << std::endl;
	*out << cgicc::iframe() << std::endl;
	
	*out << cgicc::div("Select pre-existing configuration")
		.set("class", "category tier0") << std::endl;
	*out << cgicc::div()
		.set("class", "tier1") << std::endl;
	*out << cgicc::span("System:") << std::endl;
	*out << cgicc::select()
		.set("name", "configurationDescription")
		.set("id", "configuration_description") << std::endl;
	*out << cgicc::option("Loading...") << std::endl;
	*out << cgicc::select() << std::endl;
	*out << cgicc::span("Key:") << std::endl;
	*out << cgicc::select()
		.set("name", "configurationKey")
		.set("id", "configuration_key") << std::endl;
	*out << cgicc::option("Loading...") << std::endl;
	*out << cgicc::select() << std::endl;
	*out << cgicc::button("Load from database")
		.set("name", "loadButton")
		.set("id", "load_button") << std::endl;
	*out << cgicc::div() << std::endl;
	
	*out << cgicc::div("Create a configuration from scratch")
		.set("class", "category tier0") << std::endl;
	*out << cgicc::div()
		.set("class", "tier1") << std::endl;
	*out << cgicc::button("Create empty configuration")
		.set("name", "createButton")
		.set("id", "create_button") << std::endl;
	*out << cgicc::div() << std::endl;
	
	*out << cgicc::fieldset() << std::endl;
	
	
	if (crateVector_.size() || systemName_ != "") {
		*out << cgicc::div()
			.set("class", "titlebar default_width")
			.set("id", "FED_Configuration_Editor_titlebar") << std::endl;
		*out << cgicc::div("Configuration Editor")
			.set("class", "titletext") << std::endl;
		*out << cgicc::div() << std::endl;
		
		*out << cgicc::fieldset()
			.set("class", "dialog default_width")
			.set("id", "FED_Configuration_Editor_dialog") << std::endl;
			
		*out << cgicc::div("Find a fiber")
			.set("class", "category tier0") << std::endl;
		*out << cgicc::div()
			.set("class", "tier1") << std::endl;
		*out << cgicc::span("Chamber/SP name: ") << std::endl;
		*out << cgicc::input()
			.set("type", "text")
			.set("id", "find_a_fiber")
			.set("class", "find_a_fiber") << std::endl;
		*out << cgicc::button("Find")
			.set("class", "find_button")
			.set("id", "find_button") << std::endl;
		*out << cgicc::span("No matching fiber found")
			.set("id", "find_a_fiber_error")
			.set("class", "red hidden") << std::endl;
		*out << cgicc::div() << std::endl;
		
		*out << cgicc::div("System")
			.set("class", "category tier0") << std::endl;
		*out << cgicc::table()
			.set("class", "tier1") << std::endl;
		*out << cgicc::tr() << std::endl;
		*out << cgicc::td("Database key") << std::endl;
		*out << cgicc::td() << std::endl;
		*out << cgicc::input()
			.set("type", "text")
			.set("id", "input_database_key")
			.set("value", dbKey_.toString()) << std::endl;
		*out << cgicc::td() << std::endl;
		*out << cgicc::tr() << std::endl;
		*out << cgicc::tr() << std::endl;
		*out << cgicc::td("Name") << std::endl;
		*out << cgicc::td() << std::endl;
		*out << cgicc::input()
			.set("type", "text")
			.set("id", "input_database_name")
			.set("value", systemName_.toString()) << std::endl;
		*out << cgicc::td() << std::endl;
		*out << cgicc::tr() << std::endl;
		*out << cgicc::tr() << std::endl;
		*out << cgicc::td("Timestamp") << std::endl;
		*out << cgicc::td()
			.set("id", "timestamp") << std::endl;
		*out << toolbox::TimeVal(timeStamp_).toString(toolbox::TimeVal::gmt) << std::endl;
		*out << cgicc::td() << std::endl;
		*out << cgicc::tr() << std::endl;
		*out << cgicc::table() << std::endl;
		
		*out << cgicc::div()
			.set("class", "tier1 bold") << std::endl;
		*out << cgicc::img()
			.set("id", "crates_open_close")
			.set("class", "crates_open_close pointer")
			.set("src", "/emu/emuDCS/FEDApps/images/plus.png") << std::endl;
		*out << cgicc::span("Crates")
			.set("class", "crates_open_close pointer") << std::endl;
		*out << cgicc::div() << std::endl;
		
		for (std::vector<Crate *>::iterator iCrate = crateVector_.begin(); iCrate != crateVector_.end(); iCrate++) {
			
			std::ostringstream sCrateNumber;
			sCrateNumber << (*iCrate)->getNumber();
			std::string crateNumber = sCrateNumber.str();
			*out << cgicc::table()
				.set("id", "config_crate_table_" + crateNumber)
				.set("class", "tier2 config_crate_table hidden")
				.set("crates_hidden", "1")
				.set("crate", crateNumber) << std::endl;
			*out << cgicc::tr() << std::endl;
			*out << cgicc::td()
				.set("class", "bold config_crate_name crate_open_close pointer")
				.set("crate", crateNumber) << std::endl;
			*out << cgicc::img()
				.set("class", "crate_open_close pointer")
				.set("crate", crateNumber)
				.set("src", "/emu/emuDCS/FEDApps/images/plus.png") << std::endl;
			*out << "Crate " + crateNumber << std::endl;
			*out << cgicc::td() << std::endl;
			*out << cgicc::td() << std::endl;
			*out << cgicc::button()
				.set("class", "delete_crate")
				.set("crate", crateNumber);
			*out << cgicc::img()
				.set("class", "icon")
				.set("src", "/emu/emuDCS/FEDApps/images/list-remove.png");
			*out << "Delete crate" << std::endl;
			*out << cgicc::button() << std::endl;
			*out << cgicc::td() << std::endl;
			*out << cgicc::tr() << std::endl;
			*out << cgicc::tr()
				.set("id", "config_crate_number_row_" + crateNumber)
				.set("class", "hidden")
				.set("crates_hidden", "1")
				.set("crate_hidden", "1")
				.set("crate", crateNumber) << std::endl;
			*out << cgicc::td("Number") << std::endl;
			*out << cgicc::td() << std::endl;
			*out << cgicc::input()
				.set("type", "text")
				.set("class", "input_crate_number")
				.set("crate", crateNumber)
				.set("value", crateNumber) << std::endl;
			*out << cgicc::td() << std::endl;
			*out << cgicc::tr() << std::endl;
			*out << cgicc::table() << std::endl;
			
			*out << cgicc::table()
				.set("id", "config_controller_table_" + crateNumber)
				.set("class", "tier3 controller_table hidden")
				.set("crates_hidden", "1")
				.set("crate_hidden", "1")
				.set("crate", crateNumber) << std::endl;
			*out << cgicc::tr() << std::endl;
			*out << cgicc::td("VME Controller")
				.set("class", "bold controller_name")
				.set("crate", crateNumber) << std::endl;
			*out << cgicc::td() << std::endl;
			*out << cgicc::td() << std::endl;
			*out << cgicc::tr() << std::endl;
			*out << cgicc::tr() << std::endl;
			*out << cgicc::td("Device") << std::endl;
			*out << cgicc::td() << std::endl;
			std::ostringstream controllerDevice;
			controllerDevice << (*iCrate)->getController()->getDevice();
			*out << cgicc::input()
				.set("type", "text")
				.set("class", "input_controller_device")
				.set("crate", crateNumber)
				.set("value", controllerDevice.str()) << std::endl;
			*out << cgicc::td() << std::endl;
			*out << cgicc::tr() << std::endl;
			*out << cgicc::tr() << std::endl;
			*out << cgicc::td("Link") << std::endl;
			*out << cgicc::td() << std::endl;
			std::ostringstream controllerLink;
			controllerLink << (*iCrate)->getController()->getLink();
			*out << cgicc::input()
				.set("type", "text")
				.set("class", "input_controller_link")
				.set("crate", crateNumber)
				.set("value", controllerLink.str()) << std::endl;
			*out << cgicc::td() << std::endl;
			*out << cgicc::tr() << std::endl;
			*out << cgicc::table() << std::endl;
			
			*out << cgicc::div()
				.set("id", "config_ddus_" + crateNumber)
				.set("class", "bold tier3 hidden")
				.set("crates_hidden", "1")
				.set("crate_hidden", "1")
				.set("crate", crateNumber) << std::endl;
			*out << cgicc::img()
				.set("class", "ddus_open_close pointer")
				.set("crate", crateNumber)
				.set("src", "/emu/emuDCS/FEDApps/images/plus.png") << std::endl;
			*out << cgicc::span("DDUs")
				.set("class", "ddus_open_close pointer")
				.set("crate", crateNumber) << std::endl;
			*out << cgicc::div() << std::endl;
			
			std::vector<DDU *> ddus = (*iCrate)->getDDUs();
			for (std::vector<DDU *>::iterator iDDU = ddus.begin(); iDDU != ddus.end(); ++iDDU) {
				
				std::ostringstream sDDUNumber;
				sDDUNumber << (*iDDU)->getRUI();
				std::string dduNumber = sDDUNumber.str();

				*out << cgicc::table()
					.set("id", "config_ddu_" + crateNumber + "_" + dduNumber)
					.set("class", "tier4 ddu_table hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::td()
					.set("class", "bold ddu_name ddu_open_close pointer")
					.set("rui", dduNumber)
					.set("crate", crateNumber) << std::endl;
				*out << cgicc::img()
					.set("class", "ddu_open_close pointer")
					.set("rui", dduNumber)
					.set("crate", crateNumber)
					.set("src", "/emu/emuDCS/FEDApps/images/plus.png") << std::endl;
				*out << "DDU " + dduNumber << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::button()
					.set("class", "delete_ddu")
					.set("crate", crateNumber)
					.set("rui", dduNumber);
				*out << cgicc::img()
					.set("class", "icon")
					.set("src", "/emu/emuDCS/FEDApps/images/list-remove.png");
				*out << "Delete DDU" << std::endl;
				*out << cgicc::button() << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_ddu_row_slot_" + crateNumber + "_" + dduNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::td("Slot") << std::endl;
				*out << cgicc::td() << std::endl;
				std::ostringstream slotNumber;
				slotNumber << (*iDDU)->getSlot();
				*out << cgicc::input()
					.set("type", "text")
					.set("class", "input_ddu_slot")
					.set("crate", crateNumber)
					.set("rui", dduNumber)
					.set("value", slotNumber.str()) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_ddu_row_rui_" + crateNumber + "_" + dduNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::td("RUI") << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::input()
					.set("type", "text")
					.set("class", "input_ddu_rui")
					.set("crate", crateNumber)
					.set("rui", dduNumber)
					.set("value", dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_ddu_row_fmmid_" + crateNumber + "_" + dduNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::td("FMM ID") << std::endl;
				*out << cgicc::td() << std::endl;
				std::ostringstream fmmID;
				fmmID << (*iDDU)->getFMMID();
				*out << cgicc::input()
					.set("type", "text")
					.set("class", "input_ddu_fmm_id")
					.set("crate", crateNumber)
					.set("rui", dduNumber)
					.set("value", fmmID.str()) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_ddu_row_fec_" + crateNumber + "_" + dduNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Enable forced error checks")
					.set("for", "input_ddu_force_checks_" + crateNumber + "_" + dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input fecInput;
				fecInput.set("type", "checkbox")
					.set("id", "input_ddu_force_checks_" + crateNumber + "_" + dduNumber)
					.set("class", "input_ddu_force_checks")
					.set("crate", crateNumber)
					.set("rui", dduNumber);
				if ((*iDDU)->getKillFiber() & (1 << 15)) fecInput.set("checked", "checked");
				*out << fecInput << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_ddu_row_alct_" + crateNumber + "_" + dduNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Enable forced ALCT checks")
					.set("for", "input_ddu_force_alct_" + crateNumber + "_" + dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input alctInput;
				alctInput.set("type", "checkbox")
					.set("id", "input_ddu_force_alct_" + crateNumber + "_" + dduNumber)
					.set("class", "input_ddu_force_alct")
					.set("crate", crateNumber)
					.set("rui", dduNumber);
				if ((*iDDU)->getKillFiber() & (1 << 16)) alctInput.set("checked", "checked");
				*out << alctInput << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_ddu_row_tmb_" + crateNumber + "_" + dduNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Enable forced TMB checks")
					.set("for", "input_ddu_force_tmb_" + crateNumber + "_" + dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input tmbInput;
				tmbInput.set("type", "checkbox")
					.set("id", "input_ddu_force_tmb_" + crateNumber + "_" + dduNumber)
					.set("class", "input_ddu_force_tmb")
					.set("crate", crateNumber)
					.set("rui", dduNumber);
				if ((*iDDU)->getKillFiber() & (1 << 17)) tmbInput.set("checked", "checked");
				*out << tmbInput << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_ddu_row_cfeb_" + crateNumber + "_" + dduNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Enable forced CFEB checks")
					.set("for", "input_ddu_force_cfeb_" + crateNumber + "_" + dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input cfebInput;
				cfebInput.set("type", "checkbox")
					.set("id", "input_ddu_force_cfeb_" + crateNumber + "_" + dduNumber)
					.set("class", "input_ddu_force_cfeb")
					.set("crate", crateNumber)
					.set("rui", dduNumber);
				if ((*iDDU)->getKillFiber() & (1 << 18)) cfebInput.set("checked", "checked");
				*out << cfebInput << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_ddu_row_dmb_" + crateNumber + "_" + dduNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Enable forced normal DMB")
					.set("for", "input_ddu_force_dmb_" + crateNumber + "_" + dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input dmbInput;
				dmbInput.set("type", "checkbox")
					.set("id", "input_ddu_force_dmb_" + crateNumber + "_" + dduNumber)
					.set("class", "input_ddu_force_dmb")
					.set("crate", crateNumber)
					.set("rui", dduNumber);
				if ((*iDDU)->getKillFiber() & (1 << 19)) dmbInput.set("checked", "checked");
				*out << dmbInput << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_ddu_row_gbe_" + crateNumber + "_" + dduNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::td("Gigabit ethernet prescale") << std::endl;
				*out << cgicc::td() << std::endl;
				std::ostringstream gbePrescale;
				gbePrescale << (*iDDU)->getGbEPrescale();
				*out << cgicc::input()
					.set("type", "text")
					.set("class", "input_ddu_gbe_prescale")
					.set("crate", crateNumber)
					.set("rui", dduNumber)
					.set("value", gbePrescale.str()) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_ddu_row_ccb_" + crateNumber + "_" + dduNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Invert CCB command signals")
					.set("for", "input_ddu_invert_ccb_" + crateNumber + "_" + dduNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input ccbInput;
				ccbInput.set("type", "checkbox")
					.set("id", "input_ddu_invert_ccb_" + crateNumber + "_" + dduNumber)
					.set("class", "input_ddu_invert_ccb")
					.set("crate", crateNumber)
					.set("rui", dduNumber);
				if ((*iDDU)->getRUI() == 0xc0) ccbInput.set("checked", "checked");
				*out << ccbInput << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::table() << std::endl;
					
				*out << cgicc::div()
					.set("id", "config_fibers_" + crateNumber + "_" + dduNumber)
					.set("class", "bold tier4 hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::img()
					.set("class", "fibers_open_close pointer")
					.set("crate", crateNumber)
					.set("rui", dduNumber)
					.set("src", "/emu/emuDCS/FEDApps/images/plus.png") << std::endl;
				*out << cgicc::span("Fibers")
					.set("class", "fibers_open_close pointer")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::div() << std::endl;
				
				std::vector<Fiber *> fibers = (*iDDU)->getFibers();
				for (std::vector<Fiber *>::iterator iFiber = fibers.begin(); iFiber != fibers.end(); ++iFiber) {
					
					std::ostringstream sFiberNumber;
					sFiberNumber << (*iFiber)->getFiberNumber();
					std::string fiberNumber = sFiberNumber.str();
					
					*out << cgicc::table()
						.set("id", "config_fiber_table_" + crateNumber + "_" + dduNumber + "_" + fiberNumber)
						.set("class", "tier5 fiber_table hidden")
						.set("crates_hidden", "1")
						.set("crate_hidden", "1")
						.set("ddus_hidden", "1")
						.set("ddu_hidden", "1")
						.set("fibers_hidden", "1")
						.set("crate", crateNumber)
						.set("rui", dduNumber)
						.set("fiber", fiberNumber) << std::endl;
					*out << cgicc::tr() << std::endl;
					*out << cgicc::td("Fiber " + fiberNumber)
						.set("class", "bold fiber_name")
						.set("crate", crateNumber)
						.set("rui", dduNumber)
						.set("fiber", fiberNumber) << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::button()
						.set("class", "delete_fiber")
						.set("crate", crateNumber)
						.set("rui", dduNumber)
						.set("fiber", fiberNumber);
					*out << cgicc::img()
						.set("class", "icon")
						.set("src", "/emu/emuDCS/FEDApps/images/list-remove.png");
					*out << "Delete Fiber" << std::endl;
					*out << cgicc::button() << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::tr() << std::endl;
					*out << cgicc::tr()
						.set("id", "config_fiber_row_number_" + crateNumber + "_" + dduNumber + "_" + fiberNumber)
						.set("class", "hidden")
						.set("crates_hidden", "1")
						.set("crate_hidden", "1")
						.set("ddus_hidden", "1")
						.set("ddu_hidden", "1")
						.set("fibers_hidden", "1")
						.set("crate", crateNumber)
						.set("rui", dduNumber)
						.set("fiber", fiberNumber) << std::endl;
					*out << cgicc::td("Number") << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::input()
						.set("type", "text")
						.set("class", "input_fiber_number")
						.set("crate", crateNumber)
						.set("rui", dduNumber)
						.set("fiber", fiberNumber)
						.set("value", fiberNumber) << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::tr() << std::endl;
					*out << cgicc::tr()
						.set("id", "config_fiber_row_name_" + crateNumber + "_" + dduNumber + "_" + fiberNumber)
						.set("class", "hidden")
						.set("crates_hidden", "1")
						.set("crate_hidden", "1")
						.set("ddus_hidden", "1")
						.set("ddu_hidden", "1")
						.set("fibers_hidden", "1")
						.set("rui", dduNumber)
						.set("crate", crateNumber)
						.set("fiber", fiberNumber) << std::endl;
					*out << cgicc::td("Name") << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::input()
						.set("type", "text")
						.set("class", "input_fiber_name")
						.set("crate", crateNumber)
						.set("rui", dduNumber)
						.set("fiber", fiberNumber)
						.set("value", (*iFiber)->getName()) << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::tr() << std::endl;
					*out << cgicc::tr()
						.set("id", "config_fiber_row_killed_" + crateNumber + "_" + dduNumber + "_" + fiberNumber)
						.set("class", "hidden")
						.set("crates_hidden", "1")
						.set("crate_hidden", "1")
						.set("ddus_hidden", "1")
						.set("ddu_hidden", "1")
						.set("fibers_hidden", "1")
						.set("rui", dduNumber)
						.set("crate", crateNumber)
						.set("fiber", fiberNumber) << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::label("Killed")
					.set("for", "input_fiber_killed_" + crateNumber + "_" + dduNumber + "_" + fiberNumber) << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::td() << std::endl;
					cgicc::input killInput;
					killInput.set("type", "checkbox")
						.set("id", "input_fiber_killed_" + crateNumber + "_" + dduNumber + "_" + fiberNumber)
						.set("class", "input_fiber_killed")
						.set("rui", dduNumber)
						.set("crate", crateNumber)
						.set("fiber", fiberNumber);
					if ((*iFiber)->isKilled()) killInput.set("checked", "checked");
					*out << killInput << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::tr() << std::endl;
					*out << cgicc::table() << std::endl;
					
				}
				
				*out << cgicc::div()
					.set("id", "config_fiber_add_" + crateNumber + "_" + dduNumber)
					.set("class", "tier5 hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("ddus_hidden", "1")
					.set("ddu_hidden", "1")
					.set("fibers_hidden", "1")
					.set("crate", crateNumber)
					.set("rui", dduNumber) << std::endl;
				*out << cgicc::button()
					.set("class", "add_fiber")
					.set("crate", crateNumber)
					.set("rui", dduNumber);
				*out << cgicc::img()
					.set("class", "icon")
					.set("src", "/emu/emuDCS/FEDApps/images/list-add.png");
				*out << "Add Fiber" << std::endl;
				*out << cgicc::button() << std::endl;
				*out << cgicc::div() << std::endl;
				
			}
			
			*out << cgicc::div()
				.set("id", "config_ddu_add_" + crateNumber)
				.set("class", "tier4 hidden")
				.set("crates_hidden", "1")
				.set("crate_hidden", "1")
				.set("ddus_hidden", "1")
				.set("crate", crateNumber) << std::endl;
			*out << cgicc::button()
				.set("class", "add_ddu")
				.set("crate", crateNumber);
			*out << cgicc::img()
				.set("class", "icon")
				.set("src", "/emu/emuDCS/FEDApps/images/list-add.png");
			*out << "Add DDU" << std::endl;
			*out << cgicc::button() << std::endl;
			*out << cgicc::div() << std::endl;
			
			*out << cgicc::div()
				.set("id", "config_dccs_" + crateNumber)
				.set("class", "bold tier3 hidden")
				.set("crates_hidden", "1")
				.set("crate_hidden", "1")
				.set("crate", crateNumber) << std::endl;
			*out << cgicc::img()
				.set("class", "dccs_open_close pointer")
				.set("crate", crateNumber)
				.set("src", "/emu/emuDCS/FEDApps/images/plus.png") << std::endl;
			*out << cgicc::span("DCCs")
				.set("class", "dccs_open_close pointer")
				.set("crate", crateNumber) << std::endl;
			*out << cgicc::div() << std::endl;
			
			std::vector<DCC *> dccs = (*iCrate)->getDCCs();
			for (std::vector<DCC *>::iterator iDCC = dccs.begin(); iDCC != dccs.end(); ++iDCC) {
				
				std::ostringstream sDCCNumber;
				sDCCNumber << (*iDCC)->getFMMID();
				std::string dccNumber = sDCCNumber.str();
				
				*out << cgicc::table()
					.set("id", "config_dcc_table_" + crateNumber + "_" + dccNumber)
					.set("class", "tier4 dcc_table hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::td()
					.set("class", "bold dcc_name dcc_open_close pointer")
					.set("fmmid", dccNumber)
					.set("crate", crateNumber) << std::endl;
				*out << cgicc::img()
					.set("class", "dcc_open_close pointer")
					.set("fmmid", dccNumber)
					.set("crate", crateNumber)
					.set("src", "/emu/emuDCS/FEDApps/images/plus.png") << std::endl;
				*out << "DCC " + dccNumber << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::button()
					.set("class", "delete_dcc")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber);
				*out << cgicc::img()
					.set("class", "icon")
					.set("src", "/emu/emuDCS/FEDApps/images/list-remove.png");
				*out << "Delete DCC" << std::endl;
				*out << cgicc::button() << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_dcc_row_slot_" + crateNumber + "_" + dccNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("dcc_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::td("Slot") << std::endl;
				*out << cgicc::td() << std::endl;
				std::ostringstream slotNumber;
				slotNumber << (*iDCC)->getSlot();
				*out << cgicc::input()
					.set("type", "text")
					.set("class", "input_dcc_slot")
					.set("fmmid", dccNumber)
					.set("crate", crateNumber)
					.set("value", slotNumber.str()) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_dcc_row_fmmid_" + crateNumber + "_" + dccNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("dcc_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::td("FMM ID") << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::input()
					.set("type", "text")
					.set("class", "input_dcc_fmm_id")
					.set("fmmid", dccNumber)
					.set("crate", crateNumber)
					.set("value", dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_dcc_row_slink1_" + crateNumber + "_" + dccNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("dcc_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::td("SLink 1 ID") << std::endl;
				*out << cgicc::td() << std::endl;
				std::ostringstream slink1ID;
				slink1ID << (*iDCC)->getSLinkID(1);
				*out << cgicc::input()
						.set("type", "text")
						.set("class", "input_dcc_slink1")
						.set("fmmid", dccNumber)
						.set("crate", crateNumber)
						.set("value", slink1ID.str()) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_dcc_row_slink2" + crateNumber + "_" + dccNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("dcc_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::td("SLink 2 ID") << std::endl;
				*out << cgicc::td() << std::endl;
				std::ostringstream slink2ID;
				slink2ID << (*iDCC)->getSLinkID(2);
				*out << cgicc::input()
					.set("type", "text")
					.set("class", "input_dcc_slink2")
					.set("fmmid", dccNumber)
					.set("crate", crateNumber)
					.set("value", slink2ID.str()) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_dcc_row_sw_" + crateNumber + "_" + dccNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("dcc_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Enable software switch")
					.set("for", "input_dcc_sw_switch_" + crateNumber + "_" + dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input swInput;
				swInput.set("type", "checkbox")
					.set("id", "input_dcc_sw_switch_" + crateNumber + "_" + dccNumber)
					.set("class", "input_dcc_sw_switch")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber);
				if ((*iDCC)->getSoftwareSwitch() & 0x200) swInput.set("checked", "checked");
				*out << swInput << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_dcc_row_ttc_" + crateNumber + "_" + dccNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("dcc_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Ignore TTCRx-not-ready signal")
					.set("for", "input_dcc_ignore_ttc_" + crateNumber + "_" + dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input ttcInput;
				ttcInput.set("type", "checkbox")
					.set("id", "input_dcc_ignore_ttc_" + crateNumber + "_" + dccNumber)
					.set("class", "input_dcc_ignore_ttc")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber);
				if ((*iDCC)->getSoftwareSwitch() & 0x1000) ttcInput.set("checked", "checked");
				*out << ttcInput << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_dcc_row_slinkbp_" + crateNumber + "_" + dccNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("dcc_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Ignore SLink backpressure")
					.set("for", "input_dcc_ignore_backpressure_" + crateNumber + "_" + dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input bpInput;
				bpInput.set("type", "checkbox")
					.set("id", "input_dcc_ignore_backpressure_" + crateNumber + "_" + dccNumber)
					.set("class", "input_dcc_ignore_backpressure")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber);
				if ((*iDCC)->getSoftwareSwitch() & 0x2000) bpInput.set("checked", "checked");
				*out << bpInput << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_dcc_row_slinknp_" + crateNumber + "_" + dccNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("dcc_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Ignore SLink-not-present")
					.set("for", "input_dcc_ignore_slink_" + crateNumber + "_" + dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input slinkInput;
				slinkInput.set("type", "checkbox")
					.set("id", "input_dcc_ignore_slink_" + crateNumber + "_" + dccNumber)
					.set("class", "input_dcc_ignore_slink")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber);
				if ((*iDCC)->getSoftwareSwitch() & 0x4000) slinkInput.set("checked", "checked");
				*out << slinkInput << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_dcc_row_sw4_" + crateNumber + "_" + dccNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("dcc_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Switch bit 4")
					.set("for", "input_dcc_sw4_" + crateNumber + "_" + dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input sw4Input;
				sw4Input.set("type", "checkbox")
					.set("id", "input_dcc_sw4_" + crateNumber + "_" + dccNumber)
					.set("class", "input_dcc_sw4")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber);
				if ((*iDCC)->getSoftwareSwitch() & 0x10) sw4Input.set("checked", "checked");
				*out << sw4Input << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::tr()
					.set("id", "config_dcc_row_sw5_" + crateNumber + "_" + dccNumber)
					.set("class", "hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("dcc_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::label("Switch bit 5")
					.set("for", "input_dcc_sw5_" + crateNumber + "_" + dccNumber) << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::td() << std::endl;
				cgicc::input sw5Input;
				sw5Input.set("type", "checkbox")
					.set("id", "input_dcc_sw5_" + crateNumber + "_" + dccNumber)
					.set("class", "input_dcc_sw5")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber);
				if ((*iDCC)->getSoftwareSwitch() & 0x20) sw5Input.set("checked", "checked");
				*out << sw5Input << std::endl;
				*out << cgicc::td() << std::endl;
				*out << cgicc::tr() << std::endl;
				*out << cgicc::table() << std::endl;
					
				*out << cgicc::div()
					.set("id", "config_fifos_" + crateNumber + "_" + dccNumber)
					.set("class", "bold tier4 hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("dcc_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::img()
					.set("class", "fifos_open_close pointer")
					.set("fmmid", dccNumber)
					.set("crate", crateNumber)
					.set("src", "/emu/emuDCS/FEDApps/images/plus.png") << std::endl;
				*out << cgicc::span("FIFOs")
					.set("class", "fifos_open_close pointer")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::div() << std::endl;
					
				std::vector<FIFO *> fifos = (*iDCC)->getFIFOs();
				for (std::vector<FIFO *>::iterator iFIFO = fifos.begin(); iFIFO != fifos.end(); ++iFIFO) {
					
					std::ostringstream sFIFONumber;
					sFIFONumber << (*iFIFO)->getNumber();
					std::string fifoNumber = sFIFONumber.str();
					
					*out << cgicc::table()
						.set("id", "config_fifo_table_" + crateNumber + "_" + dccNumber + "_" + fifoNumber)
						.set("class", "tier5 fifo_table hidden")
						.set("crates_hidden", "1")
						.set("crate_hidden", "1")
						.set("dccs_hidden", "1")
						.set("fifos_hidden", "1")
						.set("crate", crateNumber)
						.set("fmmid", dccNumber)
						.set("fifo", fifoNumber) << std::endl;
					*out << cgicc::tr() << std::endl;
					*out << cgicc::td("FIFO " + fifoNumber)
						.set("class", "bold fifo_name")
						.set("crate", crateNumber)
						.set("fmmid", dccNumber)
						.set("fifo", fifoNumber) << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::button()
						.set("class", "delete_fifo")
						.set("crate", crateNumber)
						.set("fmmid", dccNumber)
						.set("fifo", fifoNumber);
					*out << cgicc::img()
						.set("class", "icon")
						.set("src", "/emu/emuDCS/FEDApps/images/list-remove.png");
					*out << "Delete FIFO" << std::endl;
					*out << cgicc::button() << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::tr() << std::endl;
					*out << cgicc::tr()
						.set("id", "config_fifo_row_number_" + crateNumber + "_" + dccNumber + "_" + fifoNumber)
						.set("class", "hidden")
						.set("crates_hidden", "1")
						.set("crate_hidden", "1")
						.set("dccs_hidden", "1")
						.set("fifos_hidden", "1")
						.set("fmmid", dccNumber)
						.set("crate", crateNumber)
						.set("fifo", fifoNumber) << std::endl;
					*out << cgicc::td("Number") << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::input()
						.set("type", "text")
						.set("class", "input_fifo_number")
						.set("crate", crateNumber)
						.set("fmmid", dccNumber)
						.set("fifo", fifoNumber)
						.set("value", fifoNumber) << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::tr() << std::endl;
					*out << cgicc::tr()
						.set("id", "config_fifo_row_rui_" + crateNumber + "_" + dccNumber + "_" + fifoNumber)
						.set("class", "hidden")
						.set("crates_hidden", "1")
						.set("crate_hidden", "1")
						.set("dccs_hidden", "1")
						.set("fifos_hidden", "1")
						.set("fmmid", dccNumber)
						.set("crate", crateNumber)
						.set("fifo", fifoNumber) << std::endl;
					*out << cgicc::td("RUI") << std::endl;
					*out << cgicc::td() << std::endl;
					std::ostringstream rui;
					rui << (*iFIFO)->getRUI();
					*out << cgicc::input()
						.set("type", "text")
						.set("class", "input_fifo_rui")
						.set("crate", crateNumber)
						.set("fmmid", dccNumber)
						.set("fifo", fifoNumber)
						.set("value", rui.str()) << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::tr() << std::endl;
					*out << cgicc::tr()
						.set("id", "config_fifo_row_inuse_" + crateNumber + "_" + dccNumber + "_" + fifoNumber)
						.set("class", "hidden")
						.set("crates_hidden", "1")
						.set("crate_hidden", "1")
						.set("dccs_hidden", "1")
						.set("fifos_hidden", "1")
						.set("fmmid", dccNumber)
						.set("crate", crateNumber)
						.set("fifo", fifoNumber) << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::label("In use")
						.set("for", "input_fifo_used_" + crateNumber + "_" + dccNumber + "_" + fifoNumber) << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::td() << std::endl;
					cgicc::input useInput;
					useInput.set("type", "checkbox")
						.set("id", "input_fifo_used_" + crateNumber + "_" + dccNumber + "_" + fifoNumber)
						.set("class", "input_fifo_used")
						.set("crate", crateNumber)
						.set("fmmid", dccNumber)
						.set("fifo", fifoNumber);
					if ((*iFIFO)->isUsed()) useInput.set("checked", "checked");
					*out << useInput << std::endl;
					*out << cgicc::td() << std::endl;
					*out << cgicc::tr() << std::endl;
					*out << cgicc::table() << std::endl;
					
				}
				
				*out << cgicc::div()
					.set("id", "config_fifo_add_" + crateNumber + "_" + dccNumber)
					.set("class", "tier5 hidden")
					.set("crates_hidden", "1")
					.set("crate_hidden", "1")
					.set("dccs_hidden", "1")
					.set("fifos_hidden", "1")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber) << std::endl;
				*out << cgicc::button()
					.set("class", "add_fifo")
					.set("crate", crateNumber)
					.set("fmmid", dccNumber);
				*out << cgicc::img()
					.set("class", "icon")
					.set("src", "/emu/emuDCS/FEDApps/images/list-add.png");
				*out << "Add FIFO" << std::endl;
				*out << cgicc::button() << std::endl;
				*out << cgicc::div() << std::endl;
			}
			
			*out << cgicc::div()
				.set("id", "config_dcc_add_" + crateNumber)
				.set("class", "tier4 hidden")
				.set("crates_hidden", "1")
				.set("crate_hidden", "1")
				.set("dccs_hidden", "1")
				.set("crate", crateNumber) << std::endl;
			*out << cgicc::button()
				.set("class", "add_dcc")
				.set("crate", crateNumber);
			*out << cgicc::img()
				.set("class", "icon")
				.set("src", "/emu/emuDCS/FEDApps/images/list-add.png");
			*out << "Add DCC" << std::endl;
			*out << cgicc::button() << std::endl;
			*out << cgicc::div() << std::endl;
			
		}
		
		*out << cgicc::div()
			.set("id", "config_crate_add")
			.set("class", "tier2 hidden")
			.set("crates_hidden", "1") << std::endl;
		*out << cgicc::button()
			.set("class", "add_crate");
		*out << cgicc::img()
			.set("class", "icon")
			.set("src", "/emu/emuDCS/FEDApps/images/list-add.png");
		*out << "Add crate" << std::endl;
		*out << cgicc::button() << std::endl;
		*out << cgicc::div() << std::endl;
		
		
		*out << cgicc::button()
			.set("class", "right upload_to_db action_button")
			.set("id", "upload_to_db");
		*out << "Upload configuration to database";
		*out << cgicc::button() << std::endl;
		
		*out << cgicc::button()
			.set("class", "right write_xml action_button")
			.set("id", "write_xml");
		*out << "Write configuration to XML";
		*out << cgicc::button() << std::endl;
		
		*out << cgicc::fieldset() << std::endl;
	}
	
	*out << Footer() << std::endl;
	
}



void emu::fed::ConfigurationEditor::webGetDBKeys(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	// Need some header information to be able to return JSON
	if (cgi.getElement("debug") == cgi.getElements().end() || cgi["debug"]->getIntegerValue() != 1) {
		cgicc::HTTPResponseHeader jsonHeader("HTTP/1.1", 200, "OK");
		jsonHeader.addHeader("Content-type", "application/json");
		out->setHTTPResponseHeader(jsonHeader);
	}
	
	// Get the keys from the configurations table.
	SystemDBAgent agent(this);
	
	std::map<std::string, std::vector<std::pair<xdata::UnsignedInteger64, time_t> > > keyMap;
	
	// Make a JSON output object
	JSONSpirit::Object output;
	JSONSpirit::Array keySets;
	
	try {
		agent.connect(dbUsername_, dbPassword_);
		
		keyMap = agent.getAllKeys();
		
	} catch (emu::fed::exception::DBException &e) {
		// Signal the user that there has been an error
		JSONSpirit::Object systemObject;
		JSONSpirit::Array keyArray;
		keyArray.push_back("Exception loading keys");
		systemObject.push_back(JSONSpirit::Pair("name", "Exception loading keys"));
		systemObject.push_back(JSONSpirit::Pair("keys", keyArray));
		systemObject.push_back(JSONSpirit::Pair("error", e.what()));
		keySets.push_back(systemObject);
	}
	
	for (std::map<std::string, std::vector<std::pair<xdata::UnsignedInteger64, time_t> > >::iterator iPair = keyMap.begin(); iPair != keyMap.end(); ++iPair) {
		
		JSONSpirit::Object systemObject;
		JSONSpirit::Array keyArray;
		
		std::vector<std::pair<xdata::UnsignedInteger64, time_t> > keys = iPair->second;
		for (std::vector<std::pair<xdata::UnsignedInteger64, time_t> >::iterator iKey = keys.begin(); iKey != keys.end(); ++iKey) {
			keyArray.push_back(iKey->first.toString());
		}
		
		systemObject.push_back(JSONSpirit::Pair("name", iPair->first));
		systemObject.push_back(JSONSpirit::Pair("keys", keyArray));
		keySets.push_back(systemObject);
	}
	
	output.push_back(JSONSpirit::Pair("systems", keySets));
	
	*out << JSONSpirit::write(output);
}



void emu::fed::ConfigurationEditor::webUploadFile(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	cgicc::const_file_iterator iFile = cgi.getFile("xmlFile");
	if (iFile == cgi.getFiles().end()) {
		// ERROR!
		LOG4CPLUS_ERROR(getApplicationLogger(), "Error uploading file");
		return;
	} else {
		std::ofstream tempFile("/tmp/config_fed_upload.xml");
		if (tempFile.good()) {
			iFile->writeToStream(tempFile);
			tempFile.close();
			LOG4CPLUS_INFO(getApplicationLogger(), "Successfully uploaded file " << iFile->getName());
		} else {
			// ERROR!
			if (tempFile.is_open()) tempFile.close();
			LOG4CPLUS_ERROR(getApplicationLogger(), "Error opening local file /tmp/config_fed_upload.xml for writing");
			return;
		}
	}
	
	// Parse the XML file and build crates properly
	XMLConfigurator configurator("/tmp/config_fed_upload.xml");
	
	try {
		crateVector_ = configurator.setupCrates(true);
		systemName_ = configurator.getSystemName();
		timeStamp_ = configurator.getTimeStamp();
		dbKey_ = 0;
	} catch (emu::fed::exception::Exception &e) {
		std::ostringstream error;
		error << "Unable to create FED objects by parsing file /tmp/config_fed_upload.xml: " << e.what();
		LOG4CPLUS_ERROR(getApplicationLogger(), error.str());
		return;
	}

}



void emu::fed::ConfigurationEditor::webLoadFromDB(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	if (cgi.getElement("key") != cgi.getElements().end()) {
		dbKey_ = cgi["key"]->getIntegerValue();
	} else {
		// Unable to load key.
		// TODO report error via JSON
		std::ostringstream error;
		error << "Unable to find parameter 'key' in POST data";
		LOG4CPLUS_ERROR(getApplicationLogger(), error.str());
		return;
	}
	
	// Get the configuration from the DB
	DBConfigurator configurator(this, dbUsername_.toString(), dbPassword_.toString(), dbKey_);
	
	try {
		crateVector_ = configurator.setupCrates(true);
		systemName_ = configurator.getSystemName();
		timeStamp_ = configurator.getTimeStamp();
	} catch (emu::fed::exception::Exception &e) {
		std::ostringstream error;
		error << "Unable to create FED objects by loading key " << dbKey_.toString() << ": " << e.what();
		LOG4CPLUS_ERROR(getApplicationLogger(), error.str());
		return;
	}
}



void emu::fed::ConfigurationEditor::webCreateNew(xgi::Input *in, xgi::Output *out)
{

	crateVector_.clear();
	dbKey_ = 0;
	systemName_ = "new configuration";
	timeStamp_ = time(NULL);
	
	// TODO respond via JSON?
	return;

}



void emu::fed::ConfigurationEditor::webWriteXML(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	try {
		std::string output = XMLConfigurator::makeXML(crateVector_, systemName_);
		
		// Need some header information to be able to return JSON
		if (cgi.getElement("debug") == cgi.getElements().end() || cgi["debug"]->getIntegerValue() != 1) {
			cgicc::HTTPResponseHeader xmlHeader("HTTP/1.1", 200, "OK");
			xmlHeader.addHeader("Content-Type", "text/xml");
			std::ostringstream attachment;
			attachment << "attachment; filename=fed-system-" << toolbox::escape(systemName_ == "" ? "unnamed" : systemName_.toString()) << "-" << toolbox::TimeVal(timeStamp_).toString(toolbox::TimeVal::gmt) << ".xml";
			xmlHeader.addHeader("Content-Disposition", attachment.str());
			out->setHTTPResponseHeader(xmlHeader);
		}
		
		*out << output;
		
	} catch (emu::fed::exception::Exception &e) {
		
		*out << printException(e);
		
	}
}



void emu::fed::ConfigurationEditor::webBuildSystem(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	// Need some header information to be able to return JSON
	if (cgi.getElement("debug") == cgi.getElements().end() || cgi["debug"]->getIntegerValue() != 1) {
		cgicc::HTTPResponseHeader jsonHeader("HTTP/1.1", 200, "OK");
		jsonHeader.addHeader("Content-type", "application/json");
		out->setHTTPResponseHeader(jsonHeader);
	}
	
	// Wipe out the old configuration
	crateVector_.clear();
	
	// Make JSON output
	JSONSpirit::Object output;
	
	// Set our new system name and time stamp
	if (cgi.getElement("systemName") != cgi.getElements().end()) {
		systemName_ = cgi["systemName"]->getValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find systemName in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	if (cgi.getElement("key") != cgi.getElements().end()) {
		dbKey_ = cgi["key"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find key in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Set time to now
	timeStamp_ = time(NULL);
	
	*out << JSONSpirit::write(output);
}



void emu::fed::ConfigurationEditor::webBuildCrate(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	// Need some header information to be able to return JSON
	if (cgi.getElement("debug") == cgi.getElements().end() || cgi["debug"]->getIntegerValue() != 1) {
		cgicc::HTTPResponseHeader jsonHeader("HTTP/1.1", 200, "OK");
		jsonHeader.addHeader("Content-type", "application/json");
		out->setHTTPResponseHeader(jsonHeader);
	}
	
	// Make JSON output
	JSONSpirit::Object output;
	
	// Get the fake crate and return it
	if (cgi.getElement("fakeCrate") != cgi.getElements().end()) {
		output.push_back(JSONSpirit::Pair("fakeCrate", cgi["fakeCrate"]->getValue()));
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find fakeCrate in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Make a crate with this number
	unsigned int crateNumber = 0;
	
	if (cgi.getElement("number") != cgi.getElements().end()) {
		crateNumber = cgi["number"]->getIntegerValue();
		output.push_back(JSONSpirit::Pair("crate", (int) crateNumber));
		crateVector_.push_back(new Crate(crateNumber));
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find crate number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	*out << JSONSpirit::write(output);
}



void emu::fed::ConfigurationEditor::webBuildController(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	// Need some header information to be able to return JSON
	if (cgi.getElement("debug") == cgi.getElements().end() || cgi["debug"]->getIntegerValue() != 1) {
		cgicc::HTTPResponseHeader jsonHeader("HTTP/1.1", 200, "OK");
		jsonHeader.addHeader("Content-type", "application/json");
		out->setHTTPResponseHeader(jsonHeader);
	}
	
	// Make JSON output
	JSONSpirit::Object output;
	
	// Get the fake crate and return it
	if (cgi.getElement("fakeCrate") != cgi.getElements().end()) {
		output.push_back(JSONSpirit::Pair("fakeCrate", cgi["fakeCrate"]->getValue()));
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find fakeCrate in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get the crate
	unsigned int crateNumber = 0;
	
	if (cgi.getElement("crate") != cgi.getElements().end()) {
		crateNumber = cgi["crate"]->getIntegerValue();
		output.push_back(JSONSpirit::Pair("crate", (int) crateNumber));
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find crate number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Find the crate
	Crate *myCrate = NULL;
	
	for (std::vector<Crate *>::iterator iCrate = crateVector_.begin(); iCrate != crateVector_.end(); iCrate++) {
		if ((*iCrate)->getNumber() == crateNumber) {
			myCrate = (*iCrate);
			break;
		}
	}
	if (myCrate == NULL) {
		std::ostringstream error;
		error << "Unable to find crate matching number " << crateNumber;
		output.push_back(JSONSpirit::Pair("error", error.str()));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get the device and link
	unsigned int device = 0;
	unsigned int link = 0;
	
	if (cgi.getElement("device") != cgi.getElements().end()) {
		device = cgi["device"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find device number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	if (cgi.getElement("link") != cgi.getElements().end()) {
		link = cgi["link"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find link number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	try {
		myCrate->setController(new VMEController(device, link, true));
	} catch (emu::fed::exception::Exception &e) {
		std::ostringstream error;
		error << "Unable to build controller object: " << e.what();
		output.push_back(JSONSpirit::Pair("error", error.str()));
		*out << JSONSpirit::write(output);
		return;
	}
	
	*out << JSONSpirit::write(output);
}



void emu::fed::ConfigurationEditor::webBuildDDU(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	// Need some header information to be able to return JSON
	if (cgi.getElement("debug") == cgi.getElements().end() || cgi["debug"]->getIntegerValue() != 1) {
		cgicc::HTTPResponseHeader jsonHeader("HTTP/1.1", 200, "OK");
		jsonHeader.addHeader("Content-type", "application/json");
		out->setHTTPResponseHeader(jsonHeader);
	}
	
	// Make JSON output
	JSONSpirit::Object output;
	
	// Get the fake crate and return it
	if (cgi.getElement("fakeCrate") != cgi.getElements().end()) {
		output.push_back(JSONSpirit::Pair("fakeCrate", cgi["fakeCrate"]->getValue()));
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find fakeCrate in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get the crate
	unsigned int crateNumber = 0;
	
	if (cgi.getElement("crate") != cgi.getElements().end()) {
		crateNumber = cgi["crate"]->getIntegerValue();
		output.push_back(JSONSpirit::Pair("crate", (int) crateNumber));
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find crate number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Find the crate
	Crate *myCrate = NULL;
	
	for (std::vector<Crate *>::iterator iCrate = crateVector_.begin(); iCrate != crateVector_.end(); iCrate++) {
		if ((*iCrate)->getNumber() == crateNumber) {
			myCrate = (*iCrate);
			break;
		}
	}
	if (myCrate == NULL) {
		std::ostringstream error;
		error << "Unable to find crate matching number " << crateNumber;
		output.push_back(JSONSpirit::Pair("error", error.str()));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get slot
	unsigned int slot = 0;
	if (cgi.getElement("input_ddu_slot") != cgi.getElements().end()) {
		slot = cgi["input_ddu_slot"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find slot number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	DDU *myDDU = new DDU(slot, true);
	
	// Get RUI
	if (cgi.getElement("input_ddu_rui") != cgi.getElements().end()) {
		unsigned int rui = cgi["input_ddu_rui"]->getIntegerValue();
		output.push_back(JSONSpirit::Pair("rui", (int) rui));
		myDDU->setRUI(rui);
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find RUI number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get the fake RUI and return it
	if (cgi.getElement("fakeRUI") != cgi.getElements().end()) {
		output.push_back(JSONSpirit::Pair("fakeRUI", cgi["fakeRUI"]->getValue()));
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find fakeRUI in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get FMMID
	if (cgi.getElement("input_ddu_fmm_id") != cgi.getElements().end()) {
		myDDU->setFMMID(cgi["input_ddu_fmm_id"]->getIntegerValue());
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find FMM ID number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get GbEPrescale
	if (cgi.getElement("input_ddu_gbe_prescale") != cgi.getElements().end()) {
		myDDU->setGbEPrescale(cgi["input_ddu_gbe_prescale"]->getIntegerValue());
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find GbEPrescale in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get options
	uint32_t killfiber = 0;
	if (cgi.getElement("input_ddu_force_checks") != cgi.getElements().end()) {
		killfiber |= (cgi["input_ddu_force_checks"]->getIntegerValue()) << 15;
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find force checks checkbox in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	if (cgi.getElement("input_ddu_force_alct") != cgi.getElements().end()) {
		killfiber |= (cgi["input_ddu_force_alct"]->getIntegerValue()) << 16;
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find force ALCT checkbox in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	if (cgi.getElement("input_ddu_force_tmb") != cgi.getElements().end()) {
		killfiber |= (cgi["input_ddu_force_tmb"]->getIntegerValue()) << 17;
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find force TMB checkbox in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	if (cgi.getElement("input_ddu_force_cfeb") != cgi.getElements().end()) {
		killfiber |= (cgi["input_ddu_force_cfeb"]->getIntegerValue()) << 18;
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find force CFEB checkbox in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	if (cgi.getElement("input_ddu_force_dmb") != cgi.getElements().end()) {
		killfiber |= (cgi["input_ddu_force_dmb"]->getIntegerValue()) << 19;
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find force DMB checkbox in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	myDDU->setKillFiber(killfiber);
	
	myCrate->addBoard(myDDU);
	
	*out << JSONSpirit::write(output);
	
}



void emu::fed::ConfigurationEditor::webBuildFiber(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	// Need some header information to be able to return JSON
	if (cgi.getElement("debug") == cgi.getElements().end() || cgi["debug"]->getIntegerValue() != 1) {
		cgicc::HTTPResponseHeader jsonHeader("HTTP/1.1", 200, "OK");
		jsonHeader.addHeader("Content-type", "application/json");
		out->setHTTPResponseHeader(jsonHeader);
	}
	
	// Make JSON output
	JSONSpirit::Object output;
	
	// Get the crate
	unsigned int crateNumber = 0;
	
	if (cgi.getElement("crate") != cgi.getElements().end()) {
		crateNumber = cgi["crate"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find crate number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Find the crate
	Crate *myCrate = NULL;
	
	for (std::vector<Crate *>::iterator iCrate = crateVector_.begin(); iCrate != crateVector_.end(); ++iCrate) {
		if ((*iCrate)->getNumber() == crateNumber) {
			myCrate = (*iCrate);
			break;
		}
	}
	if (myCrate == NULL) {
		std::ostringstream error;
		error << "Unable to find crate matching number " << crateNumber;
		output.push_back(JSONSpirit::Pair("error", error.str()));
		*out << JSONSpirit::write(output);
		return;
	}

	// Get RUI
	unsigned int rui;
	if (cgi.getElement("rui") != cgi.getElements().end()) {
		rui = cgi["rui"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find RUI number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Find the DDU
	DDU *myDDU = NULL;
	
	std::vector<DDU *> dduVector = myCrate->getDDUs();
	for (std::vector<DDU *>::iterator iDDU = dduVector.begin(); iDDU != dduVector.end(); ++iDDU) {
		if ((*iDDU)->getRUI() == rui) {
			myDDU = (*iDDU);
			break;
		}
	}
	if (myDDU == NULL) {
		std::ostringstream error;
		error << "Unable to find DDU matching RUI number " << rui;
		output.push_back(JSONSpirit::Pair("error", error.str()));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get number
	unsigned int fiberNumber = 0;
	if (cgi.getElement("input_fiber_number") != cgi.getElements().end()) {
		fiberNumber = cgi["input_fiber_number"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find fiber number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get name
	std::string name = "";
	if (cgi.getElement("input_fiber_name") != cgi.getElements().end()) {
		name = cgi["input_fiber_name"]->getValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find fiber name in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get killed
	bool killed = false;
	if (cgi.getElement("input_fiber_killed") != cgi.getElements().end()) {
		killed = cgi["input_fiber_killed"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find killed checkbox in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	std::string endcap = "?";
	unsigned int station = 0;
	unsigned int ring = 0;
	unsigned int number = 0;
	
	// Check normal station name first
	if (sscanf(name.c_str(), "%*c%1u/%1u/%02u", &station, &ring, &number) == 3) {
		endcap = "-";
		// CGICC does not understand that %2B is a plus-sign, so check for that here
	} else if (sscanf(name.c_str(), "%1u/%1u/%02u", &station, &ring, &number) == 3) {
		endcap = "+";
		// Else it's probably an SP, so check that
	} else if (sscanf(name.c_str(), "SP%02u", &number) == 1) {
		endcap = (number <= 6) ? "+" : "-";
	}
	
	myDDU->addFiber(new Fiber(fiberNumber, endcap, station, ring, number, killed));
	
	*out << JSONSpirit::write(output);
}



void emu::fed::ConfigurationEditor::webBuildDCC(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	// Need some header information to be able to return JSON
	if (cgi.getElement("debug") == cgi.getElements().end() || cgi["debug"]->getIntegerValue() != 1) {
		cgicc::HTTPResponseHeader jsonHeader("HTTP/1.1", 200, "OK");
		jsonHeader.addHeader("Content-type", "application/json");
		out->setHTTPResponseHeader(jsonHeader);
	}
	
	// Make JSON output
	JSONSpirit::Object output;
	
	// Get the fake crate and return it
	if (cgi.getElement("fakeCrate") != cgi.getElements().end()) {
		output.push_back(JSONSpirit::Pair("fakeCrate", cgi["fakeCrate"]->getValue()));
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find fakeCrate in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get the crate
	unsigned int crateNumber = 0;
	
	if (cgi.getElement("crate") != cgi.getElements().end()) {
		crateNumber = cgi["crate"]->getIntegerValue();
		output.push_back(JSONSpirit::Pair("crate", (int) crateNumber));
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find crate number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Find the crate
	Crate *myCrate = NULL;
	
	for (std::vector<Crate *>::iterator iCrate = crateVector_.begin(); iCrate != crateVector_.end(); iCrate++) {
		if ((*iCrate)->getNumber() == crateNumber) {
			myCrate = (*iCrate);
			break;
		}
	}
	if (myCrate == NULL) {
		std::ostringstream error;
		error << "Unable to find crate matching number " << crateNumber;
		output.push_back(JSONSpirit::Pair("error", error.str()));
		*out << JSONSpirit::write(output);
		return;
	}

	// Get slot
	unsigned int slot = 0;
	if (cgi.getElement("input_dcc_slot") != cgi.getElements().end()) {
		slot = cgi["input_dcc_slot"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find slot number in header"));
		*out << JSONSpirit::write(output);
		return;
	}

	DCC *myDCC = new DCC(slot, true);

	// Get FMMID
	if (cgi.getElement("input_dcc_fmm_id") != cgi.getElements().end()) {
		unsigned int fmmid = cgi["input_dcc_fmm_id"]->getIntegerValue();
		output.push_back(JSONSpirit::Pair("fmmid", (int) fmmid));
		myDCC->setFMMID(fmmid);
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find FMM ID number in header"));
		*out << JSONSpirit::write(output);
		return;
	}

	// Get the fake FMMID and return it
	if (cgi.getElement("fakeFMMID") != cgi.getElements().end()) {
		output.push_back(JSONSpirit::Pair("fakeFMMID", cgi["fakeFMMID"]->getValue()));
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find fakeFMMID in header"));
		*out << JSONSpirit::write(output);
		return;
	}

	// Get SLink1 ID
	if (cgi.getElement("input_dcc_slink1") != cgi.getElements().end()) {
		myDCC->setSLinkID(1, cgi["input_dcc_slink1"]->getIntegerValue());
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find SLink 1 ID number in header"));
		*out << JSONSpirit::write(output);
		return;
	}

	// Get SLink2 ID
	if (cgi.getElement("input_dcc_slink2") != cgi.getElements().end()) {
		myDCC->setSLinkID(2, cgi["input_dcc_slink2"]->getIntegerValue());
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find SLink 2 ID number in header"));
		*out << JSONSpirit::write(output);
		return;
	}

	// Get options
	uint16_t softsw = 0;
	if (cgi.getElement("input_dcc_sw_switch") != cgi.getElements().end()) {
		softsw |= (cgi["input_dcc_sw_switch"]->getIntegerValue() ? 0x200 : 0);
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find software switch checkbox in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	if (cgi.getElement("input_dcc_ignore_ttc") != cgi.getElements().end()) {
		softsw |= (cgi["input_dcc_ignore_ttc"]->getIntegerValue() ? 0x1000 : 0);
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find ignore TTC checkbox in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	if (cgi.getElement("input_dcc_ignore_backpressure") != cgi.getElements().end() && cgi.getElement("input_dcc_ignore_slink") != cgi.getElements().end()) {
		if (cgi["input_dcc_ignore_slink"]->getIntegerValue()) softsw |= 0x4000;
		else if (cgi["input_dcc_ignore_backpressure"]->getIntegerValue()) softsw |= 0x2000;
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find ignore slink/backpressure checkboxes in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	if (cgi.getElement("input_dcc_sw4") != cgi.getElements().end()) {
		softsw |= (cgi["input_dcc_sw4"]->getIntegerValue() ? 0x10 : 0);
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find SW bit 4 checkbox in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	if (cgi.getElement("input_dcc_sw5") != cgi.getElements().end()) {
		softsw |= (cgi["input_dcc_sw5"]->getIntegerValue() ? 0x20 : 0);
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find SW bit 5 checkbox in header"));
		*out << JSONSpirit::write(output);
		return;
	}

	myDCC->setSoftwareSwitch(softsw);
	
	myCrate->addBoard(myDCC);
	
	*out << JSONSpirit::write(output);
}



void emu::fed::ConfigurationEditor::webBuildFIFO(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	// Need some header information to be able to return JSON
	if (cgi.getElement("debug") == cgi.getElements().end() || cgi["debug"]->getIntegerValue() != 1) {
		cgicc::HTTPResponseHeader jsonHeader("HTTP/1.1", 200, "OK");
		jsonHeader.addHeader("Content-type", "application/json");
		out->setHTTPResponseHeader(jsonHeader);
	}
	
	// Make JSON output
	JSONSpirit::Object output;
	
	// Get the crate
	unsigned int crateNumber = 0;
	
	if (cgi.getElement("crate") != cgi.getElements().end()) {
		crateNumber = cgi["crate"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find crate number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Find the crate
	Crate *myCrate = NULL;
	
	for (std::vector<Crate *>::iterator iCrate = crateVector_.begin(); iCrate != crateVector_.end(); ++iCrate) {
		if ((*iCrate)->getNumber() == crateNumber) {
			myCrate = (*iCrate);
			break;
		}
	}
	if (myCrate == NULL) {
		std::ostringstream error;
		error << "Unable to find crate matching number " << crateNumber;
		output.push_back(JSONSpirit::Pair("error", error.str()));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get FMMID
	unsigned int fmmid;
	if (cgi.getElement("fmmid") != cgi.getElements().end()) {
		fmmid = cgi["fmmid"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find FMM ID number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Find the DCC
	DCC *myDCC = NULL;
	
	std::vector<DCC *> dccVector = myCrate->getDCCs();
	for (std::vector<DCC *>::iterator iDCC = dccVector.begin(); iDCC != dccVector.end(); ++iDCC) {
		if ((*iDCC)->getFMMID() == fmmid) {
			myDCC = (*iDCC);
			break;
		}
	}
	if (myDCC == NULL) {
		std::ostringstream error;
		error << "Unable to find DCC matching FMM ID number " << fmmid;
		output.push_back(JSONSpirit::Pair("error", error.str()));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get number
	unsigned int fifoNumber = 0;
	if (cgi.getElement("input_fifo_number") != cgi.getElements().end()) {
		fifoNumber = cgi["input_fifo_number"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find FIFO number in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get RUI
	unsigned int rui = 0;
	if (cgi.getElement("input_fifo_rui") != cgi.getElements().end()) {
		rui = cgi["input_fifo_rui"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find FIFO RUI in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	// Get used
	bool used = false;
	if (cgi.getElement("input_fifo_used") != cgi.getElements().end()) {
		used = cgi["input_fifo_used"]->getIntegerValue();
	} else {
		output.push_back(JSONSpirit::Pair("error", "Cannot find used checkbox in header"));
		*out << JSONSpirit::write(output);
		return;
	}
	
	myDCC->addFIFO(new FIFO(fifoNumber, rui, used));
	
	*out << JSONSpirit::write(output);
}



void emu::fed::ConfigurationEditor::webUploadToDB(xgi::Input *in, xgi::Output *out)
{
	cgicc::Cgicc cgi(in);
	
	// Need some header information to be able to return JSON
	if (cgi.getElement("debug") == cgi.getElements().end() || cgi["debug"]->getIntegerValue() != 1) {
		cgicc::HTTPResponseHeader xmlHeader("HTTP/1.1", 200, "OK");
		xmlHeader.addHeader("Content-Type", "text/xml");
		std::ostringstream attachment;
		attachment << "attachment; filename=fed-system-" << toolbox::escape(systemName_ == "" ? "unnamed" : systemName_.toString()) << "-" << toolbox::TimeVal(timeStamp_).toString(toolbox::TimeVal::gmt) << ".xml";
		xmlHeader.addHeader("Content-Disposition", attachment.str());
		out->setHTTPResponseHeader(xmlHeader);
	}
	
	DBConfigurator configurator(this, dbUsername_, dbPassword_, dbKey_);
	
	JSONSpirit::Object output;
	
	try {
		configurator.uploadToDB(crateVector_, systemName_);
		
		// Need some header information to be able to return JSON
		if (cgi.getElement("debug") == cgi.getElements().end() || cgi["debug"]->getIntegerValue() != 1) {
			cgicc::HTTPResponseHeader jsonHeader("HTTP/1.1", 200, "OK");
			jsonHeader.addHeader("Content-type", "application/json");
			out->setHTTPResponseHeader(jsonHeader);
		}
		
		output.push_back(JSONSpirit::Pair("systemName", systemName_.toString()));
		output.push_back(JSONSpirit::Pair("key", dbKey_.toString()));
		
	} catch (emu::fed::exception::ConfigurationException &e) {
		
		output.push_back(JSONSpirit::Pair("error", e.what()));
		
	}
	
	*out << JSONSpirit::write(output);
}
