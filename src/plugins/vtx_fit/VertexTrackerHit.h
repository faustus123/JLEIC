// $Id$
//
//    File: VertexTrackerHit.h
// Created: Thu Jan  5 21:59:40 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _VertexTrackerHit_
#define _VertexTrackerHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class VertexTrackerHit:public jana::JObject{
	public:
		JOBJECT_PUBLIC(VertexTrackerHit);
		
		int planeid;
		double Xpos;
		double Ypos;
		double Zpos;
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "planeid", "%4d", planeid);
			AddString(items, "Xpos", "%f", Xpos);
			AddString(items, "Ypos", "%f", Ypos);
			AddString(items, "Zpos", "%f", Zpos);
		}
		
};

#endif // _VertexTrackerHit_

