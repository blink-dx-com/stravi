<?php
/**
 * manage the news blog
 * @package f.news.inc
 * @swreq: SREQ:0001097: g > show development news blog 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 */
class gNewsGui {
	
	/**
	 * show the news blog
	 * @param $newsArr array of  <pre>
		$version =>  ( $version = '2.030 2009-05-07' )
	 * 	array( pos=>array(
	 * 		'k'=> KEY,    : 'New', 'Fix', 'Improved'
	 *      't'=> TABLENAME - reflected table
	 * 		'm'=> message string
	 * 		[OPT] 'h'=>array( array(helptxt,url) ) // contains help-links in ROBO-help
	 *		[OPT] 'proj' => CCT project-ID
	 * 		)
	 * 	 )
	 * </pre>
	 * @return -
	 */
	function showAll( &$newsArr ) {
		
		
		foreach( $newsArr as $versStr=>$oneVersInfo) {
		    
			$versInfo = explode(' ',$versStr);
			echo '<b>version '.$versInfo[0].'</B> ('.$versInfo[1].') <ul>'."\n";
			
			foreach( $oneVersInfo as $oneEntry) {
			    
			    $t_nice = '';
				$projTxt =  $oneEntry['proj']!=NULL  ? 'Project-ID:'.$oneEntry['proj'] .': ' : NULL;
				
				if ($oneEntry['t']) {
				    $t_nice = ' ('.tablename_nice2($oneEntry['t']) .') ';
				}
				
				echo '<li>'.$oneEntry['k'].': '.$t_nice .  $projTxt . $oneEntry['m'];
				
				if ( !empty($oneEntry['h']) ) {
					$helpPoi = &$oneEntry['h'];
					$helpHtml = '; see help: ';
					$komma='';
					foreach( $helpPoi as $oneHelpArr) {
						$helpHtml .= $komma."<a href=\"help/robo/".$oneHelpArr[1]."\" target=help>".$oneHelpArr[0]."</a>";
						$komma=', ';
					}
					
					echo $helpHtml;
				}
				echo '</li>'."\n";
				
			}
			
			echo '</ul>'."\n";
			echo '<br>'."\n";
		}
		
	}
}
