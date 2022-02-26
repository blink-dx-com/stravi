<?php 
require_once ('down_up_load.inc');

/**
 * auto create experiment log file
 * used for NGAi
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package o.EXP.autocrea_log.inc
 * @version $Header: trunk/src/phplib/o.EXP.autocrea_log.inc 59 2018-11-21 09:04:09Z $
 */
class o_EXP_autocrea_log {
	
	/**
	 * save log-file in CACHE-Dir $_SESSION['globals']['app.upload_zip'][TYPE] . "_cache"
	 * - dynamically create _cache dir
	 * @param string $yearpath : Z:\Forschung\2017\
	 * @param string $exp_shortname : 2017-04-03.jdjdjdjjk
	 * @param string $relative_dir_from_year e.g. 2017-04/2017-04-03.jdjdjdjjk
	 */
	function save_log_file($yearpath, $exp_shortname, $relative_dir_from_year) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
	
		if ($yearpath==NULL) {
			$error->set( $FUNCNAME, 10, "Input: yearpath missing! Please inform the Admin.");
			return;
		}
	
		$linux_yearpath = netfile2serverpath($yearpath);
		if (!is_writable($linux_yearpath.'/')) {
			$error->set( $FUNCNAME, 2, "System has no write-access on YEAR dir (".$linux_yearpath.")! Please inform the Admin.");
			return;
		}
		
		$cache_path = $yearpath."\\_cache";
		$linux_path_dest = netfile2serverpath($cache_path);
	
		if (!file_exists($linux_path_dest)) {
			if (!mkdir($linux_path_dest)) {
				$error->set( $FUNCNAME, 1, "Could not create _cache dir (".$linux_path_dest.")! Please inform the Admin.");
				return;
			}
		}
	
		if (!is_writable($linux_path_dest.'/')) {
			$error->set( $FUNCNAME, 2, "System has no write-access on _cache dir (".$linux_path_dest.")! Please inform the Admin.");
			return;
		}
		
		$logname_full = $linux_path_dest . '/'. $exp_shortname . '.log';
		$myfile = fopen($logname_full, "w");
		if (!$myfile) {
			$error->set( $FUNCNAME, 3, "Error on writing zip-log-file.");
			return;
		}
		fwrite ($myfile, $relative_dir_from_year);
	
		fclose($myfile);
	}
}