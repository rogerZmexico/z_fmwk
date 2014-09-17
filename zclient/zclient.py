#!/usr/bin/python

"""
#~ ######################################################
#~ # project: zclient
#~ # file: zclient.py
#~ # author: giovanni.angeli6@gmail.com
#~ # Copyright (C) 2007 - 2011  giovanni angeli
#~ ######################################################
"""
import os, sys
import select
import socket
import traceback
import time
import calendar
import urllib
import cPickle as pickle
#~ import xml.dom.minidom as xdm
import ports

VERSION = '1.1.12 - 30/03/2010'
print '---- %s version: %s ----' %(__file__, VERSION )

HOST_PORT = 4111
#~ HOST_IP = "192.168.1.45"
#~ HOST_IP = "192.168.2.50"
HOST_IP = "127.0.0.1"

class Zclient(ports.zProcess, ):
    def __init__(self, path=None, host_ip=HOST_IP, host_port=HOST_PORT, name='zclient', run_select_timeout_sec=0.4, debug=0 ):
        ports.zProcess.__init__(self, name, run_select_timeout_sec, debug)
        self.frames = {}
        self.runFlag = 0
        self.logFile = None
        self.images = []
        self.locals = {}
        self.globals = {}

        if path == None : path = os.getcwd()
        self.confPath = os.path.join( path , '..', 'conf' )
        self.uiPath = os.path.join( path , '..', 'ui' )
        self.imagesPath = os.path.join( path , '..', 'img' )

        self.guiPort = ports.IPCport(self, host=(host_ip, host_port), name='guiport',  )
        self.ports.append(self.guiPort)

    def printToLogFile( self,  str ):
        if not self.logFile.closed :
            self.logFile.write( str + "\r\n" )
            self.logFile.flush()

    def handleExcept(self, level=2, fd=sys.stderr):
        sep = '*'*10
        fd.write(sep+"< ")
        traceback.print_exc( level, fd )
        fd.write(sep+">\n\r")
        fd.flush()

        if sys.exc_type == KeyboardInterrupt :
            self.runFlag = 0

    def exit( self, ):

        self.guiPort.write('z clearImages')
        self.guiPort.write('z showLogo 1')
        time.sleep(0.4)
        ports.zProcess.exit(self, )

    #~ def loadTranslations(self, ):

        #~ try :
            #~ path = os.path.dirname(os.path.abspath(__file__))
            #~ execfile(os.path.join(path, "translations.py"), locals(), globals())
            #~ for (k, v) in translations.items() :
                #~ try :
                    #~ if k == 'z' :
                        #~ self.dictionary = v
                    #~ else :
                        #~ self.frames[k].dictionary = v
                #~ except : self.handleExcept()
        #~ except : self.handleExcept()


    def handleServerAnswer(self, msgList ):

        if self.DEBUG & 4 : print '%s.handleServerAnswer(%s)'%(self.name,msgList)

        if len(msgList) > 2 :
            if msgList[2].split(':')[0].strip() == 'ERR' :
                try :
                    msg = ' '.join(msgList)
                    print msg
                    raise TypeError(msg)
                except : self.handleExcept()

#~  WRAPPERS: #################################################
    def appendImage( self, imgFileNameAndPath ):
        msg = "0xFFFFFFFF appendImage %s" %(urllib.quote(imgFileNameAndPath))
        self.sendMessageToTheSocket(msg)

    def removeImage( self, imgName ):
        msg = "0xFFFFFFFF removeImage %s" %(urllib.quote(imgName))
        self.sendMessageToTheSocket(msg)

    def listImages( self, ):
        msg = "0xFFFFFFFF listImages"
        self.sendMessageToTheSocket(msg)

    def setImages(self, imgList):

        self.images = imgList

    def sendMessageToTheSocket(self, msg):

        if (self.DEBUG & 8) :
            print "%s.sendMessageToTheSocket() msg:\"%s\"." % (self.name, msg )
            sys.stdout.flush()
        self.guiPort.write(msg)
        return {}

    def connectGui(self,):
        self.guiPort.open()

class Widget(ports.Localizable, ports.Pickleable):

    def __init__(self, node=None, parentProcess=None ):

        self.parentProcess = parentProcess
        self.node = node

        try : self.name
        except : self.name = 'noname'

        self.isVisible = 0

    def rpc(self, method, args=[], childName=None, frameName=None):

        if frameName == None : frameName = self.name
        if childName == None : name = '%s'%(frameName)
        else : name = '%s.%s' %(self.name, childName)
        msg = "%s %s"%(name, method)

        if args :
            for a in args :
                try : msg += " %s"%(urllib.quote(a))
                except : msg += " %s"%(a)

        if self.parentProcess.DEBUG & 2 :
            quotedArgs = []
            for a in args : quotedArgs.append(urllib.quote(a))
            print "quotedArgs=%s"%(quotedArgs,)

        self.parentProcess.sendMessageToTheSocket( msg )

    def fixUiFile(self, uiFile):

        f = open( uiFile, "r")
        lines = []
        for line in f.readlines():
            if not line.isspace() and len(line) > 0 :
                lines.append(line)
        f.close()

        i = lines[-1].find(">")
        if( lines[-1] != lines[-1][0:i+1] ) :
            lines[-1] = lines[-1][0:i+1]
            new = "".join(lines)
            f = open( uiFile, "w")
            f.write( new )
            f.close()
            print '%s.fixUiFile() <%s> fixed'%(self.name, uiFile)

    def buildFromUiFile(self, uiFileName=None ):

        if uiFileName == None : self.uiFileName = self.name + ".ui"
        else : self.uiFileName = uiFileName

        uiFile = os.path.join( self.parentProcess.uiPath , self.uiFileName )
        self.parentProcess.sendMessageToTheSocket( "0xFFFFFFFF buildFromUiFile %s"%(uiFile,) )
        self.parentProcess.frames[self.name] = self

        return self

    def callSlot(self, slotName, args=[], childName=None, frameName=None):
        return self.rpc(slotName, args, childName, frameName)

    def property(self, propertyName, args=[], childName=None, frameName=None):
        return self.rpc(propertyName, args, childName, frameName)

#~  WRAPPERS: #################################################
    def createWidget(self, name, className, parentWidgetName, pos=["10", "10"], size=["80", "64"]):
        if len(name) > 32 : name = name[:32]
        if len(className) > 32 : className = className[:32]
        self.parentProcess.sendMessageToTheSocket('z createWidget %s,%s,%s.%s'%( name, className, self.name, parentWidgetName,))

        self.callSlot('move', pos, childName=name)
        self.callSlot('resize', size, childName=name)
        self.callSlot('text', [name,], childName=name)
        self.callSlot('show', childName=name)

    def propertyNames(self, all=0 ):
        if all : return self.rpc("propertyNamesAll")
        else :   return self.rpc("propertyNames")

    def slotNames(self, all=0 ):
        if all : return self.rpc("slotNamesAll")
        else :   return self.rpc("slotNames")

    def signalNames(self, all=0 ):
        if all : return self.rpc("signalNamesAll")
        else :   return self.rpc("signalNames")

    def show(self):
        self.isVisible = 1
        r = self.callSlot("raise")
        r = self.callSlot("show")
        return r

    def hide(self, ):
        self.isVisible = 0
        r = self.callSlot("hide")
        return r

#~  CALLED BY THE SERVER: #################################################
    def clicked(self, childName):
        #~ print "clicked: %s.%s" % (self.name, childName)
        pass

    def selectionChanged(self, childName, currentText):
        #~ print "selectionChanged: %s in %s.%s" % (currentText, self.name, childName)
        pass

    def linkClicked(self, name, link):
        print "%s.linkClicked(%s, %s)"%(self.name, name, link)


    def anchorClicked(self, name, anchor, link):
        #~ print "%s.anchorClicked(%s, %s, %s)"%(self.name, name, anchor, link)
        pass

    def textChanged(self, childName, value):
        #~ print "%s.valueChanged(%s, %s)"%(self.name, childName, value)
        pass
    def valueChanged(self, childName, value):
        #~ print "%s.valueChanged(%s, %s)"%(self.name, childName, value)
        pass

#~  ###################################
class EditableItem:

    value = 0
    isCyclic = 0
    def __init__(self, name, defValue=0., sequence=None, min=0, max=10, step=1, units=[], scale=1):

        self.name = name
        self.sequence = sequence
        self.min = min
        self.max = max
        self.step = step
        self.units = units
        self.scale = scale

        v = int( defValue / scale )
        if v >= min and v <= max : self.value = v
        else : self.value = min

    def setVal(self, newVal):

        if self.isCyclic :
            if self.sequence != None :

                self.value = newVal % len(self.sequence)

            else :

                if newVal > self.max or newVal < self.min :
                    self.value = self.min + (newVal - self.min) % (self.max - self.min)
        else :
            if self.sequence != None :

                if (newVal >= len(self.sequence)) or (newVal < 0):
                    return None
                else :
                    self.value = newVal
                    return newVal
            else :

                if newVal > self.max or newVal < self.min :
                    pass
                else :
                    self.value = newVal

        return self.value

    def changeVal(self, n):

        oldVal = self.value
        return self.setVal(self.value + n) - oldVal

    def increment(self,):
        return self.changeVal(1)

    def decrement(self,):
        return self.changeVal(-1)

    def valToString(self, val=None):

        if val == None : val = self.value


        if self.isCyclic :
            if self.sequence != None :

                val = val % len(self.sequence)
                return str(self.sequence[val])

            else :

                if val > self.max : val = val % self.max
                if val < self.min : val = self.max - val

                return str(val * self.scale)

        else :

            if self.sequence != None :

                if val >= 0 and val < len(self.sequence) :
                    return str(self.sequence[val])
                else :
                    return ''

            else :

                if val > self.max : val = self.max
                if val < self.min : val = self.min

                return str(val * self.scale)


    def getExtrema(self, ):

        if self.sequence != None :
            return (0, len(self.sequence))
        else :
            return (self.min, self.max)

    def setValFromString(self, st):
        i = self.stringToInt(st)
        return self.setVal(i)

    def stringToInt(self, st):

        if self.sequence != None :
            count = 0
            for i in self.sequence :
                if st == str(i) :
                    return count
                count  += 1

            print "<%s> not found in sequence: %s" %(st, self.sequence)
            raise NameError()

        else :

            if self.scale != 1 :
                tmp_value = int( float(st) / self.scale )
            else :
                tmp_value = int(st)

            if tmp_value > self.max or tmp_value < self.min :
                print "out of range"
                raise NameError()

            return tmp_value

#~  ###################################
class TextBrowser(Widget):

    bodyBgColor = 'blue'
    fontColor = 'yellow'

    def __init__(self, parent):
        Widget.__init__(self, node=None, parentProcess=parent)
        self.name = "textbrowser"

        self.body_buff = ""
        self.body_src = ""
        self.okClicked = self.default_okClicked
        self.escClicked = self.default_escClicked
        self.anchorClicked = self.default_anchorClicked

        self.isVisible = 0

    def initialise(self, ):

        self.property('readOnly', ['1'], childName='body')

    def show(self,okClicked=None,escClicked=None,msg=None,hideOk=0,hideBack=1,hideEsc=0,hideUpDown=0,img=None):

        #~ self.property('textFormat', ['%s'%textFmt], childName='body')

        if okClicked != None : self.okClicked = okClicked
        else : self.okClicked = self.default_okClicked

        if escClicked != None : self.escClicked = escClicked
        else : self.escClicked = self.default_escClicked

        if hideOk : self.callSlot('hide', childName='b_OK')
        else : self.callSlot('show', childName='b_OK')

        if hideBack : self.callSlot('hide', childName='b_BACK')
        else : self.callSlot('show', childName='b_BACK')

        if hideEsc : self.callSlot('hide', childName='b_ESC')
        else : self.callSlot('show', childName='b_ESC')

        if hideUpDown :
            self.callSlot('hide', childName='b_UP')
            self.callSlot('hide', childName='b_DOWN')
        else :
            self.callSlot('show', childName='b_UP')
            self.callSlot('show', childName='b_DOWN')

        if msg != None :
            if img == None :
                img = 'alert'

            srcpth = os.path.join(self.parentProcess.imagesPath, '%s.png'%(img))
            self.body_buff = '<html><body bgcolor="%s">'%(self.bodyBgColor)
            self.body_buff += '<table width="100%" cellspacing="4" border="0" cellpadding="4" align="center">'
            self.body_buff += '<tr><td align="center"><IMG src="%s" border="0"></td></tr>'%(srcpth )
            self.body_buff += '<tr><td align="center">'
            self.body_buff += '<font color="%s" size="+2"><b>%s</b></font>' %(self.fontColor, msg)
            self.body_buff += '</td></tr>'
            self.body_buff += '</body></html>'

        self.property("text", [self.body_buff, ], childName="body")
        Widget.show(self, )

        #~ print self.body_buff

    def default_okClicked(self, ):
        print "%s.default_okClicked()"%(self.name, )
        self.hide()

    def default_escClicked(self, ):
        print "%s.default_escClicked()"%(self.name, )
        self.hide()

    def setSrc(self, src):
        self.body_src = src

        self.body_buff = ''
        #~ self.callSlot("setSource", [self.body_src , ], childName='body')
        if os.path.splitext(src)[1] in ['.png'] :
            self.body_buff += '<html><body bgcolor="#000033">'
            self.body_buff += '<img src="%s"></img><br>'%(src, )
            self.body_buff += '%s'%(src, )
            self.body_buff += '</body></html>'

        elif os.path.splitext(src)[1] in ['.prg', '.txt', '.ini', '.sh', '.py'] :
            f = open(src)
            self.body_buff += '<html><body bgcolor="#000033"><font color="#EEEEFF">'
            for l in f.readlines():
                self.body_buff += l + '<br>'
            self.body_buff += '</font></body></html>'

        elif os.path.splitext(src)[1] in ['.html'] :

            f = open(src)
            for l in f.readlines():
                self.body_buff += l

        else : self.body_buff = src

        self.property("text", [self.body_buff,], childName='body')

    def setText(self, txt):
        self.body_buff = txt
        self.property("text", [txt,], childName='body')

    def appendText(self, txt):
        self.body_buff += txt
        self.property("append", [txt,], childName='body')

    def clicked(self, childName):

        print "%s.clicked(%s)"%(self.name, childName)

        if   childName == "b_ESC" :
            self.hide()
            self.escClicked()
        #~ elif childName == "b_BACK" : self.anchorClicked('body', 'BACK')
        elif childName == "b_OK" :
            self.hide()
            self.okClicked()
        elif childName == "b_UP" :  self.callSlot("scrollBy", ["0","-80"], childName='body')
        elif childName == "b_DOWN" :self.callSlot("scrollBy", ["0","80"], childName='body')

    def default_anchorClicked(self, name, anchor, link):

        print "%s.default_anchorClicked(%s, %s, %s)"%(self.name, name, anchor, link)
        selection = link.lstrip("#")

    def buidHTMLTable(self, title, list, extraList=None):

            msg = '<html><body text="blue" bgcolor="white">\n'

            msg += '<table width="80%%" cellspacing="6" cellpadding="2" bgcolor="%s">\n'%(self.bodyBgColor)

            msg += '<tr><td align="center" colspan="2">\n'

            msg += '  <table bgcolor="lightBlue">\n'
            msg += '  <tr>\n'
            msg += '    <td colspan="2" align="center">\n'
            msg += '    <font color="black">%s:</font>\n' %(title)
            msg += '    </td>\n'
            msg += '  </tr>\n'
            msg += '  </table>\n'

            msg += '</td></tr>\n'
            msg += '<tr><td>\n'

            msg += '  <table cellspacing="6" bgcolor="lightBlue">\n'
            n = 0
            for o in list :
                msg += '  <tr>\n'
                msg += '    <td width="10%%">%s</td>\n' %(n)
                msg += '    <td width="90%%" align="center" bgcolor="gray">\n'
                msg += '      <a name="%s">%s</a>\n' %(self.tr(o), self.tr(o), )
                msg += '    </td>\n'
                msg += '  </tr>\n'
                n+=1
            msg += '  </table>\n'

            if extraList :
                msg += '</td>\n'
                msg += '<td>\n'

                msg += '  <table cellspacing="6" bgcolor="lightBlue">\n'
                for o in extraList :
                    msg += '  <tr>\n'
                    msg += '    <td align="center" bgcolor="gray">\n'
                    msg += '      <a name="%s">%s</a>\n' %(self.tr(o), self.tr(o), )
                    msg += '    </td>\n'
                    msg += '  </tr>\n'
                msg += '  </table>\n'

            msg += '</td></tr>\n'
            msg += '</table>\n'

            msg += '</body></html>\n'

            #~ print msg
            return msg

class AlertDlg(TextBrowser):
    def __init__(self, parent):

        TextBrowser.__init__(self, parent)
        self.name = 'alertdlg'


class Dlg(Widget):

    def defaultCallBack(self, ):

        print '%s.defaultCallBack()' %(self.name)
        self.hide()
        self.buffer = ''

    def show(self, editableItem, okCallBack=None, escCallBack=None, ):

        self.escCallBack = self.defaultCallBack
        self.okCallBack  = self.defaultCallBack
        if escCallBack != None : self.escCallBack = escCallBack
        if okCallBack  != None : self.okCallBack  = okCallBack

        self.editableItem  = editableItem

        desc = self.parentProcess.tr(self.editableItem.name) + '\n'
        if self.editableItem.sequence != None :
            desc += '[%s,%s]'%(self.editableItem.sequence[0],self.editableItem.sequence[-1],)
        else : desc += '[%s,%s]'%(self.editableItem.min,self.editableItem.max,)
        if self.editableItem.units != None :
            if len(self.cv( self.editableItem.units )) > 0 :
                desc += ' (%s)'%self.cv( self.editableItem.units )

        self.buffer = self.editableItem.valToString()

        self.property('text', [desc,], childName='l_desc')
        self.property('text', [self.buffer, ], childName='l_value')

        self.oldValue = self.editableItem.value

        Widget.show(self, )

    def clicked(self, b_name):

        print  "%s.clicked(%s) self.buffer:%s"%(self.name, b_name, self.buffer)

        if b_name == "b_ESC" :
            #~ self.editableItem.value = self.oldValue
            self.property('enabled', ['0', ])
            self.escCallBack()
            self.hide()
            self.property('enabled', ['1', ])
            return 1
        elif b_name == "b_OK" :
            self.property('enabled', ['0', ])
            try : self.editableItem.setValFromString(self.buffer)
            except : self.parentProcess.handleExcept()
            self.okCallBack()
            self.hide()
            self.property('enabled', ['1', ])
            return 1
        elif b_name == "b_HELP" :
            return 1
        elif b_name == "b_RESET" :
            self.show()
            return 1

class EditDlg(Dlg):

    def __init__(self, parent):

        Widget.__init__(self, node=None, parentProcess=parent)
        self.name = 'editdlg'

    def show(self, editableItem, okCallBack=None, escCallBack=None, ):

        Dlg.show(self, editableItem, okCallBack, escCallBack)
        self.bigStep = int((editableItem.max - editableItem.min) / 5.)

    def clicked(self, b_name):

        if Dlg.clicked(self, b_name) : pass
        elif b_name == "b_P" : self.editableItem.increment()
        elif b_name == "b_M" : self.editableItem.decrement()
        elif b_name == "b_PP" : self.editableItem.changeVal(self.bigStep)
        elif b_name == "b_MM" : self.editableItem.changeVal(- self.bigStep)

        self.buffer = self.editableItem.valToString()
        self.property('text', [self.buffer, ], childName='l_value')

class ScrollDlg(EditDlg):

    def __init__(self, parent):

        EditDlg.__init__(self, parent)
        self.name = 'scrolldlg'

    def refresh(self, ):

        if not self.isVisible : return

        seq = self.editableItem.sequence

        if len(seq) == 0 : return

        s = ''
        s += '<body align="center">\n'
        s += '<table>\n'
        s += '  <tr>\n'
        s += '    <td align="center" bgcolor="white">\n'
        s += '    <font>%s</font>\n'%(self.editableItem.name)
        s += '    </td>\n'
        s += '  </tr>\n'

        for i in seq :
        #~ for i in seq + ['dummy', 'dummy', 'dummy', 'dummy', 'dummy', 'dummy', ] :
            if i == self.editableItem.valToString() : color = 'yellow'
            else : color = 'gray'
            s += '  <tr>\n'
            s += '    <td align="center" bgcolor="%s">\n'%(color)
            s += '    <font>%s</font>\n'%(i)
            s += '    </td>\n'
            s += '  </tr>\n'
        s += '</table>\n'
        s += '</body>\n'

        self.property('text', [s, ], childName='l_value')

        #~ print '%s.refresh() s=%s'%(self.name, s)

    def show(self, editableItem, okCallBack=None, escCallBack=None, ):

        EditDlg.show(self, editableItem, okCallBack, escCallBack)
        self.refresh()

    def clicked(self, b_name):

        EditDlg.clicked(self, b_name)
        self.refresh()

        if b_name == "b_ESC" :
            self.editableItem.setVal(self.oldValue)


class InputDlg(Dlg):

    def __init__(self, parent):

        Dlg.__init__(self, node=None, parentProcess=parent)
        self.name = 'inputdlg'

    def show(self, editableItem, okCallBack=None, escCallBack=None, ):

        Dlg.show(self, editableItem, okCallBack, escCallBack)
        self.firstClickFlag = 1

    def clicked(self, b_name):

        print  "%s.clicked(%s) self.buffer:<%s>"%(self.name, b_name, self.buffer )

        if Dlg.clicked(self, b_name) :
            self.firstClickFlag = 0
            return

        if b_name == "b_PM"      :  self.buffer = str(- int(self.buffer))
        else :

            if self.firstClickFlag : self.buffer = ''

            if b_name == "b_DEL" :
                if len(self.buffer) > 0 : self.buffer = self.buffer[:-1]
                if len(self.buffer) == 0 : self.buffer = ' '
            else : self.buffer += b_name[2]

        self.property('text', [ self.buffer , ], childName='l_value')

        self.firstClickFlag = 0

class InputFloatDlg(Dlg):

    def __init__(self, parent):

        Dlg.__init__(self, node=None, parentProcess=parent)
        self.name = 'inputfloatdlg'

    def show(self, value, desc, okCallBack=None, escCallBack=None, decPoints=1):

        self.decPoints = decPoints
        self.value = value
        self.desc = desc
        self.fmt = '%%.%df'%self.decPoints

        self.escCallBack = self.defaultCallBack
        self.okCallBack  = self.defaultCallBack
        if escCallBack != None : self.escCallBack = escCallBack
        if okCallBack  != None : self.okCallBack  = okCallBack

        self.buffer = self.fmt%self.value

        self.property('text', [desc,], childName='l_desc')
        self.property('text', [self.buffer, ], childName='l_value')

        self.oldValue = value

        self.firstClickFlag = 1

        Widget.show(self, )

    def clicked(self, b_name):

        print  "%s.clicked(%s) self.buffer:<%s>"%(self.name, b_name, self.buffer )

        if b_name == "b_ESC" :
            self.value = self.oldValue
            self.property('enabled', ['0', ])
            self.escCallBack()
            self.hide()
            self.property('enabled', ['1', ])
            return 1
        elif b_name == "b_OK" :
            self.value = float(self.buffer)
            self.property('enabled', ['0', ])
            self.okCallBack()
            self.hide()
            self.property('enabled', ['1', ])

        elif b_name == "b_PM"      :
            self.buffer = self.fmt%(-float(self.buffer))
        else :
            if self.firstClickFlag : self.buffer = ''

            if b_name == "b_DEL" :
                if len(self.buffer) > 0 : self.buffer = self.buffer[:-1]
                if len(self.buffer) == 0 : self.buffer = ' '
            elif b_name == "b_point" : self.buffer += '.'
            else : self.buffer += b_name[2]

        self.property('text', [ self.buffer , ], childName='l_value')

        self.firstClickFlag = 0

class DoubleInputDlg(Widget):

    def __init__(self, parent):

        Widget.__init__(self, node=None, parentProcess=parent)
        self.name = 'doubleinputdlg'
        self.selectedItemIndex = 0
        self.buffer = ['','']
        self.oldValues = [0, 0]

    def defaultCallBack(self, ):

        print '%s.defaultCallBack()' %(self.name)
        self.hide()
        self.buffer = ['','']

    def show(self, editableItems, okCallBack=None, escCallBack=None, ):

        self.firstClickFlag = [1,1]

        self.selectedItemIndex = 0
        self.property('on', ['0',], childName='b_val_'+str((self.selectedItemIndex+1)%2))
        self.property('on', ['1',], childName='b_val_'+str(self.selectedItemIndex))

        try :
            self.escCallBack = self.defaultCallBack
            self.okCallBack  = self.defaultCallBack
            if escCallBack != None : self.escCallBack = escCallBack
            if okCallBack  != None : self.okCallBack  = okCallBack

            self.editableItems  = editableItems

            self.oldValues = [ self.editableItems[0].value, self.editableItems[1].value ]

            i=0
            for ei in self.editableItems :
                desc = self.parentProcess.tr(ei.name) + '\n'
                if ei.sequence != None :
                    desc += '[%s,%s]'%(ei.sequence[0],ei.sequence[-1],)
                else : desc += '[%s,%s]'%(ei.min,ei.max,)
                if ei.units != None :
                    if len(self.cv( ei.units )) > 0 :
                        desc += ' (%s)'%self.cv( ei.units )

                self.buffer[i] = ei.valToString()

                flag = 0
                if self.selectedItemIndex == i : flag = 1
                #~ self.callSlot('setOn', [str(flag),], childName='b_val_'+str(i))
                self.property('on', [str(flag),], childName='b_val_'+str(i))

                self.property('text', [desc,], childName='b_val_'+str(i))
                self.property('text', [self.buffer[i], ], childName='l_val_'+str(i))

                i+=1
        except : self.parentProcess.handleExcept()

        Widget.show(self, )

    def clicked(self, b_name):

        print  "%s.clicked(%s) self.buffer:%s"%(self.name, b_name, self.buffer)

        if b_name == "b_ESC" :
            self.escCallBack()
            self.hide()
            return 1
        elif b_name == "b_OK" :

            i=0
            for ei in self.editableItems :
                try : ei.setValFromString(self.buffer[i])
                except : self.parentProcess.handleExcept()
                i+=1

            self.okCallBack()
            self.hide()
            return 1
        elif b_name == "b_HELP" :
            return 1
        #~ elif b_name == "b_RESET" :
            #~ self.show()
            #~ return 1

        if b_name[0:5] == "b_val" :
            n = int(b_name[6])
            self.selectedItemIndex = n%2
            self.property('on', ['0',], childName='b_val_'+str((self.selectedItemIndex+1)%2))
            self.property('on', ['1',], childName='b_val_'+str(self.selectedItemIndex))

            return

        if self.firstClickFlag[self.selectedItemIndex] :
            self.buffer[self.selectedItemIndex] = ''

        if b_name == "b_PM" :
            self.buffer[self.selectedItemIndex] = str(- int(self.buffer[self.selectedItemIndex]))
        elif b_name == "b_DEL" :
            if len(self.buffer[self.selectedItemIndex]) > 0 :
                self.buffer[self.selectedItemIndex] = self.buffer[self.selectedItemIndex][:-1]
            if len(self.buffer[self.selectedItemIndex]) == 0 : self.buffer[self.selectedItemIndex] = ' '
        else :
            self.buffer[self.selectedItemIndex] += b_name[2]

        self.property('text', [ self.buffer[self.selectedItemIndex] , ], childName='l_val_'+str(self.selectedItemIndex))

        self.firstClickFlag[self.selectedItemIndex] = 0

#~  ###################################
class Keyboard(Widget):

    def __init__(self, parent):

        Widget.__init__(self, node=None, parentProcess=parent)
        self.name = 'keyboard'
        self.title = ""
        self.buffer = ""
        self.okClicked = self.default_okClicked
        self.escClicked = self.default_escClicked
        self.maxLen = 32

        self.firstInputFlag = 1

        self.case = "LOW"
        self.lowerChars =   ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z']
        self.upperChars =   ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z']
        self.numericChars = ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '+', '-', '*', ':', '[', ']', '(', ')', '&', '%', '$', '"', "'", '?', '!', '@']

    def clicked(self, b_name):

        try :
            c = ""
            if   b_name == "b_ESC" :
                self.hide()
                self.escClicked()
            elif b_name == "b_OK" :
                self.buffer = self.buffer.replace('', '')
                self.hide()
                self.okClicked()
            elif b_name == "b_CAPS" :
                if   self.case  == "LOW" :
                    self.case = "UP"
                    self.property("text", ["NUM", ], childName="b_CAPS")
                elif self.case  == "UP"  :
                    self.case = "NUM"
                    self.property("text", ["LOW", ], childName="b_CAPS")
                elif self.case  == "NUM" :
                    self.case = "LOW"
                    self.property("text", ["CAPS", ], childName="b_CAPS")
                self.refresh( )
            elif b_name == "b_DEL" :
                if len(self.buffer) > 1 :
                    self.buffer = self.buffer[:-1]
                else :
                    self.buffer = ''
            elif b_name[2] == "n" : c = b_name[3:]
            elif b_name[2] == "_" : c = "_"
            elif b_name == "b_BLANK" : c = ' '
            else :
                i = int(b_name[2:])
                if   self.case  == "LOW" : c = self.lowerChars[i]
                elif self.case  == "UP"  : c = self.upperChars[i]
                elif self.case  == "NUM"  : c = self.numericChars[i]

            if self.firstInputFlag == 1 :
                if b_name != "b_DEL" :
                    self.buffer = c
                self.firstInputFlag = 0
            else : self.buffer += c

            if len(self.buffer) > self.maxLen : self.buffer = self.buffer[:self.maxLen]
            if len(self.buffer) > 1 :
                if self.buffer[0] == '' :
                    self.buffer = self.buffer[1:]
            self.property("text", [self.buffer, ], childName="l_display")

        except : self.parentProcess.handleExcept()

    def show(self, title=None, buffer=None, maxLen=32, okClicked=None):

        self.maxLen = maxLen

        if okClicked != None : self.okClicked = okClicked

        if title != None : self.title = title
        self.property("text", [self.title, ], childName="l_title")

        if buffer != None : self.buffer = buffer
        self.property("text", [self.buffer, ], childName="l_display")

        #~ print "%s.show(%s)."%(self.name, buffer)

        self.firstInputFlag = 1
        Widget.show(self, )

        self.refresh()

    def refresh(self, ):

        if   self.case  == "LOW" :
            cs = self.lowerChars
            self.property("text", ["CAPS", ], childName="b_CAPS")
        elif self.case  == "UP"  :
            cs = self.upperChars
            self.property("text", ["NUM", ], childName="b_CAPS")
        elif self.case  == "NUM" :
            cs = self.numericChars
            self.property("text", ["LOW", ], childName="b_CAPS")

        for i in range(26) :
            nm = 'b_%02d' % (i,)
            c = cs[i]
            if c == '&' : c = '&&'
            self.property("text", [c, ], childName=nm)

    def default_okClicked(self, ):
        print "%s.default_okClicked(%s)"%(self.name, self.buffer)

    def default_escClicked(self, ):
        print "%s.default_okClicked(%s)"%(self.name, self.buffer)

#~  ###################################
class Files(Widget):

    def __init__(self, parent):

        Widget.__init__(self, node=None, parentProcess=parent)
        self.name = 'files'

        self.paths = [ '..', '/mnt' ]

        self.selected = [ [], [] ]

        self.transferDirection = 0

    def clicked(self, b_name):

        print  "%s.clicked(%s)"%(self.name, b_name)

        try:
            itemName = None

            if b_name == "b_ESC" :

                self.paths = [ '..', '/mnt' ]
                self.selected = [ [], [] ]
                self.hide()

            elif b_name == "b_OPEN" :

                f = None
                if len(self.selected[0]) > 0 :
                    f = self.selected[0][0]
                elif len(self.selected[1]) > 0 :
                    f = self.selected[1][0]

                if not f :
                    msg = 'e\' necessario prima selezionare il file.<br>'
                    _ok = self.show
                    _esc = self.show
                    self.parentProcess.frames['alertdlg'].show(okClicked=_ok, escClicked=_esc, hideOk=0, hideEsc=0, msg=msg, )
                    return

                l = str( os.path.splitext(f) )
                print 'os.path.splitext(f): %s'%(l)

                if os.path.splitext(f)[1] in ('.sh', '.py') :

                    msg = 'confermi la esecuzione del file: <br>'
                    msg += '%s<br>'%f
                    msg += '<br>?'
                    ok = self.execFile
                    esc = self.viewFile
                    #~ self.parentProcess.frames['alertdlg'].show(msg, showOk=1, showEsc=1, okCallBack=ok, escCallBack=esc, )
                    self.parentProcess.frames['alertdlg'].show(okClicked=ok, escClicked=esc, hideOk=0, hideEsc=0, msg=msg, )

                #~ elif os.path.splitext(f)[1] in ('.txt', '.log') :
                else :

                    self.viewFile()

            elif b_name == "b_DEL" :

                if len(self.selected[0] + self.selected[1]) == 0 :
                    msg = 'e\' necessario selezionare prima i files da cancellare.<br>'
                    ok = self.show
                else :
                    msg = 'confermi la cancellazione dei files: <br>'
                    for f in self.selected[0] + self.selected[1]  :
                        msg += '%s<br>'%f
                    msg += '<br>?'
                    ok = self.delFiles
                esc = self.show
                #~ self.parentProcess.frames['alertdlg'].show(msg, showOk=1, showEsc=1, okCallBack=ok, escCallBack=esc, )
                self.parentProcess.frames['alertdlg'].show(okClicked=ok, escClicked=esc, hideOk=0, hideEsc=0, msg=msg, )

            elif b_name == "b_0" or b_name == "b_1" :
                self.transferDirection = int(b_name[2])
                src = os.path.abspath(self.paths[(self.transferDirection+1)%2])
                dst = os.path.abspath(self.paths[self.transferDirection])

                if len(self.selected[(self.transferDirection+1)%2]) == 0 :
                    msg = 'e\' necessario selezionare prima i files da trasferire.<br>'
                    ok = self.show
                else :
                    msg = ''
                    msg += 'confermi la copia dei files: <br>\n'
                    msg += '<small>\n'
                    for f in self.selected[(self.transferDirection+1)%2] :
                        msg += '%s<br>'%f
                    msg += '<br>da: %s'%(src, )
                    msg += '<br>a: %s'%(dst, )
                    msg += '</small>\n'
                    ok = self.copyFiles
                esc = self.show
                #~ self.parentProcess.frames['alertdlg'].show(msg, showOk=1, showEsc=1, okCallBack=ok, escCallBack=esc, )
                self.parentProcess.frames['alertdlg'].show(okClicked=ok, escClicked=esc, hideOk=0, hideEsc=0, msg=msg, )

        except : self.parentProcess.handleExcept()

    def anchorClicked(self, name, anchor, link):

        print "%s.anchorClicked(%s, %s, %s)"%(self.name, name, anchor, link)

        if name[:3] == 'tb_':

            id = int(name[3])
            if os.path.isfile(os.path.join(self.paths[id], link)) :
                if link not in self.selected[id] : self.selected[id].append(link)
                else : self.selected[id].remove(link)
            elif os.path.isdir(os.path.join(self.paths[id], link)) :
                self.paths[id] = os.path.abspath(os.path.join(self.paths[id], link))
                self.selected[id] = []

        self.refresh()

        print "%s.anchorClicked() self.selected=%s, self.paths=%s."%(self.name, self.selected, self.paths)

    def viewFile(self, ):

        if len(self.selected[0]) > 0 :
            f = self.selected[0][0]
            pth = os.path.abspath(self.paths[0])

        elif len(self.selected[1]) > 0 :
            f = self.selected[1][0]
            pth = os.path.abspath(self.paths[1])

        else : return

        src = os.path.join(pth, f)
        #~ ext = os.path.splitext(f)[1]
        #~ if ext in ('txt', 'ini', ) : tf = '0'
        #~ else : tf = '1'

        try :

            self.parentProcess.frames['textbrowser'].show('loading data...')
            #~ self.parentProcess.frames['textbrowser'].property('textFormat', [tf], childName='body' )
            self.parentProcess.frames['textbrowser'].setSrc(src)

            self.parentProcess.frames['textbrowser'].property('paletteBackgroundColor', ['#333366'], childName='body')

        except : self.parentProcess.handleExcept()

    def execFile(self, ):

        if len(self.selected[0]) > 0 :
            f = self.selected[0][0]
            pth = os.path.abspath(self.paths[0])

        elif len(self.selected[1]) > 0 :
            f = self.selected[1][0]
            pth = os.path.abspath(self.paths[1])

        else : return

        src = os.path.join(pth, f)

        if os.path.splitext(f)[1] in ('sh', )   : cmd = 'cd %s ; sh %s'%(pth, src)
        elif os.path.splitext(f)[1] in ('py', ) : cmd = 'cd %s ; python %s'%(pth, src)

        os.system(cmd)

    def delFiles(self, ):

        for i in (0, 1):
            pth = os.path.abspath(self.paths[i])
            for f in self.selected[i] :
                src = os.path.join(pth, f)
                cmd = 'rm -f %s'%(src)
                print '%s.delFiles() cmd=%s.'%(self.name, cmd)
                os.system(cmd)

        self.refresh()

    def copyFiles(self, ):

        pth = os.path.abspath(self.paths[(self.transferDirection+1)%2])
        for f in self.selected[(self.transferDirection+1)%2] :
            src = os.path.join(pth, f)
            dst = os.path.abspath(self.paths[self.transferDirection])
            cmd = 'cp -f %s %s'%(src, dst)
            print '%s.copyFiles() cmd=%s.'%(self.name, cmd)
            os.system(cmd)

        self.refresh()

    def refresh(self, ):
        for i in [1,0]:

            if not os.access(os.path.abspath(self.paths[i]), os.R_OK) :
                self.paths[i] = '/mnt'

            s = os.path.abspath(self.paths[i])
            if len(s) > 32 : s = s[-32:]
            self.property('text', [s], childName='l_%d'%i)

            buff = ''
            buff += '<html>'
            buff += '<body bgcolor="#666666">'
            buff += '<table cellspacing="2" cellpadding="4">'
            bg_cols = ['#555555', '#666666']
            n = 0
            l_files = []
            l_dirs = []
            for name in os.listdir(self.paths[i]):
                if os.path.isdir(os.path.join(self.paths[i], name )) : l_dirs.append(name)
                elif os.path.isfile(os.path.join(self.paths[i], name )) : l_files.append(name)
            l_files.sort()
            l_dirs.sort()
            for fName in ( ['..', ] + l_dirs + l_files ) :
                if os.path.isdir(os.path.join(self.paths[i], fName )) : bg_col = '#336677'
                else : bg_col = bg_cols[n%2]
                buff += ' <tr bgcolor="%s">'%(bg_col, )
                if fName in self.selected[i] : col = '#FFFF33'
                else : col = '#FFFFFF'
                ref = fName
                if fName == '..' : nm = '[cartella superiore]'
                else : nm = fName
                buff += '  <td><a href="%s"><font color="%s"><b>%s</b></font></a></td>'%(ref, col, nm, )
                buff += ' </tr>'
                n += 1
            buff += '</table>'
            buff += '</body>'
            buff += '</html>'

            self.property('text', [buff], childName='tb_%d'%i)

    def mount_usb(self, ):

        if len(os.listdir('/mnt/usb')) == 0 :
            os.system('python /opt/mount_usb.py')

        if os.access('/mnt/usb', os.F_OK) and len(os.listdir('/mnt/usb')) > 0 :
            self.paths[1] = os.path.join('/mnt/usb', os.listdir('/mnt/usb')[0])

    def umount_usb(self, ):

        if len(os.listdir('/mnt/usb')) > 0 :
            os.system('python /opt/mount_usb.py --off')

    def show(self, mount_usb=0):

        buff = 'please wait...'
        for i in [1,0]:
            self.property('text', [buff], childName='tb_%d'%i)
        Widget.show(self, )

        if mount_usb : self.mount_usb()

        self.refresh()

    def hide(self, ):

        self.umount_usb()

        Widget.hide(self, )

#~  ###################################
class Test(Widget):

    def __init__(self, parent):
        Widget.__init__(self, node=None, parentProcess=parent)
        self.name = "test"

    def clicked(self, childName):
        print "clicked: %s.%s" % (self.name, childName)
        if childName == 'b_ESC' :
            self.hide()
            self.parentProcess.exit()


def test():

    z = Zclient(host_ip=HOST_IP, host_port=HOST_PORT, name='zclient_test', run_select_timeout_sec=1.,debug=1)
    z.guiPort.open()
    z.sendMessageToTheSocket('z setDbgLevel 5')
    t = Test(z)
    z.uiPath = os.path.dirname(os.path.abspath(__file__))
    t.buildFromUiFile()
    t.show()

    interPython = InterPython(parent=z, environ=locals())
    interPython.start()

    z.run( interPython, locals(), globals() )

#~  ###################################
if __name__ == '__main__':

    test()

#~  ###################################
