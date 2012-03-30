--TEST--
Renaming functions
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php
function custom_phpinfo() { echo "Successfully overwritten phpinfo()."; }
dt_rename_function('phpinfo', 'internal_phpinfo');
dt_rename_function('custom_phpinfo', 'phpinfo');
phpinfo();
?>
--EXPECT--
Successfully overwritten phpinfo().