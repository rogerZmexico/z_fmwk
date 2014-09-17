/****************************************************************************
** file:     mysockets.cpp
** project:  zserver
** author:   giovanniangeli@tbe.it
** created at: Sat, 04 Mar 2006 12:55:31 +0000; by addAClass.py ver. 0.2
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
#include "mysockets.h"

MyClient::MyClient( int sock, QObject *parent, const char *name ) 
    : QSocket( parent, name )
    ,line(0)
    ,socket(0)
{
    this->line = 0;
    connect( this, SIGNAL(readyRead()), SLOT(readClient()) );
    connect( this, SIGNAL(connectionClosed()), SLOT(deleteLater()) );
    connect( this, SIGNAL(connectionClosed()), SLOT(emitCloseSignal()) );
    setSocket( sock );
}

MyClient::~MyClient()
{
    qWarning( "%s:%d:%s()", __FILE__,__LINE__,__FUNCTION__);
}

void MyClient::readClient()
{
    QTextStream ts( this );
    while ( this->canReadLine() ) 
    {
        QString str = ts.readLine();
        
        //~ qWarning( "%s:%d:%s() str=<%s>.", __FILE__,__LINE__,__FUNCTION__, str.ascii());
        
        emit getLine( str, this );
        //~ emit logText( tr("Read: '%1'\n").arg(str) );
        //~ ts << line << ": " << str << endl;
        //~ emit logText( tr("Wrote: '%1: %2'\n").arg(line).arg(str) );
        line++;
    }
}

void MyClient::emitCloseSignal()
{
    emit closeSignal(this);
}

MyServer::MyServer( QObject* parent, int port ) 
    : QServerSocket( port, 1, parent )
    ,myClients()
{
    if ( ! this->ok() ) 
    {
        qWarning("Failed to bind to port 4242");
        //~ exit(1);
    }
    this->myClients.setAutoDelete(FALSE);
}

MyServer::~MyServer()
{
    qWarning( "%s:%d:%s()", __FILE__,__LINE__,__FUNCTION__);
    for(unsigned int i=0; i<this->myClients.count(); i++)
    {
        MyClient * c = this->myClients.at(i) ;
        c->close();
        c->deleteLater();
        this->myClients.remove( c );
    }
}

void MyServer::newConnection( int socket )
{
    MyClient *c = new MyClient( socket, this );
    //~ NOTE TO THE USER: you should implement a slot like this and connect 
    //~ it in the container constructor, like in the following: 
    //~ ...
        //~ this->serverSocket = new MyServer(this, SOCKET_PORT) ;
        //~ connect( this->serverSocket, SIGNAL(newConnect(MyClient*)), this, SLOT(newConnect(MyClient*)) );
    //~ ...
    //~ void container::newConnect(MyClient * c)
    //~ {
        //~ MY_WARNING( "%s:%d:%s: %s", __FILE__,__LINE__,__FUNCTION__, myTr(QString("New connection")).ascii() );
        //~ connect(c, SIGNAL(getLine( const QString& , MyClient *)), this, SLOT(handleSocketData( const QString& , MyClient *)) );
    //~ }
    this->connect(c, SIGNAL(closeSignal(MyClient*)), this, SLOT(closeClient(MyClient*)) );

    emit newConnect( c );
    this->myClients.append(c);
}

void MyServer::handleClientData( const QString& s, MyClient * c)
{
    qWarning( "%s:%d:%s(%s, %p)", __FILE__,__LINE__,__FUNCTION__, s.ascii(), c );
}

void MyServer::closeClient(MyClient * c)
{
    this->myClients.remove(c);
    qWarning( "%s:%d:%s(%p)", __FILE__,__LINE__,__FUNCTION__, c );
}

void MyServer::sendToClients( const QString& s)
{
    for(unsigned int i=0; i<this->myClients.count(); i++)
    {
        this->sendToClient( s, this->myClients.at(i) );
    }
}


void MyServer::sendToClient( const QString& s, MyClient* c )
{
    const QString & s1 = s ;
    //~ qWarning( "%s:%d:%s: %s", __FILE__,__LINE__,__FUNCTION__, s1.ascii() );
    int ret = c->writeBlock( s1.latin1(), s1.length() );
    c->flush();
    if( ret < 0 )
    {
        qWarning( "%s:%d:%s() ret=%d.", __FILE__,__LINE__,__FUNCTION__, ret);
    }        
    else if( ret == 0 ) /* shut down */
    {
        qWarning( "%s:%d:%s() ret=%d.", __FILE__,__LINE__,__FUNCTION__, ret);
    }
    else if( (unsigned int) ret < s1.length() )
    {
        qWarning( "%s:%d:%s() ret=%d.", __FILE__,__LINE__,__FUNCTION__, ret);
    }
}
