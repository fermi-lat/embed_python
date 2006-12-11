/** @file Module.h
    @brief declare Module class

    $Header: /nfs/slac/g/glast/ground/cvs/embed_python/embed_python/Module.h,v 1.4 2006/12/11 18:19:34 jchiang Exp $
*/
#ifndef embed_python_Module_h
#define embed_python_Module_h

#include <map>
#include <vector>
#include <string>

// this needed to avoid include of Python.h, since PyObject is a struct
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif
namespace embed_python {
/** @class Module
    @brief mangage a Python module, for convenient program initialization

    @author T.Burnett

*/
class Module {
public:

    /// @param path path to module. If empty, assume cwd
    /// @param module name of module to load, without ".py". 
    /// @param python_dir Directory to be prepended to PYTHONPATH
    /// @param verbose [false] useful output if debugging?
    Module(std::string path, std::string module, 
           const std::string & python_dir="",
           bool verbose=false);
           
    ~Module();

    /// return a numeric type
    double operator[](std::string key);

    /// set a numeric value
    void getValue(std::string attribute, double & value);

    /// set a string value
    void getValue(std::string attribute, std::string& value);

    /// set a list of strings
    void getList(std::string listname, std::vector<std::string>& names);

    /// set a list of values
    void getList(std::string listname, std::vector<double>& values);

    /// examine type of a PyObject
    std::string type(const PyObject* obj)const;

    /// call a callable object
    PyObject* call(PyObject* o, PyObject* args=0 )const;

    /// return the attribute -- exception if does not exist
    PyObject * attribute(std::string name);

    /// access root, if path was defined
    std::string root()const{return m_root;}

    /// test program, to be called directly from a main.
    static int test(int argc, char* argv[], std::string modulename);

private:

    PyObject * m_module; ///< the module we manage
    std::string m_moduleName; 

    bool m_verbose;
    bool verbose()const{return m_verbose;}
    void check_error(const std::string& text)const;
    std::string m_root; ///< python path
};

}// namespace

#endif
