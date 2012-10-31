function [] = build(monarch_dir) 
  % This path is hardcoded right now which is dumb, but 
  % it'll work for now.
  incdir = strcat('-I',monarch_dir,'/Include');
  libdir = strcat('-L',monarch_dir,'/Install/lib');
  mex('mxMonarch.cpp',incdir,libdir,'-lMonarchCore')
end
