#ifndef TRACKINGDATA_H
#define TRACKINGDATA_H

#include <ProcessorHeaders.h>

struct TrackingPosition {
    float x;
    float y;
    float width;
    float height;
};

struct TrackingData {
    uint64 timestamp;
    TrackingPosition position;
};

#endif // TRACKINGDATA_H
