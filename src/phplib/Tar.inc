<?

/**
 * php class for tar-ing directories and files. 
 * !!! Note the functionality is reduced to the special requirements of paxml.
 * for normal compress use: zip.inc !
 * @package Tar.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/Tar.inc 59 2018-11-21 09:04:09Z $
 */
class Tar
{
    var $tarFile;
    var $tarFileName;
    var $status;
    var $baseDir;



    // create a tar file
    //
    // input:   string      $tarFileName        filename
    // output:  true | false
    //
    function create($tarFileName)
    {
        if ($tarFileName != null and is_string($tarFileName))
        {
            $this->tarFileName = $tarFileName;
            if (($this->tarFile = fopen($tarFileName, "wb")) == false)
                return false;
                
            $this->status = "created";
            return true;
        }
        return false;
    }
    
    
    
    // open a tar file
    //
    // input:   string      $tarFileName        filename
    // output:  true | false
    //
    function open($tarFileName)
    {
        if ($tarFileName != null and is_string($tarFileName))
        {
            $this->tarFileName = $tarFileName;
            if (($this->tarFile = fopen($tarFileName, "rb")) == false)
                return false;
                
            $this->status = "opened";
            return true;
        }
        return false;
    }
    
    
    // change directory, where you want to start
    //
    // input:   string      $tarFileName        filename
    // output:  true | false
    //
    function cd($dir)
    {
        $this->baseDir = $dir;
    }
    

    // add a file to tar file
    //
    // input:   string      $filename           filename
    // output:  true | false
    //
    function add($filename)
    {	
        // chekc input
        if (!file_exists($filename) or $this->status != "created")
            return false;

        // get filetype
        switch(filetype($filename))
        {
            case "fifo"    : $typeflag = "6"; break;
            case "char"    : $typeflag = "3"; break;
            case "dir"     : $typeflag = "5"; break;
            case "block"   : $typeflag = "4"; break;
            case "link"    : $typeflag = "1"; break;
            case "file"    : $typeflag = "0"; break;
            case "unknown" : $typeflag = "0"; break;
        }
        
        // write header
		if (strlen($filename) > strlen($this->baseDir))
			$name = substr($filename, strlen($this->baseDir), strlen($filename) - strlen($this->baseDir));
		else
			return false;
		
        $name     = str_pad($name . (($typeflag == 5) ? "/" : ""), 100, chr(0));                    // fill up to 100 bytes; a dir ends with /
        $mode     = str_pad(decoct(fileperms($filename)), 7, decoct(chr(0)), false) . chr(0);       // octal number; null terminated
        $uid      = str_pad(decoct(fileowner($filename)), 7, decoct(chr(0)), false) . chr(0);       // octal number; null terminated
        $gid      = str_pad(decoct(filegroup($filename)), 7, decoct(chr(0)), false) . chr(0);       // octal number; null terminated
        $size     = str_pad(decoct(($typeflag == 1 or $typeflag == 5) ? 0 : filesize($filename)), 11, decoct(chr(0)), false) . chr(0);   // octal number; null terminated; if typeflag == LINKTYPE then size = 0 ??
        $mtime    = str_pad(decoct(filemtime($filename)), 11, decoct(chr(0)), false) . chr(0);      // octal number; null terminated
        $linkname = str_pad("", 100, chr(0));   ;                                                   // octal number; null terminated
        $magic    = "ustar  " . chr(0);       // null terminated
        $uname    = str_pad("pxml", 32, chr(0));        // user name; null terminated; ignore this values
        $gname    = str_pad("pxml", 32, chr(0));        // group name; null terminated; ignore this values
        $devmajor = str_pad("", 8, chr(0));
        $devminor = str_pad("", 8, chr(0));
        $prefix   = str_pad("", 155, chr(0));
					
        $headerBlock = ""
                     . $name
                     . $mode
                     . $uid
                     . $gid
                     . $size
                     . $mtime
                     . "        "                     // empty space as nlank characters for calculating checksum
                     . $typeflag
                     . $linkname
                     . $magic
                     . $uname
                     . $gname
                     . $devmajor
                     . $devminor
                     . $prefix;
        $headerBlock = str_pad($headerBlock, 512, chr(0));
        
        $chksum = decoct($this->checksum($headerBlock));
        $chksum = str_pad(str_pad($chksum, strlen($chksum) + strlen($chksum) % 2, decoct(chr(0)), false) . chr(0), 8, " ");

        $headerBlock = ""
                     . $name
                     . $mode
                     . $uid
                     . $gid
                     . $size
                     . $mtime
                     . $chksum                     // insert checksum
                     . $typeflag
                     . $linkname
                     . $magic
                     . $uname
                     . $gname
                     . $devmajor
                     . $devminor
                     . $prefix;
        $headerBlock = str_pad($headerBlock, 512, chr(0));

        // write file to tar
        $file = fopen($filename, "r");
		if (!$file)
			return false;
        
        // write header
        fwrite($this->tarFile, $headerBlock, 512);

        // recursive call ?
        if (is_dir($filename))
        {
            $dir = opendir($filename);
            while (($element =  readdir($dir)) != false)
                if ($element != "." and $element != "..")
                    $this->add($filename . "/" . $element);
            return true;
        }

        // write file data
        while (!feof($file))
        {
            $data = fread($file, 512);
            if (strlen($data) > 0)
                fwrite($this->tarFile, str_pad($data, 512, chr(0)), 512);
        }
        
        fclose($file);
        
        return true;
    }
    
    
    // calculate checksum
    function checksum($text)
    {
        $result = 0;
        for($index = 0; $index < strlen($text); $index++)
            $result += ord($text[$index]);
            
        return $result;
    }
    

    // close tar file
    function close()
    {
        if ($this->status == "created")
        {
            fwrite($this->tarFile, str_pad(chr(0), 512, chr(0)), 512);
            fwrite($this->tarFile, str_pad(chr(0), 512, chr(0)), 512);
        }
        fclose($this->tarFile);
    }
    
    
    // validate checksum
    function validateChecksum($header)
    {
        $chksum = decoct($this->checksum(substr($header, 0, 148) . "        " . substr($header, 156, 356)));
        $chksum = str_pad(str_pad($chksum, strlen($chksum) + strlen($chksum) % 2, decoct(chr(0)), false) . chr(0), 8, " ");

        // chksum == checksum(header)
        return substr($header, 148, 8) === $chksum;
    }
        
        

    // extract files. destination has to be a path (empty would be the best).
    function extractNext($destination)
    {
        $tarData = fread($this->tarFile, 512);
        
        if ($tarData === str_pad(chr(0), 512, chr(0)))
            return false;
        
        $name     = str_replace(chr(0), "", substr($tarData, 0, 100));               // use
        $mode     = octdec(substr($tarData, 100, 8));       // use
        $uid      = octdec(substr($tarData, 108, 8));       // ignore
        $gui      = octdec(substr($tarData, 116, 8));       // ignore
        $size     = octdec(substr($tarData, 124, 12));      // use
        $mtime    = octdec(substr($tarData, 136, 12));      // ignore
        $chksum   = substr($tarData, 148, 8);               // validate
        $typeflag = substr($tarData, 156, 1);               // use
        $linkname = substr($tarData, 157, 100);             // possibly ignore
        $magic    = substr($tarData, 257, 8);      // check
        $uname    = substr($tarData, 265, 32);     // ignore
        $gname    = substr($tarData, 297, 32);     // ignore

        if (!$this->validateChecksum($tarData))
            return false;
            
        // check if a) destination is a dir b) ends with /

        // is it a dir ?
        if ($typeflag == 5)
        {
            mkdir($destination . $name, $mode);
            return $name;
        }
            
        // is a stinknormal file ?
        if ($typeflag == 0)
        {
            $file = fopen($destination . $name, "wb");

            $sizeIdx = $size;
            while ($sizeIdx > 0)
            {
                $fileData = fread($this->tarFile, 512);
                fwrite($file, $fileData, (($sizeIdx > 512) ? 512 : $sizeIdx));
                $sizeIdx = $sizeIdx - 512;
            }
            
            fclose($file);
        }
        
        return $name;
    }
}
