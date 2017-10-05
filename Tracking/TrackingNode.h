/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef TRACKINGNODE_H
#define TRACKINGNODE_H

#include <ProcessorHeaders.h>
#include "TrackingServer.h"
#include "TrackingMessage.h"

#include <stdio.h>
#include <queue>
#define BUFFER_SIZE 4096

/**
    This helper class allows stores input tracking data in a circular queue.
*/
class TrackingQueue
{
public:

    TrackingQueue();
    ~TrackingQueue();

    void push (const TrackingData &message);
    TrackingData *pop();

    bool isEmpty();
    void clear();

private:
    TrackingData m_buffer[BUFFER_SIZE];
    int m_head;
    int m_tail;
};

/**
    This source processor allows you to pipe tracking data via OSC signals from Bonsai tracker.

    @see TrackingNodeEditor
*/
class TrackingNode : public GenericProcessor
{
public:
    /** The class constructor, used to initialize any members. */
    TrackingNode();

    /** The class destructor, used to deallocate memory */
    ~TrackingNode();

    AudioProcessorEditor* createEditor();

    void receiveMessage (const TrackingData &message);

    /** Defines the functionality of the processor.

        The process method is called every time a new data buffer is available.

        Adds all the new serial data that is available to the event data buffer.
     */
    void process (AudioSampleBuffer&) override;

    /**
        This should only be run by the ProcessorGraph, before acquisition will be started.

        It tries to open the serial port previsouly specified by the setDevice and setBaudrate setters.

        Returns true on success, false if port could not be opened.
    */
    bool isReady() override;

    /**
        Called immediately after the end of data acquisition by the ProcessorGraph.

        It closes the open port serial port.
     */
    //    bool disable() override;

    //debug
    int countin1sec;
    int m_msgInBufferIn1sec;
    int64 m_prevTime;
    int64 m_currentTime;
    float m_timePassed; // in seconds
    int64 m_prevTimeBuf;
    int64 m_currentTimeBuf;
    float m_timePassedBuf;


    //    int getNumEventChannels() const override;
    //    void updateSettings() override;
    void setAddress (String address);
    String address();
    void setPort (int port);
    int port();

protected:
    void createEventChannels() override;

private:

    int64 m_startingRecTimeMillis;
    int64 m_startingAcqTimeMillis;
    juce::uint8 eventId;

    CriticalSection lock;

    std::vector<float> m_message;

    bool m_positionIsUpdated;
    bool m_msgInfo;
    bool m_isRecordingTimeLogged;
    bool m_isAcquisitionTimeLogged;
    bool m_msgSent;
    int m_msgToSend;
    int m_cleared_queues;
    int m_received_msg;

    String m_address;
    int m_port;

    TrackingQueue messageQueue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackingNode);

};


#endif  // TRACKINGNODE_H
