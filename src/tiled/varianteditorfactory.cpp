/*
 * varianteditorfactory.cpp
 * Copyright (C) 2006 Trolltech ASA. All rights reserved. (GPLv2)
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "varianteditorfactory.h"

#include "fileedit.h"
#include "tilesetparametersedit.h"
#include "variantpropertymanager.h"

#include <QCompleter>

namespace Tiled {
namespace Internal {

VariantEditorFactory::~VariantEditorFactory()
{
    qDeleteAll(mFileEditToProperty.keys());
    qDeleteAll(mTilesetEditToProperty.keys());
}

void VariantEditorFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)),
            this, SLOT(slotPropertyChanged(QtProperty*,QVariant)));
    connect(manager, SIGNAL(attributeChanged(QtProperty*,QString,QVariant)),
            this, SLOT(slotPropertyAttributeChanged(QtProperty*,QString,QVariant)));
    QtVariantEditorFactory::connectPropertyManager(manager);
}

QWidget *VariantEditorFactory::createEditor(QtVariantPropertyManager *manager,
                                            QtProperty *property,
                                            QWidget *parent)
{
    const int type = manager->propertyType(property);

    if (type == VariantPropertyManager::filePathTypeId()) {
        FileEdit *editor = new FileEdit(parent);
        editor->setFilePath(manager->value(property).toString());
        editor->setFilter(manager->attributeValue(property, QLatin1String("filter")).toString());
        mCreatedFileEdits[property].append(editor);
        mFileEditToProperty[editor] = property;

        connect(editor, SIGNAL(filePathChanged(const QString &)),
                this, SLOT(fileEditFilePathChanged(const QString &)));
        connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));

        return editor;
    }

    if (type == VariantPropertyManager::tilesetParametersTypeId()) {
        auto editor = new TilesetParametersEdit(parent);
        editor->setTileset(manager->value(property).value<EmbeddedTileset>());
        mCreatedTilesetEdits[property].append(editor);
        mTilesetEditToProperty[editor] = property;

        connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));

        return editor;
    }

    QWidget *editor = QtVariantEditorFactory::createEditor(manager, property, parent);

    if (type == QVariant::String) {
        // Add support for "suggestions" attribute that adds a QCompleter to the QLineEdit
        QVariant suggestions = manager->attributeValue(property, QLatin1String("suggestions"));
        if (!suggestions.toStringList().isEmpty()) {
            if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
                QCompleter *completer = new QCompleter(suggestions.toStringList(), lineEdit);
                completer->setCaseSensitivity(Qt::CaseInsensitive);
                lineEdit->setCompleter(completer);
            }
        }
    }

    return editor;
}

void VariantEditorFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)),
               this, SLOT(slotPropertyChanged(QtProperty*,QVariant)));
    disconnect(manager, SIGNAL(attributeChanged(QtProperty*,QString,QVariant)),
               this, SLOT(slotPropertyAttributeChanged(QtProperty*,QString,QVariant)));
    QtVariantEditorFactory::disconnectPropertyManager(manager);
}

void VariantEditorFactory::slotPropertyChanged(QtProperty *property,
                                               const QVariant &value)
{
    if (mCreatedFileEdits.contains(property)) {
        for (FileEdit *edit : mCreatedFileEdits[property])
            edit->setFilePath(value.toString());
    }
    else if (mCreatedTilesetEdits.contains(property)) {
        for (TilesetParametersEdit *edit : mCreatedTilesetEdits[property])
            edit->setTileset(value.value<EmbeddedTileset>());
    }
}

void VariantEditorFactory::slotPropertyAttributeChanged(QtProperty *property,
                                                        const QString &attribute,
                                                        const QVariant &value)
{
    if (mCreatedFileEdits.contains(property)) {
        if (attribute == QLatin1String("filter")) {
            for (FileEdit *edit : mCreatedFileEdits[property])
                edit->setFilter(value.toString());
        }
    }
}

void VariantEditorFactory::fileEditFilePathChanged(const QString &value)
{
    FileEdit *fileEdit = qobject_cast<FileEdit*>(sender());
    Q_ASSERT(fileEdit);

    if (QtProperty *property = mFileEditToProperty.value(fileEdit)) {
        QtVariantPropertyManager *manager = propertyManager(property);
        if (!manager)
            return;
        manager->setValue(property, value);
    }
}

void VariantEditorFactory::slotEditorDestroyed(QObject *object)
{
    // Check if it was a FileEdit
    {
        FileEdit *fileEdit = static_cast<FileEdit*>(object);

        if (QtProperty *property = mFileEditToProperty.value(fileEdit)) {
            mFileEditToProperty.remove(fileEdit);
            mCreatedFileEdits[property].removeAll(fileEdit);
            if (mCreatedFileEdits[property].isEmpty())
                mCreatedFileEdits.remove(property);
            return;
        }
    }

    // Check if it was a TilesetParametersEdit
    {
        TilesetParametersEdit *tilesetEdit = static_cast<TilesetParametersEdit*>(object);

        if (QtProperty *property = mTilesetEditToProperty.value(tilesetEdit)) {
            mTilesetEditToProperty.remove(tilesetEdit);
            mCreatedTilesetEdits[property].removeAll(tilesetEdit);
            if (mCreatedTilesetEdits[property].isEmpty())
                mCreatedTilesetEdits.remove(property);
            return;
        }
    }
}

} // namespace Internal
} // namespace Tiled
