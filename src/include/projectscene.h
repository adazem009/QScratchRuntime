/*
 * projectscene.h
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

#ifndef PROJECTSCENE_H
#define PROJECTSCENE_H

#include <QGraphicsScene>
#include <QKeyEvent>
#include <QSettings>
#ifndef Q_OS_WASM
#include <QtConcurrent>
#endif // Q_OS_WASM
#include "core/scratchsprite.h"
#include "core/engine.h"

/*! \brief The projectScene class is a QGraphicsScene used to manage all sprites. */
class projectScene : public QGraphicsScene
{
	Q_OBJECT
	public:
		explicit projectScene(qreal scale, QObject *parent = nullptr);
		void loadSpriteList(QList<scratchSprite*> list);
		void clearSpriteList(void);
		void setFps(int fps);
		int currentFps(void);
		void setMultithreading(bool state);
		void setScale(qreal value);
		qreal sceneScale(void);

	private:
		bool projectRunning;
		int timerID = -1, fpsTimerID = -1;
		QSettings settings;
		int frames = 0, fpsValue = 0;
		bool multithreading;
		qreal scale;

	signals:
		/*! Emitted when the measured FPS value changes (every second). */
		void currentFpsChanged(int fps);

	public slots:
		void greenFlag(void);
		void stop(void);
		void backdropSwitched(QVariantMap *script);
		void broadcastSent(QString broadcastName, QVariantMap *script = nullptr);
		scratchSprite* createClone(scratchSprite *targetSprite);

	protected:
		void timerEvent(QTimerEvent *event);
		void mousePressEvent(QGraphicsSceneMouseEvent *event);
		void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
		void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
		void keyPressEvent(QKeyEvent *event);
};

#endif // PROJECTSCENE_H
