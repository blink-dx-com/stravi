<?php
require_once ('f.objview.inc');
/**
 * - show all role-righs for a user
 * - analyse table USER_RIGHT
 * @package o.DB_USER_rights_show.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oDB_USER_rigAllRig {
    
    function __construct( $user_id ) {
        $this->user_id = $user_id;
        
    }
    
    /**
     * - Get $sql - sql handle
     * - Get $object_type - like "o." , "f."
     * - Get $user_id
     * - Get $rights vector
     * @param $sql
     * @param $object_type 'o.', 'f.'
     * @param $rights
     * @param $is_func  0=object(default)  1=function
     * @param $opt
     */
    function _display_rights( &$sql, $object_type, &$rights, $is_func=0, $opt=NULL ){
        $user_id = $this->user_id;
        
        if(!$is_func) {
            $mq="select name,cct_right from user_right where user_right_id in ".
                " (select user_right_id from right_in_role where role_id in ".
                "(select role_id from user_roles where db_user_id=$user_id)) and name like '$object_type%' order by name";
        } else {
            $mq="select name,cct_right, user_right_id, notes from user_right where user_right_id in (select user_right_id from right_in_role where role_id in (select role_id from user_roles where db_user_id=$user_id)) and name like '$object_type%' order by name";
        };
        $sql->query($mq);
        
        $rights_matrix=array();
        $right_notes = array();
        $right_order = array(); // order of display
        
        $object_type_len = strlen($object_type);
        while($sql->ReadRow()) {
            $objname = $sql->RowData[0];
            if (!$is_func) {
                $objname = substr($objname,$object_type_len); // get real table name
            } else {
                $right_order[$objname]=1;
                $right_notes[$objname]=$sql->RowData[3];
            }
            $rights_matrix[$objname][$sql->RowData[1]]=1;
            
        }
        
        if ( !sizeof($rights_matrix) ) {
            echo 'not rights.<br>';
            return;
        }
        
        if (!$is_func) {
            // get nice name, sort
            reset ($rights_matrix);
            foreach( $rights_matrix as $table=>$val) {
                $nice = tablename_nice2($table);
                $right_order[$table]=$nice;
            }
            reset ($rights_matrix);
            
            // sort by nice-name
            asort($right_order);
        }
        
        $width="";
        if ($opt["width"]=="FULL") $width="width=100%";
        
        // glob_printr( $rights_matrix, "rights_matrix info" );
        
        echo '<table border="0" cellspacing="1" '.$width.' bgcolor="black">';
        ?>
        <tr>
        <td>
         <table border="0" cellspacing="0" width="100%" bgcolor="white">
    
          <tr bgcolor=#D9D9D9>
            <th> Object </th>
            <?
            foreach($rights as $ar) {
                $arout = $ar;
                if ($ar=="execute") $arout="<font color=green>allow</font>";
                if ($ar=="deny")    $arout="<font color=red>deny</font>";
    			if ($ar=="admin")   $arout = "<font color=#FF7D1F>admin</font>";
                echo("<th width=50> ".$arout." </th>\n"); 
            }
            
    	    if ($is_func) echo "<th>Notes</th>\n";
            
            echo "</tr>\n";
                
        $colarrCONST = array("#FFFFFF"=>"FFD0D0", "#EFEFFF"=>"#EFC0C0"); 
        $color1 = "#FFFFFF";  // SUN violett 
        $color2 = "#EFEFFF";  
    
      $objViewLib = new fObjViewC();
      $color='';
      
      foreach($right_order as $obj_name => $obj_nice) {
      	
      	 $obj_rights = $rights_matrix[$obj_name];
         if ($color == $color1)  $color = $color2;
         else $color = $color1;
    	 $colorred = $colarrCONST[$color];
         
         echo "<tr bgcolor=".$color.">\n ";
         echo "<td>";
         if (!$is_func) {
         	$tablename = $obj_name;
         	$iconUrl   = $objViewLib->_getIcon($tablename);
         	echo '<img src="'.$iconUrl.'"> ';
         	$objout_nice = $obj_nice;
         	if ($obj_nice=="") $objout_nice = $tablename;
         } else {
         	$objout_nice = $obj_name;
         }
         echo $objout_nice."</td>\n";
         
         
         foreach($rights as $mr) {
    	 
    	 	$tmpcolor="";
    		if ($mr=='admin') $tmpcolor=" bgcolor=".$colorred; 
    	 
            echo("<td".$tmpcolor." align=center>");
            if($obj_rights[$mr]) echo("<img src=\"images/but.checked.gif\">");
               else  echo("<img src=\"images/but.checkno.gif\">");
            echo("</td>\n");
    		
         };
         if ($is_func) echo "<td>&nbsp;".$right_notes[$obj_name]."</td>\n";
         echo("</tr> \n");
      };
    
      echo("</table> \n </td>\n </tr> \n </table>\n");
    }
    
    function sh_objects( &$sqlo ) {
    	$myr=NULL;
    	$myr[0]="read";
    	$myr[1]="write";
    	$myr[2]="insert";
    	$myr[3]="delete";
    	$myr[4]="admin";
    	
    	echo("<Font size=+2 > Objects </font> <br>");
    	$this->_display_rights( $sqlo, "o.", $myr );
    	echo("<br /> &nbsp; <br />\n"); 
    }
    
    function sh_functions( &$sqlo ) {
        $myrf=array();
    	echo("<Font size=+2> Functions </font> (right is DENIED, if not set) <br>");
    	$myrf[0]="execute";
    	$myrf[1]="deny";
    	$this->_display_rights( $sqlo, "f.", $myrf, 1);
    }
	
}
