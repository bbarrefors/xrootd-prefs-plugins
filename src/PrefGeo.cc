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

#include "XrdCms/XrdCmsPref.hh"
#include "XrdCms/XrdCmsPrefNodes.hh"
#include "XrdOuc/XrdOucEnv.hh"
#include "XrdSys/XrdSysDNS.hh"
#include "XrdSys/XrdSysError.hh"
#include "XrdSys/XrdSysLogger.hh"

XrdSysMutex *PrefGeo::mtx = new XrdSysMutex();

extern "C" {

  XrdCmsXmi *XrdCmsgetXmi(int, char **, XrdCmsXmiEnv * env)
  {
    // Load the python library with global symbol resolution.
    dlopen(PYTHON_LIB, RTLD_NOW | RTLD_NOLOAD | RTLD_GLOBAL);

    return new PrefGeo(env);
  }
}

long PrefGeo::GetDistance(const char * host_hostname, char * client_hostname, XrdSysMutexHelper * mtxhlpr) {
  XrdSysError *eDest = envinfo->eDest;
  setenv("PYTHONPATH", PATH_PYTHON_SCRIPT, 0);
  long distance = 100000;
  PyObject *pName, *pModule, *pFunc;
  PyObject *pArgs, *pValue;
  mtxhlpr->Lock(PrefGeo::mtx);
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
	// Can check for python version if needed, need to uncomment
	//const char * version = Py_GetVersion();
	//char short_ver[10];
	//strncpy(short_ver, version, 1);
	//short_ver[1] = '\0';
	//char ver_2[] = "2";
	//char ver_3[] = "3";
	//if (strcmp(short_ver,ver_2) == 0) {
	//  distance = PyLong_AsLong(pValue);
	//}
	//else if (strcmp(short_ver,ver_3) == 0) {
	distance = PyLong_AsLong(pValue);
	//}
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
  mtxhlpr->UnLock();
  return distance;
}

int PrefGeo::Pref(XrdCmsReq *, const char *, const char * opaque, XrdCmsPref &pref, XrdCmsPrefNodes &nodes) {
  XrdSysError *eDest = envinfo->eDest;
  XrdSysMutexHelper *mtxhlpr = new XrdSysMutexHelper(PrefGeo::mtx);
  eDest->Emsg("PrefGeo", "Preference plugin is PrefGeo");
  // Get the hostname of the client who sends the request
  XrdOucEnv env(opaque);
  SMask_t mask[MAX_PREF_LEVELS] = {-1,0,0,0};
  char *client_host = env.Get("client_host");
  // Set all prefs to the same first
  eDest->Emsg("PrefGeo", "Client node name is:", client_host);
  const char * node_name = NULL;
  pref.SetPreference(0, -1);  
  //SMask_t mask = 0;
  long distance[XRD_MAX_NODES];
  int num_hosts = 0;
  int hosts[XRD_MAX_NODES];
  for (unsigned int i=0; i<XRD_MAX_NODES; i++)
    {
      node_name = nodes.GetNodeName(i);
      if (node_name && *node_name) 
	{
	  hosts[i] = i;
	  num_hosts++;
	  eDest->Emsg("PrefGeo", "Server node name is:", node_name);
	  // Server node name is in the format [::IP]:PORT
	  // Get distance from python script
	  distance[i] = GetDistance(node_name, client_host, mtxhlpr);
	  long dist_tmp = distance[i];
	  std::stringstream ss;
	  ss << dist_tmp;
	  const char * dist_str = NULL;
	  dist_str = ss.str().c_str();
	  eDest->Emsg("PrefGeo", "Distance between nodes:", dist_str);
	}
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
    int j=MAX_PREF_LEVELS-1;
    int i = 0;
    while (i<num_hosts && j>0) {
      mask[j] |= 1 << hosts[i];
      j--;
      i++;
    }

    // Set the preference mask at different levels
    for (int i = 0; i < MAX_PREF_LEVELS; i++) {
        pref.SetPreference(i, mask[i]);
    }

    delete mtxhlpr;
    mtxhlpr = NULL;
  // Give highest prio to lowest distance, if more than priority levels just give the rest lowest priority
  return 0;
}
