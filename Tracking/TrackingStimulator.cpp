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

#include "TrackingStimulator.h"
#include "TrackingStimulatorEditor.h"

#include "../../UI/EditorViewport.h"

TrackingStimulator::TrackingStimulator()
    : GenericProcessor("Tracking Stim")
    , m_isOn(false)
    , m_x(-1.0)
    , m_y(-1.0)
    , m_height(1.0)
    , m_width(1.0)
    , m_positionIsUpdated(false)
    , m_positionDisplayedIsUpdated(false)
    , m_simulateTrajectory(false)
    , m_selectedCircle(-1)
    , m_timePassed(0.0)
    , m_currentTime(0.0)
    , m_previousTime(0.0)
    , m_timePassed_sim(0.0)
    , m_currentTime_sim(0.0)
    , m_previousTime_sim(0.0)
    , m_count(0)
    , m_forward(true)
    , m_rad(0.0)
    , m_outputChan(0)
{

    setProcessorType (PROCESSOR_TYPE_FILTER);

    m_circles = vector<Circle>();

    m_stimFreq = DEF_FREQ;
    m_stimSD = DEF_SD;
    m_isUniform = 1;

}

TrackingStimulator::~TrackingStimulator()
{

}

AudioProcessorEditor* TrackingStimulator::createEditor()
{
    editor = new TrackingStimulatorEditor(this, true);
    return editor;
}

void TrackingStimulator::createEventChannels()
{
    EventChannel* ev = new EventChannel(EventChannel::TTL, 8, 1, CoreServices::getGlobalSampleRate(), this);
    ev->setName("Tracking stimulator TTL output" );
    ev->setDescription("Triggers when the tracking data is within selected regions");
    ev->setIdentifier ("dataderived.tracking.triggerdstimulation");
    eventChannelArray.add (ev);
}


// Setters - Getters

TrackingSources& TrackingStimulator::getTrackingSource(int s) const
{
    if (s < sources.size())
        return sources.getReference (s);
}

float TrackingStimulator::getX(int s) const
{
    if (s < sources.size())
        return sources[s].x_pos;
    else
        return -1;
}

float TrackingStimulator::getY(int s) const
{
    if (s < sources.size())
        return sources[s].y_pos;
    else
        return -1;
}

float TrackingStimulator::getSimX() const
{
    return m_simX;
}

float TrackingStimulator::getSimY() const
{
    return m_simY;
}

float TrackingStimulator::getWidth(int s) const
{
    if (s < sources.size())
        return sources[s].width;
    else
        return -1;
}
float TrackingStimulator::getHeight(int s) const
{
    if (s < sources.size())
        return sources[s].height;
    else
        return -1;
}

bool TrackingStimulator::getSimulateTrajectory() const
{
    return m_simulateTrajectory;
}

void TrackingStimulator::setSimulateTrajectory(bool sim)
{
    m_simulateTrajectory = sim;
}

vector<Circle> TrackingStimulator::getCircles()
{
    return m_circles;
}

void TrackingStimulator::addCircle(Circle c)
{
    m_circles.push_back(c);
}

void TrackingStimulator::editCircle(int ind, float x, float y, float rad, bool on)
{
    m_circles[ind].set(x,y,rad,on);
}

void TrackingStimulator::deleteCircle(int ind)
{
    m_circles.erase(m_circles.begin() + ind);
}

void TrackingStimulator::disableCircles()
{
    for(int i=0; i<m_circles.size(); i++)
        m_circles[i].off();
}

int TrackingStimulator::getSelectedCircle() const
{
    return m_selectedCircle;
}

void TrackingStimulator::setSelectedCircle(int ind)
{
    m_selectedCircle = ind;
}

int TrackingStimulator::getOutputChan() const
{
    return m_outputChan;
}

float TrackingStimulator::getStimFreq() const
{
    return m_stimFreq;
}
float TrackingStimulator::getStimSD() const
{
    return m_stimSD;
}
bool TrackingStimulator::getIsUniform() const
{
    if (m_isUniform)
        return true;
    else
        return false;
}

void TrackingStimulator::setOutputChan(int chan)
{
    m_outputChan = chan;
}

void TrackingStimulator::setSelectedSource(int source)
{
    m_selectedSource = source;
}

void TrackingStimulator::setStimFreq(float stimFreq)
{
    m_stimFreq = stimFreq;
}
void TrackingStimulator::setStimSD(float stimSD)
{
    m_stimSD = stimSD;
}
void TrackingStimulator::setIsUniform(bool isUniform)
{
    if (isUniform)
        m_isUniform = 1;
    else
        m_isUniform = 0;
}

void TrackingStimulator::updateSettings()
{
    sources.clear();
    TrackingSources s;
    int nEvents = getTotalEventChannels();

    for (int i = 0; i < nEvents; i++)
    {
        const EventChannel* event = getEventChannel(i);
        if (event->getName().compare("Tracking data") == 0)
        {
            s.eventIndex = event->getSourceIndex();
            s.sourceId =  event->getSourceNodeID();
            s.name = "Tracking source " + String(event->getSourceIndex()+1);
            s.color = String("None");
            s.x_pos = -1;
            s.y_pos = -1;
            s.width = -1;
            s.height = -1;
            sources.add (s);
        }
    }
}


void TrackingStimulator::process(AudioSampleBuffer&)
{
    if (!m_simulateTrajectory)
    {
        checkForEvents();
    }
    else
    {
        // simulate events at 60Hz
        m_currentTime_sim = Time::currentTimeMillis();
        m_timePassed_sim = float(m_currentTime_sim - m_previousTime_sim)/1000.0; // in seconds

        if (m_timePassed_sim >= float(1.0/TRACKING_FREQ))
        {
            // generate new position sample
            m_positionIsUpdated = true;
            float theta = float(m_count/20.);

            if (m_forward)
                if (m_rad < 0.5)
                    m_rad = m_rad + 0.002;
                else
                    m_forward = false;
            else
                if (m_rad > 0)
                    m_rad = m_rad - 0.002;
                else
                    m_forward = true;

            m_simX = m_rad*std::cos(theta) + 0.5;
            m_simY = m_rad*std::sin(theta) + 0.5;
            m_x = m_simX;
            m_y = m_simY;
            m_width = 1;
            m_height = 1;

            m_previousTime_sim = Time::currentTimeMillis();
            m_timePassed_sim = 0;
            m_count++;
            m_positionIsUpdated = true;
        }
    }

    if (m_isOn)
    {

        m_currentTime = Time::currentTimeMillis();
        m_timePassed = float(m_currentTime - m_previousTime) / 1000.f; // in seconds

        lock.enter();

        // Check if current position is within stimulation areas
        bool stim = stimulate();

        if (stim)
        {
            // Check if timePassed >= latency

            float stim_interval;
            if (m_isUniform)  {
                stim_interval = float(1.f / m_stimFreq);
            }
            else                     //gaussian
            {
                int circleIn = isPositionWithinCircles(m_x, m_y);
                float dist_norm = m_circles[circleIn].distanceFromCenter(m_x, m_y) / m_circles[circleIn].getRad();
                float k = -1.0 / std::log(m_stimSD);
                float freq_gauss = m_stimFreq*std::exp(-pow(dist_norm,2)/k);
                stim_interval = float(1/freq_gauss);
            }

            float stimulationProbability = m_timePassed / stim_interval;
            std::uniform_real_distribution<float> distribution(0.0, 1.0);
            float randomNumber = distribution(generator);

            if (stimulationProbability > 1)
                std::cout << "WARNING: The tracking stimulation frequency is higher than the sampling frequency." << std::endl;

            if (randomNumber < stimulationProbability) {
                int64 timestamp = CoreServices::getGlobalTimestamp();
                setTimestampAndSamples(timestamp, 0);
                uint8 ttlData = 1 << m_outputChan;
                const EventChannel* chan = getEventChannel(getEventChannelIndex(0, getNodeId()));
                TTLEventPtr event = TTLEvent::createTTLEvent(chan, timestamp, &ttlData, sizeof(uint8), m_outputChan);
                addEvent(chan, event, 0);
            }
        }

        m_previousTime = m_currentTime;

        lock.exit();
    }
}

void TrackingStimulator::handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int)
{
    if ((eventInfo->getName()).compare("Tracking data") != 0)
    {
        return;
    }

    BinaryEventPtr evtptr = BinaryEvent::deserializeFromMessage(event, eventInfo);

    int nodeId = evtptr->getSourceID();
    int evtId = evtptr->getSourceIndex();
    const auto *position = reinterpret_cast<const TrackingPosition *>(evtptr->getBinaryDataPointer());

    int nSources = sources.size ();

    for (int i = 0; i < nSources; i++)
    {
        TrackingSources& currentSource = sources.getReference (i);
        if (currentSource.sourceId == nodeId && evtId == currentSource.eventIndex)
        {
            if(!(position->x != position->x || position->y != position->y) && position->x != 0 && position->y != 0)
            {
                currentSource.x_pos = position->x;
                currentSource.y_pos = position->y;
            }
            if(!(position->width != position->width || position->height != position->height))
            {
                currentSource.width = position->width;
                currentSource.height = position->height;
            }

            String sourceColor;
            evtptr->getMetaDataValue(0)->getValue(sourceColor);

            if (currentSource.color.compare(sourceColor) != 0)
            {
                currentSource.color = sourceColor;

            }
        }
    }
    if (m_selectedSource != -1)
    {
        m_x = sources.getReference (m_selectedSource).x_pos;
        m_y = sources.getReference (m_selectedSource).y_pos;
        m_width = sources.getReference (m_selectedSource).width;
        m_height = sources.getReference (m_selectedSource).height;
        m_aspect_ratio = m_width / m_height;
    }
    else
    {
        m_x = -1;
        m_y = -1;
        m_width = 1;
        m_height = 1;
    }
    m_positionIsUpdated = true;
}

int TrackingStimulator::isPositionWithinCircles(float x, float y)
{
    int whichCircle = -1;
    for (int i = 0; i < m_circles.size() && whichCircle == -1; i++)
    {
        if (m_circles[i].isPositionIn(x,y))
            whichCircle = i;
    }
    return whichCircle;
}

bool TrackingStimulator::stimulate()
{
    if (isPositionWithinCircles(m_x, m_y) != -1)
    {
        return true;
    }
    else
        return false;
}

bool TrackingStimulator::positionDisplayedIsUpdated() const
{
    //return m_positionDisplayedIsUpdated;
    return m_positionIsUpdated;
}

void TrackingStimulator::clearPositionDisplayedUpdated()
{
    //m_positionDisplayedIsUpdated = false;
    m_positionIsUpdated = false;
}

int TrackingStimulator::getNSources() const
{
    return sources.size ();
}

bool TrackingStimulator::getColorIsUpdated() const
{
    return m_colorUpdated;
}

void TrackingStimulator::setColorIsUpdated(bool up)
{
    m_colorUpdated = up;
}

void TrackingStimulator::startStimulation()
{
    m_isOn = true;

}

void TrackingStimulator::stopStimulation()
{
    m_isOn = false;
}

bool TrackingStimulator::saveParametersXml()
{
    //Save
    XmlElement* state = new XmlElement("TrackingStimulator");

    // save circles
    XmlElement* circles = new XmlElement("CIRCLES");
    for (int i=0; i<m_circles.size(); i++)
    {
        XmlElement* circ = new XmlElement(String("Circles_")+=String(i));
        circ->setAttribute("id", i);
        circ->setAttribute("xpos", m_circles[i].getX());
        circ->setAttribute("ypos", m_circles[i].getY());
        circ->setAttribute("rad", m_circles[i].getRad());
        circ->setAttribute("on", m_circles[i].getOn());

        circles->addChildElement(circ);
    }
    // save stimulator conf
    XmlElement* stim = new XmlElement("STIMULATION");

    stim->setAttribute("freq", m_stimFreq);
    stim->setAttribute("sd", m_stimSD);
    stim->setAttribute("uniform-gaussian", m_isUniform);

    state->addChildElement(circles);
    state->addChildElement(stim);

    if (! state->writeToFile(currentConfigFile, String::empty))
        return false;
    else
        return true;


}

bool TrackingStimulator::loadParametersXml(File fileToLoad)
{
    File currentFile = fileToLoad;

    XmlDocument doc(currentFile);
    XmlElement* xml = doc.getDocumentElement();

    if (xml == 0 || ! xml->hasTagName("TrackingStimulator"))
    {
        std::cout << "File not found." << std::endl;
        delete xml;
        return false;
    }
    else
    {

        forEachXmlChildElement(*xml, element)
        {
            if (element->hasTagName("CIRCLES"))
            {
                m_circles.clear();
                forEachXmlChildElement(*element, element2)
                {
                    int id = element2->getIntAttribute("id");
                    double cx = element2->getDoubleAttribute("xpos");
                    double cy = element2->getDoubleAttribute("ypos");
                    double crad = element2->getDoubleAttribute("rad");
                    bool on = element2->getIntAttribute("on");

                    Circle newCircle = Circle((float) cx, (float) cy, (float) crad, on);
                    m_circles.push_back(newCircle);

                }
            }
            if (element->hasTagName("STIMULATION"))
            {
                double freq = element->getDoubleAttribute("freq");
                double sd = element->getDoubleAttribute("sd");
                int uni = element->getIntAttribute("uniform-gaussian");

                m_stimFreq = freq;
                m_stimSD = sd;
                m_isUniform = uni;

            }
        }
        return true;
    }
}

void TrackingStimulator::save()
{
    if (currentConfigFile.exists())
    {
        saveParametersXml();
    }
    else
    {
        FileChooser fc("Choose the file name...",
                       File::getCurrentWorkingDirectory(),
                       "*.xml");

        if (fc.browseForFileToSave(true))
        {
            currentConfigFile = fc.getResult();
            std::cout << currentConfigFile.getFileName() << std::endl;
            saveParametersXml();
        }
        else
        {
            CoreServices::sendStatusMessage("No file chosen!");
        }

    }
}

void TrackingStimulator::saveAs()
{
    FileChooser fc("Choose the file name...",
                   File::getCurrentWorkingDirectory(),
                   "*.xml");

    if (fc.browseForFileToSave(true))
    {
        currentConfigFile = fc.getResult();
        std::cout << currentConfigFile.getFileName() << std::endl;
        saveParametersXml();
    }
    else
    {
        CoreServices::sendStatusMessage("No file chosen!");
    }
}

void TrackingStimulator::load()
{
    FileChooser fc("Choose the file name...",
                   File::getCurrentWorkingDirectory(),
                   "*.xml");

    if (fc.browseForFileToOpen())
    {
        File fileToLoad = fc.getResult();
        std::cout << currentConfigFile.getFileName() << std::endl;
        loadParametersXml(fileToLoad);
    }
    else
    {
        CoreServices::sendStatusMessage("No file chosen!");
    }
}

void TrackingStimulator::saveCustomParametersToXml(XmlElement *parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("TrackingSTIMULATOR");
    // save circles
    XmlElement* circles = new XmlElement("CIRCLES");
    for (int i=0; i<m_circles.size(); i++)
    {
        XmlElement* circ = new XmlElement(String("Circles_")+=String(i));
        circ->setAttribute("id", i);
        circ->setAttribute("xpos", m_circles[i].getX());
        circ->setAttribute("ypos", m_circles[i].getY());
        circ->setAttribute("rad", m_circles[i].getRad());
        circ->setAttribute("on", m_circles[i].getOn());

        circles->addChildElement(circ);
    }
    // save stimulator conf
    XmlElement* channels = new XmlElement("CHANNELS");
    for (int i=0; i<4; i++)
    {
        XmlElement* chan = new XmlElement(String("Chan_")+=String(i+1));
        chan->setAttribute("id", i);
        chan->setAttribute("freq", m_stimFreq);
        chan->setAttribute("sd", m_stimSD);
        chan->setAttribute("uniform-gaussian", m_isUniform);
        channels->addChildElement(chan);
    }

    mainNode->addChildElement(circles);
    mainNode->addChildElement(channels);

}

void TrackingStimulator::loadCustomParametersFromXml()
{
    if (parametersAsXml != nullptr)
    {
        forEachXmlChildElement(*parametersAsXml, mainNode)
        {
            if (mainNode->hasTagName("TrackingSTIMULATOR"))
            {
                forEachXmlChildElement(*mainNode, element)
                {
                    if (element->hasTagName("CIRCLES"))
                    {
                        m_circles.clear();
                        forEachXmlChildElement(*element, element2)
                        {
                            int id = element2->getIntAttribute("id");
                            double cx = element2->getDoubleAttribute("xpos");
                            double cy = element2->getDoubleAttribute("ypos");
                            double crad = element2->getDoubleAttribute("rad");
                            bool on = element2->getIntAttribute("on");

                            Circle newCircle = Circle((float) cx, (float) cy, (float) crad, on);
                            m_circles.push_back(newCircle);

                        }
                        //                        break;
                    }
                    if (element->hasTagName("CHANNELS"))
                    {

                        forEachXmlChildElement(*element, element2)
                        {
                            int id = element2->getIntAttribute("id");
                            if (id<4) //pulse pal channels
                            {
                                double freq = element2->getDoubleAttribute("freq");
                                double sd = element2->getDoubleAttribute("sd");
                                int uni = element2->getIntAttribute("uniform-gaussian");

                                m_stimFreq = freq;
                                m_stimSD = sd;
                                m_isUniform = uni;
                            }

                        }
                        break;
                    }
                }

            }
        }
    }
}


// Circle methods

Circle::Circle()
    : m_cx(0),
      m_cy(0),
      m_rad(0)
{}

Circle::Circle(float x, float y, float rad, bool on)
{
    m_cx = x;
    m_cy = y;
    m_rad = rad;
    m_on = on;
}

float Circle::getX()
{
    return m_cx;
}
float Circle::getY()
{
    return m_cy;
}
float Circle::getRad()
{
    return m_rad;
}
bool Circle::getOn()
{
    return m_on;
}
void Circle::setX(float x)
{
    m_cx = x;
}
void Circle::setY(float y)
{
    m_cy = y;
}
void Circle::setRad(float rad)
{
    m_rad = rad;
}
void Circle::set(float x, float y, float rad, bool on)
{
    m_cx = x;
    m_cy = y;
    m_rad = rad;
    m_on = on;
}

bool Circle::on()
{
    m_on = true;
    return m_on;
}
bool Circle::off()
{
    m_on = false;
    return m_on;
}

bool Circle::isPositionIn(float x, float y)
{
    if (pow(x - m_cx,2) + pow(y - m_cy,2)
            <= m_rad*m_rad)
        return true;
    else
        return false;
}

float Circle::distanceFromCenter(float x, float y){
    return sqrt(pow(x - m_cx,2) + pow(y - m_cy,2));
}


