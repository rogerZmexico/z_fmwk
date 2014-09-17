/****************************************************************************
** $Id: rotor.h
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



#ifndef _ROTOR___H
#define _ROTOR___H

#include <qframe.h>

#include "globals.h"

class QPixmap;

class Rotor : public QFrame
{
    Q_OBJECT

public:

    int debugLevel;
    int angle;
    QPixmap * pixMap;
    bool do_createHeuristicMask;
    QString imgFileName;

    Rotor( QWidget* parent, const char* name, const QString& imageFileName="rotor.png", bool do_createHeuristicMask=1);
    ~Rotor();

public slots:

    void setAngle(const QString&);
    void setAngle(int);
    void setPixMap(const QString&);
    void drawRotatedPixMap(int);
    void setDo_createHeuristicMask(int flag);
    void setDbgLevel(int level);

signals:

    void angleChanged(int);
};

#endif // _ROTOR___H


