/*
  ==============================================================================

    TrackingVisualizer.cpp
    Created: 5 Oct 2015 11:34:58am
    Author:  mikkel

  ==============================================================================
*/

#include "TrackingVisualizer.h"
#include "TrackingVisualizerEditor.h"

using std::cout;
using std::endl;
#include <algorithm>

//==============================================================================
TrackingVisualizer::TrackingVisualizer()
    : GenericProcessor("Tracking Visual")
    , m_positionIsUpdated(false)
    , m_sources(0)
    , m_clearTracking(false)
    , m_isRecording(false)
{
    setProcessorType (PROCESSOR_TYPE_SINK);
}

TrackingVisualizer::~TrackingVisualizer()
{
}

AudioProcessorEditor *TrackingVisualizer::createEditor()
{
    editor = new TrackingVisualizerEditor(this, true);
    return editor;
}

void TrackingVisualizer::process(AudioSampleBuffer &)
{
    checkForEvents();
    // Clear tracking when start recording
    if (CoreServices::getRecordingStatus())
        m_isRecording = true;
    else
    {
        m_isRecording = false;
        m_clearTracking = false;
    }
}

void TrackingVisualizer::handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int)
{
    if ((eventInfo->getName()).compare("Tracking data") != 0)
    {
        return;
    }

    BinaryEventPtr evtptr = BinaryEvent::deserializeFromMessage(event, eventInfo);

    if(event.getRawDataSize() != sizeof(TrackingData) + 18) { // TODO figure out why it is + 18
        cout << "Position tracker got wrong event size x,y,width,height was expected: " << event.getRawDataSize() << endl;
        return;
    }

//    const auto* rawData = (uint8*) evtptr->getBinaryDataPointer();
    auto nodeId = evtptr->getSourceID();
//    const int64 timestamp = *(rawData);
//    const float* message = (float*)(rawData+sizeof(int64));
    const auto *message = reinterpret_cast<const TrackingData *>(evtptr->getBinaryDataPointer());

    std::vector<uint8>::iterator pos = find(m_ids.begin(), m_ids.end(), nodeId);

    if (pos == m_ids.end())
    {

        if(!(message->position.x != message->position.x || message->position.y != message->position.y) && message->position.x != 0 && message->position.y != 0)
        {
            m_sources++;
            m_ids.push_back(nodeId);
            m_x.push_back(message->position.x);
            m_y.push_back(message->position.y);
        }
        if(!(message->position.width != message->position.width || message->position.height != message->position.height))
        {
            m_width.push_back(message->position.width);
            m_height.push_back(message->position.height);
        }
    }
    else
    {
        auto idx = std::distance(m_ids.begin(),  pos) ;
        if(!(message->position.x != message->position.x || message->position.y != message->position.y) && message->position.x != 0 && message->position.y != 0)
        {
            m_x[idx] = message->position.x;
            m_y[idx] = message->position.y;
        }
        if(!(message->position.width != message->position.width || message->position.height != message->position.height))
        {
            m_width[idx] = message->position.width;
            m_height[idx] = message->position.height;
        }
    }

    m_positionIsUpdated = true;

}


float TrackingVisualizer::getX(int s) const
{
    if (s < m_x.size())
        return m_x[s];
    else
        return -1;
}

float TrackingVisualizer::getY(int s) const
{
    if (s < m_y.size())
        return m_y[s];
    else
        return -1;
}
float TrackingVisualizer::getWidth(int s) const
{
    if (s < m_width.size())
        return m_width[s];
    else
        return -1;
}

float TrackingVisualizer::getHeight(int s) const
{
    if (s < m_height.size())
        return m_height[s];
    else
        return -1;
}

bool TrackingVisualizer::getIsRecording() const
{
    return m_isRecording;
}
bool TrackingVisualizer::getClearTracking() const
{
    return m_clearTracking;
}

int TrackingVisualizer::getNSources() const
{
    return m_sources;
}

void TrackingVisualizer::clearPositionUpdated()
{
    m_positionIsUpdated = false;
}

bool TrackingVisualizer::positionIsUpdated() const
{
    return m_positionIsUpdated;
}

void TrackingVisualizer::setClearTracking(bool clear)
{
    m_clearTracking = clear;
}
