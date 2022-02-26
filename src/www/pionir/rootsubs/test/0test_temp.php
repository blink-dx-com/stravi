<?php 
function _str_getInfo($val) {
    $nameFLen= strlen($val);
    $info=NULL;
    
    $info .= htmlentities( htmlentities($val,NULL, 'UTF-8') )."<hr>\n";
    
    $pos=0;
    while ($pos<$nameFLen) {
        
        $charFile = substr($val,$pos,1);
        $ordnum   = ord($charFile);
        $xinf = $ordnum >127 ? ' <span style="color:red;">>&gt;x80</span>' : NULL;
        $charcodeStr = sprintf("%x", ord($charFile) );
        $info .='pos:'.str_pad( ($pos+1), 2, " ").' char:'.$charFile.' ord:'.
            str_pad($ordnum,3," ") .' hex:'.str_pad($charcodeStr,2," ").$xinf."\n";
            
            $pos++;
            
    }
    return $info;
}

?>
<html>
<head>
<title> basic TEST</title>
</head>
<body>
<h2>BASIC test</h2>

<?php 
$a="hallo\n du Honk.\naha\n";
?>
<pre style="white-space: pre-wrap;">
<?php echo $a;?>
</pre>
<?php 
echo "ANALYSIS<br><pre>";
echo _str_getInfo($a);
echo "</pre>";
?>

</body>
</html>