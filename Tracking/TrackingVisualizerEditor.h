/*
  ==============================================================================

    TrackingVisualizerEditor.h
    Created: 5 Oct 2015 11:35:12am
    Author:  mikkel

  ==============================================================================
*/

#ifndef TRACKINGVISUALIZEREDITOR_H_INCLUDED
#define TRACKINGVISUALIZEREDITOR_H_INCLUDED

#include <VisualizerEditorHeaders.h>

//==============================================================================
/*
*/
class TrackingVisualizerEditor : public VisualizerEditor
{
public:
    TrackingVisualizerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    ~TrackingVisualizerEditor();

    void buttonCallback(Button* button);

    Visualizer* createNewCanvas();

//    void saveCustomParameters(XmlElement *parentElement) override;
//    void loadCustomParameters(XmlElement *parametersAsXml) override;

private:
    UtilityButton* clearBtn;
    void initializeButtons();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackingVisualizerEditor);

};


#endif  // TRACKINGVISUALIZEREDITOR_H_INCLUDED
