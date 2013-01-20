--TEST--
Adding a class method
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip"; 
if(!function_exists("dt_add_method")) print "skip";
?>
--FILE--
<?php
	class Test {}
	
	dt_add_method('Test', 'sayHello', '', 'echo "Hi!\n";', DT_ACC_STATIC);
	dt_add_method('Test', 'sayAge', '', 'echo "*private*\n";', DT_ACC_PRIVATE);
	dt_add_method('Test', 'sayColor', '', 'echo "Blue\n";', DT_ACC_PROTECTED);
	dt_add_method('Test', 'sayBye', '', 'echo "Bye!\n";', DT_ACC_PUBLIC);
	
	$instance = new Test();
	Test::sayHello();
	echo is_callable(array('Test', 'sayAge')) ? "Yes\n" : "No\n";
	echo is_callable(array('Test', 'sayColor')) ? "Yes\n" : "No\n";
	$instance->sayBye();
?>
--EXPECT--
Hi!
No
No
Bye!