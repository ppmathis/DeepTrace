--TEST--
Removing functions
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php
dt_remove_function('phpinfo');
if(!function_exists('phpinfo')) {
	echo "Removed function phpinfo.";
} else {
	echo "Can not remove function phpinfo.";
}
?>
--EXPECT--
Removed function phpinfo.