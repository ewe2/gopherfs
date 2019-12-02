# GopherFS

An  application  that  tries  to  interpret  the  Gopher protocol into a
filesystem.

This is a fork of the repository at git@github.com:ewe2/gopherfs.git

Due to limitations in the gopher standard gopherfs needs to download
the the whole files before they can be accessed. A cache of these  files
is created in ~/.gopherfs or /tmp, if $HOME is not set.

Usage:
	gopherfs gopher://server:port/type/query $mountpoint

Have fun.

