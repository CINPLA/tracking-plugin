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

#include "TrackingStimulatorEditor.h"
#include "TrackingStimulator.h"
#include "TrackingStimulatorCanvas.h"


TrackingStimulatorEditor::TrackingStimulatorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors)
{
    tabText = "TrackingStimulator";
    desiredWidth = 200;

    // Stimulate (toggle)
    stimulateButton = new TextButton("OFF");
    stimulateButton->setBounds(50, 60, 80, 30); // sets the x position, y position, width, and height of the button
    stimulateButton->addListener(this); // allows the editor to respond to clicks
    stimulateButton->setClickingTogglesState(true); // makes the button toggle its state when clicked
    addAndMakeVisible(stimulateButton); // makes the button a child component of the editor and makes it visible
}

TrackingStimulatorEditor::~TrackingStimulatorEditor()
{
}

Visualizer* TrackingStimulatorEditor::createNewCanvas()
{
    TrackingStimulator* processor = (TrackingStimulator*) getProcessor();
    return new TrackingStimulatorCanvas(processor);
}

void TrackingStimulatorEditor::buttonEvent(Button* button)
{
    if (button == stimulateButton)
    {
        if (button->getToggleState()==true)
        {
            TrackingStimulator *p = (TrackingStimulator *)getProcessor();
            p->startStimulation();
            stimulateButton->setButtonText(String("ON"));
        }
        else
        {
            TrackingStimulator *p = (TrackingStimulator *)getProcessor();
            p->stopStimulation();
            stimulateButton->setButtonText(String("OFF"));
        }
    }
}
