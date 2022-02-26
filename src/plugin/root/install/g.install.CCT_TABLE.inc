<?php
require_once 'insert.inc';

/**
 * temporary script: init CCT_TABLE: vario columns 2018-12-16
 * @package g.install.CCT_TABLE.inc
 * @author  Steffen Kube (steffen.kube@gmx.de)
 */
class  g_install_CCT_TABLE_XPL extends gPlugin {


	function register() {
		
		$title		  = 'Update CCT_TABLE vario columns';

		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool';
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/rootFuncs.php', 'home') );

		$this->infoarr2 = array(
				'onlyAdmin'=>1
		);

	}

	/**
	 * - creates a form with one text field "Min-value"
	 * - creates a HTML-page parameter $_REQUEST['parx']['minval'] after SUBMIT
	 */
	function form1() {

		$initarr   = NULL;
		$initarr["title"]       = "Update!"; // title of form
		$initarr["submittitle"] = "Submit"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

	
		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function saveVarioLoop( &$sqlo, $data, $key ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		foreach($data as $tablename => $value) {
			
			
			if (!gElement_exists($sqlo, 'CCT_TABLE', array($tablename) )) {
				continue;
			}
			
			$argu=array(
					'TABLE_NAME' => $tablename,
					'KEY'  => $key,
					'VALUE'=> $value
			);
			echo "insert $tablename:$key<br>";
			// insert_row_s( $sqlo, 'CCT_TABLE_DYN', $argu );
			
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'error on insert on table '.$tablename );
				return;
			}
				
		}
		
		echo 'Ready key '.$key."<br>";
	}

	function startMain() {
		global $error;

		$FUNCNAME='startMain';
		
		$go   = $_REQUEST['go'];
		$sqlo = &$this->_sqlo;
		
		if (!$go) {
			$this->form1();
			return;
		}
		
		echo "Update!<br>";
		
		$colorArray = array(
				"ABSTRACT_ARRAY" => "#6699FF", // rgb(102,153,255) -
				"ABSTRACT_PROTO" => "#9BE19B", // rgb(155,225,155) -
				"ABSTRACT_SUBST" => "#FF9A69", // rgb(255,154,105) -
				"A_CHIP_READER"  => "#606060", // rgb(96,96,96) -
				'CABNAT'		 => '#AD00C6', // rgb(173,0,198) -
				'CABNAT_TMPL'    => '#BD30D6', // rgb(189,48,214) -
				"CART_BATCH"	 => "#AD00C6", // rgb(173,0,198) -
				"CARTB_HAS_SOCK" => "#DF81ED", // rgb(223,129,237) -
				"CART_BATCH_STEP"=> "#DF81ED", // rgb(223,129,237) -
				'CHIP_READER' 	 => '#404040', // rgb(64,64,64) -
				"CONTAINER"		 => "#81DFED", // rgb(129,223,237) -
				"CONT_HAS_CSUBST"=> "#C1F0F7", // rgb(193,240,247) -
				'CYTCART'		 => '#FF3399', // rgb(255,51,153) -
				"DB_USER"		 => "#FBFD29", // rgb(251,253,41) -
				"EXP" 			 => "#A57B42", // rgb(165,123,66) -
				"EXP_TMPL" 		 => "#DDBB88", // rgb(221,187,136) -
				"H_EXP_RAW_DESC" => "#DDBB88", // rgb(221,187,136) -
				"H_UNIT"		 => "#EFFE74", // rgb(239,254,116) -
				"IMG" 			 => "#9933FF", // rgb(153,51,255) -
				"CONCRETE_ARRAY" => "#3366FF", // rgb(51,102,255) -
				"CONCRETE_PROTO" => "#42C642", // rgb(66,198,66) -
				"CONCRETE_SUBST" => "#FF6318", // rgb(255,99,24) -
				"PROJ" 			 => "#E7E763", // rgb(231,231,99) -
				"ROLE"	 		 => "#FBFD29", // rgb(251,253,41) -
				"USER_GROUP"	 => "#FBFD29", // rgb(251,253,41) -
				"W_CHIP_POSYX" 	 => "#DC96C9", // rgb(220,150,201) -
				"W_TRAY"		 => "#DE99CA", // rgb(222,153,202) -
				"W_WAFER" 		 => "#CC6699", // rgb(204,102,153) -
				"W_WAFER_GROUP"  => "#EE99CA", // rgb(238,153,202) -
				"WORKLIST"       => "#42C642", // rgb(66,198,66) -
				"ABS_WORKLIST"   => "#9BE19B", // rgb(155,225,155) -
		);
		
		$this->saveVarioLoop( $sqlo, $colorArray, 'COLOR' );
		
		$shortArr = array(
				'ABS_WORKLIST'  => 'WLA',
				'W_WAFER_GROUP' => 'ABG',
				'W_WAFER' 		=> 'ABC',
				'USER_GROUP'	=> 'GRP',
				'ROLE'	 	=> 'ROL',
				'PROJ' 		=> 'PRJ',
				'LINK' 		=> 'DOC',
				'IMG' 		=> 'IMG',
				'EXP_TMPL' 	=> 'EXT',
				'EXP' 		=> 'EXP',
				'DB_USER'	=> 'USR',
				'CONTACT' 	=> 'CON',
				'CONCRETE_SUBST' => 'SUC',
				'CONCRETE_PROTO' => 'PRC',
				'CONCRETE_ARRAY' => 'ARC',
				'CYTCART'		 => 'CYC',
				'CHIP_READER' 	 => 'DEV',
				'ABS_CYTCART'	 => 'CYA',
				'ARRAY_LAYOUT' 	 => 'ARL',
				'ABSTRACT_SUBST' => 'SUA',
				'ABSTRACT_SPOT'  => 'SPA',
				'ABSTRACT_PROTO' => 'PRA',
				'ABSTRACT_ARRAY' => 'ARA',
				'SOCKET' 	   => 'SOC',
				'W_BATCH_TMPL' => 'ABT',
				'A_CHIP_READER'=> 'DEA',
				'RESX'         => 'REX',
				'W_CHIP_POSYX' => 'ABX',
				'JOUR_ENTRY'   => 'JOU',
				'W_TRAY' 	   => 'CHT',
				'ABS_SOCKET'   => 'SKA',
				'CARTB_TMPL'   => 'CBT',
				'CART_BATCH'   => 'CBC',
				'CABNAT'  	   => 'NAB',
				'CABNAT_TMPL'  => 'NBT',
				'CONTAINER'    => 'CTI',
				'ABS_CONTAINER'    => 'CTA',
				'WORKLIST'     => 'WLI',
		);
		
		$this->saveVarioLoop( $sqlo, $shortArr, 'SHORT' );
	}

}
