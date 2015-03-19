/* Copyright (c) 2012, STANISLAW ADASZEWSKI
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of STANISLAW ADASZEWSKI nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL STANISLAW ADASZEWSKI BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "qneblock.h"

#include <QPen>
#include <QGraphicsScene>
#include <QFontMetrics>
#include <QPainter>
#include <QFileDialog>
#include <QObject>
#include <iostream>

#include "qneport.h"

QNEBlock::QNEBlock(QGraphicsItem *parent) : QGraphicsPathItem(parent)
{
    QPainterPath p;
    p.addRoundedRect(-50, -15, 100, 30, 5, 5);
    setPath(p);
    setPen(QPen(Qt::darkGreen));
	setBrush(Qt::green);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
    horzMargin = 50;
    vertMargin = 20;
	width = horzMargin;
	height = vertMargin;

    //----mod by Declan Russell----
    QPainterPath p2;
    p2.addRoundedRect(-25,-10,30,10,5,5);
    m_importBtnGI = new QGraphicsPathItem(this);
    m_importBtnGI->setPath(p2);
    m_importBtnGI->setPen(QPen(Qt::darkCyan));
    m_importBtnGI->setBrush(Qt::cyan);
}

QNEPort* QNEBlock::addPort(const QString &name, bool isOutput, int flags, int ptr)
{
	QNEPort *port = new QNEPort(this);
	port->setName(name);
	port->setIsOutput(isOutput);
	port->setNEBlock(this);
	port->setPortFlags(flags);
	port->setPtr(ptr);

	QFontMetrics fm(scene()->font());
	int w = fm.width(name);
	int h = fm.height();
	// port->setPos(0, height + h/2);
	if (w > width - horzMargin)
		width = w + horzMargin;
	height += h;

	QPainterPath p;
	p.addRoundedRect(-width/2, -height/2, width, height, 5, 5);
	setPath(p);

	int y = -height / 2 + vertMargin + port->radius();
    foreach(QGraphicsItem *port_, childItems()) {
		if (port_->type() != QNEPort::Type)
			continue;

		QNEPort *port = (QNEPort*) port_;
		if (port->isOutput())
			port->setPos(width/2 + port->radius(), y);
		else
			port->setPos(-width/2 - port->radius(), y);
		y += h;
	}

	return port;
}

void QNEBlock::addInputPort(const QString &name)
{
	addPort(name, false);
}

void QNEBlock::addOutputPort(const QString &name)
{
	addPort(name, true);
}

void QNEBlock::addInputPorts(const QStringList &names)
{
	foreach(QString n, names)
		addInputPort(n);
}

void QNEBlock::addOutputPorts(const QStringList &names)
{
	foreach(QString n, names)
		addOutputPort(n);
}

void QNEBlock::save(QDataStream &ds)
{
	ds << pos();

	int count(0);

    foreach(QGraphicsItem *port_, childItems())
	{
		if (port_->type() != QNEPort::Type)
			continue;

		count++;
	}

	ds << count;

    foreach(QGraphicsItem *port_, childItems())
	{
		if (port_->type() != QNEPort::Type)
			continue;

		QNEPort *port = (QNEPort*) port_;
		ds << (quint64) port;
		ds << port->portName();
		ds << port->isOutput();
		ds << port->portFlags();
	}
}

void QNEBlock::load(QDataStream &ds, QMap<quint64, QNEPort*> &portMap)
{
	QPointF p;
	ds >> p;
	setPos(p);
	int count;
	ds >> count;
	for (int i = 0; i < count; i++)
	{
		QString name;
		bool output;
		int flags;
		quint64 ptr;

		ds >> ptr;
		ds >> name;
		ds >> output;
		ds >> flags;
		portMap[ptr] = addPort(name, output, flags, ptr);
	}
}

#include <QStyleOptionGraphicsItem>

void QNEBlock::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (isSelected()) {
		painter->setPen(QPen(Qt::darkYellow));
		painter->setBrush(Qt::yellow);
	} else {
		painter->setPen(QPen(Qt::darkGreen));
		painter->setBrush(Qt::green);
	}
    painter->drawPath(path());


}

QNEBlock* QNEBlock::clone()
{
    QNEBlock *b = new QNEBlock(0);
    this->scene()->addItem(b);

	foreach(QGraphicsItem *port_, childItems())
	{
		if (port_->type() == QNEPort::Type)
		{
			QNEPort *port = (QNEPort*) port_;
			b->addPort(port->portName(), port->isOutput(), port->portFlags(), port->ptr());
		}
	}

	return b;
}

QVector<QNEPort*> QNEBlock::ports()
{
	QVector<QNEPort*> res;
	foreach(QGraphicsItem *port_, childItems())
	{
		if (port_->type() == QNEPort::Type)
			res.append((QNEPort*) port_);
	}
    return res;
}

//----------------------------------------------------------------------------------------------------------------------
void QNEBlock::mousePressEvent(QGraphicsSceneMouseEvent *_event)
{
    //get the possition of our button and the positions of our mouse
    QRectF btnBB = m_importBtnGI->path().boundingRect();
    QPointF pos = _event->pos();
    //test for intersection of the two
    if(pos.x()>btnBB.x()&&pos.x()<(btnBB.x()+btnBB.width())&&(pos.y()>btnBB.y()&&pos.y()<(btnBB.y()+btnBB.height()))){
        //if we have an intersection lets allow the user to select a file in import
        QString location = QFileDialog::getOpenFileName(0,QString("Import Shader"), QString("shaders/"), QString("OSL files (*.osl)"));
        //if nothing selected then we dont want to do anything
        if(location.isEmpty()) return;
        //if we do have something selected lets run it through Toby Gilbert's OSL Compilotmatic 3000
        OslReader shade;
        shade.compileOSL(location);

        OsoReader* reader = getOsoReader();
        reader->printVersion();
        reader->printShader();
        reader->printParams();
        reader->generateDeviceFunction();

        // RETURNS THE INPUT PARAMS
        std::vector<Symbol> symbols = reader->getInputParams();
        for (unsigned int i=0; i<symbols.size(); i++){
            std::cout<<symbols[i].m_name<<std::endl;
            addInputPort(QString(symbols[i].m_name.c_str()));
        }

    }
}
//----------------------------------------------------------------------------------------------------------------------

QVariant QNEBlock::itemChange(GraphicsItemChange change, const QVariant &value)
{

    Q_UNUSED(change);

	return value;
}

