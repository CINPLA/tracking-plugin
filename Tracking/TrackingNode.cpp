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
    , m_positionIsUpdated (false)
    , m_isRecordingTimeLogged (false)
    , m_isAcquisitionTimeLogged (false)
    , m_received_msg (0)
{
    setProcessorType (PROCESSOR_TYPE_SOURCE);
    sendSampleCount = false;

    cout << "Adding module" << endl;
    //    auto module = new TrackingModule(27020, "/red", "red", this);

    //    trackingModules.add (module);

    lastNumInputs = 0;

}

TrackingNode::~TrackingNode()
{
    for (int i = 0; i< trackingModules.size (); i++)
    {
        auto *current = trackingModules.getReference(i);
        std::cout << "Removing source " << i << std::endl;
        delete current;
    }
}

AudioProcessorEditor* TrackingNode::createEditor()
{
    editor = new TrackingNodeEditor (this, true);
    return editor;
}

//Since the data needs a maximum buffer size but the actual number of read bytes might be less, let's
//add that info as a metadata field.
void TrackingNode::updateSettings()
{
    cout << "Updating settings!" << endl;
    moduleEventChannels.clear();
    for (int i = 0; i < trackingModules.size(); i++)
    {
        //It's going to be raw binary data, so let's make it uint8
        EventChannel* chan = new EventChannel (EventChannel::UINT8_ARRAY, 1, 16, CoreServices::getGlobalSampleRate(), this);
        chan->setName ("Tracking data");
        chan->setDescription ("Tracking data received from Bonsai. x, y, width, height");
        chan->setIdentifier ("external.tracking.rawData");
        chan->addEventMetaData(new MetaDataDescriptor(MetaDataDescriptor::CHAR, 15, "Color", "Tracking source color to be displayed", "channelInfo.extra"));
        chan->addEventMetaData(new MetaDataDescriptor(MetaDataDescriptor::INT32, 1, "Port", "Tracking source OSC port", "channelInfo.extra"));
        chan->addEventMetaData(new MetaDataDescriptor(MetaDataDescriptor::CHAR, 15, "Address", "Tracking source OSC address", "channelInfo.extra"));
        eventChannelArray.add (chan);
    }
    lastNumInputs = getNumInputs();
}

void TrackingNode::addSource (int port, String address, String color)
{
    cout << "Adding source" << port << endl;
    try
    {
        auto *module = new TrackingModule(port, address, color, this);
        trackingModules.add (module);
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "Add source: " << e.what() << std::endl;
    }

}

void TrackingNode::addSource ()
{
    cout << "Adding empty source" << endl;
    try
    {
        auto *module = new TrackingModule(this);
        trackingModules.add (module);
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "Add source: " << e.what() << std::endl;
    }

}

void TrackingNode::removeSource (int i)
{
    auto *current = trackingModules.getReference(i);
    trackingModules.remove(i);
    delete current;
}

bool TrackingNode::isPortUsed(int port)
{
    bool used = false;
    for (int i = 0; i < trackingModules.size (); i++)
    {
        auto *current = trackingModules.getReference(i);
        if (current->m_port == port)
            used = true;
    }
    return used;
}

void TrackingNode::setPort (int i, int port)
{
    if (i < 0 || i >= trackingModules.size ())
    {
        return;
    }

    auto *module = trackingModules.getReference (i);
    module->m_port = port;
    String address = module->m_address;
    String color = module->m_color;
    if (address.compare("") != 0)
    {
        delete module;
        try
        {
            module = new TrackingModule(port, address, color, this);
			trackingModules.set(i, module);
        }
        catch (const std::runtime_error& e)
        {
            std::cout << "Set port: " << e.what() << std::endl;
        }
    }
}

int TrackingNode::getPort(int i)
{
	if (i < 0 || i >= trackingModules.size()) {
		return -1;
	}

    auto *module = trackingModules.getReference (i);
    return module->m_port;
}

void TrackingNode::setAddress (int i, String address)
{
    if (i < 0 || i >= trackingModules.size ())
    {
        return;
    }

    auto *module = trackingModules.getReference (i);
    module->m_address = address;
    int port = module->m_port;
    String color = module->m_color;
    if (port != -1)
    {
        delete module;
        try
        {
            module = new TrackingModule(port, address, color, this);
			trackingModules.set(i, module);
        }
        catch (const std::runtime_error& e)
        {
            std::cout << "Set address: " << e.what() << std::endl;
        }
    }
}

String TrackingNode::getAddress(int i)
{
    if (i < 0 || i >= trackingModules.size ()) {
		return String("");
    }

    auto *module = trackingModules.getReference (i);
    return module->m_address;
}

void TrackingNode::setColor (int i, String color)
{
	if (i < 0 || i >= trackingModules.size())
	{
		return;
	}
	auto *module = trackingModules.getReference(i);
	module->m_color = color;
	trackingModules.set(i, module);

}

String TrackingNode::getColor(int i)
{
	if (i < 0 || i >= trackingModules.size()) {
		return String("");
	}
    
    auto *module = trackingModules.getReference (i);
    return module->m_color;
}

void TrackingNode::process (AudioSampleBuffer&)
{
    if (!m_positionIsUpdated)
    {
        return;
    }

    lock.enter();

    for (int i = 0; i < trackingModules.size (); i++)
    {
        auto *module = trackingModules.getReference (i);
        while (true) {
            auto *message = module->m_messageQueue->pop ();
            if (!message) {
                break;
            }

            setTimestampAndSamples (uint64(message->timestamp), 0);
            MetaDataValueArray metadata;
            MetaDataValuePtr color = new MetaDataValue(MetaDataDescriptor::CHAR, 15);
            color->setValue(module->m_color.toLowerCase());
            metadata.add(color);
            MetaDataValuePtr port = new MetaDataValue(MetaDataDescriptor::INT32, 1);
            port->setValue(module->m_port);
            metadata.add(port);
            MetaDataValuePtr address = new MetaDataValue(MetaDataDescriptor::CHAR, 15);
            address->setValue(module->m_address.toLowerCase());
            metadata.add(address);
            const EventChannel* chan = getEventChannel (getEventChannelIndex (i, getNodeId()));
            BinaryEventPtr event = BinaryEvent::createBinaryEvent (chan,
                                                                   message->timestamp,
                                                                   reinterpret_cast<uint8_t *>(&(message->position)),
                                                                   sizeof(TrackingPosition),
                                                                   metadata);
            addEvent (chan, event, 0);
        }
    }

    lock.exit();
    m_positionIsUpdated = false;

}

int TrackingNode::getTrackingModuleIndex(int port, String address)
{
    int index = -1;
    for (int i = 0; i < trackingModules.size (); i++)
    {
        auto *current  = trackingModules.getReference(i);
        if (current->m_port == port && current->m_address.compare(address) == 0)
            index = i;
    }
    return index;
}

int TrackingNode::getNSources()
{
    return trackingModules.size ();
}

void TrackingNode::receiveMessage (int port, String address, const TrackingData &message)
{
    int index = getTrackingModuleIndex(port, address);
    if (index != -1)
    {
        auto *selectedModule = trackingModules.getReference (index);

        lock.enter();

        if (CoreServices::getRecordingStatus())
        {
            if (!m_isRecordingTimeLogged)
            {
                m_received_msg = 0;
                m_startingRecTimeMillis =  Time::currentTimeMillis();
                m_isRecordingTimeLogged = true;
                std::cout << "Starting Recording Ts: " << m_startingRecTimeMillis << std::endl;
                selectedModule->m_messageQueue->clear();
                CoreServices::sendStatusMessage ("Clearing queue before start recording");
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
                std::cout << "Starting Acquisition at Ts: " << m_startingAcqTimeMillis << std::endl;
                selectedModule->m_messageQueue->clear();
                CoreServices::sendStatusMessage ("Clearing queue before start acquisition");
            }

            m_positionIsUpdated = true;

            // NOTE: We cannot trust the getGlobalTimestamp function because it can return
            // negative time deltas. The reason is unknown.
            int64 ts = CoreServices::getSoftwareTimestamp();

            TrackingData outputMessage = message;
            outputMessage.timestamp = ts;
            selectedModule->m_messageQueue->push (outputMessage);
            m_received_msg++;
        }
        else
            m_isAcquisitionTimeLogged = false;

        lock.exit();
    }

}

bool TrackingNode::isReady()
{
    return true;
}

void TrackingNode::saveCustomParametersToXml (XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement ("TrackingNode");
    for (int i = 0; i < trackingModules.size(); i++)
    {
        auto *module = trackingModules.getReference (i);
        XmlElement* source = new XmlElement("Source_"+String(i+1));
        source->setAttribute ("port", module->m_port);
        source->setAttribute ("address", module->m_address);
        source->setAttribute ("color", module->m_color);
        mainNode->addChildElement(source);
    }
}

void TrackingNode::loadCustomParametersFromXml ()
{
    trackingModules.clear();
    if (parametersAsXml == nullptr)
    {
        return;
    }

    forEachXmlChildElement (*parametersAsXml, mainNode)
    {
        if (mainNode->hasTagName ("TrackingNode"))
        {
            forEachXmlChildElement(*mainNode, source)
            {
                int port = source->getIntAttribute("port");
                String address = source->getStringAttribute("address");
                String color = source->getStringAttribute("color");

                addSource (port, address, color);
            }
        }
    }
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

// Class TrackingServer methods
TrackingServer::TrackingServer ()
    : Thread ("OscListener Thread")
    , m_incomingPort (0)
    , m_address ("")
{
}

TrackingServer::TrackingServer (int port, String address)
    : Thread ("OscListener Thread")
    , m_incomingPort (port)
    , m_address (address)
{
}

TrackingServer::~TrackingServer()
{
    cout << "Destructing tracking server" << endl;
    // stop the OSC Listener thread running
    //    m_listeningSocket->Break();
    // allow the thread 2 seconds to stop cleanly - should be plenty of time.
    cout << "Destructed tracking server" << endl;
    delete m_listeningSocket;
}

void TrackingServer::ProcessMessage (const osc::ReceivedMessage& receivedMessage,
                                     const IpEndpointName&)
{
    int64 ts = CoreServices::getGlobalTimestamp();
    try
    {
        uint32 argumentCount = 4;

        if ( receivedMessage.ArgumentCount() != argumentCount) {
            cout << "ERROR: TrackingServer received message with wrong number of arguments. "
                 << "Expected " << argumentCount << ", got " << receivedMessage.ArgumentCount() << endl;
            return;
        }

        for (uint32 i = 0; i < receivedMessage.ArgumentCount(); i++)
        {
            if (receivedMessage.TypeTags()[i] != 'f')
            {
                cout << "TrackingServer only support 'f' (floats), not '"
                     << receivedMessage.TypeTags()[i] << "'" << endl;
                return;
            }
        }

        osc::ReceivedMessageArgumentStream args = receivedMessage.ArgumentStream();

        TrackingData trackingData;

        // Arguments:
        args >> trackingData.position.x; // 0 - x
        args >> trackingData.position.y; // 1 - y
        args >> trackingData.position.width; // 2 - box width
        args >> trackingData.position.height; // 3 - box height
        args >> osc::EndMessage;

        for (TrackingNode* processor : m_processors)
        {
            //            String address = processor->address();

            if ( std::strcmp ( receivedMessage.AddressPattern(), m_address.toStdString().c_str() ) != 0 )
            {
                continue;
            }
            // add trackingmodule to receive message call: processor->receiveMessage (m_incomingPort, m_address, trackingData);
            processor->receiveMessage (m_incomingPort, m_address, trackingData);
        }
    }
    catch ( osc::Exception& e )
    {
        // any parsing errors such as unexpected argument types, or
        // missing arguments get thrown as exceptions.
        DBG ("error while parsing message: " << receivedMessage.AddressPattern() << ": " << e.what() << "\n");
    }
}

void TrackingServer::addProcessor (TrackingNode* processor)
{
    m_processors.push_back (processor);
}

void TrackingServer::removeProcessor (TrackingNode* processor)
{
    m_processors.erase (std::remove (m_processors.begin(), m_processors.end(), processor), m_processors.end());
}

void TrackingServer::run()
{
    cout << "SLeeping!" << endl;
    sleep(1000);
    cout << "Running!" << endl;
    // Start the oscpack OSC Listener Thread
    try {
        m_listeningSocket = new UdpListeningReceiveSocket(IpEndpointName("localhost", m_incomingPort), this);
        sleep(1000);
        m_listeningSocket->Run();
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception in TrackingServer::run(): " << e.what() << std::endl;
    }
}

void TrackingServer::stop()
{
    // Stop the oscpack OSC Listener Thread
    if (!isThreadRunning())
    {
        return;
    }

    m_listeningSocket->AsynchronousBreak();
}
