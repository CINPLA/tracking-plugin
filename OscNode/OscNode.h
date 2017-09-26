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

#ifndef OSCNODE_H
#define OSCNODE_H

#include <ProcessorHeaders.h>
#include "OscServer.h"

#include <stdio.h>
#include <queue>
#define BUFFER_SIZE 4096

class OscQueue
{
    public:

        OscQueue();
        ~OscQueue();

        void enqueueMsg(char* message, int64 ts);
        char* dequeueMsg(int64* ts);

        bool isEmpty();
        void clear();

        int getHead();
        int getTail();
        int getInBuffer();
        int getBufferSize();
        int getMessageSize();

        void setMsgInfo(int msgSize);

        bool checkQueuesConsistency();

    private:

        char m_buffer[BUFFER_SIZE];
        std::queue<int64> m_timestamps;
        int m_msgHead;
        int m_msgTail;
        int m_bufferLength;
        int m_msgInBuffer;
        // this implementation assumes that the message size is constant
        int m_byteCount;

        void enqueueTimestamp(int64 ts);
        int64 dequeueTimeStamp();
};

class OscNode : public GenericProcessor
{
public:

    OscNode();
    ~OscNode();

    AudioProcessorEditor* createEditor();

//    bool isSource();
    bool isReady();

    void receiveMessage(std::vector<float> message);

    void process(AudioSampleBuffer&, MidiBuffer&) override;
    int getNumEventChannels() const override;
    void updateSettings() override;
    void setAddress(String address);
    String address();
    void setPort(int port);
    int port();

    //debug
    int countin1sec;
    int m_msgInBufferIn1sec;
    int64 m_prevTime;
    int64 m_currentTime;
    float m_timePassed; // in seconds
    int64 m_prevTimeBuf;
    int64 m_currentTimeBuf;
    float m_timePassedBuf;

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

    OscQueue msgQueue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscNode);

};


#endif  // OSCNODE_H
