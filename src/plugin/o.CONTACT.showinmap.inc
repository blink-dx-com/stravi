<?php
/**
 * o.CONTACT > show contact location in Google-Maps
 *    
 * @package o.CONTACT.showinmap.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:4452 o.CONTACT > show conatct location in Google-Maps
 *   
 */
class o_CONTACT_showinmap_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CONTACT';
		$title		  = 'show contact in Google-Maps';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);
		
	}


	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		
		$keys=array('COUNTRY','CITY', 'STREET');
		
		$features = glob_elemDataGet3( $sqlo, 'CONTACT', array('CONTACT_ID'=>$this->objid), array('CON_CONTACT_ID', 'COUNTRY','CITY', 'STREET') );
		
		if ($features['COUNTRY']==NULL or $features['CITY']==NULL or $features['STREET']==NULL) {
			// get master
			if ($features['CON_CONTACT_ID']>0) {
				$featuresMaster = glob_elemDataGet3( $sqlo, 'CONTACT', array('CONTACT_ID'=>$features['CON_CONTACT_ID']), array('COUNTRY','CITY', 'STREET') );
			}
			
			// merge data
			
			reset ($keys);
			foreach( $keys as $dummy=>$key) {
				if ($features[$key]==NULL) {
					$features[$key] = $featuresMaster[$key];
				}
			}
			
		}
		
		// need at least city and street
		if ($features['CITY']==NULL) {
			$this->htmlFoot("WARN",'No laction data exists in this contact. Need at least a City-Name');
		}
		
		// build location query
		$urlquery = NULL;
		reset ($keys);
		foreach( $keys as $dummy=>$key) {
			if ($features[$key]!=NULL) {
				$urlquery .= $komma . $features[$key];
				$komma = ', ';
			}
		}
		
		
		
		// build google request
		$desturl = 'http://maps.google.com/?q='.urlencode($urlquery);
		
		$this->_html->windowOpen($desturl,'show contact in Google Maps ...');
	}
	
}