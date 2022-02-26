# -*- coding: utf-8 -*-
__docformat__ = "restructuredtext en"

"""
abstract layer for postgres database

File:           db_pg_abs.py
Copyright:      Blink AG   
Author:         Steffen Kube <steffen@blink-dx.com>

:Example:

    db_obj = pg_database()

    sql_cmd = "id from exp where name='hallo'"
    db_obj.select_tuple(sql_cmd)
    
    all_data = []
    while db_obj.ReadRow():
        id = db_obj.RowData[0]       
        all_data.append(data_row)
            
            
"""


import psycopg2
import psycopg2.extras
import psycopg2.extensions

import os
import time
from datetime import datetime


from .f_utilities import debug
from .f_utilities import BlinkError

def cast_decimal(value, curs):
    
    if value is None: 
        return None
     
    if float(int(value)) == float(value):
        return int(value)
    return float(value)

DEC2FLOAT = psycopg2.extensions.new_type(
    psycopg2.extensions.DECIMAL.values,
    'DEC2FLOAT',
    cast_decimal )

psycopg2.extensions.register_type(DEC2FLOAT)


class pg_database:
    
    _autocommit = 1
    RowData     = None  # one result data row
    _debug      = 0
    
    def __init__(self):
        self._debug = 0
        '''
        if 'user_glob' in session:
            if type(session['user_glob'].get('debug.level','') )== str:
                if session['user_glob'].get('debug.level','')=='':
                    session['user_glob']['debug.level'] = 0 
                else:
                    session['user_glob']['debug.level'] = int(session['user_glob'].get('debug.level','0'))
            self._debug = int(session['user_glob'].get('debug.level',0))
         '''
    
    def open(self, connection_parameters):
        '''
        open connection
        :param connection_parameters: 
           dbname='magasin', user='blinkapp', host='localhost', password=''
          '''
        self._conn = psycopg2.connect(**connection_parameters)
        self.set_auto_commit(True)
        
        
    def addQuotes(self, text):
        '''
        add quotes to value
        - None   => NULL
        - string => 'string' (escaped ')
        - decimal=> '1.3454'
        '''
        if text == None:
            return 'NULL'
        
        if type(text) is not str:
            return str(text)
           
        
        text = text.replace("'","''") # special POSTGRES rule: single quote will be escaped by ''
        text = "'"+text +"'"
        return text
    
    def _get_pk_sql(self, pk_dict):
        
        if not len(pk_dict):
            raise BlinkError(1,'Input missing: pk_dict')
        
        sql_where=[]
        for key in pk_dict:      
            val=pk_dict[key]  
            val_clean = self.addQuotes(val)
            sql_where.append(key+"="+val_clean)           
        
        return sql_where
    
    def _exec_cmd(self, sql_state):
        # if you want to log every SQL-command: use this:
        if self._debug:
            debug.printx( __name__, '(111) SQL: ' + sql_state )
        self._select_curs.execute(sql_state)
    
    def insert_row(self, table, args, pk_name=''):
        
        c = self._conn.cursor()
        
        sql_cols=[]
        sql_data=[]
        
        for key in args:
            sql_cols.append(key)
            
            val=args[key]
            
            val_clean = self.addQuotes(val)
            if val_clean=="''":
                # make EMPTY string to NULL !!!
                val_clean = 'NULL'                
            
            sql_data.append(val_clean)
        
        if pk_name=='':
            pk_name  = session['db_cache']['tables'][table]['feats']['PRIM_KEYS'][0]
            
        sqlstate = "INSERT INTO "+table+ " (" + ', '.join(sql_cols) +") " + "VALUES (" + ', '.join(sql_data) +") RETURNING " + pk_name
        if self._debug:
            debug.printx( __name__, 'insert:SQL: ' + sqlstate )
       
        c.execute(sqlstate)
        obj_id = c.fetchone()[0]
        if type(obj_id) != str:
            obj_id = int(obj_id) # was decimal before ...
            
        # debug.printx( __name__, 'PK_type: ' + str( type(obj_id)) )
        # debug.printx( __name__, 'insert:LAST_ROW: ' + str(obj_id) )
        
        # Save (commit) the changes
        if self._autocommit:
            self._conn.commit()
        
        return obj_id
    
    def update_row(self, table, pk_dict, args):
        '''
        update row
        '''
        c = self._conn.cursor()
        
        sql_set=[]
        sql_where = self._get_pk_sql(pk_dict)       
        
        for key in args:
           
            val=args[key]
            val_clean = self.addQuotes(val)
            if val_clean=="''":
                # make EMPTY string to NULL !!!
                val_clean = 'NULL'
                     
            sql_set.append(key+"="+val_clean)
            
        sqlstate = "UPDATE  " + table + " set " + ', '.join(sql_set)  + " where " + ' and '.join(sql_where) 
        if self._debug:
            debug.printx( __name__, 'update:SQL: ' + sqlstate )
        c.execute(sqlstate)
        
        # Save (commit) the changes
        if self._autocommit:
            self._conn.commit()     
            
    def del_row(self, table, pk_dict):
        '''
        delete row
        '''
        c = self._conn.cursor()
        
        sql_where=self._get_pk_sql(pk_dict)
        sqlstate = "DELETE from  " + table + " where " + ' and '.join(sql_where) 
        if self._debug:
            debug.printx( __name__, 'delete:SQL: ' + sqlstate )        
        c.execute(sqlstate)
        
        # Save (commit) the changes
        if self._autocommit:
            self._conn.commit()         
        
    def select_tuple(self, sql_select): 
        '''
        get result as tuple 
        :Example:
        sql_cmd = "id from exp where name='hallo'"
        db_obj.select_tuple(sql_cmd)
        all_data = []
        while db_obj.ReadRow():
            id = db_obj.RowData[0]       
            all_data.append(data_row)
            
        '''
        sqlstate='select '+sql_select
        self.__result_type='tuple'
        
        self._select_curs = self._conn.cursor()
        self._exec_cmd(sqlstate)
        
    def select_dict(self, sql_select): 
        '''
        get result as dictionary 
        :Example:
        sql_cmd = "* from exp order by exp_id"
        db_obj.select_dict(sql_cmd)
        all_data = []
        while db_obj.ReadRow():
            id = db_obj.RowData['id']       
            
        '''
        sqlstate='select '+sql_select
        self.__result_type='dict'
        self._select_curs = self._conn.cursor() #OLD: cursor_factory=psycopg2.extras.DictCursor
        self._exec_cmd(sqlstate)    
    
    def select_col_vals(self, table, where_dict, out_cols, sel_option={} ): 
        '''
        get result as tuple 
        :param dict sel_option:
            'order' : column 
        :Example:
        
        db_obj.select_col_vals(table, {'TABLENAME':'EXP'}, ['COULN_NAME'] )
        all_data = []
        while db_obj.ReadRow():
            id = db_obj.RowData[0]       
            all_data.append(data_row)
            
        '''
        where_list = []
        for col,val in where_dict.items():
            where_list.append( col + "=" + self.addQuotes(val) )        
        
        sqlstate = "select " + ",".join(out_cols) + ' from '+ table + " where " + " and ".join(where_list)
        order_col = sel_option.get('order', '')
        if order_col!='':
            sqlstate =  sqlstate + ' order by ' + order_col
        
        self.__result_type='dict'
        
        self._select_curs = self._conn.cursor()
        self._exec_cmd(sqlstate)    
        
    def ReadRow(self):
        '''
        read one row
        - set self.RowData
        - if NO result => self.RowData = None
        :return: int 0,1 data exists ?
        '''
        
        self.RowData = None   # default value
        row = self._select_curs.fetchone()

        if row == None: return False
        
        if self.__result_type=='dict':
            
            self.RowData = {}
            descr = self._select_curs.description
            columns = [col.name for col in descr]
            
            # transform keys to UPPER
            row_new = {}
            cols_upper=[]
            i=0
            for key in columns:
                self.RowData[key.upper()] = row[i]
                i=i+1
             
        else:
            self.RowData = row
 
        # debug.printx( __name__, 'select: DATA_out: ' + str(self.RowData) )
        
        if self.RowData == None:
            return False
        else:
            return True
    
    def execute(self, sqlstate):
        self._select_curs = self._conn.cursor()
        self._exec_cmd(sqlstate)        
        
    def commit(self):
        self._conn.commit()  
        
    def set_auto_commit(self, flag):
        old_val = self._autocommit
        if not flag:
            self._autocommit = 0
            self._conn.autocommit = False
        else:
            self._autocommit = 1
            self._conn.autocommit = True
            
        return old_val
    
    def rollback(self):
        self._conn.rollback()

    def col_val(self, table, sea_column, sea_val, outcol):
        '''
        get VALUE of outcol from TABLE table
        :return: string or None
        '''
        sqlstate = outcol+ ' from '+table+ " where " + sea_column +"=" + self.addQuotes(sea_val)
        self.select_tuple( sqlstate )
        val = None
        if self.ReadRow():
            # analyse one row       
            val = self.RowData[0]  
        # debug.printx( __name__, 'col_val:: ' + str(val)  +':'+sqlstate )
         
        return val
    
    def col_val_where(self, table, where_dict, outcol):
        '''
        get VALUE of outcol from TABLE table where where_dict
        :return: string or None
        '''
        
        where_list = []
        for col,val in where_dict.items():
            where_list.append( col + "=" + self.addQuotes(val) )
        
        sqlstate = outcol+ ' from '+table+ " where " + " and ".join(where_list)
        self.select_tuple( sqlstate )
        val = None
        if self.ReadRow():
            # analyse one row       
            val = self.RowData[0] 
         
        return val    
    
    def one_row_get(self, table, where_dict, out_cols ):
        '''
        get {KEY:VALUE} from TABLE table where {COL=VALUE}
        :return: string or None
        '''
        
        where_list = []
        for col,val in where_dict.items():
            where_list.append( col + "=" + self.addQuotes(val) )
            
 
        sqlstate = ",".join(out_cols) + ' from '+ table + " where " + " and ".join(where_list)
        self.select_dict( sqlstate )
        data = None
        if self.ReadRow():
            # analyse one row       
            data = self.RowData 
            
        return data    
    
    def count_elem(self, table, where_dict ):
        '''
        get count(1) from TABLE table where {where_dict}
        :return: count
        '''
        
        where_list = []
        for col,val in where_dict.items():
            where_list.append( col + "=" + self.addQuotes(val) )
            
 
        sqlstate = 'count(1) from '+ table + " where " + " and ".join(where_list)
        self.select_tuple( sqlstate )
        self.ReadRow()     
        cnt = self.RowData[0] 
        return cnt     
    
    def column_names(self):
        """
        return UPPER-case column names
        """
        column_names = [desc[0].upper() for desc in self._select_curs.description]
       
        return column_names
    
    def Timestamp2Sql(self, time_obj=None):
        if time_obj is None:
            time_obj = datetime.now()
        nowx     = time_obj.timetuple()
        now_str  = "%04u-%02u-%02u %02u:%02u:%02u" % (nowx[0], nowx[1],nowx[2],nowx[3],nowx[4],nowx[5])        
        return now_str
    
    def date_obj_2Sql(self, time_obj=None):
        '''
        datetime object to SQL
        '''
        if time_obj is None:
            time_obj = datetime.now()
        
        time_str = time_obj.strftime("%Y-%m-%d %H:%M:%S")
        #now_str = "%04u-%02u-%02u %02u:%02u:%02u" % (nowx[0], nowx[1],nowx[2],nowx[3],nowx[4],nowx[5])        
        return time_str
    
    def close(self):
        
        self._conn.close()  
        
    
if __name__ == "__main__":
            
    from blinkapp.conf  import config
    
    connection_parameters = config.superglobal['db']['blk']
    testlib = pg_database()
    testlib.open(connection_parameters)
    
    sql_cmd='* from exp where exp_id=10'
    testlib.select_dict(sql_cmd)
    if testlib.ReadRow():
        data_row = testlib.RowData 
        print ( "DATA: " + str(data_row) )
        print ( "exp_id: " + str(data_row['EXP_ID']) )
          
    print('UPDATE')
    
    nowx = time.localtime()
    nowFormat = "%04u-%02u-%02u %02u:%02u:%02u" % (nowx[0], nowx[1],nowx[2],nowx[3],nowx[4],nowx[4])    
    
    datax = {'name':'test now', 'notes':'ole:'+nowFormat }
    pkarr = {'exp_id':10}
    testlib.update_row('exp', pkarr, datax)
    
    sql_cmd='* from exp where exp_id=10'
    testlib.select_dict(sql_cmd)
    if testlib.ReadRow():
        data_row = testlib.RowData 
        print ( "DATA: " + str(data_row) )
        print ( "exp_id: " + str(data_row['EXP_ID']) )  
        
    sql_cmd='EXP_ID, NAME, NOTES from exp where exp_id=10'
    testlib.select_tuple(sql_cmd)
    if testlib.ReadRow():
        data_row = testlib.RowData 
        print ( "TUPLE: DATA: " + str(data_row) )
        print ( "exp_id: " + str(data_row[0]) )    
        
    print('INSERT')
    
    nowx = time.localtime()
    nowFormat = "%04u-%02u-%02u %02u:%02u:%02u" % (nowx[0], nowx[1],nowx[2],nowx[3],nowx[4],nowx[4])    
    
    datax = {'db_user_id':11, 'crea_date': nowFormat, 'table_name':'LINK' }
    obj_id = testlib.insert_row('cct_access', datax)    
    print ( "NEW_OBJ: t:cct_access id:" + str(obj_id) )
    
    print("READY")