AM-language interpreter
==============

<h5>Interpreter of AM-languages (as introduced at <i>TU Dresden</i> , for eductional purposes only)</h5>

<h3>Installation:</h3>
  Requires <i>g++</i>, <i>make</i> and the <a href="http://boost.org"><i>c++ boost library</i></a> to be installed<br><br>
  <b>@<i>Ubuntu</i>:</b>
  1. <code>sudo apt-get install g++ make libboost-all-dev</code>
  2. Clone the repository
  3. Run <code>make</code> in order to compile both interpreters or <br>
  	run <code>make install</code> in order to install both interpreters and enable excecutable support

<h3>Usage:</h3>
  <i>am0</i> refers to the AM0 interpreter and <i>am1</i> refers to the AM1 interpreter<br>
  Run <code>./am0 --help</code> or <code>./am1 --help</code> for more information or<br>
  Try a test run with <code>./am0 -l run.am0</code> or <code>./am1 -l run.am1</code><br>
  
  If you used <code>make install</code>, you can make your AM-code files excecutable:
  1. Add <code>#!/bin/am0</code> or <code>#!/bin/am1</code> as a <a href="http://en.wikipedia.org/wiki/Shebang_(Unix)">shebang</a> to the beginning of your file
  2. Make your code file excecutable with <code>chmod +x FILE</code>
  3. Excecute it with <code>./FILE</code>

<h5>Feel free to report any bugs to nubtaroop@googlemail.com</h5>
