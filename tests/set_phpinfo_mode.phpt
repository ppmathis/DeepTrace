--TEST--
Setting phpinfo mode
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip";
if(!function_exists("dt_phpinfo_mode")) print "skip";
?>
--FILE--
<?php
function detectMode() {
	ob_start();
	phpinfo();
	$content = ob_get_contents();
	ob_end_clean();
	echo (strpos($content, "<br />") != 0) ? "html" : "text";
	echo "\n";
}

dt_phpinfo_mode(DT_PHPINFO_TEXT);
detectMode();
dt_phpinfo_mode(DT_PHPINFO_HTML);
detectMode();
?>
--EXPECT--
text
html