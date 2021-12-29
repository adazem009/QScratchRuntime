/*
 * projectscene.h
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

#ifndef PROJECTSCENE_H
#define PROJECTSCENE_H

#include <QGraphicsScene>
#include "core/scratchsprite.h"

/*! \brief The projectScene class is a QGraphicsScene used to manage all sprites. */
class projectScene : public QGraphicsScene
{
	public:
		explicit projectScene(qreal x = -240, qreal y = -180, qreal width = 480, qreal height = 360, QObject *parent = nullptr);
		void loadSpriteList(QList<scratchSprite*> list);

	private:
		QList<scratchSprite*> spriteList;

	protected:
		void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
};

#endif // PROJECTSCENE_H
