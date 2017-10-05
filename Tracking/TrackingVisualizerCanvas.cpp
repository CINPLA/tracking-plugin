/*
  ==============================================================================

    TrackingVisualizerCanvas.cpp
    Created: 5 Oct 2015 12:09:00pm
    Author:  mikkel

  ==============================================================================
*/

#include "TrackingVisualizerCanvas.h"
#include "TrackingVisualizer.h"

#include <math.h>
#include <string>

Position::Position(float xin, float yin, float widthin, float heightin)
    : x(xin)
    , y(yin)
    , width(widthin)
    , height(heightin)
{

}

TrackingVisualizerCanvas::TrackingVisualizerCanvas(TrackingVisualizer *TrackingVisualizer)
    : processor(TrackingVisualizer)
    , m_width(1.0)
    , m_height(1.0)
    , m_prevSet(false)
    , m_img_scale(80)
    , m_imgExists(false)
    , redColour(Colour(200, 30, 30))
    , greenColour(Colour(30, 200, 30))
    , yellowColour(Colour(200, 200, 30))
    , defaultColour(Colour(100,100,100))
    , backgroundColour(Colour(0,18,43))
{
    initButtonsAndLabels();

    for (int i = 0; i<MAX_SOURCES; i++)
    {
        m_active[i] = false;
        switch(i)
        {
        case 0:
            m_colour[i] = redColour;
            break;
        case 1:
            m_colour[i] = greenColour;
            break;
        case 2:
            m_colour[i] = yellowColour;
            break;
        default:
            m_colour[i] = defaultColour;
            break;
        }
    }

    /*String path = "/home/alessiob/Documents/Codes/C++/OpenEphys/open-ephys-plugin-GUI/Source/Plugins/TrackingVisualizer/rodent.png";
    File file(path);
    File f = File::getCurrentWorkingDirectory();
    String fullpath = f.getFullPathName();
    StringArray tokens;
    tokens.addTokens(fullpath, "_", "\"");

    std::cout << fullpath << std::endl;

    if (file.existsAsFile())
        m_imgExists = true;*/

    if (m_imgExists)
    {
        File f = File("/home/alessiob/Documents/Codes/C++/OpenEphys/open-ephys-plugin-GUI/Source/Plugins/TrackingVisualizer/rodent.png");
        rodentImg = ImageFileFormat::loadFrom(f).rescaled(m_img_scale, m_img_scale);
    }

    startCallbacks();
    update();
}

TrackingVisualizerCanvas::~TrackingVisualizerCanvas()
{
    TopLevelWindow::getTopLevelWindow(0)->removeKeyListener(this);
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

    g.setColour(backgroundColour); //background color
    g.fillRect(int(plot_bottom_left_x), int(plot_bottom_left_y),
               int(camWidth), int(camHeight));


    float currentX[MAX_SOURCES];
    float currentY[MAX_SOURCES];

    for (int i = 0; i<processor->getNSources(); i++)
    {
        if (m_positions[i].size() >= 2 && m_active[i])
        {
            for(auto it = m_positions[i].begin()+1; it != m_positions[i].end(); it++) {
                Position position = *it;
                Position prev_position = *(it-1);

                float x = camWidth*position.x + plot_bottom_left_x;
                float y = camHeight*position.y + plot_bottom_left_y;
                float x_prev = camWidth*prev_position.x + plot_bottom_left_x;
                float y_prev = camHeight*prev_position.y + plot_bottom_left_y;

                g.setColour(m_colour[i]);
                g.drawLine(x_prev, y_prev, x, y, 5.0f);
            }
        }


        if (!m_positions[i].empty() && !sameButton->getToggleState())
        {
            Position currentPos = m_positions[i].back();
            float x = camWidth*currentPos.x + plot_bottom_left_x;
            float y = camHeight*currentPos.y + plot_bottom_left_y;

            g.setColour(m_colour[i]);

            if (m_imgExists)
                g.drawImageAt(rodentImg, x-(m_img_scale/2.), y-(m_img_scale/2.));
            else
                g.fillEllipse(x, y, 0.02*getHeight(), 0.02*getHeight());
        }
        else if ((!m_positions[i].empty() && sameButton->getToggleState()))
        {
            currentX[i] = m_positions[i].back().x;
            currentY[i] = m_positions[i].back().y;
        }
    }

    if (sameButton->getToggleState())
    {
        float meanx = 0;
        float meany = 0;
        int nonEmptySources = 0;
        for (int i = 0; i<processor->getNSources(); i++)
        {
            if (!m_positions[i].empty())
            {
                meanx += currentX[i];
                meany += currentY[i];
                nonEmptySources++;
            }
        }
        if (nonEmptySources > 0)
        {
            meanx /= processor->getNSources();
            meany /= processor->getNSources();
            float x = camWidth*meanx + plot_bottom_left_x;
            float y = camHeight*meany + plot_bottom_left_y;

            g.setColour(defaultColour);

            if (m_imgExists)
                g.drawImageAt(rodentImg, x-(m_img_scale/2.), y-(m_img_scale/2.));
            else
                g.fillEllipse(x, y, 0.02*getHeight(), 0.02*getHeight());
        }
    }

    g.setFont(Font("Default", 16, Font::plain));

}

void TrackingVisualizerCanvas::resized()
{
    clearButton->setBounds(0.01*getWidth(), getHeight()-0.05*getHeight(), 0.1*getWidth(), 0.03*getHeight());
    redButton->setBounds(0.01*getWidth(), getHeight()-0.2*getHeight(), 0.1*getWidth(), 0.03*getHeight());
    greenButton->setBounds(0.01*getWidth(), getHeight()-0.24*getHeight(), 0.1*getWidth(), 0.03*getHeight());
    yellowButton->setBounds(0.01*getWidth(), getHeight()-0.28*getHeight(), 0.1*getWidth(), 0.03*getHeight());

    sourcesLabel->setBounds(0.01*getWidth(), getHeight()-0.54*getHeight(), 0.1*getWidth(), 0.03*getHeight());

    for (int i = 0; i<MAX_SOURCES; i++)
    {
        sourcesButton[i]->setBounds(0.01*getWidth() + i*(0.1/MAX_SOURCES)*getWidth(), getHeight()-0.5*getHeight(),
                                    (0.1/float(MAX_SOURCES))*getWidth(),0.03*getHeight());
        if (i < processor->getNSources())
            sourcesButton[i]->setVisible(true);
        else
            sourcesButton[i]->setVisible(false);
    }

    sameButton->setBounds(0.01*getWidth() + 0.025*getWidth(), getHeight()-0.45*getHeight(), 0.05*getWidth(), 0.03*getHeight());

    refresh();

}


bool TrackingVisualizerCanvas::keyPressed(const KeyPress &key, Component *originatingComponent)
{
    return false;
}

void TrackingVisualizerCanvas::buttonClicked(Button* button)
{
    if (button == clearButton)
    {
        clear();
    }
    else if (button == redButton)
    {
        if (redButton->getToggleState())
        {
            if (greenButton->getToggleState())
                greenButton->triggerClick();
            if (yellowButton->getToggleState())
                yellowButton->triggerClick();
            for (int i = 0; i<MAX_SOURCES; i++)
            {
                if (sourcesButton[i]->isVisible() && m_active[i])
                {
                    m_colour[i] = redColour;
                }
            }
        }
    }
    else if (button == greenButton)
    {
        if (greenButton->getToggleState())
        {
            if (redButton->getToggleState())
                redButton->triggerClick();
            if (yellowButton->getToggleState())
                yellowButton->triggerClick();
            for (int i = 0; i<MAX_SOURCES; i++)
            {
                if (sourcesButton[i]->isVisible() && m_active[i])
                {
                    m_colour[i] = greenColour;
                }
            }
        }
    }
    else if (button == yellowButton)
    {
        if (yellowButton->getToggleState())
        {
            if (greenButton->getToggleState())
                greenButton->triggerClick();
            if (redButton->getToggleState())
                redButton->triggerClick();

            for (int i = 0; i<MAX_SOURCES; i++)
            {
                if (sourcesButton[i]->isVisible() && m_active[i])
                {
                    m_colour[i] = yellowColour;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i<MAX_SOURCES; i++)
        {
            if (button == sourcesButton[i] && sourcesButton[i]->isVisible() )
            {
                // toggle button and untoggle all the others + update
                if (button->getToggleState()==true)
                    m_active[i] = true;
                else
                    m_active[i] = false;
            }
        }
    }
}

void TrackingVisualizerCanvas::comboBoxChanged(ComboBox *comboBoxThatHasChanged)
{
}

void TrackingVisualizerCanvas::refreshState()
{
}

void TrackingVisualizerCanvas::update()
{
}

void TrackingVisualizerCanvas::refresh()
{
    if (processor->positionIsUpdated()) {
        for (int i = 0; i<processor->getNSources(); i++)
        {
            m_positions[i].push_back(Position(processor->getX(i), processor->getY(i), processor->getWidth(i), processor->getHeight(i)));
            // for now, just pick one w and h
            m_height = processor->getHeight(i);
            m_width = processor->getWidth(i);

            sourcesButton[i]->setVisible(true);
            //m_active[i] = true;
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

    m_prevSet = false;
    repaint();
}

void TrackingVisualizerCanvas::initButtonsAndLabels()
{
    clearButton = new UtilityButton("Clear plot", Font("Small Text", 13, Font::plain));
    clearButton->setRadius(3.0f);
    clearButton->addListener(this);
    addAndMakeVisible(clearButton);

    redButton = new UtilityButton("red", Font("Small Text", 13, Font::plain));
    redButton->setRadius(3.0f);
    redButton->addListener(this);
    redButton->setClickingTogglesState(true);
    addAndMakeVisible(redButton);

    greenButton = new UtilityButton("green", Font("Small Text", 13, Font::plain));
    greenButton->setRadius(3.0f);
    greenButton->addListener(this);
    greenButton->setClickingTogglesState(true);
    addAndMakeVisible(greenButton);

    yellowButton = new UtilityButton("yellow", Font("Small Text", 13, Font::plain));
    yellowButton->setRadius(3.0f);
    yellowButton->addListener(this);
    yellowButton->setClickingTogglesState(true);
    addAndMakeVisible(yellowButton);

    ScopedPointer<UtilityButton> newsourceButton;

    // Create invisible circle toggle button
    for (int i = 0; i<MAX_SOURCES; i++)
    {
        newsourceButton = new UtilityButton(std::to_string(i+1), Font("Small Text", 13, Font::plain));
        newsourceButton->setRadius(5.0f);
        newsourceButton->addListener(this);
        newsourceButton->setClickingTogglesState(true);
        //newsourceButton->triggerClick();
        sourcesButton[i] = newsourceButton;
        addAndMakeVisible(sourcesButton[i]);
    }

    sameButton = new UtilityButton("same", Font("Small Text", 13, Font::plain));
    sameButton->setRadius(3.0f);
    sameButton->addListener(this);
    sameButton->setClickingTogglesState(true);
    addAndMakeVisible(sameButton);

    // Static Labels
    sourcesLabel = new Label("s_sources", "Sources");
    sourcesLabel->setFont(Font(30));
    sourcesLabel->setColour(Label::textColourId, Colour(200, 255, 0));
    addAndMakeVisible(sourcesLabel);
    sourcesLabel->setVisible(true);


}
