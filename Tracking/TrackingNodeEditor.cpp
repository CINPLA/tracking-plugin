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

#include "TrackingNodeEditor.h"
#include "TrackingNode.h"
#include "../../AccessClass.h"
#include "../../UI/EditorViewport.h"

TrackingNodeEditor::TrackingNodeEditor (GenericProcessor* parentNode, bool useDefaultParameterEditors = true)
    : GenericEditor (parentNode, useDefaultParameterEditors)
    , selectedSource(0)
{
    desiredWidth = 220;

    TrackingNode* processor = (TrackingNode*)getProcessor();

    sourceSelector = new ComboBox();
    sourceSelector->setBounds(45,30,130,20);
    sourceSelector->addListener(this);
    addAndMakeVisible(sourceSelector);

    plusButton = new UtilityButton("+", titleFont);
    plusButton->addListener(this);
    plusButton->setRadius(3.0f);
    plusButton->setBounds(10,30,20,20);
    addAndMakeVisible(plusButton);

    minusButton = new UtilityButton("-", titleFont);
    minusButton->addListener(this);
    minusButton->setRadius(3.0f);
    minusButton->setBounds(190,30,20,20);
    addAndMakeVisible(minusButton);

    portLabel = new Label ("Port", "Port:");
    portLabel->setBounds (10, 55, 140, 25);
    addAndMakeVisible (portLabel);
    int defaultPort = 27020;
    labelPort = new Label ("Port", String (defaultPort));
    labelPort->setBounds (80, 60, 80, 18);
    labelPort->setFont (Font ("Default", 15, Font::plain));
    labelPort->setColour (Label::textColourId, Colours::white);
    labelPort->setColour (Label::backgroundColourId, Colours::grey);
    labelPort->setEditable (true);
    labelPort->addListener (this);
    addAndMakeVisible (labelPort);

    adrLabel = new Label ("Address", "Address:");
    adrLabel->setBounds (10, 80, 140, 25);
    addAndMakeVisible (adrLabel);
    DBG ("in editor set default address");
    String defaultAddress = "/red";
    labelAdr = new Label ("Address", defaultAddress);
    labelAdr->setBounds (80, 85, 80, 18);
    labelAdr->setFont (Font ("Default", 15, Font::plain));
    labelAdr->setColour (Label::textColourId, Colours::white);
    labelAdr->setColour (Label::backgroundColourId, Colours::grey);
    labelAdr->setEditable (true);
    labelAdr->addListener (this);
    addAndMakeVisible (labelAdr);

    colorLabel = new Label ("Color", "Color:");
    colorLabel->setBounds (10, 105, 140, 25);
    addAndMakeVisible (colorLabel);
    String defaultColor = "red";
    labelColor = new Label ("Color", String (defaultColor));
    labelColor->setBounds (80, 110, 80, 18);
    labelColor->setFont (Font ("Default", 15, Font::plain));
    labelColor->setColour (Label::textColourId, Colours::white);
    labelColor->setColour (Label::backgroundColourId, Colours::grey);
    labelColor->setEditable (true);
    labelColor->addListener (this);
    addAndMakeVisible (labelColor);
}

TrackingNodeEditor::~TrackingNodeEditor()
{
}

void TrackingNodeEditor::labelTextChanged (Label* label)
{
    int selectedSource = sourceSelector->getSelectedId() - 1;
    TrackingNode* p = (TrackingNode*) getProcessor();

    if (label == labelAdr)
    {
        Value val = label->getTextValue();
        p->setAddress (selectedSource, val.getValue());
    }

    if (label == labelPort)
    {
        Value val = label->getTextValue();
        p->setPort (selectedSource, val.getValue());
    }

    if (label == labelColor)
    {
        Value val = label->getTextValue();
        p->setColor (selectedSource, val.getValue());
    }
    updateSettings();
}

void TrackingNodeEditor::comboBoxChanged(ComboBox* c)
{
    selectedSource = c->getSelectedId() - 1;
    updateLabels();
}

void TrackingNodeEditor::updateLabels()
{
    if (selectedSource < 0) {
        return;
    }
    TrackingNode* p = (TrackingNode*) getProcessor();
    labelAdr->setText(p->getAddress(selectedSource), dontSendNotification);
    labelPort->setText(String(p->getPort(selectedSource)), dontSendNotification);
    labelColor->setText(p->getColor(selectedSource), dontSendNotification);
}

void TrackingNodeEditor::buttonEvent(Button* button)
{
    TrackingNode* p = (TrackingNode*) getProcessor();
    if (button == plusButton && p->getNSources() < MAX_SOURCES)
        addTrackingSource();
    else if (button == minusButton && p->getNSources() > 1)
        removeTrackingSource();
    else
        CoreServices::sendStatusMessage("Number of sources must be between 1 and 10!");
    CoreServices::updateSignalChain(this);
}

void TrackingNodeEditor::addTrackingSource()
{
    std::cout << "Adding source" << std::endl;
    TrackingNode* p = (TrackingNode*) getProcessor();

    p->addSource(DEF_PORT, DEF_ADDRESS, DEF_COLOR);
    updateSettings();
    sourceSelector->setSelectedId(sourceSelector->getNumItems());
    selectedSource = sourceSelector->getSelectedId() - 1;
}

void TrackingNodeEditor::removeTrackingSource()
{
    std::cout << "Removing source" << std::endl;
    TrackingNode* p = (TrackingNode*) getProcessor();

    p->removeSource(selectedSource);
    if (selectedSource >= p->getNSources())
        selectedSource = p->getNSources() - 1;
    updateSettings();
}

void TrackingNodeEditor::updateSettings()
{
    TrackingNode* p = (TrackingNode*) getProcessor();
    sourceSelector->clear();

    for (int i = 0; i < p->getNSources(); i++)
        sourceSelector->addItem("Source " + String(i+1), i+1);

    sourceSelector->setSelectedId(selectedSource+1);
    updateLabels();
}
