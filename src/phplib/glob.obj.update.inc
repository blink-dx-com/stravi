<?php
/**
 * @namespace core::obj
 * @package glob.obj.update.inc
 */
require_once ('globals.inc');
require_once ('db_x_obj.inc'); 
require_once ('o.S_VARIO.mod.inc');
require_once ('f.update.inc');
require_once 'glob.obj.touch.inc';

// contains PHP5-code

/**
 * manage UPDATE of ONE object ( only Single-PrimaryKey objects ! )
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * $Header: trunk/src/phplib/glob.obj.update.inc 59 2018-11-21 09:04:09Z $
 */
class globObjUpdate {
 
	/**
	 * update attributes of one object
	 * - do access check at beginning
	 * - with ADVMOD (advanced modification logging)
	 *  
	 * @param object &$sqlo     sql handle    
	 * @param string $tablename name of table 
	 * @param array $args	    TYPEOF objFeatStruct = <pre>
		 array( "vals"=>..., "xobj"=>... )
			 "vals" => main object parameters
			   array [COL] = val
			   should NOT contain CCT_ACCESS_ID
			   should NOT contain a PK, if it is a single PK-object
			   EXTRA_OBJ_ID will be removed
			"xobj" => array ( 
					'extra_class_id' => INT or NULL (than delete the old data)
					'values' 		 => array(name=>val) 
					 )
			"vario" => array(
				key => val
				)
	   </pre>
	 * @param array $option (upd_opt_STRUCT) NULL <pre>
	 *	  "types" = array [COL_NAME] => type 
	 *			type: "DATE1" :: YYYY-MM-DD HH24:MI:SS
	 *    "typeAuto" : [0], 1 : analyse format automatically, e.g. for "DATE"
 	 *			ignores "types"cd 
	 *	  "noRightCheck" [0], 1 </pre>
	 * @throws exception errors on update-error
	 * 	1 : gObjUpdate::update_row error
	 * @return -
	 */
	function update_meta( &$sqlo, $tablename, $objid, $args, $option = NULL ) {
		global $varcol;
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$varcol = & Varcols::get();
		$actarr = array(
		    'd'=>array('x'=>array()),
		    'a'=>array('key'=>'mod')
		    
		);
		
		// access check ?
		// do not throw error on "noRightCheck"=1
		if ( $option["noRightCheck"] ) {
		    $o_rights=array();
		} else {
    		$o_rights = access_check($sqlo, $tablename, $objid);
    		$right='write';
    		if ( !$o_rights[$right]  ) {
    			$error->set( $FUNCNAME, 3, 'You do not have '.$right.
    				' permission on object '.$tablename.' ID:'. $objid.' !' );
    			return;
    		}
		}
		
		$advmodLib   = NULL;
		$advmod_info = array();
		// do advanced data logging
		
		require_once ('glob.obj.advmod.inc');
		$advmodLib   = new obj_advmod();
		$advmod_info = $advmodLib->setObj($sqlo, $tablename, $objid);
		if ($advmod_info['dolog']>0) {
			$advmodLib->getOriData($sqlo);
		}
		
		
		$doTouch = 0;
		$touchRet= array('timest'=>time() );
		if ( $args['xobj'] != NULL ) {

		    if ($args['xobj']['extra_class_id']) {
			     $class_name  = $varcol->class_id_to_name( $args['xobj']['extra_class_id'] );
		    } else {
		         $class_name  = NULL;
		    }
			$XobjLib = new fVarcolMeta($sqlo);
			$XobjLib->setObj( $sqlo, $tablename, $objid );
			$XobjLib->updateByClsName( $sqlo, $class_name, $args['xobj']['values'] );
			$doTouch = 1;
			$actarr['d']['x']['xob']=NULL;
		}
		
		if ( $args['vals'] != NULL ) {
			$pkName  = PrimNameGet2($tablename);
			$newArgs = $args['vals'];
			$newArgs[$pkName] = $objid;
			
			$optTypeArr = $option["types"];
			
			// generate auto-format, e.g. for DATE
			if ($option["typeAuto"]>0) {
				$optTypeArr = NULL;
				// TBD: may cache this array or use cached function !!! ...
				$colTypes = appRawTypeGet($sqlo,$tablename); 

				foreach( $colTypes as $col=>$typex) {
					if ($typex=='DATE') $optTypeArr[$col]="DATE1";
				}
			
				$option['types'] = $optTypeArr;
			}
			
			$option["noRightCheck"]=1; // will be done at beginning of method update_meta
			$option["touch"] = -1; 
			$retval = gObjUpdate::update_row( $sqlo, $tablename, $newArgs, $option );
			if ($retval<1 or $error->Got(READONLY)) {
				$error->set( $FUNCNAME, 1, 'error on gObjUpdate::update_row; suberror:'.$retval );
				return;
			}
			$doTouch = 1;
			$actarr['d']['x']['fea']=NULL;
			
		}
		
		if ( is_array($args['vario']) ) {
			
			$varioLib = new oS_VARIO_mod();
			$varioOpt = array('noAccCheck'=>1);
			$varioLib->setObject( $sqlo, $tablename, $objid, $varioOpt );
			$varioLib->set_touched(1);
			
			if ($advmod_info['dolog']>0) {
			    $vkeys = array_keys($args['vario']);
			    $advmodLib->getOriData_Vario($sqlo, $vkeys);
			}

			foreach( $args['vario'] as $key=>$val) {
				$varioLib->updateKeyVal($sqlo, $key, $val);
			}
			 
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 2, 'error on vario-update.' );
				return;
			}
			$actarr['d']['x']['var']=NULL;
			$doTouch = 1;
		}

		
		if ($advmod_info['dolog']>0 and !$error->Got(READONLY) ) {
		    $timeStamp = time();
		    $touchInfo = array('timest'=>$timeStamp ); // TBD: must come in FUTURE from gObjUpdate::update_row !
			$advmodLib->getPostData($sqlo, $touchInfo);
			if ($error->Got(READONLY))  {
				$error->logError(); // but no user output ...
			}
		}
		
		if ( $doTouch ) {
		    globObjTouch::touch( $sqlo, $tablename, $objid, $actarr);
		}
		
	}
	
	/**
	 * RAW update, do not check access-rights, can be used for system objects
	 * @param array $args 	TYPEOF objFeatStruct = 
	 * @return -
	 */
	static function update_raw(&$sqlo, $tablename,&$args, $idarr) {
		$argu=$args['vals'];
		gObjUpdate::update_row_s( $sqlo, $tablename,$argu,$idarr);
	}
 
}