#include <Python.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
} ForwardStarObject;

static PyTypeObject ForwardStarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "simplegraphs.ForwardStar",
    .tp_doc = "Forward star graph representation",
    .tp_basicsize = sizeof(ForwardStarObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
};

static PyModuleDef simplegraphsmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "simplegraphs",
    .m_doc = "Module containing graph representations.",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_simplegraphs(void)
{
    PyObject *m;
    if (PyType_Ready(&ForwardStarType) < 0)
        return NULL;

    m = PyModule_Create(&simplegraphsmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&ForwardStarType);
    PyModule_AddObject(m, "ForwardStar", (PyObject *) &ForwardStarType);
    return m;
}