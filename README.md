# DeepTrace v2.0
### Copyright (c) 2012 - 2013 Pascal Mathis (pmathis@snapserv.net), Yussuf Khalil (dev@pp3345.net)
* * *
#### License information
The code is licensed via the Apache License 2.0: http://www.apache.org/licenses/LICENSE-2.0

Means:

* You may alter the code, but have to give the changes back
* You may not use this work for commercial purposes
* You must attribute the work in the manner specified by the author or licensor

If you would like to use this code commercially, please contact us via pmathis@snapserv.net. Thank you!

#### What is DeepTrace?
DeepTrace is a small and neat extension for different low level things in PHP. You can:

* Catch calls to exit() or die()
* Remove, rename and redefine constants, classes, functions and methods
* Alter the values of static variables inside functions and methods
* Set the title of the PHP process
* Switch between HTML and text-only output of phpinfo()
* Remove includes, making them includable again via include_once()

#### How to install
Just run a few easy steps. First, download the source and unpack it somewhere. After that, run the following commands:

*phpize*

*./configure*

*sudo make*

*sudo make install*

Now add the following entry into your php.ini:

*zend_extension=DeepTrace.so*

Got a problem? If you can't find a solution on the world wide web, don't hesitate to contact us. :-)

**Important:**
DeepTrace is a zend extension, so do **not** use *extension=DeepTrace.so*!

#### Documentation?!? I can't find anything! HELP!
No problem. There are lots of tests in the tests/ directory. The tests show you many examples for using DeepTrace.

#### I want to include that library in my project
That is not a problem for us, but there are a few restrictions:

* If you change some code, you MUST give the chances back
* We would be glad if you inform us
* You must not remove any copyrights or license information

If you want to include the library in a compiled form, ask us via pmathis@snapserv.net - We won't bite, we'd just like to know who is using DeepTrace :)

#### There is something missing!
In case you have a great idea for a new feature in DeepTrace, tell us about it and we'll see what we can do. :)

#### Real examples
[Pancake - Fast webserver written in PHP and C](https://github.com/pp3345/Pancake "2012 - 2013 Yussuf Khalil")

You did something great with DeepTrace? Tell us about it and maybe it will be shown here :-)
