--TEST--
Remove a include and load it with require_once() again
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip"; 
if(!function_exists("dt_remove_include")) print "skip";
?>
--FILE--
<?php
	require_once('_include.php');
	dt_remove_include('_include.php');
	require_once('_include.php');
?>
--EXPECT--
Included
Included