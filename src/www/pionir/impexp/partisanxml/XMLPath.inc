<?php


class XMLPath
{
	var $path;


	// constructor
	function __construct()
	{
		$this->path = array();
	}



	// add new element to path
	function push($element)
	{
		array_push($this->path, $element);
	}



	// remove last element from path and return last element
	function pop()
	{
		return array_pop($this->path);
	}
}


?>
