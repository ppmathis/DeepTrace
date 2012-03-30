--TEST--
Hooking exit/die calls
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php
function exit_handler($msg = null) { echo 'Message: ' . $msg . "\n"; }
dt_set_exit_handler('exit_handler');
exit;
exit();
exit('DeepTrace Exit handler test');
?>
--EXPECT--
Message: 
Message: 
Message: DeepTrace Exit handler test