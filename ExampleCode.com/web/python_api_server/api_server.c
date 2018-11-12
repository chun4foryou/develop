#include <Python.h>
#include <stdlib.h>
#include <sys/time.h>    
#include <sys/resource.h>
#include <pthread.h>
#include <signal.h>

#include "api_server.h"


/**
* @brief Core파일 생성을 위한 ulimit 값 조절
*
* @return 
*/
static int core () {
  struct rlimit rl;


  if (getrlimit (RLIMIT_NOFILE, &rl) == -1){
    return -1;
  }
  rl.rlim_cur = rl.rlim_max;
  setrlimit (RLIMIT_NOFILE, &rl);

  if (getrlimit (RLIMIT_CORE, &rl) == -1){
    return -1;
  }
  rl.rlim_cur = rl.rlim_max;
  setrlimit (RLIMIT_CORE, &rl);
  return 0;
}


/**
* @brief Python을 이용한 웹 서버를 실행 시킨다.
*
* @return 
*/
void* startRmsaIapServer() {
  PyObject *pName, *pModule, *pDict, *pFunc;
  PyObject *result;

  // Set PYTHONPATH TO working directory
  setenv("PYTHONPATH",".",1);
  // Initialize the Python Interpreter
  Py_Initialize();
  // Build the name object
  pName = PyString_FromString((char*)"python_web_server");
  // Load the module object
  pModule = PyImport_Import(pName);
  // pDict is a borrowed reference 
  // python일 잘 못 되었을 경우 여기서 죽음
  pDict = PyModule_GetDict(pModule);

  // pFunc is also a borrowed reference 
  pFunc = PyDict_GetItemString(pDict, (char*)"start_server");

  if (PyCallable_Check(pFunc)) {
    result = PyObject_CallObject(pFunc,NULL);
    fprintf(stderr,"rmsa web is shutdown [%p]\n",result);
    PyErr_Print();
  } else {
    PyErr_Print();
  }

  // Clean up
  Py_DECREF(pModule);
  Py_DECREF(pName);
  Py_DECREF(pDict);
  Py_DECREF(pFunc);
  Py_DECREF(result);

  // Finish the Python Interpreter
  Py_Finalize();
  return 0;
}

int main() {
  int status;
  pthread_t thread_id;

  core();

  if (access(RMSA_WEB_SUTDOWN, R_OK) == 0) {
    unlink(RMSA_WEB_SUTDOWN);
  }

  if (pthread_create(&thread_id, NULL, startRmsaIapServer, NULL) < 0) { 
    fprintf(stderr, "Create Thread Error\n");
  }
  while (1) {
    if (access(RMSA_WEB_SUTDOWN, R_OK) == 0) {
      if ((status = pthread_kill(thread_id, SIGINT)) != 0) {
        fprintf(stderr,"waiting for iap server die\n");
      }
      break;
    }
    sleep(1);
  }
  pthread_join(thread_id, NULL);
  return 0;
}
