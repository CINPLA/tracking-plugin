/*
  ==============================================================================

    TrackingServer.h
    Created: 1 Oct 2015 11:16:33am
    Author:  mikkel

  ==============================================================================
*/

#ifndef TRACKINGSERVER_H_INCLUDED
#define TRACKINGSERVER_H_INCLUDED

#include "TrackingNode.h"

#include "oscpack/osc/OscOutboundPacketStream.h"
#include "oscpack/ip/IpEndpointName.h"
#include "oscpack/osc/OscReceivedElements.h"
#include "oscpack/osc/OscPacketListener.h"
#include "oscpack/ip/UdpSocket.h"

#include <unordered_map>
#include <memory>
class TrackingNode;

class TrackingServer: public osc::OscPacketListener,
    public Thread
{
public:
    TrackingServer (int port);
    ~TrackingServer();

    static std::shared_ptr<TrackingServer> getInstance (int port, bool justDelete = false);
    void run();
    int getIntOSC();
    float getFloatOSC();
    void addProcessor (TrackingNode* processor);
    void removeProcessor (TrackingNode* processor);

protected:
    virtual void ProcessMessage (const osc::ReceivedMessage& m, const IpEndpointName&);

private:
    TrackingServer (TrackingServer const&);
    void operator= (TrackingServer const&);

    int m_incomingPort;
    UdpListeningReceiveSocket m_listeningSocket;
    std::vector<TrackingNode*> m_processors;
    String m_address;
};


#endif  // TRACKINGSERVER_H_INCLUDED
