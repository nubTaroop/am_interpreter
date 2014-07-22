AM-language interpreter
==============

<h5>Interpreter of AM-languages (as introduced at <i>TU Dresden</i> , for eductional purposes only)</h5>

<h2>Installation:</h2>
  Requires <i>g++</i>, <i>make</i> and the <a href="http://boost.org"><i>c++ boost library</i></a><br>
  <h4>@<i>Ubuntu</i>:</h4>
  1. <code>sudo apt-get install g++ make libboost-all-dev</code>
  2. Clone the repository
  3. Run <code>make</code> in order to compile both interpreters or </b>
  	run <code>make install</code> in order to install both interpreters and enable excecutable support

<h2>Usage:</h2>
  <i>am0</i> refers to the AM0 interpreter and <i>am1</i> refers to the AM1 interpreter</b>
  Run <code>./am0 --help</code> or <code>./am1 --help</code> for more information<br> or</b>
  Try a test run with <code>./am0 -l run.am0</code> or <code>./am1 -l run.am1</code> or </b>
  
  If you used <code>make install</code>, you can make your AM-code files excecutable:</b>
  1. Add <code>#!/bin/am0</code> or <code>#!/bin/am1</code> as a <a href="http://en.wikipedia.org/wiki/Shebang_(Unix)">shebang</a> to the beginning of your file
  2. Make your code file excecutable with <code>chmod +x FILE</code>
  3. Excecute it with <code>./FILE</code>

 <h5>Report bugs to nubtaroop@googlemail.com</h5>
