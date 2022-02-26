"""
this is a wrapper to call a user defined python module
used by PHP-script PythonApi.inc
@module: modcaller.py
@author: Steffen Kube
@swreq   concept:0000678 g.Statistics
@version 2017-11-20: remove lib demjson
@version $Header: /group/it/cvs/partisan/pythonlib/modcaller.py,v 1.4 2017/11/28 14:17:11 steffen Exp $
"""

import sys
import os.path
#OLD: from demjson import demjson
import json

# -p : option xml
if __name__ == "__main__":

	# read command line arguments
	import getopt

	opts, args = getopt.getopt(sys.argv[1:], 'x', ["script=", "in=", "out=", "options="])

	output =None
	input  =None
	options=None
	script =None
	optionsFile = None

	for o, a in opts:
		if o =="--script":
			script = a
		if o =="--out":
			output = a
		if o =="--in":
			input = a
		if o =="--options":
			optionsFile = a
			

	if script == None:
		raise ValueError("No script given (-script)")

	this_path = os.path.dirname(  __file__ )
	# print " DEBUG: module:"+script
	sys.path.extend((this_path,)) # extend file
	
	# change current working dir
	os.chdir(this_path)
	
	
	if optionsFile != None:
		fp = open(optionsFile,"r")
		jsonstr = fp.read()
		fp.close()
		options = json.loads(jsonstr)  # demjson.decode( jsonstr )
		
	
	# must be set, e.g. for MPLCONFIGDIR (matplotlib)
	os.environ['HOME']='/var/www'
	
	moduleName = os.path.splitext(script)[0]

	# from code import Anwendertest as userclass
	exec ("from %s import %s as userclass" % (moduleName,moduleName) )

	maininst = userclass(input, output, options)
	answer   = maininst.start()
	# print ("answer" + str(answer) )
	
	
	json_object = json.dumps(answer)# demjson.encode( answer )
	
	fp = open(output,"w")
	fp.write(json_object)
	fp.close()
	
