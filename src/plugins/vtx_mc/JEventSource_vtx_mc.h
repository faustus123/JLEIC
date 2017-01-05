// $Id$
//
//    File: JEventSource_vtx_mc.h
// Created: Wed Jan  4 19:53:11 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _JEventSource_vtx_mc_
#define _JEventSource_vtx_mc_

#include <TFile.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

#include <JANA/jerror.h>
#include <JANA/JEventSource.h>
#include <JANA/JEvent.h>

#include "MCVertexTrackerHit.h"

class JEventSource_vtx_mc: public jana::JEventSource{
	public:
		JEventSource_vtx_mc(const char* source_name);
		virtual ~JEventSource_vtx_mc();
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "JEventSource_vtx_mc";}
		
		jerror_t GetEvent(jana::JEvent &event);
		void FreeEvent(jana::JEvent &event);
		jerror_t GetObjects(jana::JEvent &event, jana::JFactory_base *factory);
		
		TFile *fFile;
		TTreeReader *fReader;

		TTreeReaderValue<Int_t>    *EVENT_SIZE;
		TTreeReaderValue<Int_t>    *EVENT_NUM;
		TTreeReaderArray<Int_t>    *TrackID;
		TTreeReaderArray<Int_t>    *PlaneID;
		TTreeReaderArray<Double_t> *Xpos;
		TTreeReaderArray<Double_t> *Ypos;
		TTreeReaderArray<Double_t> *Zpos;
		//TTreeReaderArray<Int_t>    *Ipos;
		//TTreeReaderArray<Int_t>    *Jpos;
		
		class Event{
			public:
				int event_num;
				vector<MCVertexTrackerHit*> mcvtxhits;
				
				bool am_owner;
		};
};

#endif // _JEventSourceGenerator_vtx_mc_

