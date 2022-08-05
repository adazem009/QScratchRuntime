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
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif
#include <QGraphicsSceneMouseEvent>
#include <QDateTime>
#include <QMediaPlayer>
#include <QElapsedTimer>
#include <QBuffer>
#include <QSvgRenderer>
#include <QPainter>
#include <QSettings>
#include <QGraphicsScene>
#include "global.h"

class Engine;

/*! \brief The scratchSprite class is a QGraphicsPixmapItem, which represents a Scratch sprite. */
class scratchSprite : public QObject, public QGraphicsPixmapItem
{
	Q_OBJECT
	public:
		enum { Type = UserType + 1 };
		explicit scratchSprite(QJsonObject spriteObject, QString assetDir, QGraphicsItem *parent = nullptr);
		~scratchSprite();
		int type(void) const override;
		scratchSprite *getSprite(QString name);
		void setMousePos(QPointF pos);
		void frame(void);
		static void stopAllSounds(void);
		void setVolume(qreal newVolume);
		void spriteClicked(void);
		void keyPressed(int key, QString keyText);
		bool checkKey(int QtKey, QString keyText, QString scratchKey);
		void backdropSwitchEvent(QVariantMap *script);
		void emitBroadcast(QString broadcastName, QVariantMap *script = nullptr);
		void broadcastReceived(QString broadcastName, QVariantMap *script);
		void startClone(void);
		QPointer<QMediaPlayer> *playSound(QString soundName);
		Engine* engine(void);
		bool isClone(void);
		qreal mouseX, mouseY;
		bool isStage = false; /*!< True if this is a stage. */
		QString name; /*!< Sprite name. */
		qreal spriteX = 0; /*!< X position. */
		qreal spriteY = 0; /*!< Y position. */
		int currentCostume; /*!< Current costume ID. */
		int volume; /*!< Volume for sound blocks. */
		int tempo; /*!< Tempo for instrument blocks. */
		qreal size; /*!< Sprite size. */
		qreal direction; /*!< Sprite direction. */
		bool draggable; /*!< True if the sprite is draggable. */
		QString rotationStyle; /*!< Sprite rotation style ("all around", "left-right", or "don't rotate"). */
		QList<QVariantMap> costumes;
		QMap<QString,QVariantMap> frameEvents;
		QMap<QString,QVariantMap> blocks;
		QMap<QString,qreal> graphicEffects;
		QElapsedTimer timer;
		QVector<QVariantMap*> stackPointers;
		qreal sceneScale = 1;
		QJsonObject jsonObject;
		QString assetDir;

	private:
		qreal translateX(qreal x, bool toScratch = false);
		qreal translateY(qreal y, bool toScratch = false);
		void resetTimer(void);
		Engine *m_engine;
		qreal rotationCenterX, rotationCenterY;
		bool pointingLeft;
		QMap<QString,QPair<QString,QString>> variables;
		QMap<QString,QPair<QString,QList<QString>>> lists;
		QMap<QString,QString> broadcasts;
		QList<QVariantMap> sounds;
		QGraphicsPixmapItem *speechBubble;
		QGraphicsTextItem *speechBubbleText;
		QPixmap costumePixmap;
		QSettings settings;
		bool m_isClone = false;

	signals:
		/*! A signal, which is emitted from the stage when the backdrop switches. */
		void backdropSwitched(QVariantMap *script);
		/*! A signal, which is emitted when the sprite sends a broadcast. */
		void broadcast(QString broadcastName, QVariantMap *script = nullptr);

	public slots:
		void setXPos(qreal x);
		void setYPos(qreal y);
		void setSize(qreal newSize);
		void setDirection(qreal angle);
		void setCostume(int id, QVariantMap *scripts = nullptr);
		void resetGraphicEffects(void);
		void installGraphicEffects(void);
		void showBubble(QString text, bool thought = false);
		void greenFlagClicked(void);
		void stopAll(void);
		void stopSprite(void);
		void setSceneScale(qreal value);
};

extern QList<scratchSprite*> spriteList;
extern QList<scratchSprite*> cloneRequests;
extern QList<scratchSprite*> deleteRequests;

#endif // SCRATCHSPRITE_H
