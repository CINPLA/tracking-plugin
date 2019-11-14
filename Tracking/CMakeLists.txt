 #plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

file (GLOB OSCPACK RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    oscpack/ip/*.h
    oscpack/ip/*.cpp
    oscpack/osc/*.h
    oscpack/osc/*.cpp
)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
    ${OSCPACK}
    TrackingMessage.h
    TrackingNode.cpp
    TrackingNode.h
    TrackingNodeEditor.cpp
    TrackingNodeEditor.h
    TrackingStimulator.cpp
    TrackingStimulator.h
    TrackingStimulatorEditor.cpp
    TrackingStimulatorEditor.h
    TrackingStimulatorCanvas.cpp
    TrackingStimulatorCanvas.h
    TrackingVisualizer.cpp
    TrackingVisualizer.h
    TrackingVisualizerEditor.cpp
    TrackingVisualizerEditor.h
    TrackingVisualizerCanvas.cpp
    TrackingVisualizerCanvas.h
)
