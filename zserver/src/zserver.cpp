/****************************************************************************
** file:    zserver.cpp
** project: zserver
** author:  giovanniangeli@tbe.it
** created at: mar set 18 12:14:00 2007;
** license: GPL

** Copyright (C) 2007-2009  giovanni angeli www.tbe.it

** This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    If you have not received a copy of the GNU General Public License
    along with this program, see <http://www.gnu.org/licenses/>.

****************************************************************************/
//~ #include <Python.h>
#include <qdict.h>
#include <qfile.h>
#include <qimage.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdeepcopy.h>
#include <qurl.h>
#include <qtextbrowser.h>
#include <qtable.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <qbutton.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qobjectlist.h>
#include <qmetaobject.h>
#include <qsettings.h>
#include <qdir.h>
#include <qapplication.h>
#include <qtextcodec.h>
#include <qfontdatabase.h>
#include <qcursor.h>
#include <qbitmap.h>
#include <qpixmap.h>

#include <qstylefactory.h>
#include <qwindowsstyle.h>
#include <qmotifstyle.h>
#include <qcdestyle.h>
#include <qmotifplusstyle.h>
#include <qsgistyle.h>
#include <qplatinumstyle.h>
#include <polymer.h>

#include "zserver.h"
#include "mysockets.h"
#include "mywidgetfactory.h"

static void dumpRecursive(int level, QObject *object)
{
    if(object != NULL)
    {
        QString buf;
        buf.fill( '\t', level/2 );
        if ( level % 2 ) buf += "  ";
        const char *name = object->name();
        QString flags="";
        if ( object->isWidgetType() )
        {
            QWidget * w = (QWidget *)object;
            if ( w->isVisible() )
            {
                QString t( "<%1,%2,%3,%4>" );
                flags += t.arg(w->x()).arg(w->y()).arg(w->width()).arg(w->height());
            }
            else
            {
                flags += 'I';
            }
        }
        //~ qDebug( "%s%s::%s %s", (const char*)buf, object->className(), name, flags.latin1() );
        qWarning( "%s%s::%s %s", (const char*)buf, object->className(), name, flags.latin1() );
        if ( object->children() )
        {
            QObjectListIt it(*object->children());
            QObject * c;
            while ( (c=it.current()) != 0 )
            {
                ++it;
                dumpRecursive( level+1, c );
            }
        }
    }
}

Z::Z(QWidget * parent, const char * name, WFlags fl, int SOCKET_PORT, int DBG_LEVEL)
    : Splash(parent, name, fl)
    ,dummy(NULL)
    ,socketPort(SOCKET_PORT)
    ,debugLevel(DBG_LEVEL)
    ,init_delay_msec(200)
    ,timeout_delay_msec(20000)
    ,timeout_cntr(0)
    ,LCDlamp_delay_sec(60)
    ,LCDlamp_status(OFF)
    ,serverSocket(NULL)
    ,timer(NULL)
    ,currentPressedObject(NULL)
    ,answer("")
    ,currentClient(NULL)
    ,frameToClient()
    ,imagesCollection()
    ,settedCodecName("")
{
    qWarning( "%s. build: %s [%s] size: (%d, %d)",__FILE__, __DATE__,__TIME__, this->width(), this->height());

    QFontDatabase fdb;
    QStringList families = fdb.families();
    MY_WARNING(0x02)("font.families:");
    for ( QStringList::Iterator f = families.begin(); f != families.end(); ++f )
    {
        QString family = *f;
        MY_WARNING(0x02)( "\t%s", family.latin1() );
        QStringList styles = fdb.styles( "%s", family.latin1() );
        for ( QStringList::Iterator s = styles.begin(); s != styles.end(); ++s )
        {
            QString style = *s;
            QString dstyle = "\t" + style + " (";
            QValueList<int> smoothies = fdb.smoothSizes( family, style );
            for ( QValueList<int>::Iterator points = smoothies.begin(); points != smoothies.end(); ++points )
            {
                dstyle += QString::number( *points ) + " ";
            }
            dstyle = dstyle.left( dstyle.length() - 1 ) + ")";
            MY_WARNING(0x02)( "\t%s", dstyle.latin1() );
        }
    }

    QFontInfo info( this->font() );
    QString family = info.family();
    MY_WARNING(0x02)("font.family=%s", family.latin1());

    this->imagesCollection.setAutoDelete(TRUE) ;

    this->timer = new QTimer() ;
    QTimer::singleShot( this->init_delay_msec, this, SLOT(initialize()) );

    this->serverSocket = new MyServer(this, this->socketPort) ;
    this->serverSocket->setName("zServerSocket") ;
    connect( this->serverSocket, SIGNAL(newConnect(MyClient*)), this, SLOT(newConnect(MyClient*)) );
    MY_WARNING(0x01)( "%s:%d:%s: connected server, port: %d", __FILE__,__LINE__,__FUNCTION__, this->socketPort);

    QWidgetFactory::addWidgetFactory( new MyWidgetFactory );

    { /* setExtensionType */
        QMimeSourceFactory::defaultFactory()->setExtensionType("", "text/plain;charset=UTF-8");
        //~ QMimeSourceFactory::defaultFactory()->setExtensionType("html", "text/html;charset=iso8859-1");
        //~ QMimeSourceFactory::defaultFactory()->setExtensionType("htm", "text/html;charset=iso8859-1");
        QMimeSourceFactory::defaultFactory()->setExtensionType("txt", "text/plain");
        QMimeSourceFactory::defaultFactory()->setExtensionType("py", "text/plain;charset=UTF-8");
        QMimeSourceFactory::defaultFactory()->setExtensionType("xml", "text/xml;charset=UTF-8");
        QMimeSourceFactory::defaultFactory()->setExtensionType("png", "image/png");
        QMimeSourceFactory::defaultFactory()->setExtensionType("jpg", "image/jpg");
        //~ QMimeSourceFactory::defaultFactory()->setFilePath( "./html" );
    }

    {
        QString startMsg("");
        QFile file( "zsplashmsg.htm" );
        if ( file.open( IO_ReadOnly ) ) {
            QTextStream stream( &file );
            while ( !stream.atEnd() )  startMsg += stream.readLine(); // line of text excluding '\n'
            file.close();
        }

        if(startMsg.length() == 0)
        {
            const QImage * im = new QImage("zsplashmsg.png");
            if( (im != NULL) and ( not im->isNull() ) )
            {
                QPixmap p( *im );
                this->logo->setPixmap(p);
            }
            else
            {
                //~ startMsg = "<p align=\"left\"><font color=\"#000099\" size=\"+2\">zserver</font></p>";
                //~ startMsg.sprintf("%s<p align=\"left\"><font color=\"#000000\" size=\"-1\">build: %s [%s]</font></p>",
                                    //~ startMsg.latin1(), __DATE__,__TIME__);
                //~ startMsg += "<p align=\"left\"><font color=\"#000099\">copyrigth (c) tbe - www.tbe.it</font></p>";

                //~ this->logo->setText(startMsg);
            }
        } else this->logo->setText(startMsg);
    }

    this->logo->installEventFilter( this );

    this->show() ;
}

Z::~Z()
{
    this->imagesCollection.clear();

    if( this->timer != NULL )
    {
        delete this->timer ;
        this->timer = NULL ;
    }
}

void Z::initialize(void)
{
    this->switch_LCDlamp(ON);

    this->connect( (this->timer), SIGNAL(timeout()), this, SLOT(timeout()) );
    this->timer->start(this->timeout_delay_msec, FALSE/* NOT SINGLE SHOT */);

}

void Z::timeout(void)
{
    MY_WARNING(0x01)("%s() [%s][%03d]", __FUNCTION__, QTime::currentTime().toString().latin1(), this->timeout_cntr%1000 );

    if( this->timeout_cntr * this->timeout_delay_msec > this->LCDlamp_delay_sec * 1000 )
    {
        this->switch_LCDlamp(OFF) ;
        this->timeout_cntr = 0 ;
    }

    this->timeout_cntr += 1 ;
}

void Z::finalize(void)
{
    //~ Py_Finalize();
}

//~ ########################
void Z::newConnect(MyClient * c)
{
    MY_WARNING(0x01)( "%s:%d:%s()", __FILE__,__LINE__,__FUNCTION__ );
    this->connect(c, SIGNAL(getLine( const QString& , MyClient *)), this, SLOT(handleSocketData( const QString& , MyClient *)) );
    this->connect(c, SIGNAL(closeSignal(MyClient*)), this, SLOT(closeClient(MyClient*)) );
}


void Z::closeClient(MyClient * c)
{
    QMap<QWidget*, MyClient*>::Iterator it ;

    QPtrList<QWidget> framesToDelete;

    for ( it = this->frameToClient.begin(); it != this->frameToClient.end(); ++it )
    {
        if( c == it.data() ) framesToDelete.append( it.key() ) ;
    }

    dumpRecursive( 0, this );

    QWidget * w;
    for ( w = framesToDelete.first(); w; w = framesToDelete.next() )
    {
        if(w != NULL)
        {
            //~ MY_WARNING(0x01)( "%s:%d:%s() removing: %s from frameToClient map ...", __FILE__,__LINE__,__FUNCTION__ , w->name());
            delete(w);
            this->frameToClient.remove(w);
            MY_WARNING(0x01)( "%s:%d:%s() %s removed.", __FILE__,__LINE__,__FUNCTION__ , w->name());
        }else qWarning("%s:%d:%s() ALERT: trying to delete %p!", __FILE__,__LINE__,__FUNCTION__ , w);
    }

    dumpRecursive( 0, this );

    //~ MY_WARNING(0x01)( "%s:%d:%s()", __FILE__,__LINE__,__FUNCTION__ );
}

void Z::handleSocketData( const QString& msg, MyClient * client)
{
    static unsigned int s_count = 0;
    QString errMsg("");
    this->answer = "";

    this->currentClient = client ;

    QString object_id;
    QString method(QString::null);
    QTextIStream ts( &(msg) );
    ts >> object_id  >> method;

    MY_WARNING(0x04)( "%s:%d:%s(%s) s_count=%d, object_id=%s, method=%s.",
        __FILE__,__LINE__,__FUNCTION__, msg.latin1(), s_count++, object_id.latin1(), method.latin1());

    QObject * o = this->findObject(object_id);
    if(o == NULL)
    {
        errMsg.sprintf("<%s> object not found", object_id.latin1());
        MY_WARNING(0x01)( "%s:%d:%s() errMsg=%s.", __FILE__,__LINE__,__FUNCTION__,errMsg.latin1());
        return;
    }
    else
    {

        if( not this->searchInMetaObjectSpecs( o, method ) )
        {/* let's search in object's properties and slots */
            QStringList args;
            while(1) /* tokenize args */
            {
                QString tmp(QString::null);
                ts >> tmp ;
                QUrl::decode ( tmp );
                if( not tmp.isEmpty() ) args.append( tmp );
                else break;
                MY_WARNING(0x02)( "%s:%d:%s() tmp=%s.", __FILE__,__LINE__,__FUNCTION__, tmp.ascii());
            }

            bool isProperty = FALSE ;
            QStrList l = o->metaObject()->propertyNames(TRUE);
            if( l.find(method) != -1 )/* try to handle as property */
            {
                if(args.count() == 0) isProperty = this->getProperty( o , method );
                else isProperty = this->setProperty( o , method, args );
            }
            if( not isProperty ) /* try to handle as slot */
            {
                bool ok = this->callSlot( o , method, args );
                if(! ok)
                {
                    if((method == "setPixmap") and o->inherits("QTable") and (args.count() == 3)) /* last chance */
                    {
                        int row = args[0].toInt();
                        int col = args[1].toInt();
                        QImage * im = this->imagesCollection[ args[2] ] ;
                        if( (im != NULL) and ( not im->isNull() ) )
                        {
                            QPixmap pix( *im );
                            ((QTable*)o)->setPixmap(row,col,pix);
                            if(this->debugLevel & 1 ) qWarning( "%s:%d:%s() %s.setPixmap(%s,%s,%s).", __FILE__,__LINE__,__FUNCTION__,
                                o->name(),args[0].latin1(),args[1].latin1(),args[2].latin1());
                        }
                        else qWarning( "%s:%d:%s() image <%s> not found.", __FILE__,__LINE__,__FUNCTION__, args[0].latin1());
                    }
                    else if((method == "setText") and o->inherits("QTable") and (args.count() == 3)) /* very last chance */
                    {
                        int row = args[0].toInt();
                        int col = args[1].toInt();
                        ((QTable*)o)->setText(row,col,args[2]);
                    }
                    else
                    {
                        errMsg.sprintf("method not found");
                        MY_WARNING(0x01)( "%s:%d:%s() errMsg=%s.", __FILE__,__LINE__,__FUNCTION__,errMsg.latin1());
                    }
                }
            }
        }
    }

    if(errMsg.length() == 0)
    {
        if(this->answer.length() == 0)
        {
            this->answer.sprintf("z.handleServerAnswer([\"%s\",\"%s\",\"%s\",]);", o->name(), method.latin1(), "OK:" );
        }
        if(this->debugLevel & 0x1000 ) this->sendMessageToTheSocket(this->answer, this->currentClient);
    }
    else
    {
        this->answer.sprintf("z.handleServerAnswer([\"%s\",\"%s\",\"%s\",]);", o->name(), method.latin1(), ("ERR:"+errMsg).latin1() );
        this->sendMessageToTheSocket(this->answer, this->currentClient);
    }
    MY_WARNING(0x01)( "%s:%d:%s() <%s>.", __FILE__,__LINE__,__FUNCTION__, this->answer.latin1());
}

void Z::sendMessageToTheSocket(const char * msg, MyClient * client)
{
    QString s(msg);
    this->sendMessageToTheSocket( s, client );
}

void Z::sendMessageToTheSocket(const QString& msg, MyClient * client)
{
    MY_WARNING(0x08)( "%s:%d:%s(%s)[%d]",__FILE__,__LINE__,__FUNCTION__, msg.latin1(), msg.length() );
    if(client == NULL) this->serverSocket->sendToClients(msg);
    else this->serverSocket->sendToClient(msg, client);
}

//~ ########################
void Z:: createWidget(const QString& args)
{
    QString errMsg(QString::null);
    QString name;
    QString className;
    QString parent_name ;

    int n = args.contains(",");
    if( n == 2)
    {
        name = args.section(",",0,0);
        className = args.section(",",1,1);
        parent_name = args.section(",",2,2);
    }
    else
    {
        QString errMsg = "ERR: invalid args format <"+args+">" ;
        this->answer.sprintf("z.handleServerAnswer([\"\",\"\",\"%s\",]);", errMsg.latin1());
        MY_WARNING(0x02)( "%s:%d:%s() %s.", __FILE__,__LINE__,__FUNCTION__, this->answer.latin1() );
        return;
    }

    QObject * parent = findObject(parent_name);
    if(parent == NULL)
    {
        QString errMsg = "ERR: "+parent_name+" not found" ;
        this->answer.sprintf("z.handleServerAnswer([\"\",\"\",\"%s\",]);", errMsg.latin1());
        MY_WARNING(0x01)( "%s:%d:%s() %s.", __FILE__,__LINE__,__FUNCTION__, this->answer.latin1() );
        return;
    }

    QWidgetFactory wf;
    QWidget * w = wf.createWidget ( className, (QWidget *) parent, name );
    if(w == NULL)
    {
        QString errMsg = "ERR: unable to create: "+className+"."+name ;
        this->answer.sprintf("z.handleServerAnswer([\"\",\"\",\"%s\",]);", errMsg.latin1());
        MY_WARNING(0x01)( "%s:%d:%s() %s.", __FILE__,__LINE__,__FUNCTION__, this->answer.latin1() );
        return;
    }
    else
    {
        //~ QObjectList l;
        //~ QWidget * p = w ;
        //~ l.append(p);
        //~ this->iterateObjects(&l);

        QObjectList *l = w->queryList( "QObject" );
        l->append(w);
        this->iterateObjects(l);
        delete l; // delete the list, not the objects

    }
    qWarning( "%s:%d:%s() created widget=<%s>", __FILE__,__LINE__,__FUNCTION__, name.latin1() );
    this->answer = ";";
}

void Z::iterateObjects( QObjectList *l )
{// iterate over all the objects to install the eventfilter and connect slots
    QObjectListIt it( *l );
    QObject *obj;
    QString tmp("");
    while ( (obj = it.current()) != 0 )
    {
        MY_WARNING(0x01)( "%s:%d:%s() obj->name=%s", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        ++it;
        obj->installEventFilter( this );
        QMetaObject * mo = obj->metaObject();
        QStrList sl = mo->signalNames( TRUE );
        if(sl.find("clicked()") != -1)
        {
            this->connect(obj, SIGNAL(clicked()),this,SLOT(clicked()));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.clicked()>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(sl.find("clicked(int)") != -1)
        {
            this->connect(obj, SIGNAL(clicked(int)),this,SLOT(clicked(int)));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.clicked(int)>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(sl.find("clicked(int,int)") != -1)
        {
            this->connect(obj, SIGNAL(clicked(int,int)),this,SLOT(clicked(int,int)));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.clicked(int,int)>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(sl.find("clicked(int,int,int,const QPoint&)") != -1)
        {
            this->connect(obj, SIGNAL(clicked(int,int,int,const QPoint&)),this,SLOT(clicked(int,int,int,const QPoint&)));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.clicked(int,int,int,const QPoint&)>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(sl.find("linkClicked(const QString&)") != -1)
        {
            this->connect(obj, SIGNAL(linkClicked(const QString&)),this,SLOT(linkClicked(const QString&)));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.linkClicked(const QString&)>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(sl.find("anchorClicked(const QString&,const QString&)") != -1)
        {
            this->connect(obj, SIGNAL(anchorClicked(const QString&,const QString&)),this,SLOT(anchorClicked(const QString&,const QString&)));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.anchorClicked(const QString&,const QString&)>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(sl.find("selectionChanged(QListBoxItem*)") != -1)
        {
            this->connect(obj, SIGNAL(selectionChanged(QListBoxItem*)),this,SLOT(selectionChanged(QListBoxItem*)));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.selectionChanged(QListBoxItem*)>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(sl.find("valueChanged(int)") != -1)
        {
            this->connect(obj, SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.valueChanged(int)>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(sl.find("valueChanged(const QString&)") != -1)
        {
            this->connect(obj, SIGNAL(valueChanged(const QString&)),this,SLOT(valueChanged(const QString&)));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.valueChanged(const QString&)>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(sl.find("valueChanged(const QTime&)") != -1)
        {
            this->connect(obj, SIGNAL(valueChanged(const QTime&)),this,SLOT(valueChanged(const QTime&)));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.valueChanged(const QTime&)>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(sl.find("valueChanged(const QDate&)") != -1)
        {
            this->connect(obj, SIGNAL(valueChanged(const QDate&)),this,SLOT(valueChanged(const QDate&)));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.valueChanged(const QDate&)>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(sl.find("textChanged(const QString&)") != -1)
        {
            this->connect(obj, SIGNAL(textChanged(const QString&)),this,SLOT(textChanged(const QString&)));
            MY_WARNING(0x02)( "%s:%d:%s() connected <%s.textChanged(const QString&)>", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        if(obj->inherits("QTextBrowser"))
        {
            ((QTextBrowser*)obj)->setMimeSourceFactory( QMimeSourceFactory::defaultFactory() ) ;
            MY_WARNING(0x02)( "%s:%d:%s() setted %s.mimeSourceFactory.", __FILE__,__LINE__,__FUNCTION__, obj->name() );
        }
        tmp.sprintf("\"%s\":0x%08X, ", obj->name(), (unsigned int)obj );
    }
}

void Z::buildFromUiFile(const QString& uiNameAndPath)
{
    QWidget * w = NULL ;
    QString errMsg(QString::null);
    //~ this->answer = "{ \"0xFFFFFFFF\": \"OK\", \"widgetList\":{";

    QDir dir("./");
    MY_WARNING(0x01)( "%s:%d:%s() dir.canonicalPath()=%s", __FILE__,__LINE__,__FUNCTION__, dir.canonicalPath().latin1() );

    if( QFile::exists(uiNameAndPath))
    {
        if( not uiNameAndPath.isEmpty() )
        {
            QString name = uiNameAndPath.section(".",0,-2).section("/",-1,-1);

            w = QWidgetFactory::create ( uiNameAndPath, this, this, name ) ;
            qWarning( "%s:%d:%s() w=<%p>", __FILE__,__LINE__,__FUNCTION__, w);
            if(w == NULL)
            {
                //~ QString buff("");
                QFile file( uiNameAndPath );
                QString buffer("");
                if ( file.open( IO_ReadOnly ) ) {
                    QTextStream stream( &file );
                    while ( !stream.atEnd() )  buffer += stream.readLine(); // line of text excluding '\n'
                    file.close();
                }
                buffer = buffer.simplifyWhiteSpace();
                if ( file.open( IO_WriteOnly ) ) {
                    file.writeBlock(buffer.latin1(), buffer.length());
                    file.close();
                }
                w = QWidgetFactory::create ( uiNameAndPath , this, this, name ) ;
                qWarning( "%s:%d:%s() w=<%p>", __FILE__,__LINE__,__FUNCTION__, w);
            }

            if(w != NULL)
            {
                MY_ASSERT(this->currentClient != NULL);

                this->frameToClient.insert(w, this->currentClient);
                w->reparent( this , WStyle_Customize, QPoint(0,0), FALSE);
                QObjectList *l = w->queryList( "QObject" );
                this->iterateObjects(l);
                delete l; // delete the list, not the objects

                this->currentPressedObject = w; /* init to a safe (not NULL QWidget) value */

                qWarning( "%s:%d:%s() created frame=<%s>", __FILE__,__LINE__,__FUNCTION__, name.ascii() );
            }
            else
            {
                errMsg.sprintf( "%s:%d:%s() w==NULL, name=%s, uiNameAndPath=%s.",
                        __FILE__,__LINE__, __FUNCTION__, name.ascii(), uiNameAndPath.ascii() );
            }
        }
        else errMsg.sprintf( "%s:%d:%s(%s) uiNameAndPath.isEmpty.", __FILE__,__LINE__, __FUNCTION__, uiNameAndPath.latin1());
    }
    else
    {
        errMsg.sprintf( "%s:%d:%s() not found=%s.",
                __FILE__,__LINE__, __FUNCTION__, uiNameAndPath.ascii() );
    }

    if(not errMsg.isEmpty())
    {
        //~ qWarning(errMsg.latin1());
        perror(errMsg.ascii());
        //~ this->answer = "{ \"0xFFFFFFFF\": \"ERROR\", \"errMsg\":\"" + errMsg + "\" }" ;
        //~ this->answer = "raise '0xFFFFFFFF ERROR:" + errMsg + "';" ;
        //~ this->answer.sprintf("z.raiseExc('ERROR: %s');", errMsg.latin1());
        this->answer.sprintf("z.handleServerAnswer([\"\",\"\",\"%s\",]);", ("ERR:"+errMsg).latin1());
    }
    //~ else this->answer += "}, };";
    else this->answer = ";";
}

bool Z::searchInMetaObjectSpecs(QObject * o , const QString & method )
{
    QStrList l ;
    bool flag = FALSE ;

    if(method       == "propertyNames" )    {l = o->metaObject()->propertyNames()    ; flag = TRUE ;}
    else if(method  == "slotNames"     )    {l = o->metaObject()->slotNames    ()    ; flag = TRUE ;}
    else if(method  == "signalNames"   )    {l = o->metaObject()->signalNames  ()    ; flag = TRUE ;}
    else if(method  == "propertyNamesAll" ) {l = o->metaObject()->propertyNames(TRUE); flag = TRUE ;}
    else if(method  == "slotNamesAll"     ) {l = o->metaObject()->slotNames    (TRUE); flag = TRUE ;}
    else if(method  == "signalNamesAll"   ) {l = o->metaObject()->signalNames  (TRUE); flag = TRUE ;}

    QString tmp = "";
    if( flag ) /* explicitely handled, build up the answer and go out */
    {
        tmp = "[";
        for(unsigned int i=0; i<l.count(); i++)
        {
            QString s = l.at(i);
            QUrl::encode ( s );
            tmp += "'" + s + "'," ;
        }
        tmp += "]" ;
    }
    this->answer.sprintf("z.handleServerAnswer([\"%s\",\"%s\",\"%s\",]);", o->name(), method.latin1(), ("OK:"+tmp).latin1());
    return flag ;
}

bool Z::setProperty(QObject * o , const QString & method, const QStringList & args )
{
    bool ok = FALSE ;
    unsigned int object_id ;
    if(o == (QObject *) this) object_id = 0xFFFFFFFF;
    else object_id = (unsigned int) o;

    QVariant q_var ;
    if((( method == "pixmap" ) || ( method == "paletteBackgroundPixmap" )) || ( method == "setWidgetErasePixmap" ))
    {   /* cannot cast automatically */
        QImage * im = this->imagesCollection[ args[0] ] ;
        if( (im != NULL) and ( not im->isNull() ) )
        {
            QPixmap pix( *im );
            q_var = QVariant(pix);
        }
        else
        {
            QPixmap pix(args[0]);/*  try to load image from file */
            if( not pix.isNull() )
            {
                q_var = QVariant(pix);
                if(args.count() == 2)
                {
                    if( args[1].toInt() )
                    {
                        pix.setMask( pix.createHeuristicMask() );
                        MY_WARNING(0x01)("%s:%d:%s() heuristicMask created for <%s>.", __FILE__,__LINE__,__FUNCTION__, args[0].latin1());
                    }
                }
            }
            else qWarning( "%s:%d:%s() image <%s> not found.", __FILE__,__LINE__,__FUNCTION__, args[0].latin1());
        }
    }
    else
    {
        //~ use codec for text */
        if( method == "text" )
        {
            QTextCodec* codec = QTextCodec::codecForName( this->settedCodecName.ascii() );
            if( codec  != NULL )
            {
                q_var = QVariant( codec->fromUnicode( args[0] ) );
            }
            else //~ propVal = QVariant( args[0] ); /* try to cast automatically */
            {
                q_var = QVariant( args[0] );
            }
        }
        else
        {
            q_var = QVariant( args[0] );
        }
    }

    ok = o->setProperty( method.latin1(), q_var );

    if(ok) MY_WARNING(0x04)( "%s:%d:%s() succesfully setted: <%s.%s=%s> (%s)",__FILE__,__LINE__,__FUNCTION__,
                    o->name(), method.latin1(), args[0].latin1(), this->settedCodecName.latin1());

    return ok ;
}

bool Z::getProperty(QObject * o , const QString & method )
{
    bool ok = FALSE ;
    unsigned int object_id ;
    if(o == (QObject *) this) object_id = 0xFFFFFFFF;
    else object_id = (unsigned int) o;

    QString propValue = "";

    QVariant prop = o->property( method.latin1() );
    ok = prop.isValid() ;
    if( ok )
    {
        MY_WARNING(0x04)( "%s:%d:%s() reading prop: <%s.%s>",__FILE__,__LINE__,__FUNCTION__, o->name(), method.latin1() );
        if( prop.canCast(QVariant::String) )
        {
            propValue = prop.toString();
            QUrl::encode ( propValue );
        }
        else if( prop.canCast(QVariant::StringList) )
        {
            QStringList pl = prop.toStringList();
            propValue = "[";
            for(unsigned int i=0; i<pl.count(); i++)
            {
                QString tmp = pl[i];
                QUrl::encode ( tmp );
                propValue += tmp + "," ;
            }
            propValue += "]" ;
        }
        else
        {
            ok = FALSE ;
            qWarning( "%s:%d:%s() cant cast %s.%s",__FILE__,__LINE__,__FUNCTION__, o->name(), method.latin1() );
        }
    }
    if(ok)
    {
        this->answer.sprintf("z.handleServerAnswer([\"%s\",\"%s\",\"%s\",]);", o->name(), method.latin1(), ("OK:"+propValue).latin1());
        MY_WARNING(0x04)( "%s:%d:%s() succesfully got: <%s.%s> %s",__FILE__,__LINE__,__FUNCTION__,
                                    o->name(), method.latin1(), this->answer.latin1() );
    }
    return ok ;
}

bool Z::callSlot(QObject * o , const QString & method, const QStringList & args )
{
    bool ok = FALSE ;
    QStrList sl = o->metaObject()->slotNames(TRUE);
    if( (o->inherits("QTable")) and ((method == "setHeaderLabel") and (args.count() >= 2)) )
    {   /* sinopsys: setHeaderLabel(text, section, direction="h"); [slot]*/
        QHeader * h = ((QTable *) o)->horizontalHeader() ;
        QString text = args[0];
        int section = args[1].toInt();
        if(args.count() == 3)
        {
            if(args[2]=="v") h = ((QTable *) o)->verticalHeader() ;
        }
        if(h != NULL)
        {
            h->setLabel(section, text);
            ok = 1;
        }
    }
    else if(args.count() == 0)
    {
        this->disconnect(SIGNAL(generic()));
        ok = this->connect(this, SIGNAL(generic()), o, ("1"+method+"()").ascii() );
        if(ok) this->emit generic();
    }
    else if(args.count() == 1)
    {
        this->disconnect(SIGNAL(generic(const QString&)));
        ok = this->connect(this, SIGNAL(generic(const QString&)), o, ("1"+method+"(const QString&)").ascii() );
        if(ok) this->emit generic( args[0] );
        else
        {
            this->disconnect(SIGNAL(generic(int)));
            ok = this->connect(this, SIGNAL(generic(int)), o, ("1"+method+"(int)").ascii() );
            if(ok) this->emit generic( args[0].toInt() );
        }
    }
    else if(args.count() == 2)
    {
        this->disconnect(SIGNAL(generic(int,int)));
        ok = this->connect(this, SIGNAL(generic(int,int)), o, ("1"+method+"(int,int)").ascii() );
        if(ok) this->emit generic( args[0].toInt(), args[1].toInt() );
        else
        {
            this->disconnect(SIGNAL(generic(const QString&,const QString&)));
            ok = this->connect(this, SIGNAL(generic(const QString&,const QString&)), o,
                ("1"+method+"(const QString&,const QString&)").ascii() );
            if(ok) this->emit generic( args[0], args[1] );
        }
    }

    return ok ;
}

bool Z::eventFilter( QObject *o, QEvent *e )
{
    //~ MY_WARNING(0x01)( "%s:%d:%s() event from: %s, of type: %d", __FILE__,__LINE__,__FUNCTION__, o->name(), e->type() );
    //~ enum Type { None = 0, Timer = 1, MouseButtonPress = 2, MouseButtonRelease = 3, MouseButtonDblClick = 4,
        //~ MouseMove = 5, KeyPress = 6, KeyRelease = 7, FocusIn = 8, FocusOut = 9, Enter = 10, Leave = 11,
        //~ Paint = 12, Move = 13, Resize = 14, Create = 15, Destroy = 16, Show = 17, Hide = 18, Close = 19,
        //~ Quit = 20, Reparent = 21, ShowMinimized = 22, ShowNormal = 23, WindowActivate = 24, WindowDeactivate = 25,
        //~ ShowToParent = 26, HideToParent = 27, ShowMaximized = 28, ShowFullScreen = 29, Accel = 30, Wheel = 31,
        //~ AccelAvailable = 32, CaptionChange = 33, IconChange = 34, ParentFontChange = 35,
        //~ ApplicationFontChange = 36, ParentPaletteChange = 37, ApplicationPaletteChange = 38,
        //~ PaletteChange = 39, Clipboard = 40, Speech = 42, SockAct = 50, AccelOverride = 51,
        //~ DeferredDelete = 52, DragEnter = 60, DragMove = 61, DragLeave = 62, Drop = 63, DragResponse = 64,
        //~ ChildInserted = 70, ChildRemoved = 71, LayoutHint = 72, ShowWindowRequest = 73, WindowBlocked = 74,
        //~ WindowUnblocked = 75, ActivateControl = 80, DeactivateControl = 81, ContextMenu = 82, IMStart = 83,
        //~ IMCompose = 84, IMEnd = 85, Accessibility = 86, TabletMove = 87, LocaleChange = 88, LanguageChange = 89,
        //~ LayoutDirectionChange = 90, Style = 91, TabletPress = 92, TabletRelease = 93, OkRequest = 94, HelpRequest = 95,
        //~ WindowStateChange = 96, IconDrag = 97, User = 1000, MaxUser = 65535 }

    //~ if( e->type() == QEvent::MouseButtonRelease ) this->currentPressedObject = o;
    if( e->type() == QEvent::MouseButtonPress )
    {
        this->timeout_cntr = 0 ;

        if( this->LCDlamp_status == OFF )
        {
            this->switch_LCDlamp(ON) ;
            this->currentPressedObject = NULL;
            return TRUE;
        }

        this->currentPressedObject = o;
    }
    //~ if( e->type() == QEvent::MouseButtonPress ) this->currentPressedObject = o;

    return FALSE;
}

MyClient* Z::getClient(QObject * pFrame)
{
    MyClient* c = NULL;
    QMap<QWidget*, MyClient*>::Iterator it = this->frameToClient.find((QWidget*)pFrame) ;
    MY_ASSERT(it != this->frameToClient.end());
    c = it.data() ;
    return c;
}

QObject * Z::getParentFrame(QObject * c)
{
    QObject * p = c;
    while(1)
    {
        if(p == NULL) break;
        if(p->parent() == this) break;
        p = p->parent();
    }
    return p ;
}

QObject * Z::findObject(const QString & object_id)
{
    QObject * o = NULL;

    if((object_id == "0xFFFFFFFF") or (object_id == "z")) o = (QObject *) this;
    else if( object_id.contains(".") == 0 )
    {
        o = this->child(object_id, "QWidget");
    }
    else if( object_id.contains(".") == 1 )
    {
        QString frameName = object_id.section(".",0,0);
        QString childName = object_id.section(".",1,1);
        QObject * frame = this->child(frameName, "QWidget");
        if(frame != NULL) o = frame->child(childName, "QWidget");
    }
    return o;
}

//~ ########################
void Z::clicked()
{
    //~ MY_WARNING(0x01)( "%s:%d:%s() %s.%s().", __FILE__,__LINE__,__FUNCTION__,
        //~ this->currentPressedObject->parent()->name(), this->currentPressedObject->name());
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( this->currentPressedObject ) ;
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;} /* MY_ASSERT(pFrame !=NULL); */
    msg.sprintf( "z.frames[\"%s\"].clicked(\"%s\");", pFrame->name(), this->currentPressedObject->name() );
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

void Z::clicked(int section) /* QHeader, QButtonGroup */
{
    MY_WARNING(0x02)( "%s:%d:%s() %s.%s(%d).", __FILE__,__LINE__,__FUNCTION__,
        this->currentPressedObject->parent()->name(), this->currentPressedObject->name(),section);
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( this->currentPressedObject ) ;
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;} /* MY_ASSERT(pFrame !=NULL); */
    msg.sprintf( "z.frames[\"%s\"].clicked([\"%s\",\"%d\"]);",
            pFrame->name(), this->currentPressedObject->name(),section );
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

void Z::clicked(int para, int pos) /* QTextEdit::clicked ( int para, int pos ) */
{
    MY_WARNING(0x02)( "%s:%d:%s() %s.%s(%d,%d).", __FILE__,__LINE__,__FUNCTION__,
        this->currentPressedObject->parent()->name(), this->currentPressedObject->name(),para,pos);
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( this->currentPressedObject ) ;
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;} /* MY_ASSERT(pFrame !=NULL); */
    msg.sprintf( "z.frames[\"%s\"].clicked([\"%s\",\"%d\",\"%d\"]);",
            pFrame->name(), this->currentPressedObject->name(),para,pos);
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

void Z::clicked(int row, int col, int button, const QPoint & mousePos ) /* QTable */
{
    if(0) /* to avoid compiler warnings about unused args, in effect we dont need theese */
    {
        button = 0 ;
        int a = mousePos.x() ;
        a = 0;
    }
    MY_WARNING(0x02)( "%s:%d:%s() %s.%s(%d,%d).", __FILE__,__LINE__,__FUNCTION__,
        this->currentPressedObject->parent()->name(), this->currentPressedObject->name(),row,col);
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( this->currentPressedObject ) ;
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;} /* MY_ASSERT(pFrame !=NULL); */
    msg.sprintf( "z.frames[\"%s\"].clicked([\"%s\",\"%d\",\"%d\"]);",
            pFrame->name(), this->currentPressedObject->name(),row,col );
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

void Z::valueChanged(int v)
{
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( this->currentPressedObject ) ;
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;} /* MY_ASSERT(pFrame !=NULL); */
    msg.sprintf( "z.frames[\"%s\"].valueChanged(\"%s\",\"%d\");",
                    pFrame->name(), this->currentPressedObject->name(), v );
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

void Z::valueChanged(const QTime& t)
{
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( this->currentPressedObject ) ;
    //~ MY_ASSERT(pFrame !=NULL);
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;}
    msg.sprintf( "z.frames[\"%s\"].valueChanged(\"%s\",\"%s\");",
                    pFrame->name(), this->currentPressedObject->name(), t.toString().latin1() );
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

void Z::valueChanged(const QDate& d)
{
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( this->currentPressedObject ) ;
    //~ MY_ASSERT(pFrame !=NULL);
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;}
    msg.sprintf( "z.frames[\"%s\"].valueChanged(\"%s\",\"%s\");",
                    pFrame->name(), this->currentPressedObject->name(), d.toString().latin1() );
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

void Z::valueChanged(const QString& v)
{
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( this->currentPressedObject ) ;
    //~ MY_ASSERT(pFrame !=NULL);
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;}
    msg.sprintf( "z.frames[\"%s\"].valueChanged(\"%s\",\"%s\");",
                    pFrame->name(), this->currentPressedObject->name(), v.latin1() );
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

void Z::textChanged(const QString& v)
{
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( this->currentPressedObject ) ;
    //~ MY_ASSERT(pFrame !=NULL);
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;}
    msg.sprintf( "z.frames[\"%s\"].textChanged(\"%s\",\"%s\");",
                    pFrame->name(), this->currentPressedObject->name(), v.latin1() );
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

void Z::selectionChanged(QListBoxItem*i)
{
    //~ MY_WARNING(0x01)( "%s:%d:%s(%s) .", __FILE__,__LINE__,__FUNCTION__, i->text().latin1());
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( (QObject *)(i->listBox()) ) ;
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;} /* MY_ASSERT(pFrame !=NULL); */
    msg.sprintf( "z.frames[\"%s\"].selectionChanged(\"%s\",\"%s\");",
                    pFrame->name(), i->listBox()->name(), i->text().latin1() );
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

void Z::linkClicked(const QString& link)
{
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( this->currentPressedObject ) ;
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;} /* MY_ASSERT(pFrame !=NULL); */
    msg.sprintf( "z.frames[\"%s\"].linkClicked(\"%s\",\"%s\");",
        pFrame->name(), this->currentPressedObject->parent()->name(), link.latin1() );
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

void Z::anchorClicked ( const QString & name, const QString & link )
{
    //~ MY_WARNING(0x01)( "%s:%d:%s(%s) .", __FILE__,__LINE__,__FUNCTION__, link.latin1());
    QString msg("") ;
    QObject * pFrame = this->getParentFrame( this->currentPressedObject ) ;
    if(pFrame == NULL){ qWarning("%s:%d:%s() pFrame == NULL.", __FILE__,__LINE__,__FUNCTION__); return ;} /* MY_ASSERT(pFrame !=NULL); */
    //~ msg.sprintf( "z.frames[\"%s\"].anchorClicked(\"%s\",\"%s\");",
        //~ pFrame->name(), this->currentPressedObject->name(), link.latin1() );
    msg.sprintf( "z.frames[\"%s\"].anchorClicked(\"%s\",\"%s\",\"%s\");",
        pFrame->name(), this->currentPressedObject->parent()->name(), name.latin1(), link.latin1() );
    this->sendMessageToTheSocket(msg, this->getClient(pFrame));
}

//~ ########################
void Z::appendImage(const QString & imgFileNameAndPath)
{
    this->answer = "";
    if(this->debugLevel & 0x1000 ) this->answer = "{'0xFFFFFFFF': 'OK' };";
    const QImage * im = new QImage(imgFileNameAndPath);

    QFileInfo fi(imgFileNameAndPath);
    QString key = fi.baseName( TRUE );
    if( not im->isNull() ) this->imagesCollection.insert(key, im);
    else
    {
        QString errMsg = "ERR: cant load image from file:<"+imgFileNameAndPath+">" ;
        this->answer.sprintf("z.handleServerAnswer([\"\",\"\",\"%s\",]);", errMsg.latin1());
        MY_WARNING(0x03)( "%s:%d:%s() <%s>.", __FILE__,__LINE__,__FUNCTION__, this->answer.latin1());
    }
};

void Z::removeImage(const QString & key)
{
    this->answer = "";
    if(this->debugLevel & 0x1000 ) this->answer = "{'0xFFFFFFFF': 'OK' };";
    if( not this->imagesCollection.remove(key) )
    {
        QString errMsg = "ERR: cant remove image:<"+key+">" ;
        this->answer.sprintf("z.handleServerAnswer([\"\",\"\",\"%s\",]);", errMsg.latin1());
        qWarning( "%s:%d:%s() <%s>.", __FILE__,__LINE__,__FUNCTION__, this->answer.latin1());
    }
};

void Z::clearImages()
{
    this->imagesCollection.clear();
};

void Z::listImages()
{
    //~ this->answer = "{\"images\": [ " ;
    //~ this->answer = "{'0xFFFFFFFF': 'OK', 'images' : [ ";
    this->answer = "z.setImages([ ";
    QDictIterator<QImage> it( this->imagesCollection ); // See QDictIterator
    for( ; it.current(); ++it ) this->answer += "\"" + it.currentKey() + "\",";
    //~ this->answer += "] };" ;
    this->answer += "]);" ;
    MY_WARNING(0x03)( "%s:%d:%s() <%s>.", __FILE__,__LINE__,__FUNCTION__, this->answer.latin1());
};

//~ ########################
void Z::set_LCDlamp_delay_sec(int d)
{
    this->LCDlamp_delay_sec = d ;
    qWarning("%s(%d) this->LCDlamp_delay_sec = %d.", __FUNCTION__, d, this->LCDlamp_delay_sec );
}

void Z::set_timeout_delay_msec(int d)
{
    this->timeout_delay_msec = d ;
    qWarning("%s(%d) this->timeout_delay_msec = %d.", __FUNCTION__, d, this->timeout_delay_msec );

    this->timer->stop();
    this->timer->start(this->timeout_delay_msec, FALSE/* NOT SINGLE SHOT */);

}

void Z::switch_LCDlamp(int flag)
{
    if(this->LCDlamp_status != flag)
    {
        char cmd[32];
        //~ sprintf(cmd, "gpio_user 8 %d", flag);
        sprintf(cmd, "switch_LCDlamp %d", flag);
        system(cmd);
        MY_WARNING(0x01)("%s() cmd = %s.", __FUNCTION__, cmd);
        this->LCDlamp_status = flag;
    }

    MY_WARNING(0x01)("%s(%d) this->LCDlamp_status = %d.", __FUNCTION__, flag, this->LCDlamp_status );
}

//~ ########################
void Z::setDbgLevel(const QString& level)
{
    static FILE * NULLDEV = fopen("/dev/null","w");
    static FILE * STDERR = stderr;
    static FILE * STDOUT = stdout;

    if(( (int) stderr != 2 ) or ( (int) stdout != 1 ))
    {
        qWarning( "%s:%d:%s() stdout=%d, stderr=%d.", __FILE__,__LINE__,__FUNCTION__, (int) stdout, (int) stderr);
    }

    qWarning( "%s:%d:%s(%s) debugLevel=%d.", __FILE__,__LINE__,__FUNCTION__, level.latin1(), this->debugLevel);
    this->debugLevel = level.toInt();
    qWarning( "%s:%d:%s(%s) debugLevel=%d.", __FILE__,__LINE__,__FUNCTION__, level.latin1(), this->debugLevel);

    if((this->debugLevel & 0x0FFF) <= 0)
    {
        stderr = NULLDEV ;
        stdout = NULLDEV ;
    }
    else
    {
        stderr = STDERR ;
        stdout = STDOUT ;
    }
}

void Z::showString(const QString& message)
{
    QLabel * lb = (QLabel *) this->child( "logo", "QLabel");
    if(lb != NULL )
    {
        lb->setText(message);
    }
}

void Z::showLogo(const QString& onOffFlag, const QString& imgName/* ="logo" */)
{
    QLabel * lb = (QLabel *) this->child( "logo", "QLabel");
    if(lb != NULL )
    {
        int f = onOffFlag.toInt();

        if(f)
        {
            lb->raise();
            lb->show();
            this->show();
        }
        else lb->hide();

        lb->setText(imgName);

        QImage * im = this->imagesCollection[imgName] ;
        if( (im != NULL) and ( not im->isNull() ) )
        {
            QPixmap p( *im );
            lb->setPixmap(p);
        }
        else
        {
            QPixmap p(imgName);
            if( not p.isNull() )
            {
                lb->setPixmap(p);
            }
            else
            {
                QString startMsg("");
                QFile file( imgName );
                if ( file.open( IO_ReadOnly ) ) {
                    QTextStream stream( &file );
                    while ( !stream.atEnd() )  startMsg += stream.readLine(); // line of text excluding '\n'
                    file.close();
                }
                if(startMsg.length() != 0) lb->setText(startMsg);
            }
        }
    }
}


void Z::dumpObjects()
{
    dumpRecursive( 0, this );
}

void Z::setMinStructSize(int minX, int minY)
{
    this->answer = ";";
    qApp->setGlobalStrut(QSize(minX,minY));
    MY_WARNING(0x03)( "%s:%d:%s() <%s>.", __FILE__,__LINE__,__FUNCTION__, this->answer.latin1());
};

void Z::setCodecName(const QString& s)
{
    this->settedCodecName = s;
    MY_WARNING(0x03)( "%s:%d:%s() <%s>.", __FILE__,__LINE__,__FUNCTION__, this->answer.latin1());
};

void Z::setFontFromFamilyName(const QString& family)
{
    QFont f(family);
    this->setFont(f);
    MY_WARNING(0x03)( "%s:%d:%s() <%s>.", __FILE__,__LINE__,__FUNCTION__, this->answer.latin1());
}
void Z::insertItem(const QString& listBoxname, const QString& text)
{
    QListBox * lb = (QListBox *) this->child( listBoxname, "QListBox");
    if(lb == NULL)
    {
        qWarning("%s:%d:%s() lb == NULL.", __FILE__,__LINE__,__FUNCTION__);
        return ;
    }

    QPixmap pix;
    QString name = text ;
    if( text.contains("&") )
    {
        QString imName = text.section("&",-1,-1) ;
        qWarning( "%s:%d:%s() imName=%s.", __FILE__,__LINE__,__FUNCTION__, imName.latin1() );
        QImage * im = this->imagesCollection[ imName ] ;
        if( (im != NULL) and ( not im->isNull() ) )
        {
            pix = QPixmap( *im );
            qWarning("%s:%d:%s().", __FILE__,__LINE__,__FUNCTION__);
        }else qWarning("%s:%d:%s().", __FILE__,__LINE__,__FUNCTION__);
        name = text.section("&",0,-2);
    }
    if(not pix.isNull()) {lb->insertItem( pix, name );qWarning("%s:%d:%s().", __FILE__,__LINE__,__FUNCTION__);}
    else lb->insertItem( name ) ;
}


void Z::setMimeSourceFactoryPathList(const QString& path)
{
    //~ void QMimeSourceFactory::setFilePath ( const QStringList & path ) [virtual]
    //~ Sets the list of directories that will be searched when named data is
    //~ requested to the those given in the string list path.

    QMimeSourceFactory::defaultFactory()->setFilePath ( QStringList::split(',', path) ) ;

    QStringList l = QMimeSourceFactory::defaultFactory()->filePath() ;
    QString s = l.join(",");

    qWarning( "%s:%d: QMimeSourceFactory::defaultFactory()->filePath() => %s",__FILE__, __LINE__, s.latin1() );
}
void Z::addLibraryPath(const QString& path)
{
    QApplication::addLibraryPath(path);
    //~ QApplication::addLibraryPath("/usr/lib/kde3/plugins");

    QStringList librarypaths = QApplication::libraryPaths();
    MY_WARNING(0x02)("library paths:");
    for ( QStringList::Iterator s = librarypaths.begin(); s != librarypaths.end(); ++s )
    {
        QString sn = *s;
        MY_WARNING(0x02)( "\t%s", sn.latin1() );
    }
}

void Z::setStyle(const QString& styleName)
{
    //~ QStyle * qtcurve_style  = QStyleFactory::create( "QtCurveStyle" );
    //~ qWarning( "%s:%d. qtcurve_style: %p",__FILE__, __LINE__, qtcurve_style);
    //~ QStyle * plastik_style  = QStyleFactory::create( "PlastikStyle" );
    //~ qWarning( "%s:%d. plastik_style: %p",__FILE__, __LINE__, plastik_style);

    QStyle * new_style = NULL ;

    if     (styleName == "QWindowsStyle"  ) new_style = new QWindowsStyle  ();
    else if(styleName == "QMotifStyle"    ) new_style = new QMotifStyle    ();
    else if(styleName == "QCDEStyle"      ) new_style = new QCDEStyle      ();
    else if(styleName == "QMotifPlusStyle") new_style = new QMotifPlusStyle();
    else if(styleName == "QSGIStyle"      ) new_style = new QSGIStyle      ();
    else if(styleName == "QPlatinumStyle" ) new_style = new QPlatinumStyle ();
    else if(styleName == "PolymerStyle"   ) new_style = new PolymerStyle   ();
    else new_style = QStyleFactory::create(styleName);

    if( new_style == NULL) new_style = new QWindowsStyle ();

    QApplication::setStyle( new_style );

    QStringList stylenames = QStyleFactory::keys() ;
    stylenames += "QWindowsStyle"  ;
    stylenames += "QMotifStyle"    ;
    stylenames += "QCDEStyle"      ;
    stylenames += "QMotifPlusStyle";
    stylenames += "QSGIStyle"      ;
    stylenames += "QPlatinumStyle" ;
    stylenames += "PolymerStyle"   ;

    MY_WARNING(0x02)("widget styles:");
    for ( QStringList::Iterator s = stylenames.begin(); s != stylenames.end(); ++s )
    {
        QString sn = *s;
        MY_WARNING(0x02)( "\t%s", sn.latin1() );
    }
}

//~ EOF
