<?php

require_once("db_access.inc");
require_once("globals.inc");


/**
 * PROJ tree analysis
 * @package o.PROJ.tree.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oPROJ_tree {

    #   void  projtree__tree2array(obj sql, int top_proj_id) 
    static function &tree2array(  
    			&$sql,    
    			$top_proj_id,
    			$maxDepth = 0
    			) {
    # descr: creates an array( proj_id_1 => array (sub_proj_id_1 => "sub_proj_1.name", ...), sub_proj_id_1 ==> array(), ...)
    #        (sub_proj_id_1 is an example, for a project which has no subprojects)
    #        this function is non recursive
    # input: $sql         ... instance of subclass of CDBAbstract (database access class)
    #        $top_proj_id ... id of the project of which the subtree should be computed
    #		 $maxDepth    ... [optional] max analysis depth of project structure
    # return: reference to the array described above (so call the function: $res = &projtree__tree2array(...);) 
    #		  so the result array is not copied but referenced
    
        $error     = & ErrorHandler::get();
        $proj_arr  = array();
        $to_do_arr = array($top_proj_id);
        $depth     = 0;
    	
        do {
    		$doSaveSubs = 1; // save next sub-projects in $to_do_arr
    		if ( $maxDepth AND ($depth>=$maxDepth) ) $doSaveSubs=0;
    		
            $proj_id_now = array_pop($to_do_arr);
    		if ($proj_id_now=="NULL") {
    			$sqlis = "is ";
    		} else $sqlis = "= ";
    		
            $proj_arr[$proj_id_now] = array();
            $sql->query("SELECT name, proj_id FROM proj where pro_proj_id ".$sqlis.$proj_id_now." ORDER BY name");
            if ($error->got(READONLY)) return;
            while($sql->ReadRow()) {
                $proj_name = $sql->RowData[0];
                $proj_id   = $sql->RowData[1];
                $proj_arr[$proj_id_now][$proj_id] = $proj_name;
                if (!array_key_exists($proj_id, $proj_arr) AND $doSaveSubs)
                    array_push($to_do_arr, $proj_id);
            }
    		$depth++;
    		
    		
        } while(count($to_do_arr));
    
        //   print_r_pre($proj_arr);
        return $proj_arr;
    }
    
    #   void  projtree__tree_with_leafs2array(obj sql, int top_proj_id) 
    static function &tree_with_leafs2array(  &$sql,    $top_proj_id) {
    # descr: creates an array of the tree structure including projects and business objects:
    #  array( $top_proj_id => array ( "projects" => array ( sub_proj_id_1 => "sub_proj_1.name",
    #                                                       sub_proj_id_2 => "sub_proj_2.name", 
    #                                                       ... ),
    #                                 "elements" => array ( table_1_name => array ( bo_id_1,
    #                                                                               bo_id_2,
    #                                                                               ... ),
    #                                                       table_2_name => array (...),
    #                                                       ... ),
    #                               )
    #         sub_proj_id_id => array (...)
    #      )
    # tablenames are written in upper case (as stored in database)
    # if a project does not contain projects or elements then "projects" resp. "elements" points to an empty array.
    #        this function is non recursive
    # input: $sql         ... instance of subclass of CDBAbstract (database access class)
    #        $top_proj_id ... id of the project of which the subtree should be computed
    # return: reference to the array described above (so call the function: $res = &tree2array(...);) 
    #		  so the result array is not copied but referenced
    
        $error     = & ErrorHandler::get();
        $proj_arr  = array();
        $to_do_arr = array($top_proj_id);
        
        do {
            $proj_id_now = array_pop($to_do_arr);
            $proj_arr[$proj_id_now] = array();
            $proj_arr[$proj_id_now]["projects"] = array();
            $proj_arr[$proj_id_now]["elements"] = array();
            $sql->query("SELECT name, proj_id FROM proj WHERE pro_proj_id = $proj_id_now ORDER BY name");
            if ($error->got(READONLY)) return;
            while($sql->ReadRow()) {
                $proj_name = $sql->RowData[0];
                $proj_id   = $sql->RowData[1];
                $proj_arr[$proj_id_now]["projects"][$proj_id] = $proj_name;
                if (!array_key_exists($proj_id, $proj_arr)) // encountered project is not yet described
                    array_push($to_do_arr, $proj_id);
            }
            $sql->query("SELECT table_name, prim_key FROM proj_has_elem WHERE proj_id = $proj_id_now ORDER BY table_name");
            if ($error->got(READONLY)) return;
            while($sql->ReadRow()) {
                $table_name  = $sql->RowData[0];
                $prim_key_id = $sql->RowData[1];
                if (!array_key_exists($table_name, $proj_arr[$proj_id_now]["elements"]))
                    $proj_arr[$proj_id_now]["elements"][$table_name] = array();
                $proj_arr[$proj_id_now]["elements"][$table_name][] = $prim_key_id;
            }
        } while(count($to_do_arr));
    
        //   print_r_pre($proj_arr);
        return $proj_arr;
    }
    
    
    #   void  tree_with_leafs_and_info2array(obj sql, int top_proj_id) 
    static  function &tree_with_leafs_and_info2array(  
    	&$sql, 
    	$top_proj_id, 
    	$seltable=NULL  // if not null: select only this table-type
    	) {
    # descr: creates an array of the tree structure including projects and business objects:
    #  array( $top_proj_id => array ( "projects" => array ( sub_proj_id_1 => array ( "name" => sub_proj_1.name,
    #                                                                                "reading_allowed" => 0/1,
    #                                                                                "cct_access_id" => cct_access_id
    #                                                                                "extra_obj_id"  => extra_obj_id
    #                                                                              )
    #                                                       sub_proj_id_2 => array ( ... ), 
    #                                                       ... ),
    #                                 "elements" => array ( table_1_name => array ( bo_id_1 => array ( "name" => most_imp_col,
    #                                                                                                  "cct_access_id" => cct_access_id,
    #                                                                                                  "deactivated" => deactivated
    #                                                                                                 )
    #                                                                               bo_id_2 => array ( ... ),
    #                                                                               ... ),
    #                                                       table_2_name => array (...),
    #                                                       ... ),
    #                               )
    #         sub_proj_id_id => array (...)
    #      )
    # tablenames are written in upper case (as stored in database)
    # if a project does not contain projects or elements then "projects" resp. "elements" points to an empty array.
    #        this function is non recursive
    # input: $sql         ... instance of subclass of CDBAbstract (database access class)
    #        $top_proj_id ... id of the project of which the subtree should be computed
    # return: reference to the array described above (so call the function: $res = &tree2array(...);) so the result array is not copied but referenced
    # errors:
    #        db_access 2 error in query
    
        $error     = & ErrorHandler::get();
        $proj_arr  = array();
        $to_do_arr = array($top_proj_id);
    	
    	$sqlSelTable = NULL;
    	if ($seltable!="") $sqlSelTable=" AND TABLE_NAME='".$seltable."'";
        
        do {
            $proj_id_now = array_pop($to_do_arr);
            $proj_arr[$proj_id_now] = array();
            $proj_arr[$proj_id_now]["projects"] = array();
            $proj_arr[$proj_id_now]["elements"] = array();
            $sql->query('SELECT name, proj_id, extra_obj_id, cct_access_id FROM proj WHERE pro_proj_id = '.$proj_id_now.' ORDER BY name');
            if ($error->got(READONLY)) return;
            while($sql->ReadRow()) {
                $proj_name  = $sql->RowData[0];
                $proj_id    = $sql->RowData[1];
                $proj_eo_id = $sql->RowData[2];
                $proj_acc_id= $sql->RowData[3];
                $proj_arr[$proj_id_now]['projects'][$proj_id] = array('name' => $proj_name,
                                                                      'cct_access_id' => $proj_acc_id,
                                                                      'extra_obj_id' => $proj_eo_id);
                if (!array_key_exists($proj_id, $proj_arr)) // encountered project is not yet described with its subprojects
                    array_push($to_do_arr, $proj_id);
            }
    		
    		// TBD: sort by object-name, if $sqlSelTable is set !!!
            $sql->query('SELECT table_name, prim_key, deactivated, elem_order '.
                        'FROM proj_has_elem WHERE proj_id='.$proj_id_now . $sqlSelTable.' ORDER BY table_name, elem_order');
            if ($error->got(READONLY)) return;
            while($sql->ReadRow()) {
                $table_name  = $sql->RowData[0];
                $prim_key_id = $sql->RowData[1];
                $deactivated = $sql->RowData[2];
                if (!array_key_exists($table_name, $proj_arr[$proj_id_now]["elements"]))
                    $proj_arr[$proj_id_now]["elements"][$table_name] = array();
                $proj_arr[$proj_id_now]["elements"][$table_name][$prim_key_id] = array("deactivated" => $deactivated);
            }
        } while(count($to_do_arr));
    
        reset ($proj_arr);
        foreach( $proj_arr as $proj_id=>$proj_data) {
            foreach( $proj_data["projects"] as $sub_proj_id=>$dummy) { // other info on projects (done here because we have only one $sql)
                $proj_arr[$proj_id]["projects"][$sub_proj_id]["reading_allowed"] = access_reading_allowed($sql, "PROJ", $sub_proj_id);
            }
            foreach( $proj_data["elements"] as $table_name=>$prim_ids) { // get other info on elements in projects
                foreach( $prim_ids as $curr_prim_id=>$dummy) {
                    $sel_columns = importantNameGet2($table_name);
                    if ($sel_columns) 
                        $sel_columns .= " mip, ";
                    if (cct_access_has2($table_name))
                        $sel_columns .= "cct_access_id cai";
                    if ($sel_columns) { // anything to select?
                        $prim_col = PrimNameGet2($table_name);
                        $sql->query("SELECT $sel_columns FROM $table_name WHERE $prim_col = $curr_prim_id");
                        if ($error->got(READONLY)) return;
                        if ($sql->ReadArray()) {
                            if (array_key_exists ("MIP", $sql->RowData))
                                $proj_arr[$proj_id]["elements"][$table_name][$curr_prim_id]["name"] = $sql->RowData["MIP"];
                            if (array_key_exists ("CAI", $sql->RowData))
                                $proj_arr[$proj_id]["elements"][$table_name][$curr_prim_id]["cct_access_id"] = $sql->RowData["CAI"];
                        }
                    }
                }
            }
        }
        //   print_r_pre($proj_arr);
        return $proj_arr;
    }

}
