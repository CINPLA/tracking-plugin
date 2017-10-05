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

#include "TrackingNode.h"
#include "TrackingNodeEditor.h"
#include "TrackingMessage.h"

//preallocate memory for msg
#define BUFFER_MSG_SIZE 256

using namespace std;

TrackingNode::TrackingNode()
    : GenericProcessor ("Tracking Port")
    , m_startingRecTimeMillis (0)
    , m_startingAcqTimeMillis (0)
    , eventId (0)
    , m_positionIsUpdated (false)
    , m_port (27020)
    , m_isRecordingTimeLogged (false)
    , m_isAcquisitionTimeLogged (false)
    , m_msgInfo (false)
    , m_msgSent (false)
    , m_msgToSend (0)
      //debug
    , m_cleared_queues (0)
    , m_received_msg (0)
    , countin1sec (0)
    , m_prevTime (0)
    , m_currentTime (0)
    , m_timePassed (0.0)
    , m_msgInBufferIn1sec (0)
    , m_prevTimeBuf (0)
    , m_currentTimeBuf (0)
    , m_timePassedBuf (0.0)
{
    setProcessorType (PROCESSOR_TYPE_SOURCE);
    sendSampleCount = false;

    try
    {
        TrackingServer::getInstance (m_port)->addProcessor (this);
    }
    catch (std::runtime_error)
    {
        DBG ("Unable to bind port");
    }

    //    memset(m_buffer, 0, BUFFER_SIZE);
}

TrackingNode::~TrackingNode()
{
    TrackingServer::getInstance (m_port)->removeProcessor (this);
    TrackingServer::getInstance (0, true);
}

AudioProcessorEditor* TrackingNode::createEditor()
{
    editor = new TrackingNodeEditor (this, true);
    return editor;
}

//Since the data needs a meximum buffer size but the actual number of read bytes might be less, let's
//add that info as a metadata field.
void TrackingNode::createEventChannels()
{
    //It's going to be raw binary data, so let's make it uint8
    EventChannel* chan = new EventChannel (EventChannel::UINT8_ARRAY, 1, 24, CoreServices::getGlobalSampleRate(), this);
    chan->setName ("Tracking data");
    chan->setDescription ("Tracking data received from Bonsai. x, y, width, height");
    chan->setIdentifier ("external.tracking.rawData");
    eventChannelArray.add (chan);
}

void TrackingNode::setAddress (String address)
{
    m_address = address;
}

String TrackingNode::address()
{
    return m_address;
}

void TrackingNode::setPort (int port)
{
    try
    {
        TrackingServer::getInstance (m_port)->removeProcessor (this);
        m_port = port;
        TrackingServer::getInstance (port)->addProcessor (this);
    }
    catch (std::runtime_error)
    {
        DBG ("Unable to bind port");
    }
}

int TrackingNode::port()
{
    return m_port;
}

void TrackingNode::process (AudioSampleBuffer&)
{
    if (!m_positionIsUpdated)
    {
        return;
    }

    lock.enter();

    while (true) {
        auto *message = messageQueue.pop ();
        if (!message) {
            break;
        }

        setTimestampAndSamples (uint64(message->timestamp), 0);

        //setTimestamp(events, tsptr);
        const EventChannel* chan = getEventChannel (getEventChannelIndex (0, getNodeId()));
        BinaryEventPtr event = BinaryEvent::createBinaryEvent (chan,
                                                               message->timestamp,
                                                               reinterpret_cast<uint8_t *>(message),
                                                               sizeof(TrackingData));
        addEvent (chan, event, 0);
        countin1sec++;
    }

    lock.exit();
    m_positionIsUpdated = false;

}

void TrackingNode::receiveMessage (const TrackingData &message)
{
    lock.enter();

    if (CoreServices::getRecordingStatus())
    {
        if (!m_isRecordingTimeLogged)
        {
            m_received_msg = 0;
            m_startingRecTimeMillis =  Time::currentTimeMillis();
            m_isRecordingTimeLogged = true;
            std::cout << "Starting Rec Ts: " << m_startingRecTimeMillis << std::endl;
            CoreServices::sendStatusMessage ("Clearing Queue in rectimelog " + String (m_startingRecTimeMillis));
            messageQueue.clear();
        }
    }
    else
    {
        m_isRecordingTimeLogged = false;
    }


    if (CoreServices::getAcquisitionStatus()) // && !CoreServices::getRecordingStatus())
    {
        if (!m_isAcquisitionTimeLogged)
        {
            m_startingAcqTimeMillis = Time::currentTimeMillis();
            m_isAcquisitionTimeLogged = true;
            std::cout << "Starting Acq Ts: " << m_startingAcqTimeMillis << std::endl;
            messageQueue.clear();
            CoreServices::sendStatusMessage ("Clearing Queue in acqtimelog " + String (m_startingAcqTimeMillis));
        }

        m_positionIsUpdated = true;

        int64 ts = CoreServices::getGlobalTimestamp();

        TrackingData outputMessage = message;
        outputMessage.timestamp = ts;
        messageQueue.push (outputMessage);

        m_received_msg++;
    }
    else
    {
        m_isAcquisitionTimeLogged = false;
        //msgQueue.clear();
        //CoreServices::sendStatusMessage("Clearing Queue in stop acq");
    }

    lock.exit();

}

bool TrackingNode::isReady()
{
    return true;
}

// Class TrackingQueue methods
TrackingQueue::TrackingQueue()
    : m_head (-1)
    , m_tail (-1)
{
    memset (m_buffer, 0, BUFFER_SIZE);
}

TrackingQueue::~TrackingQueue() {}

void TrackingQueue::push (const TrackingData &message)
{
    m_head = (m_head + 1) % BUFFER_SIZE;
    m_buffer[m_head] = message;
}

TrackingData* TrackingQueue::pop ()
{
    if (isEmpty())
        return nullptr;

    m_tail = (m_tail + 1) % BUFFER_SIZE;
    return &(m_buffer[m_tail]);

}

bool TrackingQueue::isEmpty()
{
    return m_head == m_tail;
}

void TrackingQueue::clear()
{
    m_tail = -1;
    m_head = -1;
}
