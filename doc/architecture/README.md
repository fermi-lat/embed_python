---
package: embed_python
summary: Small static library (embed_python::Module) that starts an embedded Python interpreter, imports a module, and reads its variables, lists and dicts into C++.
layer: foundation
depends_on: []
used_by: [fitsGen]
last_reviewed_commit: 9e613ba
---

# embed_python Architecture

**embed_python** is one class, `embed_python::Module`. It was written
(T. Burnett, 2006) so that C++ programs could use a Python file as a
configuration script: import the module, then read `x = 99.`,
`names = [...]` or `mymap = {...}` into C++ variables, or get callables to
invoke.

Its only live user in the CMake build is `fitsGen::EventClassifier`, which
loads a Python event-class script (e.g. `Pass4_Classifier.py`). `st_graph`
**links** embed_python, but it does not use it: it embeds Python itself
(`st_graph/src/EmbedPython.cpp`).

`src/mainpage.h` (2006) is accurate but minimal. It still refers to a CMT
`requirements` file that no longer exists. `doc/release.notes` stops in
2014.

## Purpose and scope

**Does:**
- Initialize the interpreter (once per process), set `sys.argv`, prepend
  directories to `sys.path`, and import a named module.
- Read module attributes, including dotted ones (`"math.pi"`, `"A.a"`), as
  `double`, `int`, `std::string`, `std::vector<std::string|double>`, or
  `std::map<std::string, double | std::vector<double> | std::vector<std::string>>`.
- Return raw `PyObject*` attributes and call callables.

**Does not:**
- Manage the GIL or threads, or support sub-interpreters.
- Convert C++ values to Python. Apart from `call(o, args)`, which takes a
  ready-made `PyObject*` tuple, it is read-only.
- Manage reference counts for the caller. Raw `PyObject*` results are new
  references that the caller owns.

## Position in the system

| Direction | Package | Why |
|-----------|---------|-----|
| Depends on | Python 3 (external) | `Python3::Python`, linked `PUBLIC`, so every consumer links libpython. No Fermi package dependencies. |
| Used by | `fitsGen` | `fitsGen/EventClassifier.h` holds an `embed_python::Module *`. `EventClassifier` (in `libfitsGen`) imports the classifier script and calls its `eventClassifier` function for each event. `fitsGen` links embed_python `PUBLIC`. In the CMake build, `EventClassifier` is exercised only by `test_fitsGen`. `observationSim` links `fitsGen` but does not use `EventClassifier`, though it still links libpython transitively. `makeFT1`, the tool that originally used it, is not built by CMake. |
| Linked but unused | `st_graph` | `target_link_libraries(st_graph PRIVATE embed_python ...)`, but no `st_graph` source includes `embed_python/`. `st_graph` has its own Python embedding (`src/EmbedPython.cpp`, `MPLEngine.cxx`). The `dependency-layers.md` edge `st_graph → embed_python` is link-only. |
| Not built | `skymaps` | `PySkyFunction` uses `embed_python::Module`, and `skymaps/CMakeLists.txt` links it `PUBLIC`, but skymaps is not in `FERMI_COMPONENTS`. |

## Key abstractions

| Member | Role |
|--------|------|
| `Module(python_dir, module, argc, argv)` | Preferred constructor. `Py_Initialize()` if this process has not done so yet, `sys.argv = argv`, `sys.path.insert(0, python_dir)` (if not empty), then `sys.path.insert(0, ".")`, then `import module`. |
| `Module(path, module, python_dir="", verbose=false)` | "Deprecated" constructor, still used by `fitsGen`. Same as above, but `sys.argv = ["embed_python"]`; if `path` is non-empty it **`chdir`s into `path`** first and records it as `root()`. |
| `operator[](name)` → `double` | `PyFloat_AsDouble(attribute(name))`. |
| `getValue(name, int&/double&/std::string&)` | Required value. Throws if the attribute is missing or has the wrong type. |
| `getValue(name, T&, default)` | Uses `default` if the attribute is missing. |
| `getList(name, vector<string>&/vector<double>&)` | Iterates any Python iterable and **appends** to the vector. |
| `getDict(name, map<string,double>&)`, `map<string,vector<double>>&`, `map<string,vector<string>>&` | Iterates a dict and inserts or overwrites keys. |
| `attribute(name, check=true)` → `PyObject*` | Dotted lookup from the module. With `check=false`, returns `nullptr` and clears the error. |
| `call(o, args=0)` → `PyObject*` | `PyObject_CallObject`, followed by an error check. |
| `type(o)` | `o->ob_type->tp_name`. |
| `static test(argc, argv, modulename)` | Self-test driver used by `test_embed_python`. |

`Module.h` forward-declares `PyObject` (`struct _object`), so clients do not
need `Python.h` unless they handle `PyObject*` themselves (`fitsGen` does).

All errors go through `check_error`. If a Python exception is pending, it
calls **`PyErr_Print()`** (the traceback goes to stderr) and throws
`std::invalid_argument(text)`. Missing lists or dicts throw
`std::invalid_argument("... is not a list/dictionary")`.

## Data flow and control flow

```
fitsGen::EventClassifier("Pass4_Classifier")
  ├─ commonUtilities::setupEnvironment()
  └─ new embed_python::Module("", "Pass4_Classifier", pythonPath(), false)
        ├─ Py_Initialize()                      // first Module in process only
        ├─ sys.argv = ["embed_python"]
        ├─ sys.path.insert(0, $FERMI_INST_DIR/fitsGen/python)
        ├─ sys.path.insert(0, ".")              // cwd now ahead of python_dir
        └─ import Pass4_Classifier
  ├─ m_module->attribute("eventClassifier")     // PyObject* callable
  └─ MeritDict: m_module->getList("meritVariables", keys)
...
EventClassifier::~EventClassifier
  └─ delete m_module  → Py_DECREF(module); Py_Finalize()
```

## Public interfaces

### C++ API

```cpp
#include "embed_python/Module.h"

embed_python::Module cfg(python_dir, "MyConfig", argc, argv);
double emin;  cfg.getValue("emin", emin);
int nbins;    cfg.getValue("nbins", nbins, 10);     // default if absent
std::vector<std::string> names;  cfg.getList("names", names);
std::map<std::string, double> pars;  cfg.getDict("pars", pars);
double pi = cfg["math.pi"];
```

Keep **one** `Module` for the life of the process (see gotchas).

### Applications

None. `test_embed_python` (`src/test/runPy.cxx`) is a test program.

### Python

No bindings. `python/Test.py` and `python/embed_python_Test.py`, the test
modules, are installed into `site-packages/fermitools/`. The test uses
`embed_python_Test.py`; `Test.py` is an older copy without
`stringlistdict`.

### Environment variables

None read by embed_python itself (no `getenv`). The embedded interpreter
honours the usual Python variables (`PYTHONPATH`, `PYTHONHOME`) when
`Py_Initialize` runs. Variables set by conda activation are described
under "Runtime environment" in ScienceTools' `docs/architecture/README.md`.

### File formats

None. Input is any importable Python module.

## Build and test

- **CMake target:** `embed_python`, a **static** library (PIC, because the
  top-level build sets `CMAKE_POSITION_INDEPENDENT_CODE ON`), linked
  `PUBLIC Python3::Python`. Header installed to
  `include/fermitools/embed_python`; the library to `lib/libembed_python.a`.
  `EXPORT fermiTargets` is commented out in `install(TARGETS ...)`. This
  does no harm, because the top-level build never calls
  `install(EXPORT fermiTargets)`.
- **Test:** `test_embed_python` calls `Module::test(argc, argv, "embed_python_Test")`.
  It is **not registered with CTest**. The module is found through
  `sys.path`: `"."` (the current directory) or the normal Python path. The
  installed copy in `site-packages/fermitools/` is found only if that
  directory is on `sys.path` (via `fermitools.pth`, the module search
  path of the conda Python). Running from `src/embed_python/python/` also
  works. The test exits 0 on success and 1 on failure. No data files and
  no scratch output. The project harness `recipe/tests/ST-unit-test` runs
  `test_embed_python` in its default list (from the current directory) and
  checks only the exit status; no output is compared and no reference data
  exists anywhere. See "Testing" in ScienceTools'
  `docs/architecture/README.md`.
- **Legacy:** `SConscript`, `embed_pythonLib.py` (adds `-rdynamic` for
  Linux SCons users), `.cvsignore`.
- **CI:** `azure-pipelines.yml` (shared `fermi-lat/ci` templates).

## Invariants and gotchas

**Interpreter lifetime**
- `~Module()` **always calls `Py_Finalize()`**. A file-static
  `initialized` flag stays `true`, so a second `Module` built after the
  first is destroyed **skips `Py_Initialize()`** and runs against a
  finalized interpreter (from reading the code; not checked at runtime).
  Creating two `fitsGen::EventClassifier` objects one after another is
  enough to trigger it. Use one `Module` per process.
- Two `Module`s alive at the same time share the interpreter, but the
  first to be destroyed finalizes it under the other.
- When the code runs inside an existing Python process (e.g. a SWIG module
  loaded by Python), `Py_Initialize()` is a no-op, but the destructor still
  calls `Py_Finalize()` on the host interpreter. Do not use `Module` from
  code that may be loaded into Python.
- No GIL handling. Call it only from the thread that owns the interpreter.

**Side effects**
- The deprecated constructor `chdir`s into `path` and **never changes back**
  (it saves `oldcwd` but never uses it). Its `getcwd` buffers are 128
  bytes. `fitsGen` passes `path = ""`, so it is not affected.
- Both constructors insert `"."` at `sys.path[0]` *after* `python_dir`, so
  a same-named module in the **current working directory shadows** the
  intended one.
- Each constructor sets `sys.argv`, replacing any existing value.

**Type conversion**
- `getValue(name, int&)` (no default) uses `PyFloat_AsDouble` and
  truncates, so `nbins = 3.7` gives 3. `getValue(name, int&, default)` uses
  `PyLong_AsLong`, which rejects a float with a `TypeError` on current
  Python versions. The two overloads accept different inputs.
- `getList` accepts any iterable. A string iterates **character by
  character**, and a `range` works. Results are **appended**, so clear the
  vector first if you reuse it.
- `getDict(map<string,double>&)` does not check each conversion. A
  non-numeric value is stored as `-1` and the error appears only in the
  final `check_error`, after the map has been filled.

**Reference counting** (from reading the code; not checked at runtime)
- `attribute()` returns a new reference, and most getters never
  `Py_DECREF` it. Dotted lookups leak the intermediate object.
  `PyList_Append(mylist, Py_BuildValue(...))` leaks each `argv` string.
  These leaks are small and happen once at startup, which is fine for
  configuration use but not for hot loops.

## Extension points

- **New value type:** add a `getValue`/`getList`/`getDict` overload in
  `Module.h` and `Module.cxx`, following the existing pattern: call
  `attribute(name)`, convert, then `check_error(context)`. Add a matching
  variable to `python/embed_python_Test.py` and a check in `Module::test`.
- If you need richer embedding (C++ → Python values, GIL, interpreter
  reuse), consider pybind11 or the approach in `st_graph/src/EmbedPython.cpp`
  instead of growing this class.

## Further reading

- `src/Module.cxx`: the whole implementation (about 470 lines).
- `fitsGen/src/EventClassifier.cxx` (ScienceTools path `src/fitsGen/`): the
  only consumer.
- [Python/C API: Embedding Python](https://docs.python.org/3/extending/embedding.html),
  for `Py_Initialize`/`Py_Finalize` semantics.
