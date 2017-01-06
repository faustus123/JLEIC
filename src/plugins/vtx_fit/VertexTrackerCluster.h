// $Id$
//
//    File: VertexTrackerCluster.h
// Created: Thu Jan  5 21:59:51 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _VertexTrackerCluster_
#define _VertexTrackerCluster_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

#include "VertexTrackerHit.h"

class VertexTrackerCluster:public jana::JObject{
	public:
		JOBJECT_PUBLIC(VertexTrackerCluster);
		
		// Container of hits in the cluster. 
		// Technically we could just keep them as associated
		// objects and not define this container. However, this
		// is a little more transparent.
		vector<const VertexTrackerHit*> hits;
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "Nhits", "%4d", hits.size());
		}
		
};

#endif // _VertexTrackerCluster_

