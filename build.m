function [] = build() 
  % This path is hardcoded right now which is dumb, but 
  % it'll work for now.
  mex('mxMonarch.cpp','-I../../CPP/monarch/Include/','-L../../CPP/monarch/Install/lib','-lMonarchCore')
end
