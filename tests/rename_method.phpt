--TEST--
Renaming a class method
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip"; 
if(!function_exists("dt_rename_method")) print "skip";
?>
--FILE--
<?php
	class Test {
		function sayHi() {
			echo "Hello World!\n";
		}
		function sayBye() {
			echo "Goodbye World!\n";
		}
	}
	
	$instance = new Test();
	$instance->sayHi();
	$instance->sayBye();
	dt_rename_method('Test', 'sayHi', '_sayHi');
	dt_rename_method('Test', 'sayBye', 'sayHi');
	dt_rename_method('Test', '_sayHi', 'sayBye');
	$instance->sayHi();
	$instance->sayBye();
?>
--EXPECT--
Hello World!
Goodbye World!
Goodbye World!
Hello World!