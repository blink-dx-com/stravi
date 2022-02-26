<?php
require_once('sql_query_dyn.inc');
require_once('insert.inc');
require_once ("f.objview.inc");
require_once ("f.assocUpdate.inc");

/**
 * add selected OBJECTS ($tablename) to an ASSOCIATED TABLE ($asoctab) belonging to mother ($mtable) with ID=$id
 * @package glob.obj.assocadd.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param $id 		// ID of $mtable
		 $mtable	// table name of destination object
		 $asoctab   // associated table
		 $asoccol   // column of running number
		 $tablename	// tablename of selected Assoc BOs
		 $selorder  // e.g. x.EXP_ID ; OPTIONAL: order criteria of selected Assoc BOs
		 [$go]    ==   [0] | "1"
 */
class gObj_assocAdd {
    
    function __construct(&$sqlo, $mtable, $id, $asoctab, $asoccol, $childTable) {
        global $error;
        $FUNCNAME= __CLASS__.':init';
        $this->pkarr = primary_keys_get2($asoctab);
        $this->childTable = $childTable;
        $this->asoctab=$asoctab;
        $this->id = $id;
        $this->asoccol = $asoccol;
        $this->mtable=$mtable;
        
        // mother=$mtable
        $childColName=NULL;
        $sqlsel = "COLUMN_NAME, PRIMARY_KEY from CCT_COLUMN where TABLE_NAME=".$sqlo->addQuotes($asoctab).
        ' and CCT_TABLE_NAME='.$sqlo->addQuotes($childTable);
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadRow() ) {
            $colname = $sqlo->RowData[0];
            $pkid    = $sqlo->RowData[1];
            if ($pkid==1) continue; // ignore first primary key; could also be a child of $mtable
            $childColName = $colname;
        }
        if ($childColName==NULL) {
            $error->set( $FUNCNAME, 1, 'childColName not found for table '.$asoctab );
            return;
        }
        
        $infox=array();
        $infox["asoc_pkName"] 	    = PrimNameGet2($asoctab);	// $asocMothID =
        $infox["selbo_pk_name"]     = PrimNameGet2($childTable);
        $infox["selbo_tabNiceName"] = tablename_nice2($childTable);
        $infox["selbo_niceNameCol"] = importantNameGet2($childTable);
        $infox["asoc_childCol"] 	= $childColName;
        $this->infox = $infox;
        
        $runNumber  = 0; // the running number
        $sqls= "select MAX(".$asoccol.") from ".$asoctab." where " . $infox["asoc_pkName"] . "=".$id;
        $sqlo->query("$sqls");
        $sqlo->ReadRow();
        $runNumber = $sqlo->RowData[0];
        if ( $runNumber <= 0 ) $runNumber=0; /* start with 0 */
        $runNumber++;
        $this->runNumber=$runNumber;
        
        $this->add_result=array();
        $this->add_result['cnt']=0;
        $this->add_result['inserted']=0;
        $this->add_result['existSum']=0;
        
        $this->assoclib = new  fAssocUpdate();
        $this->assoclib->setObj( $sqlo, $asoctab, $id );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on Init.' );
            return;
        }
        
        
    }
    
    function getInfox() {
        return $this->infox;
    }
    
    // element exists ?
    function checkElemExists(&$sqlo, $childObjID) {
        $infox=$this->infox;
        $sqlsel = '1 from '.$this->asoctab.' where '.$infox["asoc_pkName"].'='.$this->id.' and '.$infox["asoc_childCol"].'='.$childObjID;
        $sqlo->Quesel($sqlsel);
        $exists=0;
        if ( $sqlo->ReadRow() ) {
            $exists = 1;
        }
        return ($exists);
    }
    
    /**
     * ADD one object
     * @param object $sqlo
     * @param int $childObjID
     * @param int $go
     */
    function add_one(object $sqlo, int $childObjID, $go) {
        global $error;
        //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        // check if element exists
        $action = 'not added';
        $do_insert=0;
        $usedNumber='';
        $errtxt = "";
        
        //$asoctab = $this->asoctab;
        $id = $this->id;
       
        $important_name = importantNameGet2($this->childTable);
        $sua_feat = glob_ObjDataGet( $sqlo, $this->childTable, $childObjID, array($important_name));
        $name_child = $sua_feat[$important_name];
        
        do {
            
            if ($this->mtable==$this->childTable and ($childObjID==$id) ) {
                $action = 'object itself';
                break;
            }
            
            $exists = $this->checkElemExists($sqlo, $childObjID);
            if ($exists) {
                $action = 'already exists';
                break;
            }
            
            $do_insert=1;
            $action = 'add';
            if ( $go>0 ) {
                
                $argu=NULL;
                $argu[$this->infox["asoc_childCol"]]  = $childObjID;
                $argu[$this->asoccol]				  = $this->runNumber;
    
                $this->assoclib->insert( $sqlo, $argu );
                
                if ($error->Got(READONLY))  {
                    $errtxt = '<span style="color:red">error at insert</span> (may be mapping exists)';
                    $error->reset();
                }  else {
                    $action = 'added';
                    $this->add_result['inserted']=$this->add_result['inserted']+1;
                }
                
            }
            
            $usedNumber = $this->runNumber;
            $this->runNumber++;

        } while(0);
        
        if ($exists) $this->add_result['existSum']= $this->add_result['existSum']+1;
        echo $this->asoccol.": ".$usedNumber." <B>".$action."</B> => [ID:".$childObjID."] NAME: $name_child $errtxt <BR>";
        
        $this->add_result['cnt']=$this->add_result['cnt']+1;
  
    }
    
    function add_loop(&$sql, $sql2, $sqlAfter, $go ) {
        global $error;
        // $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        $sqls = "select x.".$this->infox["selbo_pk_name"]." from ". $sqlAfter;
        $sql2->query($sqls);

        while  ( $sql2->ReadRow() ) {  
            $childObjID = $sql2->RowData[0];  
            $this->add_one($sql, $childObjID, $go);    
        }
        
        return $this->add_result;
    }
    
    
}
