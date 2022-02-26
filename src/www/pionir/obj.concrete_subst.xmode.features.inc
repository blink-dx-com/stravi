<?php
/**
 * www/pionir/obj.concrete_subst.xedit.php 
 */

require_once ("date_funcs.inc"); 
require_once ('f.help.inc');
require_once ('f.msgboxes.inc');
require_once ("o.CONCRETE_SUBST.subs.inc"); 

require_once('gui/o.SATTACH.imgshow.inc');
require_once 'gui/o.CONCRETE_SUBST.protos_show.inc';

class oCONCRETE_SUBST_xHelp {
    
    const PREF_VAR_NAME='o.CONCRETE_SUBST.xspref';
    public $user_prefs;
    
    function __construct($id, $sua_id, &$argu) {
        $this->id = $id;
        $this->argu = &$argu;
        $this->ABSTRACT_SUBST_ID = $sua_id;
        $this->greenball = "<img src=\"images/greenball.gif\" hspace=5>";
        $this->user_prefs = unserialize($_SESSION['userGlob'][self::PREF_VAR_NAME]);
    }
    
    function sub_title($title) {
        echo $this->greenball.'<span style="color:gray;">'.$title.':</span>';
    }
    
    function sh_deviation($sqlo) {
        if ($this->argu['DEVIATION']==NULL) return; 
        echo "<ul>\n";
        cMsgbox::showBox("warning", "<b>Deviations:</b> ". htmlspecialchars($this->argu['DEVIATION'])); 
        echo "</ul>\n";
    }
    
    // show SUC or SUA thumbnail
    function showThumbnail(&$sqlo) {
        
        $tablename='CONCRETE_SUBST';
        
        $attachLib = new oSATTACH_imgshow();
        
        $relid = $attachLib->getThumbAttach( $sqlo, $tablename, $this->id);
        if ($relid) {
            $htmlTmp = $attachLib->getImgTag($relid);
            echo $this->greenball.'<span style="color:gray;">Thumbnail:</span><ul>'.$htmlTmp.'</ul>'."\n";
            return;
        }
        
        // no thumbnail cor CONCRETE, look for the abstract
        
        $tablename='ABSTRACT_SUBST';
        
        $relid = $attachLib->getThumbAttach( $sqlo, $tablename, $this->ABSTRACT_SUBST_ID);
        if ($relid) {
            $htmlTmp = $attachLib->getImgTag($relid);
            echo $this->greenball.'<span style="color:gray;">Thumbnail:</span><ul>'.$htmlTmp.'</ul>'."\n";
        }
    }
    
    function expiry_date($sqlo, $expiry_date) {
        
        $greenball = $this->greenball;
        $QC_text = "";
        
        $expiryUnx = date_dtsql2unix( $sqlo, $expiry_date);
        $expiryLib = new oConcSubst_subC();
        $expirRes  = $expiryLib->check_ExpDateParam($expiryUnx);
        
        if ( $expirRes["days"]<0 ) {
            $QC_text .= "<img src=\"images/i40.warnTime.gif\" hspace=2> substance expired.\n";
        } else {
            $days = $expirRes["days"];
            $tmpOutTxt = '<b>'.$days.'</b> days';
            if ($days<1.0) {
                $hour = floor($days*24.0);
                $tmpOutTxt = '<b>'.$hour.'</b> hours';
            } else $tmpOutTxt = '<b>'.round($days).'</b> days';
            
            $QC_text .= "<img src=\"images/i40.calendar.gif\" hspace=5> Expiry Date: still ".$tmpOutTxt." left\n";
        }
        
        if ($QC_text!="")  {
            echo $greenball."<font color=gray><B>QC:</B></font> ".$QC_text;
            // $helpLib = new fHelpC();
            $hopt = array ( "object"=>"icon");
            $helpText = fHelpC::link_show("o.CONCRETE_SUBST.qccheck.html", "QC-help", $hopt);
            echo " &nbsp;&nbsp;".$helpText."<br>\n";
        }
    }
    
    function show_jour_entry($sqlo) {
        
        require_once 'o.JOUR_ENTRY.subs2.inc';
        require_once 'gui/o.JOUR_ENTRY.gui2.inc';
        
        $jou_obj_arr = oJOUR_ENTRY_subs::get_JOU_links($sqlo, 'CONCRETE_SUBST', $this->id );
        if (empty($this->id )) {
            return;
        }
        $this->sub_title('Related '.tablename_nice2('JOUR_ENTRY').'s');
        echo '<ul>'."\n";
        oJOUR_ENTRY_gui2::show_list1($sqlo, $jou_obj_arr);
        echo "</ul>\n";
    }
    
}
 
class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        
        $argu= &$this->argu;
        $o_rights = $this->o_rights;
        $viewmode = $this->viewmode;

        
        $greenball = "<img src=\"images/greenball.gif\" hspace=5>";
        
        //$a_proto_id = 0;
        $a_subst_id = 0;
        
        $expiry_date= $argu["EXPIRY_DATE"];
        $a_subst_id = $argu["ABSTRACT_SUBST_ID"];

        
        $helpLib = new oCONCRETE_SUBST_xHelp($id, $a_subst_id, $this->argu);
        $helpLib->sh_deviation($sqlo);
        $helpLib->showThumbnail($sqlo);
        
        $lab_special_file = '../'.$_SESSION['globals']['lab_path'].'/obj.concrete_subst.xmode.features.inc';
        if (file_exists($lab_special_file)) {
          require_once $lab_special_file;
          if (class_exists('oCONCRETE_SUBST_rwl')) {
              $rwl_lib = new oCONCRETE_SUBST_rwl($id, $argu, $this->class_argu, $o_rights, $viewmode);
              $rwl_lib->start($sqlo, $sqlo2);
          }
        }

        // OLD:
        //if ( $a_qcflag>0 ) {
        //	$QC_text .= "<img src=\"images/i40.cert.gif\" title=\"under QC-control\">"; 
        //}
        if ($expiry_date!="") {
            $helpLib->expiry_date($sqlo, $expiry_date);
        }

        if ($helpLib->user_prefs['JOUR_ENTRY.sh']) {
            $helpLib->show_jour_entry($sqlo);
        }

        
        $proto_out_lib = new oCONCRETE_SUBST_protos_show($sqlo, $id);
        if ( $proto_out_lib->has_protos( ) ) {
            echo $greenball."<font color=gray><B>Preparation protocol:</B></font>&nbsp;&nbsp;";
            
            $proto_out_lib->writeJavascript();
            $proto_out_lib->show($sqlo, $sqlo2);
        }

        return;	
    }

}


