--TEST--
Nested exception test with exit exceptions
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip";
if(!function_exists("dt_exit_mode")) print "skip";
if(!function_exists("dt_exit_fetch_exception")) print "skip";
--FILE--
<?php
	function ExitHandler($msg = null) {
		echo "Caught: " . $msg . "\n";
		return false;
	}
	
	dt_exit_mode(DT_EXIT_EXCEPTION, "ExitHandler", "DeepTraceExitException");
	try {
		try {
			try {
				exit("Nested exception test");
			} catch(Exception $e) {	}
		} catch(Exception $e) {	}
	} catch (DeepTraceExitException $e) {
		dt_exit_fetch_exception();
		echo "Caught exception: " . $e->getMessage() . "\n";
		unset($e);
	}
?>
--EXPECT--
Caught: Nested exception test
Caught exception: Nested exception test