/****************************************************************************
** file:    mywidgetfactory.cpp
** project: zserver
** author:  giovanniangeli@tbe.it
** created at: Sat, 27 May 2006 21:38:46 +0000; by addAClass.py ver. 0.2
** license: GPL
****************************************************************************/
#include "mywidgetfactory.h"

#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qerrormessage.h>

#include "plot.h"
#include "rotor.h"
#include "g_label.h"

MyWidgetFactory::MyWidgetFactory()
    :    QWidgetFactory()
{
}

MyWidgetFactory::~MyWidgetFactory()
{
}

//---------METHODS
QWidget *MyWidgetFactory:: createWidget( const QString &clName, QWidget *parent,const char *name ) const
{
    //~ qWarning("%s:%d:%s(%s,%s,%s)",__FILE__,__LINE__,__FUNCTION__,clName.latin1(),parent->name(),name);

    if     (clName == "QFileDialog"  ) return ( (QWidget *) new QFileDialog  ( parent, name ) ) ;
    else if(clName == "QMessageBox"  ) return ( (QWidget *) new QMessageBox  ( parent, name ) ) ;
    else if(clName == "QErrorMessage") return ( (QWidget *) new QErrorMessage( parent, name ) ) ;
    else if(clName == "Plot"         ) return ( (QWidget *) new Plot         ( parent, name ) ) ;
    else if(clName == "Rotor"        ) return ( (QWidget *) new Rotor        ( parent, name ) ) ;
    else if(clName == "G_Label"      ) return ( (QWidget *) new G_Label      ( parent, name ) ) ;

    return NULL;
}

QStringList MyWidgetFactory::widgets()
{
    QStringList wdgList = QWidgetFactory::widgets();

    if( wdgList.find("QFileDialog"  ) == wdgList.end() ) wdgList.append("QFileDialog"   ) ;
    if( wdgList.find("QMessageBox"  ) == wdgList.end() ) wdgList.append("QMessageBox"   ) ;
    if( wdgList.find("QErrorMessage") == wdgList.end() ) wdgList.append("QErrorMessage" ) ;
    if( wdgList.find("Plot"         ) == wdgList.end() ) wdgList.append("Plot"          ) ;
    if( wdgList.find("Rotor"        ) == wdgList.end() ) wdgList.append("Rotor"         ) ;
    if( wdgList.find("G_Label"      ) == wdgList.end() ) wdgList.append("G_Label"       ) ;

    return wdgList;
}

bool MyWidgetFactory::dumpWidgets()
{
    QStringList l = this->widgets();
    l.sort();
    QString s = "available widgets:\n\r\t- " + l.join("\n\r\t- ");
    qWarning(s.ascii());
    return TRUE;
}
//---------SLOTS

