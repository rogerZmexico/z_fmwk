/****************************************************************************
** $Id: plot.cpp   
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


#include <qpainter.h>
#include <qfile.h>
#include <qptrlist.h>
#include <qcolor.h>

#include "plot.h"

Plot::Plot( QWidget* parent, const char * name  )
    : QPushButton(parent, name )
    ,sections()
    ,currentSection(0)
    ,sectionPen()
    ,gridPen(QColor(yellow), 2)
    ,appendmode(FALSE)
    ,gridSteps()
{
    qWarning( "%s. build: %s [%s]",__FILE__, __DATE__,__TIME__);
    
    this->gridPen.setStyle(Qt::DashLine);
    this->gridPen.setColor(QColor(white));
    this->gridPen.setWidth(1);
    this->gridSteps[0] = 1 ;
    this->gridSteps[1] = 1 ;
    
    for(int i=0; i<MAX_NOF_SECTIONS; i++)
    {
        this->sections[i].setAutoDelete( TRUE );
        this->sectionPen[i] = QPen( QColor(yellow), 4);
    }
}

Plot::~Plot()
{
    //~  parents take care of destroying children 
}

void Plot::paintEvent( QPaintEvent * )
{
    QPainter paint( this );

    int d[2];
    
    d[0] = this->height();
    d[1] = this->width();
    for(int i=0; i<2; i++)
    { /* drawGrid */
        if(this->gridSteps[i] > 2)
        {
            paint.setPen( this->gridPen ); 
            int n_of_steps = d[i]/this->gridSteps[i] ;
            for(int j=0; j < n_of_steps; j++)
            {
                int p = (j+1) * this->gridSteps[i] ;
                int x0=0, x1=0 ,y0=0, y1=0;
                if(i == 0){
                    y0 = d[0] - p;
                    y1 = d[0] - p;
                    x1 = d[1];
                }else{
                    x0 = p;
                    x1 = p;
                    y1 = d[0];
                }
                paint.drawLine( QPoint(x0, y0), QPoint(x1, y1)); 
                //~ qWarning( "%s:%d:%s() x0=%d, y0=%d, x1=%d, y1=%d. ", __FILE__, __LINE__ ,__FUNCTION__ , x0, y0, x1, y1 );
            }
        }
    }

    for(int i=0; i<MAX_NOF_SECTIONS; i++)
    { /* draw plot */
        paint.setPen( this->sectionPen[i] ); 
        QPtrList<iPoint> pl = this->sections[i];
        iPoint *p0=NULL;
        iPoint *p1=NULL;
        for ( p0 = pl.first(); p0; p0 = pl.next() )
        {
            if ( p1 == NULL ) p1 = p0;
            paint.drawLine( QPoint(p0->x, d[0] - p0->y), QPoint(p1->x, d[0] - p1->y) ); 
            p1 = p0;
        }
    }
}

void Plot::clearPoints(int sectionNumber)
{
    if( sectionNumber<MAX_NOF_SECTIONS )    
    {
        this->sections[sectionNumber].clear() ;
    }
}

bool Plot::loadPoints( const QString & fileName, int sectionNumber, bool appendmode )
{
    bool ret = TRUE;
    
    if( not appendmode ) this->clearAllPoints();
    
    QFile file( fileName );
    if ( file.open( IO_ReadOnly ) ) 
    {
        QTextStream stream( &file );
        QString line;
        int x = 0, y = 0;
        while ( !stream.atEnd() ) 
        {
            line = stream.readLine(); // line of text excluding '\n'
            
            if( line.at(0) == '#') continue ; 
            
            int r = sscanf(line.ascii(), "%d,%d", &x,&y);
            if(r != 2) 
            {
                break;
                ret = FALSE;
            }
            this->addPoint(x, y, sectionNumber); 
        }
        file.close();
    }
    else
    {    
        ret = FALSE;
    }
    return ret;
}
bool Plot::addPoint( int x, int y, int sectionNumber) 
{
    //~ qWarning( "%s:%d:msg in: %s(). ", __FILE__, __LINE__ ,__FUNCTION__  );
    bool ret = FALSE;
    
    if( sectionNumber<MAX_NOF_SECTIONS )    
    {
        ret = TRUE;
        
        iPoint *p = new iPoint;
        p->x = x ;
        p->y = y ;
        this->sections[sectionNumber].append(p);
    }
    return ret;
}

bool Plot::setPenWidth( int w, int sectionNumber ) 
{
    //~ qWarning( "%s:%d:%s(). ", __FILE__, __LINE__ ,__FUNCTION__  );
    bool ret = FALSE;
    if( sectionNumber<MAX_NOF_SECTIONS )    
    {
        ret = TRUE;
        this->sectionPen[sectionNumber] = QPen( this->sectionPen[sectionNumber].color(), w);
        //~ qWarning( "%s:%d:%s() x=%d, y[%d]=%d,.", __FILE__,__LINE__,__FUNCTION__, p->x, sectionNumber, p->y);
    }
    return ret;
}

bool Plot::setPenColor( const QColor & c, int sectionNumber ) 
{
    //~ qWarning( "%s:%d:%s(). ", __FILE__, __LINE__ ,__FUNCTION__  );
    bool ret = FALSE;
    if( sectionNumber<MAX_NOF_SECTIONS )    
    {
        ret = TRUE;
        this->sectionPen[sectionNumber].setColor(c);
        //~ qWarning( "%s:%d:%s() c=%s.", __FILE__,__LINE__,__FUNCTION__, c.name().latin1());
    }
    return ret;
}

void Plot::clearAllPoints()
{
    for(int i=0; i<MAX_NOF_SECTIONS; i++)
    {
        this->clearPoints(i);
    }
}

//~ slots: ##############################
void Plot::setCurrentSection(int s){this->currentSection = s;};
void Plot::setGridSteps(int m0, int m1){ this->gridSteps[0] = m0; this->gridSteps[1] = m1;};
void Plot::setGridPenWidth(int w) {this->gridPen.setWidth(w);};
void Plot::setGridPenColor(const QString & c) {this->gridPen.setColor(QColor(c));};
void Plot::setAppendmode(int m){this->appendmode = m;};
void Plot::addPoint(int x, int y)  {this->addPoint(x,y,this->currentSection);};
void Plot::setPenWidth(int w)      {this->setPenWidth(w,this->currentSection);};
void Plot::setPenColor(const QString & c)   {this->setPenColor(QColor(c),this->currentSection);};
void Plot::clearPoints()           {this->clearPoints(this->currentSection);};
void Plot::loadPoints(const QString & fileName)
{
    this->loadPoints(fileName, this->currentSection, this->appendmode);
};
