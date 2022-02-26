# this is the test
from gozilla_jsonrpc import gozilla_jsonrpc
import time
import json 


if __name__ == "__main__":

    import traceback


    try:


        """
        # for Blinkone-Toolbox:
        #
        fp = r"C:\dev\python\blink\app\dabbox\config"
        fn = r"gozilla.yaml"
        fname  = os.path.join(fp, fn)
        stream = open(fname, 'r')
        content = yaml.load(stream)
        stream.close()

        auth = content['blocks']['goz']['param']['connect']
        auth['dbid'] = content['blocks']['goz']['param']['database']
        auth["password"] =  base64.b64decode(auth["pass_b64"])
        """

        auth = {
            "username":"test01",
            "password":"1234"
            }

        goz_config = {
           # "url": "https://jenbliapp02.blink.lan/api",
           "url": "http://jenblipart04.blink.lan/pionir/api",
           "dbid":'blk',
           "ssl_verify": 0
        }

        m = gozilla_jsonrpc( goz_config )
        print ("DEBUG: ssl_verify:"+str(m.ssl_verify))
        m.login(auth)

        answer = m.call('gGetVersion',None)
        print ('gGetVersion: answer: ')
        print (answer)
        
        
        args_use = {
            't':'CHIP_READER',
            'cols': [
              'x.NAME',
              'x.SERIAL',
              'x.STATUS_MX',                        
              'a.CREA_DATE'
              ],
             'filter':{
                  'cols': [
                      {
                      'col':'x.A_CHIP_READER_ID', 
                      'con':'=',
                      'val':'38', 
                      'bool':'AND'
                      }
                   ],
                 },
             'sort':'x.NAME',
             'pagesize':30
        }
        answer = m.call('DEF/gObj_list',args_use)
        print ('DEF/gObj_list answer: '+str(answer) )

        
        unixStamp =  time.time() # unix time stamp
        nowx = time.localtime()
        nowFormat = "%04u-%02u-%02u %02u:%02u:%02u" % (nowx[0], nowx[1],nowx[2],nowx[3],nowx[4],nowx[5])
        
        dev_name ='FLST-0001'
        date0    = nowFormat;
        args_use = {
            'serial': dev_name, 
            'service_plan_name':'Flunder Qualification',
            'date' : date0,        
        }
        answer = m.call('LAB/oDEV_log_new',args_use)
        print ('oDEV_log_new: answer: '+str(answer) )
        
        pos = answer
        datei = date0;
        args_use = {
            'log_pos': pos,
            'inspection_type': 'thermal-contact', 
            'idate'      : datei,
            'serial'     : dev_name, 
            'data_path'  : 'Z:\Data\2021\hello',
            'qc_version' : 'git1.001', 
            'istatus'    : 1,      
        }
        answer = m.call('LAB/oDEV_log_inspect_set',args_use)
        print ('oDEV_log_inspect_set: answer: '+str(answer) )
        
        datei = date0;
        args_use = {
            'log_pos': pos,
            'inspection_type': 'thermal-ir', 
            'idate'      : datei,
            'serial'     : dev_name, 
            'data_path'  : 'Z:\Data\2021\hello33',
            'qc_version' : 'git1.203', 
            'istatus'    : 0,      
        }
        answer = m.call('LAB/oDEV_log_inspect_set',args_use)
        print ('oDEV_log_inspect_set: answer: '+str(answer) )
        
        
        nowx = time.localtime()
        datei = "%04u-%02u-%02u %02u:%02u:%02u" % (nowx[0], nowx[1],nowx[2],nowx[3],nowx[4],nowx[5])
        
        args_use = {
            'log_pos': pos,
            'serial'     : dev_name, 
            'date_end'  : datei,
            'notes' :'hello',
            'status':'finished'     
        }
        answer = m.call('LAB/oDEV_log_mod',args_use)
        print ('oDEV_log_mod: answer: '+str(answer) )
        
        args_use = {
            'serial'     : dev_name, 
            'service_plan_name': 'Flunder Qualification',
    	    'result_type':'last_entry'   
        }
        answer = m.call('LAB/oDEV_log_query',args_use)
        print ('oDEV_log_query: answer: '+str(answer) )
        

        '''
        argu = { 'err': {'num': 34444, 'text': 'HALLO'}}
        answer = m.call('errout',argu)
        print ('errout: answer: ')
        print (answer)
        '''

        '''
        argu = { 't':'EXP', 'id':10, 'cols':('NAME', 'NOTES') }
        answer = m.call('DEF/ad_method',argu)
        print ('BAD_METHOD: answer: ')
        print (answer)


        argu = { 't':'EXP', 'id':10, 'cols':('NAME', 'NOTES') }
        answer = m.call('DEF/gObj_getParams',argu)
        print ('answer: ')
        print (answer)
        '''

        '''
        argu = {
            'args': {
                'vals': { 'NAME': 'test EXP 02', 'EXT_ID':'test nc681XX', 'NOTES': 'unittest notes',   }
                },
            'json': {
                'name': 'name from json',
                'id':'jkdjdjdjdaaa_9',
                'run': {
                    'start':'2019-06-04 13:45:23.334'
                    },
            },
            'projid': 100
        }
        answer = m.call('LAB/oEXP_create',argu)
        print ('oEXP_create answer: ')
        print (answer)
        '''



    except Exception:
        traceback.print_exc()
        input( "Press ENTER" )