--TEST--
Removing traits
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php
trait DeepTrace_Test {
	public function DeepTrace_Test_function() {}
}
dt_remove_class('DeepTrace_Test');
if(!trait_exists('DeepTrace_Test')) {
	echo 'Removed trait DeepTrace_Test';
} else {
	'Can not remove trait DeepTrace_Test';
}
?>
--EXPECT--
Removed trait DeepTrace_Test