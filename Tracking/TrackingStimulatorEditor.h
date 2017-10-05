#ifndef TRACKINGSTIMULATOREDITOR_H
#define TRACKINGSTIMULATOREDITOR_H


#include <VisualizerEditorHeaders.h>
#include <EditorHeaders.h>

class TrackingStimulatorEditor : public VisualizerEditor
        , public ComboBox::Listener
{
public:
    TrackingStimulatorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~TrackingStimulatorEditor();

    // Listener Interface
    void buttonEvent(Button* button);
    void comboBoxChanged(ComboBox* c);

    Visualizer* createNewCanvas();

private:
    // Stimulate button
    ScopedPointer<TextButton> stimulateButton;

    // Test button
    ScopedPointer<UtilityButton> testPatternButton;

    // Sync button
    ScopedPointer<UtilityButton> syncButton;
    ScopedPointer<ComboBox> syncTTLChanSelector;
    ScopedPointer<ComboBox> syncStimChanSelector;
    int syncStimChan;

    //Sync labels
    ScopedPointer<Label> ttlLabel;
    ScopedPointer<Label> stimLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackingStimulatorEditor);
};


#endif // TrackingSTIMULATOREDITOR_H
