--TEST--
Sets a static method variable
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip";
if(!function_exists("dt_set_static_method_variable")) print "skip";
?>
--FILE--
<?php
	class Test {
		static function myFunction() {
			static $counter;
			$counter++;
			echo "Counter: " . $counter . "\n";
		}
	}
	
	Test::myFunction();
	Test::myFunction();
	Test::myFunction();
	dt_set_static_method_variable('Test', 'myFunction', 'counter', 9000);
	Test::myFunction();
?>
--EXPECT--
Counter: 1
Counter: 2
Counter: 3
Counter: 9001