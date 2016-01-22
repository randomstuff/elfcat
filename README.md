# elfcat

Dump sections or program entries from a ELF file.

~~~sh
elfcat ./foo --section-name .text
elfcat ./foo --section-index 3
elfcat ./foo --program-index
~~~

## Installation

### Manual

~~~sh
automake --add-missing && ./configure && make
~~~

### NetBSD

Installation on NetBSD from prebuilt packages:

~~~sh
pkgin install elfcat
~~~

Installation from sources:

~~~sh
cd /usr/pkgsrc/devel/elfcat && make install
~~~
