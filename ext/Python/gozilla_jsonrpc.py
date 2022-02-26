'''

this is the GOZILLA CLIENT in PYTHON
API-Docu: http://intranet/dokuwiki/groups/it/gozilla/start
- with cookie management
INSTALL: install jsonrpcclient
package: gozilla_jsonrpc.py
Source:   GIT/gozilla/ext/Python
:author:  Steffen Kube
:version: 2019-06-24
'''

import os
from urllib.parse import urljoin


from jsonrpcclient import request

from jsonrpcclient.clients.http_client import HTTPClient
from jsonrpcclient import request
from jsonrpcclient.requests import Request

class gozilla_jsonrpc:

    sessionid=None
    cookies = None
    _config = {}

    def __init__(self, config):
        '''
        'url' : e.g. "http://gozilla_qa.blink.lan/pionir/api"
        'dbid' : blk
        'ssl_verify' : [OPTIONAL] [1], 0
        '''
        self.cookies = None
        self._config = config
        
        self.ssl_verify = True
        tmp_val = self._config.get('ssl_verify', 1)
        if type(tmp_val) == int:
            tmp_val_int = int(tmp_val)
        if tmp_val_int<=0:
            self.ssl_verify = False        


    def login(self, auth):
        '''
        login
        ':param dict auth:
           
            'username'
            'password'
        '''

        self.sessionid = None
        self.cookies   = None
        
        auth_url = self._config['url'] + '/' + 'json'
        # print(__file__+':DEBUG: URL:'+ auth_url + ' SSL_verify:' + str(self.ssl_verify)  )
        self.client = HTTPClient( auth_url )

        self.client.session.auth = ( auth['username'] , auth['password'] )
        argu = {"dbid": self._config['dbid'] }
        response = self.client.send( Request("login", argu), verify=self.ssl_verify )
        self.cookies  = self.client.session.cookies

        result = response.data.result
        if 'error' in result:
            raise ValueError ("ERROR" + str(result['error']) )
        if not 'data' in result:
            raise ValueError ("ERROR no DATA.")

        self.sessionid = result['data']['sessionid']

        self.client.session.auth = ()
        self.client.session.headers.update( {'X-RPC-Auth-Session' : self.sessionid} )

    def call(self, method, argu):
        '''
        call method
        - with cookies ...
        :param string method:
        :param dict argu: arguments
        '''

        self.client.session.cookies = self.cookies
        response = self.client.send( Request( method, argu ) )
        result = response.data.result

        if 'error' in result:
            raise ValueError ("ERROR" + str(result['error']) )
        if not 'data' in result:
            raise ValueError ("ERROR no DATA.")

        return result['data']

    def get_session(self):
        return self.sessionid

    def logout(self):
        self.sessionid = None
        self.client.session.headers.update( {'X-RPC-Auth-Session' : None} )

    def get_cookies(self):
        return self.cookies
