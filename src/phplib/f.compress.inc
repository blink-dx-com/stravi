<?php
/**
 * compress files
 * @package  f.compress.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/f.compress.inc 59 2018-11-21 09:04:09Z $
 */
class compressC {

	/* compress file. it's recommended to do this, because text files have
    // usually a very good rate. it's not recommended for data, especially for
    // image files. they have a very bad rate.
    //
    * @param  string      $inFile  a filename
    * @param  string      $outFile a filename [OPTIONAL]
    * @return  true | false
    */
    function zip($inFile, $outFile=NULL)
    {
        if ($inFile == null or !is_string($inFile))
            return false;

        // is the gzip package included ?
        if ( !( function_exists('gzopen') && function_exists('gzwrite') && function_exists('gzclose')) )
        {
        	return false;
        }
        
        if ($outFile==NULL) $zipFilename = $inFile . ".gz";            // add .gz extension; new zip file
        else $zipFilename = $outFile;

        if (!($zipFile = gzopen($zipFilename, "w9")))   // write mode; maximum compression
            return false;

        if (!($inFileFp = fopen($inFile, "r")))     // open file for read
            return false;

        // read data - compress it - write it
        gzwrite($zipFile, fread($inFileFp, filesize($inFile)), filesize($inFile));

        gzclose($zipFile);                      // close
        fclose($inFileFp);                      // close

        return true;
    }    
 
    
}
