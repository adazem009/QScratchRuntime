/*
 * scratchsprite.h
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

#ifndef SCRATCHSPRITE_H
#define SCRATCHSPRITE_H

#include <QGraphicsPixmapItem>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>
#include <QRandomGenerator>
#include <QGraphicsSceneMouseEvent>
#include <QDateTime>
#include <QSoundEffect>
#include "global.h"

/*! \brief The scratchSprite class is a QGraphicsPixmapItem, which represents a Scratch sprite. */
class scratchSprite : public QObject, public QGraphicsPixmapItem
{
	Q_OBJECT
	public:
		explicit scratchSprite(QJsonObject spriteObject, QString assetDir, QGraphicsItem *parent = nullptr);
		void loadSpriteList(QList<scratchSprite*> lists);
		void setXPos(qreal x);
		void setYPos(qreal y);
		void setMousePos(QPointF pos);
		void setCostume(int id);
		void setSize(qreal newSize);
		void setDirection(qreal angle);
		void frame(void);
		static void stopAllSounds(void);
		void setVolume(qreal newVolume);
		void spriteClicked(void);
		void keyPressed(int key, QString keyText);
		bool checkKey(int QtKey, QString keyText, QString scratchKey);
		void backdropSwitchEvent(void);
		bool isStage; /*!< True if this is a stage. */
		QString name; /*!< Sprite name. */
		qreal spriteX; /*!< X position. */
		qreal spriteY; /*!< Y position. */
		int currentCostume; /*!< Current costume ID. */
		int volume; /*!< Volume for sound blocks. */
		int tempo; /*!< Tempo for instrument blocks. */
		qreal size; /*!< Sprite size. */
		qreal direction; /*!< Sprite direction. */
		bool draggable; /*!< True if the sprite is draggable. */
		QString rotationStyle; /*!< Sprite rotation style ("all around", "left-right", or "don't rotate"). */
		QList<QVariantMap> costumes;

	private:
		QMap<QString,QString> getInputs(QVariantMap block, bool readFields = false);
		qreal translateX(qreal x, bool toScratch = false);
		qreal translateY(qreal y, bool toScratch = false);
		scratchSprite *getSprite(QString name);
		void bounce(void);
		void showBubble(QString text, bool thought = false);
		void resetGraphicEffects(void);
		void installGraphicEffects(void);
		QSoundEffect *playSound(QString soundName);
		QList<scratchSprite*> spriteList;
		QString assetDir;
		qreal mouseX, mouseY;
		qreal rotationCenterX, rotationCenterY;
		bool pointingLeft;
		QMap<QString,QPair<QString,QString>> variables;
		QMap<QString,QPair<QString,QList<QString>>> lists;
		QMap<QString,QString> broadcasts;
		QMap<QString,QVariantMap> blocks;
		QList<QVariantMap> sounds;
		QMap<QString,qreal> graphicEffects;
		QList<QVariantMap> currentExecPos;
		QGraphicsPixmapItem *speechBubble;
		QGraphicsTextItem *speechBubbleText;
		QPixmap costumePixmap;
		// Blocks
		bool motionBlocks(QString opcode, QMap<QString,QString> inputs, int processID, bool *frameEnd = nullptr, bool *processEnd = nullptr, QString *returnValue = nullptr);
		bool looksBlocks(QString opcode, QMap<QString,QString> inputs, int processID, bool *frameEnd = nullptr, bool *processEnd = nullptr, QString *returnValue = nullptr);
		bool soundBlocks(QString opcode, QMap<QString,QString> inputs, int processID, bool *frameEnd = nullptr, bool *processEnd = nullptr, QString *returnValue = nullptr);

	signals:
		/*! A signal, which is emitted from the stage when the backdrop switches. */
		void backdropSwitched();

	public slots:
		void greenFlagClicked(void);
		void stopSprite(void);
};

#endif // SCRATCHSPRITE_H
