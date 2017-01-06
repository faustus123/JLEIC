// $Id$
//
//    File: VertexTrackerTrack.h
// Created: Thu Jan  5 21:58:34 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _VertexTrackerTrack_
#define _VertexTrackerTrack_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class VertexTrackerTrack:public jana::JObject{
	public:
		JOBJECT_PUBLIC(VertexTrackerTrack);
		
		double p;
		double theta;
		double phi;
		double chisq;
		double ndf;

		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "p",     "%5.3f", p);
			AddString(items, "theta", "%5.3f", theta);
			AddString(items, "phi",   "%5.3f", phi);
			AddString(items, "chisq", "%4.2f", chisq);
			AddString(items, "ndf",   "%d",    (int)ndf);
		}
		
};

#endif // _VertexTrackerTrack_

