--TEST--
Removing user functions
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip";
if(!function_exists("dt_remove_function")) print "skip";
?>
--FILE--
<?php
	eval('function myFunction() {
		echo "Yay!\n";
	}');
	
	myFunction();
	dt_remove_function("myFunction");
	
	eval('function myFunction() {
		echo "Still alive.\n";
	}');
	
	myFunction();
	dt_remove_function("myFunction");
	
	if(!function_exists("myFunction")) {
		echo "No more cake...\n";
	}
?>
--EXPECT--
Yay!
Still alive.
No more cake...