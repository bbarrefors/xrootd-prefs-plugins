//
// PrefGeo.cc
//
// Created by Bjorn Barrefors on 6/17/13
//
// 

#include <Python.h>
#include <string.h>
#include <iostream>
#include <dlfcn.h>

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
    // Load the python library with global symbol resolution.
    dlopen(PYTHON_LIB, RTLD_NOW | RTLD_NOLOAD | RTLD_GLOBAL);

    return new PrefGeo(env);
  }
}

long PrefGeo::GetDistance(char * host_hostname, char * client_hostname) {
  XrdSysError *eDest = envinfo->eDest;
  setenv("PYTHONPATH", PATH_PYTHON_SCRIPT, 0);
  eDest->Emsg("PrefGeo", "client host name is:", hostname);
  long distance = 1000000;
  PyObject *pName, *pModule, *pFunc;
  PyObject *pArgs, *pValue;
  Py_Initialize();
  pName = PyString_FromString(IP_PLUGIN);
  pModule = PyImport_Import(pName);
  Py_DECREF(pName);
  
  if(pModule != NULL) {
    pFunc = PyObject_GetAttrString(pModule, IP_LOOKUP);
    
    if (pFunc && PyCallable_Check(pFunc)) {
      pArgs = PyTuple_New(3);
      // Fill in args
      pValue = PyString_FromString(hostname);
      PyTuple_SetItem(pArgs, 0, pValue);
      pValue = PyObject_CallObject(pFunc, pArgs);
      Py_DECREF(pArgs);
      if (pValue != NULL) {
        distance = PyInt_AsLong(pValue);
        Py_DECREF(pValue);
      }
      else {
	eDest->Emsg("PrefGeo", "Call failed");
        Py_DECREF(pFunc);
        Py_DECREF(pModule);
        PyErr_Print();
	return distance;
      }
    }
    else {
      eDest->Emsg("PrefGeo", "Cannot find function");
      if (PyErr_Occurred())
        PyErr_Print();
      return distance;
    }
    Py_XDECREF(pFunc);
    Py_DECREF(pModule);
  }
  else {
    eDest->Emsg("PrefGeo", "Failed to load file");
    PyErr_Print();
    return distance;
  }
  Py_Finalize();
  return distance;
}

int PrefGeo::Pref(XrdCmsReq *, const char *, const char * opaque, XrdCmsPref &pref, XrdCmsPrefNodes& nodes) {
  XrdSysError *eDest = envinfo->eDest;
  eDest->Emsg("PrefGeo", "Preference plugin is PrefGeo");
  // Get the hostname of the client who sends the request
  XrdOucEnv env(opaque);
  char *client_host = env.Get("client_host");
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
	  int distance = GetDistance(node_name, client_host);
	}
    }
  // Set preference mask based on distance
  //pref.SetPreference(1, mask);
  return 0;
}
