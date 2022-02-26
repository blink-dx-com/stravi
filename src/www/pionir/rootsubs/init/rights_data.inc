<?php
/**
 * - store ROLE rights data
 * - used by rights_insert.php
 * $Header: trunk/src/www/pionir/rootsubs/init/rights_data.inc 59 2018-11-21 09:04:09Z $
 * @package rights_data.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
 

function rights_get() {
	
	$object_names=array(
		'ABS_CONTAINER',
		//'ABS_CYTCART',
		'ABS_IMG',
		//'ABS_PRODUCT',
		'ABS_SOCKET',
		'ABS_WORKLIST',
		//'ABSTRACT_ARRAY',
		'ABSTRACT_PROTO',
		//'ABSTRACT_SPOT',
		'ABSTRACT_SUBST',
		'ACCEPT_PROT',
		// 'ARRAY_LAYOUT',
		// 'ARRAY_PRODUCTION',
		// 'A_SUBST_COHER',
		'A_CHIP_READER',
		//'CART_BATCH',
		//'CARTB_TMPL',
	    'AS_ASR',
		'CHIP_READER',
		//'CONCRETE_ARRAY',
		'CONCRETE_PROTO',
		'CONCRETE_SUBST',
		'CONTACT',
		'CONTAINER',
		'CONT_HAS_CSUBST',
		//'CYTCART',
	    'DB_USER',
		'DEVSIC',
		'EXP',
		'EXP_TMPL',
		'IMG',
		'JOUR_ENTRY',
		'LINK',
		'MODULE',
		//'PRODUCT',
		'PROJ',
	    'PUR',
		'REA_LOG',
		//'RESX',
		'SOCKET',
		'USER_GROUP',
		//'W_WAFER',
		//'W_WAFER_GROUP',
		//'W_CHIP_POSYX',
		//'W_BATCH_TMPL',
		//'W_TRAY',
		'WORKLIST'
		);
	
	
	
	
	//
	// POSITIVE functions rights                     
	//                     
	$function_names = array(
		'PaXml_import' 	=> 'allows PaXML import',
		'PaXml_export' 	=> 'allows PaXML export',
		//'ArrayBatch_allow_registration' => 'Allow registration of arrays in array batch.',
		'R_script_execute' => 'Right to execute R-package scripts',
		'objtab.import'    => 'allows bulk import of data',
		'o.CONCRETE_SUBST.acclog'       => 'allow adding of audit-trail entries for table "substance(concrete)"',
		'o.CONCRETE_SUBST.useRelease'  => 'user must use released QC-substances',
		// 'o.CONCRETE_SUBST.acclogRel'    => 'allow "release"-status in audit-trail for table "substance(concrete)"',
		//'o.LINK.c_queryAdvKeyGen' => 'advanced query document: generate authorization key',
		//'o.W_WAFER.acclog' => 'allow adding of access-logs for table "array-batch"',
		'o.S_OBJLINK.modify' => 'allow add/delete of object-link children (meta features of object)',
  		'g.login.NORM'     => 'use internal login method (ignore globals[DbLogin.method])',
		'g.QM_expert'	   => 'special functions for QM staff (e.g. special releases)',
		'g.QC_expert'	   => 'allow functions for QC staff (e.g. quality control lab)',
		'g.QC_expert_lev2' => 'QC staff level2 (e.g. reopen protocols)'
		);
		
	//
	// NEGATIVE functions rights                     
	//  
	$deny_function_names=array(                       
		'g.appPrefSave'  => 'Allow saving of user preferences',
		'Project_delobj' => 'Objects in projects become decativated instead of deleted.' // needed for workflow
						);
	
		
	$depricated_rig = array (  // "f.Project_deactivate_only",       
		 );
                
    return array($object_names, $function_names, $deny_function_names, $depricated_rig);
}