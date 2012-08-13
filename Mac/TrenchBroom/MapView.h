/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#import <AppKit/AppKit.h>

@class EditorHolder;

@interface MapView: NSOpenGLView <NSMenuDelegate> {
    EditorHolder* editorHolder;
    void* editorGui;
    void* editorGuiListener;
    void* fontManager;
    NSUInteger flags;
    NSMenu* popupMenu;
}

- (BOOL)mapViewFocused;
- (IBAction)createEntityFromPopupMenu:(id)sender;
- (IBAction)createEntityFromMainMenu:(id)sender;

@end

namespace TrenchBroom {
    namespace Gui {
        class EditorGui;
        class EditorGuiListener {
        private:
            EditorGui* m_editorGui;
            MapView* m_mapView;
        public:
            EditorGuiListener(EditorGui* editorGui, MapView* mapView);
            ~EditorGuiListener();
            
            void editorGuiRedraw(EditorGui& editorGui);
        };
    }
}

