'''
 test script for app server
 result used by test.appserver.php
 @swreq       SREQ:0001146: g > appserver test 
 @package     test.py
 @author      Steffen Kube
 @Copyright   (c) Alere Technologies GmbH 2013     
'''

import sys

class test_python:

    def __init__ (self , filename , outputfile =None , options = None ):
        self.filename   = filename
        self.outputfile = outputfile
        self.options    = options

 
    '''
    @return dictanswer
        
    '''
    def start ( self ):
        pyVers  = sys.version_info
        pythonVerShort = str(pyVers[0]) + '.' + str(pyVers[1]) + '.' + str(pyVers[2])
        
        return { 'filename':self.filename, 'options': self.options, 'python.version': pythonVerShort } # dictionary
        