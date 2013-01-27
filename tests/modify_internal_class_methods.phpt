--TEST--
Modifying internal class methods
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip"; 
if(!function_exists("dt_add_method")) print "skip";
if(!function_exists("dt_rename_method")) print "skip";
if(!function_exists("dt_remove_method")) print "skip";
?>
--FILE--
<?php
	error_reporting(E_ALL &~ E_NOTICE);
	dt_rename_method('\Exception', 'getTrace', 'getTrace_');
	dt_add_method('\Exception', 'getTrace', '', <<<'FUNCTIONBODY'
return $this->getTrace_();
FUNCTIONBODY
	);
	
	dt_rename_method('\Exception', 'getTraceAsString', 'getTraceAsString_');
	dt_add_method('\Exception', 'getTraceAsString', '', <<<'FUNCTIONBODY'
return $this->getTraceAsString_();
FUNCTIONBODY
	);
	dt_remove_method('\Exception', 'getTraceAsString');
	
	echo 'Does method \Exception::getTrace exist: ' .
		(method_exists('\Exception', 'getTrace') ? "Yes\n" : "No\n");
	echo 'Does method \Exception::getTrace_ exist: ' .
		(method_exists('\Exception', 'getTrace_') ? "Yes\n" : "No\n");
	echo 'Does method \Exception::getTraceAsString exist: ' .
		(method_exists('\Exception', 'getTraceAsString') ? "Yes\n" : "No\n");
	echo 'Does method \Exception::getTraceAsString_ exist: ' .
		(method_exists('\Exception', 'getTraceAsString_') ? "Yes\n" : "No\n");
?>
--EXPECT--
Does method \Exception::getTrace exist: Yes
Does method \Exception::getTrace_ exist: Yes
Does method \Exception::getTraceAsString exist: No
Does method \Exception::getTraceAsString_ exist: Yes