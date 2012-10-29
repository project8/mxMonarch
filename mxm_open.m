function result = mxm_open( filename, mode )
%MXM_OPEN Open a monarch file for reading or writing.  
%   If a file is already
%   open, the call will succeed but *no new file will be opened*, and the
%   return value will be the string 'handle_unavailable'.  If the call is a
%   success, the string 'ok' will be returned.  If the file does not exist
%   on disk, 'no_file' will be returned.
    switch(mode)
        case 'ReadMode'
            mxMonarch('open',filename,'r')
        case 'WriteMode'
            mxMonarch('open',filename,'w')
    end

    % all done
    return;
end

