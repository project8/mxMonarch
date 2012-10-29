function result = mxm_is_open( )
%MXM_IS_OPEN Check if mxMonarch is currently associated with a file.
%   Returns true or false - either mxMonarch currently has an open file 
%   handle or it does not.
    result = mxMonarch('hndl_stat');
end

