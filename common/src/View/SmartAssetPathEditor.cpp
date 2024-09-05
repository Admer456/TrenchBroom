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

#include "IO/FileSystem.h"
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
  const auto result = QFileDialog::getOpenFileName(
                        nullptr,
                        "Browsing for an asset...",
                        QString::fromStdString(getGamePath()),
                        QString::fromStdString(getFilter()))
                        .toStdString();

  setPath(transformAssetPath(result));
}

void SmartAssetPathEditor::doUpdateVisual(
  const std::vector<Model::EntityNodeBase*>& nodes)
{
  // Do nothing for now t.b.h.
}

void SmartAssetPathEditor::setPath(const std::string& value)
{
  document()->setProperty(propertyKey(), value);
}

std::string SmartAssetPathEditor::getFilter() const
{
  return "All files (*);;Models (*.mdl);;Audio (*.wav);;Sprites (*.spr)";
}

std::string SmartAssetPathEditor::getGamePath() const
{
  const auto gamePath = document()->game()->gamePath();
  const auto defaultMod = document()->game()->defaultMod();
  const auto mods = document()->mods();

  if (mods.size() >= 1)
  {
    return fmt::format("{}/{}", gamePath.string(), mods[0]);
  }

  return fmt::format("{}/{}", gamePath.string(), defaultMod);
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
