<?php 
require_once 'down_up_load.inc';
require_once 'f.directorySub.inc';

/**
 * 
 * @author Steffen
 * @package o.EXP.DATAURL.subs.inc
 */
class oEXP_DATAURL_subs {
    
    /**
     * 
     * @param string $netname windows path name e.g. Z:\data\hello
     */
    function __construct(string $netname) {
        $this->netname=$netname;
    }
    
    /**
     * directory exists on NET ?
     * @param $netname
     * @return array(
     *    'exists' ==>int $retval
     * 		 0 - image does not exist
     //      1 - image exists
      *   'path'=> string
      * @param string $netname
      * @return number[]|string[]
     */
    static function onNetExists( $netname ) {    
        $retval = 0;
        $serverName = netfile2serverpath( $netname );  
        if ( file_exists( $serverName ) ) {
            $retval = 1;
        }
        
        return array( 'exists'=>$retval, 'path'=>$serverName );
    } 
    

    
    /**
     * at least one DIR of FILE exists on the BASEDIR ?
     * @return int 0,1
     */
    function oneElementExists( ) {
        
        $retval=0;
        $serverpath = netfile2serverpath( $this->netname );

        
        if ( !file_exists( $serverpath ) ) {
            return 0;
        }
        
        if (substr($serverpath,-1,1)!='/') {
            $serverpath = $serverpath . '/';
        }
        $files = glob( $serverpath . '*', GLOB_MARK ); 

        if (is_array($files)) {
            $retval=sizeof($files);
        }
        
        return $retval;
    } 
    
    /**
     * recursive scan
     * @param string $target
     */
    private function _scanDir($target) {
        
        if(is_dir($target)){
            
            $files = glob( $target . '*', GLOB_MARK ); //GLOB_MARK adds a slash to directories returned
            $this->files_all = array_merge($this->files_all, $files);
            
            foreach( $files as $file )  {
                if (is_dir($file)) {
                    $this->_scanDir( $file );
                }
            }

        }
    }
    
    /**
     * get all files in list
     * empty dirs ? => no
     * @return array [] of dict
     *  'file' => LINUX-path-type, relative file name, starting after $this->netname
     *  'ty'   => 'f' or 'd'  file type
     *  'hash' =>
     * 
     * 
     */
    function get_filelist_hashes() {
        
        $serverpath = netfile2serverpath( $this->netname );
        
        $this->files_all=array();
        $this->_scanDir($serverpath);
        
        $filearr = &$this->files_all;
        
        if (empty($filearr)) {
            return array();
        }
        
        $serverpath_len = strlen($serverpath);
        $result=array();
        foreach($filearr as $filex) {
            $hash=NULL;
            if (is_dir($filex)) {
                //
            } else {
                $hash = md5_file($filex);
                $file_short = substr($filex,$serverpath_len+1); // just save path relative ...
                $outrow=array('file'=>$file_short, 'ty'=>'f', 'hash'=>$hash);
                $result[]=$outrow;
            }
        }
        return $result;
    }
    
  
}
