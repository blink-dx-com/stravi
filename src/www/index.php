<?php
/**
 * forwarding ALL parameters to default login page
 * @package index.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   mixed input
 */
?>
<html>
<head>
  <meta http-equiv="Content-Type" content="text/html;">
</head>
<body>

<?php
$url = "pionir/index.php";
echo "... forward page\n";

if ($_REQUEST!=NULL) {
	$makeform=1;
} else {
	?>
	<script>
		location.href="<?php echo $url;?>";
	</script>
	<?php
}

if ($makeform) {

    echo "<form method=\"post\"  name=\"editform\"  action=\"".$url."\" >\n";
	
	if (sizeof($_REQUEST)) {
	    foreach( $_REQUEST as $key=>$val) {
			echo '<input type=hidden name="'.$key.'" value="'.$val.'">'."\n";
		}
	} 

	echo "</form>";
	?>
	<script>
		document.editform.submit();
	</script>
	<?php
	
}

?>
</body>
</html>
