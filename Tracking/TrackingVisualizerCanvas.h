/*
  ==============================================================================

    TrackingVisualizerCanvas.h
    Created: 5 Oct 2015 12:09:00pm
    Author:  mikkel

  ==============================================================================
*/

#ifndef TRACKINGVISUALIZERCANVAS_H_INCLUDED
#define TRACKINGVISUALIZERCANVAS_H_INCLUDED

#include <VisualizerWindowHeaders.h>
#include "TrackingVisualizerEditor.h"
#include "TrackingVisualizer.h"
#include <vector>

class TrackingVisualizer;

class Position
{
public:
    Position(float xin, float yin, float widthin, float heightin);
    float x;
    float y;
    float width;
    float height;
};

//==============================================================================
/*
*/
class TrackingVisualizerCanvas : public Visualizer,
        public ComboBox::Listener,
        public Button::Listener,
        public KeyListener
{
public:
    TrackingVisualizerCanvas(TrackingVisualizer* TrackingVisualizer);
    ~TrackingVisualizerCanvas();

    void paint (Graphics&);
    void resized();
    void clear();
    // KeyListener interface
    virtual bool keyPressed(const KeyPress &key, Component *originatingComponent);

    // Listener interface
    virtual void buttonClicked(Button* button);

    // Listener interface
    virtual void comboBoxChanged(ComboBox *comboBoxThatHasChanged);

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

    bool m_prevSet;

    bool m_imgExists;
    Image rodentImg;
    int m_img_scale;
//    float m_curr_rot;

    ScopedPointer<UtilityButton> clearButton;
    ScopedPointer<UtilityButton> redButton;
    ScopedPointer<UtilityButton> greenButton;
    ScopedPointer<UtilityButton> yellowButton;
    ScopedPointer<UtilityButton> sourcesButton[MAX_SOURCES];
    ScopedPointer<UtilityButton> sameButton;

    ScopedPointer<Label> sourcesLabel;

    Colour redColour;
    Colour greenColour;
    Colour yellowColour;
    Colour defaultColour;
    Colour backgroundColour;

    Colour selectedColour;
    bool m_active[MAX_SOURCES];
    Colour m_colour[MAX_SOURCES];

    std::vector<Position> m_positions[MAX_SOURCES];

    void initButtonsAndLabels();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackingVisualizerCanvas);
};


#endif  // TRACKINGVISUALIZERCANVAS_H_INCLUDED
