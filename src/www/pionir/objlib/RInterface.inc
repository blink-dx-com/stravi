<?php
require_once ('f.workdir.inc');

/**
 * This is the basic class to call R software. For your own R connection derive
 * from this class and handle input and output of data.
 */ 
class RInterface
{
	var $rtool;
	var $temp; // WORK-Dir
	var $errorOccured;

	var $r_script;
	var $r_input;
	var $r_output;
	var $r_options;

	/* 
	
	structure INPUT_STRUCT {
		$input["exp_id"]     		= $id;
		$input["channel_id"]      	= $channel_id;
		$input["spot_id"]    		= $spot_id;
		$input["deleteSpotResults"]	= $deleteSpotResults;
	} 
	*/

	/**
	 * Constructor.
	 */
	function __construct( $rtool )
	{
		
	
		$this->error    = &ErrorHandler::get();	
		$this->rtool    = $rtool; //  /usr/local/lib/R/bin/R"; // ""$global["r_path"] . "R";

		$this->r_script = "RScript";
		$this->r_output = "ROutput";
		$this->r_input  = "RInput";
		$this->r_options  = "ROptions";
		
		$this->errorOccured = false;
	}
	
	
		
	/**
	 * This function is to proceed the whole normalization calculation in one 
	 * line.
	 *
	 * The functions createRInput(), saveRResult() and checkPermission() are 
	 * supposed as implemented.
	 * @param array $input  structure INPUT_STRUCT
	 * @param string $rcode path to Rcode
	 * @param string $R2PartisanLib : Rcode-lib RELATIVE to this directory "objtools"
	 */
	function run(
		$input,    
		$rcode,
		$R2PartisanLib = "RInterface.rcode"
		)
		{		
		global   $error;
		
		if ($_SESSION['userGlob']["g.debugLevel"] > 0) 
			echo " preparing ... \n<br>";
		
		// get work-path
		$workdirObj = new workDir();
		$workdir   = $workdirObj->getWorkDir ( "R.pack" );
		if ($error->Got(READONLY))  {
			$error->set("run",1, "creating work-dir failed.");
			return;
		}
		$workdir .= "/";
		
		if (!$this->prepare($workdir))
			return false;		
			
		if ($_SESSION['userGlob']["g.debugLevel"] > 0) 
			echo " checking permissions ... \n<br>";
		if (!$this->checkPermission($input))
			return false;

		if ($_SESSION['userGlob']["g.debugLevel"] > 0) 
			echo " creating input files ... \n<br>";
		if (!$this->createRInput($input))
			return false;
		
		if ($input["param"]){
			if ($_SESSION['userGlob']["g.debugLevel"] > 0) 
				echo " creating options file ... \n<br>";
			if (!$this->createROptions($input))
				return false;
		}
		
		if ($_SESSION['userGlob']["g.debugLevel"] > 0) 
			echo " prepare script ... \n<br>";
		if (!$this->prepareRScript($rcode, $R2PartisanLib))
			return false;
			
		if ($_SESSION['userGlob']["g.debugLevel"] > 0) 
			echo " running the script ... \n<br>";
			
		if (!$this->runRScript())
			return false;
			
		if ($_SESSION['userGlob']["g.debugLevel"] > 0) 
			echo " saving result to database ... \n<br>";
		if (!$this->saveROutput())
			return false;
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
	function saveROutput()
	{
	}
	
	

	/**
	 * Check permission for touched files.
	 */
	function checkPermission($input)
	{
	}	
	


	/**
	 * Prepare R script. Concatenate the content of two files, the partisan r 
	 * libary file and the user script file. The result, saved in RScript, is 
	 * the final R code, which will be executed.
	 * @param string $libary : Rcode-lib RELATIVE to this directory "objtools"
	 */
	function prepareRScript($rcode, $libary = "RInterface.rcode")
	{
				 
		// open new temporary script
		$outputfile = $this->temp . $this->r_script;
		if (!($r_script = fopen($outputfile, "w+")))
		{
			$this->error->set("RInterface.prepareRScript", 0001, "Creating temporary script file failed", CCT_ERROR);
			return false;
		}

		// open partisan r libary
		$libary_fulldir = dirname(__FILE__)."/".$libary; // build absolute path
		if (!file_exists($libary_fulldir))
		{
			$this->error->set("RInterface.prepareRScript", 0007, "R libary (".$libary_fulldir.") for Partisan not found.", CCT_ERROR);
			return false;
		}
		
		if (!($r_lib = fopen($libary_fulldir, "r")))
		{
			$this->error->set("RInterface.prepareRScript", 0006, "Opening R libary for Partisan failed", CCT_ERROR);
			return false;
		}
				
		// write partisan r libary
		while (!feof($r_lib))
		{
			$content = fgets( $r_lib, filesize($libary_fulldir) );
			if (fwrite($r_script, $content) != strlen($content))
			{
				$this->error->set("RInterface.prepareRScript", 0004, "Failure during writing temporary script file.", CCT_ERROR);
				return false;
			}
		}			
		
		// add new line
		$content = "\n";
		if (fwrite($r_script, $content) != strlen($content))
		{
			$this->error->set("RInterface.prepareRScript", 0005, "Failure during writing temporary script file.", CCT_ERROR);
			return false;
		}
						
		// open user script
		if (!file_exists($rcode))
		{
			$this->error->set("RInterface.prepareRScript", 0002, "Script file not found.", CCT_ERROR);
			return false;
		}
		
		if (!($r_script_user = fopen($rcode, "r")))
		{
			$this->error->set("RInterface.prepareRScript", 0003, "Opening script file failed", CCT_ERROR);
			return false;
		}

		// write user script part
		while (!feof($r_script_user))
		{
			$content = fgets($r_script_user, filesize($rcode));
			if (fwrite($r_script, $content) != strlen($content))
			{
				$this->error->set("RInterface.prepareRScript", 0005, "Failure during writing temporary script file.", CCT_ERROR);
				return false;
			}
		}
		
		// add new line
		$content = "\n"
				 . "# Quit R.\n"
				 . "q()\n";
		if (fwrite($r_script, $content) != strlen($content))
		{
			$this->error->set("RInterface.prepareRScript", 0005, "Failure during writing temporary script file.", CCT_ERROR);
			return false;
		}
			
		fclose($r_lib);
		fclose($r_script);
		fclose($r_script_user);
		
		return true;
	}
	
	/**
	 * handle result-file
	 * @return unknown_type
	 */
	function _handleResult() {
		
		
		if (filesize($this->temp . "RError") > 0) {
			$this->errorOccured = true;
			
			if (!($r_error = fopen($this->temp . "RError", "r"))) {
				$this->error->set("RInterface.runRScript", 0002, "Opening error output stream failed.", CCT_ERROR);
				return;
			} else {					
				echo "<blockquote><font color=red><pre>--- begin of error output ---<br><br>";	
				while (!feof($r_error))
				{
					$content = fgets($r_error);
					echo htmlspecialchars($content);
				}
				echo "<br><br>--- end of error output ---<br></pre></font></blockquote>";
			}

			// this part shell only be possible for root
			if ( glob_isAdmin() or $_SESSION['userGlob']["g.debugLevel"] > 0){
				if (!($r_script = fopen($this->temp . $this->r_script, "r"))) {
					$this->error->set("RInterface.runRScript", 0006, "Opening script file failed.", CCT_ERROR);
					return;
				}else{					
					echo "<blockquote><font color=darkblue><pre>--- begin of script ---<br><br>";	
					while (!feof($r_script))
					{
						$content = fgets($r_script);
						echo htmlspecialchars($content);
					}
					echo "<br><br>--- end of script ---<br></pre></font></blockquote>";
				}
			}
			
			$this->error->set("RInterface.runRScript", 0003, "Calling 'R' software failed.", CCT_ERROR);
			return;
		}
	}
	
	
	/**
	 * Execute R script. The R tool will be called in a silent mode, which means 
	 * not to display any version info and code lines. Only errors and wanrings
	 * will be displayed, which will be redirected to RError. 
	 */
	function runRScript()
	{		
		
		
		$cmd = $this->rtool . " --no-save --silent --slave < " . $this->temp . "RScript > ". $this->temp . "ROutput 2> ". $this->temp . "RError";
		
		if ($_SESSION['userGlob']["g.debugLevel"] > 0)
			echo "<br><font color=gray>debug.commandline: " . $cmd . "</font><br>";
		
		if (!file_exists($this->rtool))
		{
			$this->error->set("RInterface.runRScript", 0007, "Missing R software.", CCT_ERROR);
			return false;
		}
				
		$path = getcwd();
		if (!chdir($this->temp))
		{
			$this->error->set("RInterface.runRScript", 0004, "Switching to temporary directory not possible.", CCT_ERROR);
			return false;
		}
		
		system($cmd);
		
		/** test **************************************
		echo "<blockquote><font color=darkblue><pre>--- begin of script ---<br><br>";	
		while (!feof($r_script))
		{
			$content = fgets($r_script);
			echo htmlspecialchars($content);
		}
		echo "<br><br>--- end of script ---<br></pre></font></blockquote>";
		*/

		if (file_exists($this->temp . "RError"))
		{	
			$this->_handleResult();
		}
		else {
			$this->error->set("RInterface.runRScript", 0001, "Missing error output.", CCT_ERROR);
		}
		
		if (!chdir($path)) {
			$this->error->set("RInterface.runRScript", 0005, "Switching back to server directory not possible.", CCT_ERROR);
			return false;
		}
		
		if ( $this->error->Got(READONLY) ) {
			return false;
		}
		
		return true;
	}



	/**
	 * Preparing the workspace. Creating a subdirectory in temp. Also it's 
	 * connecting to database.
	 */
	function prepare($temp)
	{
		$this->temp = $temp;
	
		if (file_exists($temp))
			if (!$this->clean())
			{
				$this->error->set("RInterface.prepare", 0001, "Workspace preparation failed.", CCT_ERROR);
				return false;
			}
			
		if (!mkdir($temp))
		{
			$this->error->set("RInterface.prepare", 0002, "Workspace creation failed.", CCT_ERROR);
			return false;
		}
		
		$this->dba[0] = logon2( $_SERVER['PHP_SELF'] );
		if ($this->error->got(READONLY))
			return false;
		
		$this->temp = $temp;		
			
		return true;	
	}

	/**
	 * Preparing the workspace. Creating a subdirectory in temp. 
	 */
	function prepare2($temp)
	{
		$this->temp = $temp;			
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
			$this->error->set("RInterface.remove", 0001, "Could not open temporary directory.", CCT_ERROR);
			return false;
		}
		
		// ... and delete any file inside ...
		while($filename = readdir($dir)) 
		{
			if ($filename != "." && $filename != "..") 
				if (!unlink($tempdir . "/" . $filename))
				{
					$this->error->set("RInterface.remove", 0002, "Could not unlink tempory file: $filename.", CCT_ERROR);
					return false;
				}
		}

		// ... and remove directory itself
		closedir($dir);
 		if (!rmdir($tempdir))
		{
			$this->error->set("RInterface.remove", 0003, "Could not unlink tempory directory.", CCT_ERROR);
			return false;
		}
		
		return true;
	}
}




?>
