/****************************************************************************
** $Id: rotor.cpp
** project:  zserver
** author:   giovanniangeli@tbe.it
** created at:
** license: GPL

    Copyright (C) 2007-2009  giovanni angeli www.tbe.it

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

****************************************************************************/
#include <qframe.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qpixmap.h>

#include "rotor.h"

Rotor::Rotor( QWidget* parent, const char * name, const QString & imageFileName, bool do_createHeuMask)
    : QFrame(parent, name )
    ,debugLevel(0)
    ,angle(0)
    ,pixMap(NULL)
    ,do_createHeuristicMask(do_createHeuMask)
    ,imgFileName("")
{
    MY_WARNING(0x01)( "%s. build: %s [%s]",__FILE__, __DATE__,__TIME__);

    this->setPixMap(imageFileName);

    this->connect(this, SIGNAL(angleChanged(int)), this, SLOT(drawRotatedPixMap(int)));
}

Rotor::~Rotor()
{
    //~  parents take care of destroying children
}

//~ slots: ##############################
void Rotor::drawRotatedPixMap(int angle)
{
    MY_WARNING(0x01)("%s:%d:%s(%d)",__FILE__, __LINE__,__FUNCTION__, angle);

    QPainter painter(this);

    painter.translate( this->pixMap->width()/2, this->pixMap->height()/2 );
    painter.rotate( - angle );
    const QPoint p = QPoint( -this->pixMap->width()/2, -this->pixMap->height()/2);
    painter.drawPixmap(p, *( this->pixMap ));
    painter.end();

    //~ this->redraw();
    this->show();
}

void Rotor::setAngle(const QString & val)
{
    bool ok ;
    int v = val.toInt(&ok);

    if(ok) this->setAngle(v);
    else
    {
        MY_WARNING(0x01)( "%s:%d:%s(%s) cannot set angle.",__FILE__, __LINE__,__FUNCTION__, val.ascii());
    }
};

void Rotor::setAngle(int angle)
{
    MY_WARNING(0x01)( "%s:%d:%s(%d)",__FILE__, __LINE__,__FUNCTION__, angle);

    if(angle != this->angle)
    {
        this->angle = angle;
        emit this->angleChanged(this->angle);
    }
};

void Rotor::setPixMap(const QString & imageFileName)
{

    this->imgFileName = imageFileName;

    this->pixMap = new QPixmap(imageFileName);
    if(this->pixMap == NULL)
    {
        MY_WARNING(0x01)( "%s:%d:%s() ERR: cannot load image from:\"%s\"",__FILE__, __LINE__,__FUNCTION__, imageFileName.latin1());
    }
    else
    {
        MY_WARNING(0x01)( "%s:%d:%s() loaded image from:\"%s\"",__FILE__, __LINE__,__FUNCTION__, imageFileName.latin1());

        if(this->do_createHeuristicMask)
        {
            this->pixMap->setMask( this->pixMap->createHeuristicMask() );
        }

        this->resize(this->pixMap->width(), this->pixMap->height() );
    }
}

void Rotor::setDo_createHeuristicMask(int flag)
{
    this->do_createHeuristicMask = flag;
}

void Rotor::setDbgLevel(int level)
{
    this->debugLevel = level;
}
