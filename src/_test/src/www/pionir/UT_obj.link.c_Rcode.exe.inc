<?php
/**
 * TEST: obj.link.c_Rcode.exe
 */
require_once ("insertx.inc");
require_once 'o.SATTACH.mod.inc';
require_once 'o.LINK.subs.inc';


class UT_obj_link_c_Rcode_exe_inc extends gUnitTestSub {
	
	private function clear_doc_object(&$sqlo, $docid) {
		// clear data
		$attachlib = new oSattachMod();
		$attachlib->setObj('LINK', $docid);
		
		$attach_read_lib = new cSattachSubs();
		$attachments = $attach_read_lib->getRelArrByKey( $sqlo, 'LINK', $docid, NULL );
		if (sizeof($attachments))  {
			foreach($attachments as $relid) {
				$attachlib->delAttach($sqlo, $relid);
			}
		}
	}
	
	/**
	 * load R code: 'appserver/RScript.R' to document "UT_obj.link.c_Rcode.exe"
	 *
	 * @see gUnitTestSub::dotest()
	 */
	function dotest( &$sqlo, $options ) {
		global $varcol, $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->_saveTest_def_later('TEST01','Activate R-Package later ...');
		return 1;
		
		$classid = $varcol->class_name_to_id( 'LINK', 'Rcode' );
		
		$docname = "UT_obj.link.c_Rcode.exe";
		$docid   = glob_elementDataGet( $sqlo, 'LINK', 'NAME', $docname, 'LINK_ID');
		if (!$docid) {
			// create document
			$insertlib = new insertC();
			$args = array( 
				"vals"=> array(
					'NAME'=>$docname
				), 
				"xobj"=>array( 'extra_class_id'=>$classid) );
			
			$docid   = $insertlib->new_meta($sqlo, 'LINK', $args);
		} else {
			$this->clear_doc_object($sqlo, $docid);
		}
		
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Error on Init.' );
			return;
		}
		
		// upload file
		$test_data_path = $this->_getTestDir_path();
		$Rcode_ori = $test_data_path.'/www/pionir/obj.link.c_Rcode.exe.R';
		
		$doclib = new oLinkUpload();
		$doclib->link_cp_file($sqlo, $Rcode_ori, $docid, 'obj.link.c_Rcode.exe.R', 'Rcode');
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 2, 'Upload Rcode failed.' );
			return;
		}
		
		
		// upload attachment
		$attach1_dir  = $test_data_path.'/www/pionir/obj.link.c_Rcode.exe.attach1.tex';
		if ( !file_exists( $attach1_dir ) ) {
			$error->set( $FUNCNAME,3, 'Attachment-UT does not exist. ('.$attach1_dir.')');
			return;
		}
		
		$attach1_name = basename($attach1_dir);
		$attachlib = new oSattachMod();
		$attachlib->setObj('LINK', $docid);
		$argin = array('NAME'=>$attach1_name);
		$rel_id = $attachlib->insertAtt($sqlo, $argin);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 4, 'Insert of attachemnt failed.' );
			return;
		}
		$attachlib->copyFile($rel_id, $attach1_dir);
		
		$unittest_workdir_name = __CLASS__;
		$workdirObj = new workDir();
		$workdir    = $workdirObj->getWorkDir ($unittest_workdir_name);
		
		// copy data file
		$datafile_name = 'obj.link.c_Rcode.exe.dat';
		$datafile_dir  = $test_data_path.'/www/pionir/'.$datafile_name;
		$this->_infoNow('INPUT: DATA-file:', $datafile_dir);
		if ( !file_exists( $datafile_dir) ) {
			$error->set( $FUNCNAME,5, 'Data-File does not exist. ('.$datafile_dir.')');
			return;
		}
		
		if (!copy($datafile_dir, $workdir.'/'.$datafile_name)) {
			$error->set( $FUNCNAME,6, 'Copy of Data-File failed to workdir. ('.$workdir.')');
			return;
		}
		
		$data_full_temp = $workdir.'/'.$datafile_name;
		if ( !file_exists( $data_full_temp) ) {
			$error->set( $FUNCNAME,7, 'Data-file does not exist on WORKDIR. ('.$data_full_temp.')');
			return;
		}
		
		$input_param   = $unittest_workdir_name.'/'.$datafile_name;
		
		$this->_infoNow('INPUT: DOC-ID:', $docid);
		$this->_infoNow('Work-Dir:', $workdir);
		
		
		// start test
		
		$RObj = new oLINK_cRcodeExe( $sqlo, $input_param, $docid );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 11, 'Init failed.' );
			return;
		}
		
		
		$RObj->checkR( $sqlo );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 12, 'Check failed.' );
			return;
		}
		
		$result = $RObj->execR( $sqlo );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 13, 'Exec failed.' );
			return;
		}
		
		$this->_infoNow('R-Environ: Work-Dir:', $result['R-workdir'] );
		
		//
		// check data: files on R-workpath
		//
		$expectfile = array('RInput', $attach1_name, 'ROutput', 'RScript');
		foreach($expectfile as $onefile) {
			$fullpath = $result['R-workdir'] .'/'.$onefile;
			if (!file_exists($fullpath)) {
				$this->_saveTestResult('TestFile: '.$onefile, 0, 'File not found');
				continue;
			}
			
			if (!filesize($fullpath)) {
				$this->_saveTestResult('TestFile: '.$onefile, 0, 'File contains no data.');
				continue;
			}
			
			$this->_saveTestResult('TestFile: '.$onefile, 1);
		}

		
		$retval = 1;
		return ($retval);
	}

}
