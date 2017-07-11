#ifndef IvrSipDialog_h
#define IvrSipDialog_h

// Python stuff
#include "structmember.h"
#include <Python.h>

extern PyTypeObject IvrSipDialogType;

class AmSipDialog;
PyObject* IvrSipDialog_FromPtr(AmSipDialog* dlg);

#endif
