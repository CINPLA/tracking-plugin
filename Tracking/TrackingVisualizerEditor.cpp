/*
  ==============================================================================

    TrackingVisualizerEditor.cpp
    Created: 5 Oct 2015 11:35:12am
    Author:  mikkel

  ==============================================================================
*/

#include "TrackingVisualizerEditor.h"
#include "TrackingVisualizerCanvas.h"
#include "TrackingVisualizer.h"

//==============================================================================
TrackingVisualizerEditor::TrackingVisualizerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors)
{
    tabText = "Tracking";
    desiredWidth = 180;
}

TrackingVisualizerEditor::~TrackingVisualizerEditor()
{
    deleteAllChildren();
}

Visualizer* TrackingVisualizerEditor::createNewCanvas()
{
    TrackingVisualizer* processor = (TrackingVisualizer*) getProcessor();
    return new TrackingVisualizerCanvas(processor);
}

//void TrackingVisualizerEditor::saveCustomParameters(XmlElement *parentElement)
//{

//}

//void TrackingVisualizerEditor::loadCustomParameters(XmlElement *parametersAsXml)
//{

//}

void TrackingVisualizerEditor::initializeButtons()
{
    int w = 18;
    int h = 18;
    int xPad = 5;
    int yPad = 6;

    int xInitial = 10;
    int yInitial = 25;
    int x = xInitial;
    int y = yInitial;

    clearBtn = new UtilityButton("Clear", titleFont);
    clearBtn->setBounds(x, y, w*2 + xPad, h);
    clearBtn->setClickingTogglesState(false);
    clearBtn->addListener(this);
    addAndMakeVisible(clearBtn);

}

void TrackingVisualizerEditor::buttonCallback(Button* button)
{
    int gId = button->getRadioGroupId();

    if (gId > 0)
    {
        if (canvas != 0)
        {
            canvas->setParameter(gId-1, button->getName().getFloatValue());
        }

    }

}
