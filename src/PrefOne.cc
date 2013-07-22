#include "PrefOne.hh"

#include <string.h>
#include <iostream>

#include "PrefClient.hh"
#include "XrdCms/XrdCmsPref.hh"
#include "XrdCms/XrdCmsPrefNodes.hh"
#include "XrdOuc/XrdOucEnv.hh"
#include "XrdSys/XrdSysDNS.hh"
#include "XrdSys/XrdSysError.hh"
#include "XrdSys/XrdSysLogger.hh"

extern "C" {

XrdCmsXmi *XrdCmsgetXmi(int, char **, XrdCmsXmiEnv * env)
{
  return new PrefOne(env);
}

}

int PrefOne::Pref(XrdCmsReq *, const char *, const char *, XrdCmsPref &pref, XrdCmsPrefNodes& nodes)
{
  XrdSysError *eDest = envinfo-> eDest;
  eDest->Emsg("PrefOne", "setting preference");
  
   const char * node_name;
   pref.SetPreference(0, -1);
   SMask_t mask = 0;
   for (unsigned int i=0; i<XRD_MAX_NODES; i++)
   {
     node_name = nodes.GetNodeName(i);
     eDest->Emsg("PrefOne", "node name is:", node_name);
     if ((node_name) && (strcmp(node_name, "[::127.0.0.1]:1096") == 0))
       {
	 mask |= 1 << i;
       }
   }
   pref.SetPreference(1, mask);
   return 0;
}
