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
#include <sstream>

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

long PrefGeo::GetDistance(const char * host_hostname, char * client_hostname) {
  XrdSysError *eDest = envinfo->eDest;
  setenv("PYTHONPATH", PATH_PYTHON_SCRIPT, 0);
  long distance = 100000;
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
      pValue = PyString_FromString(host_hostname);
      PyTuple_SetItem(pArgs, 0, pValue);
      pValue = PyString_FromString(client_hostname);
      PyTuple_SetItem(pArgs, 1, pValue);
      pValue = PyString_FromString(PATH_PYGEOIP_DATABASE);
      PyTuple_SetItem(pArgs, 2, pValue);
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
  SMask_t mask[MAX_PREF_LEVELS] = {0,0,0,0};
  char *client_host = env.Get("client_host");
  // Set all prefs to the same first
  eDest->Emsg("PrefGeo", "Client node name is:", client_host);
  const char * node_name = NULL;
  pref.SetPreference(0, -1);  
  //SMask_t mask = 0;
  long distance[XRD_MAX_NODES];
  int num_hosts = 0;
  for (unsigned int i=0; i<XRD_MAX_NODES; i++)
    {
      node_name = nodes.GetNodeName(i);
      if (node_name && *node_name) 
	{
	  num_hosts++;
	  eDest->Emsg("PrefGeo", "Server node name is:", node_name);
	  // Server node name is in the format [::IP]:PORT
	  // Get distance from python script
	  distance[i] = GetDistance(node_name, client_host);
	  long dist_tmp = distance[i];
	  std::stringstream ss;
	  ss << dist_tmp;
	  const char * dist_str = NULL;
	  dist_str = ss.str().c_str();
	  eDest->Emsg("PrefGeo", "Distance between nodes:", dist_str);
	}
    }
  int hosts[XRD_MAX_NODES];
  for (int i = 0; i<num_hosts;i++) {
    hosts[i] = i;
  }
  // Sort distance array, shortest first
  // Use insertion sort because it's a fast in place algorithm on small inputs
    for (int j=1;j<num_hosts;j++) {
      long key = distance[j];
      int host_key = hosts[j];
      int i = j-1;
      while (i>=0 && distance[i]>key) {
	distance[i+1] = distance[i];
	hosts[i+1] = hosts[i];
	i = i-1;
      }
      distance[i+1] = key;
      hosts[i+1] = host_key;
    }
    // Set preference mask based on distance
    int j=0;
    while (j<num_hosts && j<MAX_PREF_LEVELS) {
      mask[j] |= 1 << hosts[j];
      j++;
    }
    while (j<num_hosts) {
      mask[j] |= 1 << hosts[j];
      j++;
    }

    // Set the preference mask at different levels
    for (int i = 0; i < MAX_PREF_LEVELS; i++) {
        pref.SetPreference(i, mask[i]);
    }

  // Give highest prio to lowest distance, if more than priority levels just give the rest lowest priority
  return 0;
}
