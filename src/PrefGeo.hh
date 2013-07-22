//
// PrefGeo.hh
//
// Created by Bjorn Barrefors on 6/17/13
// 
//

#ifndef ____PrefGeo__
#define ____PrefGeo__

#include <iostream>

#include "XrdCms/XrdCmsXmi.hh"

// This plugin set preferences based on geo location of client IP vs host IP

class PrefGeo : public XrdCmsXmi {
public:
  
  virtual int Chmod(XrdCmsReq  *,
		    mode_t      ,
		    const char *,
		    const char * ) {return -1;}

  virtual int Mkdir(XrdCmsReq  *,
		    mode_t      ,
		    const char *,
		    const char * ) {return -1;}

  virtual int Mkpath(XrdCmsReq  *,
		     mode_t      ,
		     const char *,
		     const char * ) {return -1;}

  virtual int Prep(const char *,
		   int         ,
		   const char *,
		   const char * ) {return -1;}
  
  virtual int Rename(XrdCmsReq  *,
		     const char *,
		     const char *,
		     const char *,
		     const char * ) {return -1;}
  
  virtual int Remdir(XrdCmsReq  *,
		     const char *,
		     const char * ) {return -1;}
  
  virtual int Remove(XrdCmsReq  *,
		     const char *,
		     const char * ) {return -1;}
  
  virtual int Select(XrdCmsReq  *,
		     int         ,
		     const char *,
		     const char * ) {return -1;}
  
  virtual int Stat(XrdCmsReq  *,
		   const char *,
		   const char * ) {return -1;}
  
  virtual int Pref(XrdCmsReq       *,
		   const char      *,
		   const char      *,
		   XrdCmsPref      &pref,
		   XrdCmsPrefNodes &);
  
  virtual void XeqMode(unsigned int &isNormal,
		       unsigned int &isDirect) // We only implement the "Pref" command.
  {isNormal = XMI_ALL-XMI_PREF; isDirect = XMI_ALL;}

  PrefGeo(XrdCmsXmiEnv* env) : envinfo(env) {}
  ~PrefGeo() {}

  char * GetIP(char * hostname);
  
private:
  
  XrdCmsXmiEnv * envinfo;
  
};

#endif /* defined(____PrefGeo__) */
