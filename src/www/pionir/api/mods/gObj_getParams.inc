<?php
require_once("object.info.inc");
require_once("globals.inc");
require_once 'o.PROJ.subs.inc';

/**
 * get parameters of an BO
 * @see also method gObj_getAssParams() for associated tables
 * @author  Steffen Kube
 * @package WEBAPI\gObj_getParams
 */
class gObj_getParams_XRPC extends XRPC_BASE {

	
	
	function __checkParams() {
		global $error;
		$FUNCNAME= '__checkParams';
		
		$sqlo = $this->_sqlo;
		
		$tablename = $this->_params['t'];
		$objectId = $this->_params['id'];
		$columns  = $this->_params['cols'];
		

		$this->__objectCheck($sqlo, $tablename, $objectId);
		if ( $error->got(READONLY) ) {
			return -1;
		}	
    
		// get visible columns
		if ( !is_array($columns) or !sizeof($columns) ) {
		    $columns = array();
		    $cols_tmp = columns_get2($tablename); 
		    foreach($cols_tmp as $colx) {
		        $featx = colFeaturesGet( $sqlo, $tablename, $colx);
		        if ($featx['VISIBLE']>=1) {
		            $columns[]=$colx;
		        }
		    }
		    $this->_params['cols'] = $columns; // write back ...
		    
		    // OLD error 
			// $error->set( $FUNCNAME, 5, 'no columns given' );
			// return -5;
		}
		
		// check column names
		
		$badcol=NULL;
		$komma=NULL;
		foreach($columns as $colx) {
			$featx = colFeaturesGet( $sqlo, $tablename, $colx);
			if ( !isset($featx['VISIBLE']) or $featx['VISIBLE']<1) {
				$badcol .= $komma . $colx;
				$komma=',';
			}
		}
		
		if ( $badcol!=NULL ) {
			$error->set( $FUNCNAME, 6, 'column(s) "'.$badcol.'" not possible.'  );
			return -6;
		}
			
		return 1;
	}
	
	/**
	 * get features of one object:
	 * 
	 *   - values of selected columns in return["vals"]
	 *   - if EXTRA_OBJ_ID  given as column: extra-object values, class in return["xobj"]
	 *   - if CCT_ACCESS_ID given as column: return access_data in return['access']
	 *   - if options['vario']=1 : return vario-data in return['vario']
	 *   
	 *  @global INPUT  array $this->f_param 
	 *  ``` <pre>
	 *	 [t]	tablename,
	 *   [id]	the object-ID 	
	 *   [cols] (OPTIONAL)  array(columns),
	 *   [opt]	(OPTIONAL)  options:
	 *       'vario' => 0,1 : get vario-values?
	 *       'nice'  => 0,1 : get nice column names + nice values : return['nice']
	 *       'folders' => 0,1 : array of projects, where it is linked in 
	 * 	``` </pre> 
	 * @example: 
	 * ``` 
	 * get Notes from EXP_TMPL for EXP_TMPL_ID 56
	 *   IN: array('t'=>'EXP_TMPL', 'id'=>56, 'cols'=>array('NAME','NOTES',
	 *          'EXTRA_OBJ_ID', 'CCT_ACCESS_ID') )
	 * 	  OUT: Array (
     * [0] => 0
     * [1] => Array
     *     (
     *         [vals] => Array
     *             (
     *                 [NAME] => LeAn-5F7
     *                 [NOTES] => notes
     *                 [EXTRA_OBJ_ID] => 357176
     *                 [CCT_ACCESS_ID] => 1193385
     *             )
     *         [xobj] => Array
     *             (
     *                 [extra_class_id] => 882
     *                 [values] => Array
     *                     (
     *                         [accession] => 12345
     *                         [start] => 34
     *                         [end] => 
     *                         [strand] => 
     *                         [sequence] => ATGTTAT
     *                     )
     *             )
     *         [access] => Array
     *             (
     *                 [CCT_ACCESS_ID] => 1193385
     *                 [DB_USER_ID] => 480
     *                 [WIID] => 
     *                 [TABLE_NAME] => ABSTRACT_SUBST
     *                 [DB__DB_USER_ID] => 1
     *                 [CREA_DATE] => 2008-06-16 14:37:37
     *                 [MOD_DATE] => 2013-10-11 11:57:24
     *                 [ROID] => 
     *                 [ARCH_DATE] => 
     *                 [EXPORT_DENY] => 
     *                 [H_ALOG_ACT_ID] => 
     *                 [UPINFO] => 
     *                 [ORI_ID] => 
     *                 [WFLOW_ID] => 
     *             )
     *
     *         [vario] => Array
     *             (
     *                 [ExpiryDays] => 134
     *                 [rel.policy.4eye] => -1
     *             )
     *     )
     * )
     * ``` 
	 * </pre>
	 * @return array objFeatStruct + extra
	 * ``` 
	 *   'vals => array()     -- objFeatStruct
	 *   'access'=> array()  -- objFeatStruct
     *   'nice' =>  if $opt['nice'] > 0
     *      {  'vals'=>array( COL => array('valnice'=>, 'colnice'=>) )  }
     *   'folders' = array of linked folders -- if  $opt['folders'] > 0
     * ``` 
	 * @throws exception
	 */
	function &run() {
		$objInfo = new objInfoC();
		$sqlo = $this->_sqlo;
		
		$tablename= $this->_params['t'];
		$objectId = $this->_params['id'];
		$column   = $this->_params['cols'];
		$options  = $this->_params['opt'];
    
    	$opt=NULL;
		$opt['cols'] = $column ;
    	$opt["typeAuto"]= 1;
    	if ($options["vario"]>0) $opt["vario"]= 1;
		
    	$datacube = $objInfo->getParamsRaw($sqlo, $tablename, $objectId, $opt);
    	
    	if ($options['nice']>0) {
    	    $data_nice  = $objInfo->paramRaw2nice($sqlo, $tablename, $datacube );
    	    $datacube['nice'] = $data_nice;
    	}
    	
    	if ($options['folders']>0) {
    	    
    	    $datacube['folders']=NULL;
    	    $projs = cProjSubs::getProjectsByObject($sqlo, $tablename, $objectId );
    	    if (is_array($projs)) {
    	        $projs_out = array();
    	        foreach($projs as $proj_id) {
    	            $proj_nice = obj_nice_name ( $sqlo, 'PROJ', $proj_id );
    	            $projs_out[]=array('id'=>$proj_id, 'name'=>$proj_nice);
    	        }
    	        $datacube['folders'] = $projs_out;
    	    }
    	}
    	
    	return $datacube;
	}
}