/*
 * scratchsprite.cpp
 * This file is part of QScratchRuntime
 *
 * Copyright (C) 2021 - adazem009
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
		rotationStyle = spriteObject.value("draggable").toString();
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
			startScript(blocksList[i]);
	}
}

/*! Stops the sprite. */
void scratchSprite::stopSprite(void)
{
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

/*! Starts a script. */
void scratchSprite::startScript(QString id)
{
	QString next = id;
	while(true)
	{
		// Load current block
		QVariantMap block = blocks.value(next);
		QString opcode = block.value("opcode").toString();
		QMap<QString,QString> inputs = getInputs(block);
		// Run current block
		if(opcode == "motion_movesteps")
		{
			qreal steps = inputs.value("STEPS").toDouble();
			// https://en.scratch-wiki.info/wiki/Move_()_Steps_(block)#Workaround
			setXPos(spriteX + qSin(qDegreesToRadians(direction))*steps);
			setYPos(spriteY + qCos(qDegreesToRadians(direction))*steps);
		}
		else if(opcode == "motion_turnright")
			setDirection(direction + inputs.value("DEGREES").toDouble());
		else if(opcode == "motion_turnleft")
			setDirection(direction - inputs.value("DEGREES").toDouble());
		else if(opcode == "motion_pointindirection")
			setDirection(inputs.value("DIRECTION").toDouble());
		else if(opcode == "motion_pointtowards")
		{
			QString targetName = inputs.value("TOWARDS");
			scratchSprite *targetSprite = getSprite(targetName);
			qreal deltaX = 0, deltaY = 0;
			if(targetSprite == nullptr)
			{
				deltaX = mouseX - spriteX;
				deltaY = mouseY - spriteY;
			}
			else
			{
				deltaX = targetSprite->spriteX - spriteX;
				deltaY = targetSprite->spriteY - spriteY;
			}
			// https://en.scratch-wiki.info/wiki/Point_Towards_()_(block)#Workaround
			if(deltaY == 0)
			{
				if(deltaX < 0)
					setDirection(-90);
				else
					setDirection(90);
			}
			else
			{
				qreal atanResult = qRadiansToDegrees(qAtan(deltaX/deltaY));
				if(deltaY < 0)
					setDirection(180 + atanResult);
				else
					setDirection(atanResult);
			}
		}
		else if(opcode == "motion_gotoxy")
		{
			setXPos(inputs.value("X").toDouble());
			setYPos(inputs.value("Y").toDouble());
		}
		else if(opcode == "motion_goto")
		{
			QString targetName = inputs.value("TO");
			scratchSprite *targetSprite = getSprite(targetName);
			if(targetSprite == nullptr)
			{
				if(targetName == "_mouse_")
				{
					setXPos(mouseX);
					setYPos(mouseY);
				}
				else if(targetName == "_random_")
				{
					setXPos(QRandomGenerator::global()->bounded(-240,240));
					setYPos(QRandomGenerator::global()->bounded(-180,180));
				}
			}
			else
			{
				setXPos(targetSprite->spriteX);
				setYPos(targetSprite->spriteY);
			}
		}
		else if((opcode == "motion_glidesecstoxy") || (opcode == "motion_glideto"))
		{
			qreal endX = 0, endY = 0;
			if(opcode == "motion_glidesecstoxy")
			{
				endX = inputs.value("X").toDouble();
				endY = inputs.value("Y").toDouble();
			}
			else
			{
				QString targetName = inputs.value("TO");
				scratchSprite *targetSprite = getSprite(targetName);
				if(targetSprite == nullptr)
				{
					if(targetName == "_mouse_")
					{
						endX = mouseX;
						endY = mouseY;
					}
					else if(targetName == "_random_")
					{
						endX = QRandomGenerator::global()->bounded(-240,240);
						endY = QRandomGenerator::global()->bounded(-180,180);
					}
				}
				else
				{
					endX = targetSprite->spriteX;
					endY = targetSprite->spriteY;
				}
			}
			// Animation timer
			QTimeLine *timer = new QTimeLine(inputs.value("SECS").toDouble()*1000);
			timer->setFrameRange(0,inputs.value("SECS").toFloat()*20);
			timer->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
			// Event loop
			QEventLoop animLoop;
			connect(timer,&QTimeLine::finished,&animLoop,&QEventLoop::quit);
			connect(this,&scratchSprite::stopScripts,timer,&QTimeLine::stop);
			// Animation
			QGraphicsItemAnimation *glideAnim = new QGraphicsItemAnimation;
			glideAnim->setItem(this);
			glideAnim->setTimeLine(timer);
			glideAnim->setPosAt(0.0,QPointF(translateX(spriteX),translateY(spriteY)));
			glideAnim->setPosAt(1.0,QPointF(translateX(endX),translateY(endY)));
			timer->start();
			animLoop.exec();
			setXPos(endX);
			setYPos(endY);
			
		}
		else if(opcode == "motion_changexby")
			setXPos(spriteX + inputs.value("DX").toDouble());
		else if(opcode == "motion_setx")
			setXPos(inputs.value("X").toDouble());
		else if(opcode == "motion_changeyby")
			setYPos(spriteY + inputs.value("DY").toDouble());
		else if(opcode == "motion_sety")
			setYPos(inputs.value("Y").toDouble());
		else if(opcode == "motion_ifonedgebounce")
		{
			QRectF spriteRect = boundingRect();
			// Right edge
			if(spriteX + (spriteRect.width()/2) > 240)
			{
				setDirection(-direction);
				setXPos(240 - (spriteRect.width()/2));
			}
			// Left edge
			if(spriteX - (spriteRect.width()/2) < -240)
			{
				setDirection(-direction);
				setXPos(-240 + (spriteRect.width()/2));
			}
			// Top edge
			if(spriteY + (spriteRect.height()/2) > 180)
			{
				setDirection(180-direction);
				setYPos(180 - (spriteRect.height()/2));
			}
			// Bottom edge
			if(spriteY - (spriteRect.height()/2) < -180)
			{
				setDirection(180-direction);
				setYPos(-180 + (spriteRect.height()/2));
			}
		}
		else if(opcode == "motion_setrotationstyle")
		{
			rotationStyle = inputs.value("STYLE");
			setDirection(direction);
		}
		// Get next block
		QVariant nextValue = block.value("next");
		if(nextValue.isNull())
			break;
		next = nextValue.toString();
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
