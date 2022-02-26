<?php
require_once ('o.USER_PREF.manage.inc');
require_once ("func_head.inc");  

/**
  * info about user [preferences]
 * @package o.DB_USER.info.inc

 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   
 *   [clearPrefs=1]
	  [go=0/1]   for clearPrefs
      [$tablename] for a special tablename
 */
class o_DB_USER_info_XPL extends gPlugin {

	
	function register() {
		
		$this->infoarr			 = NULL;
		$this->user_id =  $_SESSION['sec']['db_user_id'];
		$title   =  "User system information";
		
		$this->infoarr["title"] = $title;
		$this->infoarr["form_type"]= "obj";
		$this->infoarr["obj_name"] = "DB_USER";
		$this->infoarr["obj_id"]   = $this->user_id;
		$this->infoarr["checkid"]  = 1;

		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
			);

	}
	
	function _clearpref( $tablename ){
	    
	    
	    $cntis=0;
	    $tabpat = "o.". $tablename. ".";
	    $userGlobTmp = $_SESSION['userGlob'];
	    
	    foreach( $userGlobTmp as $key=>$val) {
	        
	        if ( strpos($key, $tabpat) !== FALSE ) {
	            unset ($_SESSION['userGlob'][$key]);
	            // $_SESSION['userGlob'][$key] = "";
	            $cntis++;
	        }
	    }
	    
	    echo "<br><font color=gray>Cleared preferences:</font> $cntis<br>";
	    
	}
	
	

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		
		$clearPrefs=$_REQUEST['clearPrefs'];
		$go=$_REQUEST['go'];
		$tablename=$_REQUEST['tablename'];

		if (empty($clearPrefs)) $clearPrefs = 0;
		if (empty($go)) $go = 0;
		
		
		if ($clearPrefs ) {
		    if ($go == 0) {
		        if ($tablename == "") {
		            echo '&nbsp;<blockquote>Are you sure, you want to reset all your preferences?<br>&nbsp;<br>';
		            echo '[<a href="'.$this->_plug_get_URL().'&clearPrefs=1&go=1">YES</A>] | ';
		            echo '<a href="'.$this->_plug_get_URL().'">no</A>';
		            htmlFoot('</blockquote>');
		        } else {
		            $this->_clearpref( $tablename );
		        }
		        
		    } else {  // Do IT !!!
		        $_SESSION['userGlob'] = array();
		    }
		}
		
		$sqlo->query("SELECT COUNT(1) FROM cct_access WHERE db_user_id = ".$this->user_id);
		if ($error->printLast()) htmlFoot();
		$sqlo->ReadRow();
		$setsnum = $sqlo->RowData[0];
		$prefnum = sizeof($_SESSION['userGlob']);
		
		echo "<UL><LI> <B>Data sets: </B>You are owner of <B>$setsnum</B> data sets.<br>";
		?>

        <LI> <B>User preferences:</B> [<a href="obj.db_user.settings.php">edit</a>]
        <UL>
        <?
        echo "You have $prefnum settings. &nbsp; &nbsp; [<a href='".$this->_plug_get_URL()."'>Show all</a>]  
            [<a href='".$this->_plug_get_URL()."&clearPrefs=1'>Reset all settings</a>]<br>";
        echo '<I>User preferences will be saved during the [logout].</I><br>'; 
        $tabpat = "";
        
        if ( $tablename !="" ) { 
             $tabpat = "o.". $tablename. ".";
             $nice_name =  tablename_nice2($tablename);
        
             echo "<font color=gray>Only for object type:</font> <B>".$nice_name."</B>: [<a href=\"".$this->_plug_get_URL()."&tablename=".$tablename."&clearPrefs=1\">reset</a>]<br>";
        }
        echo "<br>";
        $cntis=0;
        
        if ($prefnum) {
          foreach( $_SESSION['userGlob'] as $key=>$val) {     
          
            $put_out = 0;
        	if ($tabpat=="") $put_out = 1;
            else {   
                if ( strpos($key, $tabpat) !== FALSE ) {
                    $put_out = 1; 
                } 
            }
            if ( $put_out ) {
                echo "$key : $val<br>"; 
                $cntis++;
            }
          }
          
           
          if ($tabpat!="") echo "<br><font color=gray>Found preferences:</font> $cntis<br>";
          
        }
        
        echo '</ul></ul>';    
		
	}
	
}
