/*
  ==============================================================================

    ReceiveOSC.cpp
    Created: 1 Oct 2015 11:16:33am
    Author:  mikkel

  ==============================================================================
*/

#include "OscServer.h"
#include "OscEditor.h"
#include "OscNode.h"

using std::cout;
using std::endl;

void OscServer::ProcessMessage(const osc::ReceivedMessage& m,
                                const IpEndpointName& remoteEndpoint)
{
    (void) remoteEndpoint; // suppress unused parameter warning
//    DBG("Received message!");
//    DBG(m.AddressPattern());
    try{
        // example of parsing single messages. osc::OsckPacketListener
        // handles the bundle traversal.
        for(OscNode* processor : processors) {
            String address = processor->address();
            if( std::strcmp( m.AddressPattern(), address.toStdString().c_str() ) == 0 ) {
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
                std::vector<float> message;
                for(int i = 0; i < m.ArgumentCount(); i++) {
                    if(m.TypeTags()[i] == 'f') {
                        float argument;
                        args >> argument;
                        message.push_back(argument);
                    }
                    else if (m.TypeTags()[i] == 'i') {
                        osc::int32 argument;
                        args >> argument;
                        message.push_back(float(argument));
                    }
                    else {
                        cout << "OscServer: We only support floats or ints right now, not" << m.TypeTags()[i] << endl;
                        return;
                    }
                }
                args >> osc::EndMessage;


				processor->receiveMessage(message);
				countin1sec++;

				m_currentTime = Time::currentTimeMillis();
				m_timePassed = float(m_currentTime - m_prevTime) / 1000.0; // in seconds

				if (m_timePassed > 1.0)
				{
					m_timePassed = 0.0;
					m_prevTime = Time::currentTimeMillis();
					std::cout << "OSC msg in 1sec: " << countin1sec << std::endl;
					countin1sec = 0;
				}
                
            }
        }
    } catch( osc::Exception& e ){
        // any parsing errors such as unexpected argument types, or
        // missing arguments get thrown as exceptions.
        DBG("error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n");
    }
}

int OscServer::getIntOSC()
{
    int test = 1;
    return test;
}

float OscServer::getFloatOSC()
{
    float test = 2.19;
    return test;
}

void OscServer::addProcessor(OscNode *processor)
{
    processors.push_back(processor);
}

void OscServer::removeProcessor(OscNode *processor)
{
    processors.erase(std::remove(processors.begin(), processors.end(), processor), processors.end());
}

OscServer::OscServer(int port)
    : Thread("OscListener Thread")
    , incomingPort(port)
    , s(IpEndpointName("localhost",
                       incomingPort), this)
    //debug
    , countin1sec(0)
    , m_prevTime(0)
    , m_currentTime(0)
    , m_timePassed(0.0)
{}

OscServer::~OscServer()
{
    // stop the OSC Listener thread running
    s.AsynchronousBreak();

    // allow the thread 2 seconds to stop cleanly - should be plenty of time.
    stopThread(2000);
}
