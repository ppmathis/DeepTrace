--TEST--
Testing various exit modes (normal, handler, exception)
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
	
	dt_exit_mode(DT_EXIT_HANDLER, "ExitHandler");
	exit("Exit Message #1");
	
	dt_exit_mode(DT_EXIT_EXCEPTION, "ExitHandler", "DeepTraceExitException");
	try {
		exit("Exit Message #2");
	} catch (DeepTraceExitException $e) {
		dt_exit_fetch_exception();
		echo "Caught exception: " . $e->getMessage() . "\n";
		unset($e);
	}

	try {
		exit("Exit Message #3");
	} catch (Exception $e) {
		echo "FAIL: DeepTraceExitException should not be catched by 'Exception'.\n";
	} catch (DeepTraceExitException $e) {
		dt_exit_fetch_exception();
		echo "Caught exception: " . $e->getMessage() . "\n";
		unset($e);
	}
	
	dt_exit_mode(DT_EXIT_NORMAL);
	exit("Exit Message #4\n");
?>
--EXPECT--
Caught: Exit Message #1
Caught: Exit Message #2
Caught exception: Exit Message #2
Caught: Exit Message #3
Caught exception: Exit Message #3
Exit Message #4