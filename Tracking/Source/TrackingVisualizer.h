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

#ifndef TRACKINGVISUALIZER_H_INCLUDED
#define TRACKINGVISUALIZER_H_INCLUDED

#include <ProcessorHeaders.h>
#include "TrackingVisualizerEditor.h"
#include "TrackingMessage.h"

#include <vector>

#define MAX_SOURCES 10

/**

    Visualizes tracking from "Tracking data" events

    @see GenericProcessor, TrackingVisualizerEditor, TrackingVisualizerCanvas
*/
class TrackingVisualizer : public GenericProcessor
{
public:
    TrackingVisualizer();
    ~TrackingVisualizer();

    AudioProcessorEditor* createEditor();

    void process(AudioSampleBuffer& buffer) override;
    void handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int) override;
    void updateSettings();

    float getX(int s) const;
    float getY(int s) const;
    float getWidth(int s) const;
    float getHeight(int s) const;
    bool getIsRecording() const;
    bool getClearTracking() const;

    int getNSources() const;
    TrackingSources& getTrackingSource(int i) const;

    void setClearTracking(bool clear);

    void clearPositionUpdated();
    bool positionIsUpdated() const;
    bool getColorIsUpdated() const;
    void setColorIsUpdated(bool up);

private:
    
    Array<TrackingSources> sources;

    bool m_positionIsUpdated;
    bool m_clearTracking;
    bool m_isRecording;
    bool m_colorUpdated;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackingVisualizer);
};


#endif  // TRACKINGVISUALIZER_H_INCLUDED
