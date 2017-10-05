#ifndef TRACKINGSTIMULATOR_H
#define TRACKINGSTIMULATOR_H

#include <ProcessorHeaders.h>
#include "TrackingStimulatorEditor.h"
#include "TrackingMessage.h"
#include "serial/PulsePal.h"

#include <vector>

#define DEF_PHASE_DURATION 1
#define DEF_INTER_PHASE 1
#define DEF_INTER_PULSE 5
#define DEF_REPETITIONS 1
#define DEF_TRAINDURATION 10
#define DEF_VOLTAGE 5
#define DEF_FREQ 2
#define DEF_SD 0.5

#define TRACKING_FREQ 20

#define MAX_CIRCLES 9

class Circle
{
public:
    Circle();
    Circle(float x, float y, float r, bool on);

    float getX();
    float getY();
    float getRad();
    bool getOn();

    void setX(float x);
    void setY(float y);
    void setRad(float rad);
    void set(float x, float y, float rad, bool on);

    bool on();
    bool off();

    bool isPositionIn(float x, float y);
    float distanceFromCenter(float x, float y);

private:

    float m_cx;
    float m_cy;
    float m_rad;
    bool m_on;

};

class TrackingStimulator : public GenericProcessor
{

public:

    enum priority {REPFIRST, TRAINFIRST};

    TrackingStimulator();
    ~TrackingStimulator();

    AudioProcessorEditor* createEditor();

    void process(AudioSampleBuffer& buffer) override;
    void handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int) override;
    void saveCustomParametersToXml(XmlElement* parentElement) override;
    void loadCustomParametersFromXml() override;

    // Pulse Pal
    bool updatePulsePal();
    bool testStimulation(); //test from Editor
    bool syncStimulation(int chan); //sync from editor
    void startStimulation();
    void stopStimulation();

    // Setter-Getters
    float getX() const;
    float getY() const;
    float getWidth() const;
    float getHeight() const;

    vector<Circle> getCircles();
    void addCircle(Circle c);
    void editCircle(int ind, float x, float y, float rad, bool on);
    void deleteCircle(int ind);
    void disableCircles();
    // Circle setter can be done using Cicle class public methods
    int getSelectedCircle() const;
    void setSelectedCircle(int ind);

    bool getSimulateTrajectory() const;

    int getChan() const;

    float getStimFreq(int chan) const;
    float getStimSD(int chan) const;
    bool getIsUniform(int chan) const;

    bool getIsBiphasic(int chan) const;
    bool getNegFirst(int chan) const;
    float getPhaseDuration(int chan) const;
    float getInterPhaseInt(int chan) const;
    float getVoltage(int chan) const;
    int getRepetitions(int chan) const;
    float getInterPulseInt(int chan) const;
    float getTrainDuration(int chan) const;

    uint32_t getPulsePalVersion() const;

    void setSimulateTrajectory(bool sim);

    void setStimFreq(int chan, float stimFreq);
    void setStimSD(int chan, float stimSD);
    void setIsUniform(int chan, bool isUniform);

    void setIsBiphasic(int chan, bool isBiphasic);
    void setNegFirst(int chan, bool negFirst);
    void setPhaseDuration(int chan, float phaseDuration);
    void setInterPhaseInt(int chan, float interPhaseInt);
    void setVoltage(int chan, float voltage);
    void setRepetitions(int chan, int rep);
    void setInterPulseInt(int chan, float interPulseInt);
    void setTrainDuration(int chan, float trainDuration);
    void setChan(int chan);
    void setTTLSyncChan(int chan);
    void setStimSyncChan(int chan);

    bool checkParameterConsistency(int chan);
    void setRepetitionsTrainDuration(int chan, priority whatFirst);

    void clearPositionDisplayedUpdated();
    bool positionDisplayedIsUpdated() const;

    int isPositionWithinCircles(float x, float y);

    bool isReady();


    void save();
    void saveAs();
    void load();

private:

    CriticalSection lock;

    // OnOff
    bool m_isOn;

    // Time stim
    float m_timePassed;
    int64 m_previousTime;
    int64 m_currentTime;

    // Tim sim position
    float m_timePassed_sim;
    int64 m_previousTime_sim;
    int64 m_currentTime_sim;
    int m_count;
    bool m_forward;
    float m_rad;

    // Current Position
    float m_x;
    float m_y;
    float m_width;
    float m_height;
    float m_aspect_ratio;
    bool m_positionIsUpdated;
    bool m_positionDisplayedIsUpdated;
    bool m_simulateTrajectory;

    vector<Circle> m_circles;
    int m_selectedCircle;

    // Stimulation params
    vector<float> m_stimFreq;
    vector<float> m_stimSD;
    vector<int> m_isUniform;

    // Pulse params
    vector<int> m_isBiphasic;
    vector<int> m_negativeFirst;
    vector<float> m_phaseDuration; // ms
    vector<float> m_interPhaseInt; // ms
    vector<int> m_repetitions;
    vector<float> m_trainDuration;
    vector<float> m_voltage; // V
    vector<float> m_interPulseInt; // ms

    // Selected stimulation chan
    int m_chan;
    int m_tot_chan;

    // Save sync event
    bool m_saveSync;
    int m_TTLSyncChan;
    int m_StimSyncChan;

    // PULSE PAL
    PulsePal m_pulsePal;
    uint32_t m_pulsePalVersion;

    File currentConfigFile;

    // Stimulate decision
    bool stimulate();

    bool saveParametersXml();
    bool loadParametersXml(File loadFile);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackingStimulator);
};


#endif // TRACKINGSTIMULATOR_H


