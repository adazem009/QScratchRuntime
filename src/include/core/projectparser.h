/*
 * projectparser.h
 * This file is part of QScratchRuntime
 *
 * Copyright (C) 2021-2022 - adazem009
 *
 * QScratchRuntime is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * QScratchRuntime is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QScratchRuntime. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROJECTPARSER_H
#define PROJECTPARSER_H

#include <QObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QFileInfo>
#include "core/scratchsprite.h"

/*! \brief The projectParser class provides functions for local configuration reading and writing. */
class projectParser : public QObject
{
	Q_OBJECT
	public:
		explicit projectParser(QString fileName, QObject *parent = nullptr);
		QList<scratchSprite*> sprites(void);
		scratchSprite* stage(void);
		QList<QMap<QString,QString>> assetIDs(void);

	private:
		QJsonDocument document;
		QJsonObject mainObject;
		QString assetDir;
};

#endif // PROJECTPARSER_H
