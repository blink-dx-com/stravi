<?php
require_once 'gui/g.modfavs.inc';

/**
 * show module favorites
 * GLOBAL $_SESSION['userGlob']['modfavs'] = komma separated list of module IDs
 * @package modfavs.inc
 * @swreq UREQ:14206  g.plugin > Plugin-Favorites for Users
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 
 */
class modfavs_XPL extends gPlugin {
	
	function register() {
		
		$title		  = 'My Plugin Favorites';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow'] = array (
			array ('home.php', 'home') ,
		);

		
	}
	
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		
		$fav_arr = g_modfavs::get_my_favs($sqlo);
		if (empty($fav_arr)) {
		    echo "No favorites found.";
		    return;
		}
		
		echo "<ul>\n";
		foreach($fav_arr as $row) {
		    $mod_id=$row['mxid'];
		    $mod_info = g_modfavs::get_mod_info($sqlo, $mod_id);
			echo '<li><a href="'.$mod_info['url'].'">'.$mod_info['title'].'</a>'."</li>";
		}
		echo "</ul>\n";
		echo "<br><br>\n";
		
	}
	
}