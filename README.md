AM-language interpreter
==============

Interpreter of AM-languages (as introduced at <i>TU Dresden</i> , for eductional purposes only)

<h2>Installation:</h2>
  Requires <i>g++</i>, <i>make</i> and the <a href="http://boost.org"><i>c++ boost library</i></a><br>
  <h4>@<i>Ubuntu</i>:</h4>
  1. <code>sudo apt-get install g++ make libboost-all-dev</code>
  2. clone the repository
  3. run <code>make</code> in order to compile both interpreters or </b>
  	run <code>make install</code> in order to install both interpreters and enable excecutable support

<h2>Usage:</h2>
  run <code>./am0 --help</code>/<code>./am1 --help</code> for more information<br> or make a test run
  with <code>./am0 -l run.am0</code>/<code>./am1 -l run.am1</code> or </b>
  if you used <code>make install</code> the "run.am0/run.am1" file excecutable and run it
  <b>Running the interpreter under non-unix-like systems, can give some output errors!</b>

 <h1>Report bugs to nubtaroop@googlemail.com</h1>
