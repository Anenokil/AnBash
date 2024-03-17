<h1> Shell emulator </h1>
<h2> Description </h2>
The program accepts shell commands from the standard input stream and executes them, handling errors.<br>
Algorithm:
<ol>
  <li>
    The program reads commands of any length in parts using a buffer.
  </li>
  <li>
    Parses the input data and forms an array of tokens from them.
  </li>
  <li>
    Performs syntax validation. If the syntax is correct, go to step 4. Otherwise, it reports an error and returns to step 1.
  </li>
  <li>
    Builds a command tree based on the array.
  </li>
  <li>
    Executes commands from the tree, handling errors.
  </li>
  <li>
    Goes to step 1 while waiting for the next command.
  </li>
</ol>

The program processes the following special sequences: < > >> | || && & ; ( ) " ' \\ # $SHELL $HOME $USER $EUID

<h2> Modules </h2>
It consists of 3 main modules:
<ul>
  <li>
    <u>shellexec</u> (see below)
  </li>
  <li>
    <u>parse</u> (see below)
  </li>
  <li>
    <u>main</u>
  </li>
</ul>

It also uses following auxiliary modules:
<ul>
  <li>
    <u>colors</u> (to learn more about it, see the module README.md)
  </li>
  <li>
    <u>strarr</u> (to learn more about it, see the module README.md)
  </li>
  <li>
    <u>strarr_counter</u> (to learn more about it, see the module README.md)
  </li>
  <li>
    <u>shelltree</u> (to learn more about it, see the module README.md)
  </li>
</ul>

<h3>shellexec</h3>
`int shell_exec(ShTree *tree, int bg_pp, void (*emerg)(void));`<br>
The function executes shell commands of tree and returns exit status;
'bg_pp' is a pipe for background process to remove zombies;
'emerg' is a function that is called in son after fork if execution is failed.<br>

<h3>parse</h3>
`char ** parse(char **strarr);`<br>
The function parses strarr for shell.<br>
`ShTree * st_build(char **arr);`<br>
The function creates and returns ShTree by parsed array.<br>
