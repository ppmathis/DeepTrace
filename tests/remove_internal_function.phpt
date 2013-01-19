--TEST--
Removing internal functions
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip";
if(!function_exists("dt_remove_function")) print "skip";
?>
--FILE--
<?php
	dt_remove_function("phpinfo");
	echo !function_exists("phpinfo") ? "Success\n" : "Failure\n"
?>
--EXPECT--
Success