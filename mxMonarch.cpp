/*
 * mxMonarch.cpp
 * Written by Jared Kofron <jared.kofron@gmail.com>
 * Project day 0 was 10/25/2012.
 * mxMonarch is a wrapper for the Monarch library for Project 8.  It 
 * allows Monarch files to be accessed via MATLAB functions transparently,
 * although it has some distinct limitations owing to how MATLAB interacts 
 * with C++.  In particular, only one Monarch file can be open at a time -
 * however, the user does not have to keep track of this (mxMonarch will 
 * tell you if you are trying to do something it doesn't like).  The API
 * is as similar to the Monarch library as possible, although the user
 * does not need to keep track of pointers or references of any kind.  
 */

#include "Monarch.hpp"
#include "mex.h"
#include "matrix.h"

/*
 * A static file handle pointer and a bool to indicate an existing, open
 * file handle.
 */
static const Monarch* global_handle;
static bool file_is_open;
static const MonarchRecord* handle_data_ptr;

/* 
 * a type which defines success or failure of a function.
 */
typedef enum {
  mxm_success = 0,
  mxm_failure = 1
} mxm_success_t;


/* 
 * an enumeration over possible things to do with a monarch file.
 */
typedef enum {
  mxm_open_k = 1,
  mxm_close_k = 2,
  mxm_status_k = 3,
  mxm_next_k = 4,
  mxm_header_k = 5
} mxm_action_t;

/*
 * do_t is a struct that encodes all of the information mxMonarch needs to
 * perform an action.
 */
typedef struct {
  mxm_action_t act;
  std::string filename;
  AccessModeType file_mode;
} mxm_do_t;

/*
 * populate_open_action parses the input arguments, interpreting them as
 * instructions to open a file.
 */
mxm_success_t populate_open_action(int narg, const mxArray *args[], 
				   mxm_do_t *to_do) {
  mexPrintf("populating action for opening.\n");
  mxm_success_t result = mxm_success;
  to_do->act = mxm_open_k;
 
  // There should be two more strings - the filename and the access mode.
  for(std::size_t arg = 1; arg < 3; arg++) {
    if(mxGetClassID(args[arg]) == mxCHAR_CLASS) {
      std::size_t slen = mxGetN(args[arg]);
      char work_str[slen];

      // Try to parse the string.  If it doesn't work, die.  Otherwise...
      if( mxGetString(args[arg],work_str,slen + 1) != 0 ) {
	mexPrintf("error: couldn't parse arg#%d. mxMonarch stopped.\n",arg);
	result = mxm_failure;
      }
      else {
	// if it's arg 1, this is the filename
	if(arg == 1) {
	  (to_do->filename).assign(work_str);
	}

	// otherwise it's access mode
	if(arg == 2) {
	  if(strncmp(work_str, "r", slen) == 0) {
	    to_do->file_mode = sReadMode;
	  }
	  else if(strncmp(work_str, "w", slen) == 0) {
	    to_do->file_mode = sWriteMode;
	  }
	  else {
	    mexPrintf("error: access mode must be ReadMode or WriteMode!\n");
	  }
	}
      }
    }
    else {
      mexPrintf("error: mxMonarch only speaks strings.\n");
      result = mxm_failure;
    }
  }
  return result;
}

/*
 * input_to_do_t converts the input arguments into a mxm_do_t struct which
 * is the internal format which is used for dispatching.
 */
mxm_success_t input_to_do_t(int narg, const mxArray *args[], mxm_do_t *to_do) {
  mexPrintf("parsing input arguments.\n");
  mxm_success_t result = mxm_success;

  if(mxGetClassID(args[0]) == mxCHAR_CLASS) {
    std::size_t slen = mxGetN(args[0]);
    char work_str[slen];

    // Try to parse the string.  If it doesn't work, die.  Otherwise...
    if( mxGetString(args[0],work_str,slen + 1) != 0 ) {
      mexPrintf("string munging failed. mxMonarch cannot proceed.\n");
      result = mxm_failure;
    }
    else if( strncmp(work_str, "open", slen + 1) == 0 ) {
      // OK try to parse the rest of the arguments to generate a fully-formed
      // 'open' action.  This means that we got both the "open" keyword and a
      // filename.
      result = populate_open_action(narg, args, to_do);
    }
    else if( strncmp(work_str, "close", slen + 1) == 0 ) {
      to_do->act = mxm_close_k;
    }
    else if( strncmp(work_str, "hndl_stat", slen + 1) == 0 ) {
      to_do->act = mxm_status_k;
    }
    else if( strncmp(work_str, "next_event", slen + 1) == 0 ) {
      to_do->act = mxm_next_k;
    }
    else if( strncmp(work_str, "get_header", slen + 1) == 0 ) {
      to_do->act = mxm_header_k;
    }
  }
  else {
    mexPrintf("error: mxMonarch only speaks strings.\n");
    result = mxm_failure;
  }

  return result;
}

/*
 * After the arguments have been verified and parsed, we're ready to actually
 * do what's been asked.  do_mxm_open is the function that is ultimately called
 * when a monarch file is to be opened.  The function is successful if no file
 * is currently open and if the file which is specified can be opened 
 * successfully.  Conversely, if a file is open or the file cannot be opened,
 * the function returns a failure value.
 */
mxm_success_t do_mxm_open(int nout, mxArray *out[], const mxm_do_t *todo) {
  mxm_success_t result;
  // First check the static is_open value
  if(file_is_open) {
    mexPrintf("error: file is already open!\n");
    result = mxm_failure;
  }
  // If file_is_open is false, proceed.
  else {
    if(todo->file_mode == sReadMode) {
      global_handle = Monarch::OpenForReading(todo->filename);
      if(global_handle == NULL) {
	mexPrintf("error: couldn't open file for reading!\n");
      }
      else if(global_handle->ReadHeader()) {
	file_is_open = true;
	mexPrintf("file is open.\n");
      }
      else {
	mexPrintf("error: opening file failed.\n");
	result = mxm_failure;
      }
    }
  }

  // All done.
  return result;
}

/*
 * do_mxm_close closes the open file handle, if it is indeed open.  otherwise
 * it is a harmless no-op.
 */
mxm_success_t do_mxm_close(int nout, mxArray *out[], const mxm_do_t *todo) {
  if(file_is_open && global_handle->Close() ) {
    file_is_open = false;
  }

  // All done, no matter what.
  return mxm_success;
}

/*
 * do_mxm_header gets the file header if the file handle is open and returns the
 * data as a MATLAB struct.  if no file is open you get no_file_open instead and
 * a warning message.
 */
mxm_success_t do_mxm_header(int nout, mxArray *out[], const mxm_do_t *todo) {
  // The result, which is mxm_success if the file is open and everything goes 
  // well in terms of memory allocation.  Otherwise, it is mxm_failure.
  mxm_success_t result = mxm_success;

  // struct dimensions and such
  int ndims = 2;
  int dimary[2] = {1,1};

  // field data for the header
  int nfields = 5;
  const char *fields[] = {"filename", 
			  "acquisition_rate",
			  "acquisition_mode",
			  "record_size",
			  "acquisition_time"};

  if( file_is_open ) {
    mexPrintf("grabbing header\n");
    nout = 1;
    out[0] = mxCreateStructArray(ndims,dimary,nfields,fields);
    // Now populate the struct data.
    
    // First the filename
    std::string filename = (global_handle->GetHeader())->GetFilename();
    mxArray *fname = mxCreateString(filename.c_str());
    mxSetField(out[0],0,"filename",fname);

    // Now grab acquisition rate
    double acq_rate = (global_handle->GetHeader())->GetAcqRate();
    mxArray *arate = mxCreateDoubleScalar(acq_rate);
    mxSetField(out[0],0,"acquisition_rate",arate);

    // Acquisition mode.  here we turn the mode into a string
    std::string acq_mode;
    if( global_handle->GetHeader()->GetAcqMode() == sOneChannel ) {
      acq_mode.assign("OneChannel");
    }
    else if( global_handle->GetHeader()->GetAcqMode() == sTwoChannel ) {
      acq_mode.assign("TwoChannel");
    }
    mxArray *amode = mxCreateString(acq_mode.c_str());
    mxSetField(out[0],0,"acquisition_mode",amode);

    // Record size.  This is an integer number of bytes.  This code is also
    // BEYOND GOOFY but somehow it works.  Get it together, MathWorks.
    uint64_T rec_size = global_handle->GetHeader()->GetRecordSize();
    int dims[2] = {1,1};
    mxArray *rsize = mxCreateNumericArray(2,dims,mxUINT64_CLASS,mxREAL);
    *((uint64_T*)mxGetData(rsize)) = rec_size;
    mxSetField(out[0],0,"record_size",rsize);

    // Acquisition time.  
    uint64_T acq_time = global_handle->GetHeader()->GetAcqTime();
    mxArray *atime = mxCreateNumericArray(2,dims,mxUINT64_CLASS,mxREAL);
    *((uint64_T*)mxGetData(atime)) = acq_time;
    mxSetField(out[0],0,"acquisition_time",atime);
  }
  else {
    mexPrintf("error: no file open!\n");
    nout = 1;
    out[0] = mxCreateString("no_file_open");
    result = mxm_failure;
  }

  // All done
  return result;
}

/*
 * do_mxm_status returns the present file handle status.
 */
mxm_success_t do_mxm_status(int nout, mxArray *out[], const mxm_do_t *todo) {
  // The result of the operation.  This is true if the memory could be allocated
  // and everything worked out OK.  Otherwise it returns mxm_failure.
  mxm_success_t result = mxm_success;
  
  // We return only one value.
  nout = 1;
  if( (out[0] = mxCreateLogicalScalar(file_is_open)) == NULL ) {
    mexPrintf("error in do_mxm_status: couldn't allocate memory!\n");
    result = mxm_failure;
  }

  // All done, no matter what.
  return mxm_success;
}

/*
 * do_mxm_next grabs the next event and returns it as a struct.  If no file is open,
 * instead you get a string 'no_open_file' and a warning message.  If the struct could
 * be created and returned, mxm_success.  Otherwise, mxm_failure.
 */
mxm_success_t do_mxm_next(int nout, mxArray *out[], const mxm_do_t *todo) {
  // Result
  mxm_success_t result = mxm_success;

  // The field names
  int ardims[2] = {1,1};
  int nfields = 4;
  const char *fields[] = {"acquisition_id",
			  "record_id",
			  "timestamp",
			  "data"};

  if(file_is_open) {
    nout = 1;
    if( global_handle->ReadRecord() == true ) {
      out[0] = mxCreateStructArray(2,ardims,nfields,fields);
      //FIXME need to populate fields.
    }
    // Otherwise we are EOF
    else {
      out[0] = mxCreateString("eof");
    }
  }

  // handle isn't open
  else {
    mexPrintf("error: no handle open to get next event from.  did you mxm_open?\n");
    nout = 1;
    out[0] = mxCreateString("no_open_file");
    result = mxm_failure;
  }

  // All done, no matter what.
  return mxm_success;
}


/*
 * dispatch takes the parsed mxm_do_t structure and performs the task which
 * has been specified by the user (or fails).
 */
mxm_success_t dispatch(int nout, mxArray *out[], const mxm_do_t *todo) {
  mxm_success_t result = mxm_success;

  // Check what we're supposed to do.
  if(todo->act == mxm_open_k) do_mxm_open(nout, out, todo);
  else if(todo->act == mxm_close_k) do_mxm_close(nout, out, todo);
  else if(todo->act == mxm_status_k) do_mxm_status(nout, out, todo);
  else if(todo->act == mxm_next_k) do_mxm_next(nout, out, todo);
  else if(todo->act == mxm_header_k) do_mxm_header(nout, out, todo);

  // all done
  return result;
}

/*
 * mexFunction is the MATLAB entry point.  It inspects the input arguments
 * and chooses the right thing to do.  It is essentially a gateway to the
 * rest of the code.
 */
void mexFunction(int nargout, mxArray *argout[], 
		 int nargin, const mxArray *argin[]) {
  // The structure which will ultimately be populated using the input
  // mxArray
  mxm_do_t todo;

  // If the number of arguments is zero, barf.
  if(nargin == 0) {
    mexPrintf("error: Bad call to mxMonarch - no arguments provided.\n");
    return;
  }
  // Otherwise, start the dispatch process.  Parse the input mxArray into a
  // do struct.  If this is successful, then we can ship that struct off to
  // methods which accomplish the desired result.
  else {
    if(input_to_do_t(nargin,argin,&todo) == mxm_success) {
      dispatch(nargout, argout, &todo);
    }
  }

  // return, we're done
  return;
}
