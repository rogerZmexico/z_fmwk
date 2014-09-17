/****************************************************************************
** $Id: plot.h 
** project:  zserver
** author:   giovanniangeli@tbe.it
** created at: 
** license: GPL
    
    Copyright (C) 2007  giovanni angeli www.tbe.it

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



#ifndef _PLOT___H
#define _PLOT___H

#include <qpushbutton.h>
#include <qpen.h>

#include "globals.h"

#define MAX_NOF_SECTIONS 10

class QPtrlist;
class QPen;
class QColor;
class QPushButton;

typedef struct 
{
    int x;
    int y;
} iPoint ;

class Plot : public QPushButton
{
    Q_OBJECT

public:
    
    Plot( QWidget* parent=NULL, const char * name=NULL );
    ~Plot();

    QPtrList<iPoint> sections[MAX_NOF_SECTIONS];
    int currentSection;
    QPen sectionPen[MAX_NOF_SECTIONS];
    QPen gridPen;

    bool appendmode;
    int gridSteps[2];
    
    void paintEvent( QPaintEvent * );

    void clearPoints(int section);

    bool loadPoints( const QString & fileName, int sectionNumber, bool appendmode=FALSE );

    bool addPoint( int x, int y, int sectionNumber) ;
    bool setPenWidth( int w, int sectionNumber ) ;
    bool setPenColor( const QColor & c, int sectionNumber ) ;

public slots:
    
    void clearAllPoints();
    void clearPoints();

    void setCurrentSection(int s);
    void setGridSteps(int sx, int sy);
    void setGridPenWidth(int w);
    void setGridPenColor(const QString & c);
    void setAppendmode(int m);
    void addPoint(int x, int y);
    void setPenWidth(int w);
    void setPenColor(const QString & c) ;
    void loadPoints(const QString & fileName);

};

#endif // _PLOT___H


