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
	volume = spriteObject.value("volume").toInt();
	tempo = spriteObject.value("tempo").toInt();
	if(isStage)
	{
		setVisible(true);
		setXPos(0);
		setYPos(0);
		size = 100;
		setDirection(90);
		draggable = false;
	}
	else
	{
		setVisible(spriteObject.value("visible").toBool());
		setXPos(spriteObject.value("x").toDouble());
		setYPos(spriteObject.value("y").toDouble());
		size = spriteObject.value("size").toInt();
		rotationStyle = spriteObject.value("rotationStyle").toString();
		setDirection(spriteObject.value("direction").toDouble());
		draggable = spriteObject.value("draggable").toBool();
	}
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

/*! Stops the sprite. */
void scratchSprite::stopSprite(void)
{
	currentExecPos.clear();
	// TODO: Remove event loops depending on obsolete stopScripts() signal.
	emit stopScripts();
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
	QPixmap costumePixmap(assetDir + "/" + costumes[id].value("md5ext").toString());
	setPixmap(costumePixmap);
	rotationCenterX = costumes[id].value("rotationCenterX").toDouble();
	rotationCenterY = costumes[id].value("rotationCenterY").toDouble();
	setTransformOriginPoint(QPointF(rotationCenterX,rotationCenterY));
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
			motionBlocks(opcode,inputs,frame_i,&frameEnd,&processEnd);
			// Get next block
			QVariant nextValue = block.value("next");
			if(nextValue.isNull())
			{
				currentExecPos[frame_i]["id"] = currentID;
				operationsToRemove += currentID;
				frameEnd = true;
			}
			else
			{
				next = nextValue.toString();
				if(frameEnd)
					currentExecPos[frame_i]["id"] = currentID;
				if(processEnd)
					currentExecPos[frame_i]["id"] = next;
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
		QString finalValue;
		if(readFields)
		{
			// Input is in the first item
			finalValue = blockInputs.value(blockInputsList[i]).toArray().at(0).toString();
		}
		else if(inputValue.isArray())
		{
			// Input representation as an array
			finalValue = inputValue.toArray().at(1).toString();
		}
		else
		{
			// Load reporter block
			// Note: Dropdown menus and color inputs are treated as reporter blocks
			QVariantMap reporterBlock = blocks.value(inputValue.toString());
			QString opcode = reporterBlock.value("opcode").toString();
			QMap<QString,QString> inputs = getInputs(reporterBlock);
			// Get reporter block value
			if(opcode == "motion_pointtowards_menu")
				finalValue = inputs.value("TOWARDS");
			else if(opcode == "motion_goto_menu")
				finalValue = inputs.value("TO");
			else if(opcode == "motion_glideto_menu")
				finalValue = inputs.value("TO");
			else if(opcode == "motion_xposition")
				finalValue = QString::number(spriteX);
			else if(opcode == "motion_yposition")
				finalValue = QString::number(spriteY);
			else if(opcode == "motion_direction")
				finalValue = QString::number(direction);
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
