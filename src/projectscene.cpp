/*
 * projectscene.cpp
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

#include "projectscene.h"

/*! Constructs projectScene. */
projectScene::projectScene(qreal x, qreal y, qreal width, qreal height, QObject *parent) :
	QGraphicsScene(x, y, width, height, parent)
{
	projectRunning = false;
	// TODO: Add a way to change FPS
	startTimer(1000/30);
}

/*! Loads list of sprite pointers. */
void projectScene::loadSpriteList(QList<scratchSprite*> list)
{
	spriteList = list;
	for(int i=0; i < spriteList.count(); i++)
	{
		if(spriteList[i]->isStage)
			connect(spriteList[i],&scratchSprite::backdropSwitched,this,&projectScene::backdropSwitched);
		connect(spriteList[i],&scratchSprite::broadcast,this,&projectScene::broadcastSent);
	}
}

/*! Clears list of sprite pointers. */
void projectScene::clearSpriteList(void)
{
	spriteList.clear();
}

/*! Overrides QGraphicsScene#mousePressEvent(). */
void projectScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsItem *clickedItem = itemAt(event->scenePos(),QTransform());
	if(clickedItem == nullptr)
		return;
	if(clickedItem->type() == scratchSprite::Type)
	{
		scratchSprite *clickedSprite = (scratchSprite*) clickedItem;
		if(clickedSprite != nullptr)
			clickedSprite->spriteClicked();
	}
	event->accept();
}

/*! Overrides QGraphicsScene#mouseDoubleClickEvent(). */
void projectScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	mousePressEvent(event);
}

/*! Overrides QGraphicsScene#mouseMoveEvent(). */
void projectScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	for(int i=0; i < spriteList.count(); i++)
		spriteList[i]->setMousePos(event->scenePos());
	event->accept();
}

/*! Overrides QGraphicsScene#keyPressEvent(). */
void projectScene::keyPressEvent(QKeyEvent *event)
{
	for(int i=0; i < spriteList.count(); i++)
		spriteList[i]->keyPressed(event->key(),event->text());
	event->accept();
}

/*! Connected from %clicked() signal of greenFlag in MainWindow UI. */
void projectScene::greenFlag(void)
{
	for(int i=0; i < spriteList.count(); i++)
		spriteList[i]->greenFlagClicked();
	projectRunning = true;
}

/*! Stops the project. */
void projectScene::stop(void)
{
	for(int i=0; i < spriteList.count(); i++)
		spriteList[i]->stopAll();
	projectRunning = false;
}

/*! Overrides QObject#timerEvent(). */
void projectScene::timerEvent(QTimerEvent *event)
{
	for(int i=0; i < spriteList.count(); i++)
	{
		do {
			__run_frame_again = false;
			spriteList[i]->frame();
		} while(__run_frame_again);
	}
	event->accept();
}

/*! Connected from scratchSprite#backdropSwitched() (only from the stage). */
void projectScene::backdropSwitched(QVariantMap *script)
{
	for(int i=0; i < spriteList.count(); i++)
		spriteList[i]->backdropSwitchEvent(script);
}

/*! Connected from scratchSprite#broadcast(). */
void projectScene::broadcastSent(QString broadcastName, QVariantMap *script)
{
	for(int i=0; i < spriteList.count(); i++)
		spriteList[i]->broadcastReceived(broadcastName,script);
}
