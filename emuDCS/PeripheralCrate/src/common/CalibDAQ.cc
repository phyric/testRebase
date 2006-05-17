//-----------------------------------------------------------------------
// $Id: CalibDAQ.cc,v 2.20 2006/05/17 14:16:44 mey Exp $
// $Log: CalibDAQ.cc,v $
// Revision 2.20  2006/05/17 14:16:44  mey
// Update
//
// Revision 2.19  2006/04/28 13:41:17  mey
// Update
//
// Revision 2.18  2006/04/27 18:46:04  mey
// UPdate
//
// Revision 2.17  2006/04/25 14:50:45  mey
// Update
//
// Revision 2.16  2006/04/25 13:25:19  mey
// Update
//
// Revision 2.15  2006/04/24 14:57:21  mey
// Update
//
// Revision 2.14  2006/04/20 15:33:54  mey
// Update
//
// Revision 2.13  2006/04/19 15:19:06  mey
// Update
//
// Revision 2.12  2006/04/18 08:17:29  mey
// UPdate
//
// Revision 2.11  2006/04/11 15:27:42  mey
// Update
//
// Revision 2.10  2006/03/30 13:55:38  mey
// Update
//
// Revision 2.9  2006/03/27 09:51:42  mey
// UPdate
//
// Revision 2.8  2006/03/27 09:22:36  mey
// Update
//
// Revision 2.7  2006/03/24 16:40:36  mey
// Update
//
// Revision 2.6  2006/03/24 14:35:04  mey
// Update
//
// Revision 2.5  2006/03/07 09:22:58  mey
// Update
//
// Revision 2.4  2006/02/20 13:31:14  mey
// Update
//
// Revision 2.3  2006/02/15 22:39:57  mey
// UPdate
//
// Revision 2.2  2006/01/18 19:38:16  mey
// Fixed bugs
//
// Revision 2.1  2006/01/11 17:00:01  mey
// Update
//
// Revision 2.0  2005/06/06 10:01:03  geurts
// calibration routines by Alex Tumanov and Jason Gilmore
//
//
//-----------------------------------------------------------------------
#include <iostream>
#include <unistd.h> // for sleep
#include <string>
#include "CalibDAQ.h"
#include "Crate.h"
#include "DAQMB.h"
#include "CCB.h"
#include "JTAG_constants.h"
#include "ChamberUtilities.h"

void CalibDAQ::loadConstants() {

  std::vector<Crate*> myCrates = theSelector.crates();

  for(unsigned i = 0; i < myCrates.size(); ++i) {
    loadConstants(myCrates[i]);
  }

}


void CalibDAQ::loadConstants(Crate * crate) {
  //
  crate->configure();
  //
  std::vector<DAQMB*> myDmbs = theSelector.daqmbs(crate);
  for(unsigned i =0; i < myDmbs.size(); ++i) {
    myDmbs[i]->restoreCFEBIdle();
    myDmbs[i]->restoreMotherboardIdle();
    myDmbs[i]->configure();
  }

}
//
void CalibDAQ::rateTest() {
  //
  int chip,ch,brd, nstrip;
  int counter = 0;
  float dac;
  //
  std::vector<Crate*> myCrates = theSelector.crates();
  //
  for(unsigned j = 0; j < myCrates.size(); ++j) {
    //
    CCB * ccb = myCrates[j]->ccb();
    std::vector<DAQMB*> myDmbs = theSelector.daqmbs(myCrates[j]);
    std::vector<TMB*> myTmbs   = theSelector.tmbs(myCrates[j]);
    //
    for (unsigned i=0; i<myTmbs.size(); i++) {
      myTmbs[i]->DisableCLCTInputs();
      std::cout << "Disabling inputs for slot " << myTmbs[i]->slot() << std::endl;
      myTmbs[i]->DisableALCTInputs();
    }
    //
    for (nstrip=0;nstrip<16;nstrip++) {  
      for (int j=0; j<10; j++) {
	dac=0.2+0.2*j;
	//
	for(unsigned i =0; i < myDmbs.size(); ++i) {
	  myDmbs[i]->set_cal_dac(dac,dac);
	}
	for(unsigned i =0; i < myDmbs.size(); ++i) {    
	  for(brd=0;brd<5;brd++){
	    for(chip=0;chip<6;chip++){
	      for(ch=0;ch<16;ch++){
		myDmbs[i]->shift_array[brd][chip][ch]=NORM_RUN;
	      }
	      myDmbs[i]->shift_array[brd][chip][nstrip]=EXT_CAP;
	    }
	  }
	  myDmbs[i]->buck_shift();
	}
	//
	::usleep(100000);
	//
	for (int tries=0;tries<20; tries++){
          counter++;
	  std::cout << "dac = " << dac <<
	    "  strip = " << nstrip << 
	    "  try = " << tries << 
	    "  event  = " << counter << std::endl;
	  ::usleep(200000);
	  ccb->pulse(1, 0xff);//pulse all dmbs in this crate
	  ::usleep(200000);
	}
	//
      } //end of loop by strips
    } //endof loop by amplitudes
  }//end of loop by crates
}
//
void CalibDAQ::pulseAllWires(){
  //
  std::vector<Crate*> myCrates = theSelector.crates();
  //
  for(unsigned j = 0; j < myCrates.size(); ++j) {
    //
    (myCrates[j]->chamberUtilsMatch())[0].CCBStartTrigger();
    usleep(100);
    //
    std::vector<TMB*>   myTmbs   = theSelector.tmbs(myCrates[j]);
    //
    for (unsigned i=0; i<myTmbs.size(); i++) {
      myTmbs[i]->DisableCLCTInputs();
      std::cout << "Disabling inputs for slot " << myTmbs[i]->slot() << std::endl;
    }
    //
    std::vector<ChamberUtilities> utils = (myCrates[j]->chamberUtilsMatch()) ;
    //
    for (int i = 0; i < utils.size() ; i++ ) {
      //
      ::usleep(200000);
      utils[i].PulseAllWires();
      ::usleep(200000);
      //
    }
    //
  }
  //
}
//
void CalibDAQ::pulseComparatorPulse(){
  //
  for (int thresh=0; thresh<35; thresh++) {
    for (int i=0;i<16;i++) {  
      injectComparator(25, i, 0.15, 1000,thresh*0.003+0.013);
    }
  }
  //
  for (int thresh=0; thresh<35; thresh++) {
    for (int i=0;i<16;i++) {  
      injectComparator(25, i, 0.35, 1000,thresh*0.003+0.049);
    }
  }
  //
  //
}
//
void CalibDAQ::pulseRandomWires(int delay){
  //
  std::vector<Crate*> myCrates = theSelector.crates();
  //
  for(unsigned j = 0; j < myCrates.size(); ++j) {
    //
    (myCrates[j]->chamberUtilsMatch())[0].CCBStartTrigger();
    usleep(100);
    //
    std::vector<TMB*>   myTmbs   = theSelector.tmbs(myCrates[j]);
    //
    for (unsigned i=0; i<myTmbs.size(); i++) {
      myTmbs[i]->DisableCLCTInputs();
      std::cout << "Disabling inputs for slot " << myTmbs[i]->slot() << std::endl;
    }
    //
    std::vector<ChamberUtilities> utils = (myCrates[j]->chamberUtilsMatch()) ;
    //
    for (int i = 0; i < utils.size() ; i++ ) {
      //
      utils[i].PulseRandomALCT(delay);
      //
    }
    //
  }
  //
}
//
void CalibDAQ::timeCFEB() { 
  //
  float dac;
  int counter=0;
  int nsleep = 100;  
  dac = 1.0;
  //
  for (int i=0;i<16;i++) {  
    for (int ntim=0;ntim<20;ntim++) {
      pulseAllDMBs(ntim, i, dac, nsleep);  
      counter++;
      std::cout << "dac = " << dac <<
	"  strip = " << i <<
	"  ntim = " << ntim <<
	"  event  = " << counter << std::endl;
    }
  }
  //
}
//
void CalibDAQ::CFEBSaturation() { 
  //
  float dac;
  int counter=0;
  int nsleep = 100;  
  //
  for (int nstrip=0;nstrip<16;nstrip++) {  
    for (int j=0; j<24; j++) {
      dac=0.2+0.2*j;
      for (int ntim=0;ntim<20;ntim++) {	  
	pulseAllDMBs(1, nstrip, dac, nsleep);  
	counter++;
	std::cout << "dac = " << dac <<
	  "  strip = " << nstrip <<
	  "  ntim = " << ntim <<
	  "  event  = " << counter << std::endl;
      }
    }
  }
  //
}
//
void CalibDAQ::pedestalCFEB() { 
  //
  int ntim = 1;
  float dac = 0.01;
  int nsleep(100);
  int counter = 0;
  int nevents = 1000;
  //
  //std::vector<Crate*> myCrates = theSelector.crates();
  //
  for (int events = 0; events<nevents; events++) {
    pulseAllDMBs(ntim, -1, dac, nsleep,0);  
    counter++;
    std::cout << "dac = " << dac <<
      "  event  = " << counter << std::endl;
  }
}  
//
void CalibDAQ::pulseAllDMBs(int ntim, int nstrip, float dac, int nsleep,int calType) { 
  //injects identical pulse to all dmbs
  //in all crates one crate at a time          
  //disable TMBs and ALCts
  //
  int chip,ch,brd;
  std::vector<Crate*> myCrates = theSelector.crates();
  //
  for(unsigned j = 0; j < myCrates.size(); ++j) {
    //
    (myCrates[j]->chamberUtilsMatch())[0].CCBStartTrigger();
    usleep(100);
    //
    CCB * ccb = myCrates[j]->ccb();
    std::vector<DAQMB*> myDmbs = theSelector.daqmbs(myCrates[j]);
    std::vector<TMB*>   myTmbs   = theSelector.tmbs(myCrates[j]);
    //
    for (unsigned i=0; i<myTmbs.size(); i++) {
      myTmbs[i]->DisableCLCTInputs();
      std::cout << "Disabling inputs for slot " << myTmbs[i]->slot() << std::endl;
      myTmbs[i]->DisableALCTInputs();
    }
    //
    for(unsigned i =0; i < myDmbs.size(); ++i) {
      //
      // set amplitude
      //
      myDmbs[i]->set_cal_dac(dac,dac);
      //myDmbs[i]->set_comp_thresh(thresh);
      //
      // set external pulser for strip # nstrip on all 6 chips
      for(brd=0;brd<5;brd++){
	for(chip=0;chip<6;chip++){
	  for(ch=0;ch<16;ch++){
	    myDmbs[i]->shift_array[brd][chip][ch]=NORM_RUN;
	  }
	  if ( nstrip != -1 ) myDmbs[i]->shift_array[brd][chip][nstrip]=EXT_CAP;
	}
      }
      //
      myDmbs[i]->buck_shift();
      //
      myDmbs[i]->set_cal_tim_pulse(ntim);   
      //
    }
    //
    ::usleep(100000);
    //
    std::cout << "Sending pulse" <<std::endl;
    if(calType==2){
      ccb->pedestal(1,0xff);
    } else {
      ccb->pulse(1, 0xff);//pulse all dmbs in this crate
    }
    //
    ::usleep(100000);
    //
    for(unsigned i =0; i < myDmbs.size(); ++i) {
      std::cout << "Slot " << myDmbs[i]->slot();
      myDmbs[i]->PrintCounters();
    }
    //
    std::cout <<"After"<<std::endl;
    for(unsigned i =0; i < myTmbs.size(); ++i) {
      myTmbs[i]->GetCounters();
      myTmbs[i]->PrintCounters();
      myTmbs[i]->ResetCounters();
      myTmbs[i]->DisableCLCTInputs();
    }
    //
  }
}
//
void CalibDAQ::injectComparator(int ntim, int nstrip, float dac, int nsleep, float thresh) { 
  //
  //injects identical pulse to all dmbs
  //in all crates one crate at a time          
  //
  int chip,ch,brd;
  std::vector<Crate*> myCrates = theSelector.crates();
  //
  for(unsigned j = 0; j < myCrates.size(); ++j) {
    //
    (myCrates[j]->chamberUtilsMatch())[0].CCBStartTrigger();
    usleep(100);
    //
    CCB * ccb = myCrates[j]->ccb();
    std::vector<DAQMB*> myDmbs = theSelector.daqmbs(myCrates[j]);
    std::vector<TMB*> myTmbs   = theSelector.tmbs(myCrates[j]);
    //
    for (unsigned i=0; i<myTmbs.size(); i++) {
      myTmbs[i]->DisableCLCTInputs();
      //myTmbs[i]->EnableInternalL1aSequencer();
      std::cout << "Disabling inputs for slot " << myTmbs[i]->slot() << std::endl;
      myTmbs[i]->DisableALCTInputs();
    }
    //
    for(unsigned i =0; i < myDmbs.size(); ++i) {
      //
      // set amplitude
      //
      myDmbs[i]->settrgsrc(0); //Disable out L1a generation
      myDmbs[i]->set_cal_dac(dac,dac);
      myDmbs[i]->set_comp_thresh(thresh);
      //
      // set external pulser for strip # nstrip on all 6 chips
      //for(brd=0;brd<5;brd++){
      //for(chip=0;chip<6;chip++){
      //  for(ch=0;ch<16;ch++){
      //    myDmbs[i]->shift_array[brd][chip][ch]=NORM_RUN;
      //  }
      //  if ( nstrip != -1 ) myDmbs[i]->shift_array[brd][chip][nstrip]=SMALL_CAP;
      //}
      //}
      //
      //myDmbs[i]->buck_shift();
      //
      int HalfStrip = nstrip ;
      //
      int hp[6] = {HalfStrip, HalfStrip, HalfStrip, HalfStrip, HalfStrip, HalfStrip};       
      //
      // Set the pattern
      //
      myDmbs[i]->trigsetx(hp);
      //
      //set timing
      //ntim is the same as pulse_delay initially set in xml configuration file
      //
      myDmbs[i]->set_cal_tim_pulse(ntim);   
      //
    }
    //
    ::usleep(1000);
    //
    for (unsigned i=0; i<myTmbs.size(); i++) {
      myTmbs[i]->EnableCLCTInputs(0x1f);
    }
    //
    usleep(1000);
    //
    //for(unsigned i =0; i<myDmbs.size(); ++i) {
    //myDmbs[i]->inject(1,0x4f);
    //}
    //
    std::cout << "Sending inject" <<std::endl;
    ccb->inject(1, 0xff);//pulse all dmbs in this crate
    //
    usleep(1000);
    //
    for(unsigned i =0; i < myDmbs.size(); ++i) {
      std::cout << "Slot " << myDmbs[i]->slot();
      myDmbs[i]->PrintCounters();
    }
    //
    std::cout <<"After"<<std::endl;
    for(unsigned i =0; i < myTmbs.size(); ++i) {
      myTmbs[i]->GetCounters();
      myTmbs[i]->PrintCounters();
      myTmbs[i]->ResetCounters();
      myTmbs[i]->DisableCLCTInputs();
    }
    //
  }
}
//
void CalibDAQ::FindL1aDelayALCT() { 
  //
  int counter[200][9];
  //
  for(int i=0; i<200;i++) for(int j=0;j<9;j++) counter[i][j] = 0;
  //
  std::vector<Crate*> myCrates = theSelector.crates();
  //
  for(int delay=100;delay<150;delay++){
    for(unsigned j = 0; j < myCrates.size(); ++j) {
      //
      std::cout << std::endl;
      //
      (myCrates[j]->chamberUtilsMatch())[0].CCBStartTrigger();
      usleep(100);
      //
      std::vector<DAQMB*> myDmbs = theSelector.daqmbs(myCrates[j]);
      std::vector<TMB*>   myTmbs = theSelector.tmbs(myCrates[j]);
      //
      pulseRandomWires(delay);
      //
      for(unsigned i =myDmbs.size(); i < myDmbs.size(); ++i) {
	std::cout << "Slot " << myDmbs[i]->slot();
	myDmbs[i]->PrintCounters();
	int mycounter = myDmbs[i]->GetAlctDavScope();
	if ( mycounter>0 ) counter[delay][i] = mycounter;
      }
      //
    }
  }
  //
  for(unsigned j = 0; j < myCrates.size(); ++j) {
    //
    std::vector<DAQMB*> myDmbs = theSelector.daqmbs(myCrates[j]);
    for(int delay=0; delay<200;delay++) {
      std::cout << delay << " " ;
      for(unsigned i =0; i < myDmbs.size(); ++i) std::cout << counter[delay][i] << " ";
      std::cout << std::endl;
    }
  }
  //
}
//
void CalibDAQ::FindL1aDelayComparator() { 
//injects identical pulse to all dmbs (EXT capacitors)
//in all crates one crate at a time          
  //
  int chip,ch,brd;
  int counter[200][9];
  //
  int ntim=25;
  float thresh = 0.1;
  float dac = 0.4;
  //
  for(int i=0; i<200;i++) for(int j=0;j<9;j++) counter[i][j] = 0;
  //
  std::vector<Crate*> myCrates = theSelector.crates();
  //
  for(int delay=100;delay<150;delay++){
    for(unsigned j = 0; j < myCrates.size(); ++j) {
      //
      (myCrates[j]->chamberUtilsMatch())[0].CCBStartTrigger();
      usleep(100);
      //
      CCB * ccb = myCrates[j]->ccb();
      std::vector<DAQMB*> myDmbs = theSelector.daqmbs(myCrates[j]);
      std::vector<TMB*> myTmbs   = theSelector.tmbs(myCrates[j]);
      //
      for (unsigned i=0; i<myTmbs.size(); i++) {
	myTmbs[i]->DisableCLCTInputs();
	myTmbs[i]->lvl1_delay(delay);
	myTmbs[i]->EnableInternalL1aSequencer();
	std::cout << "Disabling inputs for slot " << myTmbs[i]->slot() << std::endl;
	myTmbs[i]->DisableALCTInputs();
      }
      //
      for(unsigned i =0; i < myDmbs.size(); ++i) {
	//
	// set amplitude
	//
	myDmbs[i]->set_cal_dac(dac,dac);
	myDmbs[i]->set_comp_thresh(thresh);
	//
	//myDmbs[i]->set_comp_thresh(0.4);
	//
	// set external pulser for strip # nstrip on all 6 chips
	//for(brd=0;brd<5;brd++){
	//for(chip=0;chip<6;chip++){
	//  for(ch=0;ch<16;ch++){
	//    myDmbs[i]->shift_array[brd][chip][ch]=NORM_RUN;
	//  }
	//  if ( nstrip != -1 ) myDmbs[i]->shift_array[brd][chip][nstrip]=SMALL_CAP;
	//}
	//}
	//
	//myDmbs[i]->buck_shift();
	//
	int HalfStrip = 16;
	//
	int hp[6] = {HalfStrip, HalfStrip, HalfStrip, HalfStrip, HalfStrip, HalfStrip};       
	//
	// Set the pattern
	//
	myDmbs[i]->trigsetx(hp);
	//
	//set timing
	//ntim is the same as pulse_delay initially set in xml configuration file
	//
	myDmbs[i]->set_cal_tim_pulse(ntim);   
	//
      }
      //
      ::usleep(1000);
      //
      for (unsigned i=0; i<myTmbs.size(); i++) {
	myTmbs[i]->EnableCLCTInputs(0x1f);
      }
      //
      usleep(1000);
      //
      //for(unsigned i =0; i<myDmbs.size(); ++i) {
      //myDmbs[i]->inject(1,0x4f);
      //}
      //
      std::cout << "Sending inject" <<std::endl;
      ccb->inject(1, 0xff);//pulse all dmbs in this crate
      //
      usleep(1000);
      //
      for(unsigned i =myDmbs.size()-1; i < myDmbs.size(); ++i) {
	std::cout << "Slot " << myDmbs[i]->slot();
	myDmbs[i]->PrintCounters();
	int mycounter = myDmbs[i]->GetTmbDavScope();
	if ( mycounter>0 ) counter[delay][i] = mycounter;
      }
      //
      std::cout <<"After"<<std::endl;
      for(unsigned i =0; i < myTmbs.size(); ++i) {
	myTmbs[i]->GetCounters();
	myTmbs[i]->PrintCounters();
	myTmbs[i]->ResetCounters();
	myTmbs[i]->DisableCLCTInputs();
      }
      //
    }
  }
  //
  for(unsigned j = 0; j < myCrates.size(); ++j) {
    //
    std::vector<DAQMB*> myDmbs = theSelector.daqmbs(myCrates[j]);
    for(int delay=0; delay<200;delay++) {
      std::cout << delay << " " ;
      for(unsigned i =0; i < myDmbs.size(); ++i) std::cout << counter[delay][i] << " ";
      std::cout << std::endl;
    }
  }
  //
}
//




