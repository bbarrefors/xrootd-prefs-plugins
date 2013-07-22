
#include <iostream>
#include "XrdCms/XrdCmsXmi.hh"

// A simple, silly plugin for testing preferences
// This prefers the server in slot one, but will accept any others.

class PrefOne : public XrdCmsXmi {
public:
  
  virtual int  Chmod (      XrdCmsReq      *,
			    mode_t          ,
			    const char           *,
			    const char           *) {return -1;}
  
  virtual int  Mkdir (      XrdCmsReq      *,
			    mode_t          ,
			    const char           *,
			    const char           *) {return -1;}
  
  virtual int Mkpath(      XrdCmsReq       *,
			   mode_t          ,
			   const char           *,
			   const char           *) {return -1;}
  
  virtual int  Prep  (const char           *,
		      int             ,
		      const char           *,
		      const char           *) {return -1;}
  
  virtual int  Rename(      XrdCmsReq      *,
			    const char           *,
			    const char           *,
			    const char           *,
			    const char           *) {return -1;}
  
  virtual int  Remdir(      XrdCmsReq      *,
			    const char           *,
			    const char           *) {return -1;}
  
  virtual int  Remove(      XrdCmsReq      *,
			    const char           *,
			    const char           *) {return -1;}
  
  virtual int  Select(      XrdCmsReq      *, // See description above
			    int             ,
			    const char           *,
			    const char           *) {return -1;}
  
  virtual int  Stat  (      XrdCmsReq      *,
			    const char           *,
			    const char           *) {return -1;}
  
  virtual int  Pref  (      XrdCmsReq      *,
			    const char           *,
			    const char           *,
			    XrdCmsPref      &pref,
			    XrdCmsPrefNodes      &
			    );
  
  virtual void XeqMode(unsigned int &isNormal,
		       unsigned int &isDirect) // We only implement the "Pref" command.
  {isNormal = XMI_ALL-XMI_PREF; isDirect = XMI_ALL;}

  PrefOne(XrdCmsXmiEnv* env) : envinfo(env) {}
  ~PrefOne() {}
  
private:
  
  XrdCmsXmiEnv * envinfo;
  
};
