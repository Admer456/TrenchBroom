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

#pragma once

#include "Assets/PropertyDefinition.h"
#include "View/SmartPropertyEditor.h"

#include <memory>
#include <vector>

class QColor;
class QWidget;
class QPushButton;
class QRadioButton;

namespace TrenchBroom
{
namespace View
{
class ColorButton;
class ColorTable;
class MapDocument;

class SmartColorEditor : public SmartPropertyEditor
{
  Q_OBJECT
private:
  static const size_t ColorHistoryCellSize = 15;
  using ColorRange = int;
  using wxColorList = std::vector<Color>;

  QLineEdit* m_extraValueBox;
  ColorButton* m_colorPicker;
  ColorTable* m_colorHistory;
  ColorRange m_colorType;
  Color m_currentColor;
  bool m_withExtraValue;

public:
  explicit SmartColorEditor(
    std::weak_ptr<MapDocument> document,
    bool range255,
    bool hasExtraValue,
    QWidget* parent = nullptr);

private:
  void createGui();
  void doUpdateVisual(const std::vector<Model::EntityNodeBase*>& nodes) override;

  void updateColorHistory();
  void updateExtraValueBox(float value);
  float getExtraValue(bool convertColorRange = false) const;

  // Qt's colour type does not support going beyond 1 or 255
  // The smart colour editor was modified to use TB's Color internally,
  // and when it writes the property value in "extra" mode, it samples
  // the 4th value from the extra value textbox
  void setColor(const QColor& color);
  void setColor(const Color& color);
  Color getColor() const;

  void colorPickerChanged(const QColor& color);
  void colorTableSelected(QColor color);
  void extraValueChanged(const QString& value);
};
} // namespace View
} // namespace TrenchBroom
