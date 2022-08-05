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
projectScene::projectScene(qreal sceneScale, QObject *parent) :
	QGraphicsScene(parent)
{
	setScale(sceneScale);
	projectRunning = false;
	multithreading = settings.value("main/multithreading", false).toBool();
	timerID = startTimer(1000.0 / settings.value("main/fps", 30).toInt());
	fpsTimerID = startTimer(1000); // for measuring FPS
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
		spriteList[i]->setSceneScale(scale);
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
	if(event->timerId() == timerID)
	{
#ifndef Q_OS_WASM
		QVector<QFuture<void>> futureList;
		if(multithreading)
		{
			int threadCount = QThread::idealThreadCount();
			int threads = 0;
			for(int i=0; i < spriteList.count(); i++)
			{
				futureList += QtConcurrent::run(spriteList[i]->engine(), &Engine::frame);
				threads++;
				if(threads >= threadCount)
				{
					for(int j=0; j < futureList.count(); j++)
						futureList[j].waitForFinished();
					futureList.clear();
					threads = 0;
				}
			}
			for(int i=0; i < futureList.count(); i++)
				futureList[i].waitForFinished();
		}
		else
#endif // Q_OS_WASM
		{
			for(int i=0; i < spriteList.count(); i++)
				spriteList[i]->engine()->frame();
		}
		// Delete requested sprites
		for(int i=0; i < deleteRequests.count(); i++)
		{
			removeItem(deleteRequests[i]);
			spriteList.removeAll(deleteRequests[i]);
			deleteRequests[i]->deleteLater();
		}
		deleteRequests.clear();
		if(cloneRequests.count() > 0)
		{
			// Create clones and run a frame on them
			for(int i=0; i < cloneRequests.count(); i++)
			{
				scratchSprite *clone = createClone(cloneRequests[i]);
				if(clone != nullptr)
					clone->engine()->frame();
			}
			cloneRequests.clear();
		}
		frames++;
	}
	else if(event->timerId() == fpsTimerID)
	{
		fpsValue = frames;
		frames = 0;
		emit currentFpsChanged(fpsValue);
	}
	event->accept();
}

/*! Sets FPS. */
void projectScene::setFps(int fps)
{
	settings.setValue("main/fps", fps);
	killTimer(timerID);
	timerID = startTimer(1000.0 / fps);
}

/*! Returns measured FPS value. */
int projectScene::currentFps(void)
{
	return fpsValue;
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

/*! Toggles multithreading. */
void projectScene::setMultithreading(bool state)
{
	settings.setValue("main/multithreading", state);
	multithreading = state;
}

/*! Sets scene scale. */
void projectScene::setScale(qreal value)
{
	scale = value;
	setSceneRect(-240 * scale, -180 * scale, 480 * scale, 360 * scale);
	for(int i=0; i < spriteList.count(); i++)
		spriteList[i]->setSceneScale(scale);
}

/*! Returns scene scale. */
qreal projectScene::sceneScale(void)
{
	return scale;
}

/*! Creates a clone of the given sprite and returns it (or nullptr if the clone can't be created due to limits). */
scratchSprite* projectScene::createClone(scratchSprite *targetSprite)
{
	// Count existing clones
	int count = 0;
	for(int i=0; i < spriteList.count(); i++)
	{
		if(spriteList[i]->isClone())
			count++;
	}
	// Clone limit
	if((count >= 300) && !settings.value("main/infiniteClones", false).toBool())
		return nullptr;
	// Create the clone
	scratchSprite *clone = new scratchSprite(targetSprite->jsonObject, targetSprite->assetDir);
	addItem(clone);
	spriteList.append(clone);
	// Copy properties from target sprite to the clone
	clone->setSceneScale(targetSprite->sceneScale);
	clone->setXPos(targetSprite->spriteX);
	clone->setYPos(targetSprite->spriteY);
	clone->setCostume(targetSprite->currentCostume);
	clone->setVolume(targetSprite->volume);
	clone->tempo = targetSprite->tempo;
	clone->setSize(targetSprite->size);
	clone->rotationStyle = targetSprite->rotationStyle; // Rotation style must be set before direction
	clone->setDirection(targetSprite->direction);
	clone->draggable = targetSprite->draggable; // TODO: draggable will probably need a function later
	clone->graphicEffects = targetSprite->graphicEffects;
	clone->installGraphicEffects();
	clone->setVisible(targetSprite->isVisible());
	// TODO: Copy variables
	// TODO: Copy lists
	clone->startClone();
	return clone;
}
