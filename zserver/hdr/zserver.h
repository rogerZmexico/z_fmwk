/****************************************************************************
** file:    zserver.h
** project: zserver
** author:  giovanniangeli@tbe.it
** created at: mar set 18 12:14:00 2007;
** license: GPL

**  Copyright (C) 2007-2009  giovanni angeli www.tbe.it

**  This program is free software: you can redistribute it and/or modify
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

#ifndef Z_H
#define Z_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <qdict.h>

#include "globals.h"

#include "splash.h"

class QWidget ;
class QFile ;
class QImage ;
class QString ;
class QStringList ;
class QWSEvent ;
class QButtonGroup ;
class QListBoxItem ;
class QDate ;
class QTime ;
class MyWidgetFactory ;
class MyClient ;
class MyServer ;

class Z : public Splash
{
    Q_OBJECT

public: // methods

    Z(QWidget * parent, const char * name, WFlags fl, int SOCKET_PORT, int DBG_LEVEL);
    ~Z();

private: // methods

    bool eventFilter( QObject *o, QEvent *e );

    void sendMessageToTheSocket(const char * msg, MyClient * client=NULL);
    void sendMessageToTheSocket(const QString& msg, MyClient * client=NULL);

    QObject * findObject (const QString & name);

    MyClient* getClient(QObject * pFrame);
    QObject * getParentFrame(QObject * c);

    bool callSlot(QObject * o , const QString & method, const QStringList & args );
    bool setProperty(QObject * o , const QString & method, const QStringList & args );
    bool getProperty(QObject * o , const QString & method );
    bool searchInMetaObjectSpecs(QObject * o , const QString & method );
    void iterateObjects( QObjectList * w );

public: // members
private: // members

    QWidget * dummy;

    int socketPort;
    int debugLevel ;
    int init_delay_msec ;
    int timeout_delay_msec ;
    int timeout_cntr ;
    int LCDlamp_delay_sec ;
    bool LCDlamp_status ;

    MyServer * serverSocket;

    QTimer *timer ;
    QObject *currentPressedObject;
    QString answer ;
    MyClient *currentClient;
    QMap<QWidget*, MyClient*> frameToClient ;

    QDict<QImage> imagesCollection ;

    QString settedCodecName ;

public slots:
private slots:

    void initialize(void);
    void finalize(void);
    void timeout();

    void set_LCDlamp_delay_sec(int d);
    void switch_LCDlamp(int flag) ;
    void set_timeout_delay_msec(int d);

    //~ ########################
    void handleSocketData( const QString& s, MyClient * c);
    void buildFromUiFile(const QString& name);
    void createWidget(const QString& args);
    void insertItem(const QString& listBoxname, const QString& text);
    void newConnect(MyClient * c);
    void closeClient(MyClient * c);
    //~ ########################
    void clicked();
    void clicked(int section);
    void clicked(int r, int c);
    void clicked(int row, int col, int button, const QPoint & mousePos);
    void valueChanged(int);
    void valueChanged(const QString&);
    void valueChanged(const QTime&);
    void valueChanged(const QDate&);
    void textChanged(const QString&);
    void selectionChanged(QListBoxItem*i);
    void linkClicked(const QString&);
    void anchorClicked(const QString&,const QString&);

    void appendImage(const QString & imgFileNameAndPath);
    void removeImage(const QString & key);
    void clearImages();
    void listImages();

    void setDbgLevel(const QString& level);
    void addLibraryPath(const QString& path);
    void setStyle(const QString& styleName);
    void setCodecName(const QString& name);
    void setMimeSourceFactoryPathList(const QString& path);
    void setFontFromFamilyName(const QString& family);
    void setMinStructSize(int minX, int minY);
    void showString(const QString& message);
    void showLogo(const QString& onOffFlag, const QString& imgName="logo");
    void dumpObjects();

signals:

    void generic();
    void generic(int);
    void generic(const QString&);
    void generic(int,int);
    void generic(const QString&,const QString&);
};

#endif // Z_H
