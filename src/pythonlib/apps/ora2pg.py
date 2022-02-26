# -*- coding: utf-8 -*-
"""
WINDOWS:
  set PYTHONPATH=...../pythonlib
  r'C:/Users/Steffen/Documents/Code/Gozilla/src'
LINUX:
  export PYTHONPATH=/opt/magasin/
  
  pip3  install psycopg2

"""

import os

from pythonlib.db_pg_abs import pg_database
from pythonlib.f_utilities import debug
from pythonlib.f_utilities import BlinkError


class TextUtils:
    
    def find_text(text_src_arr, pattern_list):
        '''
        :param pattern_list: list of string
        '''
        
        pattern0 = pattern_list[0]
        pattern0_len=len(pattern0)
                
        found_level   = -1
        found_init    = 0
        found_line_id = -1
        found_arr = [-1,-1]
        
        line_id = 0
        for line in text_src_arr:
            
            if found_level<0: 
                if line.find(pattern0)>=0:
                    found_init=1
                    
            if found_level>=0 or found_init:
                pattern_index=0
                for pattern in pattern_list:
                    # look for next level
                    if (found_level+1)==pattern_index:
                       
                        if line.find(pattern)>=0:
                            found_arr[pattern_index]=line_id
                            found_level=found_level+1
                            break
                        
                    pattern_index = pattern_index + 1
                    
            if (found_level+1)==len(pattern_list):
                found_line_id=line_id
                break
                    
                
            line_id = line_id + 1   
            
        return found_arr   
    
    def find_text_simple(text_src_arr, pattern):
        
        found_line_id=-1
        line_id = 0
        for line in text_src_arr:
           
            if line.find(pattern)>=0:
                found_line_id=line_id
                break

            line_id = line_id + 1   
    
        return found_line_id           
        
    
    def insert_text(textori, after_pos, text):
        '''
        :param after_pos: POS => put text after this pos
        abx.dfg
        '''
        tmp_text = textori
        new_text = tmp_text[0:after_pos+1] + text +  tmp_text[after_pos+1:]
        return new_text
    
    def replace_text(ori_line, search, newtext):

        new_text = ori_line.replace(search, newtext)
        return new_text
        
        
    def remove_lines(text_src_arr, pos1, pos2):
        
        i = pos1
        while i<=pos2:
            text_src_arr[i]=''
            i=i+1
        

class Ora2pg:
    
    def __init__(self, basedir, outdir):
        
        self.basedir=basedir
        self.outdir =outdir
        
        self.files = {
        'INSERT':    'INSERT_output.steffen.sql',
        'PROCEDURE': 'PROCEDURE_output.steffen.sql',
        'SEQUENCE':  'SEQUENCE_output.steffen.sql',
        'TABLE':     'TABLE_output.steffen.sql',
        'TRIGGER':   'TRIGGER_output.steffen.sql',
        'VIEW':      'VIEW_output.steffen.sql',
        'pk_tables': 'pk_tables.txt',
        'post_insert': 'post_insert.sql'
        }
        
        self.text_arr_g={}
        self.text_arr_g['TABLE']   = self._file_to_arr('TABLE')
        self.text_arr_g['TRIGGER'] = self._file_to_arr('TRIGGER')
        self.text_arr_g['VIEW'] = self._file_to_arr('VIEW')
        self.text_arr_g['SEQUENCE'] = self._file_to_arr('SEQUENCE')
        
        # self.text_help = TextUtils()
        
    def _file_to_arr(self, file_key):
        
        filename_seq = os.path.join( self.basedir, self.files[file_key] )
        text_arr=[]
        
        # read table-names
        fo = open(filename_seq, 'r')
        for line in fo:
            text_arr.append(line)
        fo.close()  
        
        return text_arr
        
    def _get_pk_tables(self):
        '''
        get PK-tables
        '''
        
        filename_seq = os.path.join(self.basedir,self.files['pk_tables'])
        tables=[]
        
        # read table-names
        fo = open(filename_seq, 'r')
        for line in fo:
            line=line.strip()
            table = line.lower()
            tables.append(table)
                
        fo.close()  
        
        return tables
    
    def _do_one_sequence(self, table):
        '''
        SEQUENCE + TRIGGER
        
        abstract_proto_id bigint NOT NULL,
        =>
        'CREATE TABLE abstract_proto' ,
        'abstract_proto_id bigint NOT NULL DEFAULT nextval('table_name_id_seq')' ,
        '''
        
        TABLE_text_arr = self.text_arr_g['TABLE']
        
        converter = {
            'h_wiid': {'pkname':'wiid'},
            'module': {'pkname':'mxid'},
        }
        
        pk_name = table + '_id'
        
        if table in converter:
            pk_name = converter[table]['pkname']
            
        
        pattern_list= [
            'CREATE TABLE '+ table ,
            "\t" + pk_name + ' bigint' ,            
        ]
        
        found_arr = TextUtils.find_text( TABLE_text_arr, pattern_list )
        found_pos = found_arr[1]
        
        if found_pos<0:
            raise BlinkError(1,'Pattern for table "'+table+'" not found')
        
        # print("DEB:"+table+' :'+ str(found_pos))
        
        # replace ...
        pattern_line = TABLE_text_arr[found_pos]
        
        komma_pos = pattern_line.find(',')
        
        sequ_text = " DEFAULT nextval('"+table+"_seq')"
        
        new_text = TextUtils.insert_text(pattern_line, komma_pos-1, sequ_text )
        TABLE_text_arr[found_pos] = new_text
        
        # TRIGGER
        TRIGGER_text_arr = self.text_arr_g['TRIGGER']
        
        pattern_list= [
            'DROP TRIGGER IF EXISTS '+table+'_aic',
            'EXECUTE PROCEDURE trigger_fct_'+table+'_aic()'
        ]        
        found_arr = TextUtils.find_text( TRIGGER_text_arr, pattern_list )
        
        found_pos1 = found_arr[1]
        if found_pos1<0:
            raise BlinkError(2,'Trigger-Pattern for table "'+table+'" not found.')
        
        # remove code
        TextUtils.remove_lines( TRIGGER_text_arr, found_arr[0], found_arr[1] )
            
    def save_output(self):
    
        keys = [
           
            'TABLE',
            'TRIGGER',
            'VIEW',
            'SEQUENCE'
        ]
        
        for key in keys:
            
            filename_seq = os.path.join( self.outdir, self.files[key] )
            print('- OUT: '+ filename_seq)
            
            # read table-names
            fp = open(filename_seq, 'w')
            
            tmp_text_arr = self.text_arr_g[key]
            for line in tmp_text_arr:
                fp.write( line )   
            fp.close()              
          
     
    def open(self, connection_parameters):
        '''
        open connection
        :param connection_parameters: 
           dbname='magasin', user='blinkapp', host='localhost', password=''
          '''
        
        self.db_obj = pg_database()
        self.db_obj.open(connection_parameters)
    
    def test1(self):
        
        sql_cmd = "* from exp order by exp_id"
        self.db_obj.select_dict(sql_cmd)
       
        self.db_obj.ReadRow()
        data=self.db_obj.RowData   
        debug.printx('haöllo', 'DATA:'+str(data))
        
    def _TABLE_data_types(self):
        '''
        replace DATA_TYPEs 
        '''
        text_arr = self.text_arr_g['TABLE']
         
        pattern='numeric(38)'
        newtext='integer'
        
        found_pattern=1
        while found_pattern:
    
            line_id = TextUtils.find_text_simple( text_arr, pattern )
            if line_id<0:
                break
    
            text_arr[line_id] = TextUtils.replace_text( text_arr[line_id], pattern, newtext)     
        
    def do_table_basis(self):
        
        TABLE_text_arr = self.text_arr_g['TABLE']
        
        
        pattern_list= [
            'SET search_path ='         
        ]
        found_arr = TextUtils.find_text( TABLE_text_arr, pattern_list )
        found_pos = found_arr[0]       
        if found_pos<0:
            raise BlinkError(1,'Pattern "'+str(pattern_list)+'" not found.')
        TextUtils.remove_lines( TABLE_text_arr, found_arr[0], found_arr[0] )
        
        # name of constarint was double, make it uniwue ...
        pattern_list= [
            'ALTER TABLE user_right ADD CONSTRAINT ak_user_right'         
        ]    
        found_arr = TextUtils.find_text( TABLE_text_arr, pattern_list )
        found_pos = found_arr[0]       
        if found_pos<0:
            raise BlinkError(2,'Pattern "'+str(pattern_list)+'" not found.')
        ori_line = TABLE_text_arr[found_pos]
        new_text = TextUtils.replace_text(ori_line, 'ak_user_right', 'ak2_user_right')
        TABLE_text_arr[found_pos] = new_text
        
        # CONSTRAINT bcbatch_h_soc PRIMARY KEY
        pattern_list= [
            'CONSTRAINT bcbatch_h_soc PRIMARY KEY'         
        ]    
        found_arr = TextUtils.find_text( TABLE_text_arr, pattern_list )
        found_pos = found_arr[0]       
        if found_pos<0:
            raise BlinkError(3,'Pattern "'+str(pattern_list)+'" not found.')
        ori_line=TABLE_text_arr[found_pos]
        new_text = TextUtils.replace_text(ori_line, 'bcbatch_h_soc', 'pk_bcbatch_h_soc')
        TABLE_text_arr[found_pos] = new_text   
        
        self._TABLE_data_types()
        
    
    def do_trigger_basis(self):
        
        text_arr = self.text_arr_g['TRIGGER']
        
        pattern_list= [
            'SET search_path ='         
        ]
        found_arr = TextUtils.find_text( text_arr, pattern_list )
        found_pos = found_arr[0]       
        if found_pos<0:
            raise BlinkError(1,'Pattern "'+str(pattern_list)+'" not found.')
        TextUtils.remove_lines( text_arr, found_arr[0], found_arr[0] )        
        
        
        pattern_list= [
            'CREATE TRIGGER "',
            '"'
        ]
        
        same_line=0
        found_pattern=1
        found_cnt=0
        while found_pattern:

            found_arr = TextUtils.find_text( text_arr, pattern_list )
            
            if found_arr[0]<0:
                break
            
            found_line = text_arr[found_arr[0]]
            pos = found_arr[0]
            if found_line.find('morder_deo')>=0:
                a=1
                pass
            
            pos0 = found_line.find( '"' )
            pos1 = found_line.find( '"', pos0+1 ) # is there a second QUOTE in line ?
            
            new_text = TextUtils.replace_text( found_line, '"', '' )
            text_arr[found_arr[0]] = new_text
            
            if pos1>=0:
                # there was a second QUOTE in line
                pass
            else:    
                if found_arr[1]<0:
                    raise BlinkError( 1, 'Pattern (no:2) "'+str(pattern_list)+'" not found.')    
                else:
                    new_text = TextUtils.replace_text( text_arr[found_arr[1]], '"', '' )
                    text_arr[found_arr[1]] = new_text  
                
            found_cnt = found_cnt + 1
        
        if not found_cnt:
            raise BlinkError(2,'Pattern "'+str(pattern_list)+'" not found.')    
    
    def do_view_basis(self):
        
        text_arr = self.text_arr_g['VIEW']
        
        # 0.
        pattern_list= [
            'SET search_path ='         
        ]
        found_arr = TextUtils.find_text( text_arr, pattern_list )
        found_pos = found_arr[0]       
        if found_pos<0:
            raise BlinkError(1,'Pattern "'+str(pattern_list)+'" not found.')
        TextUtils.remove_lines( text_arr, found_arr[0], found_arr[0] )   
        
        # 1.
        pattern_list= [
            'CREATE OR REPLACE VIEW idv',
            'COMMENT ON VIEW idv'
        ]
        found_arr = TextUtils.find_text( text_arr, pattern_list )  
        if found_arr[1] <0:
            raise BlinkError(1,'Pattern "'+str(pattern_list)+'" not found.')
        TextUtils.remove_lines( text_arr, found_arr[0], found_arr[1] )  
        
        # 2.
        pattern_list= [
        'CREATE OR REPLACE VIEW cct_col_view',
        ';'
        ]
        found_arr = TextUtils.find_text( text_arr, pattern_list )  
        if found_arr[1] <0:
            raise BlinkError(2,'Pattern "'+str(pattern_list)+'" not found.')
        TextUtils.remove_lines( text_arr, found_arr[0], found_arr[1] )  
        
        # 3.
        pattern_list= [
        'REATE OR REPLACE VIEW cct_tab_view',
        ';'
        ]
        found_arr = TextUtils.find_text( text_arr, pattern_list )  
        if found_arr[1] <0:
            raise BlinkError(2,'Pattern "'+str(pattern_list)+'" not found.')
        TextUtils.remove_lines( text_arr, found_arr[0], found_arr[1] )          
        
        new_text = '''CREATE or REPLACE VIEW cct_tab_view AS 
            SELECT table_name, table_type, table_name as name, table_type as kind  
            from information_schema.tables where table_schema not in ('pg_catalog', 'information_schema') order by table_name; \n'''
        text_arr.append( new_text )    
        
        new_text = '''CREATE or REPLACE VIEW cct_col_view  AS  
            SELECT TABLE_NAME, COLUMN_NAME, DATA_TYPE from information_schema.columns 
            WHERE  table_schema not in ('pg_catalog', 'information_schema')  order by table_name; \n'''
        text_arr.append( new_text )            
    
    def do_sequence_basis(self):
        
        text_arr = self.text_arr_g['SEQUENCE']
        
        # 0.
        pattern_list= [
            'SET search_path ='         
        ]
        found_arr = TextUtils.find_text( text_arr, pattern_list )
        found_pos = found_arr[0]       
        if found_pos<0:
            raise BlinkError(1,'Pattern "'+str(pattern_list)+'" not found.')
        TextUtils.remove_lines( text_arr, found_arr[0], found_arr[0] )   
        
    def do_sequences(self):
        
        pk_tables = self._get_pk_tables()    
        
        ignore_tables = [
            'id',
            'idv',
            'cct_table',
            'globals',
            'app_data_type'
            
        ]
        
        for table in pk_tables:
            if table in ignore_tables:
                continue
            
            self._do_one_sequence(table)

    def do_INSERT(self):
        
        key = 'INSERT'
        filename_seq = os.path.join( self.basedir, self.files[key] )
        filename_out = os.path.join( self.outdir, self.files[key] )
        
        print('OUTPUT: '+filename_out)
        
        # read table-names
        fp_out = open(filename_out, 'w')
       
        
        # read table-names
        fo = open(filename_seq, 'r')
        line_no=0
        
        fp_out.write( "SET session_replication_role TO 'replica'; \n") # to support insert of original primary key values
        
        for line in fo:
            
            while 1:
                
                if line.find('SET search_path = blk')>=0:
                    line=''
                
                fp_out.write( line )   
                
                break # final break
            
            line_no = line_no + 1
            
        fp_out.write( "SET session_replication_role TO 'origin'; \n")
            
        fo.close()  
        fp_out.close()  
        
    def do_POST_insert(self):
        '''
        img_path	/data/appdata/blk/images	
        data_path	/data/appdata/blk/data	
        work_path	/data/parti_temp/work_blk
        
        '''
        
        key = 'post_insert'
        filename_out = os.path.join( self.outdir, self.files[key] )
        print('OUTPUT: '+filename_out)
        
        # read table-names
        fp_out = open(filename_out, 'w')
          
    
        lines = '''
        
        update globals set value='/data/magasin/blkpg/images' where name='img_path';
        update globals set value='/data/magasin/blkpg/data' where name='data_path';
        update globals set value='/data/magasin/blkpg/tmp' where name='work_path';
        '''
        fp_out.write( lines )  
        
        fp_out.close()
        
        
        
if __name__ == "__main__":
    
    
    connection_parameters = {
        'dbname': 'magasin', 
        'user':'blkpg', 
        'host':'localhost', 
        'password':'hsdhsh62367WE'
    }
    
    OS_syst='WINDOWS'
    #OS_syst='DEBIAN' 
    
    if OS_syst=='WINDOWS':
        basedir = os.path.dirname(__file__) + r'/../../tmp_data/pythonlib/in'
        dataout = os.path.dirname(__file__) + r'/../../tmp_data/pythonlib/out'
    else:
        basedir = '/home/osboxes/Code/Ora2pg/datain'
        dataout = '/home/osboxes/Code/Ora2pg/dataout'      
    
    a = Ora2pg(basedir, dataout)
    #a.open(connection_parameters)
    #a.test1()
    
    a.do_table_basis()
    a.do_view_basis()
    a.do_trigger_basis()
    a.do_sequence_basis()
    
    a.do_sequences()
    
    a.do_INSERT()
    
    a.do_POST_insert()
    
    
    a.save_output()
    
    print("READY")