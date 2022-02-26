<?php 
/**
 * lib for HTTP client for UnitTests
 * @author steffen
 * @package http_client.inc
 */
class http_client {
    
    private $_http_header;
	
	/**
	 * 
	 * @param unknown $connect_params
	 * "srv_url"
	 * "hostname"
	 * "protocol" [OPTIONAL]
	 */
	function __construct($connect_params) {
		$this->connpar = $connect_params; 
		$this->debugx  = $_SESSION["userGlob"]["g.debugLevel"];
	}
	
	/**
	 * 
	 * @param unknown $input_vars
	 * @param array $filepath_arr
	 *   'field'=>, 
	 *   'path'=>,
	 *   'shortname' => [OPTIONAL] short file name
	 */
	private function _file_upoad($input_vars, $filepath_arr) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		// 21 strips
		$MULTIPART_BOUNDARY = '--------------------------'.microtime(true);
		
		$header = 'Content-Type: multipart/form-data; boundary='.$MULTIPART_BOUNDARY;
		
		foreach( $filepath_arr as $file_row ) {
		
			// $filename = "/path/to/uploaded/file.zip";
			$filename   = $file_row['path'];
			$FORM_FIELD = $file_row['field'];
			
			if ($filename==NULL or $FORM_FIELD==NULL) {
				$error->set( $FUNCNAME, 1, 'Input-params missing: path or field' );
				return;
			}
			
			$file_base  = basename($filename);
			if ($file_row['shortname']!=NULL) {
				$file_base = $file_row['shortname'];
			}
			
			$file_contents = file_get_contents($filename);
			
			$content =  "--".$MULTIPART_BOUNDARY."\r\n".
					"Content-Disposition: form-data; name=\"".$FORM_FIELD."\"; filename=\"".$file_base."\"\r\n".
					"Content-Type: application/file\r\n\r\n".
					$file_contents."\r\n";
		}
		// add some POST fields to the request too: $_POST['foo'] = 'bar'
		
		foreach( $input_vars as $key => $val ) {
			$content .= "--".$MULTIPART_BOUNDARY."\r\n".
				"Content-Disposition: form-data; name=\"".$key."\"\r\n\r\n".$val."\r\n";
		}
		// signal end of request (note the trailing "--")
		$content .= "--".$MULTIPART_BOUNDARY."--\r\n";
		
		return array($header,$content);
	}
	
	/**
	 * - easy send data on HTTP;
     * - supports file upload
	 * @param array $input_vars
	 * @param array $filepath_arr [OPTIONAL] array to path of uploaded files
	 * @return unknown
	 */
	function send($input_vars, $filepath_arr=NULL) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$header = "Content-Type: application/x-www-form-urlencoded";
		
		if ( sizeof($filepath_arr) ) {
			list($header, $postdata) = $this->_file_upoad($input_vars, $filepath_arr);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'Error on FileUpoad.' );
				return;
			}
		} else {
			$postdata = http_build_query($input_vars);
		}
		
		$header .= "\r\nCache-Control: no-cache\r\n";
		
		$context = stream_context_create(array('http' => array(
				'method' => "POST",
				'header' => $header,
				'content'=> $postdata
		)));

	
		$hostname        = $this->connpar["hostname"];
		$hostname_new='http://'.$hostname;
		if ($this->connpar["protocol"]=='ssl') {
			$hostname_new = 'https://'.$hostname;
			$port=443;
		}
		$url= $hostname_new.$this->connpar["srv_url"];
	
		if ($this->debugx>1) {
			echo $FUNCNAME.":DEBUG:URL:".$url."<br>";
			echo "DEBUG:postdata: \n";
			var_dump($postdata);
			echo "<br>\n";
		}
		
		// close session, because, the URL calls the same server with the same session ...
		session_write_close();

		/*
		$fp = @fopen($url, 'rb', false, $context);
		if (!$fp)
		{
			echo ("Problem with $url, $php_errormsg");
			return;
		}
		$response = @stream_get_contents($fp);
		if ($response === false)
		{
			echo ("Problem reading data from $url, $php_errormsg");
			return;
		}
		*/
		
		$url_nocache = $url;
		if (strstr($url,'?')==NULL) {
			$url_nocache .= '?';
		} else {
			$url_nocache .= '&';
		}
		// $url_nocache .= rand(1,1000000);
		$response = file_get_contents($url_nocache, false, $context);
		
		$tmp_http_header = $http_response_header;
		
		// get page status ...
		$status_header = $tmp_http_header[0];
		$status_tmp_arr = explode(" ",$status_header);
		$this->_page_status = $status_tmp_arr[1];
		$this->_http_header = array();
		foreach($tmp_http_header as $one_val) {
		    $row_arr = explode(":", $one_val,2);
		    $this->_http_header[trim($row_arr[0])] = trim($row_arr[1]);
		}
	
		session_start();
	
		
		return ($response);
	}
	
	function get_http_headers() {
	    return $this->_http_header;
	}
	
	function get_page_status() {
	    return $this->_page_status;
	}
}