--TEST--
Bypassing constant cache
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip"; 
if(!function_exists("dt_remove_constant")) print "skip";
?>
--FILE--
<?php
	error_reporting(E_ALL &~ E_NOTICE);

	function ABC() {
		var_dump(XY_Z);
	}
	 
	define('XY_Z', '123');
	var_dump(XY_Z);
	ABC();
	dt_remove_constant('XY_Z');
	var_dump(XY_Z);
	
	define('XY_Z', '456');
	var_dump(XY_Z);
	ABC();
	ABC();
	dt_remove_constant('XY_Z');
	 
	var_dump(XY_Z);
	define('XY_Z', '789');
	var_dump(XY_Z);
	ABC();
	ABC();
?>
--EXPECT--
string(3) "123"
string(3) "123"
string(4) "XY_Z"
string(3) "456"
string(3) "456"
string(3) "456"
string(4) "XY_Z"
string(3) "789"
string(3) "789"
string(3) "789"