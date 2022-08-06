/*
 * blocks.h
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

#ifndef BLOCKS_H
#define BLOCKS_H

#include "projectscene.h"
#include "core/scratchsprite.h"

class Engine;

/*! \brief The Blocks class contains the implementation of Scratch blocks. */
class Blocks : public QObject
{
	Q_OBJECT
	public:
		explicit Blocks(scratchSprite *spritePtr, QObject *parent = nullptr);
		bool runBlock(QString opcode, QMap<QString,QString> inputs, QString *returnValue = nullptr);
	private:
		scratchSprite *sprite;
		Engine *engine;
		int processID;
		bool motionBlocks(QString opcode, QMap<QString,QString> inputs, QString *returnValue = nullptr);
		bool looksBlocks(QString opcode, QMap<QString,QString> inputs, QString *returnValue = nullptr);
		bool soundBlocks(QString opcode, QMap<QString,QString> inputs, QString *returnValue = nullptr);
		bool eventBlocks(QString opcode, QMap<QString,QString> inputs, QString *returnValue = nullptr);
		bool controlBlocks(QString opcode, QMap<QString,QString> inputs, QString *returnValue = nullptr);
};

#endif // BLOCKS_H
