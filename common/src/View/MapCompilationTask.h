/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MapCompilationTask_h
#define MapCompilationTask_h

#include "StringUtils.h"
#include "IO/Path.h"

#include <wx/event.h>
#include <wx/process.h>
#include <wx/thread.h>

#include <list>

class wxTimer;
class wxTimerEvent;

namespace TrenchBroom {
    namespace View {
        class MapCompilationContext;
        
        class MapCompilationTask {
        public:
            typedef std::list<MapCompilationTask*> List;
            
            class TaskRunner {
            protected:
                MapCompilationContext& m_context;
            private:
                TaskRunner* m_next;
            public:
                TaskRunner(MapCompilationContext& context, TaskRunner* next);
                virtual ~TaskRunner();
                
                void execute();
                void terminate();
            protected:
                void executeNext();
            private:
                virtual void doExecute() = 0;
                virtual void doTerminate() = 0;
            private:
                TaskRunner(const TaskRunner& other);
                TaskRunner& operator=(const TaskRunner& other);
            };
        protected:
            MapCompilationTask();
        public:
            virtual ~MapCompilationTask();
            
            TaskRunner* createTaskRunner(MapCompilationContext& context, TaskRunner* next = NULL) const;
        private:
            virtual TaskRunner* doCreateTaskRunner(MapCompilationContext& context, TaskRunner* next) const = 0;
        private:
            MapCompilationTask(const MapCompilationTask& other);
            MapCompilationTask& operator=(const MapCompilationTask& other);
        };
        
        class MapCompilationCopyFiles : public MapCompilationTask {
        private:
            class Runner : public TaskRunner {
            private:
                IO::Path m_sourcePath;
                IO::Path m_targetPath;
            public:
                Runner(MapCompilationContext& context, TaskRunner* next, const String& sourceSpec, const String& targetSpec);
            private:
                void doExecute();
                void doTerminate();
            private:
                Runner(const Runner& other);
                Runner& operator=(const Runner& other);
            };
            
            String m_sourceSpec;
            String m_targetSpec;
        public:
            MapCompilationCopyFiles(const String& sourceSpec, const String& targetSpec);
        private:
            TaskRunner* doCreateTaskRunner(MapCompilationContext& context, TaskRunner* next) const;
        private:
            MapCompilationCopyFiles(const MapCompilationCopyFiles& other);
            MapCompilationCopyFiles& operator=(const MapCompilationCopyFiles& other);
        };

        class MapCompilationRunTool : public MapCompilationTask {
        private:
            class Runner : public wxEvtHandler, public TaskRunner {
            private:
                IO::Path m_toolPath;
                String m_parameters;
                wxProcess* m_process;
                wxCriticalSection m_processSection;
                wxTimer* m_processTimer;
            public:
                Runner(MapCompilationContext& context, TaskRunner* next, const String& toolSpec, const String& parameterSpec);
                ~Runner();
            private:
                void doExecute();
                void doTerminate();
            private:
                void OnTerminateProcess(wxProcessEvent& event);
                void OnProcessTimer(wxTimerEvent& event);
                String readStream(wxInputStream* stream);
                
                void createProcess();
                void startProcess(const String& cmd);
                void deleteProcess();
            private:
                Runner(const Runner& other);
                Runner& operator=(const Runner& other);
            };
            
            String m_toolSpec;
            String m_parameterSpec;
        public:
            MapCompilationRunTool(const String& toolSpec, const String& parameterSpec);
        private:
            TaskRunner* doCreateTaskRunner(MapCompilationContext& context, TaskRunner* next) const;
        private:
            MapCompilationRunTool(const MapCompilationRunTool& other);
            MapCompilationRunTool& operator=(const MapCompilationRunTool& other);
        };
        
        class MapCompilationRunGame : public MapCompilationTask {
        private:
            MapCompilationRunGame(const MapCompilationRunGame& other);
            MapCompilationRunGame& operator=(const MapCompilationRunGame& other);
        };
    }
}

#endif /* MapCompilationTask_h */
