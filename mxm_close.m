function result = mxm_close( )
%MXM_CLOSE Close the monarch file currently associated with mxMonarch.
%   This function will close the currently open handle associated with 
%   mxMonarch.  If no handle is currently open, nothing happens.  If for
%   some reason closing the handle fails, warning messages will happen
%   but nothing catastrophic should occur.

    % call the mxMonarch gateway with appropriate arguments to close the
    % currently open handle.
    mxMonarch('close');
    
    % all done
    return;
end

