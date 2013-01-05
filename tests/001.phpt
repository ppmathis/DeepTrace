--TEST--
Checking for DeepTrace presence
--INI--
zend_extension=modules/DeepTrace.so
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
--FILE--
<?php 
echo "DeepTrace extension is available";
?>
--EXPECT--
PHP Warning:  Module 'DeepTrace' already loaded in Unknown on line 0

Warning: Module 'DeepTrace' already loaded in Unknown on line 0
DeepTrace extension is available