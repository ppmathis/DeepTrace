--TEST--
Removing a constant
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip"; 
if(!function_exists("dt_remove_constant")) print "skip";
?>
--FILE--
<?php
	define('DEEPTRACE', 'Before');
	echo DEEPTRACE . "\n";
	dt_remove_constant('DEEPTRACE');
	
	define('DEEPTRACE', 'After');
	echo DEEPTRACE . "\n";
?>
--EXPECT--
Before
After