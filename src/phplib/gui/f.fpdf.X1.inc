<?php
require_once ('ots/fpdf/PDF_MC_Table.php');

/**
 * extension for fpdf

 * @package f.fpdf.X1.inc 
 * @swreq SREQ:0003050: g > PDF-support : FPDF-library 
 * @swreq SREQ:0003050:002: UTF-8-Supportsupport UTF-8 encoded text input: set $this->z_iniarr['encoding']= 'UTF-8'
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/gui/f.fpdf.X1.inc 59 2018-11-21 09:04:09Z $
 */
class pdfX1 extends PDF_MC_Table {

	var $z_lineHeight = 6;

	/**
	 * init class
	 * @param $iniarr
	 *   'footText' => string e.g. QC2-Report; Document-Creation-Tool:LAB/o.CONCRETE_SUBST.qc2rep
	 *   'headText'=> string header-text; e.g. "creation date: 2012-03-01"
	 *   'watermarkText' => string
	 *   'encoding' => ['UTF-8'] : standard text input; this variable cyn be empty
	 */
	function z_x1Init($iniarr) {
		$this->z_iniarr = $iniarr;
		if ($this->z_iniarr['encoding']==NULL) $this->z_iniarr['encoding'] = 'UTF-8'; // default
	}
	
	function Header() {
		if ($this->z_iniarr['watermarkText']!=NULL) {
	    	$this->SetFont('Arial','I',80);
	    	$this->SetTextColor(230,230,230);
	    	$this->Text(60, 60, $this->z_iniarr['watermarkText']);
	    	$this->SetTextColor(0,0,0);
	    	$this->SetFont('Arial','I',10);
	    }
	}
	
	/**
	 * page footer
	 * @global $this->z_iniarr
	 *   'watermarkText'
	 *   'footText' : footer text
	 *   'headText' : header text
	 * @see phplib/ots/fpdf/FPDF::Footer()
	 */
	function Footer()
	{
		$this->SetY(10); 
	    $this->SetFont('Arial','I',10);
	    $this->Cell(80,10, 'XXX GmbH; '.$this->z_iniarr['headText'],1,0,'L');
	    
	    
		
	    //Position at 1.5 cm from bottom
	    $this->SetY(-11); // -15
	    //Arial italic 8
	    $this->SetFont('Arial','I',10);
	    $pageno = $this->PageNo();
	    $this->Write(6, $this->z_iniarr['footText'].'  Page: '.$pageno.'/{nb}');
	    
	}
	
	/**
	 * 
	 * @param $stylearr 'Arial','I',10
	 */
	function z_initFont($stylearr) {
		$this->z_stylearr = $stylearr;
		$this->SetFont($this->z_stylearr[0], $this->z_stylearr[1], $this->z_stylearr[2]);
	}
	
	/**
	 * open special font
	 * @param array $stylearr : each entry can be empty
	 */
	function z_openFont($stylearr) {
		
		if ($stylearr[0]==NULL) $stylearr[0]=$this->z_stylearr[0];
		if ($stylearr[1]==NULL) $stylearr[1]=$this->z_stylearr[1];
		if ($stylearr[2]==NULL) $stylearr[2]=$this->z_stylearr[2];
		
		$this->SetFont($stylearr[0], $stylearr[1], $stylearr[2]);
	}
	
	/**
	 * set back to standard font
	 * @param $stylearr
	 */
	function z_closeFont() {
		$this->SetFont($this->z_stylearr[0], $this->z_stylearr[1], $this->z_stylearr[2]);
	}
	
	/**
	 * draw table
	 * @param $dataarr
	 * @param array $colsizeArr
	 * 		(index => size)
	 * @param  $options
	 * 	'pageXOff' => number
	 *  'header' => headerarray
	 */
	function z_tableOut($dataarr, $colsizeArr, $options=NULL) {
		
		if (is_array($options['header'])) {
			if ($options['pageXOff']>0)
	    		$this->Cell($pageXOff);
	    		
	    	$gray = 230;
	    	$this->SetFillColor($gray);
	    	
	    	$head = &$options['header'];
	    	
	    	$this->SetWidths($colsizeArr);
	    	$this->SetAligns('C');
	    	
	    	if ($this->z_iniarr['encoding']=='UTF-8') {
	    		reset ($head);
	    		foreach( $head as $key=>$val) {
	    			$head[$key] = utf8_decode($val);
	    		}
	    		reset ($head);
	    	}
	    	
	    	$this->Row($head,1);
	    	$this->SetAligns('L');
	    	
	    	
		}
		
		if (is_array($dataarr)) {
		
			foreach($dataarr as $row) {
		    	if ($options['pageXOff']>0)
		    		$this->Cell($pageXOff);
		    		
				if ($this->z_iniarr['encoding']=='UTF-8') {
					// @swreq SREQ:0003050:002: UTF-8-Support
					foreach( $row as $key=>$val) {
						$row[$key] = utf8_decode($val);
					}
					reset ($row); 
				}
	
		    	$this->SetWidths($colsizeArr);
		    	$this->Row($row,0);
		    	/*
		        reset ($colsizeArr);
		        foreach( $colsizeArr as $key=>$colSize) {
		        	$this->Cell($colSize,$this->z_lineHeight,$row[$key],1);
		        }
		        $this->Ln();
		        */
		       
		    }
		}
	}
	
	// write float line with LF
	function z_fullLine($text, $style=NULL) {
		
		if ($this->z_iniarr['encoding']=='UTF-8') {
			// @swreq SREQ:0003050:002: UTF-8-Support
			$text = utf8_decode($text);
		}
		
		if ($style=='B') {
			if ($this->z_stylearr[0]==NULL) echo "<b>ERROR:</b> z_fullLine() no general style set!<br>";
			$this->SetFont($this->z_stylearr[0], 'B', $this->z_stylearr[2]);
		}
		$this->Write($this->z_lineHeight, $text);
		if ($style=='B') {
			$this->SetFont($this->z_stylearr[0], $this->z_stylearr[1], $this->z_stylearr[2]);
		}
		$this->Ln();
	}
	
}
