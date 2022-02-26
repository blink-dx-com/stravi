<?php

/**
 * DEVSIC GUI methods
 * - support 'x' : variable-NICE-name now
 * @package o.DEVSIC.gui.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
require_once ('func_form.inc');
require_once ("visufuncs.inc");
require_once ("o.DEVSIC.subs.inc");

/**
 * FORM for INFOARR
 */
class oDEVSIC_INFOARR_gui {
    
    var $MAX_NUM_ROWS=15;
    
    function __construct($id) {
        $this->id=$id;
        $this->devdic_lib = new oDEVSIC_obj_sub($id);
    }
    
    function get_info_struct($sqlo) {
        return $this->devdic_lib->get_info_struct($sqlo);
    }
    
    /**
     * 
     * @param array $checkarr -- INFOARR structure
     */
    function editfrom(array $checkarr) {
        
        $maxi    = $this->MAX_NUM_ROWS; // max rows
        
        $tabobj  = new visufuncs();
        $headOpt = array( "title" => "Service Plan Action Table");
        $headx   = array ("#", "Variable", "Title<br>(optional)", "Type", "Notes");
        
        
        echo "<form style=\"display:inline;\" method=\"post\" ".
            " name=\"extraform\"  action=\"p.php\" >\n";
        echo "<input type=hidden name='mod' value='DEF/o.DEVSIC.infoarr'>\n";
        echo "<input type=hidden name='go' value='1'>\n";
        echo "<input type=hidden name='id' value='".$this->id."'>\n";
        
        $tabobj->table_head($headx,   $headOpt);
        
        $feldtmp = array('checkbox'=>'checkbox', 'text'=>'text');
        
        $i=0;
        while ($i<$maxi) {
            
            $onerow      = $checkarr[$i];
            if (empty($onerow)) $onerow=array();
            
            $preselected = $onerow['type'];
            if ($preselected==NULL) $preselected='checkbox';
            $typeone = formc::selectFget('row['.$i.'][type]',$feldtmp,$preselected);
            
            $dataArr = array(
                ($i+1),
                '<input type=text name="row['.$i.'][t]"  value="' .$onerow['t']. '" size=15 maxlength="'.
                   oDEVSIC_subs::INFOARR_var_MAX_SIZE.'">',
                '<input type=text name="row['.$i.'][x]"  value="' .$onerow['x']. '" size=20 maxlength='.
                   oDEVSIC_subs::INFOARR_title_MAX_SIZE.'>',
                $typeone,
                '<input type=text name="row['.$i.'][n]"  value="' .$onerow['n']. '" size=60 maxlength='.
                oDEVSIC_subs::INFOARR_notes_MAX_SIZE.'>'
            );
            
            $tabobj->table_row ($dataArr);
            $i++;
        }
        
        $tabobj->table_close();
        echo '<input type=submit class="yButton" value="Speichern">'."\n"; // SUBMIT
        echo "</form><br>\n";
        
        echo "<br><br>";
        htmlInfoBox( "Kurzhilfe", "", "open", "HELP" );
        
        
        ?>
        
        <b>News:</b> since 28.12.2021: New field "Variable"; Title-field is now optional<br><br>
        <b>Fields:</b>
		<ul>
		    
		    <li><b>Variable</b>: this name must be UNIQUE in this plan; max <?php echo oDEVSIC_subs::INFOARR_var_MAX_SIZE; ?>
		      Chars allowed; can contain only Letters, Numbers, underscore "_", minus, brackets "()" or spaces </li>
			<li><b>Title</b>: [OPTIONAL], max  <?php echo oDEVSIC_subs::INFOARR_title_MAX_SIZE; ?> characters allowed; </li>
			<li><b>Typ</b>:  Type of user input in Service-Plan-Step: Checkbox or Text</li>
			<li><b>Notes</b>: Notes to the Service-Plan-Step</li>
		</ul>
		<?
		htmlInfoBox( "", "", "close" );
	}
	
	function viewfrom($checkarr) {
		$tabobj = new visufuncs();
		$headOpt = array( "title" => "Service Plan Action Table");
		$headx  = array ("#", "Variable", "Title", "Type", "Notes");
		
		$tabobj->table_head($headx,   $headOpt);
	
		$maxi = $this->MAX_NUM_ROWS; // max rows;
		$i=0;
		while ($i<$maxi) {
		    
			$onerow = $checkarr[$i];
			$var  =$onerow['t'];
			$title=$onerow['x'];
			
			
			$dataArr = array(
				$i+1, 
			    $var, 
		        $title,
				$onerow['type'],
				htmlspecialchars($onerow['n'])
			 );
				 
			$tabobj->table_row ($dataArr);
			$i++;
		} 
		
		$tabobj->table_close();
	}
}
