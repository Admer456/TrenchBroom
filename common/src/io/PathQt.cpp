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

#include "PathQt.h"

#include <QFile>

namespace tb::io
{

QString pathAsQString(const std::filesystem::path& path)
{
  return QFile{path}.fileName();
}

QString pathAsGenericQString(const std::filesystem::path& path)
{
  return QFile{path}.fileName();
}

std::filesystem::path pathFromQString(const QString& path)
{
  return QFile{path}.filesystemFileName();
}

} // namespace tb::io
