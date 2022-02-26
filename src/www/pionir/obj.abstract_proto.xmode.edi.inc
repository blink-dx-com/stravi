<?php
require_once "visufuncs.inc";
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.S_VARIO.subs.inc';
require_once "gui/o.PROTO.stepout1.inc";
require_once 'func_form.inc';
require_once 'f.msgboxes.inc';
require_once 'subs/obj.abstract_proto.guisub.inc';

/**
 * protocol steps editor 
 * save data in DB: o.ABSTRACT_PROTO.mod.inc
 * - since 2020-05-10 with Step: Drag-Drop-Support
 * @package obj.abstract_proto.xmode.edi.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ: o.ABSTRACT_PROTO > abstract protocol steps anzeigen/editieren 
 * 
 * 
 * --- Architecture of the HTML table for the editor ---
 * --- sortable support managed in o.PRA_editor.js
 * <table class="table">
 * <thead></thead>
 * <tbody>
 * <tr>   -- first TR-row contains the CLONE
 * <td></td>..
 * <tr>
 * <tr>   -- then real steps follow
 *   <td><img src="images/ic.modpx9.gif"></td>  -- contains the drag/drop image, no data
 *   <td class="contenteditable" contenteditable="true"><input type=hidden value=1>Untitled</td>  
 *      -- first cell with data, hidden input field contains the STEP_NO of the protocol
 *   <td></td>..
 *   <td></td>..
 * <tr>
 * </tbody>
 */
class oABSTRACT_PROTO_xmode_edi {
    
    private $ori_steps;
    
private function showstyle() {
        ?>
 <script>
 function showXargsForm()  {  
	  id="xMoreForm";
      if (document.getElementById(id).style.display == 'none')   {  
           document.getElementById(id).style.display = '';  
      }  
 }  
 </script>
	<style>
.table-editable {
  position: relative;
}

.table {
  border-collapse: collapse; 
}

.table tr:hover { background-color: #ffffCC; }


.table-editable td, .table-editable th {
  padding-right: 12px;
}

.xAct  div {
    display: none;
}

.xAct:hover  div {
    display: block;
}

.table-editable tr {
  border-bottom: 1px solid #ccc;
}

.table-editable .glyphicon {
  font-size: 20px;
}

.table-remove {
  color: #700;
  cursor: pointer;
}
.table-remove:hover {
  color: #f00;
}

.table-up, .table-down {
  color: #007;
  cursor: pointer;
}
.table-up:hover, .table-down:hover {
  color: #00f;
}

.table-add {
  color: #070;
  cursor: pointer;
}
.table-add:hover {
  color: #0b0;
}
</style>
<?php
}


/**
 * init some vars
 * @param object $sqlo
 */
private function init(&$sqlo) {
    
    $this->row_cnt=NULL;
    
    $sqlsel = "H_UNIT_ID, NAME from H_UNIT order by NAME";
    $sqlo->Quesel($sqlsel);
    $this->H_UNIT_vals =array();
    while ( $sqlo->ReadRow() ) {
        $this->H_UNIT_vals[$sqlo->RowData[0]] = $sqlo->RowData[1];
    }
    
    $this->inp_DATAURL = oS_VARIO_sub::getValByTabKey($sqlo, 'ABSTRACT_PROTO', $this->id, 'inp.DATAURL');
    
    $keyarray = array('NAME', 'QUANTITY', 'ABSTRACT_SUBST_ID', 'H_UNIT_ID', 'ABS_DEV_ID', 'SUB_STEP_NR', 'NOTES', 'IS_SAMPLE', 'MANDATORY_VALUES'	);
    $this->aProtoLib = new  oABSTRACT_PROTO_stepx();
    $this->aProtoLib->setObj($this->id);
    $this->ori_steps = $this->aProtoLib->getStepArray_i2( $sqlo, $keyarray );
}

/**
 * 
 * @param $type
 *   'NORM', 'CLONE'
 * @param $stepnr  $one_step['STEP_NR'] 
 */
private function _sh_start_row($type, $stepnr) {
    
    $class=NULL;
    if ($type=='CLONE') {
        $class='class="hide" style="display: none;"';
    } else {
        $this->row_cnt++;
    }
    echo '<tr '.$class.'>'."\n";
    echo '<td><img src="images/ic.modpx9.gif"><input type=hidden value='.$stepnr.'></td>'."\n";
    
}

/**
 * show one column
 * @param object $sqlo
 * @param string $table
 * @param int $objid
 */
private function _sh_column(&$sqlo, $table, $dataval) {
    
    $configarr=array(
        'A_CHIP_READER' =>array('but_code'=>"fyqx"),
        'ABSTRACT_SUBST'=>array('but_code'=>"fyqy"),
    );
    
    switch ($table) {
        
        case 'MANDATORY_VALUES':
        case 'NOTES':
        case 'QUANTITY':
            echo '<td class="contenteditable" contenteditable="true">';
            echo htmlspecialchars($dataval);
            echo '</td>'."\n";
            break;
        case 'H_UNIT':
            echo '<td>'."\n";
            echo formc::selectFget( 'H_UNIT_ID', $this->H_UNIT_vals, $dataval);
            echo '</td>'."\n";
            break;
        case 'IS_SAMPLE':
            $checked='';
            if ($dataval) $checked='checked';
            echo '<td>'."\n";
            echo '<input type="checkbox" value="1" '.$checked.'>';
            echo '</td>'."\n";
            break;
        case 'A_CHIP_READER':
        case 'ABSTRACT_SUBST':
    
            $but_code = $configarr[$table]['but_code'] . $this->row_cnt;   // BUTTON-code must be unique inside the form
            $nicename = '---';
            if ($dataval) {
                $nicename = obj_nice_name ( $sqlo, $table, $dataval );
            }
            echo '<td class="xAct">'."\n";
            gJS_edit::getLinkSpecial1($table, $but_code, $dataval, $nicename);
            echo '</td>'."\n";

           break;
    }
}
    
private function _sh_col_rest() {

?>
    <td>
      <span class="table-remove glyphicon glyphicon-remove"><img src="images/ic.del.gif" title="Delete step"></span>
    </td>
<?php
  /* OLD :UP/DOWN buttons 
   <td>
      <span class="table-up glyphicon glyphicon-arrow-up"><img src="images/ic.arr_up.png"></span>
      <span class="table-down glyphicon glyphicon-arrow-down"><img src="images/ic.arr_down.png"></span>
    </td>
  */
}

private function view_only_proto($sqlo, $sqlo2) {
    
    
    if ( empty($this->ori_steps) ) {
        echo "<font color=gray>No steps defined</font><br>\n";
        
    } else {
        
        $showCheckBoxes=1;
        $options = array("showStepLnk"=>1);
        $editAllow=0;
        $proto_step_c = new  protostep( $sqlo, $sqlo2, $this->id, $editAllow, $showCheckBoxes, 0, $options  );
       
        echo '<form name="xtraform" ACTION="obj.abstract_proto.stepact.php?id='.$this->id.'" METHOD=POST>'."\n";
		
		$proto_step_c->tableOut($sqlo);
		
		$editAllow=0;
		if ($showCheckBoxes) {
		    $feld = NULL;
		    $feld["copy"]="copy";
		    if ( $editAllow  ){
		        $feld["del"]  ="delete";
		        $feld["paste"]="paste";
		    }
		    $preselected = "";
		    $seloption = array("selecttext"=>"-- action --");
		    $seltext   = formc::selectFget( "act", $feld, $preselected, $seloption);
		    
		    echo "&nbsp;&nbsp;".$seltext." ";
		    echo "<input type=submit value=\"Submit\">";
		}

		echo "</form>\n";
	
		
	}
	
}


// PLEASE use this function; still optional 
function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow, $editmode) {
	global $error;
	
	$this->showstyle();
	
	$this->id = $id;
	$this->init($sqlo);
	
	echo "<ul>\n";
	
	$allow_edit=0;
	if ($x_rights['write']>0) {
	    $allow_edit=1;
	} else $allow_edit=0;

	if ($editmode!='edit') {
	    $allow_edit=0;
	}
	
	//$helplib = new oABSTRACT_PROTO_guisub($id);
	//$helplib->object_tracking($sqlo);
	
	if (!$allow_edit) {
	    $this->view_only_proto($sqlo, $sqlo2);
	    return;
	}
	
	$inp_DATAURL_checked='';
	if($this->inp_DATAURL>0) $inp_DATAURL_checked='checked';

	
	?>
	<div class="container">
  <span class="yGgrayH1">Protocol Step Editor </span>
     <span class="table-add" style="padding-left:20px;"><img src="images/ic.plus.png"> ADD step</span>
     <span style="padding-left:20px;"><a href="obj.abstract_proto.stepact.php?id=<?php echo $id;?>&act=paste"><img src="images/but.paste.gif"> Paste steps</a></span>
     <?php 
     // FUTURE: <span style="padding-left:20px;"><a href="p.php?mod=DEF/o.ABSTRACT_PROTO.formula_p&id= echo $id;">Formula</a></span>
     ?>
     <span style="padding-left:20px;"><a href="obj.abstract_proto.stepact.php?id=<?php echo $id;?>&act=delall"><img src="images/but.delete.gif"> Delete all</a></span>
     <span style="padding-left:20px;"><a href="javascript:showXargsForm()"><img src="res/img/settings.svg"> ... More </a></span>
     <?php 
     if ($this->inp_DATAURL)  {
         echo ' &nbsp;<span style="color:gray">(Extra field active: <b>Raw-Data-URL</b>)<span>';
     }
     ?>
  <div id="xMoreForm" style="display:none;">
      <form name="xargsform" action="obj.abstract_proto.stepact.php">
      User must input: "Raw-Data-URL": 
        <input type=checkbox name="xargs[inp.DATAURL]" value="1" <?php echo $inp_DATAURL_checked;?>>
        <input type="hidden" name="xargs[inp.DATAURL.set]" value="1" > 
        <input type="submit" class="yButSmall" value="Save" > 
        <input type="hidden" name="id" value="<?php echo $id;?>" > 
        <input type="hidden" name="act" value="xargs" > 
      </form>
  </div>  
  <p />
  
  <div id="table" class="table-editable">
    <form name="editform">
    <table class="table">
      <thead>
      <tr> 
        <th></th>
        <th>Description</th>
		<th>Equipment</th>
		<th>Material planned</th>
		<th>Quantity<br>planned</th>
        <th>Unit</th>
		<th>Notes planned</th>
		<th title="Mandatory values:qns">Mand.</th>
		<th>Sample</th>
        <th></th>
        <th></th>
      </tr>
      </thead>
      <tbody>
    <!-- This is our clonable table line -->


	<?php 
    $this->_sh_start_row('CLONE', -1);
    ?>
        <td class="contenteditable" contenteditable="true">Untitled</td>
	<?php
	
    $this->_sh_column($sqlo, 'A_CHIP_READER', 0);
    $this->_sh_column($sqlo, 'ABSTRACT_SUBST', 0);
    $this->_sh_column($sqlo, 'QUANTITY', '');
    $this->_sh_column($sqlo, 'H_UNIT', 0);
    $this->_sh_column($sqlo, 'NOTES', '');
    $this->_sh_column($sqlo, 'MANDATORY_VALUES', '');
    $this->_sh_column($sqlo, 'IS_SAMPLE', '0');
    $this->_sh_col_rest();
    echo '</tr> <!-- END clone --> '."\n";

    if (!empty($this->ori_steps)) {
        
        $cnt=0;
        foreach($this->ori_steps as $one_step ) {
            
            $NAME_out = htmlspecialchars($one_step['NAME'], ENT_QUOTES, 'UTF-8');
            if ($one_step['SUB_STEP_NR']==1) {
                $NAME_out = '&nbsp;&nbsp;&nbsp;'.$NAME_out;
            }
            if ($one_step['SUB_STEP_NR']==2) {
                $NAME_out = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'.$NAME_out;
            }
            
            
            $this->_sh_start_row('NORM', $one_step['STEP_NR'] );
            echo 
                '   <td class="contenteditable" contenteditable="true">'.$NAME_out.'</td>'."\n";
        
           
            $this->_sh_column($sqlo, 'A_CHIP_READER' , $one_step['ABS_DEV_ID'] );
            $this->_sh_column($sqlo, 'ABSTRACT_SUBST', $one_step['ABSTRACT_SUBST_ID'] );
            $this->_sh_column($sqlo, 'QUANTITY', $one_step['QUANTITY']);
            $this->_sh_column($sqlo, 'H_UNIT', $one_step['H_UNIT_ID']);
            $this->_sh_column($sqlo, 'NOTES', $one_step['NOTES']);
            $this->_sh_column($sqlo, 'MANDATORY_VALUES', $one_step['MANDATORY_VALUES']);
            $this->_sh_column($sqlo, 'IS_SAMPLE', $one_step['IS_SAMPLE']);
            
            $this->_sh_col_rest();
            echo ' </tr>'."\n";
            $cnt++;
        }
    }
    ?>
      
      
    </tbody>
    </table>
    </form>
  </div>
  
  <button id="export-btn" class="actbutton">Submit</button> 
  <p id="export"></p>
  <p id="debugx"></p>
  
  
</div>
	<?php
	
	echo '<script src="res/js/jquery-ui.1-12.1.js"></script>'."\n"; // jquery-ui for support of TABLE > .sortable
	echo '<script type="text/javascript" src="res/js_goz/o.PRA_editor.js?dummy=2"></script>'."\n";
	echo '<script type="text/javascript" >'."\n";
	echo ' x_set_proto_id('.$this->id.');'."\n";
	echo '</script>'."\n";
	echo "</ul>\n";
}

}