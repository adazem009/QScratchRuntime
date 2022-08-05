/*
 * scratchsprite.cpp
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

#include "core/scratchsprite.h"
#include "core/engine.h"

QList<scratchSprite*> spriteList;
QList<scratchSprite*> cloneRequests;
QList<scratchSprite*> deleteRequests;

/*! Constructs scratchSprite. */
scratchSprite::scratchSprite(QJsonObject spriteObject, QString spriteAssetDir, QGraphicsItem *parent) :
	QGraphicsPixmapItem(parent),
	jsonObject(spriteObject),
	assetDir(spriteAssetDir),
	m_engine(new Engine(this, this))
{
	assetDir = spriteAssetDir;
	int i;
	pointingLeft = false;
	// Load costumes
	QJsonArray costumesArray = spriteObject.value("costumes").toArray();
	costumes.clear();
	for(i=0; i < costumesArray.count(); i++)
		costumes += costumesArray[i].toObject().toVariantMap();
	currentCostume = spriteObject.value("currentCostume").toInt();
	setCostume(currentCostume);
	// Load attributes
	isStage = spriteObject.value("isStage").toBool();
	name = spriteObject.value("name").toString();
	setVolume(spriteObject.value("volume").toDouble());
	tempo = spriteObject.value("tempo").toInt();
	if(isStage)
	{
		setZValue(0);
		setVisible(true);
		setXPos(0);
		setYPos(0);
		setSize(100);
		setDirection(90);
		draggable = false;
	}
	else
	{
		speechBubble = new QGraphicsPixmapItem(QPixmap(":res/images/speech_bubble.png"),this);
		speechBubbleText = new QGraphicsTextItem(this);
		speechBubbleText->setDefaultTextColor(QColor(0,0,0));
		speechBubbleText->setPos(10,10);
		speechBubble->setVisible(false);
		speechBubbleText->setVisible(false);
		setZValue(spriteObject.value("layerOrder").toInt());
		setVisible(spriteObject.value("visible").toBool());
		setXPos(spriteObject.value("x").toDouble());
		setYPos(spriteObject.value("y").toDouble());
		setSize(spriteObject.value("size").toDouble());
		rotationStyle = spriteObject.value("rotationStyle").toString();
		setDirection(spriteObject.value("direction").toDouble());
		draggable = spriteObject.value("draggable").toBool();
	}
	resetGraphicEffects();
	timer.start();
	// Load sounds
	QJsonArray soundsArray = spriteObject.value("sounds").toArray();
	sounds.clear();
	for(i=0; i < soundsArray.count(); i++)
		sounds += soundsArray[i].toObject().toVariantMap();
	// TODO: Load variables
	// TODO: Load lists
	// Load blocks
	QJsonObject blocksObject = spriteObject.value("blocks").toObject();
	QStringList blocksList = blocksObject.keys();
	blocks.clear();
	frameEvents.clear();
	for(i=0; i < blocksList.count(); i++)
	{
		blocks.insert(blocksList[i],blocksObject.value(blocksList[i]).toObject().toVariantMap());
		QString opcode = blocks.value(blocksList[i]).value("opcode").toString();
		if(opcode == "event_whengreaterthan")
		{
			QVariantMap block = blocks[blocksList[i]];
			block.insert("special_timereventused",false);
			blocks.insert(blocksList[i],block);
			frameEvents.insert(blocksList[i],blocks.value(blocksList[i]));
		}
	}
	// Connections
	connect(m_engine, &Engine::setSceneScale, this, &scratchSprite::setSceneScale);
	connect(m_engine, &Engine::setX, this, &scratchSprite::setXPos);
	connect(m_engine, &Engine::setY, this, &scratchSprite::setYPos);
	connect(m_engine, &Engine::setCostume, this, &scratchSprite::setCostume);
	connect(m_engine, &Engine::setSize, this, &scratchSprite::setSize);
	connect(m_engine, &Engine::setDirection, this, &scratchSprite::setDirection);
	connect(m_engine, &Engine::resetGraphicEffects, this, &scratchSprite::resetGraphicEffects);
	connect(m_engine, &Engine::installGraphicEffects, this, &scratchSprite::installGraphicEffects);
	connect(m_engine, &Engine::showBubble, this, &scratchSprite::showBubble);
	connect(m_engine, &Engine::setVisible, this, [this](bool visible) { setVisible(visible); });
	connect(m_engine, &Engine::setZValue, this, [this](qreal z) { setZValue(z); });
	
}

/*! Destroys the scratchSprite object. */
scratchSprite::~scratchSprite()
{
	for(int i=0; i < stackPointers.count(); i++)
	{
		if(stackPointers[i])
			delete stackPointers[i];
	}
}

/*! Returns user type of QGraphicsItem. */
int scratchSprite::type(void) const
{
	return Type;
}

/*! Returns a pointer to the sprite if it exists. Otherwise returns a null pointer. */
scratchSprite *scratchSprite::getSprite(QString targetName)
{
	scratchSprite *targetSprite = nullptr;
	for(int i=0; i < spriteList.count(); i++)
	{
		if(spriteList[i]->name == targetName)
		{
			targetSprite = spriteList[i];
			break;
		}
	}
	return targetSprite;
}

/*!
 * Connected from the green flag button.
 * Starts all green flag scripts.
 */
void scratchSprite::greenFlagClicked(void)
{
	stopAll();
	QStringList blocksList = blocks.keys();
	for(int i=0; i < blocksList.count(); i++)
	{
		QVariantMap block = blocks.value(blocksList[i]);
		if(block.value("opcode").toString() == "event_whenflagclicked")
		{
			QVariantMap blockMap;
			blockMap.clear();
			blockMap.insert("id",blocksList[i]);
			blockMap.insert("special","");
			m_engine->currentExecPos += blockMap;
		}
	}
}

/*! Stops the sprite, resets the timer and stops all sounds coming from the sprite. */
void scratchSprite::stopAll(void)
{
	stopSprite();
	resetTimer();
	stopAllSounds();
	if(isClone() && !deleteRequests.contains(this))
		deleteRequests += this;
}

/*! Resets the timer. */
void scratchSprite::resetTimer(void)
{
	timer.start();
	QStringList blocksList = frameEvents.keys();
	for(int i=0; i < blocksList.count(); i++)
	{
		QVariantMap block = frameEvents.value(blocksList[i]);
		if(block.value("opcode").toString() == "event_whengreaterthan")
		{
			block.insert("special_timereventused",false);
			frameEvents.insert(blocksList[i],block);
		}
	}
}

/*! Starts "when this sprite clicked" event blocks when this sprite is clicked. */
void scratchSprite::spriteClicked(void)
{
	QStringList blocksList = blocks.keys();
	for(int i=0; i < blocksList.count(); i++)
	{
		QVariantMap block = blocks.value(blocksList[i]);
		if((block.value("opcode").toString() == "event_whenthisspriteclicked") || (block.value("opcode").toString() == "event_whenstageclicked"))
		{
			// Stop running instances of this event
			QList<QVariantMap> operationsToRemove;
			operationsToRemove.clear();
			for(int i2=0; i2 < m_engine->currentExecPos.count(); i2++)
			{
				if(m_engine->currentExecPos[i2]["toplevelblock"] == blocksList[i])
					operationsToRemove += m_engine->currentExecPos[i2];
			}
			for(int i2=0; i2 < operationsToRemove.count(); i2++)
				m_engine->currentExecPos.removeAll(operationsToRemove[i2]);
			// Start the script
			QVariantMap blockMap;
			blockMap.clear();
			blockMap.insert("id",blocksList[i]);
			blockMap.insert("special","");
			m_engine->currentExecPos += blockMap;
		}
	}
}

/*! Starts "when key pressed" event blocks when a key is pressed. */
void scratchSprite::keyPressed(int key, QString keyText)
{
	QStringList blocksList = blocks.keys();
	for(int i=0; i < blocksList.count(); i++)
	{
		QVariantMap block = blocks.value(blocksList[i]);
		if(block.value("opcode").toString() == "event_whenkeypressed")
		{
			QMap<QString,QString> inputs = m_engine->getInputs(block);
			if(checkKey(key,keyText,inputs.value("KEY_OPTION")))
			{
				// Stop running instances of this event
				QList<QVariantMap> operationsToRemove;
				operationsToRemove.clear();
				for(int i2=0; i2 < m_engine->currentExecPos.count(); i2++)
				{
					if(m_engine->currentExecPos[i2]["toplevelblock"] == blocksList[i])
						operationsToRemove += m_engine->currentExecPos[i2];
				}
				for(int i2=0; i2 < operationsToRemove.count(); i2++)
					m_engine->currentExecPos.removeAll(operationsToRemove[i2]);
				// Start the script
				QVariantMap blockMap;
				blockMap.clear();
				blockMap.insert("id",blocksList[i]);
				blockMap.insert("special","");
				m_engine->currentExecPos += blockMap;
			}
		}
	}
}

/*! Returns true if key ID or key text matches the key in string. */
bool scratchSprite::checkKey(int keyID, QString keyText, QString scratchKey)
{
	// Any key
	if(scratchKey == "any")
		return true;
	// Compare key text
	if(QString::compare(keyText,scratchKey,Qt::CaseInsensitive) == 0)
		return true;
	// Generate map of built-in Scratch keys
	QMap<int,QString> keyMap;
	keyMap.clear();
	keyMap.insert(Qt::Key_Space,"space");
	keyMap.insert(Qt::Key_Up,"up arrow");
	keyMap.insert(Qt::Key_Down,"down arrow");
	keyMap.insert(Qt::Key_Right,"right arrow");
	keyMap.insert(Qt::Key_Left,"left arrow");
	keyMap.insert(Qt::Key_Enter,"enter");
	keyMap.insert(Qt::Key_Return,"enter");
	// Check key match
	return keyMap.keys(scratchKey).contains(keyID);
}

/*! Starts "when backdrop switches to" event blocks when the backdrop switches. */
void scratchSprite::backdropSwitchEvent(QVariantMap *script)
{
	QStringList blocksList = blocks.keys();
	for(int i=0; i < blocksList.count(); i++)
	{
		QVariantMap block = blocks.value(blocksList[i]);
		if(block.value("opcode").toString() == "event_whenbackdropswitchesto")
		{
			QMap<QString,QString> inputs = m_engine->getInputs(block);
			scratchSprite *stagePtr = getSprite("Stage");
			if(inputs.value("BACKDROP") == stagePtr->costumes[stagePtr->currentCostume].value("name"))
			{
				// Stop running instances of this event
				QList<QVariantMap> operationsToRemove;
				operationsToRemove.clear();
				for(int i2=0; i2 < m_engine->currentExecPos.count(); i2++)
				{
					if(m_engine->currentExecPos[i2]["toplevelblock"] == blocksList[i])
						operationsToRemove += m_engine->currentExecPos[i2];
				}
				for(int i2=0; i2 < operationsToRemove.count(); i2++)
					m_engine->currentExecPos.removeAll(operationsToRemove[i2]);
				// Start the script
				QVariantMap blockMap;
				blockMap.clear();
				blockMap.insert("id",blocksList[i]);
				blockMap.insert("special","");
				if(script != nullptr)
					script->insert("activescripts",script->value("activescripts").toInt()+1);
				blockMap.insert("callerptr", (qlonglong) (intptr_t) script);
				m_engine->currentExecPos += blockMap;
			}
		}
	}
}

/*! Emits the broadcast() signal. */
void scratchSprite::emitBroadcast(QString broadcastName, QVariantMap *script)
{
	emit broadcast(broadcastName, script);
}

/*! Starts "when broadcast received" event blocks. */
void scratchSprite::broadcastReceived(QString broadcastName, QVariantMap *script)
{
	QStringList blocksList = blocks.keys();
	for(int i=0; i < blocksList.count(); i++)
	{
		QVariantMap block = blocks.value(blocksList[i]);
		if(block.value("opcode").toString() == "event_whenbroadcastreceived")
		{
			QMap<QString,QString> inputs = m_engine->getInputs(block);
			if(inputs.value("BROADCAST_OPTION") == broadcastName)
			{
				// Stop running instances of this broadcast event
				QList<QVariantMap> operationsToRemove;
				operationsToRemove.clear();
				for(int i2=0; i2 < m_engine->currentExecPos.count(); i2++)
				{
					if(m_engine->currentExecPos[i2]["toplevelblock"] == blocksList[i])
						operationsToRemove += m_engine->currentExecPos[i2];
				}
				for(int i2=0; i2 < operationsToRemove.count(); i2++)
					m_engine->currentExecPos.removeAll(operationsToRemove[i2]);
				// Start the script
				QVariantMap blockMap;
				blockMap.clear();
				blockMap.insert("id",blocksList[i]);
				blockMap.insert("special","");
				if(script != nullptr)
					script->insert("activescripts",script->value("activescripts").toInt()+1);
				blockMap.insert("callerptr", (qlonglong) (intptr_t) script);
				m_engine->currentExecPos += blockMap;
			}
		}
	}
}

/*! Starts all "when I start as a clone" blocks. */
void scratchSprite::startClone(void)
{
	m_isClone = true;
	QStringList blocksList = blocks.keys();
	for(int i=0; i < blocksList.count(); i++)
	{
		QVariantMap block = blocks.value(blocksList[i]);
		if(block.value("opcode").toString() == "control_start_as_clone")
		{
			QVariantMap blockMap;
			blockMap.clear();
			blockMap.insert("id", blocksList[i]);
			blockMap.insert("special", "");
			m_engine->currentExecPos += blockMap;
		}
	}
}

/*! Stops the sprite. */
void scratchSprite::stopSprite(void)
{
	m_engine->currentExecPos.clear();
	if(!isStage)
	{
		speechBubble->setVisible(false);
		speechBubbleText->setVisible(false);
	}
	resetGraphicEffects();
}

/*! Sets sprite X position. */
void scratchSprite::setXPos(qreal x)
{
	spriteX = x;
	setX(translateX(x));
}

/*! Sets sprite Y position. */
void scratchSprite::setYPos(qreal y)
{
	spriteY = y;
	setY(translateY(y));
}

/*! Translates X position from Scratch coordinate system to QGraphicsScene coordinate system or vice versa. */
qreal scratchSprite::translateX(qreal x, bool toScratch)
{
	if(toScratch)
	{
		if(pointingLeft)
			return (x - rotationCenterX) / sceneScale;
		else
			return (x + rotationCenterX) / sceneScale;
	}
	else
	{
		if(pointingLeft)
			return x * sceneScale + rotationCenterX;
		else
			return x * sceneScale - rotationCenterX;
	}
}

/*! Translates Y position from Scratch coordinate system to QGraphicsScene coordinate system or vice versa. */
qreal scratchSprite::translateY(qreal y, bool toScratch)
{
	if(toScratch)
		return -y / sceneScale - rotationCenterY;
	else
		return -y * sceneScale - rotationCenterY;
}

/*! Used by projectScene to set mouse-pointer position. \see projectScene#mouseMoveEvent() */
void scratchSprite::setMousePos(QPointF pos)
{
	mouseX = pos.x();
	mouseY = -pos.y();
}

/*! Sets the sprite costume. */
void scratchSprite::setCostume(int id, QVariantMap *script)
{
	currentCostume = id;
	QString dataFormat = costumes[id].value("dataFormat").toString();
	QString assetId = costumes[id].value("assetId").toString();
	QByteArray data;
	if(assetDir == "")
		data = *projectAssets[assetId];
	else
	{
		QFile assetFile(assetDir + "/" + assetId + "." + dataFormat);
		assetFile.open(QFile::ReadOnly);
		data = assetFile.readAll();
	}
	double scale = 1;
	if((dataFormat == "svg") && settings.value("main/hqsvg", true).toBool())
	{
		QSvgRenderer renderer(data);
		QImage image(renderer.defaultSize() * sceneScale, QImage::Format_ARGB32);
		image.fill(0);
		QPainter painter(&image);
		renderer.render(&painter);
		costumePixmap = QPixmap::fromImage(image);
	}
	else
	{
		costumePixmap = QPixmap();
		costumePixmap.loadFromData(data);
		costumePixmap = costumePixmap.scaled(costumePixmap.width() * sceneScale, costumePixmap.height() * sceneScale);
	}
	costumePixmap = costumePixmap.scaledToHeight(costumePixmap.height() * scale);
	setPixmap(costumePixmap);
	rotationCenterX = costumes[id].value("rotationCenterX").toDouble() * scale * sceneScale;
	rotationCenterY = costumes[id].value("rotationCenterY").toDouble() * scale * sceneScale;
	setTransformOriginPoint(QPointF(rotationCenterX,rotationCenterY));
	setXPos(spriteX);
	setYPos(spriteY);
	installGraphicEffects();
	if((isStage) && (script != nullptr))
		emit backdropSwitched(script);
}

/*! Resets the values of all graphic effects. */
void scratchSprite::resetGraphicEffects(void)
{
	graphicEffects.clear();
	graphicEffects.insert("COLOR",0);
	graphicEffects.insert("FISHEYE",0);
	graphicEffects.insert("WHIRL",0);
	graphicEffects.insert("PIXELATE",0);
	graphicEffects.insert("MOSAIC",0);
	graphicEffects.insert("BRIGHTNESS",0);
	graphicEffects.insert("GHOST",0);
	installGraphicEffects();
}

/*!
 * Sets graphic effects.\n
 * Note: Only ghost, color and brightness is supported.
 */
void scratchSprite::installGraphicEffects(void)
{
	/*
	 * TODO: Implement other effects in the loop below.
	 * It should be possible without using QGraphicsEffect.
	 */
	qreal colorEffect = graphicEffects["COLOR"];
	qreal brightnessEffect = graphicEffects["BRIGHTNESS"];
	qreal ghostEffect = graphicEffects["GHOST"];
	if((colorEffect != 0) || (colorEffect != 0) || (ghostEffect != 0))
	{
		if((colorEffect == 0) && (brightnessEffect == 0))
		{
			setOpacity((100 - ghostEffect) / 100.0);
			return;
		}
		QImage costumeImage = costumePixmap.toImage().convertToFormat(QImage::Format_ARGB32);
		int h=0, s=0, v=0, a=0;
		for(int y=0; y < costumeImage.height(); y++)
		{
			for(int x=0; x < costumeImage.width(); x++)
			{
				costumeImage.pixelColor(x,y).getHsv(&h,&s,&v,&a);
				// Color effect
				h += colorEffect*1.8;
				h %= 360;
				if(h < 0)
					h += 360;
				// Brightness effect
				int brightness = brightnessEffect;
				if(brightness >= 100)
				{
					v = 255;
					s = 0;
				}
				else if(brightness > 0)
					s -= brightness * (s/100.0);
				else if(brightness > -100)
					v += brightness * (v/100.0);
				else if(brightness <= -100)
					v = 0;
				// Ghost effect
				int ghost = ghostEffect;
				if(ghost >= 100)
					a = 0;
				else if(ghost > 0)
					a -= ghost * (a/100.0);
				costumeImage.setPixelColor(x,y,QColor::fromHsv(h,s,v,a));
			}
		}
		setPixmap(QPixmap::fromImage(costumeImage));
	}
}

/*! Sets the sprite size. */
void scratchSprite::setSize(qreal newSize)
{
	if(newSize < 0)
		newSize = 0;
	setScale(newSize/100.0);
	size = newSize;
}

/*! Sets the sprite direction. */
void scratchSprite::setDirection(qreal angle)
{
	direction = angle;
	if(direction > 180)
		direction -= 360;
	else if(direction < -180)
		direction += 360;
	resetTransform();
	pointingLeft = false;
	if(rotationStyle == "left-right")
	{
		setRotation(0); // direction 90°
		if(direction < 0)
		{
			pointingLeft = true;
			setTransform(transform().scale(-1,1));
			setXPos(spriteX);
			setYPos(spriteY);
		}
		else
			setTransform(transform().scale(1,1));
	}
	else if(rotationStyle == "don't rotate")
	{
		setRotation(0); // direction 90°
		setTransform(transform().scale(1,1));
	}
	else if(rotationStyle == "all around")
	{
		setRotation(direction-90);
		setTransform(transform().scale(1,1));
	}
}

/*!
 * Starts playing a sound and returns a pointer to the QPointer<QMediaPlayer> object of the playing sound.\n
 * Returns nullptr if the sound isn't found.
 */
QPointer<QMediaPlayer> *scratchSprite::playSound(QString soundName)
{
	// Play the sound
	int soundID = -1;
	for(int i=0; i < sounds.count(); i++)
	{
		if(sounds[i].value("name").toString() == soundName)
		{
			soundID = i;
			break;
		}
	}
	if(soundID != -1)
	{
		QPointer<QMediaPlayer> sound = new QMediaPlayer(this);
		if(assetDir == "")
		{
			QBuffer *buffer = new QBuffer(sound);
			buffer->setData(*projectAssets[sounds[soundID].value("assetId").toString()]);
			buffer->open(QBuffer::ReadOnly);
			sound->setMedia(QMediaContent(), buffer);
		}
		else
			sound->setMedia(QUrl::fromLocalFile(assetDir + "/" + sounds[soundID].value("assetId").toString() + "." + sounds[soundID].value("dataFormat").toString()));
		sound->setVolume(volume);
		sound->play();
		connect(sound, &QMediaPlayer::stateChanged, this, [sound](QMediaPlayer::State state) {
			if(state == QMediaPlayer::StoppedState)
				sound->deleteLater(); // delete the sound after it stops
		});
		allSounds += sound;
		return &allSounds[allSounds.count() - 1];
	}
	else
		return nullptr;
}

/*! Stops all sounds. */
void scratchSprite::stopAllSounds(void)
{
	for(int i=0; i < allSounds.count(); i++)
	{
		if(!allSounds[i].isNull() && (allSounds[i]->state() == QMediaPlayer::PlayingState))
			allSounds[i]->stop();
	}
	allSounds.clear();
}

/*! Sets sound volume. */
void scratchSprite::setVolume(qreal newVolume)
{
	volume = newVolume;
	for(int i=0; i < allSounds.count(); i++)
		allSounds[i]->setVolume(newVolume/100.0);
}

/*! Shows a speech or thought bubble. */
void scratchSprite::showBubble(QString text, bool thought)
{
	if(text == "")
	{
		speechBubble->setVisible(false);
		speechBubbleText->setVisible(false);
		return;
	}
	if(thought)
		speechBubble->setPixmap(QPixmap(":res/images/thought_bubble.png"));
	else
		speechBubble->setPixmap(QPixmap(":res/images/speech_bubble.png"));
	speechBubbleText->setPlainText(text);
	speechBubble->setPos(boundingRect().right(),boundingRect().top()-35);
	speechBubbleText->setTextWidth(140);
	speechBubbleText->setPos(speechBubble->pos() + QPointF(30,speechBubbleText->boundingRect().height()/3.0));
	speechBubble->setTransform(QTransform().scale(speechBubbleText->boundingRect().width()/65.0,speechBubbleText->boundingRect().height()/35.0));
	speechBubble->setVisible(true);
	speechBubbleText->setVisible(true);
}

/*! Runs blocks that can be run without screen refresh.*/
void scratchSprite::frame(void)
{
	m_engine->frame();
}

/*! Returns a pointer to the Engine of this sprite. */
Engine* scratchSprite::engine(void)
{
	return m_engine;
}

/*! Sets scene scale. */
void scratchSprite::setSceneScale(qreal value)
{
	sceneScale = value;
	setXPos(spriteX);
	setYPos(spriteY);
	setCostume(currentCostume);
}

/*! Returns true if this is a clone. */
bool scratchSprite::isClone(void)
{
	return m_isClone;
}
