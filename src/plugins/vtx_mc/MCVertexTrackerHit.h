// $Id$
//
//    File: MCVertexTrackerHit.h
// Created: Wed Jan  4 21:34:19 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _MCVertexTrackerHit_
#define _MCVertexTrackerHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class MCVertexTrackerHit:public jana::JObject{
	public:
		JOBJECT_PUBLIC(MCVertexTrackerHit);
		
		int trackid;
		int planeid;
		double Xpos;
		double Ypos;
		double Zpos;
		//int Ipos;
		//int Jpos;
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "trackid", "%4d", trackid);
			AddString(items, "planeid", "%4d", planeid);
			AddString(items, "Xpos", "%f", Xpos);
			AddString(items, "Ypos", "%f", Ypos);
			AddString(items, "Zpos", "%f", Zpos);
		}
		
};

#endif // _MCVertexTrackerHit_

