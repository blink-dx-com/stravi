<?php
require_once ('f.workdir.inc');

/**
 * This is the basic class to call Python. For your own connection derive
 * from this class and handle input and output of data.
 * 
 * python command line: python code.py --script==test.py --in=datafile -out=output.xml -opt=options.xml
 * 
 * - debug-options: 
 *   - $_SESSION['userGlob']["g.debugLevel"]>0
 *   - $_SESSION['userGlob']["g.debugKey"] = 'PythonApi'
 *   - 
 * @package PythonApi.inc
 * @namespace core::misc
 * @swreq   concept:0000678 g.Statistics > implement new statistics technology 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/lev1/PythonApi.inc 59 2018-11-21 09:04:09Z $
 */ 
class PythonApi
{
	var $pythonExe;
	var $temp;
	var $errorOccured;

	var $py_script;
	var $r_input;
	var $r_output;
	var $r_options;
	var $pycodePath; // path of python code

	/**
	 * Constructor.
	 */
	function __construct( )
	{
		global $error;
		$FUNCNAME= $this->__CLASS__.':PythonApi';
		
	
		$this->error    = &ErrorHandler::get();	
		$this->pythonExe    =  $_SESSION['globals']['exe.Python']; // e.g. "/usr/local/lib/python"
		if ( !file_exists($this->pythonExe) ) {
			$error->set( $FUNCNAME, 1, '_SESSION[globals][exe.Python] ('.$this->pythonExe.') not exists ' );
			return;
		}

		$this->_debug     = $_SESSION['userGlob']["g.debugLevel"];
		$this->py_script  = "code.py";
		$this->py_modcaller="modcaller.py";
		$this->r_output   = "ROutput";
		$this->r_input    = "RInput";
		$this->r_options  = "ROptions";
		$this->pycodePath = NULL;
		$this->showErrorOutput = 1;
		
		
		if ( strstr($_SESSION["userGlob"]["g.debugKey"],'PythonApi')==NULL ) $this->_debug = 0; // not active
		
		$this->pythonlibPath    = $_SESSION['s_sessVars']['AppRootDir'].'/pythonlib/';
		$this->py_modcallerFull = $this->pythonlibPath . $this->py_modcaller;
		
		$this->errorOccured = false;
	}
	
	/**
	 * set output silence on error
	 * @param int $flag
	 * 	 0: normal
	 *   -1: full output
	 *   1: be silent
	 */
	function setSilence($flag) {
		if ($flag<0) $this->showErrorOutput = 1;
		if ($flag>0) $this->showErrorOutput = 0;
		
	}
	
	
	/**
	 * Abstract function. Must be implemented by subobject. This function will 
	 * be called before the R script is executed. It's task is to write RInput
	 * file, maybe filled with data from database. 
	 */
	function createRInput()
	{
	}
	
	/**
	 * Abstract function. Must be implemented by subobject. This function will 
	 * be called before the R script is executed. It's task is to write ROptions
	 * file, maybe filled with data from database. 
	 */
	function createROptions()
	{
	}
	
		
	/**
	 * Abstract function. Must be implemented by subobject. This function will 
	 * be called after the R script was executed. It's task is to read ROutput
	 * file, maybe to store data into database. 
	 */
	function saveOutput()
	{
	}
	
	

	/**
	 * Check permission for touched files.
	 */
	function checkPermission($input)
	{
	}	
	


	/**
	 * Prepare Python script. Concatenate the content of two files, the partisan r 
	 * libary file and the user script file. The result, saved in PyScript, is 
	 * the final code, which will be executed.
	 */
	function prepareScript($pycode)
	{
		$FUNCNAME= $this->__CLASS__.':prepareScript';		 
		// open new temporary script
		
		$this->py_script = basename($pycode);
		$pyTempFile = $this->temp . $this->py_script;
		
		if (!file_exists($pycode)) {
			$this->error->set($FUNCNAME, 0002, 'Sourcecode file "'.$pycode.'" doeas not exist.', CCT_ERROR);
			return;
		}
		
		$answer = copy($pycode,$pyTempFile);
		if (!$answer) {
			$this->error->set($FUNCNAME, 0003, "Creation of temp python-script failed.", CCT_ERROR);
			return;
		}
		$this->pycodePath = $pyTempFile;
		
		// copy the module caller
		$answer = copy($this->py_modcallerFull, $this->temp . $this->py_modcaller);
		if (!$answer) {
			$this->error->set($FUNCNAME, 0004, "Creation of temp py_modcaller failed.", CCT_ERROR);
			return;
		}
		$answer = copy($this->pythonlibPath.'__init__.py', $this->temp.'__init__.py');
		if (!$answer) {
			$this->error->set($FUNCNAME, 0004, "Creation of __init__.py failed.", CCT_ERROR);
			return;
		}
		
		
		return true;
	}
	
	/**
	 * copy a python-module (e.g. a needed package) to the temp-environment
	 * @param string $filename
	 * @return -
	 */
	function addPythonModule($filename) {
		
		$FUNCNAME=__CLASS__.':addPythonModule';
		
		$pyBaseName = basename($filename);
		$answer = copy($filename, $this->temp . $pyBaseName);
		if (!$answer) {
			$this->error->set($FUNCNAME, 0001, 'Adding module "'.$filename.'" failed.', CCT_ERROR);
			return;
		}
	}
	
	/**
	 * handle result-file
	 * @return array $answerDict : the dictionary from the python-code
	 */
	function _handleResult() {
		
		$FUNCNAME=__CLASS__.':_handleResult';
		
		if (filesize($this->temp . "RError") > 0) {
			$this->errorOccured = true;
			
			if (!($r_error = fopen($this->temp . "RError", "r"))) {
				$this->error->set($FUNCNAME, 0002, "Opening error output stream failed.", CCT_ERROR);
				return;
			} else {		
				if ($this->showErrorOutput or $_SESSION['userGlob']["g.debugLevel"] > 0 ) {			
					echo "<blockquote><font color=red><pre>--- begin of error output ---<br><br>";	
					while (!feof($r_error))
					{
						$content = fgets($r_error);
						echo htmlspecialchars($content);
					}
					echo "<br><br>--- end of error output ---<br></pre></font></blockquote>";
				}
			}

			// show script, if debug>1
			if (  $_SESSION['userGlob']["g.debugLevel"]>0 )
			{
				if (!($py_script = fopen($this->pycodePath, "r"))) {
					$this->error->set($FUNCNAME, 0006, "Opening script file failed.", CCT_ERROR);
					return;
				} else {					
					echo "<blockquote><font color=darkblue><pre>--- begin of script ---<br><br>";	
					while (!feof($py_script))
					{
						$content = fgets($py_script);
						echo htmlspecialchars($content);
					}
					echo "<br><br>--- end of script ---<br></pre></font></blockquote>";
				}
			}
			
			$this->error->set($FUNCNAME, 0003, "Calling script failed.", CCT_ERROR);
			return;
		}
		
		$answerDict = NULL;
		// read JSON-file and convert it to an array
		if (filesize($this->temp . "ROutfile") > 0) {
			
			// $json_str   = file_get_contents ( $this->temp . "ROutfile" );
			if ($this->_debug> 1) {
				debugOut( 'UsedMem [bytes]:'.memory_get_usage(True), $FUNCNAME.':memory2: ');
			}
			$answerDict = json_decode ( file_get_contents ( $this->temp . "ROutfile" ) , True);
			if ($answerDict===NULL) {
				$this->error->set($FUNCNAME, 0007, "output-file ROutfile contains no valid JSON format; JsonErr:" .
							json_last_error_msg ( ), CCT_ERROR);
				return;
			}
			if ($this->_debug> 1) {
				debugOut( 'UsedMem [bytes]:'.memory_get_usage(True), $FUNCNAME.':memory3: ');
			}
			// OLD: used Services_JSON for older PHP-versions < PHP5.1
			//$jsonLib    = new Services_JSON(SERVICES_JSON_LOOSE_TYPE);
			//$answerDict = $jsonLib->decode($json_str);
			//if ($jsonLib->isError($answerDict)) {
			//	$this->error->set($FUNCNAME, 0007, "output-file ROutfile contains no valid JSON format", CCT_ERROR);
			//	return;
			//}
		}
		
		return $answerDict;
	}
	
	
	/**
	 * Execute python script. Python will be called in a silent mode, which means 
	 * not to display any version info and code lines. Only errors and wanrings
	 * will be displayed, which will be redirected to RError. 
	 * @param  string $inputfile : only file name
	 * @param  array $inoptions
	 * @return array $result : dictionary
	 */
	function runScript($inputfile, $inoptions=NULL)
	{		
		
		global $error;
		$FUNCNAME= "PythonApi.runScript";
		
		if ($this->pycodePath==NULL) {
			$this->error->set("PythonApi.runScript", 0005, "Missing input script.", CCT_ERROR);
			return false;
		}
		$pyTempFile = $this->temp . $this->py_script;
		
		if ($inoptions!=NULL) {
			//OLD: $jsonLib    = new Services_JSON(SERVICES_JSON_LOOSE_TYPE);
			// $answerStr = $jsonLib->encode($inoptions);
			$answerStr = json_encode($inoptions);
			if ($answerStr===False) {
				$this->error->set($FUNCNAME, 0007, "error on producing options-file. JsonErr:" .
							json_last_error_msg (), CCT_ERROR);
				return;
			}
			$optFileName=$this->temp .$this->r_options;
			$fp = fopen($optFileName, 'w');
			$retVal = fputs( $fp, $answerStr );
			fclose( $fp );
			$pyInOptions = ' --options='.$optFileName.' ';
			
		} else $pyInOptions=NULL;
		
		# hide:DeprecationWarning; some python script throw DeprecationWarning
		$cmd = $this->pythonExe . 
			" -W ignore::DeprecationWarning:".
			"  ". $this->temp . $this->py_modcaller .
			" --script=" . $this->py_script . $pyInOptions .
			" --in=". $this->temp .$inputfile. " ".
			" --out=".$this->temp ."ROutfile > ". $this->temp . "ROutput 2> ". $this->temp . "RError";
		
		if ($this->_debug> 0)
		 	debugOut($cmd, $FUNCNAME.':python-cmd: ');
		
		if (!file_exists($this->pythonExe))
		{
			$this->error->set("PythonApi.runScript", 0007, "Missing Python software.", CCT_ERROR);
			return false;
		}


		system($cmd);
		if ($this->_debug> 1) {
			debugOut( 'UsedMem [bytes]:'.memory_get_usage(True), $FUNCNAME.':memory1: ');
		}
		$result = NULL;
		

		if (file_exists($this->temp . "RError"))
		{	
			$result = $this->_handleResult();
		}
		else {
			$this->error->set("PythonApi.runScript", 0001, "Missing error output.", CCT_ERROR);
		}
		
		if ( $this->error->Got(READONLY) ) {
			return NULL;
		}
		
		return $result;
	}



	/**
	 * Preparing the workspace. Creating a subdirectory in temp. Also it's 
	 * connecting to database.
	 */
	function prepare($temp)
	{
		$this->temp = $temp;
	
		if (!file_exists($temp)) {
			$this->error->set("PythonApi.prepare", 0001, "Workspace not exists.", CCT_ERROR);
			return false;
		}		
			
		return true;	
	}

	/**
	 * Preparing the workspace. Creating a subdirectory in temp. 
	 */
	function prepare2($temp)
	{
		$this->temp = $temp;	
		if (substr($temp,-1,1)!='/') $this->temp .=  '/';		
		return true;	
	}

	/**
	 * Remove previously used temporary directory. 
	 *
	 * @return		boolean		failed or not
	 * @param		string		path of temporary directory to remove
	 */
	function clean()
	{
		$tempdir = $this->temp;
	
		// is it a directory ?
		if (!is_dir($tempdir)) 
			return true;
			
		// open directory ...
		if (!($dir = opendir($tempdir)))
		{
			$this->error->set("PythonApi.remove", 0001, "Could not open temporary directory.", CCT_ERROR);
			return false;
		}
		
		// ... and delete any file inside ...
		while($filename = readdir($dir)) 
		{
			if ($filename != "." && $filename != "..") 
				if (!unlink($tempdir . "/" . $filename))
				{
					$this->error->set("PythonApi.remove", 0002, "Could not unlink tempory file: $filename.", CCT_ERROR);
					return false;
				}
		}

		// ... and remove directory itself
		closedir($dir);
 		if (!rmdir($tempdir))
		{
			$this->error->set("PythonApi.remove", 0003, "Could not unlink tempory directory.", CCT_ERROR);
			return false;
		}
		
		return true;
	}
	
	function getOutPutFileName() {
		return $this->temp . "ROutput";
	}
}

