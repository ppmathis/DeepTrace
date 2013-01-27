--TEST--
Removing a class constant
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip"; 
if(!function_exists("dt_remove_constant")) print "skip";
if(!function_exists("dt_remove_class")) print "skip";
if(!function_exists("dt_destroy_class_data")) print "skip";
?>
--FILE--
<?php
	class Test {
		const DeepTrace = 'Hello';
	}
	echo Test::DeepTrace . "\n";
	
	dt_destroy_class_data('Test');
	dt_remove_class('Test');
	
	class Test {
		const DeepTrace = 'World';
	}
	echo Test::DeepTrace . "\n";
?>
--EXPECT--
Hello
World