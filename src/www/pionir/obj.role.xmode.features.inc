<?PHP
/**
 * Role editor: show current role rights
 * handle role-rights for:
 * - objects
 * - functions
 * - plugins
 * 
 * this script interacts with script obj.role.param.php : UPDATE role rights 
 * 

 * @swreq UREQ:
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        $o_rights = &$this->o_rights;
        $viewmode = $this->viewmode;
	
    	$editAllow = $o_rights["write"];
    	if ($viewmode!= "edit" ) $editAllow = 0;
    	
    	$rolShowLib = new oRoleDispMatrix( $sqlo, $id, $editAllow );
    	$r = $rolShowLib->getRights();
    	
    	$numObjRights2 = sizeof($r)*2;
    	?>
    	<UL>
        <script>
      	<!--
      	<?php 
      	//  javascript params
      	//  butt_id: 0,1,2,3 
    	//	num: number of fields
      	?>
    	function checkall( butt_id, num ) {
    		
    		i=0;
    		var butt_name = "checker_" + butt_id;
    		if (document.right_matrixo.elements[butt_name].value == 'check') {
    		  document.right_matrixo.elements[butt_name].value = 'uncheck';
    		  val = true;
    		} else {
    		  document.right_matrixo.elements[butt_name].value = 'check';
    		  val = false;
    		}
    		  
    		for( i=0; i<num; i++ )
    		{
    			document.right_matrixo.elements[i*<?echo $numObjRights2?>+2*butt_id].checked = val; 
    			document.right_matrixo.elements[i*<?echo $numObjRights2?>+2*butt_id+1].checked = val; 
    		}
    	}  	
    	//-->
      	</script>
    	<?
    	
        echo ("<B>Object rights:</B><br>\n");
        $rolShowLib->display_matrix( $sqlo, $r,"o", 0 );
        echo("<br>\n");
    
        echo ("<B>Function rights:</B> (right is DENIED, if not set)<br>\n");
        $rf=array();
        $rf[0]="execute";
        $rf[1]="deny";
        $rolShowLib->display_matrix( $sqlo, $rf, "f", 1 );
        echo("<br>\n");
        
        echo ("<B>Plugin rights:</B> (right is DENIED, if not set)<br>\n");
        $rp=array();
        $rp[0]="execute";
        $rp[1]="deny";
        $rolShowLib->display_matrix( $sqlo, $rp, "p", 1 );
    
        echo "</UL>";
        return 0;
    }

}

class oRoleDispMatrix{
	
	var $right_matrix;

    function __construct( &$sql, $id, $editAllow ) {
    	$this->roleid = $id;
    	$this->editAllow=$editAllow;
    	
    	$this->right[0]="read";
        $this->right[1]="write";
        $this->right[2]="insert";
        $this->right[3]="delete";
    	$this->right[4]="admin";
    }
    
    function getRights() {
    	return $this->right;
    }
    
    /**
     * get function rights
     * @param object $sql
     * @param $type_of_objects : "f."
     */
    function _f_rightsGet( &$sql, $type_of_objects) {
    
    	$right_matrix 	 = NULL;
    	$mq="select a.user_right_id, a.name, a.cct_right, a.notes from user_right a ".
    			" where a.name like '$type_of_objects%' order by upper(a.name)";
    	$sql->query($mq);
    	$right_matrix=array();
    	while($sql->ReadRow()) {
    		$right_name = $sql->RowData[1];
    		$right_matrix[$right_name][$sql->RowData[2]] = $sql->RowData[0];
    		
    		$this->niceNameInfo[$right_name] = array("nice"=>$right_name);
    		
    		// if notes exist -> save them
    		if ( $sql->RowData[3]!="" )   $this->niceNameInfo[$right_name]["notes"] = $sql->RowData[3];
    	} 
    	$this->right_matrix = $right_matrix;
    }
    
    function getRightMatrix ( 
    		&$sql, 
    		$type_of_objects // "f.", "o."
    	 ) {
    	// FUNCTION: get right matrix
    	// RETURN: $this->right_matrix = array( right_group, cct_right ) = user_right_id
    	// 		   $this->niceNameInfo[$tab_name] = array( "nice" => $nicename, "notes"=> ... );
    	
    	$this->niceNameInfo = NULL;
    	$this->right_matrix = NULL;
    	
    	switch ($type_of_objects) {
    		case "f.":
            	$this-> _f_rightsGet( $sql, "f." );   
            	break;
            case "p.":
            	$this-> _f_rightsGet( $sql, "p." );
            	break;
    		default:   
    	    	$right_matrix 		= NULL;
    	        $mq="select a.user_right_id, b.nice_name, a.cct_right, a.notes, b.TABLE_NAME from user_right a,cct_table b ".
    				" where a.name like '$type_of_objects%' and a.name='$type_of_objects'||b.table_name order by b.nice_name";
    	        $sql->query($mq);
    	        $right_matrix=array();
    	        while($sql->ReadRow()) {
    				$nicename  = $sql->RowData[1];
    				$cct_right = $sql->RowData[2];
    				$tab_name = $sql->RowData[4];
    	            $right_matrix[$tab_name][$cct_right] = $sql->RowData[0]; 
    				$this->niceNameInfo[$tab_name] = array( "nice" => $nicename );
    	        }
    	        $this->right_matrix = $right_matrix;
        }
    	
    }
    
    function _showFuncTab(
    	$type_of_objects,
    	&$rights,
    	&$right_vector,
     	$option=NULL      // "tabNoEnd" = 0,1
    					  // "tabNoHead"= 0,1
    	) {
    	
    	$editAllow    = $this->editAllow;
    	$numObjRights = sizeof($rights);
    	
    	$same_viualization=array('f.', 'p.');
    	
    	if ( !$option["tabNoHead"] ) {
    		?>  
    		<table border="0" cellspacing="1" bgcolor="black">
    		<tr>
    		<td>
    		<table border="0" cellspacing="1" cellpadding="1" width="100%" bgcolor="white">
    	
    		<tr bgcolor=#D0D0D0>
    			<?
    			
    			if ($type_of_objects=="o.") {
    				echo "<th>&nbsp;</th>";
    			}
    			echo "<th> Object </th>";
    			
    			foreach($rights as $ar) {  
    				$arout = $ar;
    				if ($ar=="execute") $arout="<font color=green>allow</font>";
    				if ($ar=="deny")    $arout="<font color=red>deny</font>";
    				if ($ar=="admin") $arout = "<font color=#FF7D1F>admin</font>";
    				
    				echo("<th width=40> ".$arout." </th>\n");
    			} 
    		
    		if ( in_array($type_of_objects, $same_viualization) ) {
    			echo "<th> <font face=\"Helvetica,Arial\">Notes</font> </th>\n";	
    		}
    		
    		echo "</tr>\n"; 
    	}
    
    	$cnt=0; 
    	$colarrCONST = array("#FFFFFF"=>"FFD0D0", "#EFEFFF"=>"#EFC0C0"); 
        $color1 = "#FFFFFF";
        $color2 = "#EFEFFF"; 
        $color = NULL;
        
    	foreach( $this->right_matrix as $rm_row_name=>$rm_row){
    	
    		if ($color == $color1)  $color = $color2;
    		else $color = $color1; 
    		$colorred = $colarrCONST[$color];
    		
    		$nice_row_name = $this->niceNameInfo[$rm_row_name]["nice"];
    		
    		echo "<tr bgcolor=".$color." align=\"center\">\n";
    		if ($type_of_objects=="o.") {
    			$tablename = $rm_row_name;
    			$icon = htmlObjIcon( $tablename, 1);
    			echo "<td><img src=\"".$icon."\"></td>";
    		}
    		echo "<td align=\"left\"> ".$nice_row_name." </td> \n";
    		
    		if( $editAllow ) {
    			foreach($rights as $ar) { 
    				$tmpcolor="";
    				if ($ar=='admin') $tmpcolor=" bgcolor=".$colorred; 
    				echo "<td".$tmpcolor.">";
    				if ( isset($rm_row[$ar]) ) {
    					echo ("<input type=\"hidden\" name=\"".$rm_row[$ar]."\" value=\"off\">\n");
    					echo ("<input type=\"checkbox\" name=\"".$rm_row[$ar]."\"");
    					foreach($right_vector as $rv) if($rv==$rm_row[$ar]) echo(" checked");
    					echo (" >");
    				}
    				echo "</td>\n";
    			};
    		} else {
    			foreach($rights as $ar) {
    
    				$tt=0;
    				$tmpcolor="";
    				if ($ar=='admin') $tmpcolor=" bgcolor=".$colorred; 
    				echo("<td".$tmpcolor.">"); 
    				if ( isset($rm_row[$ar]) ) {
    					foreach($right_vector as $rv) if($rv==$rm_row[$ar]) { echo("<img src=\"images/but.checked.gif\">"); $tt=1; };
    					if (!$tt) echo("<img src=\"images/but.checknx.gif\">");
    				}
    				echo("</td>\n"); 
    				
    			};
    		};
    		
    		if ( in_array($type_of_objects, $same_viualization) ) {
    			echo("<td align=left>&nbsp;".$this->niceNameInfo[$rm_row_name]["notes"] ."</td>");
    		}
    		$cnt++;
    		echo("</tr>");
    	};
    	
    		
    	if (($editAllow) && ( $type_of_objects=="o.")) { 
    		echo "<tr bgcolor=#EFEFEF>";
    		if ($type_of_objects=="o.") {
    			echo "<td>&nbsp;</td>";
    		}
    		echo "<td align=right><b>ALL</b></td>";
    		for ($i=0; $i<$numObjRights; $i++) {
    			echo "<td><input type=button name='checker_".$i."' value='check'
    			onclick=javascript:checkall(".$i.",".$cnt.")></td>\n";
    		}
    		echo "</tr>";
    	}
    	
    	if ( !$option["tabNoEnd"] ) {
    		echo("</table></td> </tr>  </table>");
    		if($editAllow) {
    			echo('<input type="submit" value="Set Rights" class="yButton">');
    			echo("</form>\n");
    		}
    	}
    }
    
    
    /**
     * display the right matrix
     * @param object $sql - database handle
     * @param array $rights - array with rights displayed  e.g. "execute" or "deny"
     * @param string $object_type_key - type of objects displayed in table (e.g. "o" - objects ; "f" functions)
     */
    function display_matrix( &$sql, $rights, $object_type_key ) {
    	 
    	$id = $this->roleid;
    	$editAllow =  $this->editAllow;
    	
    	echo("\n");
    	
    	if(sizeof($rights)==0) {
    		echo("No rights selected <br> \n");
    		exit(1);
    	}; 
         
        //$numObjRights = sizeof($rights);
    
        if($editAllow) {
         	$form_name = "right_matrix".$object_type_key;
            echo ("<form name=".$form_name." method=post action='obj.role.param.php?id=$id'>\n");
        };
    	
    	
    	$mq="select user_right_id from right_in_role where role_id=$id order by user_right_id";
        $sql->query($mq);
        $i=0;
        $right_vector=array();
        while($sql->ReadRow()) {
            $right_vector[$i]=$sql->RowData[0];
            $i++;
        }
         
        $object_prefix = $object_type_key.'.';
    
        $this->getRightMatrix ( $sql, $object_prefix );
    
        $this->_showFuncTab( $object_prefix, $rights, $right_vector );
    
    }

}