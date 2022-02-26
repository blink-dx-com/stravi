<?php

/**
 * new form class
 * @package f.form2.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @example <pre>
 * 
	require_once ('gui/f.form2.inc');
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select compare mode";
	$initarr["startform"]   = -1, 0, [1]; start the form 
	$initarr["submittitle"] = "Submit";

	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $tablename;

	$formobj = new f_form2($initarr, $hiddenarr, 0);
    
	$fieldx = array ( 
		"title" => "Overview", 
		"name"  => "action",
		"object"=> "radio",
		"val"   => $parx["action"], 
		"inits" => "overview",
		"notes" => "show only the protocol names"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close();
	
  </pre>
  
 */
class f_form2 {
    
    /**
    * - init the HTML-form
    * - set <form> tag
    * - start form table
    * - manage hidden-fields
    * - variable-array: parx[]
    *
    * @param $initarr <pre>
    "action"  = URL
    "ENCTYPE" = "", "multipart/form-data"
    "title"
    "submittitle"
    */
    function __construct ( &$initarr,  &$hiddenarr,  $go  ) {
            
            $this->initarr   = $initarr;
            $this->hiddenarr = $hiddenarr;
            $this->go        = $go;  
    }
    
    /**
     * start form tag
     */
    function start_form_only() {
        
        $initarr = &$this->initarr;
        
        $tmpform = $initarr["FORMXTRA"];
        if ( $initarr["ENCTYPE"] != "" ) $tmpform .= " ENCTYPE=\"".$initarr["ENCTYPE"]."\"";
        echo "<form method=\"post\" name=\"editform\" action=\"". $initarr["action"] ."\" ".$tmpform.">\n"; 
    }
    
    /**
     * start form tag
     */
    function start_all() {
        
        echo '<div class="container" id="form_id">'."\n";
        $this->start_form_only();
       
        echo '<h2>'.$this->initarr['title'].'</h2>'."\n";
    }
    
    /**
     * 
     * @param array  $fieldx
     * 'object'
     *    'select'
     *       'inits_opt' OPTIONAL: ['complex'], 'simple' -- type of inits-field
     *       'inits' : array(array(id, text))  [complex]
     *                 or array(id1, id2, id3) [simple]
     * 'ext_row_class' : extend the row class
     * 'inits' => e.g.
     */
    public function fieldOut( $fieldx ) {
        
        $ext_row_class=NULL;
        if ($fieldx['ext_row_class']) {
            $ext_row_class = ' '.$fieldx['ext_row_class'];
        }
        $field_name = $fieldx['name'];
        $title = $fieldx['title'];
        
       
        echo '<div class="form-group row '.$ext_row_class.'">'."\n"; // <form-group row>
        
        if ($fieldx['object']=='info') {
            echo '<h3>'.$title.'</h3>';
            echo '</div>'."\n";
            return; // !!!
        }
        if ($fieldx['object']=='space') {
            echo '&nbsp;';
            echo '</div>'."\n";
            return; 
        }

        echo ' <div class="col-sm-3">'.$title.'</div>'."\n";
        // echo ' <label class="col-sm-2 col-form-label">'.$fieldx['title'].'</label>'."\n";
        echo ' <div class="col-sm-6">'."\n";
        switch ($fieldx['object']) {
            case 'checkbox':  
                $tmpopt = "";
                if ( $fieldx["val"] == 1 ) $tmpopt = " checked";
                echo '  <input type="checkbox" class="form-check-input" name="'.$field_name.'" value="1" '.$tmpopt.'>'."\n";
                break;
            case 'select':
                $select_type='complex';
                if ($fieldx['inits_opt']=='simple') $select_type='simple';
                    
                $val = $fieldx["val"];
                echo '  <select class="form-control" name="'.$field_name.'" >'."\n";
                foreach( $fieldx['inits'] as $row) {

                    if ($select_type=='complex') {
                        $id  =$row[0];
                        $text=$row[1];
                    } else {
                        $id  =$row;
                        $text=$row;
                    }
                    $tmpsel = "";
                    if ( $val == $id ) {
                        $tmpsel = "selected";                 
                    }
                    echo  "<option value=\"". $id ."\" ".$tmpsel."> ".$text."</option>\n";
                }
                echo '  </select>'."\n"; 
                break;
            case 'info':
                break;
            case 'text':
                echo '  <input type="text" name="'.$field_name.'" class="form-control" value="'.$fieldx["val"].'">'."\n";
                break;
            default:
                echo '  <input type="text" class="form-control-plaintext" value="">'."\n";
        }
        
        echo ' </div>'."\n";
        echo ' <div class="col-sm-3">'.$fieldx['notes'].'</div>'."\n";
        
        echo '</div>'."\n"; // </form-group row>
       
    }
    
    /**
     * - set submit buttons
     * - set hidden fields
     * - close form
     * - close table
     *
     * @param $option = NULL
     *  - ["allowFlag"]  
     *     [TRUE] - submit button ACTIVE
     *     FALSE  - submit button incative
     *  - ["noSubmitButton"]  0|1 do not show any submit button
     *  - ["noRow"]      0|1 do not show any BOTTOM row (including the buttons) !
     *  - ["addObjects"] = "TEXT" TEXT contains HTML text like extra buttons
     *  - ["noBackButton"] = 0|1
     */
    function close( $option = NULL ) {
        
        $gotmp = $this->go + 1;
        if ( $this->initarr["goNext"]!="" ) $gotmp = $this->initarr["goNext"];
        echo "<input type=hidden name=\"go\" value=\"".$gotmp."\">\n";
        
        if (sizeof($this->hiddenarr)) {
            foreach( $this->hiddenarr as $idx=>$valx) {
                $valx = str_replace('"', '&quot;', $valx); // replace " by &quot;
                echo "<input type=hidden name=\"". $idx ."\" value=\"".$valx."\">\n";
            }
            reset($this->hiddenarr);
            echo "\n";      
        }
        echo "\n";
        
        echo "</form>\n";
        echo '</div>'."\n";
    }
    
}