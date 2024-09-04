/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "SmartColorEditor.h"

#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QScrollArea>

#include "Assets/ColorRange.h"
#include "Color.h"
#include "Model/Entity.h"
#include "Model/EntityColor.h"
#include "Model/EntityNode.h"
#include "Model/EntityNodeBase.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/BorderLine.h"
#include "View/ColorButton.h"
#include "View/ColorTable.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/ViewConstants.h"

#include "kdl/overload.h"
#include "kdl/vector_set.h"

namespace TrenchBroom
{
namespace View
{
SmartColorEditor::SmartColorEditor(
  std::weak_ptr<MapDocument> document, bool range255, bool hasExtraValue, QWidget* parent)
  : SmartPropertyEditor(document, parent)
  , m_extraValueBox(nullptr)
  , m_colorPicker(nullptr)
  , m_colorHistory(nullptr)
  , m_withExtraValue(hasExtraValue)
  , m_colorType(range255 ? Assets::ColorRange::Byte : Assets::ColorRange::Float)
{
  createGui();
}

void SmartColorEditor::createGui()
{
  assert(m_extraValueBox == nullptr);
  assert(m_colorPicker == nullptr);
  assert(m_colorHistory == nullptr);

  if (m_withExtraValue)
  {
    m_extraValueBox = new QLineEdit();
  }
  auto colorLabel = new QLabel(tr("Color"));
  makeEmphasized(colorLabel);
  m_colorPicker = new ColorButton();
  m_colorHistory = new ColorTable(ColorHistoryCellSize);

  auto* colorHistoryScroller = new QScrollArea();
  colorHistoryScroller->setWidget(m_colorHistory);
  colorHistoryScroller->setWidgetResizable(true);
  colorHistoryScroller->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

  auto* leftLayout = new QVBoxLayout();
  leftLayout->setContentsMargins(0, 0, 0, 0);
  leftLayout->setSpacing(LayoutConstants::NarrowVMargin);
  leftLayout->addWidget(colorLabel);
  leftLayout->addWidget(m_colorPicker);
  if (m_extraValueBox != nullptr)
  {
    auto extraValueLabel = new QLabel(tr("Intensity"));
    makeEmphasized(extraValueLabel);
    leftLayout->addWidget(extraValueLabel);
    leftLayout->addWidget(m_extraValueBox);
  }
  leftLayout->addStretch(1);

  auto* outerLayout = new QHBoxLayout();
  outerLayout->setContentsMargins(LayoutConstants::WideHMargin, 0, 0, 0);
  outerLayout->setSpacing(0);
  outerLayout->addLayout(leftLayout);
  outerLayout->addSpacing(LayoutConstants::WideHMargin);
  outerLayout->addWidget(new BorderLine(BorderLine::Direction::Vertical));
  outerLayout->addWidget(colorHistoryScroller, 1);
  setLayout(outerLayout);

  if (m_extraValueBox != nullptr)
  {
    connect(
      m_extraValueBox,
      &QLineEdit::textEdited,
      this,
      &SmartColorEditor::extraValueChanged);
  }
  connect(
    m_colorPicker,
    &ColorButton::colorChangedByUser,
    this,
    &SmartColorEditor::colorPickerChanged);
  connect(
    m_colorHistory,
    &ColorTable::colorTableSelected,
    this,
    &SmartColorEditor::colorTableSelected);
}

void SmartColorEditor::doUpdateVisual(const std::vector<Model::EntityNodeBase*>& nodes)
{
  ensure(m_colorPicker != nullptr, "colorPicker is null");
  ensure(m_colorHistory != nullptr, "colorHistory is null");

  updateColorHistory();
}

void SmartColorEditor::updateExtraValueBox(float value)
{
  if (m_extraValueBox == nullptr)
  {
    return;
  }

  auto valueString = QString{};

  if (m_colorType == Assets::ColorRange::Byte)
  {
    valueString = QString::number(int(value * 255.0f));
  }
  else
  {
    valueString = QString::number(value);
  }

  m_extraValueBox->setText(valueString);
}

float SmartColorEditor::getExtraValue(bool convertColorRange) const
{
  if (m_extraValueBox == nullptr)
  {
    return 1.0f;
  }

  if ( convertColorRange && m_colorType == Assets::ColorRange::Byte )
  {
    return m_extraValueBox->text().toInt() / 255.0f;
  }

  return m_extraValueBox->text().toFloat();
}

template <typename Node>
static std::vector<Color> collectColors(
  const std::vector<Node*>& nodes, const std::string& propertyKey)
{
  struct ColorCmp
  {
    bool operator()(const Color& lhs, const Color& rhs) const
    {
      const auto lr = lhs.r();
      const auto lg = lhs.g();
      const auto lb = lhs.b();
      const auto rr = rhs.r();
      const auto rg = rhs.g();
      const auto rb = rhs.b();

      float lh, ls, lbr, rh, rs, rbr;
      Color::rgbToHSB(lr, lg, lb, lh, ls, lbr);
      Color::rgbToHSB(rr, rg, rb, rh, rs, rbr);

      if (lh < rh)
      {
        return true;
      }
      else if (lh > rh)
      {
        return false;
      }
      else if (ls < rs)
      {
        return true;
      }
      else if (ls > rs)
      {
        return false;
      }
      else if (lbr < rbr)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
  };

  kdl::vector_set<Color, ColorCmp> colors;

  const auto visitEntityNode = [&](const auto* node) {
    if (const auto* value = node->entity().property(propertyKey))
    {
      colors.insert(Model::parseEntityColor(*value, true));
    }
  };

  for (const auto* node : nodes)
  {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, const Model::WorldNode* world) {
        world->visitChildren(thisLambda);
        visitEntityNode(world);
      },
      [](auto&& thisLambda, const Model::LayerNode* layer) {
        layer->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const Model::GroupNode* group) {
        group->visitChildren(thisLambda);
      },
      [&](const Model::EntityNode* entity) { visitEntityNode(entity); },
      [](const Model::BrushNode*) {},
      [](const Model::PatchNode*) {}));
  }

  return colors.get_data();
}

static std::vector<QColor> toQColors( const std::vector<Color> colors )
{
  auto result = std::vector<QColor>(colors.size());
  for ( const auto& color : colors )
  {
    // The alpha component skews the colour preview, so make it always 100%
    auto colorModified = Color{color.r(), color.g(), color.b()};
    result.emplace_back(toQColor(colorModified));
  }
  return result;
}

void SmartColorEditor::updateColorHistory()
{
  m_colorHistory->setColors(
    toQColors(collectColors(std::vector<Model::Node*>{document()->world()}, propertyKey())));

  const auto selectedColors =
    collectColors(document()->allSelectedEntityNodes(), propertyKey());
  m_colorHistory->setSelection(toQColors(selectedColors));
  const auto selectedColor =
    !selectedColors.empty() ? selectedColors.back() : Color{0, 0, 0};
  updateExtraValueBox(selectedColor.a());
  m_colorPicker->setColor(toQColorClampAlpha(selectedColor));
  m_currentColor = selectedColor;
}

void SmartColorEditor::setColor(const QColor& color)
{
  const auto tbColor = Color{
    float(color.redF()),
    float(color.greenF()),
    float(color.blueF()),
    getExtraValue(true)};

  setColor(tbColor);
}

void SmartColorEditor::setColor(const Color& color)
{
  auto value =
    Model::entityColorAsString(color, m_colorType, m_withExtraValue);
  document()->setProperty(propertyKey(), value);
  m_currentColor = color;
}

Color SmartColorEditor::getColor() const
{
  return m_currentColor;
}

void SmartColorEditor::colorPickerChanged(const QColor& color)
{
  setColor(color);
}

void SmartColorEditor::colorTableSelected(QColor color)
{
  setColor(color);
}

void SmartColorEditor::extraValueChanged(const QString& value)
{
  setColor(toQColor(m_currentColor));
}
} // namespace View
} // namespace TrenchBroom
