#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION // to avoid a warning
#include <numpy/arrayobject.h>
#include "gate_helpers.h"

static PyObject *wrap_calc_wind_count(PyObject *self, PyObject *args) {
    double point_x, point_y;
    int vert_count;
    PyObject *poly_vertices;

    // parse the input args tuple
    if (!PyArg_ParseTuple(args, "ddiO!", &point_x, &point_y, &vert_count, &PyArray_Type, &poly_vertices)) {
        return NULL;
    }

    PyArrayObject *poly_vert_array = (PyArrayObject *) PyArray_FROM_OTF(poly_vertices, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
    if (!poly_vert_array) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to convert poly_vertices to NumPy array");
        return NULL;
    }
    double *poly_vertices_c = (double *) PyArray_DATA(poly_vert_array);

    // now we can call our function!
    int wind_count = calc_wind_count(point_x, point_y, vert_count, poly_vertices_c);

    Py_DECREF(poly_vert_array);

    return Py_BuildValue("i", wind_count);
}

static PyObject *wrap_points_in_polygon(PyObject *self, PyObject *args) {
    PyObject *poly_vertices;
    PyObject *points;
    int vert_count;
    int point_count;

    // parse the input args tuple
    if (!PyArg_ParseTuple(args, "OiOi", &poly_vertices, &vert_count, &points, &point_count)) {
        return NULL;
    }

    PyArrayObject *poly_vert_array = (PyArrayObject *) PyArray_FROM_OTF(poly_vertices, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
    if (!poly_vert_array) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to convert poly_vertices to NumPy array");
        return NULL;
    }
    double *poly_vertices_c = (double *) PyArray_DATA(poly_vert_array);

    PyArrayObject *points_array = (PyArrayObject *) PyArray_FROM_OTF(points, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
    if (!points_array) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to convert points to NumPy array");
        return NULL;
    }
    double *points_c = (double *) PyArray_DATA(points_array);

    // now we can call our function!
    int *is_in_polygon = malloc(point_count * sizeof(int));

    points_in_polygon(is_in_polygon, poly_vertices_c, vert_count, points_c, point_count);

    Py_DECREF(poly_vert_array);
    Py_DECREF(points_array);

    npy_intp dims[1];
    dims[0] = point_count;

    PyObject *arr = PyArray_SimpleNewFromData(1, dims, NPY_INT32, is_in_polygon);

    // enable our array to free the memory we allocated for is_in_polygon
    PyArray_ENABLEFLAGS((PyArrayObject*) arr, NPY_ARRAY_OWNDATA);

    return arr;
}

static PyMethodDef module_methods[] = {
    {"calc_wind_count", wrap_calc_wind_count, METH_VARARGS, NULL},
    {"points_in_polygon", wrap_points_in_polygon, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef gatingdef = {
    PyModuleDef_HEAD_INIT, 
    "gating_c", 
    NULL, 
    -1, 
    module_methods
};

PyMODINIT_FUNC PyInit_gating_c(void) {
    PyObject *m = PyModule_Create(&gatingdef);
    if (m == NULL) {
        return NULL;
    }

    import_array();
    if (PyErr_Occurred()) {
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
#else
PyMODINIT_FUNC initgating_c(void) {
    PyObject *m = Py_InitModule3("gating_c", module_methods, NULL);
    if (m == NULL) {
        return;
    }

    import_array();
    if (PyErr_Occurred()) {
        Py_DECREF(m);
        return;
    }

    return;
}
#endif
