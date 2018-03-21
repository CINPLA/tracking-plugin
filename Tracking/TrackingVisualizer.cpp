/*
    ------------------------------------------------------------------

    This file is part of the Tracking plugin for the Open Ephys GUI
    Written by:

    Alessio Buccino     alessiob@ifi.uio.no
    Mikkel Lepperod
    Svenn-Arne Dragly

    Center for Integrated Neuroplasticity CINPLA
    Department of Biosciences
    University of Oslo
    Norway

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "TrackingVisualizer.h"
#include "TrackingVisualizerEditor.h"

#include <algorithm>

TrackingVisualizer::TrackingVisualizer()
    : GenericProcessor("Tracking Visual")
    , m_positionIsUpdated(false)
    , m_clearTracking(false)
    , m_isRecording(false)
    , m_colorUpdated(false)
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

void TrackingVisualizer::updateSettings()
{
    sources.clear();
    TrackingSources s;
    int nEvents = getTotalEventChannels();
    for (int i = 0; i < nEvents; i++)
    {
        const EventChannel* event = getEventChannel(i);
        if (event->getName().compare("Tracking data") == 0)
        {
            s.eventIndex = event->getSourceIndex();
            s.sourceId =  event->getSourceNodeID();
            s.name = "Tracking source " + String(event->getSourceIndex()+1);
            s.color = "None";
            s.x_pos = -1;
            s.y_pos = -1;
            s.width = -1;
            s.height = -1;
            sources.add (s);
            m_colorUpdated = true;
        }
    }
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

    int nodeId = evtptr->getSourceID();
    int evtId = evtptr->getSourceIndex();
    const auto *position = reinterpret_cast<const TrackingPosition *>(evtptr->getBinaryDataPointer());

    int nSources = sources.size ();

    for (int i = 0; i < nSources; i++)
    {
        TrackingSources& currentSource = sources.getReference (i);
        if (currentSource.sourceId == nodeId && evtId == currentSource.eventIndex)
        {
            if(!(position->x != position->x || position->y != position->y) && position->x != 0 && position->y != 0)
            {
                currentSource.x_pos = position->x;
                currentSource.y_pos = position->y;
            }
            if(!(position->width != position->width || position->height != position->height))
            {
                currentSource.width = position->width;
                currentSource.height = position->height;
            }

            String sourceColor;
            evtptr->getMetaDataValue(0)->getValue(sourceColor);

            if (currentSource.color.compare(sourceColor) != 0)
            {
                currentSource.color = sourceColor;
                m_colorUpdated = true;
            }
        }
    }

    m_positionIsUpdated = true;

}

TrackingSources& TrackingVisualizer::getTrackingSource(int s) const
{
    if (s < sources.size())
        return sources.getReference (s);
}


float TrackingVisualizer::getX(int s) const
{
    if (s < sources.size())
        return sources[s].x_pos;
    else
        return -1;
}

float TrackingVisualizer::getY(int s) const
{
    if (s < sources.size())
        return sources[s].y_pos;
    else
        return -1;
}
float TrackingVisualizer::getWidth(int s) const
{
    if (s < sources.size())
        return sources[s].width;
    else
        return -1;
}

float TrackingVisualizer::getHeight(int s) const
{
    if (s < sources.size())
        return sources[s].height;
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

bool TrackingVisualizer::getColorIsUpdated() const
{
    return m_colorUpdated;
}

void TrackingVisualizer::setColorIsUpdated(bool up)
{
    m_colorUpdated = up;
}


int TrackingVisualizer::getNSources() const
{
    return sources.size ();
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
