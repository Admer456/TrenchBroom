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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Controller/NewDocumentCommand.h"
#include "IO/Path.h"
#include "Model/EntityDefinitionFileSpec.h"
#include "Model/GameConfig.h"
#include "Model/MockGame.h"
#include "Model/Map.h"
#include "Model/ModelTypes.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        TEST(NewDocumentCommandTest, newDocumentInEmptyDocument) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            const BBox3d worldBounds(-8192.0, 8192.0);
            
            Model::MockGamePtr game = Model::MockGame::newGame();
            EXPECT_CALL(*game, doNewMap(Model::MapFormat::Standard)).WillOnce(Return(new Model::Map(Model::MapFormat::Standard)));
            const Model::GameConfig::FlagsConfig contentFlags;
            EXPECT_CALL(*game, doContentFlags()).WillOnce(ReturnRef(contentFlags));
            EXPECT_CALL(*game, doExtractEntityDefinitionFile(_)).WillOnce(Return(Model::EntityDefinitionFileSpec::external(IO::Path("/somefile.def"))));
            EXPECT_CALL(*game, doGamePath()).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doFindEntityDefinitionFile(_, _)).WillOnce(Return(IO::Path("/somefile.def")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path("/somefile.def"))).WillOnce(Return(Assets::EntityDefinitionList()));
            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));
            
            View::MapDocumentSPtr doc = View::MapDocument::newMapDocument();
            const Model::MapFormat::Type format = Model::MapFormat::Standard;
            
            Command::Ptr command = Command::Ptr(new NewDocumentCommand(doc, worldBounds, game, format));
            ASSERT_FALSE(command->undoable());
            ASSERT_TRUE(command->performDo());
            ASSERT_TRUE(doc->map() != NULL);
            ASSERT_EQ(format, doc->map()->format());
            ASSERT_EQ(IO::Path("unnamed.map"), doc->path());
            ASSERT_FALSE(doc->modified());
        }

        TEST(NewDocumentCommandTest, newDocumentInExistingDocument) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;

            const BBox3d worldBounds(-8192.0, 8192.0);
            const Model::GameConfig::FlagsConfig contentFlags;
            Model::MockGamePtr game = Model::MockGame::newGame();
            const IO::Path path("data/Controller/NewDocumentCommandTest/Cube.map");

            Model::Map* map = new Model::Map(Model::MapFormat::Standard);
            EXPECT_CALL(*game, doLoadMap(worldBounds, path)).WillOnce(Return(map));
            EXPECT_CALL(*game, doContentFlags()).WillOnce(ReturnRef(contentFlags));
            EXPECT_CALL(*game, doExtractEnabledMods(map)).WillOnce(Return(StringList()));
            EXPECT_CALL(*game, doSetAdditionalSearchPaths(IO::Path::List()));

            EXPECT_CALL(*game, doExtractEntityDefinitionFile(map)).WillOnce(Return(Model::EntityDefinitionFileSpec::external(IO::Path("/somefile.def"))));
            EXPECT_CALL(*game, doGamePath()).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doFindEntityDefinitionFile(_, _)).WillOnce(Return(IO::Path("/somefile.def")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path("/somefile.def"))).WillOnce(Return(Assets::EntityDefinitionList()));
            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));
            
            EXPECT_CALL(*game, doExtractExternalTextureCollections(map)).WillOnce(Return(EmptyStringList));
            EXPECT_CALL(*game, doGamePath()).WillOnce(Return(IO::Path("Quake")));
            
            EXPECT_CALL(*game, doNewMap(Model::MapFormat::Standard)).WillOnce(Return(new Model::Map(Model::MapFormat::Standard)));
            EXPECT_CALL(*game, doContentFlags()).WillOnce(ReturnRef(contentFlags));
            EXPECT_CALL(*game, doExtractEntityDefinitionFile(_)).WillOnce(Return(Model::EntityDefinitionFileSpec::external(IO::Path("/someotherfile.def"))));
            EXPECT_CALL(*game, doGamePath()).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doFindEntityDefinitionFile(_, _)).WillOnce(Return(IO::Path("/someotherfile.def")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path("/someotherfile.def"))).WillOnce(Return(Assets::EntityDefinitionList()));
            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));
            
            View::MapDocumentSPtr doc = View::MapDocument::newMapDocument();
            doc->openDocument(worldBounds, game, path);
            const Model::MapFormat::Type format = Model::MapFormat::Standard;
            
            ASSERT_TRUE(doc->map() != NULL);
            ASSERT_EQ(format, doc->map()->format());
            Command::Ptr command = Command::Ptr(new NewDocumentCommand(doc, worldBounds, game, format));
            ASSERT_FALSE(command->undoable());
            ASSERT_TRUE(command->performDo());
            ASSERT_TRUE(doc->map() != NULL);
            ASSERT_EQ(format, doc->map()->format());
            ASSERT_EQ(IO::Path("unnamed.map"), doc->path());
            ASSERT_FALSE(doc->modified());
            
            ASSERT_TRUE(doc->map()->entities().empty());
            ASSERT_TRUE(doc->map()->worldspawn() == NULL);
        }
    }
}
