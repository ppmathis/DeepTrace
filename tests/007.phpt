--TEST--
Removing include
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php
ob_start();
include('001.phpt');
ob_end_clean();
dt_remove_include('001.phpt');
if(count(get_included_files()) == 1) {
	echo 'Removed include.';
} else {
	echo 'Can not remove include.';
}
?>
--EXPECT--
Removed include.