/*
  ==============================================================================

    ReceiveOSC.cpp
    Created: 1 Oct 2015 11:16:33am
    Author:  mikkel

  ==============================================================================
*/

#include "TrackingServer.h"
#include "TrackingNodeEditor.h"
#include "TrackingNode.h"
#include "TrackingMessage.h"

using std::cout;
using std::endl;

void TrackingServer::ProcessMessage (const osc::ReceivedMessage& receivedMessage,
                                     const IpEndpointName&)
{
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
        // 0 - x
        // 1 - y
        // 2 - box width
        // 3 - box height
        args >> trackingData.position.x;
        args >> trackingData.position.y;
        args >> trackingData.position.width;
        args >> trackingData.position.height;
        args >> osc::EndMessage;

        for (TrackingNode* processor : m_processors)
        {
            String address = processor->address();

            if ( std::strcmp ( receivedMessage.AddressPattern(), address.toStdString().c_str() ) != 0 )
            {
                continue;
            }

            processor->receiveMessage (trackingData);
        }
    }
    catch ( osc::Exception& e )
    {
        // any parsing errors such as unexpected argument types, or
        // missing arguments get thrown as exceptions.
        DBG ("error while parsing message: " << receivedMessage.AddressPattern() << ": " << e.what() << "\n");
    }
}

int TrackingServer::getIntOSC()
{
    int test = 1;
    return test;
}

float TrackingServer::getFloatOSC()
{
    float test = 2.19;
    return test;
}

void TrackingServer::addProcessor (TrackingNode* processor)
{
    m_processors.push_back (processor);
}

void TrackingServer::removeProcessor (TrackingNode* processor)
{
    m_processors.erase (std::remove (m_processors.begin(), m_processors.end(), processor), m_processors.end());
}

TrackingServer::TrackingServer (int port)
    : Thread ("OscListener Thread")
    , m_incomingPort (port)
    , m_listeningSocket (IpEndpointName ("localhost", m_incomingPort), this)
{}

TrackingServer::~TrackingServer()
{
    // stop the OSC Listener thread running
    m_listeningSocket.AsynchronousBreak();

    // allow the thread 2 seconds to stop cleanly - should be plenty of time.
    stopThread (2000);
}

std::shared_ptr<TrackingServer> TrackingServer::getInstance(int port, bool justDelete)
{
    // TODO Handle case where port cannot be assigned
    static std::unordered_map<int, std::shared_ptr<TrackingServer>> instances;

    std::vector<int> toDelete;

    for (auto r : instances)
    {
        if (r.first != port && r.second->m_processors.size() < 1)
        {
            toDelete.push_back (r.first);
        }
    }

    for (auto port : toDelete)
    {
        instances.erase (port);
    }

    if (justDelete)
    {
        // the function was invoked only to delete stale instances
        return nullptr;
    }

    if (instances.count (port) < 1)
    {
        try
        {
            instances[port] = std::make_shared<TrackingServer> (port);
        }
        catch (std::runtime_error& e)
        {
            DBG ("Error unable to bind port:");
            DBG (port);
        }
    }

    if (!instances[port]->isThreadRunning())
    {
        instances[port]->startThread();
    }

    return instances[port];
}

void TrackingServer::run()
{
    // Start the oscpack OSC Listener Thread
    // NOTE: s.Run() won't return unless we force it to with
    // s.AsynchronousBreak() as is done in the destructor
    DBG ("TrackingServer: Running thread");
    m_listeningSocket.Run();
}
