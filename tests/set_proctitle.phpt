--TEST--
Changing process title (proctitle)
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip";
if(!function_exists("dt_set_proctitle")) print "skip";
?>
--FILE--
<?php
echo dt_set_proctitle("DeepTrace") ? "success" : "failure"; 
?>
--EXPECT--
success