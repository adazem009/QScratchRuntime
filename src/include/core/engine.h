/*
 * engine.h
 * This file is part of QScratchRuntime
 *
 * Copyright (C) 2022 - adazem009
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

#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QVariantMap>

class scratchSprite;
class Blocks;

/*! \brief The Engine class processes sprite frames. */
class Engine : public QObject
{
	Q_OBJECT
	public:
		explicit Engine(scratchSprite *sprite, QObject *parent = nullptr);
		void frame(void);
		QMap<QString,QString> getInputs(QVariantMap block, bool readFields = false);
		QList<QVariantMap> currentExecPos;
		bool runFrameAgain;
		int processID;
		QVariantMap *newStack;
		bool frameEnd, processEnd;

	private:
		void spriteTimerEvent(void);
		scratchSprite *m_sprite;
		Blocks *blocks;

	signals:
		void setSceneScale(qreal value);
		void setX(qreal x);
		void setY(qreal y);
		void setSize(qreal size);
		void setDirection(qreal angle);
		void setCostume(int id, QVariantMap *scripts = nullptr);
		void resetGraphicEffects(void);
		void installGraphicEffects(void);
		void showBubble(QString text, bool thought = false);
		void setVisible(bool visible);
		void setZValue(qreal z);
};

#endif // ENGINE_H
