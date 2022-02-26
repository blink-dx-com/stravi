<?php
/**
 * layer for home.php
 * @namespace core::gui::homeLayer
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class themeCls extends gThemeClsAbs {
	
function layer_show( &$sql, &$sql2 ) {
  

  $showAdvanced=0;
  if ($_SESSION['s_suflag'] OR $_SESSION['sec']['appuser']=="root") $showAdvanced=1; 
  ?>
  <center>
  <form>
  <br>
  <table width="550" border="0">
    <tr>
      <td align="left"><?
	  $this->__homeFuncObj->home_showHead("exp");
	  ?></td>
      <td align="right"><font size="-1" color="#000000">
	  <? 
	  $this->__homeFuncObj->selMainObjects( $showAdvanced ); 
	  ?>
	  
      </font>
  </td></tr></table>
  </form>
  <?
  //$proj_id =  oDB_USER_sub2::userHomeProjGet($sql); 

 
  ?>
  <IMG SRC = "images/layerimg.EXP.png" BORDER="0" USEMAP ="#imagemap1">
<MAP NAME ="imagemap1">
    <area target="_self" alt="protocol" title="protocol" href="javascript:linkto('CONCRETE_PROTO')" coords="251,126,112,60" shape="rect">
    <area target="_self" alt="protocol template" title="protocol template" href="javascript:linkto('ABSTRACT_PROTO')" coords="148,131,253,167" shape="rect">
    <area target="_self" alt="material" title="material" href="javascript:linkto('CONCRETE_SUBST')" coords="127,217,251,273" shape="rect">
    <area target="_self" alt="material template" title="material template" href="javascript:linkto('ABSTRACT_SUBST')" coords="155,281,257,315" shape="rect">
    <area target="_self" alt="equipment" title="equipment" href="javascript:linkto('CHIP_READER')" coords="124,335,252,391" shape="rect">
    <area target="_self" alt="equipment template" title="equipment template" href="javascript:linkto('ABS_CHIP_READER')" coords="150,398,253,430" shape="rect">
    <area target="_self" alt="experiment template" title="experiment template" href="javascript:linkto('EXP_TMPL')" coords="330,129,455,168" shape="rect">
    <area target="_self" alt="experiment" title="experiment" href="javascript:linkto('EXP')" coords="325,172,488,246" shape="rect">
    <area target="_self" alt="equipment (for test)" title="equipment (for test)" href="javascript:linkto('CHIP_READER')" coords="560,91,692,147" shape="rect">
    <area target="_self" alt="cartridge" title="cartridge" href="javascript:linkto('SOCKET')" coords="563,261,694,318" shape="rect">
    <area target="_self" alt="cartridge template" title="cartridge template" href="javascript:linkto('ABS_SOCKET')" coords="588,330,696,363" shape="rect">
</MAP>
  </center>
<?
}

}
