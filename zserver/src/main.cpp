/****************************************************************************
** $Id: zserver/main.cpp  
**
** author:   giovanniangeli@tbe.it
*****************************************************************************/

#include <qapplication.h>

#include "zserver.h"

int main( int argc, char **argv )
{
    //~ int dummy = 0;
    int port = 0;
    int dbgLevel = 1;
    for(int i = 0; i < argc; i++)
    {
        if( not strcmp(argv[i],"-p") and ( argc > i+1 ) ) 
        {
            port = QString(argv[i+1]).toInt();
        }
        else if( not strcmp(argv[i],"-d") and ( argc > i+1 ) ) 
        {
            dbgLevel = QString(argv[i+1]).toInt();
        }    
        qWarning( "%s. argv[%d/%d]: %s",__FILE__, i,argc, argv[i]);
    }
    qWarning( "%s. build: %s [%s]",__FILE__, __DATE__,__TIME__);
    
    QApplication a(argc,argv);
    
    //~ qWarning( "%s:%d:%s()",__FILE__, __LINE__,__FUNCTION__);
    
    Z z( NULL, "z", QApplication::WStyle_Customize, port, dbgLevel );
    a.setMainWidget( &z );
    z.show();
    a.connect( &a, SIGNAL(lastWindowClosed()), SLOT(quit()) );
    return a.exec();
}
