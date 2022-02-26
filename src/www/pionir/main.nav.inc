<?php

/*MODULE: main.nav.inc  
  DESCR: contains main navigation functions
  AUTHOR: qbi
  VERSION: 0.1 - 20040505 	
*/ 

function navGetHomeUrl( &$sql ) {
	
    $firstpage = "home.php"; 
    $firstpage_tmp = $_SESSION['globals']["htmlFrameTop.homeBut"];
    if ( $firstpage_tmp !="" ) $firstpage = $firstpage_tmp; 
    return ($firstpage); 
}  

