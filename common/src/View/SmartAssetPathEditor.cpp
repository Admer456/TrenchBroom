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

#include "SmartAssetPathEditor.h"

#include "Assets/PropertyDefinition.h"
#include "IO/FileSystem.h"
#include "Model/EntityNodeBase.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "fmt/format.h"

#include <qfiledialog.h>

namespace TrenchBroom
{
namespace View
{
SmartAssetPathEditor::SmartAssetPathEditor(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : SmartPropertyEditor(document, parent)
{
  createGui();
}

void SmartAssetPathEditor::createGui()
{
  auto* browseButton = new QPushButton("Browse...");

  auto* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(LayoutConstants::NarrowVMargin);
  layout->addWidget(browseButton);
  layout->addStretch(1);
  setLayout(layout);

  connect(
    browseButton,
    &QAbstractButton::pressed,
    this,
    &SmartAssetPathEditor::onBrowsePressed);
}

void SmartAssetPathEditor::onBrowsePressed()
{
  const auto startingPath = QString::fromStdString(getGamePath());
  const auto extensionFilter = QString::fromStdString(getFilter());
  const auto result =
    shouldBrowseDirectories()
      ? QFileDialog::getExistingDirectory(
          nullptr, "Browsing for a directory...", startingPath)
          .toStdString()
      : QFileDialog::getOpenFileName(
          nullptr, "Browsing for an asset...", startingPath, extensionFilter)
          .toStdString();

  setPath(transformAssetPath(result));
}

void SmartAssetPathEditor::doUpdateVisual(
  const std::vector<Model::EntityNodeBase*>& nodes)
{
  // There is nothing to do here, so
}

bool SmartAssetPathEditor::shouldBrowseDirectories() const
{
  const auto* node = *document()->allSelectedEntityNodes().begin();
  const auto* propDef = static_cast<const Assets::PathPropertyDefinition*>(
    Model::propertyDefinition(node, propertyKey()));
  return propDef->pathType() == Assets::PathPropertyType::DirectoryPath;
}

void SmartAssetPathEditor::setPath(const std::string& value)
{
  document()->setProperty(propertyKey(), value);
}

std::string SmartAssetPathEditor::getFilter() const
{
  const auto* node = *document()->allSelectedEntityNodes().begin();
  const auto* propDef = static_cast<const Assets::PathPropertyDefinition*>(
    Model::propertyDefinition(node, propertyKey()));
  const auto pathType = propDef->pathType();

  switch (pathType)
  {
  case Assets::PathPropertyType::ModelPath:
    return "Model files (*.mdl *.gltf *.glb *.obj);;MDLs (*.mdl);;glTF 2.0 (*.gltf "
           "*.glb);;OBJ (*.obj);;All files (*)";
  case Assets::PathPropertyType::SoundPath:
    return "Audio files (*.wav *.flac *.ogg *.mp3);;All files (*)";
  case Assets::PathPropertyType::SpritePath:
    return "Sprite files (*.spr *.png);;All files (*)";
  }

  return "All files (*)";
}

std::string SmartAssetPathEditor::getGamePath() const
{
  const auto rootPath = document()->game()->gamePath().string();
  const auto mods = document()->mods();
  // In case of 2 mods or more, this chooses the first enabled mod. Must let
  // the user know this through UI or even let them choose the starting directory
  const auto mod = mods.size() >= 1 ? mods[0] : document()->game()->defaultMod();

  return fmt::format("{}/{}", rootPath, mod);
}

std::string SmartAssetPathEditor::transformAssetPath(std::string_view fullAssetPath) const
{
  const auto gamePath = document()->game()->gamePath();
  const auto defaultMod = document()->game()->defaultMod();
  const auto mods = document()->mods();

  const auto fullDefaultModPath = fmt::format("{}/{}", gamePath.string(), defaultMod);
  if (fullAssetPath.starts_with(fullDefaultModPath))
  { // +1 to eat the leading slash
    return std::string{fullAssetPath.substr(fullDefaultModPath.size() + 1)};
  }

  for (const auto& mod : mods)
  {
    const auto fullModPath = fmt::format("{}/{}", gamePath.string(), mod);
    if (fullAssetPath.starts_with(mod))
    {
      return std::string{fullAssetPath.substr(fullAssetPath.size() + 1)};
    }
  }

  return std::string{fullAssetPath};
}
} // namespace View
} // namespace TrenchBroom
