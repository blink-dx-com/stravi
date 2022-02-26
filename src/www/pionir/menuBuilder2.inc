<?php
/**
 * build HTML-CSS-menu
 * @package menuBuilder2.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0003213: g > HTML MENU generator (library)
 */

/* dart to left as sign for 'close menu'
 * action at 'close menu'
 * $miCloseAction = 'javascript:hideAllBut(0);';

 * this class provides a datastructure for a menu. a menu is a container for
 * MenuItem's objects

  active or not ? that's the question.
	$miActive   = 0;
	$miInactive = 1;
	$miRuler    = 2;
	
	// direction of menu
	$miVertical = true;
	$miHorizontal = false;
 * Enter description here ...
 * @author steffen
 *
 */
class Menu {
  var $isVert;        // true - vertical menu / false - horizontal menu
  var $popInd;        // the sign used to show, that there's a submenu
  var $x;             // relative position (root menu relative to window)
  var $y;
  var $width;         // width/height of menu (depends on isVert)
  var $overCol;       // color at mouseover event
  var $backCol;       // normal color
  var $borderClass;   // stylesheet class used for item borders
  var $textClass;     // stylesheet class used for text within items
  var $length;        // general length or width of subitems
  
  function __construct($isVert, $popInd) {
	global $glLength;

	$this->isVert = $isVert;
	$this->popInd = $popInd;
  }
}



// this class provides a datastructure for menu item.
class MenuItem {
  var $text;          // text of item
  var $href;          // url ('http://...', '#anchor')
  var $frame;         // browser frame
  var $spacing;       // spacing to next item
  var $target;        // submenu - 0 no submenu
  var $active;        // 0 - active / 1 - inactive (optional parameter)
                       // 2- HR tag
  var $length;        // another length as general length of Menu object
                      // if this value is not null it forces the program to
                      // use this value (optional parameter)

  function __construct($text, $href, $target, $active = 0, $length = null) {
	
        
	$this->text = $text;
	$this->href = $href;
	$this->frame = null;
	$this->spacing = -1;
	$this->target = $target;
	$this->active = $active;    // 0 = active, 1 = not active
	// $this->length = $length;
  }
}

/*
 in script, where to integrate menu ...

 menu[X][0] must be always set with instance of 'Menu' class
 menu[X][Y] with Y > 0 must be alway set with instance of 'MenuItem' class

 menu[menuID][menuItemNumber]

      menuID - meaning: identifying and order
      menuItemNumber - meaning: only for order

     write menu

    // horizontal menu, no popup indicator, x = 5, y = 5 (relative to window), height of 20, colors and styles
    $menu[0][0] = new Menu($miHorizontal, '', 5, 5, 20, 25, 'menuBorder', 'itemText');
    // itemtext = ' M ', link = '#', no special frame, width=25, spacing =1 and submenu=1
    $menu[0][1] = new MenuItem('<center>M</center>', '#', 1, 0);

    // horizontal menu, no popup indicator, x = 30 (relative to window), y = 0 (relative to parent menu), height of 20, ...
    $menu[1][0] = new Menu($miHorizontal, '', 30, 0, 20, 70, 'itemBorder', 'itemText');
    // item text, no link, no frame, width if item, spacing (-1 means overlapping of one pix), submenus)
    $menu[1][1] = new MenuItem($popdown . 'edit ', '',  3);
    $menu[1][2] = new MenuItem($popdown . 'object ', '',   9);
    $menu[1][3] = new MenuItem($popdown . 'view ', '', 7);
    $menu[1][4] = new MenuItem('&nbsp; &nbsp; search', '', -1, 140);
    $menu[1][5] = new MenuItem($popdown . 'application ', '',  2, $miActive , 140);
    $menu[1][6] = new MenuItem(' &nbsp; ? ', '#', -1, $miActive , 20);
    $menu[1][7] = new MenuItem($miCloseSign, $miCloseAction,  0, $miActive , 20);

    $menu[2][...] = ...

    $menu[3][...] = ...


     add to body tag:

        echo $mb->addToBodyTag();



     and add directly after body tag
        <script language='JavaScript'>
        if (!isNS4)
        {
            writeMenus();
            window.setInterval("checkPosition()", 700);
        }
        </script>
*/



// create a menu in HTML
class menuBuilder {
	
	function __construct( $textOnly=NULL ) {
		$this->textOnly = $textOnly;
	}
  
	function _subMenu( &$menu, &$items, $level ) {
		
	  	echo "  <ul>\n";
		for ($iidx = 0; $iidx < count($items); $iidx++) {
			// WARNING! no check for valid variables
			if ($iidx == 0) {
			  // top entry
			  
			} else {
				if ($items[$iidx]->active==2) {
					echo '<li><hr>'; // <hr>
				} else {
				    if ($items[$iidx]->active==1) echo '  <li><span>'.$items[$iidx]->text.'</span>';
				    else echo '  <li><a href="'.$items[$iidx]->href.'">'.$items[$iidx]->text.'</a>';
				    
				}
			  // ;
			  // $items[$iidx]->active;
			  // $items[$iidx]->spacing;
			  // ];
			 
			  if ( $items[$iidx]->target ) {
			  	$targetid = $items[$iidx]->target;
		  		$this->_subMenu($menu, $menu[$targetid], 2 );
			  }
			  
			  echo "</li>\n";
			  
			}
	    }
		   echo "  </ul>\n";
	    }
  
  // run this function to write menu code
  function createMenu($menu, $toRoot = "") {
	
	// array ?
	if (!is_array($menu))
	  return false;
        

	// add slash to root if necessary
	if ($toRoot != "")
	  if ($toRoot[strlen($toRoot) - 1] != '/')
		$toRoot .= '/';
	
	echo '<div class="menuBox yMColor1">'."\n"; // OUTER DIF
	echo '<div class="menu">'."\n";

	// e.g. menu[0]
	  $items  = &$menu[0];
	 
	  echo "<ul>\n";
	  for ($iidx = 0; $iidx < count($items); $iidx++) {
		// WARNING! no check for valid variables
		if ($iidx == 0) {
		  // top entry
		} else {
		  echo '<li><a href="#">'.$items[$iidx]->text.'</a>';
		  
		  if ( $items[$iidx]->target ) {
		  	echo "\n";
		  	$targetid = $items[$iidx]->target;
		  	$this->_subMenu($menu, $menu[$targetid], 1);
		  }
		  
		  echo "</li>\n";
		  
		}
	  }
	  echo "</ul>\n";
	
        
	echo "</div>\n";
  }
  
  function menuRight($text) {
  	
	echo '<div class="right1">' . "\n";
	echo $text;
	echo '</div>'. "\n";
	echo '</div>'. "\n"; // OUTER DIF
	echo '<div style="clear: both;"></div>'. "\n";
  }
  
  //@deprecated
  static function getCssInclude() {
  	return '<link rel="stylesheet" type="text/css" href="res/css/glob.menu.css" />'."\n";
  }
  static function getCssInclude2() {
      return '<link rel="stylesheet" type="text/css" href="res/css/glob.menu2.css?d=1" />'."\n";
  }
  
  function printAsText( &$menu ) {
      
  	$mapping = array();
  	$menux   = array();
        
	$index = 0;
	// array ?
	if (!is_array($menu))
	  return false;
	  
	foreach($menu as $key => $value) {
	  $mapping[$key] = $index++;
	}

	
	foreach($menu as $key => $items) {
	  
	  for ($iidx = 0; $iidx < count($items); $iidx++) {
		
		if ($iidx == 0) {
		  $menux[$mapping[$key]][$iidx] = array(); // new menu
		} 
		
	  }
	}
	
	$subMenucnt=0;
	$row = 0;
	$col = 0;
	$txtarr = NULL;
	$mainMenuCol = 0;
	foreach($menu as $key => $items) {
	  $menux[$mapping[$key]] = Array();
	  
	  for ($iidx = 0; $iidx < count($items); $iidx++) {
		
		if ($iidx == 0) {
		  $isVertical  = $items[$iidx]->isVert;
		  $menux[$mapping[$key]][$iidx] = array(); // new menu
		  if ($isVertical) {
		  	$row = 1;
			$col = $mainMenuCol;
			$mainMenuCol++;
		  } else {
		  	$col = 0;
		  }
		} else {
		  $params=array();
		  $params["text"]  = $items[$iidx]->text;
		  $params["href"]  = $items[$iidx]->href;
		  $params["active"]= $items[$iidx]->active;
		  // $params["target"]= ($mapping[$items[$iidx]->target] == null) ? 0 : $mapping[$items[$iidx]->target];
		  
		  $celltxt = "";
		  if ($params["href"]!="") $celltxt = "<a href=\"".$params["href"]."\">".$params["text"]."</a>"; 
		  else $celltxt =  $params["text"];
		  $txtarr[$row][$col] = $celltxt;
		  
		  if (!$isVertical) { // horinzontal ?
			$col++;
		  } else {
		  	$row++;
		  }
		  
		}
	  }
	  $subMenucnt++;
	}
	
	// print text
	echo "<table>";
	foreach( $txtarr as $row=>$subarr) {
		echo "<tr>";
		foreach( $subarr as $col=>$textx) {
			echo "<td>".$textx."</td>";
		}
		echo "</tr>\n";
	}
	echo "</table>\n\n";
	reset ($txtarr); 
  	
  }
}
