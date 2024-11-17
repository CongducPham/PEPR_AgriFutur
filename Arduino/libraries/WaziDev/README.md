WaziDev Library
===============

This library allows to pilot the [WaziDev](http://www.waziup.io/documentation/wazidev/).

Install
-------
This library is compatible with Arduino IDE > 1.8.10. Check your version!

Download the [zip file](https://github.com/Waziup/wazidev-lib/archive/master.zip).
In your Arduino IDE select `sketch / Include Library / Add .ZIP library` and select the zip file downloaded.
You're done!

Usage
-----


Development
-----------

You can use the Makefile included by installing Arduino Makefile: https://github.com/sudar/Arduino-Makefile/
You can then use the traditional `make` commands:
```
make
sudo make upload
```

You can monitor the Serial port of your WaziDev this way:
```
sudo make monitor
```


It is also possible to recompile automatically each time you change a file with `entr`:
```
ls | entr make
```
or:
```
ls | entr -s 'sudo make upload && sudo make monitor'
```


Copyright
---------

Copyright Waziup 2019.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
