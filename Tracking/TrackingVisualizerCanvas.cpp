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

#include "TrackingVisualizerCanvas.h"
#include "TrackingVisualizer.h"

#include <math.h>
#include <string>

//SourceListBox methods

SourceListBox::SourceListBox() : ListBox ("listbox", 0)
{
    setModel (this);
    setMultipleSelectionEnabled (true);
    setClickingTogglesRowSelection	(true);
    setColour (ListBox::outlineColourId, Colour(200, 255, 0));
    setColour (ListBox::backgroundColourId, Colours::grey.withAlpha (0.7f));
}

int SourceListBox::getNumRows()
{
    return array.size();
}

void SourceListBox::paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (Colours::lightblue);

    g.setColour (Colours::black);
    g.setFont (height * 0.95f);

    String rowData = array[rowNumber];
    g.drawText (rowData, 5, 0, width, height, Justification::centredLeft, true);
}

void SourceListBox::setData(Array<String> data)
{
    array = data;
}

TrackingVisualizerCanvas::TrackingVisualizerCanvas(TrackingVisualizer *TrackingVisualizer)
    : processor(TrackingVisualizer)
    , m_width(1.0)
    , m_height(1.0)
{
    initButtonsAndLabels();
    startCallbacks();
}

TrackingVisualizerCanvas::~TrackingVisualizerCanvas()
{
}

void TrackingVisualizerCanvas::paint (Graphics& g)
{

    float plot_height = 0.97*getHeight();
    float plot_width = 0.85*getWidth();
    float plot_bottom_left_x = 0.15*getWidth();
    float plot_bottom_left_y = 0.01*getHeight();

    // set aspect ratio to cam size
    float aC = m_width / m_height;
    float aS = plot_width / plot_height;
    int camHeight = (aS > aC) ? plot_height : plot_height * (aS / aC);
    int camWidth = (aS < aC) ? plot_width : plot_width * (aC / aS);

    g.setColour(Colours::black); // backbackround color
    g.fillRect(0, 0, getWidth(), getHeight());

    g.setColour(color_palette["background"]); //background color
    g.fillRect(int(plot_bottom_left_x), int(plot_bottom_left_y),
               int(camWidth), int(camHeight));

    for (int i = 0; i < processor->getNSources (); i++)
    {
        bool source_active = listbox->isRowSelected(i);
        TrackingSources& source = processor->getTrackingSource(i);
        Colour source_colour = color_palette[source.color];
        g.setColour(source_colour);

        // update colors
        if (processor->getColorIsUpdated())
        {
            update();
            processor->setColorIsUpdated(false);
        }

        // Plot trajectory as lines
        if (m_positions[i].size () >= 2 && source_active)
        {
            for(auto it = m_positions[i].begin()+1; it != m_positions[i].end(); it++)
            {
                TrackingPosition position = *it;
                TrackingPosition prev_position = *(it-1);

                // if tracking data are empty positions are set to -1
                if (prev_position.x != -1 && prev_position.y != -1)
                {
                    float x = camWidth*position.x + plot_bottom_left_x;
                    float y = camHeight*position.y + plot_bottom_left_y;
                    float x_prev = camWidth*prev_position.x + plot_bottom_left_x;
                    float y_prev = camHeight*prev_position.y + plot_bottom_left_y;
                    g.drawLine(x_prev, y_prev, x, y, 5.0f);
                }
            }
            // Plot current position as ellipse
            if (!m_positions[i].empty ())
            {
                TrackingPosition position = m_positions[i].back();
                float x = camWidth*position.x + plot_bottom_left_x;
                float y = camHeight*position.y + plot_bottom_left_y;
                g.fillEllipse(x - 0.01*getHeight(), y - 0.01*getHeight(), 0.02*getHeight(), 0.02*getHeight());
            }
        }
    }
}

void TrackingVisualizerCanvas::resized()
{
    clearButton->setBounds(0.01*getWidth(), getHeight()-0.05*getHeight(), 0.13*getWidth(), 0.03*getHeight());
    sourcesLabel->setBounds(0.01*getWidth(), getHeight()-0.7*getHeight(), 0.13*getWidth(), 0.03*getHeight());
    listbox->setBounds(0.01*getWidth(), getHeight()-0.65*getHeight(), 0.13*getWidth(), 0.4*getHeight());
    refresh();
}

void TrackingVisualizerCanvas::buttonClicked(Button* button)
{
    if (button == clearButton)
        clear();
}

void TrackingVisualizerCanvas::refreshState()
{

}

void TrackingVisualizerCanvas::update()
{
    Array<String> listboxData;

    int nSources = processor->getNSources();
    for (int i = 0; i < nSources; i++)
    {
        TrackingSources& source = processor->getTrackingSource(i);
        String name = source.name;
        listboxData.add (name);
    }

    listbox->setData(listboxData);
    listbox->updateContent();

}

void TrackingVisualizerCanvas::refresh()
{
    if (processor->positionIsUpdated()) {
        for (int i = 0; i<processor->getNSources(); i++)
        {
            TrackingPosition currPos;
            currPos.x = processor->getX(i);
            currPos.y = processor->getY(i);
            currPos.width = processor->getWidth(i);
            currPos.height = processor->getHeight(i);
            m_positions[i].push_back(currPos);

            // for now, just pick one w and h
            m_height = processor->getHeight(i);
            m_width = processor->getWidth(i);
        }
        processor->clearPositionUpdated();
        repaint();
    }
    if (processor->getIsRecording()){
        if (!processor->getClearTracking())
        {
            processor->setClearTracking(true);
            clear();
        }
    }
}

void TrackingVisualizerCanvas::beginAnimation()
{
    startCallbacks();
}

void TrackingVisualizerCanvas::endAnimation()
{
    stopCallbacks();
}

void TrackingVisualizerCanvas::setParameter(int, float)
{
}

void TrackingVisualizerCanvas::setParameter(int, int, int, float)
{
}

void TrackingVisualizerCanvas::clear()
{
    for (int i = 0; i<MAX_SOURCES; i++)
        m_positions[i].clear();
    repaint();
}

void TrackingVisualizerCanvas::initButtonsAndLabels()
{
    clearButton = new UtilityButton("Clear plot", Font("Small Text", 13, Font::plain));
    clearButton->setRadius(3.0f);
    clearButton->addListener(this);
    addAndMakeVisible(clearButton);

    listbox = new SourceListBox();
    addAndMakeVisible(listbox);

    // Static Labels
    sourcesLabel = new Label("s_sources", "Sources");
    sourcesLabel->setFont(Font(28));
    sourcesLabel->setColour(Label::textColourId, Colour(200, 255, 0));
    addAndMakeVisible(sourcesLabel);
    sourcesLabel->setVisible(true);

}
