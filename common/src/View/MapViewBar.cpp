/*
 Copyright (C) 2010 Kristian Duske

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

#include "MapViewBar.h"

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QStackedLayout>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/ViewConstants.h"
#include "View/ViewEditor.h"

namespace TrenchBroom
{
namespace View
{
MapViewBar::MapViewBar(std::weak_ptr<MapDocument> document, QWidget* parent)
  : ContainerBar(Sides::BottomSide, parent)
  , m_document(document)
  , m_toolBook(nullptr)
  , m_viewEditor(nullptr)
{
  createGui(document);
}

QStackedLayout* MapViewBar::toolBook()
{
  return m_toolBook;
}

void MapViewBar::createGui(std::weak_ptr<MapDocument> document)
{
  setAttribute(Qt::WA_MacSmallSize);

  m_toolBook = new QStackedLayout();
  m_toolBook->setContentsMargins(0, 0, 0, 0);

  m_viewEditor = new ViewPopupEditor(std::move(document));

#ifdef __APPLE__
  const auto vMargin = pref(Preferences::Theme) == Preferences::darkTheme()
                         ? LayoutConstants::MediumVMargin
                         : 0;
#else
  const auto vMargin = LayoutConstants::MediumVMargin;
#endif

  auto* layout = new QHBoxLayout();
  layout->setContentsMargins(
    LayoutConstants::WideHMargin, vMargin, LayoutConstants::WideHMargin, vMargin);
  layout->setSpacing(LayoutConstants::WideHMargin);
  layout->addLayout(m_toolBook, 1);
  layout->addWidget(m_viewEditor, 0, Qt::AlignVCenter);

  setLayout(layout);
}
} // namespace View
} // namespace TrenchBroom
