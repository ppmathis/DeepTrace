--TEST--
Changing process title
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php
if(dt_set_proctitle('DeepTrace_Test')) {
	echo 'Changed process title.';
} else {
	echo 'Can not change process title.';
}
?>
--EXPECT--
Changed process title.