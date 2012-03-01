--TEST--
Check for DeepTrace functions
--SKIPIF--
<?php if (!extension_loaded("DeepTrace")) print "Extension not loaded"; ?>
<?php if (!function_exists("dt_rename_function")) print "Function dt_rename_function() not available"; ?>
<?php if (!function_exists("dt_remove_function")) print "Function dt_remove_function() not available"; ?>
<?php if (!function_exists("dt_remove_constant")) print "Function dt_remove_constant() not available"; ?>
<?php if (!function_exists("dt_remove_include")) print "Function dt_remove_include() not available"; ?>
<?php if (!function_exists("dt_remove_class")) print "Function dt_remove_class() not available"; ?>
<?php if (!function_exists("dt_set_proctitle")) print "Function dt_set_proctitle() not available"; ?>
<?php if (!function_exists("dt_set_exit_handler")) print "Function dt_set_exit_handler() not available"; ?>
<?php if (!function_exists("dt_throw_exit_exception")) print "Function dt_throw_exit_exception() not available"; ?>
--FILE--
<?php 
echo "DeepTrace functions are available";
?>
--EXPECT--
DeepTrace functions are available
