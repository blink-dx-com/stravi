<?php

/**
 * functions for date/time; for WORK calculations

 * @package date_work.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 */
class Date_work {
    
    static $HOUR_PER_DAY=8;
    
    
    /**
     * add working days to a date UNIX time stamp -- consider weekend!
     * TBD: support also float days !
     * @param int $date_unx
     * @param int $day
     * @return \DateTime|number
     */
    static public function addWorkingDays($date_unx, $day) {
        
        $newDate = new \DateTime();
        $newDate->setTimestamp($date_unx);
        
        if ($day == 0) {
            return $newDate->getTimestamp();
        }
        
        $i = 1;
        
        while ($i <= abs($day)) {
            
            $newDate->modify(($day > 0 ? ' +' : ' -') . '1 day');
            
            $next_day_number = $newDate->format('N');
            
            if (!in_array($next_day_number, [6, 7])) {
                $i++;
            }
            
        }
        
        return $newDate->getTimestamp();
        
    }
    
}

