/****************************************************************************
** $Id: g_label.cpp
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

#include <qtimer.h>
#include <qpixmap.h>
#include <qguardedptr.h>

#include "g_label.h"

#define AUTO_REPEAT_DELAY  300
#define AUTO_REPEAT_PERIOD 100

//~ ##############################
G_Label::G_Label( QWidget* parent, const char * name)
    : QLabel(parent, name )
    , timer(NULL)
    , mlbDown(FALSE)
    , buttonDown(FALSE)
    , autorepeat(FALSE)
{
    qWarning( "%s. build: %s [%s]",__FILE__, __DATE__,__TIME__);

    this->timer = new QTimer() ;
    this->connect(this->timer, SIGNAL(timeout()), this, SLOT(autoRepeatTimeout()));
}

G_Label::~G_Label()
{
    //~  parents take care of destroying children
    if( this->timer != NULL )
    {
        delete this->timer ;
        this->timer = NULL ;
    }
}

//~ ##############################
bool G_Label::hitButton( const QPoint &pos ) const
{
    return this->rect().contains( pos );
}

//~ ##############################
void G_Label::mousePressEvent( QMouseEvent *e )
{
    qWarning( "%s:%d:%s()",__FILE__, __LINE__,__FUNCTION__);

    if ( this->hitButton( e->pos()  ) ) // mouse press on button
    {
        this->mlbDown = TRUE; // left mouse button down
        this->buttonDown = TRUE;

        this->repaint( FALSE );

        emit this->pressed();

        if ( this->timer && this->autorepeat ) this->timer->start( AUTO_REPEAT_DELAY, TRUE );
    }
}

void G_Label::mouseReleaseEvent( QMouseEvent *e)
{
    qWarning( "%s:%d:%s()",__FILE__, __LINE__,__FUNCTION__);

    if ( ! this->mlbDown ) return;

    if ( this->timer ) this->timer->stop();

    const bool oldButtonDown = this->buttonDown;
    this->mlbDown = FALSE;				// left mouse button up
    this->buttonDown = FALSE;

    if ( this->hitButton( e->pos() ) ) // mouse release on button
    {
        this->emit released();
        this->emit clicked();
    }
    else
    {
        this->repaint( FALSE );
        if (oldButtonDown) this->emit released();
    }
}



void G_Label::autoRepeatTimeout()
{
    qWarning( "%s:%d:%s()",__FILE__, __LINE__,__FUNCTION__);

    if ( this->mlbDown && this->autorepeat )
    {
        if ( this->buttonDown )
        {
            this->emit released();
            this->emit clicked();
            this->emit pressed();
        }

        this->timer->start( AUTO_REPEAT_PERIOD, TRUE );
    }
}



//~ SLOTS : ##############################
void G_Label::setWidgetErasePixmap( const QString & pixmapNameAndPath)
{
    QPixmap pix(pixmapNameAndPath);
    if( not pix.isNull() )
    {
        this->setErasePixmap(pix);
    }
}

void G_Label::setAutorepeat( int r )
{
    this->autorepeat = (bool) r;
}

//~ void G_Label::setBgPixmap( const QString &name  )
//~ {
    //~ this->;
//~ }

/*!
    Draws the label contents using the painter \a p.
*/

//~ void G_Label::drawContents( QPainter *p )
//~ {
    //~ QRect cr = contentsRect();

    //~ QPixmap *pix = pixmap();
    //~ QPicture *pic = picture();
    //~ const int mov = 0;

    //~ if ( !mov && !pix && !pic )
    //~ {
        //~ int m = indent();
        //~ if ( m < 0 && frameWidth() ) // no indent, but we do have a frame
            //~ m = fontMetrics().width('x') / 2 - margin();
        //~ if ( m > 0 )
        //~ {
            //~ int hAlign = QApplication::horizontalAlignment( align );
            //~ if ( hAlign & AlignLeft ) cr.setLeft( cr.left() + m );
            //~ if ( hAlign & AlignRight ) cr.setRight( cr.right() - m );
            //~ if ( align & AlignTop ) cr.setTop( cr.top() + m );
            //~ if ( align & AlignBottom ) cr.setBottom( cr.bottom() - m );
        //~ }
    //~ }

    //~ if ( doc )
    //~ {
        //~ doc->setWidth(p, cr.width() );
        //~ int rh = doc->height();
        //~ int yo = 0;
        //~ if ( align & AlignVCenter ) yo = (cr.height()-rh)/2;
        //~ else if ( align & AlignBottom ) yo = cr.height()-rh;
        //~ if (! isEnabled() && style().styleHint(QStyle::SH_EtchDisabledText, this))
        //~ {
            //~ QColorGroup cg = colorGroup();
            //~ cg.setColor( QColorGroup::Text, cg.light() );
            //~ doc->draw(p, cr.x()+1, cr.y()+yo+1, cr, cg, 0);
        //~ }

        //~ // QSimpleRichText always draws with QColorGroup::Text as with
        //~ // background mode PaletteBase. QLabel typically has
        //~ // background mode PaletteBackground, so we create a temporary
        //~ // color group with the text color adjusted.
        //~ QColorGroup cg = colorGroup();
        //~ if ( backgroundMode() != PaletteBase && isEnabled() ) cg.setColor( QColorGroup::Text, paletteForegroundColor() );

        //~ doc->draw(p, cr.x(), cr.y()+yo, cr, cg, 0);
    //~ }
    //~ else if ( pic )
    //~ {
        //~ QRect br = pic->boundingRect();
        //~ int rw = br.width();
        //~ int rh = br.height();
        //~ if ( scaledcontents )
        //~ {
            //~ p->save();
            //~ p->translate( cr.x(), cr.y() );
            //~ p->scale( (double)cr.width()/rw, (double)cr.height()/rh );
            //~ p->drawPicture( -br.x(), -br.y(), *pic );
            //~ p->restore();
        //~ }
        //~ else
        //~ {
            //~ int xo = 0;
            //~ int yo = 0;
            //~ if ( align & AlignVCenter )
            //~ yo = (cr.height()-rh)/2;
            //~ else if ( align & AlignBottom )
            //~ yo = cr.height()-rh;
            //~ if ( align & AlignRight )
            //~ xo = cr.width()-rw;
            //~ else if ( align & AlignHCenter )
            //~ xo = (cr.width()-rw)/2;
            //~ p->drawPicture( cr.x()+xo-br.x(), cr.y()+yo-br.y(), *pic );
        //~ }
    //~ }
    //~ else
    //~ {
        //~ if ( scaledcontents && pix )
        //~ {
            //~ if ( !d->img )
            //~ d->img = new QImage( lpixmap->convertToImage() );

            //~ if ( !d->pix )
            //~ d->pix = new QPixmap;
            //~ if ( d->pix->size() != cr.size() )
            //~ d->pix->convertFromImage( d->img->smoothScale( cr.width(), cr.height() ) );
            //~ pix = d->pix;
        //~ }
        //~ int alignment = align;
        //~ if ((align & ShowPrefix) && !style().styleHint(QStyle::SH_UnderlineAccelerator, this)) alignment |= NoAccel;
        //~ // ordinary text or pixmap label
        //~ style().drawItem( p, cr, alignment, colorGroup(), isEnabled(), pix, ltext );
        //~ if (pix != NULL && ltext != NULL)  style().drawItem( p, cr, alignment, colorGroup(), isEnabled(), NULL, ltext );
    //~ }
//~ }



