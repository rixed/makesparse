MakeSparse
==========

Simple command to 'compress' files on the filesystem by skipping over blocs
of zeros, thus creating [sparse file](http://en.wikipedia.org/wiki/Sparse_file)
out of regular file.

This was done to copy arround `vmdk` files but could be usefull in other
situations as well.

Usage
-----

`makesparse output` will read stdin and write it into the given output file,
creating a hole each time a block of more than 256 zeroes are found.

Not much to say.

