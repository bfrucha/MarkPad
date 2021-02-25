//
//  MacTouch.h
//  Multitouch detection
//
 
#ifndef MacTouch_h
#define MacTouch_h

struct MTPhase {
  enum {
    NotTracking = 0,
    Start = 1,
    Hover = 2,
    BeginTouch = 3,
    Touch = 4,
    EndTouch = 5,
    Linger = 6,
    OutOfRange = 7
  };
};

typedef float MTFloat;

struct MTPoint {
  MTFloat x, y;
};

struct MTSize {
  MTFloat width, height;
};

struct MTRect {
  MTFloat x, y, width, height;
};

struct MTReadout {
  MTPoint pos, velocity;
};

// MultiTouchSupport Framework
struct MTTouch {
  int32_t frame;        // current frame
  double time;          // event timestamp
  int32_t ident;        // identifier, unique for life of touch per device
  int32_t phase;        // distance from touchpad (see MTTouchPhase)
  int32_t fingerID;
  int32_t handID;
  MTReadout norm;       // normalized position and velocity (between 0,0 and 1,0)
  float size;           // area of the touch (estimates pressure), between 0,0 and 1,0
  int32_t unknown1;
  float angle;          // angle of the touch ellipse
  float majorAxis;      // major axis of the touch ellipse
  float minorAxis;      // minor axis of the touch ellipse
  MTReadout mm;         // absolute position and velocity?
  int32_t unknown2;
  int32_t unknown3;
  float density;
};

struct MTDevice;

class MTReader {
public:
  static int start(class MarkPad&);
  static void stop(class MarkPad&);
};

#endif
