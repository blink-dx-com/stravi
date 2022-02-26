<?
/**
 * functions for bracode EAN-13
 * - first version: 0.1 - 20040331 	
 * @package barcode.ean-13.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/barcode.ean-13.inc 59 2018-11-21 09:04:09Z $
 */
class barcodeEan13 {    

    var $im; // image object
	var $IMG_X_LEN;
	var $IMG_Y_LEN;

function calcCodeCheckBit( $code ) {
    // Calculate NumberCode + "CheckBit"
    
    $extsum = $this->calcCheck( $code );
    
    $check = (floor($extsum/10)*10+10) - $extsum;

    if ($check == 10)
    {
        $check = 0;
    }
    $codeAndCheck = $code . $check; 
    
    return ($codeAndCheck);
}

function getImgDim() {
	return (array($this->IMG_X_LEN, $this->IMG_Y_LEN));
}


function calcCheck( $code ) {     
    // Calculate Checksum
    $oddeven = 1;

    for ($i = 1; $i <= 12; $i++)
    {
        $num = substr($code, $i-1, 1);
        if ($oddeven == 1)
        {
            $intsum = $num * $oddeven;
            $extsum = $extsum + $intsum;
            $oddeven = 3;
        }
        else
        {
            $intsum = $num * $oddeven;
            $extsum = $extsum + $intsum;
            $oddeven = 1;
        }
    }
    
    return ($extsum);

}


function buildImage( 
	$code, 
	$option=NULL // "IMG_Y_LEN"
	) {
    // Check validity of $code
    // RETURN: $error_flag: <0 => error
    //         $error_text
    
    if (strlen($code) != 12)
    {
        return array(-1,"Code $code is not valid; need 12 digits.");
    }

    $IMG_FONT       = 2;  // font size [5]

    $IMG_Y_LEN      = 80; // [100]
	if ($option["IMG_Y_LEN"]>0) $IMG_Y_LEN = $option["IMG_Y_LEN"];
	
    $IMG_NUMDIFF_Y  = 20; // offset of numbers from bottom [20]
    $STRIP_NUM      = 95; // number of strips => FIX, do not change 
    $STRIPS_START_X = 4;  // X-start of first strip [14]
    $STRIP_DIFF_Y   = 2;  // original:[3]
    $IMG_BOTTOM_CUT = 8;  // bottom WHITE cut
    $IMG_XPOS_AFTER_CODE =  $STRIPS_START_X + $STRIP_NUM * $STRIP_DIFF_Y + 1;  // X-pos after BARCODE
    $IMG_X_LEN      = $IMG_XPOS_AFTER_CODE + 14;     // 304;
    $IMG_NUMSTART_Y = $IMG_Y_LEN - $IMG_NUMDIFF_Y;   // start of numbers

	$this->IMG_X_LEN = $IMG_X_LEN;
	$this->IMG_Y_LEN = $IMG_Y_LEN;

    for ($i = 1; $i <= 12; $i++)
    {
        if ((substr($code, $i-1, 1) <= 0) && ((substr($code, $i-1, 1) >= 9)))
        {
            return array(-2,"Code $code is not valid; only digits!");		
        } 
    }


    // Define bitcode for Numbers
    $left[0][O] = "0001101";
    $left[0][E] = "0100111";
    $left[1][O] = "0011001";
    $left[1][E] = "0110011";
    $left[2][O] = "0010011";
    $left[2][E] = "0011011";
    $left[3][O] = "0111101";
    $left[3][E] = "0100001";
    $left[4][O] = "0100011";
    $left[4][E] = "0011101";
    $left[5][O] = "0110001";
    $left[5][E] = "0111001";
    $left[6][O] = "0101111";
    $left[6][E] = "0000101";
    $left[7][O] = "0111011";
    $left[7][E] = "0010001";
    $left[8][O] = "0110111";
    $left[8][E] = "0001001";
    $left[9][O] = "0001011";
    $left[9][E] = "0010111";
    $right[0] = "1110010";
    $right[1] = "1100110";
    $right[2] = "1101100";
    $right[3] = "1000010";
    $right[4] = "1011100";
    $right[5] = "1001110";
    $right[6] = "1010000";
    $right[7] = "1000100";
    $right[8] = "1001000";
    $right[9] = "1110100";


    // Calculate Checksum  
    $code = $this->calcCodeCheckBit( $code );


    // Build Array from $code string

    for ($i = 1; $i <= 13; $i++)
    {
        $c[$i] = substr($code, $i-1, 1);
    }

    // Set parity

    if ($c[1] == 0)
    {
        $parity = "OOOOO";
    }
    else if ($c[1] == 1)
    {
        $parity = "OEOEE";
    }
    else if ($c[1] == 2)
    {
        $parity = "OEEOE";
    }
    else if ($c[1] == 3)
    {
        $parity = "OEEEO";
    }
    else if ($c[1] == 4)
    {
        $parity = "EOOEE";
    }
    else if ($c[1] == 5)
    {
        $parity = "EEOOEE";
    }
    else if ($c[1] == 6)
    {
        $parity = "EEEOO";
    }
    else if ($c[1] == 7)
    {
        $parity = "EOEOE";
    }
    else if ($c[1] == 8)
    {
        $parity = "EOEEO";
    }
    else if ($c[1] == 9)
    {
        $parity = "EEOEO";
    }

    // Start generating bitcode for barcode
    $barbit = "101"; // Startguard

    $barbit = $barbit . $left[$c[2]][O]; // 2nd char is always odd


    for ($i = 3; $i <= 7; $i++) // generate first 5 digits with parity in bitcode
    {
        $par = substr($parity, $i - 3, 1);
        $barbit = $barbit . $left[$c[$i]][$par];
    }

    $barbit = $barbit . "01010"; // Middleguard

    for ($i = 8; $i <= 13; $i++) // generate bitcode for 5 digits and 1 checksum
    {
        $barbit = $barbit . $right[$c[$i]];
    }

    $barbit = $barbit . "101"; // Endguard

    // Create Image
    if ( !($im = @ImageCreate ($IMG_X_LEN, $IMG_Y_LEN) ) )
        return (array(-3,"Cannot Initialize new GD image stream")); 
        
        
    $bg = ImageColorAllocate ($im, 255, 255, 255);
    $fg = ImageColorAllocate ($im, 0, 0, 0); 

    /*
    $red   = ImageColorAllocate ($im, 255, 0, 0);   
    $green = ImageColorAllocate ($im, 0, 255, 0);
    $blue  = ImageColorAllocate ($im, 0, 0, 255);
    */

    $start       = $STRIPS_START_X;
    $STRIP_MAX_Y = $IMG_Y_LEN;

    for ($i = 1; $i <= $STRIP_NUM; $i++)
    {
        $end = $start + $STRIP_DIFF_Y;
        $bit = substr($barbit, $i-1, 1);
        if ($bit == 0) {
            Imagefilledrectangle ($im, $start, 0, $end, $STRIP_MAX_Y, $bg);
        }
        else {
            Imagefilledrectangle ($im, $start, 0, $end, $STRIP_MAX_Y, $fg);
        }
        $start = $end;
    }

    //$start = $start-$STRIP_DIFF_Y;
    //Imagefilledrectangle ($im, $start, 0, $end, $STRIP_MAX_Y, $red);        


    //  remove last RIGHT line (coming from DRAW algorithm
    $strip[0] = $STRIPS_START_X + $STRIP_NUM * $STRIP_DIFF_Y; // last STRIPX_end-1
    $strip[1] = 0;
    $strip[2] = $IMG_X_LEN;   
    $strip[3] = $IMG_Y_LEN;
    Imagefilledrectangle ($im, $strip[0], $strip[1], $strip[2], $strip[3], $bg);   

    // FIRST NUMBER space
    $strip[0] = $STRIPS_START_X + 3 * $STRIP_DIFF_Y;
    $strip[1] = $IMG_NUMSTART_Y;
    $strip[2] = $strip[0] + 42 * $STRIP_DIFF_Y -1;   // 148
    $strip[3] = $IMG_Y_LEN;
    Imagefilledrectangle ($im, $strip[0], $strip[1], $strip[2], $strip[3], $bg); 
    $save_strip2 = $strip[2];
    $IMG_NUM1_START = $strip[0] + 2*$STRIP_DIFF_Y; 


    // SECOND NUMBER space
    $strip[0] = $save_strip2 + 5 * $STRIP_DIFF_Y +1; // 164 ???
    $strip[1] = $IMG_NUMSTART_Y;
    $strip[2] = $strip[0] + 42 * $STRIP_DIFF_Y -1;
    $strip[3] = $IMG_Y_LEN;
    Imagefilledrectangle ($im, $strip[0], $strip[1], $strip[2], $strip[3], $bg); 
    $IMG_NUM2_START = $strip[0] + 2*$STRIP_DIFF_Y;

    // full space VERTICAL 
    $strip[0] = 0;
    $strip[1] = $IMG_Y_LEN - $IMG_BOTTOM_CUT;
    $strip[2] = $IMG_X_LEN;
    $strip[3] = $IMG_Y_LEN;
    Imagefilledrectangle ($im, $strip[0], $strip[1], $strip[2], $strip[3], $bg);              


    ImageString ($im, $IMG_FONT, $IMG_NUM1_START, $IMG_NUMSTART_Y+3, "$c[1] $c[2] $c[3] $c[4] $c[5] $c[6]", $fg);
    ImageString ($im, $IMG_FONT, $IMG_NUM2_START, $IMG_NUMSTART_Y+3, "$c[7] $c[8] $c[9] $c[10] $c[11] $c[12]", $fg);
    ImageString ($im, $IMG_FONT, $IMG_XPOS_AFTER_CODE+2, $IMG_NUMSTART_Y-10, $c[13], $fg);    // CHECK-DIGIT
    $this->im = $im;
    
    return array(0,"");   
}

function htmlOut() {
    // Sending Headers
    header ("Content-type: image/png");
    header ("Expires: Mon, 26 Jul 1997 05:00:00 GMT");    // Date in the past
    header ("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT"); // always modified
    header ("Cache-Control: no-cache, must-revalidate");  // HTTP/1.1
    header ("Pragma: no-cache");                          // HTTP/1.0   
    
    ImagePng ($this->im);
} 

function saveImagePng( $filename ) { 
    ImagePng ( $this->im, $filename );
}

function saveImageJpg( $filename ) {
    $jpgQuality = 75; 
    ImageJpeg ( $this->im, $filename, $jpgQuality );
} 
  

}
