--TEST--
Removing user-defined constants
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php 
define('DEEPTRACE_TEST_CONSTANT', 'test');
dt_remove_constant('DEEPTRACE_TEST_CONSTANT');
if(!defined('DEEPTRACE_TEST_CONSTANT')) {
	echo 'Removed user-defined constant.';
} else {
	echo 'Can not remove user-defined constant.';
}
?>
--EXPECT--
Removed user-defined constant.