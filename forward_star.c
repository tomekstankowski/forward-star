#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "Python.h"

#define MAX_VERTICES 16
#define MAX_EDGES (MAX_VERTICES * (MAX_VERTICES - 1)) / 2

typedef struct
{
    PyObject_HEAD int pntrs[MAX_VERTICES + 1], end_vertices[MAX_EDGES * 2];
    int v, e;
} ForwardStar;

static PyObject *G6Error;
static PyObject *NoVerticesError;
static PyObject *TooManyVerticesError;

static PyObject *fromString(ForwardStar *self, PyObject *args)
{
    char *text;
    if (!PyArg_ParseTuple(args, "z", &text))
        return NULL;
    const int len = strlen(text);
    char c;
    int i, j, k, l = 0;
    char adjacency_matrix[MAX_VERTICES][MAX_VERTICES];

    if (len == 0)
    {
        PyErr_SetString(G6Error, "too short text");
        return NULL;
    }
    c = text[l++] - 63;
    if (c < 1 || c > 16)
    {
        PyErr_SetString(G6Error, "wrong order");
        return NULL;
    }
    self->v = c;

    for (i = 1, k = 0; i < self->v; i++)
        for (j = 0; j < i; j++)
        {
            if (k == 0)
            {
                if (l == len)
                {
                    PyErr_SetString(G6Error, "too short text");
                    return NULL;
                }
                c = text[l++] - 63;
                if (c < 0 || c > 63)
                {
                    PyErr_SetString(G6Error, "wrong character");
                    return NULL;
                }
                k = 6;
            }
            k--;
            adjacency_matrix[i][j] = adjacency_matrix[j][i] = c & (1 << k);
        }
    if (l < len)
    {
        PyErr_SetString(G6Error, "too long text");
        return NULL;
    }

    for (i = 1, self->e = 0; i < self->v; i++)
        for (j = 0; j < i; j++)
            if (adjacency_matrix[i][j])
                self->e++;

    // now create forward star
    for (i = 0, k = 0; i < self->v; i++)
    {
        self->pntrs[i] = k;
        for (j = 0; j < self->v; j++)
        {
            if (i == j)
                continue;
            if (adjacency_matrix[i][j])
                self->end_vertices[k++] = j;
        }
    }
    // sentinel
    self->pntrs[self->v] = 2 * self->e;
    Py_RETURN_TRUE;
}

static int ForwardStar__init__(ForwardStar *self, PyObject *args)
{
    char *text = NULL;
    if (!PyArg_ParseTuple(args, "|z", &text))
        return -1;
    if (text == NULL)
    {
        self->pntrs[0] = 0;
        // sentinel
        self->pntrs[1] = 0;
        self->v = 1;
        self->e = 0;
    }
    else
    {
        if (fromString(self, args) == NULL)
            return -1;
    }
    return 0;
}

static PyObject *order(const ForwardStar *const self)
{
    return PyLong_FromLong((long)self->v);
}

static PyObject *isEdge(const ForwardStar *const self, PyObject *args)
{
    int i, u, v;
    if (!PyArg_ParseTuple(args, "ii", &u, &v))
        return NULL;
    for (i = self->pntrs[u]; i < self->pntrs[u + 1]; i++)
        if (self->end_vertices[i] == v)
            Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject *addVertex(ForwardStar *const self)
{
    if (self->v == MAX_VERTICES)
    {
        PyErr_SetString(TooManyVerticesError, "too many vertices");
        return NULL;
    }
    self->v++;
    // sentinel
    self->pntrs[self->v] = 2 * self->e;
    Py_RETURN_TRUE;
}

static PyObject *addEdge(ForwardStar *const self, PyObject *args)
{
    int u, v;
    if (!PyArg_ParseTuple(args, "ii", &u, &v))
        return NULL;
    int new_end_vertices[MAX_EDGES * 2];
    int new_pntrs[MAX_VERTICES + 1];

    int v_i, e_i, new_e_i;
    for (v_i = 0, e_i = 0, new_e_i = 0; v_i < self->v; v_i++)
    {
        new_pntrs[v_i] = new_e_i;
        while (e_i < self->pntrs[v_i + 1])
            new_end_vertices[new_e_i++] = self->end_vertices[e_i++];
        if (v_i == u)
            new_end_vertices[new_e_i++] = v;
        if (v_i == v)
            new_end_vertices[new_e_i++] = u;
    }

    self->e++;
    // sentinel
    new_pntrs[self->v] = 2 * self->e;

    memcpy(self->pntrs, new_pntrs, sizeof(int) * (MAX_VERTICES + 1));
    memcpy(self->end_vertices, new_end_vertices, sizeof(int) * MAX_EDGES * 2);
    Py_RETURN_TRUE;
}

static PyObject *deleteVertex(ForwardStar *const self, PyObject *args)
{
    int v;
    if (!PyArg_ParseTuple(args, "i", &v))
        return NULL;
    if (self->v == 1)
    {
        PyErr_SetString(NoVerticesError, "graph must have vertices");
        return NULL;
    }
    const int v_deg = self->pntrs[v + 1] - self->pntrs[v];
    const int new_e = self->e - v_deg;
    const int new_v = self->v - 1;
    int new_end_vertices[MAX_EDGES * 2];
    int new_pntrs[MAX_VERTICES + 1];

    int v_i, new_v_i, e_i, new_e_i;
    for (v_i = 0, new_v_i = 0, e_i = 0, new_e_i = 0; v_i < self->v; v_i++)
    {
        if (v_i == v)
            continue;
        new_pntrs[new_v_i++] = new_e_i;
        for (; e_i < self->pntrs[v_i + 1]; e_i++)
            if (self->end_vertices[e_i] != v)
                new_end_vertices[new_e_i++] = self->end_vertices[e_i];
    }
    // sentinel
    new_pntrs[new_v] = 2 * new_e;

    self->v = new_v;
    self->e = new_e;
    memcpy(self->pntrs, new_pntrs, sizeof(int) * (MAX_VERTICES + 1));
    memcpy(self->end_vertices, new_end_vertices, sizeof(int) * MAX_EDGES * 2);
    Py_RETURN_TRUE;
}

static PyObject *deleteEdge(ForwardStar *const self, PyObject *args)
{
    int u, v;
    if (!PyArg_ParseTuple(args, "ii", &u, &v))
        return NULL;
    if(isEdge(self, args) == Py_False)
        Py_RETURN_TRUE;
    int new_end_vertices[MAX_EDGES * 2];
    int new_pntrs[MAX_VERTICES + 1];
    int v_i, e_i, new_e_i;
    for (v_i = 0, e_i = 0, new_e_i = 0; v_i < self->v; v_i++)
    {
        new_pntrs[v_i] = new_e_i;
        for (; e_i < self->pntrs[v_i + 1]; e_i++)
            if (!(v_i == u && self->end_vertices[e_i] == v) && !(v_i == v && self->end_vertices[e_i] == u))
                new_end_vertices[new_e_i++] = self->end_vertices[e_i];
    }
    // sentinel
    new_pntrs[self->v] = ((self->e - 1) * 2);
    self->e--;
    memcpy(self->pntrs, new_pntrs, sizeof(int) * (MAX_VERTICES + 1));
    memcpy(self->end_vertices, new_end_vertices, sizeof(int) * MAX_EDGES * 2);
    Py_RETURN_TRUE;
}

static PyObject *__eq__(const ForwardStar *const self, const ForwardStar *const other)
{
    if (self->v != other->v)
        Py_RETURN_FALSE;
    if (self->e != other->e)
        Py_RETURN_FALSE;
    if (memcmp(self->pntrs, other->pntrs, sizeof(int) * (self->v + 1)) != 0)
        Py_RETURN_FALSE;
    if (memcmp(self->end_vertices, other->end_vertices, sizeof(int) * self->e) != 0)
        Py_RETURN_FALSE;
    Py_RETURN_TRUE;
}

static PyObject *__ne__(const ForwardStar *const self, const ForwardStar *const other)
{
    if (self->v != other->v)
        Py_RETURN_TRUE;
    if (self->e != other->e)
        Py_RETURN_TRUE;
    if (memcmp(self->pntrs, other->pntrs, sizeof(int) * (self->v + 1)) != 0)
        Py_RETURN_TRUE;
    if (memcmp(self->end_vertices, other->end_vertices, sizeof(int) * self->e) != 0)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject *__str__(const ForwardStar *const self)
{
    int i, j, k, l;
    char c;
    int adjacency_matrix[MAX_VERTICES][MAX_VERTICES];
    for (i = 0; i < self->v; i++)
        for (j = 0; j < self->v; j++)
            adjacency_matrix[i][j] = 0;
    for (i = 0; i < self->v; i++)
        for (j = self->pntrs[i]; j < self->pntrs[i + 1]; j++)
            adjacency_matrix[i][self->end_vertices[j]] = 1;

    char str[25];
    str[0] = self->v + 63;
    for (i = 1, k = 5, l = 1, c = 0; i < self->v; i++)
        for (j = 0; j < i; j++)
        {
            if (adjacency_matrix[i][j])
                c |= (1 << k);
            if (k == 0)
            {
                str[l++] = c + 63;
                k = 6;
                c = 0;
            }
            k--;
        }
    if (k != 5)
        str[l++] = c + 63;
    str[l] = '\0';
    return PyUnicode_FromFormat("%s", str);
}

static PyMethodDef ForwardStarMethods[] = {
    {"order", (PyCFunction)order, METH_NOARGS, "Zwraca liczbę wierzchołków grafu."},
    {"addVertex", (PyCFunction)addVertex, METH_NOARGS, "Dodaje do grafu nowy izolowany wierzchołek."},
    {"deleteVertex", (PyCFunction)deleteVertex, METH_VARARGS, "Usuwa z grafu wskazany wierzchołek."},
    {"isEdge", (PyCFunction)isEdge, METH_VARARGS, "Zwraca informację o tym, czy podane wierzchołki sąsiadują z sobą."},
    {"addEdge", (PyCFunction)addEdge, METH_VARARGS, "Dodaje podaną krawędź."},
    {"deleteEdge", (PyCFunction)deleteEdge, METH_VARARGS, "Usuwa podaną krawędź."},
    {"fromString", (PyCFunction)fromString, METH_VARARGS, "Przekształca reprezentację tekstową grafu w graf."},
    {"__str__", (PyCFunction)__str__, METH_NOARGS, "Przekształca graf w reprezentację tekstową."},
    {"__eq__", (PyCFunction)__eq__, METH_O, "Test równości dwóch reprezentacji grafów."},
    {"__ne__", (PyCFunction)__ne__, METH_O, "Test różności dwóch reprezentacji grafów."},
    {NULL}};

static PyTypeObject ForwardStarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "ForwardStar.ForwardStar",
    .tp_doc = "graph representation as forward stars",
    .tp_basicsize = sizeof(ForwardStar),
    .tp_itemsize = 0,
    .tp_str = (reprfunc)__str__,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)ForwardStar__init__,
    .tp_methods = ForwardStarMethods,
};

static PyModuleDef simple_graphs_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "simple_graphs",
    .m_doc = "Implementation of simple_graphs using forward stars.",
    .m_size = -1,
};

PyMODINIT_FUNC PyInit_simplegraphs(void)
{
    ForwardStarType.tp_dict = PyDict_New();
    if (!ForwardStarType.tp_dict)
        return NULL;
    G6Error = PyErr_NewException("ForwardStar.G6Error", PyExc_Exception, NULL);
    Py_INCREF(G6Error);
    PyDict_SetItemString(ForwardStarType.tp_dict,
                         "G6Error", G6Error);

    TooManyVerticesError = PyErr_NewException("ForwardStar.TooManyVerticesError", PyExc_Exception, NULL);
    Py_INCREF(TooManyVerticesError);
    PyDict_SetItemString(ForwardStarType.tp_dict,
                         "TooManyVerticesError", TooManyVerticesError);

    NoVerticesError = PyErr_NewException("ForwardStar.NoVerticesError", NULL, NULL);
    Py_INCREF(NoVerticesError);
    PyDict_SetItemString(ForwardStarType.tp_dict,
                         "NoVerticesError", NoVerticesError);
    if (PyType_Ready(&ForwardStarType) < 0)
        return NULL;
    PyObject *m = PyModule_Create(&simple_graphs_module);
    if (m == NULL)
        return NULL;
    Py_INCREF(&ForwardStarType);
    PyModule_AddObject(m, "ForwardStar",
                       (PyObject *)&ForwardStarType);

    return m;
}