--TEST--
Renaming user functions
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip";
if(!function_exists("dt_rename_function")) print "skip";
?>
--FILE--
<?php
	function __phpinfo() {
		echo "Hello World\n";
	}
	
	dt_rename_function("__phpinfo", "_phpinfo");
	_phpinfo();
?>
--EXPECT--
Hello World