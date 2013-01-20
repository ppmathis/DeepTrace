--TEST--
Removing a class method
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip"; 
if(!function_exists("dt_remove_method")) print "skip";
?>
--FILE--
<?php
	class Test {
		function sayHi() {
			echo "Hello World :)\n";
		}
	}
	
	$instance = new Test();
	echo method_exists('Test', 'sayHi') ? "Yes\n" : "No\n";
	dt_remove_method('Test', 'sayHi');
	echo method_exists('Test', 'sayHi') ? "Yes\n" : "No\n";
?>
--EXPECT--
Yes
No