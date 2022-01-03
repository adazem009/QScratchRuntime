/*
 * blocks.cpp
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

/*
 * This file was created to make the scratchSprite class simpler to read.
 * All functions in this file are members of scratchSprite.
 */

#include "core/blocks.h"

/*! Runs motion blocks. */
bool scratchSprite::motionBlocks(QString opcode, QMap<QString,QString> inputs, bool *frameEnd)
{
	if(frameEnd == nullptr)
		frameEnd = new bool;
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
		*frameEnd = true;
		// TODO: Implement glide as a multi-frame operation.
		/*qreal endX = 0, endY = 0;
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
		setYPos(endY);*/
		
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
	else
		return false;
	return true;
}
