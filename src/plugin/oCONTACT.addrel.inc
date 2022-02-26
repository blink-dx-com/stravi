<?php
require_once('o.S_OBJLINK.subs.inc');
require_once('o.S_OBJLINK.mod.inc');

/**
 * add related CONTACT $relid to mother contact $id
*
* @package oCONTACT.addrel.inc
* @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
* @swreq  UREQ:0003969: o.CONTACT > add related contact
* @param  $relid ID of related contact
*/
class oCONTACT_addrel_XPL extends gPlugin {

	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}

	function register() {
		$tablename	= 'CONTACT';
		$title		  = 'Add releated contact';

		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj';
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;


		$this->infoarr2 = array(
				'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array('write') )
		);

	}

	/**
	 * update relation
	 * @param  $sqlo
	 * @param  $relid
	 * @return number
	 * 		1: already linked
	 *      2: update o.k.
	 */
	function update($sqlo, $relid) {
		
		$IS_LINKED = 1;
		$UPDATE_OK = 2;
		
		// relation exists ?
		$linkSub    = new oS_OBJLINK_subs();
		$table      = 'CONTACT';
		$motherID   = $this->objid;
		
		// check link: mother => relid
		$isLinked = $linkSub->chiObjLinked($sqlo, $table, $motherID, $table, $relid);
		if ($isLinked) return $IS_LINKED;
		
		// check link: relid => mother
		$isLinked = $linkSub->chiObjLinked($sqlo, $table, $relid, $table, $motherID);
		if ($isLinked) return $IS_LINKED;
		
		$modlib = new oS_OBJLINK_mod();
		$modlib->setObj($sqlo, $table, $motherID);
		$modlib->addLink($sqlo, $table, $relid, 'relation');
		
		return $UPDATE_OK;
	}


	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';

		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		
		if (!is_numeric($_REQUEST['relid'])) {
			$this->htmlFoot('ERROR','Input parameter "Related Contact" missing.');
		}
		
		$relid = intval($_REQUEST['relid']);
		
		if ($relid==$this->objid) {
			$this->htmlFoot('ERROR','Can not link to yourself!');
		}

		
		if (!gObject_exists ($sqlo, 'CONTACT', $relid) ) {
			$this->htmlFoot('ERROR','Contact with ID='.$relid.' not exists.');
		}
		
		$answer = $this->update($sqlo, $relid);
		if ($answer!=2) {
			if ($answer==1) $this->htmlFoot('ERROR','Contact with ID='.$relid.' already linked.');
		}
		
		if ($error->Got(READONLY))  {
			return;
		}
		
		$this->_html->pageForward('edit.tmpl.php?t=CONTACT&id='.$this->objid , 'Back to contact');

	}

}