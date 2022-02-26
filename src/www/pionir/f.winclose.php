<?php 
// INPUT: tablename 
session_start(); 

?>
<html> 
<body>
<?
$tablename=$_REQUEST['tablename'];
if ( is_array($_SESSION['s_formback'][$tablename])) {
    unset($_SESSION['s_formback'][$tablename]);  // clear entry
}      
?>
<script language="JavaScript">
<!--
  	window.close();
//-->
</script>
</body>    
</html> 