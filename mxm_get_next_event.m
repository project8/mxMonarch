function event_data = mxm_get_next_event()
%MXM_GET_NEXT_EVENT Grab the next event out of the file. 
%    mxm_get_next_event either returns the next event as a MATLAB struct with all of the
%    event data as fields.
  event_data = mxMonarch('next_event');
end
