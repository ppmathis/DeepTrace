--TEST--
Checking for DeepTrace presence
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php 
echo "DeepTrace extension is available";
?>
--EXPECT--
DeepTrace extension is available