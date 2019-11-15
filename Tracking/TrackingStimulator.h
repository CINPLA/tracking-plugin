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

#ifndef TRACKINGSTIMULATOR_H
#define TRACKINGSTIMULATOR_H

#include <ProcessorHeaders.h>
#include "TrackingStimulatorEditor.h"
#include "TrackingMessage.h"

#include <vector>
#include <random>

#define DEF_PHASE_DURATION 1
#define DEF_INTER_PHASE 1
#define DEF_INTER_PULSE 5
#define DEF_REPETITIONS 1
#define DEF_TRAINDURATION 10
#define DEF_VOLTAGE 5
#define DEF_FREQ 2
#define DEF_SD 0.5
#define DEF_DUR 2

#define TRACKING_FREQ 20

#define MAX_CIRCLES 9

/**

  Class for Abstrac Stimulation Area

*/
class StimArea
{
public:
    StimArea();
    StimArea(float x, float y, bool on);

    float getX();
    float getY();
    bool getOn();
    void setX(float x);
    void setY(float y);

    bool on();
    bool off();

    virtual bool isPositionIn(float x, float y) = 0;
    virtual float distanceFromCenter(float x, float y) = 0;
    virtual String returnType() = 0;

protected:

    float m_cx;
    float m_cy;
    bool m_on;

};
/**

  Class for Stimulation Circles

*/
class Circle : public StimArea
{
public:
    Circle();
    Circle(float x, float y, float r, bool on);

    float getRad();

    void setRad(float rad);
    void set(float x, float y, float rad, bool on);

    bool isPositionIn(float x, float y);
    float distanceFromCenter(float x, float y);
    String returnType();

private:
    float m_rad;
};

/**

  Class for Stimulation Rectangle

*/
class Rect : public StimArea
{
public:
    Rect();
    Rect(float x, float y, float w, float h, bool on);

    float getW();
    float getH();

    void setW(float w);
    void setH(float h);
    void set(float x, float y, float w, float h, bool on);

    bool isPositionIn(float x, float y) override;
    float distanceFromCenter(float x, float y) override;
    String returnType() override;

private:
    float m_w;
    float m_h;
};


typedef enum
{
  uniform,
  gauss,
  ttl
} stim_mode;

/**

    Select stimulation regions for closed-loop tracking stimulation.

    @see GenericProcessor, TrackingStimulatorEditor, TrackingStimulatorCanvas
*/
class TrackingStimulator : public GenericProcessor
{

public:

    TrackingStimulator();
    ~TrackingStimulator();

    AudioProcessorEditor* createEditor();

    void process(AudioSampleBuffer& buffer) override;
    void handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int) override;
    void saveCustomParametersToXml(XmlElement* parentElement) override;
    void loadCustomParametersFromXml() override;
    void updateSettings();

    void startStimulation();
    void stopStimulation();

    // Setter-Getters
    float getX(int s) const;
    float getY(int s) const;
    float getSimX() const;
    float getSimY() const;
    float getWidth(int s) const;
    float getHeight(int s) const;

    int getNSources() const;
    TrackingSources& getTrackingSource(int s) const;

    std::vector<Circle> getCircles();
    void addCircle(Circle c);
    void editCircle(int ind, float x, float y, float rad, bool on);
    void deleteCircle(int ind);
    void disableCircles();
    // Circle setter can be done using Cicle class public methods
    int getSelectedCircle() const;
    void setSelectedCircle(int ind);

    bool getSimulateTrajectory() const;
    int getOutputChan() const;
    int getSelectedSource() const;

    float getStimFreq() const;
    float getStimSD() const;
//    bool getIsUniform() const;
    stim_mode getStimMode() const;
    int getTtlDuration() const;

    void setSimulateTrajectory(bool sim);
    void setOutputChan(int chan);
    void setSelectedSource(int source);

    void setStimFreq(float stimFreq);
    void setStimSD(float stimSD);
//    void setIsUniform(bool isUniform);
    void setStimMode(stim_mode mode);
    void setTtlDuration(int dur);

    void clearPositionDisplayedUpdated();
    bool positionDisplayedIsUpdated() const;
    bool getColorIsUpdated() const;
    void setColorIsUpdated(bool up);

    int isPositionWithinCircles(float x, float y);

    void save();
    void saveAs();
    void load();

protected:
    void createEventChannels() override;

private:

    CriticalSection lock;
    Array<TrackingSources> sources;

    // OnOff
    bool m_isOn;

    // Time stim
    float m_timePassed;
    int64 m_previousTime;
    int64 m_currentTime;
    bool m_ttlTriggered;

    std::default_random_engine generator;


    // Time sim position
    float m_timePassed_sim;
    int64 m_previousTime_sim;
    int64 m_currentTime_sim;
    int m_count;
    bool m_forward;
    float m_rad;

    // Current Position
    float m_x;
    float m_y;
    float m_simX;
    float m_simY;
    float m_width;
    float m_height;
    float m_aspect_ratio;
    bool m_positionIsUpdated;
    bool m_positionDisplayedIsUpdated;
    bool m_simulateTrajectory;
    bool m_colorUpdated;

    std::vector<Circle> m_circles;
    int m_selectedCircle;

    // Stimulation params
    float m_stimFreq;
    float m_stimSD;
//    int m_isUniform;
    stim_mode m_stimMode;
    int m_pulseDuration;

    // Selected stimulation chan
    int m_outputChan;
    int m_selectedSource;

    File currentConfigFile;

    // Stimulate decision
    bool stimulate();
    void triggerEvent();

    bool saveParametersXml();
    bool loadParametersXml(File loadFile);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackingStimulator);
};


#endif // TRACKINGSTIMULATOR_H


