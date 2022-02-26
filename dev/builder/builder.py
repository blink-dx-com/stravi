"""
Build an installable artifact for Gozilla
CREATE:

   %cd GIT/dev/builder
   %python builder.py  -v -c -src "[GIT]/src" --type=LIMS  -t LIMS.tar.gz
   %python builder.py  -v -c -src "../../src" --type=LIMS  -t LIMS.tar.gz
   %python builder.py  -v -c --src=C:\\Users\\Steffen\\Documents\\Code\\Gozilla\\src --type=Blink  -t LIMS.tar.gz
   
   OLD:
    %cd ~/html/partisan
    %python _dev/tools/parti_patch.py -i ~/2017/Partisan/2017-07.27.ball4.txt -v -c -t tarfile.tar
    %python _dev/tools/parti_patch.py -v -c -t tarfile.tar -i ~/2017/Partisan/2017-07.27.ball4.txt

INSTALL:

   # copy tarfile.tar to /opt/install
   %cd /opt/install
   %python builder.py -v -x -t LIMS.tar.gz --dest="/opt/gozilla"

   OLD: 
     # copy tarfile.tar to /opt/partisan
     %cd /opt/partisan
     %python _dev/tools/parti_patch.py -e -t tarfile.tar

CREATE:
    - create a TAR-file
    - TAR-file contains a _PartiPatchID.txt file as indicator

FORMAT _GozillaInstallID.txt:
    - CSV-format; KEY-VALUE separator: ":"
    - user: name of Install creator
    - creation_date: date of creation
    - patch-ID: ID of Patch
    - info: short text
    
:changes:
  2022-01-03: new structure of Gozilla-Directories ==> LAB-Types
    
:author: Steffen Kube
"""

import sys
import getopt
import os
import string
import tarfile
import time
import shutil

from hashlib import md5

__version__ = "0.2 2022-01-03"

'''
main patch class
'''
class patchx:
    
    _TEST_CMF=r'%python builder.py  -v -c --src="C:\Users\Steffen\Documents\Code\Gozilla_Latex\0_build_example\src" --type=LIMS  -o LIMS.tar.gz '
    
    APP_TYPES=[
        {'id':'Blink', 'src_app_dir':'Blink' , 'lab_dir':'lab_blk'},
        {'id':'HUB-D', 'src_app_dir':'Blink' , 'lab_dir':'lab_blk', '_app_types_dir':'2021_hubd' },
        {'id':'Alere', 'src_app_dir':'Alere' , 'lab_dir':'lab'}, # here lab is the old original LAB-dir
    ]
    
    APP_BASE_NAME='Gozilla'

    PatchID_file = '_GozillaInstallID.txt' # Identificator file of current patch
    patchlogfile = 'patch_log.txt'     # LOG file
    patchlog_dir = '.' # LOG file directory
    infile = ''
    temp_dest_dir= None # TEMP destination dir
    
    app_args = {}
    '''
        'src' source dir
    '''

    def __init__(self):
        self.verbose=0
    
    def info(self,text, min_verbose=1):
        print ('INFO: ' + text )
        
    def set_verbose(self, verbose):
        self.verbose=verbose
    
    def _get_app_type(self, key):
        found=0
        for row in self.APP_TYPES:
            if row['id']==key:
                found=1
                break
        if not found:
            raise ValueError ('APP-type "'+key+'" not found.')
        
        return row

    def get_full_patchlog_url(self):
        return os.path.join(self.patchlog_dir, self.patchlogfile)

    def set_infile(self, infile):
        self.infile = infile

    def set_patchlog_dir(self, dir):
        self.patchlog_dir =  dir

    '''
    create patch ID file self.PatchID_file

    '''
    def _creaPatchID_file(self, infoarr):
        import getpass

        filename_short = self.PatchID_file
        filename = os.path.join(self.temp_dest_dir, filename_short)
        patchinfo=''
        patchID  = ''

        datex_H = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())

        datadict = {
            'user': getpass.getuser(),
            'creation_date': datex_H,
            'app-type': infoarr.get('type',''),
            'patch-ID': patchID,
            'info': patchinfo
        }

        fo = open(filename, 'w')
        for (key, val) in datadict.items():
            fo.write(key+":"+val+ "\n")

        fo.close()

        return filename
    
    def _rm_dir(self, temp_dest_dir):
        pass
    
    def _remove_files(self, rm_dirs, rm_files ):

        temp_dest_dir = self.temp_dest_dir
        
        for dirx in rm_dirs:
            dirx_full = os.path.join(temp_dest_dir,dirx)
            if os.path.exists(dirx_full): 
                shutil.rmtree(dirx_full)
                
        for filex in rm_files:
            filex_full = os.path.join(temp_dest_dir,filex)
            if os.path.exists(filex_full): 
                os.remove(filex_full)      

    def _remove_other_lab_dirs(self, keep_dir):
        
        temp_dest_dir = self.temp_dest_dir
        dirx_full = os.path.join(temp_dest_dir,'www')
        all_files = os.listdir(dirx_full)
        for filex in all_files:
            dirx_loop_full = os.path.join(dirx_full, filex)
            if os.path.isdir(dirx_loop_full):
                if filex.startswith('lab_'):
                    if filex!=keep_dir:
                        self.info('Remove dir: '+dirx_loop_full,1)
                        shutil.rmtree(dirx_loop_full)
    
    def _apptype_subdir_handle(self, app_info):
        '''
        handle a sub-type like app_info['id']='HUB-D'
        '''
        
        temp_dest_dir = self.temp_dest_dir
        
        # remove all LAB dirs in www/ except one
        self._remove_other_lab_dirs(app_info['lab_dir'])
        
        app_type_dir = app_info['_app_types_dir']
        app_type_dir_full = os.path.join( temp_dest_dir, '_app_types', app_type_dir)
        
        if not os.path.exists(app_type_dir_full):
            raise ValueError ('app-type-dir "'+ app_type_dir_full +'" missing.')

        # copy recursive ...
        shutil.copytree(app_type_dir_full, temp_dest_dir, dirs_exist_ok=True)        

    def _apptype_handle(self, app_info):
        '''
        handle the APP-type
        find LAB-src in self.app_args['src']
        #test-it
        '''
        
        temp_dest_dir = self.temp_dest_dir
        
        # remove all LAB dirs in www/ except one
        self._remove_other_lab_dirs(app_info['lab_dir'])
        
        app_type_dir  = app_info['src_app_dir']
        src_dir_BASE  = self.app_args['src']
        src_dir_super = os.path.join(src_dir_BASE, '..', '..') 
        app_type_dir_full  = os.path.join(src_dir_super, self.APP_BASE_NAME+'_'+app_type_dir, 'src')
        app_type_dir_full =  os.path.realpath(app_type_dir_full)
        #OLD: app_type_dir_full = os.path.join( temp_dest_dir, '_app_types', app_type_dir)

        if not os.path.exists(app_type_dir_full):
            raise ValueError ('app-type-dir "'+ app_type_dir_full +'" missing.')
        
        #OLD if app_info['id']=='LIMS':
        # copy recursive ...
        shutil.copytree(app_type_dir_full, temp_dest_dir, dirs_exist_ok=True)
        
        if app_info.get('_app_types_dir','')!='':
            self._apptype_subdir_handle(app_info)

    def _create_basics(self, app_info):
        
        rm_dirs= [
            'config',
            '_test'
        ]
        rm_files= [
            '.git',
            '.project',
            '.buildpath'
        ]   
        
        if app_args.get('withUT', 0):
            tst_index = rm_dirs.index('_test')
            del rm_dirs[tst_index]
        
        self._remove_files(rm_dirs, rm_files)  
        
        self._apptype_handle(app_info) 
        
        # post delete 
        rm_dirs= [
            '_app_types'
        ]       
        
        self._remove_files(rm_dirs, rm_files) 
    '''
    create INSTALL TAR-file
    # --src="[GIT]/src" --type=LIMS  -t LIMS.tar.gz
    '''
    def create(self, app_args):
        
        self.app_args = app_args
        temp_path = 'temp'
        
        if not os.path.exists(app_args['src']):
            raise ValueError ('src-path "'+ app_args['src'] +'" missing')
        
        app_info = self._get_app_type(app_args['type'])
        
        out_file = app_args['tarfile']
        
        # create TEMP dir
        if os.path.exists(temp_path): 
            shutil.rmtree(temp_path)
            
        #os.mkdir(temp_path)
        
        temp_dest_dir = temp_path
        self.temp_dest_dir = temp_dest_dir
        
        # creat  temp, by copying  to temp
        shutil.copytree(app_args['src'], temp_dest_dir)
        
        # analyse, move, delete
        self._create_basics(app_info)
        
        infoarr={
            'type': app_info['id']
        }
        id_filename = self._creaPatchID_file(infoarr)
               
        
        # CREATE tar a files in TEMP
        
        temp_tar_full = os.path.join(temp_dest_dir,out_file)
        self.info('Create TAR-file: '+ temp_tar_full)
        with tarfile.open(temp_tar_full, mode='w:gz') as archive:
            archive.add(temp_dest_dir, recursive=True, arcname='')  
            

    '''
    create TAR-file
    '''
    def patch_create(self, tarfilex):

        filename = self.infile
        blacklist = ['config/']

        if not os.path.exists(filename):
            raise ValueError ("file "+filename+" does not exist")

        dataArray = []

        tar = tarfile.open(tarfilex, "w")

        fo = open(filename, 'r')
        for line in fo.readlines():

            onefile = line.strip()
            if onefile == "":
                continue

            print ("- "+onefile)
            error = 0

            while True:
                if not os.path.exists(onefile): # file_exists
                    print ("  ERROR: not found")
                    error=1
                    break

                for black_pattern in blacklist:
                    part_file = onefile[0:len(black_pattern)]
                    if part_file==black_pattern:
                        print ("  ERROR: Pattern not allowed!")
                        error=1
                        break
                if error:
                    break

                tar.add(onefile)
                dataArray.append(onefile)

                break


        fo.close()

        infoarr={}
        id_filename = self._creaPatchID_file(infoarr)
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
        for line in fo.readlines():
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

        print ("... Patch_info: user:"+patch_info['user']+" Patch_creation_date:"+patch_info['creation_date'])

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

        print ("... PatchLogFile: " + patchlogfile)

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

            print ("- " + onefile)

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
        '''
        %python builder.py  -v -c --src="[GIT]/src" --type=LIMS  -t LIMS.tar.gz
        '''
        types_nice_arr=[]
        for row in self.APP_TYPES:
            types_nice_arr.append(row['id'])
        
        print ("create/extract Patch for Partisan application")
        
        print (" create INSTALL: ")
        print (" -c : create TAR")
        print (" --src=DIR : source file")
        print (" --type=LIMS : type of APP: " + ', '.join(types_nice_arr) )
        print (" -t FILE : TAR-file name for CREATE")
        print (" --withUT : with unit-tests")
        
        print (" extract INSTALL: ")
        
        print (" -x : extract TAR")
        print (" -t FILE : TAR-file name for EXTRACT")
        print (" or simple-bash: in dir /opt/gozilla  ")
        print (" %tar -xvf LIMS.tar.gz  ")
        
        #print ("  [optional] --patchLogDir {DIR} to weite the logfile ")
        print ("  ")
        print (" create Patch: ")
        print (" -p : create TAR")
        print (" -src=DIR : source file")
        print (" -pfile=FILE : path-list-file")
        print (" extract Patch: ")  
        print (" -y : extract TAR")
        print ("  ")
        print (" ... OTHER ... ")
        print (" -v verbose ")


if __name__ == "__main__":

    infile = None
    tarfilex= None
    
    action =''
    mainobj = patchx()
    # read command line arguments

    app_args={}

    opts, args = getopt.getopt(sys.argv[1:], 'cevx:t:s:o:', ['src=', 'pfile=', 'patchLogDir=', 'type=', 'withUT', 'version'])
    for o, a in opts:
        
        if o == "-v":
            verbose = True
        if o in ("-h", "--help"):
            mainobj.usage()
            sys.exit()
        if o in ("-t", ):
            app_args['tarfile']=a
            print ("- tarfile: " +  app_args['tarfile'])    
            
            
        if o in ("", "--pfile="):
            infile = a
            print ("- path-list-file: " + infile)
        if o in ("--patchLogDir",):
            patchLogDir = a
            print ("- patchLogDir: " + patchLogDir)
            mainobj.set_patchlog_dir(patchLogDir)

        if o in ("-c", ): # create
            action = 'create'
        if o in ("-s", "--src",):
            app_args['src']=a
            print ("- src: " + app_args['src'])     
        if o in ("--type",):
            app_args['type']=a
            print ("- type: " + app_args['type'])  
                    
        if o in ("--withUT", ): # create
            app_args['withUT']=1
            print ("- with UnitTests " )
            
        if o in ("--version", ): 
            print ("Version: "+ __version__ )
            print ("READY")
            sys.exit()  
            
            
        if o in ("-x", ): # extract
            action = 'extract'
        if o in ("-p", ): # create
            action = 'p_create'
        if o in ("-y", ): # extract
            action = 'p_extract'            

    if action=='':
        mainobj.usage()
        sys.exit()

    # os.chdir('X:\\html\\partisan')
    # os.chdir('X:\\Development\\Python\\Patch\\data')
    cwd = os.getcwd()
    print ("PWD: "+cwd)

    mainobj.set_verbose(verbose)
    mainobj.set_infile(infile)

    if action=='create':
        print ("create INSTALL TAR-file")
        # --src="[GIT]/src" --type=LIMS  -t LIMS.tar.gz
        if app_args.get('src','')=='':
            raise ValueError ('src missing')
        if app_args.get('type','')=='':
            raise ValueError ('type missing')
        if app_args.get('tarfile','')=='':
            raise ValueError ('tarfile-name missing')        

        # print ("- Possible copy-cmd> scp "+app_args['tarfile'] +" root@robur:/opt/partisan")
        mainobj.create(app_args)

    if action=='extract':
        print ("... extract TAR-file: " + app_args['tarfile'])
        mainobj.extract(app_args['tarfile'])

    print ("READY")

