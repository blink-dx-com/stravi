<?php
/**
 * QC datafile analyses
 * - analyse an object-attachement
 *  
 * @package g.QC_datafile_analysis.inc
 * @swreq:  concept:0000937
 */
require_once ( "o.SATTACH.subs.inc" );
require_once ('f.workdir.inc');
require_once ('lev1/PythonApi.inc');
require_once ('f.directorySub.inc');

class g_QC_datafileWorker {
	var $dataFileShort; // only the name of the attachment-datafile
	var $rel_id;        // analysed REL_ID of SATTACH (attachment)
	
	function __construct( &$sqlo, $tablename, $id, $modScriptid) {
		$this->tablename = $tablename;
		$this->id = $id;
		$this->rel_id = 0;
		
		$this->ATTACH_KEY = 'data.%';
		
		$this->attachfileOri = NULL;  // original attachment file URL
		$this->dataFileShort = NULL; 
		$this->modScriptid=$modScriptid; // module script-ID
		$this->pyScriptBaseDir = $_SESSION['s_sessVars']['AppLabLibDir'].'/python/QC';
	}
	
	function setScript($pyScript, $rel_id ) {
		$this->pyScript = $pyScript ; // python-Script-name (without .py)
		$this->rel_id   = $rel_id;
	}
	
	// get possible python scripts
	function _getPyScripts() {
		$scriptArr= NULL;
		$basePath = $this->pyScriptBaseDir;
		$dirLib = new fDirextoryC();
		$files  = $dirLib->scanDir($basePath,'.py'); 
		
		if (!sizeof($files)) return;
		
		$filesOut=NULL;
		reset ($files);
		foreach( $files as $dummy=>$oneFile) {
			// remove '.py' extension
			$oneFileNoExt = basename($oneFile, ".py"); 
			$filesOut[$oneFileNoExt] = $oneFileNoExt;
		}
		reset ($files); 
		
		return $filesOut;
	}
	
	/**
	 * get all data-attachments
	 * @param $sqlo
	 * @return array of rel_id => attachment-name
	 */
	function getAttachments(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		$attachLib = new cSattachSubs();
		$seaopt    = array('search'=>'like');
		$key = $this->ATTACH_KEY;
		$relid_arr = $attachLib->getRelArrByKey( $sqlo, $this->tablename, $this->id, $key, $seaopt);
		if (!sizeof($relid_arr)) {
			$error->set( $FUNCNAME, 1, 'No attachments found with key "'.$key.'"' );
			return;
		}
		
		$selectArr=NULL;
		foreach( $relid_arr as $dummy=>$rel_id) {
			$datarow = $attachLib->getEntryByRelid( $sqlo, $this->tablename, $this->id, $rel_id);
			$selectArr[$rel_id]=$datarow['NAME'];
		}
		reset ($relid_arr); 
		return $selectArr;
	}
	
	/**
	 * get attachment of object
	 * @param  $sqlo
	 * @global $this->rel_id [INPUT]
	 * @global $this->attachfileOri [OUTPUT]
	 * @global $this->dataFileShort [OUTPUT]
	 * @return -
	 */
	function initObj(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':initObj';
		
		$this->attachNiceName = NULL;
		
		$attachLib = new cSattachSubs();
		
		$relid = $this->rel_id;
		if (!$relid) {
			$error->set( $FUNCNAME, 1, 'REL_ID missing' );
			return;
		}

		$datarow = $attachLib->getEntryByRelid( $sqlo, $this->tablename, $this->id, $relid);
		if (!is_array($datarow) )  {
			$error->set( $FUNCNAME, 1, 'No attachment found with rel_id "'.$relid.'"' );
			return;
		}
		$this->attachNiceName = $datarow['NAME'];
		
		$attachFile = $attachLib->getDocumentPath($this->tablename, $this->id, $relid);
		if (!file_exists($attachFile)) {
			$error->set( $FUNCNAME, 2, 'Attachment-file REL_ID:'.$relid.' not exists.' );
			return;
		}
		$this->attachfileOri = $attachFile;
		$this->dataFileShort = basename($this->attachfileOri);
		
		echo 'Attachment: '.$this->attachNiceName.'<br />'."\n";
	}
	
	function _startPythonCode() {
		global $error;
		$FUNCNAME= __CLASS__.':startPythonCode';
		
		$infileShort = $this->dataFileShort;
		$pythonCode  = $this->pyScriptBaseDir .'/'. $this->pyScript.'.py'; 
		
		$statsPythonPck = $_SESSION['s_sessVars']['AppRootDir'].'/pythonlib/StatsHelp.py'; // needed package
		$workdir     = $this->workPathFull;
	    
	    // RInterface
	    $pyLib	= new PythonApi(  );
	    
	    if (!$pyLib->prepare2($workdir)) {
	        $error->printLast();
	    }
	    
	    if (!$pyLib->prepareScript($pythonCode)) {
	        $error->set( $FUNCNAME,3, "script prepare failed.");
	        return;
	    }   
	    $pyLib->addPythonModule($statsPythonPck);
	   
	    $result = $pyLib->runScript($infileShort);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME,4, "running script failed.");
	        return;
	    }    
		
		if ($_SESSION["userGlob"]["g.debugLevel"]> 0)
			glob_printr( $result, $FUNCNAME.": result info" );
		
	    
	    if (is_array( $result['resultfiles']) ) {
	    	reset ($result['resultfiles']);
			foreach( $result['resultfiles'] as $dummy=>$valarr) {
				$filename = $valarr['name'];
				echo 'file: '.$filename.'<br>';
				echo 'Erklaerung: ' . htmlspecialchars($valarr['notes']).'<br>';
				echo '<img src="f_workfile_down.php?file='.$this->modScriptid.'/'.$filename.'"></a>';
				
			}
			reset ($result['resultfiles']); 
	    }
	    
	    return $result;
	}
	
	function startAnalysis(&$sqlo) {
		
		// create work-dir
		$workdirObj = new workDir();
		$this->workPathFull  = $workdirObj->getWorkDir ( $this->modScriptid );
		
		copy( $this->attachfileOri, $this->workPathFull.'/'.$this->dataFileShort );
		$answerDict = $this->_startPythonCode();
		
		
	}
}


/**
 * QC datafile analyses
 * - analyse an object-attachement
 * 
 *    
 * @swreq:  concept:0000937
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['t']  tablename
 * @param $_REQUEST['id'] id of object
 * @param $_REQUEST['go'] 
 * 	 0 : show python-file select form
 *   1 : start analysis
 * @param $parx['script'] Python-file-name (without *.py) based in the resource-dir "www/lab/python"
 * @param $parx['rel_id'] REL_ID of attachment (needed on $go=1
 * 
 *   
 */
class g_QC_datafile_analysis_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= $_REQUEST['t'];
		$title		= 'Analyse attachment-datafiles';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = 'g.QC_datafile_analysis';
		$this->infoarr['title']    = $title;
		#$this->infoarr['help_url'] = '';
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);
		
	}

	
	function form1(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':form1';
		
		$scriptArr = $this->analysisLib->_getPyScripts();
		$attachArr = $this->analysisLib->getAttachments($sqlo);
		
		if (!sizeof($scriptArr)) {
			$error->set( $FUNCNAME, 1, 'no Python-scripts found for this tool.' );
			return;
		}
		if (!sizeof($attachArr)) {
			$error->set( $FUNCNAME, 1, 'no attachments on this object.' );
			return;
		}
		
		$initarr   = NULL;
		$initarr["title"]       = "Select the Python-script and attachment";
		$initarr["submittitle"] = "Next";
		$initarr["tabwidth"]    = "AUTO";
		
		$hiddenarr=NULL;
		$hiddenarr['t'] =$_REQUEST['t'];
		$hiddenarr['id']=$_REQUEST['id'];
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		
		
		$fieldx = array ( 
			"title" => "Attachment", 
			"name"  => "rel_id",			
			"notes" => "the used attachment"
			 );
		if (sizeof($attachArr)==1) {
		    
			$rel_id=key($attachArr);
			$name  =current($attachArr);
			
			$fieldx["val"]   = $rel_id;
			$fieldx["object"] = 'info2';
			$fieldx["inits"]  = $name;
		} else {
			$fieldx["object"] = 'select';
			$fieldx["inits"]  = $attachArr;
		}
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array ( 
			"title" => "Python-script", 
			"name"  => "script",
			"object"=> "select",
			"val"   => $parx['script'], 
			"inits" => $scriptArr,
			"notes" => "the python -scripts"
			 );
		$formobj->fieldOut( $fieldx );
		
		
		
		$formobj->close( TRUE );
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		
		$this->analysisLib = new g_QC_datafileWorker($sqlo, $_REQUEST['t'], $_REQUEST['id'], $this->infoarr['scriptID']);
		
		if ( $go==0 ) {
			$this->form1($sqlo);
			return;
		}
		
		if ($go==1) {
			if ( $parx['script']==NULL ) {
				$this->htmlFoot('ERROR','Parameter: Missing script-name.');
			}
			if ( $parx['rel_id']==NULL ) {
				$this->htmlFoot('ERROR','Parameter: Missing rel_id.');
			}
			
			$this->analysisLib->setScript($parx['script'], $parx['rel_id']);
			
			$this->analysisLib->initObj($sqlo);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'initobj failed.' );
				return;
			}
			
			$this->analysisLib->startAnalysis($sqlo);
		}
	}
	
}
