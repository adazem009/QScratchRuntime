/*
 * projectparser.cpp
 * This file is part of QScratchRuntime
 *
 * Copyright (C) 2021 - adazem009
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

#include "core/projectparser.h"

/*! Constructs projectParser. */
projectParser::projectParser(QString fileName, QObject *parent) :
	QObject(parent)
{
	QFileInfo fileInfo(fileName);
	assetDir = fileInfo.path();
	// Read JSON
	QFile jsonFile(fileName);
	if(!jsonFile.exists())
	{
		jsonFile.open(QIODevice::WriteOnly | QIODevice::Text);
		jsonFile.close();
	}
	jsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
	document = QJsonDocument::fromJson(jsonFile.readAll());
	jsonFile.close();
	// Read object
	mainObject = document.object();
}

/*! Returns list of sprites (including stage). */
QList<scratchSprite*> projectParser::sprites(void)
{
	QList<scratchSprite*> out;
	out.clear();
	QJsonArray targets = mainObject.value("targets").toArray();
	for(int i=0; i < targets.count(); i++)
		out += new scratchSprite(targets[i].toObject(),assetDir);
	return out;
}

/*! Returns a pointer to stage. */
scratchSprite *projectParser::stage(void)
{
	QJsonArray targets = mainObject.value("targets").toArray();
	scratchSprite *currentSprite;
	for(int i=0; i < targets.count(); i++)
	{
		currentSprite = new scratchSprite(targets[i].toObject(),assetDir);
		if(currentSprite->isStage)
			return currentSprite;
	}
	return nullptr;
}
