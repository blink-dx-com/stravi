<?php
/**
 * contains function for single object preferences
 * 
 * @package obj.abstract_subst.xspref.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class o_CONCRETE_SUBST_XSPREF  extends o_xspref_ABS {
    
    const PREF_VAR_NAME= 'o.CONCRETE_SUBST.xspref';
    const PRC_var      = 'o.CONCRETE_PROTO.show';

    function singlePref( &$sqlo, $info) {
    	prefEmptLine();
    	
    	$tmpopt     = unserialize($_SESSION['userGlob'][self::PREF_VAR_NAME]);
    	//$tmpopt_PRC = unserialize($_SESSION['userGlob'][self::PRC_var]);
    	
    	// set a var for CONCRETE_PROTO
    	$this->prefNameShow( "Mark mandatory step fields" );
    	$vartmp = "showRequ";
    	$infotmp= $tmpopt[$vartmp];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[o.CONCRETE_PROTO.show.".$vartmp."]\" value=\"1\" ".$checked_tmp."></td>";
    	$this->prefClose ("In the Protocol-steps: mark quantity, substance and other fields, which have to be filled.");
    
    	$this->prefNameShow( "show related journal entries" );
    	$vartmp = 'JOUR_ENTRY.sh';
    	$infotmp= $tmpopt[$vartmp];
    	
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[".$vartmp."]\" value=\"1\" ".$checked_tmp."> </td>";
    	echo "</td>";
    	$this->prefClose ("Automtically shows related ".tablename_nice2('JOUR_ENTRY') );
    	
    	$this->head2('<img src="images/icon.CONCRETE_PROTO.gif"> Prep protocol tab');
    	
    	
    	$infotmp= $_SESSION['userGlob']['o.CONCRETE_PROTO.qm_check'];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	$field= "<input type=checkbox name=\"setti[o.CONCRETE_PROTO.qm_check]\" value=\"1\" ".$checked_tmp.">";
    	$this->pref_full_row("QM-warnings", $field, 'Show QM Warnings ?');
    	
    	$infotmp= $_SESSION['userGlob']['o.CONCRETE_PROTO.nonqm_check'];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	$field= "<input type=checkbox name=\"setti[o.CONCRETE_PROTO.nonqm_check]\" value=\"1\" ".$checked_tmp.">";
    	$this->pref_full_row("Non-QM-warnings", $field, 'Show Non-QM Warnings ?');
    	
    	$infotmp= $_SESSION['userGlob']['o.CONCRETE_SUBST.plan_check'];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	$field= "<input type=checkbox name=\"setti[o.CONCRETE_SUBST.plan_check]\" value=\"1\" ".$checked_tmp.">";
    	$this->pref_full_row("Planned parameters check?", $field, 
    	    'Show Planned parameters check warnings? These parameters are defined in the material order.');
    	
    	
    	$this->head2('<img src="images/icon.CONCRETE_PROTO.gif"> QC protocol tab');
    	$infotmp= $_SESSION['userGlob']['o.CONCRETE_SUBST.sh.proto_sample_use'];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	$field= "<input type=checkbox name=\"setti[o.CONCRETE_SUBST.sh.proto_sample_use]\" value=\"1\" ".$checked_tmp.">";
    	$this->pref_full_row("Show other protocols", $field, 'Show all Protocols, where this MAC is used as SAMPLE.');
    }
    
    function prefSave( &$sql, $info, $setti ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        $tmpopt = unserialize($_SESSION['userGlob'][self::PREF_VAR_NAME]);
        //$tmpopt_PRC = unserialize($_SESSION['userGlob'][self::PRC_var]);
    	
        $flagname="showRequ";
        $tmp_val = $setti["o.CONCRETE_PROTO.show.".$flagname] ;
        $tmpopt[$flagname] = $tmp_val;
        
        $formvar = 'JOUR_ENTRY.sh';
        $tmpopt[$formvar] = NULL;
        $tmp_val = $setti[$formvar] ;
        if ($tmp_val!=NULL) $tmpopt[$formvar] = $tmp_val;
        else unset($tmpopt[$formvar]);
    
        $_SESSION['userGlob'][self::PREF_VAR_NAME] = serialize($tmpopt);
        // $_SESSION['userGlob'][self::PRC_var] = serialize($tmpopt_PRC);

        
        $_SESSION['userGlob']['o.CONCRETE_PROTO.qm_check']    = $setti["o.CONCRETE_PROTO.qm_check"] ;
        $_SESSION['userGlob']['o.CONCRETE_PROTO.nonqm_check'] = $setti["o.CONCRETE_PROTO.nonqm_check"] ;
        $_SESSION['userGlob']['o.CONCRETE_SUBST.plan_check']  = $setti["o.CONCRETE_SUBST.plan_check"] ;
        $_SESSION['userGlob']['o.CONCRETE_SUBST.sh.proto_sample_use']  = $setti["o.CONCRETE_SUBST.sh.proto_sample_use"] ;
        
        
    }
}
