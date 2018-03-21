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

#ifndef TRACKINGNODEEDITOR_H
#define TRACKINGNODEEDITOR_H

#define MAX_SOURCES 10

#include <EditorHeaders.h>

class TrackingNodeEditor :
        public GenericEditor,
        public Label::Listener,
        public ComboBox::Listener
{
public:
    TrackingNodeEditor (GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~TrackingNodeEditor();

    virtual void labelTextChanged (Label* labelThatHasChanged) override;
    void buttonEvent(Button* button);
    virtual void comboBoxChanged (ComboBox* c) override;

    virtual void updateSettings();
    void updateLabels();

private:
    String color_palette[MAX_SOURCES] = {"red", "green", "blue", "magenta", "cyan",
                                         "orange", "pink", "grey", "violet", "yellow"};

    ScopedPointer<ComboBox> sourceSelector;
    ScopedPointer<UtilityButton> plusButton;
    ScopedPointer<UtilityButton> minusButton;
    int selectedSource;

    void addTrackingSource();
    void removeTrackingSource();

    ScopedPointer<Label> positionLabel;
    ScopedPointer<Label> labelPort;
    ScopedPointer<Label> portLabel;
    ScopedPointer<Label> labelAdr;
    ScopedPointer<Label> adrLabel;
    ScopedPointer<Label> labelColor;
    ScopedPointer<Label> colorLabel;
    ScopedPointer<ComboBox> colorSelector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackingNodeEditor);

};


#endif  // TRACKINGNODEEDITOR_H
