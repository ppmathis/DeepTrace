--TEST--
Toggling phpinfo() mode
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php 
@dt_show_plain_info(false);
ob_start();
phpinfo();
$result = ob_get_clean();
if(!strpos($result, '<html>')) {
	echo 'Toggled phpinfo() mode.';
} else {
	echo 'Can not toggle phpinfo() mode.';
}
?>
--EXPECT--
Toggled phpinfo() mode.