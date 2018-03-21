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


#include "TrackingStimulatorCanvas.h"
#include "TrackingStimulator.h"


TrackingStimulatorCanvas::TrackingStimulatorCanvas(TrackingStimulator* TrackingStimulator)
    : processor(TrackingStimulator)
    , m_x(0.5)
    , m_y(0.5)
    , m_width(1.0)
    , m_height(1.0)
    , m_updateCircle(true)
    , m_onoff(false)
    , m_isDeleting(true)
    , selectedSource(-1)
    , outputChan(0)
    , buttonTextColour(Colour(255,255,255))
    , labelColour(Colour(200, 255, 0))
    , labelTextColour(Colour(255, 200, 0))
    , labelBackgroundColour(Colour(100,100,100))
    , backgroundColour(Colours::black)
{
    // Setup buttons
    initButtons();
    // Setup Labels
    initLabels();

    addKeyListener(this);
    m_ax = new DisplayAxes(TrackingStimulator, this);

    startCallbacks();
    update();
}

TrackingStimulatorCanvas::~TrackingStimulatorCanvas()
{
    TopLevelWindow::getTopLevelWindow(0)->removeKeyListener(this);
}

float TrackingStimulatorCanvas::my_round(float x)
{
    return x < 0.0 ? ceil(x - 0.5) : floor(x + 0.5);
}

bool TrackingStimulatorCanvas::areThereCicles()
{
    if (processor->getCircles().size()>0)
        return true;
    else
        return false;
}

void TrackingStimulatorCanvas::paint (Graphics& g)
{
    if(m_x != m_x || m_y != m_y || m_width != m_width || m_height != m_height)
    { // is it nan?
        m_x = m_prevx;
        m_y = m_prevy;
    }

    float plot_height = 0.98*getHeight();
    float plot_width = 0.75*getWidth();
    float plot_bottom_left_x = 0.01*getWidth();
    float plot_bottom_left_y = 0.01*getHeight();

    float left_limit = 0.75*getWidth();

    // set aspect ratio to cam size
    float aC = m_width / m_height;
    float aS = plot_width / plot_height;
    int camHeight = (aS > aC) ? plot_height : plot_height * (aS / aC);
    int camWidth = (aS < aC) ? plot_width : plot_width * (aC / aS);

    g.setColour(Colours::black); // backbackround color
    g.fillRect(0, 0, getWidth(), getHeight());

    g.setColour(backgroundColour); // backbackround color
    g.fillRect(0, 0, getWidth(), getHeight());

    // on-off LED
    if (m_onoff)
        g.setColour(labelColour);
    else
        g.setColour(labelBackgroundColour);
    g.fillEllipse(getWidth() - 0.065*getWidth(), 0.41*getHeight(), 0.03*getWidth(), 0.03*getHeight());

    if (camWidth > left_limit)
    {
        camWidth = left_limit;
        camHeight = camWidth / aC;
    }

    m_ax->setBounds(int(plot_bottom_left_x), int(plot_bottom_left_y),
                    int(camWidth), int(camHeight));
    m_ax->repaint();

}

void TrackingStimulatorCanvas::resized()
{

//    m_ax->setBounds(int(0.01*getHeight()), int(0.01*getHeight()), int(0.98*getHeight()), int(0.98*getHeight()));
    addAndMakeVisible(m_ax);

    // Buttons
    saveButton->setBounds(getWidth() - 0.2*getWidth(), 0.9*getHeight(), 0.06*getWidth(),0.04*getHeight());
    saveAsButton->setBounds(getWidth() - 0.14*getWidth(), 0.9*getHeight(), 0.06*getWidth(),0.04*getHeight());
    loadButton->setBounds(getWidth() - 0.08*getWidth(), 0.9*getHeight(), 0.06*getWidth(),0.04*getHeight());

    simTrajectoryButton->setBounds(getWidth() - 0.2*getWidth(), 0.95*getHeight(), 0.09*getWidth(),0.04*getHeight());
    clearButton->setBounds(getWidth() - 0.2*getWidth() + 0.09*getWidth(), 0.95*getHeight(), 0.09*getWidth(),0.04*getHeight());

    newButton->setBounds(getWidth() - 0.2*getWidth(), 0.3*getHeight(), 0.06*getWidth(),0.04*getHeight());
    editButton->setBounds(getWidth() - 0.14*getWidth(), 0.3*getHeight(), 0.06*getWidth(),0.04*getHeight());
    delButton->setBounds(getWidth() - 0.08*getWidth(), 0.3*getHeight(), 0.06*getWidth(),0.04*getHeight());

    onButton->setBounds(getWidth() - 0.065*getWidth(), 0.46*getHeight(), 0.03*getWidth(),0.03*getHeight());

    for (int i = 0; i<MAX_CIRCLES; i++)
    {
        circlesButton[i]->setBounds(getWidth() - 0.2*getWidth()+i*(0.18/MAX_CIRCLES)*getWidth(), 0.5*getHeight(),
                                    (0.18/MAX_CIRCLES)*getWidth(),0.03*getHeight());
        if (i<processor->getCircles().size())
            circlesButton[i]->setVisible(true);
        else
            circlesButton[i]->setVisible(false);
    }

    uniformButton->setBounds(getWidth() - 0.2*getWidth(), 0.65*getHeight(), 0.09*getWidth(),0.03*getHeight());
    gaussianButton->setBounds(getWidth() - 0.2*getWidth() + 0.09*getWidth(), 0.65*getHeight(), 0.09*getWidth(),0.03*getHeight());

    availableChans->setBounds(getWidth() - 0.2*getWidth(), 0.05*getHeight(), 0.18*getWidth(),0.04*getHeight());
    outputChans->setBounds(getWidth() - 0.2*getWidth(), 0.15*getHeight(), 0.18*getWidth(),0.04*getHeight());

    // Static Labels
    sourcesLabel->setBounds(getWidth() - 0.2*getWidth(), 0.0*getHeight(), 0.08*getWidth(), 0.04*getHeight());
    outputLabel->setBounds(getWidth() - 0.2*getWidth(), 0.1*getHeight(), 0.08*getWidth(), 0.04*getHeight());
    circlesLabel->setBounds(getWidth() - 0.2*getWidth(), 0.25*getHeight(), 0.08*getWidth(), 0.04*getHeight());
    paramLabel->setBounds(getWidth() - 0.2*getWidth(), 0.6*getHeight(), 0.08*getWidth(), 0.04*getHeight());
    controlLabel->setBounds(getWidth() - 0.2*getWidth(), 0.85*getHeight(), 0.08*getWidth(), 0.04*getHeight());

    cxLabel->setBounds(getWidth() - 0.2*getWidth(), 0.35*getHeight(), 0.06*getWidth(),0.04*getHeight());
    cyLabel->setBounds(getWidth() - 0.2*getWidth(), 0.4*getHeight(), 0.06*getWidth(),0.04*getHeight());
    cradLabel->setBounds(getWidth() - 0.2*getWidth(), 0.45*getHeight(), 0.06*getWidth(),0.04*getHeight());

    fmaxLabel->setBounds(getWidth() - 0.2*getWidth(), 0.7*getHeight(), 0.1*getWidth(),0.04*getHeight());
    sdevLabel->setBounds(getWidth() - 0.2*getWidth(), 0.75*getHeight(), 0.1*getWidth(),0.04*getHeight());

    // Edit Labels
    cxEditLabel->setBounds(getWidth() - 0.14*getWidth(), 0.35*getHeight(), 0.06*getWidth(),0.04*getHeight());
    cyEditLabel->setBounds(getWidth() - 0.14*getWidth(), 0.4*getHeight(), 0.06*getWidth(),0.04*getHeight());
    cradEditLabel->setBounds(getWidth() - 0.14*getWidth(), 0.45*getHeight(), 0.06*getWidth(),0.04*getHeight());

    fmaxEditLabel->setBounds(getWidth() - 0.1*getWidth(), 0.7*getHeight(), 0.08*getWidth(),0.04*getHeight());
    sdevEditLabel->setBounds(getWidth() - 0.1*getWidth(), 0.75*getHeight(), 0.08*getWidth(),0.04*getHeight());
    refresh();
}

void TrackingStimulatorCanvas::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox == availableChans)
    {
        if (comboBox->getSelectedId() > 1)
        {
            int index = comboBox->getSelectedId() - 2;
            selectedSource = index;
        }
        else
            selectedSource = -1;
        processor->setSelectedSource(selectedSource);
    }
    else if (comboBox == outputChans)
    {
        if (comboBox->getSelectedId() > 0)
        {
            int index = comboBox->getSelectedId() - 1;
            outputChan = index;
        }
        else
            outputChan = -1;
        processor->setOutputChan(outputChan);
    }
}



bool TrackingStimulatorCanvas::keyPressed(const KeyPress &key, Component *originatingComponent)
{
    //copy/paste/delete of circles
    if (key.getKeyCode() == key.deleteKey)
        if (areThereCicles())
            delButton->triggerClick();
    if (key.getKeyCode() == 'c' && key.getModifiers() == ModifierKeys::ctrlModifier)
        if (areThereCicles())
            m_ax->copy();
    if (key.getKeyCode() == 'v' && key.getModifiers() == ModifierKeys::ctrlModifier)
        if (areThereCicles())
            m_ax->paste();
    return false;
}

void TrackingStimulatorCanvas::buttonClicked(Button* button)
{
    if (button == clearButton)
    {
        clear();
    }
    else if (button == saveButton)
    {
        processor->save();
    }
    else if (button == saveAsButton)
    {
        processor->saveAs();
    }
    else if (button == loadButton)
    {
        processor->load();
        // update labels and buttons
        uploadCircles();
    }
    else if (button == simTrajectoryButton)
    {
        if (simTrajectoryButton->getToggleState() == true)
        {
            processor->setSimulateTrajectory(true);
            m_width = 1;
            m_height = 1;
        }
        else
            processor->setSimulateTrajectory(false);
    }
    else if (button == newButton)
    {
        m_updateCircle = true;

        // Create new circle
        Value x = cxEditLabel->getTextValue();
        Value y = cyEditLabel->getTextValue();
        Value rad = cradEditLabel->getTextValue();
        if (processor->getCircles().size() < MAX_CIRCLES)
        {
            processor->addCircle(Circle(float(x.getValue()), float(y.getValue()), float(rad.getValue()), m_onoff));
            processor->setSelectedCircle(processor->getCircles().size()-1);
            circlesButton[processor->getSelectedCircle()]->setVisible(true);

            // toggle current circle button (untoggles all the others)
            if (circlesButton[processor->getSelectedCircle()]->getToggleState()==false)
                circlesButton[processor->getSelectedCircle()]->triggerClick();
            m_isDeleting = false;
        }
        else
            CoreServices::sendStatusMessage("Max number of circles reached!");


    }
    else if (button == editButton)
    {
        m_updateCircle = true;

        Value x = cxEditLabel->getTextValue();
        Value y = cyEditLabel->getTextValue();
        Value rad = cradEditLabel->getTextValue();
        if (areThereCicles())
            processor->editCircle(processor->getSelectedCircle(),x.getValue(),y.getValue(),rad.getValue(),m_onoff);
    }
    else if (button == delButton)
    {
        m_updateCircle = true;

        processor->deleteCircle(processor->getSelectedCircle());
        // make visible only the remaining labels
        for (int i = 0; i<MAX_CIRCLES; i++)
        {
            if (i<processor->getCircles().size())
                circlesButton[i]->setVisible(true);
            else
                circlesButton[i]->setVisible(false);
        }

        // Blank labels and untoggle all circle buttons
        processor->setSelectedCircle(-1);
        cxEditLabel->setText(String(""), dontSendNotification);
        cyEditLabel->setText(String(""), dontSendNotification);
        cradEditLabel->setText(String(""), dontSendNotification);
        m_onoff = false;

        for (int i = 0; i<MAX_CIRCLES; i++)
            //            circlesButton[i]->setEnabledState(false);
            circlesButton[i]->setToggleState(false, true);

    }
    else if (button == onButton)
    {
        m_onoff = !m_onoff;
        // make changes immediately if circle already exist
        if (processor->getSelectedCircle() != -1)
            editButton->triggerClick();
    }
    else if (button == uniformButton)
    {
        if (button->getToggleState()==true){
            if (gaussianButton->getToggleState()==true)
            {
                gaussianButton->triggerClick();
                sdevLabel->setVisible(false);
                sdevEditLabel->setVisible(false);
            }

            processor->setIsUniform(true);
        }
        else
            if (gaussianButton->getToggleState()==false)
            {
                gaussianButton->triggerClick();
                sdevLabel->setVisible(true);
                sdevEditLabel->setVisible(true);
            }
    }
    else if (button == gaussianButton)
    {
        if (button->getToggleState()==true){
            if (uniformButton->getToggleState()==true)
            {
                uniformButton->triggerClick();
                sdevLabel->setVisible(true);
                sdevEditLabel->setVisible(true);
            }

            processor->setIsUniform(false);
        }
        else
            if (uniformButton->getToggleState()==false)
            {
                uniformButton->triggerClick();
                sdevLabel->setVisible(false);
                sdevEditLabel->setVisible(false);
            }

    }
    else
    {
        // check if one of circle button has been clicked
        bool someToggled = false;
        for (int i = 0; i<MAX_CIRCLES; i++)
        {
            if (button == circlesButton[i] && circlesButton[i]->isVisible() )
            {
                // toggle button and untoggle all the others + update
                if (button->getToggleState()==true)
                {
                    for (int j = 0; j<MAX_CIRCLES; j++)
                        if (i!=j && circlesButton[j]->getToggleState()==true)
                        {
                            //                        circlesButton[j]->triggerClick();
                            //                            circlesButton[j]->setEnabledState(false);
                            circlesButton[j]->setToggleState(false, true);

                        }
                    processor->setSelectedCircle(i);
                    someToggled = true;
                    // retrieve labels and on button values
                    if (areThereCicles())
                    {
                        cxEditLabel->setText(String(processor->getCircles()[processor->getSelectedCircle()].getX()), dontSendNotification);
                        cyEditLabel->setText(String(processor->getCircles()[processor->getSelectedCircle()].getY()), dontSendNotification);
                        cradEditLabel->setText(String(processor->getCircles()[processor->getSelectedCircle()].getRad()), dontSendNotification);
                        m_onoff = processor->getCircles()[processor->getSelectedCircle()].getOn();
                    }
                }
            }
        }
        if (!someToggled)
        {
            // blank labels
            processor->setSelectedCircle(-1);
            cxEditLabel->setText(String(""), dontSendNotification);
            cyEditLabel->setText(String(""), dontSendNotification);
            cradEditLabel->setText(String(""), dontSendNotification);
            m_onoff = false;

        }
    }
    repaint();
}

void TrackingStimulatorCanvas::uploadCircles()
{
    // circle buttons visible
    for (int i = 0; i<MAX_CIRCLES; i++)
    {
        if (i<processor->getCircles().size())
            circlesButton[i]->setVisible(true);
        else
            circlesButton[i]->setVisible(false);
    }
}

void TrackingStimulatorCanvas::labelTextChanged(Label *label)
{
    // instance a new circle only when the add new button is clicked
    if (label == cxEditLabel)
    {
        Value val = label->getTextValue();
        if (!(float(val.getValue())>=0 && float(val.getValue())<=1))
        {
            CoreServices::sendStatusMessage("Select value must be within 0 and 1!");
            label->setText("", dontSendNotification);
        }
    }
    if (label == cyEditLabel)
    {
        Value val = label->getTextValue();
        if (!(float(val.getValue())>=0 && (float(val.getValue())<=1)))
        {
            CoreServices::sendStatusMessage("Select value must be within 0 and 1!");
            label->setText("", dontSendNotification);
        }
    }
    if (label == cradEditLabel)
    {
        Value val = label->getTextValue();
        if (!(float(val.getValue())>=0 && (float(val.getValue())<=1)))
        {
            CoreServices::sendStatusMessage("Select value must be within 0 and 1!");
            label->setText("", dontSendNotification);
        }
    }
    if (label == fmaxEditLabel)
    {
        Value val = label->getTextValue();
        if ((float(val.getValue())>=0 && float(val.getValue())<=10000))
            processor->setStimFreq(float(val.getValue()));
        else
        {
            CoreServices::sendStatusMessage("Selected values cannot be negative!");
            label->setText("", dontSendNotification);
        }
    }
    if (label == sdevEditLabel)
    {
        Value val = label->getTextValue();
        if ((float(val.getValue())>=0 && float(val.getValue())<=1))
            if (float(val.getValue())>0)
                processor->setStimSD(float(val.getValue()));
            else
                processor->setStimSD(1e-10);
        else
        {
            CoreServices::sendStatusMessage("Selected values must be between 0 and 1!");
            label->setText("", dontSendNotification);
        }
    }
}


void TrackingStimulatorCanvas::refreshState()
{
}

void TrackingStimulatorCanvas::update()
{
    availableChans->clear();
    int nSources = processor->getNSources();
    std::cout << nSources << std::endl;
    int nextItem = 2;
    availableChans->addItem("None", 1);
    for (int i = 0; i < nSources; i++)
    {
        TrackingSources& source = processor->getTrackingSource(i);
        String name = source.name;
        availableChans->addItem(name, nextItem++);
    }

}

void TrackingStimulatorCanvas::refresh()
{

    if (processor->positionDisplayedIsUpdated())
    {
        processor->clearPositionDisplayedUpdated();
        m_prevx = m_x;
        m_prevy = m_y;
        if (processor->getSimulateTrajectory())
        {
            m_x = processor->getSimX();
            m_y = processor->getSimY();
            m_width = 1;
            m_height = 1;
        }
        else
        {
            m_x = processor->getX(selectedSource);
            m_y = processor->getY(selectedSource);
            m_width = processor->getWidth(selectedSource);
            m_height = processor->getHeight(selectedSource);
        }
        repaint();
    }
}

void TrackingStimulatorCanvas::beginAnimation()
{
    startCallbacks();
}

void TrackingStimulatorCanvas::endAnimation()
{
    stopCallbacks();
}

void TrackingStimulatorCanvas::initButtons()
{

    clearButton = new UtilityButton("Clear plot", Font("Small Text", 13, Font::plain));
    clearButton->setRadius(3.0f);
    clearButton->addListener(this);
    addAndMakeVisible(clearButton);

    saveButton = new UtilityButton("Save", Font("Small Text", 13, Font::plain));
    saveButton->setRadius(3.0f);
    saveButton->addListener(this);
    addAndMakeVisible(saveButton);

    saveAsButton = new UtilityButton("Save As", Font("Small Text", 13, Font::plain));
    saveAsButton->setRadius(3.0f);
    saveAsButton->addListener(this);
    addAndMakeVisible(saveAsButton);

    loadButton = new UtilityButton("Load", Font("Small Text", 13, Font::plain));
    loadButton->setRadius(3.0f);
    loadButton->addListener(this);
    addAndMakeVisible(loadButton);

    simTrajectoryButton = new UtilityButton("Simulate", Font("Small Text", 13, Font::plain));
    simTrajectoryButton->setRadius(3.0f);
    simTrajectoryButton->addListener(this);
    simTrajectoryButton->setClickingTogglesState(true);
    addAndMakeVisible(simTrajectoryButton);

    newButton = new UtilityButton("New", Font("Small Text", 13, Font::plain));
    newButton->setRadius(3.0f);
    newButton->addListener(this);
    addAndMakeVisible(newButton);

    editButton = new UtilityButton("Edit", Font("Small Text", 13, Font::plain));
    editButton->setRadius(3.0f);
    editButton->addListener(this);
    addAndMakeVisible(editButton);

    delButton = new UtilityButton("Del", Font("Small Text", 13, Font::plain));
    delButton->setRadius(3.0f);
    delButton->addListener(this);
    addAndMakeVisible(delButton);

    onButton = new UtilityButton("On", Font("Small Text", 13, Font::plain));
    onButton->setRadius(3.0f);
    onButton->addListener(this);
    addAndMakeVisible(onButton);

    availableChans = new ComboBox("Event Channels");

    availableChans->setEditableText(false);
    availableChans->setJustificationType(Justification::centredLeft);
    availableChans->addListener(this);
    availableChans->setSelectedId(0);

    addAndMakeVisible(availableChans);

    outputChans = new ComboBox("Output Channels");

    outputChans->setEditableText(false);
    outputChans->setJustificationType(Justification::centredLeft);
    outputChans->addListener(this);
    outputChans->setSelectedId(0);

    for (int i=1; i<9; i++)
        outputChans->addItem(String(i), i);

    outputChans->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(outputChans);


    // Create invisible circle toggle button
    for (int i = 0; i<MAX_CIRCLES; i++)
    {
        newcircButton = new UtilityButton(to_string(i+1), Font("Small Text", 13, Font::plain));
        newcircButton->setRadius(5.0f);
        newcircButton->addListener(this);
        newcircButton->setClickingTogglesState(true);
        circlesButton[i] = newcircButton;
        addAndMakeVisible(circlesButton[i]);
    }

    uniformButton = new UtilityButton("uni", Font("Small Text", 13, Font::plain));
    uniformButton->setRadius(3.0f);
    uniformButton->addListener(this);
    uniformButton->setClickingTogglesState(true);
    addAndMakeVisible(uniformButton);

    gaussianButton = new UtilityButton("gauss", Font("Small Text", 13, Font::plain));
    gaussianButton->setRadius(3.0f);
    gaussianButton->addListener(this);
    gaussianButton->setClickingTogglesState(true);
    addAndMakeVisible(gaussianButton);


    // Update button toggle state with current chan1 parameters
    if (processor->getIsUniform())
        uniformButton->triggerClick();
    else
        gaussianButton->triggerClick();

}

void TrackingStimulatorCanvas::initLabels()
{
    // Static Labels
    sourcesLabel = new Label("s_sources", "Input Source");
    sourcesLabel->setFont(Font(20));
    sourcesLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(sourcesLabel);

    outputLabel = new Label("s_output", "Output Channel");
    outputLabel->setFont(Font(20));
    outputLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(outputLabel);

    circlesLabel = new Label("s_circles", "Circles");
    circlesLabel->setFont(Font(20));
    circlesLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(circlesLabel);

    paramLabel = new Label("s_param", "Trigger parameters");
    paramLabel->setFont(Font(20));
    paramLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(paramLabel);

    controlLabel = new Label("s_control", "Control");
    controlLabel->setFont(Font(20));
    controlLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(controlLabel);


    cxLabel = new Label("s_cx", "xpos [%]:");
    cxLabel->setFont(Font(15));
    cxLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(cxLabel);

    cyLabel = new Label("s_cy", "ypos [%]:");
    cyLabel->setFont(Font(15));
    cyLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(cyLabel);

    cradLabel = new Label("s_crad", "radius [%]:");
    cradLabel->setFont(Font(15));
    cradLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(cradLabel);

    fmaxLabel = new Label("s_fmax", "fmax [Hz]:");
    fmaxLabel->setFont(Font(20));
    fmaxLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(fmaxLabel);

    sdevLabel = new Label("s_sdev", "sd [%]:");
    sdevLabel->setFont(Font(20));
    sdevLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(sdevLabel);
    if (processor->getIsUniform())
        sdevLabel->setVisible(false);
    else
        sdevLabel->setVisible(true);


    // Edit Labels
    cxEditLabel = new Label("cx", " ");
    cxEditLabel->setFont(Font(15));
    cxEditLabel->setColour(Label::textColourId, labelTextColour);
    cxEditLabel->setColour(Label::backgroundColourId, labelBackgroundColour);
    cxEditLabel->setEditable(true);
    cxEditLabel->addListener(this);
    addAndMakeVisible(cxEditLabel);

    cyEditLabel = new Label("cy", " ");
    cyEditLabel->setFont(Font(15));
    cyEditLabel->setColour(Label::textColourId, labelTextColour);
    cyEditLabel->setColour(Label::backgroundColourId, labelBackgroundColour);
    cyEditLabel->setEditable(true);
    cyEditLabel->addListener(this);
    addAndMakeVisible(cyEditLabel);

    cradEditLabel = new Label("crad", " ");
    cradEditLabel->setFont(Font(15));
    cradEditLabel->setColour(Label::textColourId, labelTextColour);
    cradEditLabel->setColour(Label::backgroundColourId, labelBackgroundColour);
    cradEditLabel->setEditable(true);
    cradEditLabel->addListener(this);
    addAndMakeVisible(cradEditLabel);

    fmaxEditLabel = new Label("fmax", String(DEF_FREQ));
    fmaxEditLabel->setFont(Font(20));
    fmaxEditLabel->setColour(Label::textColourId, labelTextColour);
    fmaxEditLabel->setColour(Label::backgroundColourId, labelBackgroundColour);
    fmaxEditLabel->setEditable(true);
    fmaxEditLabel->addListener(this);
    addAndMakeVisible(fmaxEditLabel);

    sdevEditLabel = new Label("sdev", String(DEF_SD));
    sdevEditLabel->setFont(Font(20));
    sdevEditLabel->setColour(Label::textColourId, labelTextColour);
    sdevEditLabel->setColour(Label::backgroundColourId, labelBackgroundColour);
    sdevEditLabel->setEditable(true);
    sdevEditLabel->addListener(this);
    addAndMakeVisible(sdevEditLabel);
    sdevEditLabel->setVisible(false);
}

int TrackingStimulatorCanvas::getSelectedSource() const
{
    return selectedSource;
}

void TrackingStimulatorCanvas::clear()
{
    // set all circles to off
    processor->disableCircles();
    repaint();
}

bool TrackingStimulatorCanvas::getUpdateCircle(){
    return m_updateCircle;
}

void TrackingStimulatorCanvas::setUpdateCircle(bool onoff){
    m_updateCircle = onoff;
}

void TrackingStimulatorCanvas::setOnButton()
{
    m_onoff = true;
}


void TrackingStimulatorCanvas::setParameter(int, float)
{
}

void TrackingStimulatorCanvas::setParameter(int, int, int, float)
{
}


// DisplayAxes methods

DisplayAxes::DisplayAxes(TrackingStimulator* TrackingStimulator, TrackingStimulatorCanvas* TrackingStimulatorCanvas)
    : processor(TrackingStimulator)
    , canvas(TrackingStimulatorCanvas)
    , selectedCircleColour(Colour(255,155,30))
    , unselectedCircleColour(Colour(200,255,30))
    , backgroundColour(Colour(0,18,43))
    , outOfCirclesColour(Colour(30, 200, 180))
    , inOfCirclesColour(Colour(255,30,30))
    , m_creatingNewCircle(false)
    , m_movingCircle(false)
    , m_mayBeMoving(false)
    , m_doubleClick(false)
    , m_firstPaint(true)
    , m_copy (false)

{
    xlims[0] = getBounds().getRight() - getWidth();
    xlims[1] = getBounds().getRight();
    ylims[0] = getBounds().getBottom() - getHeight();
    ylims[1] = getBounds().getBottom();

}

DisplayAxes::~DisplayAxes(){}

void DisplayAxes::paint(Graphics& g){

    xlims[0] = getBounds().getRight() - getWidth();
    xlims[1] = getBounds().getRight();
    ylims[0] = getBounds().getBottom() - getHeight();
    ylims[1] = getBounds().getBottom();

    g.setColour(backgroundColour); //background color
    g.fillAll();

    if (canvas->getUpdateCircle())
    {
        for (int i = 0; i < processor->getCircles().size(); i++)
        {
            // draw circle if it is ON
            if (processor->getCircles()[i].getOn())
            {
                float cur_x, cur_y, cur_rad;
                int x_c, y_c, x, y, radx, rady;

                cur_x = processor->getCircles()[i].getX();
                cur_y = processor->getCircles()[i].getY();
                cur_rad = processor->getCircles()[i].getRad();


                x_c = int(cur_x * getWidth() + xlims[0]);
                y_c = int(cur_y * getHeight() + ylims[0]);

                radx = int(cur_rad * getWidth());
                rady = int(cur_rad * getHeight());
                // center ellipse
                x = x_c - radx;
                y = y_c - rady;


                if (i==processor->getSelectedCircle())
                {
                    // if circle is being moved or changed size, don't draw static circle
                    if (!(m_movingCircle || m_doubleClick))
                    {
                        if (processor->getIsUniform())
                            g.setColour(selectedCircleColour);
                        else
                        {
                            ColourGradient Cgrad = ColourGradient(Colours::darkmagenta, double(x_c), double(y_c),
                                                                  Colours::yellow, double(x_c+radx), double(y_c+rady), true);
                            g.setGradientFill(Cgrad);
                        }
                        g.fillEllipse(x, y, 2*radx, 2*rady);
                    }
                }
                else
                {
                    if (processor->getIsUniform())
                        g.setColour(unselectedCircleColour);
                    else
                    {
                        ColourGradient Cgrad = ColourGradient(Colours::orange, double(x_c), double(y_c),
                                                              Colours::lightgoldenrodyellow, double(x_c+radx), double(y_c+rady), true);
                        g.setGradientFill(Cgrad);
                    }
                    g.fillEllipse(x, y, 2*radx, 2*rady);
                }
            }
        }
    }

    // Draw a point for the current position
    // if inside circle display in RED
    if (processor->getSimulateTrajectory())
    {
        float pos_x = processor->getSimX();
        float pos_y = processor->getSimY();

        if ((pos_x >= 0 && pos_x <= 1) && (pos_y >= 0 && pos_y <= 1))
        {
            int x, y;

            x = int(pos_x * getWidth() + xlims[0]);
            y = int(pos_y * getHeight() + ylims[0]);

            int circleIn = processor->isPositionWithinCircles(pos_x, pos_y);

            if (circleIn != -1 && processor->getCircles()[circleIn].getOn())
                g.setColour(inOfCirclesColour);
            else
                g.setColour(outOfCirclesColour);
            g.fillEllipse(x, y, 0.02*getHeight(), 0.02*getHeight());
        }
    }
    else if (canvas->getSelectedSource() != -1)
    {
        float pos_x = processor->getX(canvas->getSelectedSource());
        float pos_y = processor->getY(canvas->getSelectedSource());

        if ((pos_x >= 0 && pos_x <= 1) && (pos_y >= 0 && pos_y <= 1))
        {
            int x, y;

            x = int(pos_x * getWidth() + xlims[0]);
            y = int(pos_y * getHeight() + ylims[0]);

            int circleIn = processor->isPositionWithinCircles(pos_x, pos_y);

            if (circleIn != -1 && processor->getCircles()[circleIn].getOn())
                g.setColour(inOfCirclesColour);
            else
                g.setColour(outOfCirclesColour);
            g.fillEllipse(x, y, 0.02*getHeight(), 0.02*getHeight());
        }
    }


    // Draw moving, creating, copying or resizing circle
    if (m_creatingNewCircle || m_movingCircle || m_doubleClick || m_copy)
    {
        // draw circle increasing in size
        int x_c, y_c, x, y, radx, rady;

        x_c = int(m_newX * getWidth() + xlims[0]);
        y_c = int(m_newY * getHeight() + ylims[0]);
        radx = int(m_tempRad * getWidth());
        rady = int(m_tempRad * getHeight());
        // center ellipse
        x = x_c - radx;
        y = y_c - rady;

        if (processor->getIsUniform())
            g.setColour(unselectedCircleColour);
        else
        {
            ColourGradient Cgrad = ColourGradient(Colours::darkmagenta, double(x_c), double(y_c),
                                                  Colours::yellow, double(x_c+radx), double(y_c+rady), true);
            g.setGradientFill(Cgrad);
        }
        g.fillEllipse(x, y, 2*radx, 2*rady);

    }
}

void DisplayAxes::clear(){}

void DisplayAxes::mouseMove(const MouseEvent& event){
    if (m_creatingNewCircle)
    {
        // compute m_tempRad
        m_tempRad = sqrt((pow(float(event.x)/float(getWidth())-m_newX, 2) +
                          pow(float(event.y)/float(getHeight())-m_newY, 2)));
        repaint();
    }
    if (m_doubleClick)
    {
        // compute m_tempRad
        m_tempRad = sqrt((pow(float(event.x)/float(getWidth())-m_newX, 2) +
                          pow(float(event.y)/float(getHeight())-m_newY, 2)));
        if (canvas->areThereCicles())
        {
            m_newX = processor->getCircles()[processor->getSelectedCircle()].getX();
            m_newY = processor->getCircles()[processor->getSelectedCircle()].getY();
        }
        repaint();
    }
    if (m_copy)
    {
        m_newX = float(event.x)/float(getWidth());
        m_newY = float(event.y)/float(getHeight());

        // Check boundaries
        if (!(m_newX <= 1 && m_newX >= 0) || !(m_newY <= 1 && m_newY >= 0))
            m_copy = false;
        repaint();
    }

}

void DisplayAxes::mouseEnter(const MouseEvent& event){
    setMouseCursor(MouseCursor::CrosshairCursor);
}

void DisplayAxes::mouseExit(const MouseEvent& event){
    m_movingCircle = false;
    m_creatingNewCircle = false;
    m_mayBeMoving = false;
    m_doubleClick = false;
}

void DisplayAxes::mouseDown(const MouseEvent& event)
{
    if (!event.mods.isRightButtonDown())
    {
        // check previous click time
        int64 current = Time::currentTimeMillis();
        int circleIn = processor->isPositionWithinCircles(float(event.x)/float(getWidth()),
                                                          float(event.y)/float(getHeight()));
        if (m_doubleClick)
        {
            m_doubleClick = false;
            m_newRad = sqrt((pow(float(event.x)/float(getWidth())-m_newX, 2) +
                             pow(float(event.y)/float(getHeight())-m_newY, 2)));

            if (m_newRad > 0.005)
            {
                canvas->cradEditLabel->setText(String(m_newRad), dontSendNotification);

                // Add new circle
                canvas->setOnButton();
                canvas->editButton->triggerClick();
            }

        }
        // If on a circle -> select circle!
        else if (circleIn != -1)
        {
            if (canvas->circlesButton[circleIn]->getToggleState() != true)
                canvas->circlesButton[circleIn]->triggerClick();
            m_creatingNewCircle = false;
            m_newX = float(event.x)/float(getWidth());
            m_newY = float(event.y)/float(getHeight());
            m_mayBeMoving = true;

            if (current-click_time <= 300)
                m_doubleClick = true;
            else
            {
                m_mayBeMoving = true;
                m_doubleClick = false;
                click_time = Time::currentTimeMillis();
            }
        }
        else if (m_creatingNewCircle)
        {
            m_creatingNewCircle = false;
            setMouseCursor(MouseCursor::CrosshairCursor);

            m_newRad = sqrt((pow(float(event.x)/float(getWidth())-m_newX, 2) +
                             pow(float(event.y)/float(getHeight())-m_newY, 2)));

            if (m_newRad > 0.005)
            {
                canvas->cradEditLabel->setText(String(m_newRad), dontSendNotification);
                // Add new circle
                canvas->setOnButton();
                canvas->newButton->triggerClick();
            }

        }
        else  // Else -> create new circle (set center and drag radius)
        {
            m_creatingNewCircle = true;
            m_newX = float(event.x)/float(getWidth());
            m_newY = float(event.y)/float(getHeight());
            m_tempRad = 0.005;
            setMouseCursor(MouseCursor::DraggingHandCursor);
            canvas->cxEditLabel->setText(String(m_newX), dontSendNotification);
            canvas->cyEditLabel->setText(String(m_newY), dontSendNotification);
        }
    }
}

void DisplayAxes::mouseUp(const MouseEvent& event){
    if (m_movingCircle)
    {
        // Change to new center and edit circle
        m_newX = float(event.x)/float(getWidth());
        m_newY = float(event.y)/float(getHeight());
        canvas->cxEditLabel->setText(String(m_newX),dontSendNotification);
        canvas->cyEditLabel->setText(String(m_newY),dontSendNotification);

        canvas->editButton->triggerClick();
        m_movingCircle = false;
        m_doubleClick = false;
    }
}

void DisplayAxes::mouseDrag(const MouseEvent& event){
    if (m_mayBeMoving)
    {
        // if dragging is grater than 0.05 -> start moving circle
        // compute m_tempRad
        float cx = 0, cy = 0;
        if (canvas->areThereCicles())
        {
            cx = processor->getCircles()[processor->getSelectedCircle()].getX();
            cy = processor->getCircles()[processor->getSelectedCircle()].getY();
        }
        m_tempRad = sqrt((pow(float(event.x)/float(getWidth())-cx, 2) +
                          pow(float(event.y)/float(getHeight())-cy, 2)));

        if (m_tempRad > 0.02)
        {
            m_movingCircle = true;
            m_mayBeMoving = false;
        }
    }
    if (m_movingCircle)
    {
        m_newX = float(event.x)/float(getWidth());
        m_newY = float(event.y)/float(getHeight());

        if (canvas->areThereCicles())
            m_tempRad = processor->getCircles()[processor->getSelectedCircle()].getRad();

        // Check boundaries
        if (!(m_newX <= 1 && m_newX >= 0) || !(m_newY <= 1 && m_newY >= 0))
            m_movingCircle = false;
        repaint();
    }
}

void DisplayAxes::copy()
{
    m_copy = true;
    if (canvas->areThereCicles())
        m_tempRad = processor->getCircles()[processor->getSelectedCircle()].getRad();
}

void DisplayAxes::paste()
{
    m_copy = false;
    canvas->cxEditLabel->setText(String(m_newX), dontSendNotification);
    canvas->cyEditLabel->setText(String(m_newY), dontSendNotification);
    // Add new circle
    canvas->setOnButton();
    canvas->newButton->triggerClick();

}



