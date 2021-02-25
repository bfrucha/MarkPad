//
//  MacTouch.mm (Mac version)
//  Multitouch detection
//

#ifdef __APPLE__

#include <iostream>
#include <Foundation/Foundation.h>
#include "MTouch.h"
#include "MarkPad.h"
#include "Pad.h"

extern "C" {  
  // returns a pointer to the default device (the trackpad?)
  MTDevice* MTDeviceCreateDefault();
  
  // returns a CFMutableArrayRef array of all multitouch devices
  NSMutableArray* MTDeviceCreateList();
  
  // Touch callback function
  typedef int (*MTContactFrameCallback)(MTDevice*, const MTTouch *contacts,
                                        int numContacts, double timestamp, int frame);
  
  // registers a device's frame callback to your callback function
  void MTRegisterContactFrameCallback(MTDevice*, MTContactFrameCallback);
  void MTUnregisterContactFrameCallback(MTDevice*, MTContactFrameCallback);
  
  // start sending events
  void MTDeviceStart(MTDevice*, int32_t = 0);
  void MTDeviceStop(MTDevice*);
  void MTDeviceRelease(MTDevice*);
  
  bool MTDeviceIsAvailable();
  bool MTDeviceIsRunning(MTDevice*);
  double MTAbsoluteTimeGetCurrent();
  
  int MTDeviceGetSensorDimensions(MTDevice*, int32_t *rows, int32_t *cols);
  int MTDeviceGetSensorSurfaceDimensions(MTDevice*, int32_t *width, int32_t *height);
  
  bool MTDeviceIsValid(MTDevice*);
  bool MTDeviceIsOpaqueSurface(MTDevice*);
  OSStatus MTDeviceGetFamilyID(MTDevice*, int32_t* familyId);
  OSStatus MTDeviceGetDeviceID(MTDevice*, uint64_t* ID) __attribute__ ((weak_import));
  OSStatus MTDeviceGetDriverType(MTDevice*, int32_t*);
  OSStatus MTDeviceGetGUID(MTDevice*, uuid_t* guid);
}


void MTReader::stop(MarkPad& mp) {
  for (Pad* p : mp.getPads()) {
    if (p && p->device()) MTDeviceStop(p->device());
  }
}


int MTReader::start(MarkPad& mp) {
  NSMutableArray* trackpadList = MTDeviceCreateList();
  if (!trackpadList) return 0;
  
  // retrieve touchpad size and register callback for retrieving touch events
  int count = 0;
  for (int k = 0; k < int([trackpadList count]); k++) {
    if (MTDevice* trackpad = (__bridge MTDevice*)[trackpadList objectAtIndex:k]) {
      count++;
      int width = 0, height = 0;
      MTDeviceGetSensorSurfaceDimensions(trackpad, &width, &height); // get trackpad size
      
      if (mp.addPad(trackpad, width/100.f, height/100.f))
        std::cout << "*** Touchpad size: "<<width/100.<<"x"<<height/100.<<" mm"<<std::endl;
      else
        MarkPad::warning("MarkPad: touchpad has invalid size!");
      
      // register callback that gets touch events
      MTRegisterContactFrameCallback(trackpad, MarkPad::touchCallback);
      
      // start sending events
      MTDeviceStart(trackpad, 0);
    }
  }
  return count;
}

#endif

