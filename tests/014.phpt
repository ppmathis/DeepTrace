--TEST--
Removing interfaces
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php
interface DeepTrace_Test {
	public function DeepTrace_Test_function();
}
dt_remove_interface('DeepTrace_Test');
if(!interface_exists('DeepTrace_Test')) {
	echo 'Removed interface DeepTrace_Test';
} else {
	'Can not remove interface DeepTrace_Test';
}
?>
--EXPECT--
Removed interface DeepTrace_Test