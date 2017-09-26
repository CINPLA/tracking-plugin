/*
  ==============================================================================

    PositionTracker.cpp
    Created: 5 Oct 2015 11:34:58am
    Author:  mikkel

  ==============================================================================
*/

#include "PositionTracker.h"
#include "PositionTrackerEditor.h"

using std::cout;
using std::endl;
#include <algorithm>

//==============================================================================
PositionTracker::PositionTracker()
    : GenericProcessor("Position Tracker")
    , m_positionIsUpdated(false)
    , m_sources(0)
    , m_clearTracking(false)
    , m_isRecording(false)
{
    setProcessorType (PROCESSOR_TYPE_SINK);
}

PositionTracker::~PositionTracker()
{
}

AudioProcessorEditor *PositionTracker::createEditor()
{
    editor = new PositionTrackerEditor(this, true);
    return editor;
}

void PositionTracker::process(AudioSampleBuffer &buffer, MidiBuffer &events)
{
    checkForEvents(events);
    // Clear tracking when start recording
    if (CoreServices::getRecordingStatus())
        m_isRecording = true;
    else
    {
        m_isRecording = false;
        m_clearTracking = false;
    }
}

void PositionTracker::handleEvent(int eventType, MidiMessage &event, int samplePosition)
{
    if(eventType == BINARY_MSG) {
        const uint8* rawData = event.getRawData();
        if(event.getRawDataSize() != 6 + sizeof(float)*4 + sizeof(int64)) {
            cout << "Position tracker got wrong event size x,y,width,height was expected: " << event.getRawDataSize() << endl;
        }
        const uint8* nodeId = rawData + 1;
        const int64 timestamp = *(rawData + 6);
        const float* message = (float*)(rawData+6+sizeof(int64));

        std::vector<uint8>::iterator pos = find(m_ids.begin(), m_ids.end(), *nodeId);

        if (pos == m_ids.end())
        {

            if(!(message[0] != message[0] || message[1] != message[1]) && message[0] != 0 && message[1] != 0)
            {
                m_sources++;
                m_ids.push_back(*nodeId);
                m_x.push_back(message[0]);
                m_y.push_back(message[1]);
            }
            if(!(message[2] != message[2] || message[3] != message[3]))
            {
                m_width.push_back(message[2]);
                m_height.push_back(message[3]);
            }
        }
        else
        {
            auto idx = std::distance(m_ids.begin(),  pos) ;
            if(!(message[0] != message[0] || message[1] != message[1]) && message[0] != 0 && message[1] != 0)
            {
                m_x[idx] = message[0];
                m_y[idx] = message[1];
            }
            if(!(message[2] != message[2] || message[3] != message[3]))
            {
                m_width[idx] = message[2];
                m_height[idx] = message[3];
            }
        }

        m_positionIsUpdated = true;
    }
}

float PositionTracker::getX(int s) const
{
    if (s < m_x.size())
        return m_x[s];
    else
        return -1;
}

float PositionTracker::getY(int s) const
{
    if (s < m_y.size())
        return m_y[s];
    else
        return -1;
}
float PositionTracker::getWidth(int s) const
{
    if (s < m_width.size())
        return m_width[s];
    else
        return -1;
}

float PositionTracker::getHeight(int s) const
{
    if (s < m_height.size())
        return m_height[s];
    else
        return -1;
}

bool PositionTracker::getIsRecording() const
{
    return m_isRecording;
}
bool PositionTracker::getClearTracking() const
{
    return m_clearTracking;
}

int PositionTracker::getNSources() const
{
    return m_sources;
}

void PositionTracker::clearPositionUpdated()
{
    m_positionIsUpdated = false;
}

bool PositionTracker::positionIsUpdated() const
{
    return m_positionIsUpdated;
}

void PositionTracker::setClearTracking(bool clear)
{
    m_clearTracking = clear;
}
