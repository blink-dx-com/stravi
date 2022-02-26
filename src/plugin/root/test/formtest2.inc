<?php
require_once ('func_form.inc');
require_once ('visufuncs.inc');
require_once ('gui/f.box_modal.inc');
/**
 * test a from
   - saves the variables also in $_SESSION['userGlob'] 
 * @package formtest2.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   int $go
        $parx[logfile] 
        $parx[action] => "analyze" | "actlink" | "llist" 
        $parx[hostip] 
        $parx["extall"]  => [0] | 1 all file extensions?
                            0: only php
                            1: all (also html,...)
        $parx["searchtext"] => search text
        $parx["txtrefneg"]  => TEXT (ignore text from referer)
        $parx["txtref"]     => TEXT ( search in referer )
 * @param string $action
 */


class _testForm{
    
    function __construct($_mod) {
        $this->_mod = $_mod;
    }
    
    function form1(&$parx) {
        
        ?>
	<script type="text/JavaScript">  
      
    <!--  
    function showField(id)  
    {  
         if (document.getElementById(id).style.display == 'none')  
         {  
              document.getElementById(id).style.display = '';  
         }  
    }  
    //-->  
        
    </script>  
    <?php
		
		$initarr   = NULL;
	    $initarr["action"] = $_SERVER['PHP_SELF'];
	    $initarr["title"]  = "Give import information";        
	    $initarr["ENCTYPE"]= "multipart/form-data";		// for uploading files
		$initarr["submittitle"] = "Analyze now!"; 
		$initarr["dblink"] = 1;
	    
		$hiddenarr=array();
		$hiddenarr['mod'] = $this->_mod;
		$hiddenarr['action'] = 'form1';
		
		$go=0;
	         
	    $formobj = new formc( $initarr, $hiddenarr, $go );
	     
	    $fieldx = array ("title" => "Log_file", "name"  => "logfile", "fsize" => 40,
	                     "colspan"=> 2, "val"   => $parx["logfile"], "notes" => "example: field-size:40", "object" => "text" ); 
	    $formobj->fieldOut( $fieldx );  
	    
	     
	    $fieldx = array ("title" => "Lines_analyze", "name"  => "linesAnal", 
	                     "val"   => $parx["linesAnal"], "notes" => "required field", "object" => "text", "req"=>1 );        
	    $formobj->fieldOut( $fieldx );
	    
	    $fieldx = array (
	    	"title" => "abstract substance", "name"  => "objid", "object" => "dblink" ,
	        "val"   => $parx["objid"], "notes" => "type: dblink", 
	    	"inits" => array( "table"=>"ABSTRACT_SUBST", "objname"=>' -------- ', "pos" =>"0", "projlink"=> 1, "urlfull"=>1),	
	    	);        
	    $formobj->fieldOut( $fieldx );     
	         
	    $fieldx = array ("title" => "Action", "name"  => "action",
	                     "inits" => array("analyze" => "Overview", "actlink" => "Activate Log", "llist" => "Long list"), 
	                     "val"   => $parx["action"], "notes" => "overview, activate, long list", "object" => "select" );        
	    $formobj->fieldOut( $fieldx );
	    
	    $fieldx = array ("title" => "Optional parameters", "object" => "info") ;        
	    $formobj->fieldOut( $fieldx ); 
	    
	    $fieldx = array ("title" => "Optional field", "name"  => "searchtext", "fsize" => 30,  'optional'=>1 ,
	                     "val"   => $parx["searchtext"], "notes" => "search for text", "object" => "text" ); 
	    $formobj->fieldOut( $fieldx );  
	    
	    // END: optional
	    
	    // recommended to use name "userfile"
	    $fieldx = array ("title" => "Import file", "name"  => "userfile", "namex" => TRUE,
	                     "notes" => "file to import", "object" => "file", "req"=>1 ); 
	    $formobj->fieldOut( $fieldx ); 
	    
	    
	    $fieldx = array ("title" => "Checkbox", "name"  => "extall", 
	                     "val"   => $parx["extall"], "notes" => "Show all file types? (not checked: only php-files)", 
	                     "inits" => 1, "object" => "checkbox" );        
	    $formobj->fieldOut( $fieldx ); 
	
	    $fieldx = array ("title" => "Radio1", "name"  => "extall", 
	                     "val"   => $parx["extall"], "optx"=>array("rowbr"=>0),
	                     "notes" => "Show all file types?", 
	                     "inits" => array("analyze" => "Overview", "actlink" => "Activate", "llist" => "Long list"), 
	                     "object" => "radio" );        
	    $formobj->fieldOut( $fieldx ); 
	     $fieldx = array ("title" => "Radio2", "name"  => "extall", 
	                     "val"   => $parx["extall"], "optx"=>array("rowbr"=>1),
	                     "notes" => "Vertical", 
	                     "inits" => array("analyze" => "Overview", "actlink" => "Activate", "llist" => "Long list"), 
	                     "object" => "radio" );        
	    $formobj->fieldOut( $fieldx ); 
	    
	    $fieldx = array ("title" => "Field X", "name"  => "linesAnal2",  'errtxt'=>'input problem',
	                     "val"   => $parx["linesAnal"], "notes" => "juhu", "object" => "text" );        
	    $formobj->fieldOut( $fieldx );
	    
	    $fieldx = array ("title" => "Not editable", "name"  => "test3", 
	                     "val"   => $parx["test3"], "notes" => "You can not edit this value", 
	                     "object" => "info2" );        
	    $formobj->fieldOut( $fieldx );  
	    
	    $fieldx = array ("object" => "hr" );        
	    $formobj->fieldOut( $fieldx );
	    
	    $fieldx = array ("title" => "Test_field", "name"  => "area1", 
	                     "val"   => $parx["area1"], "notes" => "colspan=2",
	                     "colspan"=> 2, 
	                     "inits" => array("cols"=>60), "object" => "textarea" );        
	    $formobj->fieldOut( $fieldx );  
	                                        
	    $fieldx = array ("title" => "hidden row", "object" => "info") ;        
	    $formobj->fieldOut( $fieldx );   
	    
	    $fieldx = array (
	    	"title" => "", 
	 	    "val"   => "<a href=\"javascript:showField('idQval')\">show Q-Val</a>", 
	 	    "notes" => "activate a field",
	        "object" => "info2" ); 
	    $formobj->fieldOut( $fieldx ); 

	     $fieldx = array (
	    	"title"   => "the field", 
	     	"name"    =>"hiddenField",
	 	    "val"     => "21", 
	     	"rowid"   => "idQval",
	     	"rowStyle"=> "DISPLAY: none",
	 	    "notes"   => "",
	        "object" => "text" ); 
	    $formobj->fieldOut( $fieldx );
	    
	    $formobj->close( TRUE );   
	}
	
	function form3() {
		$initarr   = NULL;
	    $initarr["action"] = $_SERVER['PHP_SELF'];
	    $initarr["title"]  = "Give import information";        
		$initarr["submittitle"] = "Analyze now!"; 
		$initarr["tabwidth"] = "AUTO";
		
		
		$hiddenarr=array();
		$hiddenarr['mod'] = $this->_mod;
	    
	    $parx=NULL;
	         
	    $formobj = new formc( $initarr, $hiddenarr, $go );
	    
	    echo '<tr valign=top bgcolor=#FFFFFF>'; 

	    
	    $fieldx = array ("title" => "Log_file", "name"  => "logfile", "fsize" => 20,
	                     "colspan"=> 2, "val"   => $parx["logfile"], "object" => "text" ); 
	    $formobj->slimField($fieldx);  
	    $fieldx = array ("title" => "Log2_file", "name"  => "log2file", "fsize" => 20,
	                     "colspan"=> 2, "val"   => $parx["logfile"], "object" => "text" ); 
	    $formobj->slimField( $fieldx );  
	    echo '</tr>';
	    echo '<tr valign=top bgcolor=#FFFFFF>';
	    
	    $fieldx = array ("title" => "Action", "name"  => "action",
                     "inits" => array("analyze" => "Overview", "actlink" => "Activate Log", "llist" => "Long list"), 
                     "val"   => $parx["action"], "notes" => "overview, activate, long list", "object" => "select" );        
	    $formobj->slimField( $fieldx );
	    
	    
	    echo '</tr>';
	    
	    $formobj->close( TRUE );   
	    
	    echo "<br><br>";
	}
	
	function form2($parx) {
	    echo "<br><b>2. Form</b><br>";
	    
	    $go=0;
	    
	    $hiddenarr=array();
	    $hiddenarr['mod'] = $this->_mod;
	    
	    $initarr   = NULL;
	    $initarr["action"] = $_SERVER['PHP_SELF'];
	    $initarr["title"]  = "Second form";
	    $initarr["submittitle"] = "do now!";
	    $initarr["tabwidth"] = "AUTO";
	    $initarr["tabBgColor"] = "#FFE0E0";
	    // $initarr["tabBgHdImg"] = "../../images/ic.blume04.gif";
	    $formobj = new formc( $initarr, $hiddenarr, $go );
	    
	    $fieldx = array ("title" => "Test_field", "name"  => "area2",
	        "val"   => $parx["area2"], "notes" => "Text field",
	        "object" => "text" );
	    $formobj->fieldOut( $fieldx );
	    
	    $cl_opt=array("addObjects"=>' <input type=submit class="yButSmall" value="Small submit">');
	    $formobj->close( TRUE, $cl_opt );
	}
	
	function form4($parx) {
	    echo "<br><b>Hidden Modal Box</b><br>";
  
	    $title_base = 'Change Audit-Status of protocol to';
	    $box_id    ='xModalTest1';
	    $hbox_lib  = new F_box_modal($box_id);
	    $js_string = $hbox_lib->get_click_js();
	    
	    ?>
	    <script>
    	function xSubmit(prc_id, action) {
    		title_compo = document.getElementById('<?php echo $box_id;?>_title');
    		title_compo.innerHTML = "<?php echo $title_base?>" + " " +action;
			document.prc_audit.id.value=prc_id;
			document.prc_audit.elements['parx[statName]'].value=action;
			<?php echo $js_string;?>
    	}
		</script>
		<?php

	    echo '<a href="#" onClick="xSubmit(1234,\'tech_ready\')">OPEN Modal Form Box</a><br>';
	    

	    $hbox_lib->draw_open($title_base);
	    
	    $suc_id=1234;
	    //http://jenblipart04/pionir/glob.obj.acclock.php?t=CONCRETE_PROTO&id=6637&parx[mo.CERT_FLAG]=1
	    //&action=add&parx[action]=24&backurl=edit.tmpl.php%3Ft%3DCONCRETE_SUBST%26id%3D4252

	    $hiddenarr=array();
	    $hiddenarr['mod'] = $this->_mod;
	    $hiddenarr['t'] = 'CONCRETE_PROTO'; 
	    $hiddenarr['id']= NULL;	
	    $hiddenarr['parx[statName]']= NULL;	
	    $hiddenarr['action']= 'add';
	    $hiddenarr['backurl'] = urlencode('edit.tmpl.php?t=CONCRETE_SUBST&id='.$suc_id);
	    $hiddenarr["parx[mo.CERT_FLAG]"] = 1;
	    
	    $initarr   = NULL;
	    $initarr["action"] = 'p.php';
	    $initarr["title"]  = "Change Audit-Status of protocol";
	    $initarr["submittitle"] = "";
	    $initarr["no_html"]  = 1;
	    $initarr["formname"] = 'prc_audit';

	    
	    $formobj = new formc( $initarr, $hiddenarr, 0 );
	    $formobj->start_form();
	    
	    echo '<table>'."\n";
	    
	    $fieldx = array ("title" => "Notes", "name"  => "notes",
	        "val"    =>'',
	        "object" => "textarea" 
	    );
	    $formobj->fieldOut( $fieldx );

	    $fieldx = array ("title" => "Save", "object" => "submit" );
	    $formobj->fieldOut( $fieldx );
	    

	    echo '</table>'."\n";
	   
	    
	    $formobj->close_form();
	   
	    
	    $hbox_lib->draw_close();
	    
	    echo "<br><br>";
	}
}
 
function this_infox( $keyx, $valx ) {
    echo "<tr><td><font color=gray>$keyx:</font></td><td><B>$valx</B></td></tr>\n";
}  


class formtest2_XPL extends gPlugin {
	
	
	function register() {
		
		$title		= 'GUI tests';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/rootFuncs.php', 'home') );

	}

	
	function startMain() {
		
		
		
		$sqlo = &$this->_sqlo;
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$action = $_REQUEST['action'];
		
		echo "<ul>\n";
		
		if ($action=='form1') {
		
    		echo "<b>1. Form</b><br>";
    		$iniarr=array();
    		$iniarr["logfile"] = "/usr/local/apache/logs/access_log";
    		$iniarr["numShowFuncs"] = 30;
    		$iniarr["linesAnal"]    = NULL;
    		$iniarr["showRelevant"] = 1;
    		$iniarr["action"] = "analyze";
    		$iniarr["test3"]  = "fix value";
    
    		// set init values
    		if (sizeof($iniarr)) {
    		    foreach( $iniarr as $key=>$val) {
    		        if ($parx[$key]=="") $parx[$key] = $val;
    		    }
    		   
    		}
    		
    		if ( !$go AND !isset($parx["showRelevant"])  )  $parx["showRelevant"] = $iniarr["showRelevant"];
		}
		
		
		if ( !$_SESSION['s_suflag'] && ($_SESSION['sec']['appuser']!="root") ) {
		    echo "Sorry, you must be root or have su_flag.";
		    return 0;
		}

		if ( !$go ) {
		    $form_lib = new _testForm($this->_mod);
		    $form_lib->form1($parx);
		    $form_lib->form2($parx);
		    
		    echo "<br><b>3. Form</b> (free table cells)<br>";
		    $form_lib->form3();
		 
		    $form_lib->form4($parx);
		    
		    return;
		}
		
		
		//
		// GO = 1
		//
		echo "the Parameters:<pre>";
		print_r($_REQUEST);
		echo "</pre>";

		//$filename = $parx["logfile"];
		
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Refine or Reload";
		$initarr["submittitle"] = "Reload";
		$initarr["goNext"]      = 1;
		
// 		$taction  = $parx["action"];
// 		$listformat = "";
// 		if ($parx["action"]=="llist") {
// 		    $taction="actlink";
// 		    $listformat="long";
// 		}
		
		$hiddenarr = NULL;
		$hiddenarr['mod']=$this->_mod;
		if ( sizeof($parx) ) {
		    foreach( $parx as $idx=>$valx) {
		        $hiddenarr['parx['.$idx.']']    = $valx;
		    }
		   
		}
		$formobj = new formc( $initarr, $hiddenarr, $go );
		
		$formobj->close( TRUE );
		
		
		
		echo "<table bgcolor=#EFEFEF border=0>\n";
		
		$actionhtml = "Analyze log";
		if ( $parx["action"] == "actlink") $actionhtml = "Show detailed lines, activate them." ;
		if ( $parx["action"] == "llist") $actionhtml    = "Show detailed lines (with referer info)." ;
		this_infox( "Action", $actionhtml );
		
		this_infox( "File", "'".$parx["logfile"]."'");
		
		
		this_infox( "Lines to analyze", $parx["linesAnal"]. "  &nbsp;&nbsp;&nbsp;</B><font color=gray>Lines in file:</font> ");
		if ($parx["timestart"]!="")
		    this_infox( "Time range",$parx["timestart"]." =&gt; ".$parx["timeend"]);
		    
	    this_infox("From host only", $parx["hostip"] );
	    if ($parx["searchtext"] !="") this_infox("Search for text: ", $parx["searchtext"]);
	    
	    
	    
	    echo "</table>\n";
	}
}
