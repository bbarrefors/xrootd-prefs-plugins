//
// PrefGeo.cc
//
// Created by Bjorn Barrefors on 6/17/13
//
// 

#include <Python.h>
#include <string.h>
#include <iostream>

#include "PrefGeo.hh"
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
    return new PrefGeo(env);
  }
}

char * PrefGeo::GetIP(char * hostname) {
  XrdSysError *eDest = envinfo->eDest;
  setenv("PYTHONPATH", "/home/barrefors/xrootd-prefs-plugins/src", 0);
  //Py_SetProgramName();
  //eDest->Emsg("PrefGeo", "Current working dir is:", get_current_dir_name());
  char * addr = NULL;
  PyObject *pName, *pModule, *pFunc;
  PyObject *pArgs, *pValue;
  Py_Initialize();
  pName = PyString_FromString("GetHostname");
  pModule = PyImport_Import(pName);
  Py_DECREF(pName);
  
  if(pModule != NULL) {
    pFunc = PyObject_GetAttrString(pModule, "GetHostname");
    
    if (pFunc && PyCallable_Check(pFunc)) {
      pArgs = PyTuple_New(1);
      pValue = PyString_FromString(hostname);
      PyTuple_SetItem(pArgs, 0, pValue);
      pValue = PyObject_CallObject(pFunc, pArgs);
      Py_DECREF(pArgs);
      if (pValue != NULL) {
	eDest->Emsg("PrefGeo", "Value was returned");
        addr = PyString_AsString(pValue);
        Py_DECREF(pValue);
      }
      else {
	eDest->Emsg("PrefGeo", "Call failed");
        Py_DECREF(pFunc);
        Py_DECREF(pModule);
        //PyErr_Print();
	//fprintf(stderr, "Call Failed\n");
	return addr;
      }
    }
    else {
      eDest->Emsg("PrefGeo", "Cannot find function");
      //if (PyErr_Occurred())
        //PyErr_Print();
	//fprintf(stderr, "Cannot find function\n");
      return addr;
    }
    Py_XDECREF(pFunc);
    Py_DECREF(pModule);
  }
  else {
    eDest->Emsg("PrefGeo", "Failed to load file");
    //PyErr_Print();
    //fprintf(stderr, "Failed to load file\n");
    return addr;
  }
  Py_Finalize();
  return addr;
}

int PrefGeo::Pref(XrdCmsReq *, const char *, const char * opaque, XrdCmsPref &pref, XrdCmsPrefNodes& nodes) {
  XrdSysError *eDest = envinfo->eDest;
  eDest->Emsg("PrefGeo", "Preference plugin is PrefGeo");
  // Get the hostname of the client who sends the request
  XrdOucEnv env(opaque);
  char *client_host = env.Get("client_host");
  eDest->Emsg("PrefGeo", "client host name is:", client_host);
  
  //Translate client host name to IP address
  char * client_ip = GetIP(client_host);
  eDest->Emsg("PrefGeo", "client IP is:", client_ip);
  
  // Set all prefs to the same first
  const char * node_name = NULL;
  pref.SetPreference(0, -1);  
  //SMask_t mask = 0;
  for (unsigned int i=0; i<XRD_MAX_NODES; i++)
    {
      node_name = nodes.GetNodeName(i);
      if (node_name && *node_name) 
	{
	  eDest->Emsg("PrefGeo", "server node name is:", node_name);
	  // Server node name is in the format [::IP]:PORT
	  // Get distance from python script
	}
    }
  // Set preference mask based on distance
  //pref.SetPreference(1, mask);
  return 0;
}
