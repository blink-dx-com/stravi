"""
:summary: create patches for Partisan
CREATE:

    %cd ~/html/partisan
    %python _dev/tools/parti_patch.py -i ~/2017/Partisan/2017-07.27.ball4.txt -v -c -t tarfile.tar
    %python _dev/tools/parti_patch.py -v -c -t tarfile.tar -i ~/2017/Partisan/2017-07.27.ball4.txt

    -i X:\Development\Python\Patch\files.txt -v -c -t tarfile.tar

INSTALL:
     # copy tarfile.tar to /opt/partisan
     %cd /opt/partisan
     %python _dev/tools/parti_patch.py -e -t tarfile.tar

CREATE:
    - create a TAR-file
    - TAR-file contains a _PartiPatchID.txt file as indicator

FORMAT _PartiPatchID.txt:
    - CSV-format; KEY-VALUE separator: ":"
    - user: name of Patch creator
    - creation_date: date of creation
    - patch-ID: ID of Patch
    - info: short text

:version: 21.11.2018
:author: Steffen Kube
"""

import sys
import getopt
import os
import string
import tarfile
import time

try:
   from hashlib import md5
except ImportError:
   from md5 import md5

'''
main patch class
'''
class patchx:

    PatchID_file = '_PartiPatchID.txt' # Identificator file of current patch
    patchlogfile = 'patch_log.txt'     # LOG file
    patchlog_dir = '/var/log/partisan' # LOG file directory
    infile = ''

    def __init__(self):
        pass

    def get_full_patchlog_url(self):
        return os.path.join(self.patchlog_dir, self.patchlogfile)

    def set_infile(self, infile):
        self.infile = infile

    def set_patchlog_dir(self, dir):
        self.patchlog_dir =  dir

    '''
    create patch ID file self.PatchID_file

    '''
    def _creaPatchID_file(self):
        import getpass

        filename = self.PatchID_file
        patchinfo=''
        patchID  = ''

        datex_H = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())

        datadict = {
            'user': getpass.getuser(),
            'creation_date': datex_H,
            'patch-ID': patchID,
            'info': patchinfo
        }

        fo = open(filename, 'w')
        for (key, val) in datadict.items():
            fo.write(key+":"+val+ "\n")

        fo.close()

        return filename

    '''
    create TAR-file
    '''
    def create(self, tarfilex):

        filename = self.infile
        blacklist = ['config/']

        if not os.path.exists(filename):
            raise ValueError ("file "+filename+" does not exist")

        dataArray = []

        tar = tarfile.open(tarfilex, "w")

        fo = open(filename, 'r')
        for line in fo.xreadlines():

            onefile = line.strip()
            if onefile == "":
                continue

            print "- "+onefile
            error = 0

            while True:
                if not os.path.exists(onefile): # file_exists
                    print "  ERROR: not found"
                    error=1
                    break

                for black_pattern in blacklist:
                    part_file = onefile[0:len(black_pattern)]
                    if part_file==black_pattern:
                        print "  ERROR: Pattern not allowed!"
                        error=1
                        break
                if error:
                    break

                tar.add(onefile)
                dataArray.append(onefile)

                break


        fo.close()

        id_filename = self._creaPatchID_file()
        tar.add(id_filename)

        tar.close()

    def _getFileInfo(self, filename):

        datex_H = ""
        hashx   = ""

        if os.path.exists(filename):
            datex   = time.localtime( os.path.getmtime(filename) )

            datex_H = time.strftime("%Y-%m-%d %H:%M:%S", datex)
            hashx = md5(open(filename,'rb').read()).hexdigest()

        result = {'date':datex_H, 'hash':hashx }
        return result

    '''
    check, if Patch is valid
    '''
    def _checkPatch_valid(self, tar, filenames):

        self.patch_id_info = {}

        PatchID_file = self.PatchID_file
        found=0
        for onefile in filenames:
            if onefile == PatchID_file:
                found=1
                break

        if not found:
            raise ValueError ("PatchID_file not found.")

        tar.extract(PatchID_file)

        # read PatchID_file
        fo = open(PatchID_file, 'r')
        id_info_dict={}
        for line in fo.xreadlines():
            if line!='':
                pos = line.find(':')
                key = line[0:pos]
                val = line[pos+1:]
                val = string.strip(val)
                id_info_dict[key]=val

        self.patch_id_info = id_info_dict

        if not id_info_dict.has_key('user') or id_info_dict['user']=='':
            raise ValueError ("PatchID_file: user not defined.")

        return id_info_dict

    '''
    write ID info to log
    P DATE_INSTALL, USER, PATCH_ORI_DATE, PATCH_ID
    '''
    def _write_ID_log(self, f_log, patch_info):

        needvals=('user','creation_date', 'patch-ID', 'info')
        for key in needvals:
            if not patch_info.has_key(key):
                patch_info[key]=''

        datex_H = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())

        print "... Patch_info: user:"+patch_info['user']+" Patch_creation_date:"+patch_info['creation_date']

        outarr = ( 'P', datex_H, patch_info['user'],  patch_info['creation_date'], patch_info['patch-ID'] )
        for strx in outarr:
            f_log.write(str(strx)+";")

        f_log.write("\n")

    '''
    extract TAR-FILE
    - write log file

    FORMAT patchlogfile
    IDENT DATE_extract FILENAME DATE_oldfile HASH_oldfile DATE_newfile HASH_newfile
    - IDENT:
        P: patch info line
        D: patch details
    '''
    def extract(self,tarfilex):

        patchlogfile= self.get_full_patchlog_url()

        if not os.path.exists(tarfilex):
            raise ValueError ("TAR-file "+tarfilex+" does not exist")

        print "... PatchLogFile: " + patchlogfile

        try:
            f_log = open(patchlogfile,"a") # append
        except:
            message = 'Unable to write to log-file ' + patchlogfile
            raise ValueError (message)


        tar = tarfile.open(tarfilex)
        filenames = tar.getnames()

        patch_info = self._checkPatch_valid(tar, filenames)

        self._write_ID_log(f_log, patch_info)

        print ("... extract files:")
        for onefile in filenames:

            print "- " + onefile

            oldFile_info = self._getFileInfo(onefile)

            nowx = time.localtime()
            nowFormat = "%04u-%02u-%02u %02u:%02u:%02u" % (nowx[0], nowx[1],nowx[2],nowx[3],nowx[4],nowx[5])

            tar.extract(onefile)

            newFile_info = self._getFileInfo(onefile)
            outarr = ( 'D', nowFormat,onefile,oldFile_info['date'],oldFile_info['hash'],newFile_info['date'],newFile_info['hash'] )
            for strx in outarr:
                f_log.write(str(strx)+";")

            f_log.write("\n")

        tar.close()
        f_log.close()


    def usage(self):
        print "create/extract Patch for Partisan application"
        print " -t FILE : TAR-file name"
        print " create Patch: "
        print " -c : create TAR"
        print " -i FILE : input file, contains list of files"
        print " extract Patch: "
        print " -e : extract TAR"
        print " [optional] --patchLogDir {DIR} to weite the logfile "
        print " ... writes log file to " + self.get_full_patchlog_url()


if __name__ == "__main__":

    infile = None
    tarfilex= None
    source = None
    action =''
    mainobj = patchx()
    # read command line arguments


    opts, args = getopt.getopt(sys.argv[1:], 'cevi:t:s:', ['infile=', 'patchLogDir='])
    for o, a in opts:
        if o == "-v":
            verbose = True
        if o in ("-h", "--help"):
            mainobj.usage()
            sys.exit()
        if o in ("-i", "--infile"):
            infile = a
            print "- infile: " + infile
        if o in ("--patchLogDir",):
            patchLogDir = a
            print "- patchLogDir: " + patchLogDir
            mainobj.set_patchlog_dir(patchLogDir)
        if o in ("-t", ):
            tarfilex = a
        if o in ("-s", ):
            source = a
        if o in ("-c", ): # create
            action = 'create'
        if o in ("-e", ): # extract
            action = 'extract'

    if action=='':
        mainobj.usage()
        sys.exit()

    # os.chdir('X:\\html\\partisan')
    # os.chdir('X:\\Development\\Python\\Patch\\data')
    cwd = os.getcwd()
    print "PWD: "+cwd


    mainobj.set_infile(infile)

    if action=='create':
        print "create TAR-file from "+ infile
        print "- Output: "+ tarfilex
        print "- Possible copy-cmd> scp "+tarfilex +" root@robur:/opt/partisan"
        mainobj.create(tarfilex)

    if action=='extract':
        print "... extract TAR-file: " + tarfilex
        mainobj.extract(tarfilex)

    print "READY"

