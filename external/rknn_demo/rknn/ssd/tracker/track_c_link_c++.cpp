#include "track_c_link_c++.h"
#include "objects_update.h"
#include "objects_tracker.h"




OdtDetector object = OdtDetector();

void object_track(int maxTrackLifetime, int track_num_input, object_T* object_input, int* track_num_output, object_T* object_output, int width, int height){
	
	object.update(maxTrackLifetime, track_num_input, object_input, track_num_output, object_output, width, height);
}



































