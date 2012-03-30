--TEST--
Removing internal constants
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php 
@dt_remove_constant('PHP_SAPI');
if(!defined('PHP_SAPI')) {
	echo 'Removed internal constant.';
} else {
	echo 'Can not remove internal constant.';
}
?>
--EXPECT--
Removed internal constant.