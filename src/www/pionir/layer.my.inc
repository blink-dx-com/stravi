<?php
require_once 'subs/layer.my.subs.inc';

/**
 * MY layer for home.php
 * @package layer.my.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $action :
 *   'user-list-coll' show user list collection
 */
class themeCls extends gThemeClsAbs {
   
    
    function layer_params() {
      $lparams = array ( "bodycss" => "xBodyNorm");
      return ($lparams);
    }
    
    
    
    function layer_show( &$sqlo, &$sqlo2 ) {
        global $error;
    	$homeFuncObj = &$this->__homeFuncObj;

    	?>
    	<style>
    	.x-bg-text {
    	    /*background-color: rgb(0,0,0);*/ /* Fallback color */
    	    /*background-color: rgba(0,0,0, 0.4);*/ /* Black w/opacity/see-through */
    	    color: white;
    	    font-weight: bold;
    	    /* border: 3px solid #f1f1f1; */
    	    position: absolute;
    	    top: 40px;
    	    left: 20px;;
    	    /*
    	    transform: translate(-50%, -50%);
    	    width: 80%;
    	    */
    	    z-index: 2;
    	    padding: 20px;
    	    text-align: left;
    	}
    	.x-bg-img {
            background-image: url(images/bg.my.jpg);
            background-size: cover;
            /* filter: blur(4px); */
            padding: 50px;
        }
        </style>
    	<?php
    	
    	$svg_text = f_html_img::svg_file_load('res/img/settings.svg');
    	$button='<a href="p.php?mod=DEF/g.layer.my_config" style="color:white;">'."\n". $svg_text.  ' Config</a>';
    	echo '
        <div class="x-bg-img"></div>

    	<div class="x-bg-text">
    	 <span style="font-size: 2em;">my world</span><span style="padding-left:30px;">&nbsp;</span>'. $button .
        '</div>
        <div style="padding-top:20px;">&nbsp;</div>
        ';  	
    	
    	$main_lib = new g_theme_my_cls();
    	$main_lib->show($sqlo, $sqlo2);
    }

}