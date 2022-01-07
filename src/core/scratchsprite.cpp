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

/*! Constructs scratchSprite. */
scratchSprite::scratchSprite(QJsonObject spriteObject, QString spriteAssetDir, QGraphicsItem *parent) :
	QGraphicsPixmapItem(parent)
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
	// Load sounds
	QJsonArray soundsArray = spriteObject.value("sounds").toArray();
	sounds.clear();
	for(i=0; i < soundsArray.count(); i++)
		sounds += soundsArray[i].toObject().toVariantMap();
	// TODO: Load variables
	// TODO: Load lists
	// TODO: Load broadcasts
	// Load blocks
	QJsonObject blocksObject = spriteObject.value("blocks").toObject();
	QStringList blocksList = blocksObject.keys();
	blocks.clear();
	for(i=0; i < blocksList.count(); i++)
		blocks.insert(blocksList[i],blocksObject.value(blocksList[i]).toObject().toVariantMap());
}

/*! Loads list of sprite pointers. */
void scratchSprite::loadSpriteList(QList<scratchSprite*> list)
{
	spriteList = list;
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
	stopSprite();
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
			currentExecPos += blockMap;
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
		if(block.value("opcode").toString() == "event_whenthisspriteclicked")
		{
			QVariantMap blockMap;
			blockMap.clear();
			blockMap.insert("id",blocksList[i]);
			blockMap.insert("special","");
			currentExecPos += blockMap;
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
			QMap<QString,QString> inputs = getInputs(block);
			if(checkKey(key,keyText,inputs.value("KEY_OPTION")))
			{
				QVariantMap blockMap;
				blockMap.clear();
				blockMap.insert("id",blocksList[i]);
				blockMap.insert("special","");
				currentExecPos += blockMap;
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

/*! Stops the sprite. */
void scratchSprite::stopSprite(void)
{
	currentExecPos.clear();
	if(!isStage)
		speechBubble->setVisible(false);
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
	if(toScratch != pointingLeft)
		return x+rotationCenterX;
	else
		return x-rotationCenterX;
}

/*! Translates Y position from Scratch coordinate system to QGraphicsScene coordinate system or vice versa. */
qreal scratchSprite::translateY(qreal y, bool toScratch)
{
	if(toScratch)
		return -y-rotationCenterY;
	else
		return -y-rotationCenterY;
}

/*! Used by projectScene to set mouse-pointer position. \see projectScene#mouseMoveEvent() */
void scratchSprite::setMousePos(QPointF pos)
{
	mouseX = pos.x();
	mouseY = -pos.y();
}

/*! Sets the sprite costume. */
void scratchSprite::setCostume(int id)
{
	currentCostume = id;
	costumePixmap = QPixmap(assetDir + "/" + costumes[id].value("assetId").toString() + "." + costumes[id].value("dataFormat").toString());
	setPixmap(costumePixmap);
	rotationCenterX = costumes[id].value("rotationCenterX").toDouble();
	rotationCenterY = costumes[id].value("rotationCenterY").toDouble();
	setTransformOriginPoint(QPointF(rotationCenterX,rotationCenterY));
	setXPos(spriteX);
	setYPos(spriteY);
	installGraphicEffects();
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
 * Sets graphic effects.
 * Note: Only ghost, color and brightness is supported.
 */
void scratchSprite::installGraphicEffects(void)
{
	/*
	 * TODO: Implement other effects in the loop below.
	 * It should be possible without using QGraphicsEffect.
	 */
	QImage costumeImage = costumePixmap.toImage().convertToFormat(QImage::Format_ARGB32);
	if((graphicEffects["COLOR"] != 0) || (graphicEffects["BRIGHTNESS"] != 0) || (graphicEffects["GHOST"] != 0))
	{
		for(int y=0; y < costumeImage.height(); y++)
		{
			for(int x=0; x < costumeImage.width(); x++)
			{
				QColor pixelColor = costumeImage.pixelColor(x,y);
				int h, s, v, a;
				pixelColor.getHsv(&h,&s,&v,&a);
				// Color effect
				h += graphicEffects["COLOR"]*1.8;
				// Brightness effect
				int brightness = graphicEffects["BRIGHTNESS"];
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
				int ghost = graphicEffects["GHOST"];
				if(ghost >= 100)
					a = 0;
				else if(ghost > 0)
					a -= ghost * (a/100.0);
				pixelColor.setHsv(h,s,v,a);
				costumeImage.setPixelColor(x,y,pixelColor);
			}
		}
	}
	setPixmap(QPixmap::fromImage(costumeImage));
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
 * Starts playing a sound and returns a pointer to the QSoundEffect object of the playing sound.\n
 * Returns nullptr if the sound isn't found.
 */
QSoundEffect *scratchSprite::playSound(QString soundName)
{
	// Remove finished sounds
	QList<QSoundEffect*> soundsToRemove;
	soundsToRemove.clear();
	for(int i=0; i < allSounds.count(); i++)
	{
		if(!allSounds[i]->isPlaying())
			soundsToRemove += allSounds[i];
	}
	for(int i=0; i < soundsToRemove.count(); i++)
		allSounds.removeAll(soundsToRemove[i]);
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
		QSoundEffect *sound = new QSoundEffect();
		sound->setSource(QUrl::fromLocalFile(assetDir + "/" + sounds[soundID].value("assetId").toString() + "." + sounds[soundID].value("dataFormat").toString()));
		sound->setVolume(volume/100.0);
		sound->play();
		allSounds += sound;
		return sound;
	}
	else
		return nullptr;
}

/*! Stops all sounds. */
void scratchSprite::stopAllSounds(void)
{
	for(int i=0; i < allSounds.count(); i++)
		allSounds[i]->stop();
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
	QStringList operationsToRemove;
	operationsToRemove.clear();
	for(int frame_i=0; frame_i < currentExecPos.count(); frame_i++)
	{
		QString next = currentExecPos[frame_i]["id"].toString();
		bool frameEnd = false;
		while(!frameEnd)
		{
			// Load current block
			QString currentID = next;
			QVariantMap block = blocks.value(currentID);
			QString opcode = block.value("opcode").toString();
			QMap<QString,QString> inputs = getInputs(block);
			bool processEnd = false;
			// Run current block
			motionBlocks(opcode,inputs,frame_i,&frameEnd,&processEnd) ||
			looksBlocks(opcode,inputs,frame_i,&frameEnd,&processEnd) ||
			soundBlocks(opcode,inputs,frame_i,&frameEnd,&processEnd);
			// Get next block
			QVariant nextValue = block.value("next");
			if(frameEnd)
				currentExecPos[frame_i]["id"] = currentID;
			else if(nextValue.isNull())
			{
				currentExecPos[frame_i]["id"] = currentID;
				operationsToRemove += currentID;
				frameEnd = true;
			}
			else
			{
				next = nextValue.toString();
				if(processEnd)
				{
					currentExecPos[frame_i]["id"] = next;
					currentExecPos[frame_i]["special"] = "";
				}
			}
		}
	}
	for(int i=0; i < operationsToRemove.count(); i++)
	{
		bool removeEnd = false;
		while(!removeEnd)
		{
			removeEnd = true;
			for(int i2=0; i2 < currentExecPos.count(); i2++)
			{
				if(currentExecPos[i2]["id"].toString() == operationsToRemove[i])
				{
					currentExecPos.removeAt(i2);
					removeEnd = false;
					break;
				}
			}
		}
	}
}

/*! Reads block inputs and fields and returns a map. */
QMap<QString,QString> scratchSprite::getInputs(QVariantMap block, bool readFields)
{
	QJsonObject blockInputs;
	if(readFields)
		blockInputs = block.value("fields").toJsonObject();
	else
		blockInputs = block.value("inputs").toJsonObject();
	QStringList blockInputsList = blockInputs.keys();
	QMap<QString,QString> out;
	out.clear();
	for(int i=0; i < blockInputsList.count(); i++)
	{
		QJsonValue inputValue = blockInputs.value(blockInputsList[i]).toArray().at(1);
		bool typeConverted = false;
		QJsonValue finalRawValue;
		QString finalValue = "";
		if(readFields)
		{
			// Input is in the first item
			finalRawValue = blockInputs.value(blockInputsList[i]).toArray().at(0);
		}
		else if(inputValue.isArray())
		{
			// Input representation as an array
			finalRawValue = inputValue.toArray().at(1);
		}
		else
		{
			typeConverted = true;
			// Load reporter block
			// Note: Dropdown menus and color inputs are treated as reporter blocks
			QVariantMap reporterBlock = blocks.value(inputValue.toString());
			QString opcode = reporterBlock.value("opcode").toString();
			QMap<QString,QString> inputs = getInputs(reporterBlock);
			// Get reporter block value
			motionBlocks(opcode,inputs,0,nullptr,nullptr,&finalValue) ||
			looksBlocks(opcode,inputs,0,nullptr,nullptr,&finalValue) ||
			soundBlocks(opcode,inputs,0,nullptr,nullptr,&finalValue);
		}
		if(!typeConverted)
		{
			if(finalRawValue.isString())
				finalValue = finalRawValue.toString();
			else if(finalRawValue.isDouble())
				finalValue = QString::number(finalRawValue.toDouble());
			else if(finalRawValue.isBool())
				finalValue = finalRawValue.toString();
		}
		out.insert(blockInputsList[i],finalValue);
	}
	if(!readFields)
	{
		// TODO: Equivalent to this is out.insert(getInputs(block,true)); but this works in Qt >= 5.15
		QMap<QString,QString> inputs = getInputs(block,true);
		QStringList inputList = inputs.keys();
		for(int i=0; i < inputList.count(); i++)
			out.insert(inputList[i],inputs.value(inputList[i]));
	}
	return out;
}
