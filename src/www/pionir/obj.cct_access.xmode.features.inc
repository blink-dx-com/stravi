<?php
require_once ("f.objview.inc");	

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
  
        echo "<ul>";

        do {
      
        	  $sqlo->query('SELECT table_name FROM cct_access WHERE cct_access_id = '.$sqlo->addquotes($id));
        	  if (!$sqlo->ReadRow()) return -1;
        	  $tab = $sqlo->RowData[0];
        	  if (empty($tab)) {
        		 echo '<font color=red>Warning:</font> no table defined!<br>';
        		 break;
        	  }
        	  
        	  $sqlo->query('SELECT count(1) FROM '.$tab.' WHERE cct_access_id = '.$id);
        	  $sqlo->ReadRow();
        	  $numused = $sqlo->RowData[0];
        	  
        	  if ($numused>1) {
        	  	echo "<font color=red>Warning:</font> this entry is used by <B>$numused</B> objects!<br>";
        		echo '<a href="view.tmpl.php?t='.$tab.'&condclean=1&tableSCond=x.CCT_ACCESS_ID='.$id.'">=> view list</a>';
        	  	break;
        	  }
        	  
        	  $sqlo->query('SELECT '.PrimNameGet2($tab).' FROM '.$tab.' WHERE cct_access_id = '.$sqlo->addquotes($id));
        	  if (!$sqlo->ReadRow()) {
        		echo '<font color=red>Warning:</font> no relation!';
        		break;
        	  }
        	  $objid = $sqlo->RowData[0];
        	  
        
        	  // get edit-link of BO + NAME + icon (object)
        	  $objLinkLib = new fObjViewC();
        	  $objhtml = $objLinkLib->bo_display( $sqlo, $tab, $objid );
        			
        	  echo 'used by object: <b>'.tablename_nice2($tab).' '.$objhtml.'</b> [ID:'.$objid.']<br>'."\n";
      
      } while (0);
      
      echo "</ul>\n";
      return 0;
   }

}
