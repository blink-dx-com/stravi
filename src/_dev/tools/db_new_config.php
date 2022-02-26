<?php
/**
 * - config a new database
 * PLEASE check /etc/php/7.3/cli/php.ini before using ...
 * @module db_new_config.php
 */
require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('access_check.inc');
require_once ('table_access.inc');
require_once '../../www/pionir/rootsubs/db_imp/g.dbImpAct.inc';


class DB_new_main {
    
    
    function __construct($options) {
        $this->options = $options;
        $this->_lib = new db_imp_act();
        $this->_lib->setshowCmd(1);
        $this->_lib->setModiFlag(1);
        
    }
    
    static function error_stop($text) {
        echo "ERROR: ".$text."\n";
        echo "STOP\n";
        exit;
    }
    static function text_info($text) {
        echo $text."\n";
        
    }
    
    function load_config() {
        
        $database_access=array();
        include ('../../config/config.local.inc');
        
        
        $dbid = $this->options['dbid'];
        
        if (empty($database_access[$dbid])) {
            throw new Exception('No database_access entry exists for dbid:'.$dbid);
        }
        
        $this->db_access= array();
        
        $tmp = &$database_access[$dbid];
        
        $this->db_access['db'] = $tmp["db"]     ;                 /* DBMS service name */
        $this->db_access['user'] = $tmp["LogName"];            /* DBMS user name */
        $this->db_access['pw'] = $tmp["passwd"];        /* DBMS password */
        $this->db_access['dbtype'] = $tmp["_dbtype"];       /* DBMS type */
        
        $filex='../../config/confInst.local.inc';
        if (!file_exists($filex)) {
            throw new Exception('Installation-file "'.$filex.'" is missing.');
        }
        require_once $filex;
        $this->install_conf = gConfigInstGet();
        
        // check 
        $app_data_root = $this->install_conf['appDataDir'];
        if (!file_exists($app_data_root)) {
            throw new Exception('Installation-file: VAR:app_data_root="'.$app_data_root.'" not found in file system.');
        }
        

    }
    
    function create_all() {
        
        $db_user = $this->db_access['user'];
        
        $sqlo = $this->_lib->userlogin($this->db_access);
        
        $magasin_serial = $this->options['db_serial'];
        
        $configDict = $this->install_conf;
        $this->_lib->do_appDirsSet( $sqlo, $db_user, $configDict );
        $this->_lib->do_set_serial( $sqlo, $db_user, $magasin_serial );
    }
}

global $error;
$error = & ErrorHandler::get();

$longopts  = array(
    "dbid:",     // Required value
    "db_serial:",   
);

$options = getopt("c", $longopts);

if(empty($options)) {
    echo "Configure a new database system.\n";
    echo "an entry in config/config.locals.inc must exist.\n";
    echo "config/confInst.local.inc must exist.\n";
    echo "USAGE: ".__FILE__.' -c --dbid="blk3" --db_serial=34'."\n";
    exit;
}

echo "OPTIONS: ".print_r($options,1)."\n";

$main_lib = new DB_new_main($options);

if ($options['dbid']==NULL) {
    DB_new_main::error_stop('no dbid given.');
}

$main_lib->load_config();

if (array_key_exists('c', $options ) ) {
    
    if ( !is_numeric($options['db_serial']) ) {
        DB_new_main::error_stop('db_serial not given.');
    }
    
    DB_new_main::text_info('DO it ...');
    
    $main_lib->create_all();
    if ($error->Got(READONLY))  {
        $allErrarr = $error->getAllArray();
        $out = print_r($allErrarr,1 );
        DB_new_main::error_stop('create_all: '.$out);
    }
}

DB_new_main::text_info('READY');
