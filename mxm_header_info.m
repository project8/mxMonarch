function result = mxm_header_info()
%MXM_HEADER_INFO gets the header information for the currently open file handle.
%    If a file handle is currently open, mxm_header_info will fetch the header information
%    and return it in a struct.  Otherwise, 'no_file_open' is returned
  result = mxMonarch('get_header');
end
