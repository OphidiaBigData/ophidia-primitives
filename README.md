<h1>Ophidia Primitives</h1>

<h3>Description</h3>
The Ophidia Primitives is set of shared libraries for an Ophidia I/O node. Each function (or <i>primitive</i>) is a low-level operator which work on binary arrays.</br>
Each library can be plugged-in a MySQL instance such as an User-Defined Function. The primitive can be explited within SQL queries like the embedded functions.

<h3>Requirements</h3>
In order to compile and run the Ophidia Primitives, make sure you have the following packages (all available through CentOS official repositories and the epel repository) properly installed:
<ol>
  <li>mysql-community-server</li>
  <li>zlib</li>
  <li>GSL - GNU Scientific Library</li>
  <li>GNU libmatheval</li>
</ol>
<b>Note</b>:</br>
Ophidia Primitive exploits a version of GNU libmatheval that has been patched so that it can be used as an extension of MySQL server.

<h3>How to Install</h3>
The source code has been packaged with GNU Autotools, so look at the INSTALL file or simply type:</br></br>
<code>./configure --prefix=<i>prefix</i></br>
make</br>
make install</br>
cp <i>prefix</i>/liboph_*.so $(mysql_config --plugindir)</br>
mysql -u <i>user</i> -p -H <i>hostname</i> -P <i>port</i> mysql < <i>prefix</i>/etc/create_func.sql</br>
</br></code>
Type:</br></br>
<code>./configure --help</code></br></br>
to see all available options.</br></br>

Further information can be found at <a href="http://ophidia.cmcc.it/documentation">http://ophidia.cmcc.it/documentation</a>.

