<?php
require_once("role.inc");

/**
 * Control center for PaXml import/export of a project
 * - user with $_SESSION['s_suflag']=1 has more options
 * @namespace core::gui
 * @package paxml_io.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param $id -- PROJ_ID
 */
class paxml_io_XPL extends gPlugin {

	function register() {

	    $title = "PartisanXML import/export";
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['obj_name'] = "PROJ";
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		$this->infoarr["help_url"] = "export_of_objects.html";

		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('read') )
			);
	}
	
	function tabrow($outarr) {
	    echo "<tr><td>".$outarr[0]."</td><td>".$outarr[1]."</td></tr>\n";
	}
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		// $sqlo2= &$this->_plug_regSql2();
		
		$proj_id = $_REQUEST['id'];
		
		$o_rights = access_check( $sqlo, "PROJ", $proj_id );
		$sqls     = "select name from proj where proj_id=" . $proj_id;
		$sqlo->query($sqls);
		if ($sqlo->ReadRow()) $proj_name = $sqlo->RowData[0];
		else  {
		   $this->htmlFoot("ERROR",  "Project not found.");
		}
		    
	    if (!$o_rights["read"])
	    {
	        $this->htmlFoot("ERROR", "no read permissions on project '$proj_name'!");
	    }
	    
	    $url_exp="impexp/partisanxml/export.select.php?cct_table=PROJ&cct_id=" .$proj_id;
	    $url_imp="impexp/partisanxml/import.select.php?projHidden=1&proj_id="  .$proj_id;
	    $url_val="impexp/partisanxml/validate.select.php?projHidden=1&proj_id=".$proj_id;
	    
	    $img_exp = "<img src=\"images/f.dbexport.png\" border=0>";
	    $img_imp = "<img src=\"images/f.dbimport.png\" border=0>";
	    
	    echo "<ul>";
	    
	    echo "<table cellpadding=5 cellspacing=4 border=0>";
	    
	    $role_right_name = "f.PaXml_export";
	    $role_right      = role_check($sqlo, $role_right_name);
	    if (!$role_right["execute"] &&  !glob_isAdmin()  ) {
	        $outarr= array( $img_exp,"Export objects from project. &nbsp; &nbsp; <font color=red>Not possible:</font> You must have role right '$role_right_name'.</font>");
	    } else {
	        $outarr= array( "<a href=\"" . $url_exp . "\">".$img_exp."</A>" ,
	            "<a href=\"" . $url_exp . "\"><B>Export objects</B></a> &nbsp;&nbsp;&nbsp; from project '$proj_name'.");
	        if (glob_isAdmin() or $_SESSION['s_suflag']>0) {
	            $outarr[1] = $outarr[1] . " (<a href=\"".$url_exp."&prefs=1\">show options</A>)";
	        }
	    }
	    
	    $this->tabrow($outarr);
	    
	    $notpossibleTxt  = "";
	    $role_right_name = "f.PaXml_import";
	    $role_right      = role_check($sqlo, $role_right_name);
	    if (!$role_right["execute"] && ($_SESSION['sec']['appuser']!="root")) {
	        $notpossibleTxt = "You must have role right '$role_right_name' or be root.";
	    }
	    if ($notpossibleTxt=="")
	        if ( !$o_rights["insert"]) $notpossibleTxt = "You do not have write permission on this project!";
	        
	        if ($notpossibleTxt!="") {
	            $outarr= array( $img_imp, "Import objects to project. &nbsp; &nbsp; <font color=red>Not possible:</font> $notpossibleTxt");
	        } else {
	            $outarr= array("<a href=\"" . $url_imp . "\">".$img_imp."</A>","<a href=\"" . $url_imp . "\"><B>Import paxml-file</B></a>  to project '$proj_name'");
	            
	        }
	        
	        $this->tabrow($outarr);
	        echo "</table>\n";
	        echo '<br />';
	        htmlInfoBox( "Short info", "", "open", "CALM" );
	        ?>
        <ul>
        <li>Export objects: by default, exports only public data (no production tables, like array-batch)</li>
        <li>users with flag s_suflag=1 have more options (export also internal production data)</li>
        </ul>
        <?php
        htmlInfoBox( "", "", "close" );
        
        // Mapping and flipping of ROID/WIID (piet):
        // only user root
        if (glob_isAdmin() ) {
            $url_map  = "rootsubs/ROID-map.php?PROJ_ID=" . $proj_id;
            // $url_flip = $_SESSION['s_sessVars']['loginURL'] . "/rootsubs/ROID-flip.php?PROJ_ID=" . $proj_id;
        
        	echo "<br><hr size=1 noshade>\n";
            echo "<br><b>root functions</b><br><br><ul>";
        	echo "<li><a href=\"" . $url_val . "\"><B>Validate PartisanXML</B></a> data file to database for project '$proj_name'.</a> &nbsp; </li>\n";
            echo "<li><a href=\"" . $url_map . "\">Generate roid/wiid mapping file</A>. Flipping master and copy of business objects in this project. This function runs on all business object linked to this project.";
            echo "</li>\n";
            echo "</ul>";
        }
        echo "</ul>";
		
	}
}
