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
#include "core/blocks.h"

class scratchSprite;

/*! \brief The Engine class processes sprite frames. */
class Engine : public QObject
{
	Q_OBJECT
	public:
		explicit Engine(scratchSprite *sprite, QObject *parent = nullptr);
		void frame(void);
		QMap<QString,QString> getInputs(QVariantMap block, bool readFields = false);

	private:
		void spriteTimerEvent(void);
		scratchSprite *m_sprite;
		QVariantMap *newStack;
};

#endif // ENGINE_H
