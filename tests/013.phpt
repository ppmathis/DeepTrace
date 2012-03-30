--TEST--
Removing classes
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php
class DeepTrace_Test {
	function DeepTrace_Test_function() { }
}
dt_remove_class('DeepTrace_Test');
if(!class_exists('DeepTrace_Test')) {
	echo 'Removed class DeepTrace_Test';
} else {
	'Can not remove class DeepTrace_Test';
}
?>
--EXPECT--
Removed class DeepTrace_Test