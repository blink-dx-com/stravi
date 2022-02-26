<?php
/**
 * manage preferences for ProtoQuant
 * - manage main structure
 * - manage Sub-Structure
 * @package obj.protocol.quant.php
 * @swreq UREQ:0001578: o.PROTO > ProtoQuant : Analyse protocol steps 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $go: 
  			0
			1
			2
  		   $apid	: the selected abstract_protocol
  		   $so_id  - index of OBJ_STRUCT
  		   
		   $backurl (encoded)
		   $action = 
		       ["home"] : main header, no need of docid, 
		          if docid : switch to "overview"
		          ["overview"] : overview
		      "steps", define CONCRETE_PROTO steps
		         need: $parx['aprotoid']
              "crea_config"
              "sel_config" : select existing DOCID (configuration)
                     set $_SESSION['s_formState']['o.'.$q_table.'.pq']
              "autoselect" : switch to autoselect
              "conf_filt"   : configure the table condition, INPUT var:  $adv OLD:  tab_cond
                  
              "sel_apid"   : select protocol
                 $parx["aprotoid"] 
              "sub_struct"  : define SUB structure for following PROTO:STEPNR: 
                   $x['cols'] colname = 0,1
              "sub_struct_new" : create new, need  
                  parx: 
                    mo_id - mother id
                    t     - destination table
                    fkt   - foreign table
                    ty    - type
                    col   - column        
              "sub_struct_del" : 
                    delete sub structs ident by: $SX[key]     
              'sub_struct_cm' : change the abstract_subst of a sub_struct (need parx[mo_apid]=0&parx[stepnr]=0 )   
		   $parx   (for "xCols" or  "crea_config")
		      'mo_apid'
		      'stepnr'
		   $adv array for $action='conf_filt'   
		   $x[stepnr] protocol steps
		      array( "q"=>, "s"=> "n"=>, "d"=> "D"=>, "S"=>)
		   $docid : if given: save settings to docid
		   $q_table: destination table for protquant; e.g. CONCRETE_SUBST or EXP
 * GLOBAL:  
 *  $_SESSION['s_formState']['f.protoquant'] -- current CONFIG, e.g. 'current' == docid
    $_SESSION['s_formState']['o.'.$q_table.'.pq'] -- e.g. 'o.CONCRETE_SUBST.pq'
 *  DEPRECATED: 2021-05-04: $_SESSION['userGlob']["o.proto.Quant"] : see definition in o.PROTO.quant.inc
 *  DEPRECATED: 2021-05-04: $_SESSION['userGlob']["o.proto.Quant_sel"][TABLE] : ID of selected CONFIG-doc
 */


// extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("visufuncs.inc");
require_once ("f.objview.inc");
require_once ("func_form.inc");
require_once 'o.proj.profile.inc';
require_once 'o.PROJ.subs.inc';
require_once 'glob.obj.conabs.inc';
// require_once 'gui/glob.objtab.searchform.inc';
require_once 'gui/glob.objtab.filter_GUI.inc';

require_once ('impexp/protoquant/o.PROTO.quant.inc');
require_once ('impexp/protoquant/obj.concrete_subst.quant.inc');
// require_once ('impexp/protoquant/o.PROTO.quant_gui.inc');
require_once ('impexp/protoquant/o.PROTO.conf_gui2.inc');
require_once ('impexp/protoquant/o.PROTO.def.inc');
require_once ('impexp/protoquant/o.PROTO.quant_sub_struct.inc');

class main_gui {
    
    const VERSION='2021-05-04';
    
    function __construct($q_table, $backurl) {
        $this->backurl = $backurl;
        $this->q_table=$q_table;
        
    }
    
    function gui_head($sqlo,  $docid) {
        
        $q_table = $this->q_table;
        
        $conf_url = $_SERVER['PHP_SELF']."?q_table=".$q_table;
        
        echo "<table cellpadding=0 cellspacing=0 border=0><tr><td valign=top>";
        echo "<img src=\"images/f.protoquant.steine2.png\"></td><td>\n";
        echo "&nbsp;&nbsp;&nbsp;</td><td valign=top style=\"padding-top:6px;\">\n";
       

        $run_url=NULL;
        if ($q_table and $docid) {
            $pquant_obj = new oProtoQuantC();
            $pquant_obj->set_docid($sqlo, $docid);
            $run_url = $pquant_obj->get_run_url();  
        }
        
        if ($run_url) {
            echo '<a href="'.$run_url.'"><img src="res/img/play-circle.svg" title="RUN!"> Run</a> &nbsp;&nbsp;';
        } else {
            echo '<img src="res/img/play-circle.svg" title="RUN inactive" > Run &nbsp;&nbsp;';
        }
        echo '<a href="'.$conf_url.'"&action=home"><img src="res/img/settings.svg" title="config HOME"> Config HOME</a> &nbsp;&nbsp;';
        
        
        echo '<a href="'.$conf_url.'&action=sel_config"><img src="res/img/align-justify.svg" '.
          'title="Select a config"> Select configuration</a> &nbsp;&nbsp;';
        echo '<a href="'.$conf_url.'&action=crea_config"><img src="images/ic.plus.png" title="NEW config"> NEW configuration</a> &nbsp;&nbsp;';
        // echo "[<a href=\"".$conf_url."&action=overview\">Config Overview</a>] ";
        
        echo "<br>";
        echo '<span style="color:gray;"><b>Info:</b> This tool sets preferences for [ProtoQuant].'.
          ' Version: '.self::VERSION.'<br>'."\n";
        echo '<b>Config:</b> ';
        if ($docid) {
            $objLinkLib = new fObjViewC();
            $html_tmp = $objLinkLib->bo_display( $sqlo, 'LINK', $docid );
            echo $html_tmp;
        } else {
            echo 'None';
        }
        echo '</span>';
        echo "</td><tr></table>\n";
    }
    
    function show_next($url, $text) {
        echo "[<a href=\"".$url."\">".$text." &gt;&gt;</a>]<br>\n";
    }
    
    function ShowBackurl() {
        $backurl = $this->backurl;
        if ( $backurl!="" ) {
            $urlreal = urldecode($backurl);
            echo "[<a href=\"".$urlreal."\">back to ProtoQuant &gt;&gt;</a>]<br>\n";
        }
        
    }
    
    function go_home($extra_url_params=NULL) {
        $conf_url = $_SERVER['PHP_SELF']."?q_table=".$this->q_table . $extra_url_params;
        js__location_replace( $conf_url, 'back config home'); 
    }
    function go_sub_struct($so_id) {
        $conf_url = $_SERVER['PHP_SELF']."?q_table=".$this->q_table . '&action=sub_struct&so_id='.$so_id;
        js__location_replace( $conf_url, 'back config > sub structure');
    }
}



class fQuant_overview {
    
    function __construct($sqlo, $docid) {
        
        $this->docid   = $docid;
        $this->quantLib = new oProtoQuantC();
        $this->quantLib->set_docid($sqlo, $docid);
        
        $this->globset = $this->quantLib->get_ana_params($sqlo, $docid);
        $this->tablename = $this->globset['table'];
    }
    
    function show_STRUCT() {
        $this->quantLib->_show_STRUCT();
    }
    
    function _key_to_apid_step($key) {
        $key_arr=explode(":", $key);
        $apid  =$key_arr[0];
        $stepnr=$key_arr[1];
        return array($apid, $stepnr);
    }
    
    private function _one_row(&$sqlo, $so_id, $row) {
        
        
        $objLinkLib = new fObjViewC();
          
        
        // $extra='';
        if ($so_id==0) {

            $mo_apid=0;
            $mo_stepnr=0;
        } else {
            list($mo_apid, $mo_stepnr) = $this->_key_to_apid_step($key);  
        }
        
       
        
        $table = $row['table'];
        $table_nice = tablename_nice2($table);
        $table_icon = '<img src="'.$objLinkLib->_getIcon($table).'">';
        
        $mo_table_nice ='';
        $mo_id = $row['from'];
        if ($mo_id) {
            $mo_obj   = $this->globset[$mo_id];
            $mo_table = $mo_obj['table'];
            $mo_table_nice = tablename_nice2($mo_table);
        }
        
        $src_data = $row['src'];
        $src_nice = 'ty:'.$src_data['ty'];
        if ($src_data['ty']=='ass') {
            $fkt = $src_data['t'];
            $fkt_nice = tablename_nice2($fkt);
            $src_nice .= '; from: '.$fkt_nice;
            
            if ($src_data['step']) {
                $src_nice .= '; step: '.$src_data['step'];
            }
        }
        if ($src_data['abs_id']) {
            
            $html_tmp = fQuant_helper::get_abs_obj_html( $sqlo, $table,  $src_data['abs_id']);
            $src_nice .= '; obj: '.$html_tmp;
        }
        
        
        $names=array();
        foreach($row['cols'] as $t_row) {
            
            $col_loop = $t_row['name'];
            $tmp_col_feats = colFeaturesGet2( $table, $col_loop );
            $col_nice = $tmp_col_feats['NICE_NAME'];
            
            $names[] = $col_nice;
        }
        $cols_show = implode(', ',$names);
        
        if ( !empty($row['steps']) ) {
            $cols_show .= ', Steps:'.sizeof($row['steps']);
        }
        
        
       
        /*
         *  $objLinkLib = new fObjViewC();
        $html_sua='';
        if ($row['table']=='ABSTRACT_SUBST') {
            if ($row['obj_id'])    $html_sua = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST',  $row['obj_id'] );
        }
        */
       
        $delbox = '<input type=checkbox name="SX['.$so_id.']" value=1>';
        
        $actions = '[<a href="'.$_SERVER['PHP_SELF'].
        '?action=sub_struct&so_id='.$so_id.'">Config</a>]';
        
        //$actions .= ' [<a href="'.$_SERVER['PHP_SELF'].
        //    '?action=sub_struct&parx[mo_apid]='.$mo_apid.'&parx[stepnr]='.$mo_stepnr . $extra.'">Set Steps</a>]';
        
        
        $outarr = array( $delbox, $so_id, $actions, $table_icon.' '.$table_nice, $mo_id, $src_nice, $cols_show);
        
        /*
        $tmp_ch_maa = '';
        if ($row['table']=='ABSTRACT_SUBST') {
            $tmp_ch_maa =' [<a href="'.$_SERVER['PHP_SELF'].
                '?action=sub_struct_cm&parx[mo_apid]='.$mo_apid.'&parx[stepnr]='.$mo_stepnr.'">Other MAA</a>]';
        }
        $outarr[] = $tmp_ch_maa;
        */
        
      
        
        return $outarr;
    }
    
    function show($sqlo) {
        
        $baseurl = $_SERVER['PHP_SELF'];
        
        $select = $this->globset['select'];
        if (!empty($select)) {
            $sea_help = new g_objtab_filter_GUI($this->tablename, $select, '');
            $cond_nice = $sea_help->get_cond_nice($sqlo);
        } else {
            $cond_nice = 'No filter active.';
        }
        echo '<a href="'.$baseurl.'?action=conf_filt"><img src="res/img/filter.svg" title="Filter settings"> '.
           ' [Configure list filter]</a> ';
        echo ' &nbsp; Condition: '.$cond_nice;
        echo '<br>'."\n";
        
        
        
        
        echo '<form style="display:inline;" method="post" '.
            ' name="editform"  action="'.$_SERVER['PHP_SELF'].'" >'."\n";
        echo '<input type=hidden name="action" value="sub_struct_del">'."\n";
        echo '<input type=hidden name="go" value="1">'."\n";

        
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Protocol overview");
        $headx  = array ("#", "Index", "Action", "Object", "MO-ID", "Data source", "Show data");
        $tabobj->table_head($headx,   $headOpt);

        
        $objects = $this->globset['objects'];
        $found_root=0;
        if (!empty($objects)) {
            foreach($objects as $so_id => $row) {
                if ($so_id==0) {
                    $found_root=1;
                }
                $dataArr = $this->_one_row($sqlo, $so_id, $row);   
                $tabobj->table_row ($dataArr);
            }
        }
        $tabobj->table_close();
        
        if ($this->quantLib->has_write_access($sqlo)) {
            echo '<input type=submit value="Delete rows">'."\n"; 
        }
        echo '</form>'."<br>\n";
        
        $show_raw=0;
        if ( $show_raw or $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
            echo "<br><br>";
            $this->show_STRUCT();
        }
    }
}

class fQuant_tabCond {
    
    function __construct($sqlo, $docid, $baseurl) {
        
        $this->do_reload_page=0;
        
        $this->baseurl = $baseurl; //TBD:
        $this->docid   = $docid;
        $this->quantLib = new oProtoQuantC();
        $this->quantLib->set_docid($sqlo, $docid);
        $this->globset = $this->quantLib->get_ana_params($sqlo, $docid);
        
        $tablename = $this->globset['table'];
        $json_arr = $this->globset['select'];
        if (!is_array($json_arr)) $json_arr=array();
        
        // $this->sea_help = new gobjtab_searchform($sqlo, $tablename);
        $this->filt_gui_lib = new g_objtab_filter_GUI($tablename, $json_arr, $baseurl);
    }
    
    private function check_and_save($sqlo, $json_arr) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $filter_arr = $this->filt_gui_lib->get_filter_arr();
        
        if (!empty($filter_arr)) {
            $this->filt_gui_lib->create_SQL($sqlo);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 20, 'Error on SQL building.' );
                return;
            }
    
            $sqlAfter = $this->filt_gui_lib->get_sqlAfterSort();
            $sqlsel = '1 from '.$sqlAfter;
            $sqlo->Quesel($sqlsel);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 20, 'Error on filter SQL execution.' );
                return;
            }
        }
        $main_features = array('select'=>$json_arr);
        $this->quantLib->set_globset_main($main_features);
        $this->quantLib->save_globset($sqlo);
    }
    
    function show_form($sqlo, $go) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $parx2      = $_REQUEST['parx2'];
        $filt_gui_lib = &$this->filt_gui_lib;
        $filter_arr = $filt_gui_lib->get_filter_arr();
        

        if (empty($filter_arr)) $filter_arr=array();
        if (empty($filter_arr) and $parx2['action']==NULL and !$go) {
            $parx2['action']='add_col';
        }
        
        if (is_array($parx2)) {
            
            switch ($parx2['action']) {
                case 'add_col':
                    if (!$go) {
                        $new_i = sizeof($filter_arr);
                        $filt_gui_lib->form2_1_column($sqlo, $new_i);
                        return;
                    } else {
                        $go=0;
                        $filt_gui_lib->add_col($parx2['col']);
                        // continuem but do not SAVE !
                    }
                    break;
                    
                case 'del_col':
                    $filt_gui_lib->del_col($parx2['col_i']);
                    $json_arr = $filt_gui_lib->get_config_full();
                    $this->check_and_save($sqlo, $json_arr);
                    $this->do_reload_page=1;
                    break;
            }
        }
        
        if (!$go) {
            
            $main_options=array('with_order_form'=>1);
            $filt_gui_lib->form_main($sqlo, $_REQUEST['parx'], $main_options);
            
        } else {
            
            debugOut('(423) SAVE:start ', $FUNCNAME, 1);
            
            $filt_gui_lib->save_filter($sqlo, $_REQUEST['parx']);
            
            if ($parx2['sort']['col']==NULL) {
                $error->set( $FUNCNAME, 10, 'Sort column missing.' );
                return;
            }
            
            $new_sort_arr=array( array('col'=>$parx2['sort']['col'], 'dir'=>$parx2['sort']['dir']) );
            $filt_gui_lib->save_sort($sqlo, $new_sort_arr);
            $json_arr = $filt_gui_lib->get_config_full();

            $this->check_and_save($sqlo, $json_arr);
            $this->do_reload_page=1;
        }
        
    }
    
    function reload_page_flag() {
        return $this->do_reload_page;
    }
    
   
}

// ------------------------------------------------------------------------------------------------------



// --------------------------------------------------- 
global $error;
$FUNCNAME='MAIN';

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sql2  = logon2( );
if ($error->printLast()) htmlFoot();

$docid  = $_REQUEST['docid'];
$q_table= $_REQUEST['q_table'];
$parx   = $_REQUEST['parx'];
$apid   = $_REQUEST['apid'];
$action   = $_REQUEST['action'];
$backurl = $_REQUEST['backurl'];
$go     = $_REQUEST['go'];
$so_id  = $_REQUEST['so_id'];

if ($q_table) {
    $user_globals_qu = $_SESSION['s_formState']['o.'.$q_table.'.pq'];
}
//$user_globals_qu = unserialize($_SESSION['userGlob']["o.proto.Quant_sel"]); // see o_proto_Quant_sel_STRUCT
debugOut('(824) user_globals_qu:'.print_r($user_globals_qu,1), $FUNCNAME, 1);

if (!$docid) {
    if ($q_table) {
        if (!empty($user_globals_qu))  {
            $docid = $user_globals_qu['docid'];
        }
    } 
    if (!$docid) {
        if( !empty($_SESSION['s_formState']['f.protoquant']) ) {
            $docid = $_SESSION['s_formState']['f.protoquant']['current'];
        }
    }
}
if ($docid) {
    if (!gObject_exists($sql, 'LINK', $docid)) {
        $docid=0;
    }
}


// if ($parx['aprotoid']) {
// 	$apid   = $parx['aprotoid'];
// }


if ($docid and ($action!='crea_config' and $action!='sel_config' and $action!='autoselect') ) {
   $quantLib = new oProtoQuantC();
   $quantLib->set_docid($sql, $docid);
   $tmp_struct = $quantLib->get_globset();
   $q_table  = $tmp_struct['table'];  
   debugOut('got TABLE from DOC:'.$docid, $FUNCNAME, 1);
}

if ($q_table==NULL) {
    $error->set($FUNCNAME, 1, 'Input: table missing');
}
    


$tablename	= $q_table;


$gui_lib2 = new main_gui($q_table, $backurl);

$title       		 = "Preferences for [ProtoQuant]";
$infoarr			 = NULL;

$infoarr["title"]    = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["design"] = 'slim';


$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );
?>
<script language="JavaScript">
  	<!--
	function checkall( butt_id, num ) {
		/* butt_id: 0,1,2 
		   num: number of fields */
		i=0;
		var butt_name = "checker_" + butt_id;
		if (document.editform.elements[butt_name].value == 'all') {
		  document.editform.elements[butt_name].value = 'none';
		  val = true;
		} else {
		  document.editform.elements[butt_name].value = 'all';
		  val = false;
		}
		
		var tickPerline = 6;
		for( i=0; i<num; i++ )
		{
			document.editform.elements[i*tickPerline+butt_id].checked = val; 
		}
	}  	
	//-->
  	</script>
<?

$pagelib->_startBody($sql, $infoarr);



if ($error->Got(READONLY))  {
    $pagelib->chkErrStop();
    return;
}

if ( $action == "" ) $action = "home";

$goArray=array( "0"=>"Select parameters");
$extra_extratext='';

if ( $action == "home" ) {
    $goArray   = array( "0"=>"Home" );
} 
if ( $action == "overview" ) {
    $goArray   = array( "0"=>"Overview" );
}
if ( $action == "conf_filt" ) {
    $goArray   = array( "0"=>"Configure list filter" );
}
if ( $action == "sub_struct" ) {
    $goArray   = array( "0"=>"Set Output for one Object type", 1=>"..." );
    $extra_extratext=' (Sub structure)';
} 
if ( $action == "sel_config" ) {
    $goArray   = array( "0"=>"Select Configuration", 1=>"Save Configuration" );
} 
if ( $action == "sub_struct_del" ) {
    $goArray   = array( "0"=>"Select rows", 1=>"Delete rows" );
} 
if ( $action == "sub_struct_cm" ) {
    $goArray   = array( "0"=>"Change the template", 1=>"Change template now" );
} 

if ( $action == "crea_config" ) {
    $goArray   = array( "0"=>"Select parameters", 1=>"Save parameters" );
} 
if ( $action == "sel_apid" ) {
    $goArray   = array( "0"=>"Select parameters  (APID)", 1=>"Save parameters" );
    
} 
if ( $action == "steps" ) {
	$goArray   = array( "0"=>"Select a protocol parameters", 1=>"Select step details", 2=>"Save details" );
} 

$gui_lib2->gui_head($sql, $docid);


echo "<ul>";



$extratext = $extra_extratext." [<a href=\"".$_SERVER['PHP_SELF']."\">Start again</a>]";

$formPageLib = new FormPageC();
$formPageLib->init( $goArray, $extratext );
$formPageLib->goInfo( $go ); 
echo "<br>\n";

$no_need_docid = array('crea_config', 'sel_config', 'home');

if ( !in_array($action, $no_need_docid) ) {
    if (!$docid) {
        htmlFoot('USERERROR', 'Config-file missing.');
    }
}

if ($action == "home" ) {
    if (!$docid) {
        $action = "sel_config"; // CHANGE action ...
    } else {
        $action = "overview";  // CHANGE action ...
    }
}


if ($action == "overview" ) {
    
    $help_lib = new fQuant_overview($sql, $docid);
    $help_lib->show($sql);
    
    // $help_lib->show_STRUCT();
}
if ($action == "conf_filt" ) {

    $cond_help_lib = new fQuant_tabCond($sql, $docid, $_SERVER['PHP_SELF'].'?q_table='.$q_table );
    
    $cond_help_lib->show_form($sql, $go);
    $pagelib->chkErrStop();
    
    if ( $cond_help_lib->reload_page_flag() ) {
        $gui_lib2->go_home('&action=conf_filt');
    }
}

if ( $action == "crea_config" ) {
    
    if ($q_table=='') {
        $pagelib->htmlFoot("ERROR", 'No destination table given.');
    }
    
    $conf_crea_lib = new fQuantConfNew ( $sql, $parx, $q_table );
    
    if ( !$go ) {
        $conf_crea_lib->form1( $sql );
        htmlFoot("<hr>");
    }
    
    if ( $go == 1 ) {
        echo "... save config<br>\n";
        
        $conf_crea_lib->analyse($sql, $parx);
        if ($error->Got(READONLY))  {
            $pagelib->chkErrStop();
            return;
        }
        
        $docid = $conf_crea_lib->create_doc($sql);
        if ($error->Got(READONLY))  {
            $pagelib->chkErrStop();
            return;
        }
        echo "<br>\n";
        
        fQuant_helper::set_session_doc($docid, $q_table);

        $gui_lib2->go_home();
        htmlFoot("<br><hr>");
    }
    
}

if ( $action == "sel_config" ) {
    
    
    $guilib  = new oProtoQuant_ConfGuiC($sql, $q_table);
    if ($error->Got(READONLY))  {
        $pagelib->chkErrStop();
        return;
    }
    
    if ( !$go ) {
        $guilib->form_seldoc($sql, $parx);
        $pagelib->htmlFoot();
    }
    
    if ($parx['docid']) {  
        
        fQuant_helper::set_session_doc($parx['docid'], $q_table);
        echo "Config ID:".$parx['docid']." activated.<br>";
        
        //$gui_lib2->ShowBackurl();
        $gui_lib2->go_home();
    } else {
        echo "No config selected.<br>";
    }
    
}

// DEPRECATED
// if ($action =="autoselect") {
//     $user_globals_qu[$q_table]['doc.sel']='AUTO';
//     if ($q_table=='CONCRETE_SUBST') {
//         $docid = oProtoQuantGuiC::get_doc_id_by_MAA($sql);
//         $user_globals_qu[$q_table]['docid']  = $docid;
//         $user_globals_qu['current'] = $docid;
//         echo "... set DOC-ID: ".$docid."<br>";
//     }
//     $_SESSION['userGlob']["o.proto.Quant_sel"] = serialize($user_globals_qu);
//     echo 'ok<br>';
// }

if ($action == "sub_struct_new") {
    
    echo "Create new sub structure ...<br>";
    
    if (!$parx['mo_id'] or !$parx['t'] or !$parx['ty']) {
        $pagelib->htmlFoot("ERROR", 'Input missing.');
    }
    
   
    $q_table='';
    $conf_crea_lib = new fQuantConfNew ( $sql, $parx, $q_table );
    $conf_crea_lib->create_sub_struct($sql, $docid, $parx);
    
    echo 'new sub structure created ...<br>';
    $gui_lib2->go_home();
}

if ($action == "sub_struct") {
    
    // echo "Define output of sub structure ...<br>";
    
    if (!$so_id) {
        $pagelib->htmlFoot("ERROR", 'No ID for sub-structure given.');
    }
    
    $sub_lib = new fQuant_sub_struct($sql, $so_id, $docid);
    if (!$go) {
        $sub_lib->show_struct($sql, $sql2);
        return;
    }
    
    if ($go) {
        $sub_lib->save_output1($sql, $_REQUEST['x'] );
        $gui_lib2->go_sub_struct($so_id); 
    }
}

if ( $action=='sel_apid' ) {
    
    if (!$so_id) {
        $pagelib->htmlFoot("ERROR", 'No ID for sub-structure given.');
    }

   
    
    if ($go and  $parx["aprotoid"] ) {
        echo "Save Protocol setting.<br>";
        
        $q_table='';
        $conf_crea_lib = new fQuantConfNew ( $sql, $parx, $q_table );
        $sub_feats = array( 
           't'=>'CONCRETE_PROTO',
           'mo_id' => $so_id,
           'col'=>'CONCRETE_PROTO_ID', 
           'ty'=>"ass", 
           'fkt'   =>$parx["fkt"] , // foreign table
           'abs_id'=>$parx["aprotoid"]  
        );
        $conf_crea_lib->create_sub_struct($sql, $docid, $sub_feats);
        
        $pagelib->chkErrStop();
        $gui_lib2->go_home();
    }
}



if ( $action == "steps" ) {

    $x = $_REQUEST['x'];
    
    $quantLib = new oProtoQuantC();
    $quantLib->set_docid($sql, $docid);
    $OBJ_STRUCT = $quantLib->get_sub_struct($so_id);
    
    $step_lib = new oAbsProtoStepDet($sql, $docid, $x, 'sub_struct');
    $step_lib->set_sub_struct($so_id, $OBJ_STRUCT);

	if ( $go > 0) {
		$step_lib->saveVars($sql);
		$gui_lib2->go_home();
	}
}


if ($action == "sub_struct_del") {
    
    echo "Delete Sub-structure ...<br>";
    
    if ($go) {
        
        $SX = $_REQUEST['SX'];
        $quantLib = new oProtoQuantC();
        $quantLib->set_docid($sql, $docid);
        
        foreach($SX as $key => $val) {
            $quantLib->sub_struct_unset($key);
        }
        $quantLib->save_globset($sql);
        $pagelib->chkErrStop();
        $gui_lib2->go_home();
    }
}

if ($action == "sub_struct_cm") {
    // TBD: recode ...
    if ( !$so_id) {
        $pagelib->htmlFoot('ERROR', 'SO-ID missing.');
    }

   
    $sub_struct_lib = new fQuant_sub_struct($sql, $so_id, $docid);
    
    if (!$go)  {
        $sub_struct_lib->change_maa_form($sql);
        htmlFoot("<br><hr>");
    }
    
    $sub_struct_lib->change_maa_check($sql, $parx);
    $pagelib->chkErrStop();
   
        
    $sub_struct_lib->change_maa($sql, $parx['abs_id']);
    $pagelib->chkErrStop();
    
    echo "saved.<br>";
    $sub_struct_lib->save_globset($sql);
    

    $gui_lib2->go_home();
    
}


$pagelib->chkErrStop();
htmlFoot("<hr>");
