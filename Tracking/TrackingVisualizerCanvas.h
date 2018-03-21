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

#ifndef TRACKINGVISUALIZERCANVAS_H_INCLUDED
#define TRACKINGVISUALIZERCANVAS_H_INCLUDED

#include <VisualizerWindowHeaders.h>
#include "TrackingVisualizerEditor.h"
#include "TrackingVisualizer.h"
#include <vector>
#include <map>

class TrackingVisualizer;

class SourceListBox : public ListBox,
        public ListBoxModel
{

public:

    SourceListBox();
    int getNumRows();
    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected);
    void setData(Array<String> data);

private:
    Array<String> array;

};


class TrackingVisualizerCanvas : public Visualizer,
        public Button::Listener
{
public:
    TrackingVisualizerCanvas(TrackingVisualizer* TrackingVisualizer);
    ~TrackingVisualizerCanvas();

    void paint (Graphics&);
    void resized();
    void clear();

    // Button Listener interface
    virtual void buttonClicked(Button* button);

    // Visualizer interface
    virtual void refreshState();
    virtual void update();
    virtual void refresh();
    virtual void beginAnimation();
    virtual void endAnimation();
    virtual void setParameter(int, float);
    virtual void setParameter(int, int, int, float);

private:

    TrackingVisualizer* processor;
    float m_x;
    float m_y;
    float m_width;
    float m_height;

    ScopedPointer<SourceListBox> listbox;
    ScopedPointer<UtilityButton> clearButton;
    ScopedPointer<UtilityButton> sameButton;
    ScopedPointer<Label> sourcesLabel;

    std::map<String, Colour> color_palette = {
        { "red", Colours::red },
        { "green", Colours::green },
        { "blue", Colours::blue },
        { "cyan", Colours::cyan },
        { "magenta", Colours::magenta },
        { "yellow", Colours::yellow },
        { "orange", Colours::orange },
        { "pink", Colours::pink },
        { "grey", Colours::grey },
        { "violet", Colours::violet },
        { "yellow", Colours::yellow },
        { "white", Colours::white },
        { "background", Colour(0, 18, 43) }
    };

    std::vector<TrackingPosition> m_positions[MAX_SOURCES];
    void initButtonsAndLabels();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackingVisualizerCanvas);
};


#endif  // TRACKINGVISUALIZERCANVAS_H_INCLUDED
