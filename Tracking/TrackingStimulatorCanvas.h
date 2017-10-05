#ifndef TRACKINGSTIMULATORCANVAS_H
#define TRACKINGSTIMULATORCANVAS_H


#include <VisualizerWindowHeaders.h>
#include "TrackingStimulatorEditor.h"
#include "TrackingStimulator.h"


class DisplayAxes;

class TrackingStimulatorCanvas : public Visualizer,
        public Button::Listener,
        public Label::Listener,
        public KeyListener
//        public ComboBox::Listener,
{
public:
    TrackingStimulatorCanvas(TrackingStimulator* TrackingStimulator);
    ~TrackingStimulatorCanvas();

    void paint (Graphics&);
    void resized();
    void clear();
    void initButtons();
    void initLabels();

    TrackingStimulator* getProcessor();

    // KeyListener interface
    virtual bool keyPressed(const KeyPress &key, Component *originatingComponent);

    // Listener interface
    virtual void buttonClicked(Button* button);
    virtual void labelTextChanged(Label *label) override;

    // Visualizer interface
    virtual void refreshState();
    virtual void update();
    virtual void refresh();
    virtual void beginAnimation();
    virtual void endAnimation();
    virtual void setParameter(int, float);
    virtual void setParameter(int, int, int, float);


    bool getUpdateCircle();
    void setUpdateCircle(bool onoff);
    bool areThereCicles();
    void setOnButton();
    float my_round(float x);
    void uploadInfoOnLoad();

    // *** Maybe adjust with proper accessors instead of keep public *** //
    ScopedPointer<UtilityButton> clearButton;
    ScopedPointer<UtilityButton> saveButton;
    ScopedPointer<UtilityButton> saveAsButton;
    ScopedPointer<UtilityButton> loadButton;
    ScopedPointer<UtilityButton> newButton;
    ScopedPointer<UtilityButton> editButton;
    ScopedPointer<UtilityButton> delButton;
    ScopedPointer<UtilityButton> onButton;
    ScopedPointer<UtilityButton> newcircButton;
    ScopedPointer<UtilityButton> circlesButton[MAX_CIRCLES];
    ScopedPointer<UtilityButton> uniformButton;
    ScopedPointer<UtilityButton> gaussianButton;
    ScopedPointer<UtilityButton> negFirstButton;
    ScopedPointer<UtilityButton> posFirstButton;
    ScopedPointer<UtilityButton> biphasicButton;
    ScopedPointer<UtilityButton> chan1Button;
    ScopedPointer<UtilityButton> chan2Button;
    ScopedPointer<UtilityButton> chan3Button;
    ScopedPointer<UtilityButton> chan4Button;

    ScopedPointer<UtilityButton> simTrajectoryButton;

    // Label with non-editable text
    ScopedPointer<Label> cxLabel;
    ScopedPointer<Label> cyLabel;
    ScopedPointer<Label> cradLabel;
    ScopedPointer<Label> onLabel;
    ScopedPointer<Label> fmaxLabel;
    ScopedPointer<Label> sdevLabel;
//    ScopedPointer<Label> elecLabel;
    ScopedPointer<Label> pulsePalLabel;
    ScopedPointer<Label> phaseLabel;
    ScopedPointer<Label> interphaseLabel;
    ScopedPointer<Label> voltageLabel;
    ScopedPointer<Label> interpulseLabel;
    ScopedPointer<Label> repetitionsLabel;
    ScopedPointer<Label> trainDurationLabel;


    // Labels with editable test
    ScopedPointer<Label> cxEditLabel;
    ScopedPointer<Label> cyEditLabel;
    ScopedPointer<Label> cradEditLabel;
    ScopedPointer<Label> fmaxEditLabel;
    ScopedPointer<Label> sdevEditLabel;
//    ScopedPointer<Label> elecEditLabel;
    ScopedPointer<Label> phaseEditLabel;
    ScopedPointer<Label> interphaseEditLabel;
    ScopedPointer<Label> voltageEditLabel;
    ScopedPointer<Label> interpulseEditLabel;
    ScopedPointer<Label> repetitionsEditLabel;
    ScopedPointer<Label> trainDurationEditLabel;


private:
    TrackingStimulator* processor;
    float m_x;
    float m_y;
    float m_prevx;
    float m_prevy;
    float m_width;
    float m_height;

    float m_current_cx;
    float m_current_cy;
    float m_current_crad;

    bool m_onoff;
    bool m_updateCircle;
    bool m_isDeleting;

    Colour buttonTextColour;
    Colour labelColour;
    Colour labelTextColour;
    Colour labelBackgroundColour;
    Colour backgroundColour;

    ScopedPointer<DisplayAxes> m_ax;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackingStimulatorCanvas);
};

///**

//  Class for displaying and draw circles and current position

//*/

class DisplayAxes : public Component
{
public:
    DisplayAxes(TrackingStimulator* TrackingStimulator, TrackingStimulatorCanvas* TrackingStimulatorCanvas);
    ~DisplayAxes();

    void setXLims(double xmin, double xmax);
    void setYLims(double ymin, double ymax);

    void paint(Graphics& g);

    void clear();

    void mouseMove(const MouseEvent& event);
    void mouseEnter(const MouseEvent& event);
    void mouseExit(const MouseEvent& event);
    void mouseDown(const MouseEvent& event);
    void mouseUp(const MouseEvent& event);
    void mouseDrag(const MouseEvent& event);

    void copy();
    void paste();


private:

    double xlims[2];
    double ylims[2];

    TrackingStimulator* processor;
    TrackingStimulatorCanvas* canvas;

    Colour selectedCircleColour;
    Colour unselectedCircleColour;
    Colour backgroundColour;
    Colour outOfCirclesColour;
    Colour inOfCirclesColour;

    int64 click_time;

    bool m_firstPaint;

    bool m_creatingNewCircle;
    bool m_mayBeMoving;
    bool m_movingCircle;
    bool m_doubleClick;
    bool m_copy;

    float m_newX;
    float m_newY;
    float m_newRad;
    float m_tempRad;

    MouseCursor::StandardCursorType cursorType;

};

#endif // TRACKINGSTIMULATORCANVAS_H
