--TEST--
Removing a class
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip"; 
if(!function_exists("dt_remove_class")) print "skip";
if(!function_exists("dt_destroy_class_data")) print "skip";
?>
--FILE--
<?php
	class Test { }
	dt_destroy_class_data("Test");
	dt_remove_class("Test");
	
	class Test { }
	dt_destroy_class_data("Test");
	dt_remove_class("Test");
	
	echo !class_exists("Test") ? "Success\n" : "Failure\n";
?>
--EXPECT--
Success