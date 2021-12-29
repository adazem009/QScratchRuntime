/*
 * projectscene.cpp
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

#include "projectscene.h"

/*! Constructs projectScene. */
projectScene::projectScene(qreal x, qreal y, qreal width, qreal height, QObject *parent) :
	QGraphicsScene(x, y, width, height, parent) { }

/*! Loads list of sprite pointers. */
void projectScene::loadSpriteList(QList<scratchSprite*> list)
{
	spriteList = list;
}

/*! Overrides QGraphicsScene#mouseMoveEvent(). */
void projectScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	for(int i=0; i < spriteList.count(); i++)
		spriteList[i]->setMousePos(event->scenePos());
	
}
