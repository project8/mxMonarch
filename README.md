mxMonarch
=========

mxMonarch is a set of wrappers around the Monarch file i/o library for Project 8.  The quick 
overview:

* mxMonarch supports both reading from and writing to Monarch files
* mxMonarch uses native MATLAB types to read and write events
* mxMonarch only supports one open file at a time (read below)

To use mxMonarch, you will need the Monarch library built on your system, and you will 
additionally need to have MATLAB running such that your LD_LIBRARY_PATH can find the 
libMonarchCore library.  On my system (OSX Mountain Lion), this means starting MATLAB like this:

> export DYLD_LIBRARY_PATH=<path to your monarch libraries>
> open /Applications/MATLAB_R2012b.app/

A quick example which assumes that you have a file called "test.egg" in your working
directory.  First we build mxMonarch using the build.m function:

(at MATLAB workspace prompt)
> monarch_dir = "/Users/kofron/Code/CPP/monarch" % adjust to your system
> build(monarch_dir) % this should go off without a hitch
> mxm_open('test.egg','ReadMode')
> h = mxm_header_info()
> n = mxm_get_next_event()
> n.data % should be the raw digital data from the event
> mxm_close()

To write data to a new file (currently mxMonarch uses sensible defaults for the digitization
rate and so on, this will be tunable in the future):

> mxm_open('write_test.egg','WriteMode');
> test_data = ones(2,4194304,'uint8');
> mxm_write(test_data)
> mxm_close()
> mxm_open('write_test.egg','ReadMode');
> n = mxm_get_next_event();

and now n.data should be the data you wrote to the file.  mxMonarch uses the following defaults
for now when writing data:

* 250MHz sample rate
* 4MB record size
* two channel acquisition

