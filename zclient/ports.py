#!/usr/bin/python
"""
#~ ######################################################
#~ # project: zclient
#~ # file: ports.py
#~ # author: giovanni.angeli6@gmail.com
#~ # Copyright (C) 2007-2011  giovanni angeli
#~ ######################################################
"""
import os
import sys
import time
import traceback
import socket
import select
import code
import thread
import threading
import ConfigParser
try: import serial
except : print "serial not available."

VERSION = '1.1.6 - 09/04/2009'
print '---- %s version: %s ----' %(__file__, VERSION )

POL = ENQ = '\x05'
SEL = BEL = '\x07'
RES = EOT = '\x04'
SOM = STX = '\x02'
EOM = ETX = '\x03'
AFF = ACK = '\x06'
NEG = NAK = '\x15'

def handleExcept(level=5, fd=sys.stderr):
    traceback.print_exc( level, sys.stderr )
    sys.stderr.flush()




class IniConfigurable:

    def setIniOptionValue(self, section, option, value, store=0):

        if not self.configParser.has_section(section) :
            self.configParser.add_section(section)

        self.configParser.set(section, option, value)

        if store : self.storeConfigIni()

    def getIniOptionValue(self, section='main', option=None, default=None):

        val = default

        if self.configParser.has_option(section, option) :
            val = self.configParser.get(section, option)
        else :
            if not self.configParser.has_section(section) :
                self.configParser.add_section(section)
            self.configParser.set(section, option, val)

        return val

    def storeConfigIni(self, ):

        #~ if not hasattr(self, 'fileNameAndPath' ) :
            #~ self.fileNameAndPath = os.path.join( os.getcwd(), '..', 'conf', '%s.ini'%(self.name))

        f = open(self.fileNameAndPath, 'w')
        self.storeComments(f)
        self.configParser.write(f)
        f.close()

    def storeComments(self, fd):

        if hasattr(self, 'comments') :
            for c in self.comments :
                fd.write(c)

    def loadComments(self, fd):

        fd.seek(0)
        self.comments = []
        for line in fd:
            if line[0] == '#' :
                self.comments += line

    def loadConfigIni(self, fileNameAndPath=None):

        if fileNameAndPath == None :
            fileNameAndPath = os.path.join( os.getcwd(), '..', 'conf', '%s.ini'%self.name)

        self.fileNameAndPath = fileNameAndPath

        self.configParser = ConfigParser.RawConfigParser()

        if os.path.exists(self.fileNameAndPath) :
            f = open(self.fileNameAndPath)
            self.configParser.readfp(f)
            self.loadComments(f)
            f.close()

class Pickleable:

    toBePickled = {}
    picklePath = os.path.join( os.path.dirname(os.path.abspath(__file__)) , '..', 'pkl' )

    def dump(self, itemName=None):

        #~ print "%s.dump(%s) self.toBePickled=%s" % (self.name, itemName, self.toBePickled)
        if itemName == None :
            for (k, v) in self.toBePickled.items() :
                try :
                    fname = os.path.join(self.picklePath, self.name+'_'+k)
                    f = open(fname,'w')
                    pickle.dump(v, f)
                    f.close()
                    os.system('chmod 777 '+fname)
                except : traceback.print_exc( 2, sys.stderr)
        else :
            if itemName in self.toBePickled.keys() :
                try :
                    fname = os.path.join(self.picklePath, self.name+'_'+itemName)
                    f = open(fname,'w')
                    pickle.dump(self.toBePickled[itemName], f)
                    f.close()
                    os.system('chmod 777 '+fname)
                except : traceback.print_exc( 2, sys.stderr)
            else :
                print "%s: dumping %s, not found in keys: %s" % (self.name, itemName, self.toBePickled.keys())

    def load(self, itemName=None):

        ret = 0

        #~ print "%s.load(%s) self.toBePickled=%s" % (self.name, itemName, self.toBePickled)

        if itemName == None :
            for k in self.toBePickled.keys() :
                try :
                    f = os.path.join(self.picklePath, self.name+'_'+k)
                    f = open(f,'r')
                    tmp = pickle.load(f)
                    f.close()
                    self.toBePickled[k] = tmp
                    ret = 1
                except : traceback.print_exc( 2, sys.stderr)
        else :
            if itemName in self.toBePickled.keys() :
                try :
                    f = os.path.join(self.picklePath, self.name+'_'+itemName)
                    f = open(f,'r')
                    tmp = pickle.load(f)
                    f.close()
                    self.toBePickled[itemName] = tmp
                    ret = 1
                except : traceback.print_exc( 2, sys.stderr)
            else :
                print "%s: loading %s, not found in keys: %s" % (self.name, itemName, self.toBePickled.keys())

        return ret

class Localizable:

    translations = {}
    LANGUAGE = 0
    UNITS = 0

    def cv(self, keyString):
        ret = ''

        try : self.UNITS = self.parentProcess.toBePickled['options'][1].value
        except :

            try : self.UNITS = self.toBePickled['options'][1].value
            except : self.UNITS = 0

        if type(keyString) == type(['a','b']) :
            #~ print 'cv() keyString is a sequence'
            if len(keyString) > self.UNITS : ret = '%s'%(keyString[self.UNITS])
        else : ret = str(keyString)

        return ret

    def tr(self, keyString):

        keyString = keyString.strip()
        ret = ''
        try : self.LANGUAGE = self.parentProcess.toBePickled['options'][0].value
        except :
            try : self.LANGUAGE = self.toBePickled['options'][0].value
            except : self.LANGUAGE = 0
        try :
            ret = self.translations[keyString][self.LANGUAGE]
            if len(ret) == 0 :
                try : ret = self.translations[keyString][0]
                except : ret = ''
        except :
            try : ret = self.translations[keyString][0]
            except : ret = ''
        if len(ret) == 0 : ret = keyString
        return ret

#~  ###################################
 #~ ########################################################################
class InterPython(threading.Thread):

    name = 'interpython'
    def __init__(self, parent=None, environ=globals()):
        threading.Thread.__init__(self)
        try:
            import readline, rlcompleter
            readline.parse_and_bind("set show-all-if-ambiguous on")
            readline.parse_and_bind("tab: complete")
        except :
            print "readline not available."
            pass
        self.parent = parent
        self.interp = code.InteractiveConsole(environ)
        self.runFlag = 1

        print '%s.__init__() pid=%s' % (self.name, os.getpid())

    def run(self):
        try : self.interp.interact("[%s] interPython console>>>" % self.parent)
        except : traceback.print_exc( 2, sys.stderr )

        if not self.runFlag : return

    def stop(self):
        self.runFlag = 0
        self._Thread__stop()
        cmd = 'kill -9 %s'%(os.getpid())
        print '%s.stop() cmd=%s' % (self.name, cmd)
        os.system(cmd)

 #~ ########################################################################
class zProcess(Localizable, Pickleable, IniConfigurable):
    DEBUG = 0
    ports = []
    children_idles = []
    run_select_timeout_sec = 1.0
    interactive = 0
    def __init__(self, name, run_select_timeout_sec, debug=0):
        self.DEBUG = debug
        self.name = name
        self.run_select_timeout_sec = run_select_timeout_sec

    def idle(self,):
        ret = 0
        r = [] ; w = [] ; e = []
        for p in self.ports :
            if (self.DEBUG & 3) == 3 : print '%s.idle() p.fileno()->%s'%(self.name, p.fileno())
            if p.fileno() != None and p.fileno() > 0 :
                r.append( p.fileno() )
        try:
            (r, w, e) = select.select(r, w, e, self.run_select_timeout_sec)
            if len(r) == 0 :
                if (self.DEBUG & 3) == 3 : print "run() select TIMEOUTED! (%s)"%(self.run_select_timeout_sec)
                ret = 1
            else :
                for p in self.ports :

                    if (self.DEBUG & 3) == 3 : print '%s.idle() r=%s'%(self.name, r)

                    if p.fileno() in r :
                        if (self.DEBUG & 3) == 3 : print 'calling: %s.read()'%( p.name)
                        p.read()
        except :
            self.handleExcept()
            if sys.exc_type == KeyboardInterrupt :
                self.runFlag = 0

        for idl in self.children_idles : idl()

        return ret

    def run(self, locals=locals(), globals=globals()):

        import getopt

        print 'sys.argv = %s'%(sys.argv)
        optlist, args = getopt.getopt(sys.argv[1:], 'id:')
        DBG, INTERACTIVE = (0, 0)
        for o, a in optlist:
            if o == "-i": INTERACTIVE = 1
            elif o in ("-d", "--dbg"): DBG = int(a)
        print '%s.DBG=%s, %s.INTERACTIVE=%s.'%(self.name, DBG, self.name, INTERACTIVE)

        self.DEBUG = int(DBG)
        self.interactive = INTERACTIVE

        self.locals = locals
        self.globals = globals
        self.console = None
        if self.interactive :
            self.console = InterPython(parent=None, environ=locals)
            self.console.start()

        self.runFlag = 1

        if self.DEBUG & 4 : print '%s.run() '%(self.name, )

        while (self.runFlag) :

            self.idle()

        self.exit()

    def exit( self, ):
        try :
            if self.console :
                self.console.stop()

        except : self.handleExcept()
        for p in self.ports :
            if self.DEBUG : print 'closing: %s'%(p.name)
            try : p.close()
            except : self.handleExcept()

        self.runFlag = 0

    def handleExcept(self, level=2, fd=sys.stderr):
        sep = '*'*10
        fd.write(sep+"< ")
        traceback.print_exc( level, fd )
        fd.write(sep+">\n\r")
        fd.flush()

        if sys.exc_type == KeyboardInterrupt :
            self.runFlag = 0


class zPort:
    DEBUG = 0
    inPackTerminator = ';'
    outPackTerminator = ';'
    deviceName = ''
    def __init__(self, parent, name='zport', deviceName='', ):

        self.parent = parent
        self.name = name
        self.deviceName = deviceName

        self.mode = ''
        self.data_buff = ''
        self.packet = ''
        self.device = None
        self.DATA_BUFF_LEN = 256

    def fileno(self,):
        try : return self.device.fileno()
        except :
            self.parent.handleExcept()
            return -1

    def open(self, mode='rw' ):
        self.mode = mode
        try :
            self.device = open(self.deviceName, mode)
            if self not in self.parent.ports : self.parent.ports.append(self)
        except : self.parent.handleExcept()

    def close(self,):
        try : self.device.close()
        except : self.parent.handleExcept()

        try : self.parent.ports.remove(self)
        except : self.parent.handleExcept()

    def write(self, data):
        try:
            self.device.write(data + self.outPackTerminator)
            self.device.flush()
        except:
            print '%s.write() self.mode=%s.'%(self.name, self.mode)
            self.parent.handleExcept()

    def read(self,):
        if self.DEBUG & 8 : print '%s.read() 0 self.inPackTerminator=%s.'%(self.name, self.inPackTerminator)
        try:
            data = self.device.read(self.DATA_BUFF_LEN)
            if self.DEBUG & 8 : print '%s.read() data(%d)=<%s>'%(self.name, len(data), data )
            if(len(data) > 0) : self.bufferizeInput(data)
        except : self.parent.handleExcept()
        if self.DEBUG & 8 : print '%s.read() 1'%(self.name, )

    def bufferizeInput(self, data):
        self.data_buff += data
        while(1):
            i = self.data_buff.find(self.inPackTerminator)
            if i >= 0 :
                cmd = self.data_buff [:i]
                self.data_buff = self.data_buff [i+1:]
                if len(cmd) > 0 : self.handlePacket(cmd)
            else : break

    def handlePacket(self, cmd):
        try :  eval(cmd, self.parent.locals, self.parent.globals )
        except :
            print "\r\ncmd(%d):<%s>"%(len(cmd), cmd)
            #~ print 'self.parent.locals=%s, self.parent.globals=%s'%(self.parent.locals, self.parent.globals )
            self.parent.handleExcept()

 #~ ########################################################################
class FilePort(zPort):

    def __init__(self, parent, name, deviceName):

        zPort.__init__(self, parent, name, deviceName, )

    def read(self, ):
        if 'r' in self.mode :
            try :
                l = self.device.readline()
                if len(l) > 0 :
                    self.handlePacket(l)
            except: self.parent.handleExcept()

    def handlePacket(self, pack):

        print '%s.handlePacket(%s)'%(self.name, pack)

 #~ ########################################################################
class IPCport(zPort):
    outPackTerminator = '\n'
    def __init__(self, parent, host=None, name='ipcclient', family=socket.AF_INET):

        zPort.__init__(self, parent, name)

        self.host = host
        self.SOCK_DATA_LEN = 512

        self.device = None

        self.family = family

    def open( self ):
        flag = 1
        l = (self.name, self.host[0], self.host[1] )
        while(flag) :
            time.sleep(0.5)
            try :
                print '%s trying to connect to %s:%d...' % (l)
                self.device = socket.socket(self.family, socket.SOCK_STREAM)
                #~ print '%s.device: %s.' % ( self.name, self.device)
                self.device.connect( self.host )
                print '%s connected to %s:%d.' % (l)
                flag = 0

                if self not in self.parent.ports : self.parent.ports.append(self)

            except :
                del self.device
                self.parent.handleExcept()

    def read(self,):
        try:
            data = self.device.recv(self.SOCK_DATA_LEN)
            if self.DEBUG & 4 : print '%s.read() data(%d)=<%s>'%(self.name, len(data), data )
            if(len(data) > 0) : self.bufferizeInput(data)
            else : self.close()
        except: self.parent.handleExcept()

    def write(self, data):
        try: self.device.sendall(data + self.outPackTerminator )
        except: self.parent.handleExcept()

    def close(self):
        print '%s.close()'%(self.name, )

        try : self.parent.ports.remove(self)
        except : self.parent.handleExcept()

        if self.device :
            flag = None
            try : flag = self.device.SHUT_WR
            except : flag = 2
            self.device.shutdown(flag)
            self.device.close()
            del self.device

class IPCServer(zProcess):

    allow_reuse_address = 1

    def __init__(self, HOST='', PORT=4411, name=None, run_select_timeout_sec=1.0, debug=0, family=socket.AF_INET):

        if name == None : name = 'ipcserver@[%s:%s]'%(HOST, PORT)

        zProcess.__init__(self, name, run_select_timeout_sec, debug)

        self.host = (HOST, PORT)
        self.serverSock = None
        self.addr = None

        self.ipcport = IPCport(self, )
        self.ipcport.DEBUG = self.DEBUG

        self.family = family

    def waitForConnection(self,):

        print '%s.waitForConnection() on %s' %(self.name, self.host)
        try :
            self.serverSock = socket.socket(socket.AF_INET, self.family)
            if self.allow_reuse_address: self.serverSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.serverSock.bind(self.host)
            self.serverSock.listen(2)
            (self.ipcport.device, self.addr) = self.serverSock.accept()
            print 'Connection from: %s' %(self.addr, )
            print 'self.ipcport.device: %s' %(self.ipcport.device, )

            #~ if self.ipcport not in self.ports : self.ports.append(self.ipcport)
            if self.ipcport not in self.ports : self.ports.insert(0, self.ipcport)

        except : self.handleExcept()

    def run(self, locals, globals, delay=2):

        self.waitForConnection()

        time.sleep(delay)

        if locals == None or globals == None : zProcess.run(self, )
        else : zProcess.run(self, locals, globals)

 #~ ########################################################################
class SerialPort(zPort):
    def __init__(self, parent=None, name='serialport', deviceName="/dev/ttyS0", params=None, ):

        zPort.__init__(self, parent, name,)

        self.deviceName = deviceName

        self.params = {
            "baudrate"    : 38400 ,
            "bitesize"    : 7,      #number of databits
            "parity"      : 'E',    #enable parity checking
            "stopbits"    : 1,      #number of stopbits
            "timeout"     : None,   #set a timeout value, None to wait forever
            "xonxoff"     : 0,      #enable software flow control
            "rtscts"      : 0,      #enable RTS/CTS flow control
            "writeTimeout": None,   #set a timeout for writes
            "dsrdtr"      : None,   #None: use rtscts setting, dsrdtr override if true or false
        }

        if params != None :
            try :
                for pk in self.params.keys()  :
                    if pk in params.keys() :
                        self.params[pk] = params[pk]
            except : handleExcept()

    def open(self, ):

        try :
            self.device = serial.Serial( self.deviceName ,
                baudrate       = self.params["baudrate" ],
                bytesize       = self.params["bitesize" ],
                parity         = self.params["parity"   ],
                stopbits       = self.params["stopbits" ],
                timeout        = self.params["timeout"  ],
                xonxoff        = self.params["xonxoff"  ],
                rtscts         = self.params["rtscts"   ],
                writeTimeout   = self.params["writeTimeout"],
                dsrdtr         = self.params["dsrdtr"   ] )

            self.device.open()
            if self not in self.parent.ports : self.parent.ports.append(self)
        except : handleExcept()

        if self.DEBUG :
            msgs = str(self.device).split(',')
            for i in msgs :
                print '\t' , i
            print 'self.device.fileno()=%s'%(self.device.fileno())

 #~ ########################################################################
class serial_port:

    DEBUG = 0

    startOfpack = None
    endOfpack   = None
    maxPackLength = 200
    packLength = 200

    callBack = None

    logFile = None

    name='noname'
    packCntr = 0
    serial_data = ""

    _serial_status_IDLE = 0
    _serial_status_BUFFERING = 1

    def __init__(self, parent=None, device_name="/dev/ttyS0", params=None, ):

        self.callBack = self.defaultCallBack
        self._serial_status = self._serial_status_IDLE

        self.deviceName = device_name
        self.params = {
            "baudrate"    : 38400 ,
            "bitesize"    : 7,      #number of databits
            "parity"      : 'E',    #enable parity checking
            "stopbits"    : 1,      #number of stopbits
            "timeout"     : None,   #set a timeout value, None to wait forever
            "xonxoff"     : 0,      #enable software flow control
            "rtscts"      : 0,      #enable RTS/CTS flow control
            "writeTimeout": None,   #set a timeout for writes
            "dsrdtr"      : None,   #None: use rtscts setting, dsrdtr override if true or false
        }

        if params :
            try :
                for pk in self.params.keys()  :
                    if pk in params.keys() :
                        self.params[pk] = params[pk]
            except : handleExcept()

        self.serialPort = None
        self.fd = -1

    def open(self):

        self.serialPort = serial.Serial( self.deviceName ,
            baudrate       = self.params["baudrate" ],
            bytesize       = self.params["bitesize" ],
            parity         = self.params["parity"   ],
            stopbits       = self.params["stopbits" ],
            timeout        = self.params["timeout"  ],
            xonxoff        = self.params["xonxoff"  ],
            rtscts         = self.params["rtscts"   ],
            writeTimeout   = self.params["writeTimeout"],
            dsrdtr         = self.params["dsrdtr"   ] )

        self.serialPort.open()

        if self not in self.parent.ports : self.parent.ports.append(self)

        if self.DEBUG :
            msgs = str(self.serialPort).split(',')
            for i in msgs :
                print '\t' , i
            print 'self.serialPort.fileno()=%s'%(self.serialPort.fileno())

        self.fd = self.serialPort.fileno()
        self.packCntr = 0

    def read(self, size=1):

        if not self.serialPort :
            raise "self.serialPort = None"
            return

        r = self.serialPort.read(size)

        for c in r : self.handleInputChar(c)

        return r

    def write(self, data):

        if self.serialPort :

            try:
                self.serialPort.write(data)
                self.serialPort.flush()
                #~ if self.DEBUG & 8 : print '%s.write(%s)'%(self.name, data)
            except : self.handleExcept()

    def close(self):

        self.fd = -1
        try :
            if self.serialPort :

                self.serialPort.close()
        except : pass

        del self.serialPort
        self.serialPort = None

        try :
            self.printToLogFile("CLOSE-----------------------------------\r\n")
            self.logFile.flush()
            os.system("sync")

            #~ del self.logFile
            #~ self.logFile = None
        except :
            pass


    def reset(self, deviceName=None, params={}):

        if deviceName : self.deviceName = deviceName
        if params != {} : self.params = params

        self.close()
        time.sleep(1)

        self.open()

        try :
            self.printToLogFile("RESET-----------------------------------\r\n")
            self.logFile.flush()
            os.system("sync")

            #~ del self.logFile
            #~ self.logFile = None
        except :
            pass

    def printToLogFile( self,  str ):

        if self.logFile :
            if not self.logFile.closed :
                #~ self.logFile.write( str + "\r\n" )
                self.logFile.write( str  )
                self.logFile.flush()

    def handleInputChar(self, c):

        if self.DEBUG & 2 :
            print "%s.handleInputChar() c=%s[%s], self._serial_status=%s" % ( self, c , hex(ord(c)), self._serial_status )

        self.serial_data += c

        if len(self.serial_data) > self.maxPackLength :
            msg = 'len(self.serial_data)=%d, self.maxPackLengt=%d'%(len(self.serial_data), self.maxPackLength)
            self.serial_data = ""
            self._serial_status = self._serial_status_IDLE
            raise msg

        if self._serial_status == self._serial_status_IDLE :
            if c == self.startOfpack :
                self.serial_data = c
                self._serial_status = self._serial_status_BUFFERING
            else :
                msg = '%s.handleInputChar() _serial_status=%d, c=%s'%(self.name, self._serial_status, c)
                self.serial_data = ""
                self._serial_status = self._serial_status_IDLE
                raise msg

        elif (self._serial_status == self._serial_status_BUFFERING ) :
            if(c == self.endOfpack) or len(self.serial_data) >= self.packLength :
                try : self.callBack()
                except : handleExcept()
                self.serial_data = ""
                self._serial_status = self._serial_status_IDLE

    def defaultCallBack(self, ):

        print "%s.defaultCallBack(%s)." % ( self, self.serial_data )

    def fileno(self, ):
        return self.serialPort.fileno()
    def handleExcept(self, level=2, fd=sys.stderr):
        sep = '*'*10
        fd.write(sep+"< ")
        traceback.print_exc( level, fd )
        fd.write(sep+">\n\r")
        fd.flush()

        if sys.exc_type == KeyboardInterrupt :
            self.runFlag = 0


class threadedSerialPort(threading.Thread, serial_port):

    def __init__(self, device_name="/dev/ttyS0", params=None, ):

        serial_port.__init__(self, None, device_name, params )
        self.serial_runFlag = 0
        self.lock = threading.Lock()
        threading.Thread.__init__(self)

    def run(self):
        self.open()
        self.serial_runFlag = 1
        while( self.serial_runFlag ):
            c = self.read(1)
            self.handleInputChar(c)
        self.close()
        raise

    def terminate(self):
        self.lock.acquire()
        self.serial_runFlag = 0
        self.lock.release()

 #~ ########################################################################
def timerCallBack():

    #~ msg = 'timer expired. t=%s' % (time.time())
    msg = 't %s' % (str(time.time())[-6:-1])
    print msg

def test00():

    DEV_NAME = "/dev/ttyAM2"
    THREADED = 1
    print "THREADED: %s" %( THREADED )
    print "test00() my PID is: %s" %( os.getpid() )

    ps = {
        "baudrate"    : 115200 ,
        "bitesize"    : 8,      #number of databits
        "parity"      : 'N',    #enable parity checking
        "stopbits"    : 1,      #number of stopbits
        "timeout"     : None,   #set a timeout value, None to wait forever
        "xonxoff"     : 0,      #enable software flow control
        "rtscts"      : 0,      #enable RTS/CTS flow control
        "writeTimeout": None,   #set a timeout for writes
        "dsrdtr"      : None,   #None: use rtscts setting, dsrdtr override if true or false
    }

    if(THREADED) :
        p = threadedSerialPort( device_name=DEV_NAME, params = ps )
        p.DEBUG = 1
        p.startOfpack = '*'
        p.endOfpack = '\r'
        p.callBack = timerCallBack
        p.start()
    else :
        p = serial_port( parent=None, device_name=DEV_NAME, params = ps )
        p.DEBUG = 1
        p.open()
    try:
        import readline, rlcompleter
        readline.parse_and_bind("set show-all-if-ambiguous on")
        readline.parse_and_bind("tab: complete")
    except : pass
    while(1):
        try :

            s = raw_input('-->')
            if s == 'quit' or s == 'q' : break
            elif s == 'switch' or s == 's' :
                t0 = time.time()
                i=0
                while(i<10000) :
                    p.write('*\r')
                    time.sleep(0.01)
                    i += 1
                t1 = time.time()
                print 't1-t0=%f sec'%(t1-t0)
            else : p.write(s+"\r")
        except : handleExcept()
    if(THREADED) : p._Thread__stop()

def test01():

    DEV_NAME = "/dev/ttyAM2"
    print "test01() my PID is: %s" %( os.getpid() )

    ps = {
        "baudrate"    : 115200 ,
        "bitesize"    : 8,      #number of databits
        "parity"      : 'N',    #enable parity checking
        "stopbits"    : 1,      #number of stopbits
        "timeout"     : None,   #set a timeout value, None to wait forever
        "xonxoff"     : 0,      #enable software flow control
        "rtscts"      : 0,      #enable RTS/CTS flow control
        "writeTimeout": None,   #set a timeout for writes
        "dsrdtr"      : None,   #None: use rtscts setting, dsrdtr override if true or false
    }

    p = serial_port( parent=None, device_name=DEV_NAME, params = ps )
    p.DEBUG = 1
    p.open()

    count = 0
    while(1):
        r = p.read(1)
        p.write(r)
        print '[%d](%8.5f)%s'%(count,time.time(),r)
        count += 1

def main():

    print 'sys.argv: %s'%(sys.argv)
    if len(sys.argv) > 1 :
        exec(sys.argv[1]+'()')
    else :
        test01()


 #~ ########################################################################
if __name__ == '__main__':

    main()

 #~ EOF
