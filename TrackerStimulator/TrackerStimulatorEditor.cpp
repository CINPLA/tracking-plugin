/*
  ==============================================================================

    TrackerStimulatorEditor.cpp
    Created: 28 Jun 2016 11:59:59am
    Author:  alessio

  ==============================================================================
*/

#include "TrackerStimulatorEditor.h"
#include "TrackerStimulator.h"
#include "TrackerStimulatorCanvas.h"


TrackerStimulatorEditor::TrackerStimulatorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors)
{
    tabText = "TrackerStimulator";
    desiredWidth = 200;


    // Add buttons

    // Stimulate (toggle)
//    stimulateButton = new TextButton("ON", Font("Small Text", 13, Font::plain));
    stimulateButton = new TextButton("OFF");
//    stimulateButton->setRadius(3.0f); // sets the radius of the button's corners
    stimulateButton->setBounds(60, 30, 80, 30); // sets the x position, y position, width, and height of the button
    stimulateButton->addListener(this); // allows the editor to respond to clicks
    stimulateButton->setClickingTogglesState(true); // makes the button toggle its state when clicked
    addAndMakeVisible(stimulateButton); // makes the button a child component of the editor and makes it visible

    //testPattern
    testPatternButton = new UtilityButton("Test", Font("Small Text", 13, Font::plain));
    testPatternButton->setRadius(3.0f); // sets the radius of the button's corners
    testPatternButton->setBounds(70, 100, 60, 20); // sets the x position, y position, width, and height of the button
    testPatternButton->addListener(this); // allows the editor to respond to clicks
    testPatternButton->setClickingTogglesState(false); // makes the button toggle its state when clicked
    addAndMakeVisible(testPatternButton); // makes the button a child component of the editor and makes it visible


    //sync
    syncButton = new UtilityButton("Sync", Font("Small Text", 13, Font::plain));
    syncButton->setRadius(3.0f); // sets the radius of the button's corners
    syncButton->setBounds(70, 70, 60, 20); // sets the x position, y position, width, and height of the button
    syncButton->addListener(this); // allows the editor to respond to clicks
    syncButton->setClickingTogglesState(false); // makes the button toggle its state when clicked
    addAndMakeVisible(syncButton); // makes the button a child component of the editor and makes it visible

    syncTTLChanSelector = new ComboBox();
    syncTTLChanSelector->setBounds(15, 70, 40, 20);
    syncTTLChanSelector->addListener(this);

    for (int i=1; i<10; i++)
        syncTTLChanSelector->addItem(String(i), i);

    syncTTLChanSelector->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(syncTTLChanSelector);

    syncStimChanSelector = new ComboBox();
    syncStimChanSelector->setBounds(140, 70, 40, 20);
    syncStimChanSelector->addListener(this);

    for (int i=1; i<5; i++)
        syncStimChanSelector->addItem(String(i), i);

    syncStimChanSelector->setSelectedId(4, dontSendNotification);
    addAndMakeVisible(syncStimChanSelector);

    //sync labels
    ttlLabel = new Label("TTL", "TTL");
    ttlLabel->setBounds(20, 55, 40, 15);
    ttlLabel->setFont(Font("Default", 12, Font::plain));
    addAndMakeVisible(ttlLabel);

    stimLabel = new Label("Stim", "Stim");
    stimLabel->setBounds(135, 55, 40, 15);
    stimLabel->setFont(Font("Default", 12, Font::plain));
    addAndMakeVisible(stimLabel);

    TrackerStimulator *p= (TrackerStimulator *)getProcessor();
    p->setStimSyncChan(syncStimChanSelector->getSelectedId()-1);
    p->setTTLSyncChan(syncTTLChanSelector->getSelectedId()-1);
    syncStimChan = syncStimChanSelector->getSelectedId()-1;

}

TrackerStimulatorEditor::~TrackerStimulatorEditor()
{
    //deleteAllChildren();
}

Visualizer* TrackerStimulatorEditor::createNewCanvas()
{
    TrackerStimulator* processor = (TrackerStimulator*) getProcessor();
    return new TrackerStimulatorCanvas(processor);
}


void TrackerStimulatorEditor::buttonEvent(Button* button)
{
//    int gId = button->getRadioGroupId();

//    if (gId > 0)
//    {
//        if (canvas != 0)
//        {
//            canvas->setParameter(gId-1, button->getName().getFloatValue());
//        }

//    }

    // testPattern
    if (button == testPatternButton)
    {
        TrackerStimulator *p= (TrackerStimulator *)getProcessor();

        if (!button->isDown())
            p->testStimulation();
    }
    else if (button == syncButton)
    {
        TrackerStimulator *p= (TrackerStimulator *)getProcessor();

        if (!button->isDown())
            p->syncStimulation(syncStimChan);
    }
    else if (button == stimulateButton)
    {
        if (button->getToggleState()==true){
            TrackerStimulator *p= (TrackerStimulator *)getProcessor();
            p->startStimulation();
            stimulateButton->setButtonText(String("ON"));
        }
        else {
            TrackerStimulator *p= (TrackerStimulator *)getProcessor();
            p->stopStimulation();
            stimulateButton->setButtonText(String("OFF"));
        }
    }

}

void TrackerStimulatorEditor::comboBoxChanged(ComboBox* c)
{
    if (c==syncStimChanSelector)
    {
        TrackerStimulator *p= (TrackerStimulator *)getProcessor();
        p->setStimSyncChan(c->getSelectedId()-1);
    }
    else if (c==syncTTLChanSelector)
    {
        TrackerStimulator *p= (TrackerStimulator *)getProcessor();
        p->setTTLSyncChan(c->getSelectedId()-1);
        syncStimChan = c->getSelectedId()-1;
    }

}

