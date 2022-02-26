<?php
/**
 * various tests
 * @package 
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   int $go
 * @param string $action
 */
class formtest_XPL extends gPlugin {
	
	
	function register() {
		
		$title		= 'GUI tests';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/rootFuncs.php', 'home') );
		
		$this->infoarr['jsFile']   = 'res/choices/scripts/choices.min.js';
		$this->infoarr['cssFile']  = array(
		    'res/choices/styles/base.min.css',
		    'res/choices/styles/choices.min.css',
		);
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('read') )
			);
	}
	

	function _multiselect() {
	    
	    echo "REQUEST:".print_r($_REQUEST)."|<br><br>";
	    
	    ?>
	    
	     <h2>Multiple select input</h2>
    <form style="display:inline;" method="post"  name="editform"  action="<?php echo $_SERVER['PHP_SELF'];?>" >
    <div style="color: #000000; background-color: #FFFFFF;">
    <label for="choices-multiple-remove-button">With remove button</label>
    <select class="form-control" name="choices[]" id="choices-multiple-remove-button" placeholder="This is a placeholder" multiple>
    <option value="Choice 1" selected>Choice 1</option>
    <option value="Choice 2">Choice 2</option>
    <option value="Choice 3">Choice 3</option>
    <option value="Choice 4">Choice 4</option>
    </select>
    </div>
    <input type=hidden name="action" value="multiselect">
    <input type=hidden name="mod" value="<?php echo $this->_mod;?>">
    <input type=submit value="Submit" class="yButton">
    </form>
    
    
    <small>If the following example do not load, the Discogs rate limit has
    probably been reached. Try again later!</small
              >
   
   <script>
    // Pass single element
  var multipleCancelButton = new Choices(
          '#choices-multiple-remove-button',
          {
            removeItemButton: true,
          }
        );
  
   </script>
<?php
	    
	    
	}
	
	function startMain() {
		global $error;
		
		
		$sqlo = &$this->_sqlo;
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$action = $_REQUEST['action'];
		
		echo "action: ".$action."<br><br>\n";
		
		switch ($action) {
		    case 'multiselect':
		        $this->_multiselect();
		        break;
		}
	}
}
