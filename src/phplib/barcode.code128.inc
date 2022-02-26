<?php
require_once ("barcode.Abstract.inc");

/**
 * functions to generate an image for barcode Code128
 * - suport for subtype: B,C
 * @package barcode.code128.inc
 * @swreq   UREQ:0001137: g > support Code128 barcode image gen 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @see Code128 defs: http://grandzebu.net/index.php?page=/informatique/codbar/code128.htm
 */
class barcodeCode128 extends barcodeAbstractC {    

	var $BAR_CODE128_quatMaxNum = 6;
    var $BAR_CODE128_quatNumBitLast =11;
    var $CODE128_Astartdigit= 103;
    var $CODE128_Bstartdigit= 104;
    var $CODE128_startdigit = 105;
	var $CODE128_stopdigit  = 106;
	
	
	
function __construct( $params ) {

	parent::__construct( $params );
	
	$this->fontTrans = array(
		8 => 1,
		10=> 2,
		12=> 3,
		14=> 4,
		16=> 5
	);

	$this->C128TableQubin = array(
    0xd98,
    0xcd8,
    0xccc,
    0x930,
    0x918,
    0x898,
    0x990,
    0x988,
    0x8c8,
    0xc90,
    0xc88,
    0xc48,
    0xb38,
    0x9b8,
    0x99c,
    0xb98,
    0x9d8,
    0x9cc,
    0xce4,
    0xcb8,
    0xc9c,
    0xdc8,
    0xce8,
    0xedc,
    0xe98,
    0xe58,
    0xe4c,
    0xec8,
    0xe68,
    0xe64,
    0xdb0,
    0xd8c,
    0xc6c,
    0xa30,
    0x8b0,
    0x88c,
    0xb10,
    0x8d0,
    0x8c4,
    0xd10,
    0xc50,
    0xc44,
    0xb70,
    0xb1c,
    0x8dc,
    0xbb0,
    0xb8c,
    0x8ec,
    0xeec,
    0xd1c,
    0xc5c,
    0xdd0,
    0xdc4,
    0xddc,
    0xeb0,
    0xe8c,
    0xe2c,
    0xed0,
    0xec4,
    0xe34,
    0xef4,
    0xc84,
    0xf14,
    0xa60,
    0xa18,
    0x960,
    0x90c,
    0x858,
    0x84c,
    0xb20,
    0xb08,
    0x9a0,
    0x984,
    0x868,
    0x864,
    0xc24,
    0xca0,
    0xf74,
    0xc28,
    0x8f4,
    0xa78,
    0x978,
    0x93c,
    0xbc8,
    0x9e8,
    0x9e4,
    0xf48,
    0xf28,
    0xf24,
    0xdbc,
    0xdec,
    0xf6c,
    0xaf0,
    0xa3c,
    0x8bc,
    0xbd0,
    0xbc4,
    0xf50,
    0xf44,
    0xbbc,
    0xbdc,
    0xebc,
    0xf5c,
    0xd08,
    0xd20,
    0xd38,
    0xc74
    );
    
	$b_chr=array();
    $b_chr['00'] = " ";
	$b_chr['01'] = "!";
	$b_chr['02'] = '"';
	$b_chr['03'] = "#";
	$b_chr['04'] = "$";
	$b_chr['05'] = "%";
	$b_chr['06'] = "&";
	$b_chr['07'] = "'";
	$b_chr['08'] = "(";
	$b_chr['09'] = ")";
	$b_chr['10'] = "*";
	$b_chr['11'] = "+";
	$b_chr['12'] = ",";
	$b_chr['13'] = "-";
	$b_chr['14'] = ".";
	$b_chr['15'] = "/";
	$b_chr['16'] = "0";
	$b_chr['17'] = "1";
	$b_chr['18'] = "2";
	$b_chr['19'] = "3";
	$b_chr['20'] = "4";
	$b_chr['21'] = "5";
	$b_chr['22'] = "6";
	$b_chr['23'] = "7";
	$b_chr['24'] = "8";
	$b_chr['25'] = "9";
	$b_chr['26'] = ":";
	$b_chr['27'] = ";";
	$b_chr['28'] = "<";
	$b_chr['29'] = "=";
	$b_chr['30'] = ">";
	$b_chr['31'] = "?";
	$b_chr['32'] = "@";
	$b_chr['33'] = "A";
	$b_chr['34'] = "B";
	$b_chr['35'] = "C";
	$b_chr['36'] = "D";
	$b_chr['37'] = "E";
	$b_chr['38'] = "F";
	$b_chr['39'] = "G";
	$b_chr['40'] = "H";
	$b_chr['41'] = "I";
	$b_chr['42'] = "J";
	$b_chr['43'] = "K";
	$b_chr['44'] = "L";
	$b_chr['45'] = "M";
	$b_chr['46'] = "N";
	$b_chr['47'] = "O";
	$b_chr['48'] = "P";
	$b_chr['49'] = "Q";
	$b_chr['50'] = "R";
	$b_chr['51'] = "S";
	$b_chr['52'] = "T";
	$b_chr['53'] = "U";
	$b_chr['54'] = "V";
	$b_chr['55'] = "W";
	$b_chr['56'] = "X";
	$b_chr['57'] = "Y";
	$b_chr['58'] = "Z";
	$b_chr['59'] = "[";
	$b_chr['60'] = "\\";
	$b_chr['61'] = "]";
	$b_chr['62'] = "^";
	$b_chr['63'] = "_";
	$b_chr['64'] = "`";
	$b_chr['65'] = "a";
	$b_chr['66'] = "b";
	$b_chr['67'] = "c";
	$b_chr['68'] = "d";
	$b_chr['69'] = "e";
	$b_chr['70'] = "f";
	$b_chr['71'] = "g";
	$b_chr['72'] = "h";
	$b_chr['73'] = "i";
	$b_chr['74'] = "j";
	$b_chr['75'] = "k";
	$b_chr['76'] = "l";
	$b_chr['77'] = "m";
	$b_chr['78'] = "n";
	$b_chr['79'] = "o";
	$b_chr['80'] = "p";
	$b_chr['81'] = "q";
	$b_chr['82'] = "r";
	$b_chr['83'] = "s";
	$b_chr['84'] = "t";
	$b_chr['85'] = "u";
	$b_chr['86'] = "v";
	$b_chr['87'] = "w";
	$b_chr['88'] = "x";
	$b_chr['89'] = "y";
	$b_chr['90'] = "z";
	$b_chr['91'] = "{";
	$b_chr['92'] = "|";
	$b_chr['93'] = "}";
	$b_chr['94'] = "~";
	$b_chr['95'] = "DEL";
	$b_chr['96'] = "FNC3";
	$b_chr['97'] = "FNC2";
	$b_chr['98'] = "SHIFT";
	$b_chr['99'] = "Code C";
	$b_chr['100'] = "FNC4";
	$b_chr['101'] = "Code A";
	$b_chr['102'] = "FNC1";
	$b_chr['103'] = "START A";
	$b_chr['104'] = "START B";
	$b_chr['105'] = "START C";
	
	$this->b_chr = $b_chr;
}

function _saveOneStrip($stripCnt) {
	$this->stripBuf[] = $stripCnt;
}

function _drAtomStrip($bw) {
	$start = $this->drawXpos;
	$end   = $start + $this->STRIP_XONE-1;
	
	if ( $bw>0 ) {
		Imagefilledrectangle ( $this->im, $start, $this->STRIP_BOT_Y, 
			$end,  $this->STRIP_TOP_Y, $this->colorx["fg"] );
	}
	$this->drawXpos = $this->drawXpos + $this->STRIP_XONE;
	
	/*
	echo "DEXV: ".$this->atomStrCnt.":$bw<br>";
	$atomStrCnt1 = $this->atomStrCnt+1;
	if ( ($atomStrCnt1 / 11) == intval($atomStrCnt1 / 11) ) {
		echo " --- <br>";
	}
	*/
}

function encode128One( $twochar ) {
    # onechar: decimal number 0..99 */
	
	$code128bin  = $this->C128TableQubin[$twochar];
	$i=0;
	$bitcnt = 0;
	$bw = 1;
	$last_bw = $bw;
	$stripCnt= 0;
	
	# produce bit-pattern */
	
	while ($bitcnt < $this->BAR_CODE128_quatNumBitLast ) {
	
		$bw = ( $code128bin >> $this->BAR_CODE128_quatNumBitLast) & 1;
		
		$this->_drAtomStrip($bw);
		
		$this->atomStrCnt++;
		if ( $bw !=$last_bw) {
			$this->_saveOneStrip($stripCnt);
			$stripCnt = 0;
		}
	
		$stripCnt++;
		$last_bw = $bw;
		$code128bin = $code128bin << 1;
		$bitcnt++;
	}
	
	$this->_saveOneStrip($stripCnt);
	
	if ($twochar==$this->CODE128_stopdigit) {
		$this->_drAtomStrip(1);
		$this->_drAtomStrip(1);
	}
	
	return ( $code128bin );
}

/**
 *  check code128 parity
 * @param  $code128BufPoi
 * @param  $digitsCodedNum
 * @return $parityCalc
 */
function  _calcparity( $code128BufPoi, $digitsCodedNum ) {
        
        /*
		
        */
        
       $oneInt=0;
       $i=0;
       $sum=0;
       $parityCalc=0;

       $sum = 0;
        # for start + DataDigits
       $i=0;
        # TBD: correct the parity algorithm in C !!!
       $sum =$sum +$code128BufPoi[$i]; # first digit
       $i=$i+1;
       while ( $i <$digitsCodedNum + 1) {
           $oneInt =$code128BufPoi[$i] *$i;
           $sum =$sum +$oneInt;;
           $i++;
		}

        $parityCalc =$sum % 103;

        return ( $parityCalc);
}

function _startDraw() {
	$this->drawXpos = $this->STRIPS_START_X;
	$this->atomStrCnt=0;
	$this->stripBuf = NULL;
}

/**
 * create Subtype C (decimal)
 * @param string $inbuf string of numbers '848858'
 */
function  encode128_decimal( $inbuf ) {
       $onechar= '';
       $twochar= '';
       $code128Word = 0;
       $digitsCodedNum = strlen( $inbuf ) / 2;
       $code128Buf = NULL;
	   
	   $this->_startDraw();

       $i=0;
       $twochar       = $this->CODE128_startdigit;
       $code128Word   = $this->encode128One( $twochar );
       $code128Buf[0] = $twochar;

       $i=0;
       $inbufcnt=0;
       while ( $i < $digitsCodedNum ) {
           $onechar = $inbuf[$inbufcnt]; 
           $twochar = ( intval( $onechar ) * 10 );
           $inbufcnt++;
           $onechar = $inbuf[$inbufcnt];
           $inbufcnt++;
           $twochar = $twochar +$onechar;
           $code128Word = $this->encode128One( $twochar );
           $code128Buf[$i+1] = $twochar;
           $i++;
		}

        # calc parity ... */
        
       $twochar = $this->_calcparity( $code128Buf,$digitsCodedNum );
       $code128Word = $this->encode128One($twochar);
       $code128Word = $this->encode128One( $this->CODE128_stopdigit );
	   
	   $this->code128Buf = $code128Buf;
}

/**
 * create Subtype B
 * @param string $code the characters of subtype B 'ABCD(3)-47'
 */
function encode128_B($inbuf) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$onechar= '';
    $twochar= '';
    $code128Word = 0;
    $digitsCodedNum = strlen( $inbuf );
    $code128Buf = NULL;
   
    $this->_startDraw();

    $i=0;
    $twochar    = $this->CODE128_Bstartdigit;
    $code128Word   = $this->encode128One( $twochar );
    $code128Buf[0] = $twochar;

    $i=0;
    $inbufcnt=0;
    while ( $i < $digitsCodedNum ) {
    	
        $onechar = $inbuf[$inbufcnt]; 
        $codeNum = array_search( $onechar, $this->b_chr,true );
        if ($codeNum===FALSE) {
        	$error->set( $FUNCNAME, 1, 'Char ORD('.ord($onechar).') is not valid for subtype B' );
			return;
        }
        $codeNum = intval($codeNum);
        
        $inbufcnt++;
        $code128Word = $this->encode128One( $codeNum );
        $code128Buf[$i+1] = $codeNum;
        $i++;
        
	}

    # calc parity ... */
    $twochar = $this->_calcparity( $code128Buf, $digitsCodedNum );
    $code128Word = $this->encode128One($twochar);
    $code128Word = $this->encode128One( $this->CODE128_stopdigit );
   
   $this->code128Buf = $code128Buf;
}

/**
 * build the image
 * @param $code 	string of ASCII-numbers or Code128 chars
 * @param $option=NULL  
 * 	"pretxt" : text before code
 * 	"subtype"   : A,B,[C] Code128 subtype, C is default
 * 
 * @global $this->params["charExpect"] : e.g. 6 (INPUT)
 */
function buildImage( 
	$code, 		 
	$option=NULL 
	) {
	global $error;
	$FUNCNAME= "buildImage";
	
	$this->subtype='C';
	if ($option['subtype']!=NULL) $this->subtype = $option['subtype'];
	
	$fu_charExpect = $this->params["charExpect"];
	
	if ($this->subtype=='B') {
		if ( (strlen($code)*2) != $fu_charExpect ) {
	        $error->set( $FUNCNAME, 1, "Code is not valid; need ".($fu_charExpect/2)." chars." );
			return;
	    }
	}
	if ($this->subtype=='C') {
	    if ($fu_charExpect and strlen($code) != $fu_charExpect ) {
	        $error->set( $FUNCNAME, 1, "Code is not valid; need ".$fu_charExpect." digits." );
			return;
	    }
	}
	
    $this->IMG_FONT       = 3;  // font size [5]
    $IMG_Y_LEN      = 80; // [100]
	
	if ($this->params["IMG_Y_LEN"]>0) $IMG_Y_LEN = $this->params["IMG_Y_LEN"];
	if ($this->params["txt_font"]>0 ) $this->IMG_FONT  = $this->fontTrans[$this->params["txt_font"]];
	if (!$this->IMG_FONT) $this->IMG_FONT = 3;
	
	$numCodedigits = $fu_charExpect/2 + 3; // e.g. 6
	$this->STRIP_XONE = 1;  // original:[3]
    $IMG_NUMDIFF_Y  = 20; // offset of numbers from bottom [20]
	$STOPBITS 		= 2;  // for the stopbit bb
	$X_WHITE_SPACE  = 9*$this->STRIP_XONE;
    $STRIP_NUM      = $this->BAR_CODE128_quatNumBitLast * $numCodedigits + $STOPBITS; // number of strips => FIX, do not change 
    $this->STRIPS_START_X = 4 + $X_WHITE_SPACE;  // X-start of first strip [14]
    
    $IMG_BOTTOM_CUT = 20;  // bottom WHITE cut
	$IMG_TOP_CUT    = 3;  // bottom WHITE cut
	$X_STRIPS_POST  = 5;
	
    $IMG_XPOS_AFTER_CODE =  $this->STRIPS_START_X + $STRIP_NUM * $this->STRIP_XONE + 1;  // X-pos after BARCODE
    $IMG_X_LEN      = $IMG_XPOS_AFTER_CODE + $X_WHITE_SPACE;     // 304;
    $IMG_NUMSTART_Y = $IMG_Y_LEN - $IMG_NUMDIFF_Y;   // start of numbers
	$IMG_NUMX_START = $this->STRIPS_START_X;

	$this->IMG_X_LEN = $IMG_X_LEN;
	$this->IMG_Y_LEN = $IMG_Y_LEN;
	$this->STRIP_TOP_Y = $this->IMG_Y_LEN-$IMG_BOTTOM_CUT;
	$this->STRIP_BOT_Y = $IMG_TOP_CUT;
	
	for ($i = 1; $i <= $fu_charExpect; $i++)
    {
		$onechar = substr($code, $i-1, 1);
        if ( ($onechar<0) OR ($onechar > 9) ) {
            $error->set( $FUNCNAME, 1, "Digit $i is not valid; must be 0<=digit<=9." );
			return;	
        } 
    }

	// Create Image
    if ( !($this->im = @ImageCreate ($IMG_X_LEN, $IMG_Y_LEN) ) ) {
        $error->set( $FUNCNAME, 3, "Cannot Initialize new GD image stream" ); 
		return;
	}
	
    $this->colorx["bg"] = ImageColorAllocate ($this->im, 255, 255, 255);
    $this->colorx["fg"] = ImageColorAllocate ($this->im, 0, 0, 0); 

    switch ($this->subtype) {
    	case 'B':
			$this->encode128_B($code);
			break;
    	case 'C':
			$this->encode128_decimal($code);
			break;
    }
	// glob_printr( $this->stripBuf, "stripBuf");


	// code as string
	$outstr = $code;
	if ($this->params["txt_noZero"]>0 and $this->subtype=='C') {
		$outstr = ltrim($code,"0");
	}
	if ( $option["pretxt"] != NULL) {
		$outstr = $option["pretxt"] . $outstr;
	}
	$fontUse = $this->IMG_FONT;
	ImageString ($this->im, $fontUse, $IMG_NUMX_START, $IMG_NUMSTART_Y, $outstr, $this->colorx["fg"] );
    
}

}
