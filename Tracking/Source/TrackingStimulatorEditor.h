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

#ifndef TRACKINGSTIMULATOREDITOR_H
#define TRACKINGSTIMULATOREDITOR_H


#include <VisualizerEditorHeaders.h>
#include <EditorHeaders.h>

class TrackingStimulatorEditor : public VisualizerEditor
{
public:
    TrackingStimulatorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~TrackingStimulatorEditor();

    // Listener Interface
    void buttonEvent(Button* button);

    Visualizer* createNewCanvas();

private:
    // Stimulate button
    ScopedPointer<TextButton> stimulateButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackingStimulatorEditor);
};

#endif // TrackingSTIMULATOREDITOR_H
