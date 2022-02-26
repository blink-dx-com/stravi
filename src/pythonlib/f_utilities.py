# -*- coding: utf-8 -*-
__docformat__ = "restructuredtext en"

"""
main utilslike error class
File:           f_utilities.py
Copyright:      Blink AG   
Author:         Steffen Kube <steffen@blink-dx.com>
"""
import sys, traceback

class debug:
    
    def printx(funcname, text, min_debug_lev=0):
        '''
        static method
        '''
        print('DEBUG_OUT: ' +  funcname + ': ' + text )

class ErrorStack:
    errstack=[] # error stack for many errors
    
    @classmethod
    def add(cls, errnum, errtext, errkey, exc_stack):
        cls.errstack.append( {'num':errnum, 'text':errtext, 'key':errkey, 'stack':exc_stack } )
        
    @classmethod
    def reset(cls):  
        cls.errstack = []
        
    @classmethod
    def last_error_stack_method(cls):
        '''
        :return: string
        '''
        if not len(cls.errstack): 
            return ''
        
        last_error = cls.errstack[-1]
        err_stack = last_error['stack']
        last_elem = err_stack[-1]
        
        filename, lineno, funname, line = last_elem
        
        return funname

class BlinkError(Exception):
    
    errtext=''
    errnum =0
    
    def __str__(self):
        return self.errtext
    
    
    def __init__(self, errnum, errtext, errkey=''):
        '''
        err-stack:
        0: highest-modules
        ..
        x: the module where the error occured
        '''
        
        self.errnum  = errnum
        self.errtext = errtext
        
        super().__init__(self, errtext )

        
        err_stack = traceback.extract_stack() # format_stack() 
        del err_stack[-1] # delete call to this __init__-method 
        
        if errkey=='':
            # get last call-entry
            last_elem  = err_stack[-1]
            filename, lineno, funname, line = last_elem
            
            name_arr  = filename.split('/')
            shortname = name_arr[-1]
            errkey     = shortname+':'+funname
            
        self.errkey  = errkey        
        
        ErrorStack.add( errnum, errtext, errkey, err_stack )

    

