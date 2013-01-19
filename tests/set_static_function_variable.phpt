--TEST--
Sets a static function variable
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip";
if(!function_exists("dt_set_static_function_variable")) print "skip";
?>
--FILE--
<?php
	function myFunction() {
		static $counter;
		$counter++;
		echo "Counter: " . $counter . "\n";
	}
	
	myFunction();
	myFunction();
	myFunction();
	dt_set_static_function_variable("myFunction", "counter", 9000);
	myFunction();
?>
--EXPECT--
Counter: 1
Counter: 2
Counter: 3
Counter: 9001