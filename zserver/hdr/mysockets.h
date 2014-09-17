/****************************************************************************
** file:        mysockets.h
** project:  zserver
** author:   giovanniangeli@tbe.it
** created at: giovedì 19 ottobre 2006 12:55:31 +0000; by addAClass.py ver. 0.2
** license: GPL
****************************************************************************/
#ifndef MYSOCKETS_H
#define MYSOCKETS_H

#include <qptrlist.h>
#include <qsocket.h>
#include <qserversocket.h>

#include "globals.h"

class MyClient : public QSocket
{
    Q_OBJECT

public:
 
    MyClient( int sock, QObject *parent=0, const char *name=0 ) ;
    ~MyClient();

    int getSocket(void){ return this->socket ; };
    
signals:
    
    void getLine( const QString& , MyClient *);
    void closeSignal(MyClient*);

private slots:
    
    void readClient();
    void emitCloseSignal();

private:
    
    int line;
    int socket;
};

class MyServer : public QServerSocket
{
    Q_OBJECT

public:
    
    MyServer( QObject* parent=0, int port=4242 );
    ~MyServer();
    
    QSocketDevice * getSocketDevice(){ return this->socketDevice() ; };
    void newConnection( int socket );
    void sendToClients( const QString& s);
    void sendToClient( const QString& s, MyClient* c );

public:
    
    QPtrList<MyClient> myClients;

signals:
    
    void newConnect( MyClient* c );

private slots:
    
    void handleClientData( const QString& s, MyClient * c);
    void closeClient(MyClient *);
};


#endif // MYSOCKETS_H
