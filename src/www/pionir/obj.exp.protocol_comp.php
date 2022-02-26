<?php
/**
 * compare protocol logs - OVERVIEW
 *  selection of list elements
  $_SESSION['s_formState']["oPROTOcmp"] = array ("aprotoid"=> , "step_no"=> "proj_id"=>)
   SINCE 20020726
 * @package obj.exp.protocol_comp.php
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   
 *       $go
         $step_no
         $editmode
		 [$proj_id]
		 [$obj_ids]
		 $exp_ref_id : ID of reference experiment

 */

session_start(); 

require_once ('reqnormal.inc');
require_once ("sql_query_dyn.inc"); 
require_once 'o.EXP.proto_mod.inc';
require_once 'o.EXP.proto.inc';
require_once ('javascript.inc');
require_once ("subs/obj.concrete_proto.cmpgui.inc");


class oEXP_proto_comp_G {
    
    private $exp_proto_arr; // STEP_NO => APID
    
    function __construct($sql, $editmode, $exp_model, $step_no) {
        $this->editmode=$editmode;
        $this->exp_proto_arr=array();
        
        $this->exp_model = $exp_model;
        $this->step_no=$step_no;
        
        $this->exp_model_arr=array();
        $this->EXP_MODEL_CNAME='model_experiment';
        
        if ( $exp_model ) {
            $sqls= "select NAME, EXP_TMPL_ID from EXP where EXP_ID=". $exp_model;
            $sql->query("$sqls");
            if ( $sql->ReadRow() ) {
                $this->exp_model_arr["NAME"]         = $sql->RowData[0];
                $this->exp_model_arr["EXP_TMPL_ID"]  = $sql->RowData[1];
            }
        }
    }
    
    function formModelExp ($exp_model, $exp_model_name) {
        $EXP_MODEL_CNAME=$this->EXP_MODEL_CNAME; 
        
        echo "<form method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
        echo "<table cellpadding=5 cellspacing=5 border=0 bgcolor=#EFEFEF>\n";
        echo "<tr><td>\n";
        echo "<B>Select a $EXP_MODEL_CNAME:</B> \n"; 
        $exp_model_nameurl = $exp_model_name;
        if ($exp_model_nameurl=="") $exp_model_nameurl= " --- select ---" ; 
    	$jsFormLib = new gJS_edit();
    	//$fopt = array();
    	$answer = $jsFormLib->getAll("EXP", "exp_ref_id", $exp_model, $exp_model_nameurl, 0 );
    	echo $answer;
        echo "\n<input type=submit value=\"Take it!\">\n";
        echo "</td></tr></table>\n";
        echo "</form>\n\n"; 
    }     
    
    
    /**
     * 
     * @param object $sql
     * @param object $sql2
     * @param int $exp_id
     * @param int $max_no_theo
     * @param string $bgcolor
     * @param array $opt   
     *   "go" 
         "step_no" 
         "editmode"
         "c_model_id" -- concrete_proto_id 
         "a_model_id"
         "is_model"
                 
     * @return string[][]
     */
    private function expout( &$sql, &$sql2, $exp_id, string $bgcolor,  $opt=NULL    ) {  
    	global $error;
    	$FUNCNAME = "_expout";
    	
    	
    
    	$retarr=array();
        $access_info = "";
    
        $tmpl_name   = "&nbsp;";
        if ( $opt["is_model"] AND $opt["editmode"] == "edit" ) $tmpl_name   = "<font color=red>missing</font>";
    	
        $sqls= "select NAME, EXP_TMPL_ID from EXP where EXP_ID=". $exp_id;
     	$sql->query("$sqls");
     	$sql->ReadRow();
     	$name    = $sql->RowData[0];
        $tmpl_id = $sql->RowData[1];  
        
    	if (  $tmpl_id ) {
            $sqls= "select NAME from EXP_TMPL where EXP_TMPL_ID=".$tmpl_id;
            $sql->query("$sqls");
            $sql->ReadRow();
            $tmpl_name  = $sql->RowData[0];
        }           
        
        if ( $opt["editmode"] == "edit" AND !$opt["is_model"] ) {
             $o_rights = access_check( $sql, "EXP", $exp_id);
             $access_info = " <font color=green>access</font>"; 
             if ( !$o_rights["insert"] ) {
                $access_info = " <font color=red>modification denied</font>";
             }
        }
        
    	$sqls= "select max(STEP_NO), count(STEP_NO) from EXP_HAS_PROTO where EXP_ID=".$exp_id;
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	$max_no     =	$sql->RowData[0];
    	$num_step_no=	$sql->RowData[1];
        

    	echo "<tr bgcolor=".$bgcolor." valign=top><td  class=xt1>"; 
        if ($opt["extrakey"]!="") echo $opt["extrakey"];
        echo "<a href=\"edit.tmpl.php?tablename=EXP&id=".$exp_id."\">$name</a>".$access_info."</td>";
        
        echo "<td class=xtextm>".$tmpl_name;      
        echo "</td>";
    	
    	
        
    	$proto_lib = new oEXP_proto_mod();
        
    	foreach ( $this->exp_proto_arr as $step_no=>$apid ) {
     	    
     	    // $i
    	    $c_proto_tmp = "";
    		$abstract_proto_id = "";
            $tmpcreainfo = "";
            echo "<td  class=xt1>";
    		
    		$sqls= "select l.CONCRETE_PROTO_ID, l.STEP_NO, p.ABSTRACT_PROTO_ID from EXP_HAS_PROTO l, CONCRETE_PROTO p 
    			where l.EXP_ID=".$exp_id. " AND l.STEP_NO=".$step_no. " AND l.CONCRETE_PROTO_ID=p.CONCRETE_PROTO_ID";
    			
    		$sql->query("$sqls");
    		if ( $sql->ReadRow() ) {
    		
     			$c_proto_tmp	=$sql->RowData[0];
    			$step_no_tmp	=$sql->RowData[1];
    			$abstract_proto_id =$sql->RowData[2];
    			
                $retarr[$step_no_tmp] = array($c_proto_tmp, $abstract_proto_id);
                
    			$sql->query("select NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$abstract_proto_id);
    			if ( $sql->ReadRow() ) {
    				$abstract_proto_name = $sql->RowData[0];
    			}
    			
                //if ($opt["showbut"]) {  
                //     echo "<input type=button name='dummy' value='Create' onclick=\"thisCreate('".$step_no_tmp."');\"><br>\n";
                //}
    			echo $abstract_proto_name; // <font color=gray>$abstract_proto_id:</font>
    		} else echo "&nbsp;";  
            
    		if ( $opt["step_no"]==$step_no AND $opt["c_model_id"] AND $o_rights["insert"] ) {
                if ( !$c_proto_tmp ) {   // only, if no step exists 
                
                    // COPY steps from model NOW !!!
                    if ( $opt["go"]  ) {  
    
                        $proto_lib->set_exp($sql, $exp_id);
                        $proto_lib->create($sql, $sql2, $opt["a_model_id"],  $opt["step_no"], array(), $opt["c_model_id"]);
                        if ($error->Got(READONLY))  {
    						$tmpcreainfo = "<font color=red>Error during creation</font>";
    						$error->reset();
    					} else {
    						$tmpcreainfo = "<font color=green>Created</font>"; 
                        }
                        echo " ".$tmpcreainfo;                        
                    }
                }
            }
    		
    		echo "</td>";
    		
    	}
    
    	echo "</tr>\n";  
        return ($retarr);
    }
    
    private function ana_exp_tmpl($sql, $exp_id) {
        
        $sqls= "select NAME, EXP_TMPL_ID from EXP where EXP_ID=". $exp_id;
        $sql->query("$sqls");
        $sql->ReadRow();
        
        $exp_tmpl_id   = $sql->RowData[1];
        if (!$exp_tmpl_id) {
            htmlErrorBox("Warning", "First experiment has no experiment template!");
            
            echo "<br>";
        } else {
            $sqls= "select NAME from EXP_TMPL where EXP_TMPL_ID=". $exp_tmpl_id;
            $sql->query("$sqls");
            $sql->ReadRow();
            $this->exp_tmpl_name = $sql->RowData[0];
            
            // calc theoretical number of steps
            $sqls="select PROTO_ID, ABSTRACT_PROTO_ID, STEP_NO from EXP_TMPL_HAS_PROTO where EXP_TMPL_ID=".$exp_tmpl_id. " order by STEP_NO";
            $sql->query("$sqls");
            $tmpstep_no = 0;
            while ( $sql->ReadRow() ) {
                $tmpstep_no = $sql->RowData[2];
                $a_proto_id = $sql->RowData[1];
                $this->exp_proto_arr[$tmpstep_no] = $a_proto_id;
            }
            
        }  
        $this->exp_tmpl_id = $exp_tmpl_id;
    }
    
    function pre_analyse_one_exp($sqlo, &$exp_proto_lib, $exp_id) {
        
        $proto_log = $exp_proto_lib->getCProtos($sqlo, $exp_id);
        foreach($proto_log as $no=>$cpid) {
            if (!$this->exp_proto_arr[$no]) {
                $apid = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $cpid, 'ABSTRACT_PROTO_ID');
                $this->exp_proto_arr[$no] = $apid;
            }
        }
    }
    
    /**
     * output:
     * $this->exp_proto_arr
     * @param object $sql
     * @param int $first_exp
     */
    function pre_analyse($sqlo, &$sel) {
        
        $this->max_no_theo = 0;
        
        $first_exp = current($sel);
        $this->ana_exp_tmpl($sqlo, $first_exp);
        
        $exp_proto_lib = new oEXPprotoC();
        
        if ($this->exp_model)
            $this->pre_analyse_one_exp($sqlo, $exp_proto_lib, $this->exp_model);
        
        // analyse all experiments ...
        foreach($sel as $exp_id) {
            $this->pre_analyse_one_exp($sqlo, $exp_proto_lib, $exp_id);
        }
        
        // sort
        ksort($this->exp_proto_arr);
        
        $step_no_keys = array_keys($this->exp_proto_arr);
        $this->max_no_theo = max($step_no_keys);
        if (!$this->max_no_theo)  $this->max_no_theo = 5;
       
    }
    
    /**
     * input: 
     * $this->exp_proto_arr
     * @param object $sql
     * @param object $sql2
    
     */
    function form_main($sql, $sql2, &$sel, $go) {
        
        $step_no= $this->step_no;
        $max_no_theo = $this->max_no_theo;
        $editmode=$this->editmode;
        $exp_tmpl_id = $this->exp_tmpl_id;
        $exp_model = $this->exp_model;
        $EXP_MODEL_CNAME=$this->EXP_MODEL_CNAME;
        
        $cnt=0;
        if ($editmode=="edit") {
            echo "\n\n<form method=\"post\"  name=\"editform2\"  action=\"".$_SERVER['PHP_SELF']."?go=1\" >\n";
            echo "<input type=hidden name=step_no value=\"\">";
        }
        
        echo "<table border=0 cellpadding=1 cellspacing=1 bgcolor=#FFFFFF>\n";
        echo "<tr bgcolor=#336699>";
        echo "<th class=xt1>Experiment</td>";
        echo "<th class=xt1>Exp_template</td>";
        foreach ( $this->exp_proto_arr as $step_no=>$apid ) {
            echo "<th class=xprot>Proto ".($step_no)."</td>\n";
        }
        echo "</tr>\n";
        
        //
        // print exp template parameters
        //
        
        echo "<tr bgcolor=#6699FF>";
        echo "<td class=xt1 ><img src=\"images/icon.EXP_TMPL.gif\" border=0> <B>template from first exp</B></td>";
        echo "<td class=xtextm ><a href=\"edit.tmpl.php?t=EXP_TMPL&id=$exp_tmpl_id\">".$this->exp_tmpl_name."</a></td>";
        
        foreach ( $this->exp_proto_arr as $step_no=>$apid ) {
            
            $abstract_proto_name = "&nbsp;";

            $sql->query("select NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$apid);
            $sql->ReadRow();
            $abstract_proto_name = $sql->RowData[0];
            
            echo "<td class=xt1>".$abstract_proto_name."</td>\n";
            
        }
        echo "</tr>\n";
        
        $tab_colspan = $max_no_theo+2;
        
        // print exp model
        
        $tmpimg = "<img src=\"images/icon.EXP.gif\" border=0> <B>Model:</B> ";
        if ( !empty($this->exp_model_arr)) {
            
            $opt = NULL;
            $opt["extrakey"] = $tmpimg;
            $opt["is_model"] = 1;
            $opt["editmode"] = $editmode;
            $exp_model_arr2  = $this->expout( $sql, $sql2, $exp_model, "#99DDFF", $opt );
            
           
            
            if ($editmode=="edit") {
                //
                // print CREATE buttons
                //
                
                
                echo "<tr bgcolor=#99DDFF>";
                if ( sizeof($exp_model_arr2) ) {
                    
                    echo "<td  class=xt1><font color=gray>Create missing protocols:</font></td><td>&nbsp;</td>\n";
                    foreach ( $this->exp_proto_arr as $step_no=>$apid ) {
                       
                        $tmparr = $exp_model_arr2[$step_no];
                        echo "<td>";
                        if ( is_array($tmparr) ) {
                            echo "<input type=button name='dummy' value='Create from model' onclick=\"thisCreate('".$step_no."');\">";
                        } else echo "&nbsp;";
                        echo "</td>\n";
                    }
                } else {
                    echo "<td class=xt1>&nbsp;</td><td>&nbsp;</td>\n";
                    echo "<td class=xt1 bgcolor=#FFEFEF colspan=".($tab_colspan-2)."><font color=red>No protocols from ".$EXP_MODEL_CNAME.". Protocol creation not possible!</font></td>";
                }
                echo "</tr>\n\n";
            }
            
        } else {
            echo "<tr bgcolor=#99DDFF>";
            echo "<td  class=xt1 colspan=".$tab_colspan.">".$tmpimg." no ".$EXP_MODEL_CNAME;
            echo "</td>";
            echo "</tr>";
        }
        
        $color1 = "#EFEFEF";  // SUN violett
        $color2 = "#EFEFFF";
     
        
        $opt = NULL;
        $opt["editmode"]   = $editmode;
        
        if ( $go ) {
            $tmparr = $exp_model_arr2[$step_no];
            $opt["c_model_id"] = $tmparr[0];     // the model protocol
            if (!$opt["c_model_id"]) {
                
                $go = 0;   // fall back
                echo "<tr bgcolor=#FFD0D0>";
                echo "<td  class=xt1 colspan=".$tab_colspan.">";
                echo " <font color=red>Error:</font> concrete protocol from $EXP_MODEL_CNAME missing";
                echo "</td>";
                echo "</tr>";
            }
            $opt["a_model_id"] = $tmparr[1];     // the model abstract protocol
            $opt["go"]         = $go;
            $opt["step_no"]    = $step_no;
            
        }
        
        $color='';
        foreach($sel as $th1 ) {
            $exp_id = $th1;
            if ($color == $color1)  $color = $color2;
            else $color = $color1;
            
            $this->expout( $sql, $sql2, $exp_id, $color, $opt );
            
            $cnt++;
        }
        echo "</table>\n";
        
        if ($editmode=="edit") {
            echo "</form>\n\n";
        }  
    }
}

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );


$go       =$_REQUEST['go'];
$proj_id  =$_REQUEST['proj_id'];
$step_no  =$_REQUEST['step_no'];
$editmode=$_REQUEST['editmode'];
$exp_ref_id=$_REQUEST['exp_ref_id'];
$obj_ids =$_REQUEST['obj_ids'];

$title = "Compare protocol logs of experiments"; 


$formModelshown  = 0;
$tablename 	     = "EXP";
if ( isset($editmode) )   $_SESSION['s_sessVars']['o.EXP.editmode'] = $editmode; 
if ( isset($exp_ref_id) ) $_SESSION['s_sessVars']["o.EXP.blueprint"]= $exp_ref_id;

$exp_model = $_SESSION['s_sessVars']["o.EXP.blueprint"]; 
$editmode  = $_SESSION['s_sessVars']['o.EXP.editmode'];
if ( $editmode == "" )  $editmode = "view";

if ($proj_id) $_SESSION['s_formState']["oPROTOcmp"]["proj_id"] = $proj_id; // save as SAVED parameter
$proj_id = $_SESSION['s_formState']["oPROTOcmp"]["proj_id"];

$compGuiAll    = new oPROTO_cmpgui();
$compGuiAll->init($proj_id, $tablename);

$infoarr=array();
$compGuiAll->showHeader( $sql, $title, $obj_ids, $infoarr, "overview" );
$sqlAfter = $compGuiAll->sqlAfter;


//  td.xtextm for exp_template
?>
<style type="text/css">       
    th.xt1 { color:white; }
    th.xprot { color:white; background-color:#008000 }
    td.xt1 { font-size:0.8em; }
    td.xtextm { font-size:0.8em; color:#606060;}
    
</style>
 

<script language="JavaScript">
    function thisCreate( step_no ) {
        document.editform2.step_no.value = step_no;
        document.editform2.submit();
    }
</script>

<?  

js__openwin();
js__linkto();
js__inputRemote();
js__openproj();

$main_lib = new oEXP_proto_comp_G($sql, $editmode, $exp_model, $step_no);

$EXP_MODEL_CNAME = $main_lib->EXP_MODEL_CNAME;
// gHtmlMisc::func_hist( "obj.EXP.protocol_comp", $title_short,  $_SERVER['PHP_SELF'] ); 


  
echo " <font color=gray>Mode:</font> <B>$editmode</B> ";
if ($editmode=="view") echo " [<a href=\"".$_SERVER['PHP_SELF']."?editmode=edit\">Edit mode</a>]";  
else                   echo " [<a href=\"".$_SERVER['PHP_SELF']."?editmode=view\">View mode</a>]";
echo "<br>";   

if ($go) echo "Create protocol: No: $step_no from ".$EXP_MODEL_CNAME."<br>\n";  

// get selected objects
$sql->query("SELECT EXP_ID FROM $sqlAfter"); 
$sel  = array();
$num_elem =0;
while ( $sql->ReadRow() ) { 
    $sel[] = $sql->RowData[0];
    $num_elem++;
}   

$num_elem = sizeof($sel) ;
if ( !$num_elem ) {
	echo "<b>ERROR</B>: no experiments selected to compare.<br>";
	return 0;
}            


$main_lib->pre_analyse($sql, $sel);

if ( $editmode=="edit" AND !$exp_model ) {   
           
    echo "<br>\n";
    htmlErrorBox("Missing", "Need a ". $EXP_MODEL_CNAME ." for the protocol update! Please select one");
    
    $main_lib->formModelExp ($exp_model, $main_lib->exp_model_arr["NAME"]);
    $formModelshown=1;
}   

$main_lib->form_main($sql, $sql2, $sel, $go);
$error->printAll();

if ($editmode=="edit" AND !$formModelshown) {              
    echo "<br>\n";
    $main_lib->formModelExp ($exp_model, $main_lib->exp_model_arr["NAME"]);
} 
htmlFoot('<hr>');

