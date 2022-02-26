<?php
require_once 'date_funcs.inc';

/**
 * lib for SUA version management; only analysis methods
 * @package o.SUA.versman.sub.inc
 * @swreq   SREQ:0003315: o.ABSTRACT_SUBST > [VersMan] > library 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oSUA_versman_sub {
	
	private $versmanActive;
	
	public function __construct() {
		$this->versmanActive=0;
		if (glob_column_exists('ABSTRACT_SUBST', 'VERSFLAG')) {
			$this->versmanActive=1;
		}
	}
	
	public function versman_active() {
		return $this->versmanActive;
	}
	
	/**
	 * get info, if SUA is: 
	 *  0: normal SUA (no Version management)
	 *  1: is a VersionMaster-SUA 
	 *  2: is a SUA with VERSION-management
	 * @param  $sqlo
	 * @param  $suaid
	 * @return $typex
	 */
	public function getSUA_type(&$sqlo, $suaid) {
		if (!$this->versmanActive) return 0;
		
		$sua_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$suaid),
			 array('VERSFLAG') );
			 
		if (!$sua_feats['VERSFLAG']) $sua_feats['VERSFLAG']=0; // set to a defined numeric value
			 
		return $sua_feats['VERSFLAG'];
	}
	
	/**
	 * get version number
	 * @param  $sqlo
	 * @param  $suaid
	 * @return INT $version
	 */
	public function getVersion($sqlo, $suaid) {
		return 1; // TBD: must be programmed !!!
	}
	
	/**
	 * get ID of MasterVersion-SUA
	 * @param  $sqlo
	 * @param  $suaid
	 * @return long id of MasterVersion-SUA
	 */
	public function getMasterID($sqlo, $suaid) {
		if (!$this->versmanActive) return 0;
		
		$sua_feats = glob_elemDataGet3( $sqlo, 'SUA_VERS', array( 'CHILD_ID'=>$suaid), array('ABSTRACT_SUBST_ID') );
		return $sua_feats['ABSTRACT_SUBST_ID'];
	}
	
	/**
	 * get valid SUA-ID from MasterVersion-SUA
	 * - version must be valid in DATE-range
	 * @param  $sqlo
	 * @param  $suaid
	 * @return long id of ChilD
	 */
	public function getValidSuaFrMaster(&$sqlo, $masterid) {
		if (!$this->versmanActive) return $masterid;
		
		$now        = time();
		$nowSQL     = $sqlo->Timestamp2Sql($now);
		$nowVersEnd = datex_shift($now, 'dayStart');
		$nowEndSQL  = $sqlo->Timestamp2Sql($nowVersEnd);
		
		$sqlsel = "CHILD_ID from SUA_VERS where ABSTRACT_SUBST_ID=".$masterid.
			" and DATE_START<=".$nowSQL. ' and DATE_END>='.$nowEndSQL;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$childid = $sqlo->RowData[0];
		
		// TBD: what to do, if more versions ?
		
		return $childid;
		
	}
	
	/**
	 * get main version params of (childid)
	 * @param  $sqlo
	 * @param  $childid
	 * @return array('XVERS', 'DATE_START', 'DATE_END', 'ABSTRACT_SUBST_ID')
	 */
	public function getMainParams($sqlo, $childid) {
		if (!$this->versmanActive) return 0;
		$colarr = array('XVERS', 'DATE_START', 'DATE_END', 'ABSTRACT_SUBST_ID');
		$vers_feats = glob_elemDataGet3( $sqlo, 'SUA_VERS', array( 'CHILD_ID'=>$childid), $colarr );
		return $vers_feats;
	}
	
}

class oSUA_versman_child {
	/**
	 * attributes of version
	 * @var array $vers_feats
	 *  'XVERS', 
	 *  'DATE_START', 
	 *  'DATE_END', 
	 *  'ABSTRACT_SUBST_ID', 
	 *  'NOTES'
	 * 	'versflag' -- atribute SUA:VERSFLAG
	 */
	private $vers_feats;
	
	/**
	 * init child
	 * @param  $sqlo
	 * @param  $childid
	 */
	public function setChild(&$sqlo, $childid) {
		$colarr = array('XVERS', 'DATE_START', 'DATE_END', 'ABSTRACT_SUBST_ID', 'NOTES');
		$this->vers_feats = glob_elemDataGet3( $sqlo, 'SUA_VERS', array( 'CHILD_ID'=>$childid), $colarr );
		
		$sua_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$childid),
			 array('VERSFLAG') );
		$this->vers_feats['versflag']=$sua_feats['VERSFLAG'];
		
		return $this->vers_feats;
	}	
	
	/**
	 * get Type: 
	 *  0: normal SUA (no Version management)
	 *  1: is a VersionMaster-SUA 
	 *  2: is a SUA with VERSION-management
	 */
	public function getSuaType() {
		return $this->vers_feats['versflag'];
	}
	
	public function getVersFeats() {
		return $this->vers_feats;
	}
	
	/**
	 * get "version is valid" flag
	 * -1 : no dates defined
	 * 1 : not yet active
	 * 2 : isActive
	 * 3 : finished
	 */
	public function getValidFlag() {
		$now = time();
		
		if ($this->vers_feats['DATE_START']==NULL or $this->vers_feats['DATE_END']==NULL) {
			return -1;
		}
		
		$unix1 = date_str2unix($this->vers_feats['DATE_START'], 5);
		$unix2 = date_str2unix($this->vers_feats['DATE_END'], 5);
		
		// round UP: e.g. value: "2013-03-12 00:00" => will check against 2013-03-13
		$tmpDateArr = adodb_getdate($unix2);
		if ($tmpDateArr["hours"]==0 and $tmpDateArr["minutes"]==0) {
			// round up one day
			$unix2 = $unix2 + date_unixDay2Sec(1)-1;
		}
		
		if ($now<$unix1) {
			return 1;
		}
		
		if ($now>=$unix1 and $now<=$unix2) {
			return 2;
		}
		
		if ($now>=$unix2) {
			return 3;
		}
		
	}
}
