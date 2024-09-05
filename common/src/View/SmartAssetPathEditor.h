/*
 Copyright (C) 2024 Admer Šuko

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

#pragma once

#include "View/SmartPropertyEditor.h"

namespace TrenchBroom
{
namespace View
{
class SmartAssetPathEditor : public SmartPropertyEditor
{
  Q_OBJECT
public:
  explicit SmartAssetPathEditor(
    std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

private:
  void createGui();
  void onBrowsePressed();
  void setPath(const std::string& value);

  void doUpdateVisual(const std::vector<Model::EntityNodeBase*>& nodes) override;

  bool shouldBrowseDirectories() const;
  std::string getFilter() const;
  std::string getGamePath() const;
  std::string transformAssetPath(std::string_view fullAssetPath) const;
};
} // namespace View
} // namespace TrenchBroom
