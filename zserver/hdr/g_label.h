/****************************************************************************
** $Id: g_label.h
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



#ifndef _G_LABEL___H
#define _G_LABEL___H

#include <qlabel.h>
#include <qbutton.h>

#include "globals.h"

class QTimer;

class G_Label : public QLabel
{
    Q_OBJECT

public:

    G_Label( QWidget* parent, const char* name);
    ~G_Label();

    //~ void drawContents( QPainter *p );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

    QTimer * timer;

private :

    bool hitButton( const QPoint &pos ) const ;

    bool mlbDown;
    bool buttonDown;
    bool autorepeat;

public slots:

    void setAutorepeat( int r );
    //~ void setBgPixmap( const QString &name );
    void setWidgetErasePixmap( const QString & pixmapNameAndPath);

private slots:

    void	autoRepeatTimeout();

signals:

    void pressed();
    void released();
    void clicked();

};

#endif // _G_LABEL___H


