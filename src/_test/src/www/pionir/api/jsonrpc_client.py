# test for www/pionir/api/jsonrpc.php
# this is the CLIENT in PYTHON

from jsonrpcclient import request

url="http://gozilla_qa.blink.lan/pionir/api/jsonrpc.php"


from jsonrpcclient.clients.http_client import HTTPClient
from jsonrpcclient import request
from jsonrpcclient.requests import Request

client = HTTPClient(url)

argu = {}
response = client.send( Request("gGetVersion", argu ) )
result = response.data.result
print ("DEBUG_No_session: "+str(result))


client.session.auth = ("test", "1234AbCd")
argu = {"dbid": "blk"}
response = client.send( Request("login", argu) )
result = response.data.result
print ("DEBUG_1: "+str(result))
if 'error' in result:
    raise ValueError ("ERROR" + str(result['error']) )
if not 'data' in result:
    raise ValueError ("ERROR no DATA.")


sessionid = result['data']['sessionid']
print ("SESS: "+str(sessionid))

client.session.auth = ()
client.session.headers.update({'X-RPC-Auth-Session' : sessionid})

argu = {"num": 3}
response = client.send( Request("gGetVersion", argu ) )
result = response.data.result
print ("DEBUG_2: "+str(result))

argu = { 't':'EXP', 'id':10, 'cols':('NAME', 'NOTES') }
response = client.send( Request("DEF/gObj_getParams", argu ) )
result = response.data.result
print ("DEBUG_3: "+str(result))