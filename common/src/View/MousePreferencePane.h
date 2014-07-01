/*
 Copyright (C) 2010-2014 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MousePreferencePane__
#define __TrenchBroom__MousePreferencePane__

#include "View/PreferencePane.h"

class wxButton;
class wxCheckBox;
class wxChoice;
class wxSlider;
class wxStaticText;

namespace TrenchBroom {
    namespace View {
        class MousePreferencePane : public PreferencePane {
        private:
            wxSlider* m_lookSpeedSlider;
            wxCheckBox* m_invertLookHAxisCheckBox;
            wxCheckBox* m_invertLookVAxisCheckBox;
            wxSlider* m_panSpeedSlider;
            wxCheckBox* m_invertPanHAxisCheckBox;
            wxCheckBox* m_invertPanVAxisCheckBox;
            wxSlider* m_moveSpeedSlider;
            wxCheckBox* m_enableAltMoveCheckBox;
            wxCheckBox* m_invertAltMoveAxisCheckBox;
            wxCheckBox* m_moveInCursorDirCheckBox;
            
        public:
            MousePreferencePane(wxWindow* parent);

            void OnLookSpeedChanged(wxScrollEvent& event);
            void OnInvertLookHAxisChanged(wxCommandEvent& event);
            void OnInvertLookVAxisChanged(wxCommandEvent& event);
            
            void OnPanSpeedChanged(wxScrollEvent& event);
            void OnInvertPanHAxisChanged(wxCommandEvent& event);
            void OnInvertPanVAxisChanged(wxCommandEvent& event);
            
            void OnMoveSpeedChanged(wxScrollEvent& event);
            
            void OnEnableAltMoveChanged(wxCommandEvent& event);
            void OnInvertAltMoveAxisChanged(wxCommandEvent& event);
            void OnMoveCameraInCursorDirChanged(wxCommandEvent& event);

        private:
            void createGui();
            wxWindow* createMousePreferences();

            void bindEvents();
            
            void doUpdateControls();
            bool doValidate();
        };
    }
}

#endif /* defined(__TrenchBroom__MousePreferencePane__) */
