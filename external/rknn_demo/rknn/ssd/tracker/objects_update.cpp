#include "stdio.h"
#include "objects_update.h"

OdtDetector::OdtDetector()
{  
	printf("success build\n");
}

OdtDetector::~OdtDetector()
{
}

int OdtDetector::update(int maxTrackLifetime, int track_num_input, object_T* object_input, 
					int* track_num_output, object_T* object_output, int width, int height)
{
  	m_width = width;
	m_height = height;

    std::vector<Rect_T> objects_Rect_T;
    std::vector<int> objects_class;

	for (int i = 0; i < track_num_input; i++) {
		objects_Rect_T.push_back(object_input[i].r);
        objects_class.push_back(object_input[i].obj_class);
    }

    m_objects_tracker.updateTrackedObjects(maxTrackLifetime, objects_Rect_T, objects_class, m_width, m_height);
	objects_class.clear();
	
    std::vector<ObjectsTracker::ExtObject> extObjects;
    m_objects_tracker.getObjects(extObjects);
    
	int nobjects = extObjects.size();
	*track_num_output = nobjects;
	
    int i = 0;
    for (; i < nobjects && i < 100; i++)
    {
		if(extObjects[i].miss == 0){
	        //object_output[i].r = extObjects[i].location;	
	        object_output[i].r = extObjects[i].smooth_rect;
		}
		else{
			object_output[i].r = extObjects[i].predict_loc_when_miss;
		}
		object_output[i].obj_class = extObjects[i].obj_class;
	    object_output[i].id = extObjects[i].id;
    }
    return nobjects;
}
