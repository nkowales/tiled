/*
 * changetileimagesource.h
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TILED_INTERNAL_CHANGETILEIMAGESOURCE_H
#define TILED_INTERNAL_CHANGETILEIMAGESOURCE_H

#include <QUndoCommand>

namespace Tiled {

class Tile;

namespace Internal {

class MapDocument;

class ChangeTileImageSource : public QUndoCommand
{
public:
    ChangeTileImageSource(MapDocument *mapDocument,
                          Tile *tile,
                          const QString &imageSource);

    void undo() { apply(mOldImageSource); }
    void redo() { apply(mNewImageSource); }

private:
    void apply(const QString &imageSource);

    MapDocument *mMapDocument;
    Tile *mTile;
    QString mOldImageSource;
    QString mNewImageSource;
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_CHANGETILEIMAGESOURCE_H
