#ifndef MATWLIB_HDR
#define MATWLIB_HDR

#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <type_traits>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "any_iterator/any_iterator.hpp"

#include "matwlib_const.h"
#include "matwlib_ipc.h"

namespace MatWLib {
   typedef enum {
      #if defined(HAS_MATLAB)
        ENGINE_MATLAB,
      #endif
      #if defined(HAS_OCTAVE)
        ENGINE_OCTAVE_STATE,
        ENGINE_OCTAVE_STATE_INTERPRETER,
        ENGINE_OCTAVE_STATELESS,
      #endif
      #if !defined(MATWLIB_PREFER_STATIC_ERRORS) || (defined(MATWLIB_PREFER_STATIC_ERRORS) && defined(HAS_EIGEN))
        ENGINE_EIGEN,
      #endif

      ENGINE_OFFLINE
   } ENGINE_TYPE;

   typedef enum {
      OK,
      ERROR,
      UNKNOWN
   } STATUS;

   //Matwlib Exception
     struct MatWlibException : public std::runtime_error {
            MatWlibException(std::string msg) : std::runtime_error(msg) {};
     };
   //Iterator
     template<class T> using FWD_IT_T = IteratorTypeErasure::any_iterator<T, std::forward_iterator_tag, T>;
     template<class T> using CONST_FWD_IT_T = IteratorTypeErasure::any_iterator<const T, std::forward_iterator_tag, const T>;
   //Unique number generator
     size_t getUniqueNumber();
     void resetUniqueNumberGenerator();

   //Information handling
     //Data - represent undecoded data of some (maybe undetermined) type
       struct Data {
           Data() {}
           virtual ~Data() {}
       };
     //Value - represent decoded data
       struct Value {
           Value();
           virtual ~Value();

           virtual std::string toString() const = 0;
       };
       struct Value_Array : public Value {
           size_t arrSize;
           std::vector<Value *> arr;

           Value_Array();
           virtual ~Value_Array();
           void addElement(Value *vt);
           virtual std::string toString() const;
       };
       struct Value_Matrix : public Value {
           std::vector<std::vector<Value *> *> mat;

           Value_Matrix();
           virtual ~Value_Matrix();
           void addRow();
           void addElementToLastRow(Value *vt);
           virtual std::string toString() const;
       };
       struct Value_Struct : public Value {
           std::vector<Value *> vecChildren;

           Value_Struct();
           Value_Struct(std::vector<Value *> vecChildren_);
           virtual ~Value_Struct();
           virtual std::string toString() const;
       };
       template<class T> struct Value_Scalar : public Value {
           T val;
           Value_Scalar(T val_) : val(val_) {}
           virtual ~Value_Scalar() {}
           virtual std::string toString() const {
               std::stringstream ss;
               ss.setf(std::ios::showpoint);
               ss << val;
               return ss.str();
           }
       };

       //Returned from decodeData!, can be decomposed into Value_Array/Value_Matrix
       template<class T> struct Value_RawMatrix : public Value {
           T *val;
           size_t nDim, *dimSz;
           Value_RawMatrix(T *val_, size_t nDim_, const size_t *dimSz_) : val(val_), nDim(nDim_) {
               dimSz = new size_t[nDim_];
               for(size_t i = 0; i < nDim_; i++)
                   dimSz[i] = dimSz_[i];
           }
           Value_RawMatrix(T *val_, size_t nDim_, std::vector<size_t> &dimSz_) : val(val_), nDim(nDim_) {
               dimSz = new size_t[nDim_];
               for(size_t i = 0; i < nDim_; i++)
                   dimSz[i] = dimSz_[i];
           }

           virtual ~Value_RawMatrix() {
               delete[] dimSz;
           }
           virtual std::string toString() const {
               assert(false);
           }
       };

       Value *getArg(Value *in, size_t row, size_t col);
       Value *getArg(Value *in, size_t index);
       template<class T> T getValue(Value *in);
       template<class T> void setValue(Value *in, T &value);

   //Function
     struct Function {
        //Argument descriptions
          struct Argument {
            Argument() {}
            virtual ~Argument() {}
          };
          struct Argument_Array : public Argument {
            Argument *t;
            Argument_Array(Argument *t_) : t(t_) {}
            virtual ~Argument_Array() {
               delete t;
            }
          };
          struct Argument_Struct : public Argument {
             std::vector<Argument *> structData;
             Argument_Struct() {}
             virtual ~Argument_Struct() {
                 for(std::vector<Argument *>::iterator it = structData.begin(); it != structData.end(); ++it)
                     delete *it;
             }
          };
          struct Argument_Element : public Argument {
            TYPE t;
            Argument_Element(TYPE t_) : t(t_) {}
            virtual ~Argument_Element() {}
          };
        //Data
          #ifdef MATWLIB_USE_CPP0x_FUNC_CALLBACK
            typedef std::function<bool(const std::vector<Value *> &, std::vector<Value *> &)> FUNC_T;
          #else
            typedef bool(*FUNC_T)(const std::vector<Value *> &, std::vector<Value *> &);
          #endif
          FUNC_T funcHandler;
          std::vector<Argument *> vecInputs, vecOutputs;
        //Functions
          Function(FUNC_T funcHandler_, std::vector<Argument *> &vecInputs_, std::vector<Argument *> &vecOutputs_)
            : funcHandler(funcHandler_), vecInputs(vecInputs_), vecOutputs(vecOutputs_) {}
          ~Function() {
              for(std::vector<Argument *>::iterator it = vecInputs.begin(); it != vecInputs.end(); ++it)
                  delete *it;
              for(std::vector<Argument *>::iterator it = vecOutputs.begin(); it != vecOutputs.end(); ++it)
                  delete *it;
          }
     };
     template<> std::string Value_Scalar<std::string>::toString() const;

     Value *createValTree(Function::Argument *arg);
   //Session
     struct SessionCmd {
         typedef enum {CMD_READ, CMD_WRITE, CMD_EXEC} CMD_T;
         typedef enum {TYPE_DOUBLE, TYPE_STRING, TYPE_SIZET, TYPE_UNKNOWN} TYPE_T;
         typedef std::pair<size_t, size_t> MAT_SIZE_CONST_T;
         typedef std::pair<size_t &, size_t &> MAT_SIZE_VAR_T;
   
         CMD_T c;
         TYPE_T t;
         size_t opId;
         MAT_SIZE_VAR_T *szVar;
         MAT_SIZE_CONST_T *szConst;
         //(De/Con)structor(s)
           SessionCmd() {}
           SessionCmd(size_t opId_, CMD_T c_, TYPE_T t_, MAT_SIZE_CONST_T &sz_) : c(c_), t(t_), opId(opId_), szVar(NULL), szConst(&sz_) {}
           SessionCmd(size_t opId_, CMD_T c_, TYPE_T t_, MAT_SIZE_VAR_T *sz_) : c(c_), t(t_), opId(opId_), szVar(sz_), szConst(NULL) {}
           SessionCmd(const SessionCmd &x) : c(x.c), t(x.t), opId(x.opId), szVar(x.szVar), szConst(x.szConst) {}
           virtual ~SessionCmd() {}
     };
     struct ExecSessionCmd : public SessionCmd {
         std::string cmd;
         //(De/Con)structor(s)
           ExecSessionCmd(std::string cmd_, size_t opId_) 
               : SessionCmd(opId_, SessionCmd::CMD_EXEC, SessionCmd::TYPE_UNKNOWN, NULL), cmd(cmd_) {}
           virtual ~ExecSessionCmd() {}
     };
     template<class T> struct TypedSessionCmd : public SessionCmd {
         std::string varName;
         T &var;
         //(De/Con)structor(s)
           TypedSessionCmd(std::string varName_, T &var_, size_t opId_, SessionCmd::MAT_SIZE_VAR_T *sz_);    //Read
           TypedSessionCmd(std::string varName_, T &var_, size_t opId_, SessionCmd::MAT_SIZE_CONST_T &sz_);  //Write  
           virtual ~TypedSessionCmd();       
     };

     struct SessionErrorReport {
         std::set<size_t> errorId;
         //(De/Con)structor(s)
           SessionErrorReport() {}
           SessionErrorReport(const SessionErrorReport &x) : errorId(x.errorId) {}
           ~SessionErrorReport() {}
         
           SessionErrorReport &operator=(const SessionErrorReport &x) {
               this->errorId.clear();
               this->errorId.insert(x.errorId.begin(), x.errorId.end());
               return *this;
           }
     };




   //State
     template<class STATE_T> using NAME_T = typename STATE_T::NAME;

     class State {
      private:

      public:
         //(De/Con)structor(s)
           State();
           virtual ~State();
         //Write data
           virtual void writeVar(const std::string strVarName, std::string val) = 0;
           virtual void writeVar(const std::string strVarName, double val) = 0;
           virtual void writeVar(const std::string strVarName, size_t val) = 0;
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, const double *mat) = 0;
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, const size_t *mat) = 0;  
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, FWD_IT_T<double> begin, FWD_IT_T<double> end) = 0;
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, FWD_IT_T<size_t> begin, FWD_IT_T<size_t> end) = 0;
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, CONST_FWD_IT_T<double> begin, CONST_FWD_IT_T<double> end) = 0;
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, CONST_FWD_IT_T<size_t> begin, CONST_FWD_IT_T<size_t> end) = 0;
         //Read data
           virtual std::string readStrVar(const std::string strVarName) = 0;
           virtual double readDoubleVar(const std::string strVarName) = 0;
           virtual size_t readIntVar(const std::string strVarName) = 0;
           virtual double *readDoubleArr(const std::string strVarName, std::vector<size_t> &szDims) = 0;
           virtual size_t *readIntArr(const std::string strVarName, std::vector<size_t> &szDims) = 0;
         /*//Write data
           template<class T> void writeVar(const std::string strVarName, T val);
           template<class T> void writeVar(const std::string strVarName, size_t m, size_t n, const T *mat);  
           template<class IT> void writeVar(const std::string strVarName, size_t m, size_t n, IT begin, IT end);
         //Read data
           template<class T> T readVar(const std::string strVarName);
           template<class T> T *readVar(const std::string strVarName, std::vector<size_t> &szDims);
           template<class IT> void readVar(const std::string strVarName, std::vector<size_t> &szDims, IT begin, IT end);*/
         //Misc. functions
           //virtual std::string getUniqueVariableName();

           virtual std::string toString() const;
           virtual bool good() const;
     };
     template<class STATE_T> STATE_T *cast_state(State *s);

     //Variable interface
       struct VariableSupply {
         VariableSupply();
         virtual ~VariableSupply();

         virtual std::string createNewVariable() = 0;
       };
     //Function interface
       struct FunctionInterface {
         //(De/Con)structor(s)
           FunctionInterface();
           virtual ~FunctionInterface();
         //Function handling
           virtual bool registerFunction(std::string funcName, Function *funcDesc) = 0;
           virtual bool updateFunction(std::string funcName, Function::FUNC_T fn) = 0;
           virtual bool removeFunction(std::string funcName) = 0;
       };
     //Session interface
       struct SessionInterface {
         //(De/Con)structor(s)
           SessionInterface();
           virtual ~SessionInterface();
         //Function handling
           SessionErrorReport session(const std::vector<SessionCmd *> cmds);
       };
     //Raw data interface - for arbitrary data types with a common interface (e.g. as per Data)
       template<class NAME_T> struct RawDataInterface {
         #ifdef MATWLIB_USE_CPP0x_FUNC_CALLBACK
            typedef std::function<bool(size_t nrDims, size_t *elemPos, TYPE elemType, Data *val)> CELL_FUNC_T;
         #else
            typedef bool(*CELL_FUNC_T)(size_t nrDims, size_t *elemPos, TYPE elemType, Data *val);
         #endif
         //(De/Con)structor(s)
           RawDataInterface() {}
           virtual ~RawDataInterface() {}
         //Value handling
           virtual Value *decodeData(Data *d) = 0;
         //Data handling
           virtual void writeVar(const NAME_T varName, Data *d) = 0;
           virtual Data *readVar(const NAME_T varName) = 0;
           virtual void writeVar(const NAME_T varName, CELL_FUNC_T fn) = 0;
           virtual void readVar(const NAME_T varName, CELL_FUNC_T fn) = 0;
       };
     //Raw command interface - execute command based on string command
       struct RawExecuteInterface {
         //(De/Con)structor(s)
           RawExecuteInterface();
           virtual ~RawExecuteInterface();
         //Execute
           virtual void execute(std::string cmd) = 0;
       };




   //Interface functions
     State *init_engine(ENGINE_TYPE e, std::string path, bool enableSharedFunc = true); //TODO
     void finit_engine(State *s);

   //Operations
     enum class OPs {
       //Arithmetic
         OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MIN, OP_MAX, OP_SQRT, OP_ABS,

         OP_NEG, OP_CONJ, OP_TRANS, OP_INVERSE,

         OP_SCALE, OP_TRANSLATE,
       //Logical
         OP_AND, OP_OR, OP_NOT, OP_XOR
     };
     template<OPs o> struct OP_T {
       static const OPs op = o;
     };

     enum class INITIALIZATIONs {
       INIT_RANDOM,
       INIT_ZERO,
       INIT_ONE,
       INIT_NONE
     };

     enum class SPECIAL_MATRICES {
       MATRIX_IDENT,
       MATRIX_HANKEL,
     };

     enum class SPECIAL_VECTORS {

     };

     template<typename ... ARGS> struct has_op : public boost::false_type {};
     template<typename ... ARGS> struct state_has_op : public boost::false_type {};
     template<class VAL_T, class OP> struct has_op<VAL_T, OP> : public boost::false_type {};
     template<class VVAL_T, class MVAL_T, class OP> struct has_op<VVAL_T, MVAL_T, OP> : public boost::false_type {};
     template<class STATE_T, class OP> struct state_has_op<STATE_T, OP> : public boost::false_type {};

     //State-less
       template<class VAL_T> class MV;
       template<class VAL_T> class VV;


       //Vector
         template<class VVAL_T, class MVAL_T> MV<MVAL_T> toMatrix(const VV<VVAL_T> &x);
         template<class VVAL_T, class MVAL_T> MV<MVAL_T> toDiagonalMatrix(const VV<VVAL_T> &x);
         template<class VAL_T> VV<VAL_T> clone(const VV<VAL_T> &x);
         template<class VAL_T> void append(VV<VAL_T> &x, const VV<VAL_T> &app);
         template<class VAL_T> size_t numDims(const VV<VAL_T> &x) {return 1;};
         template<class VAL_T> size_t size(const VV<VAL_T> &x, size_t dim);
         template<class VAL_T, class CONST_T> VV<VAL_T> create(size_t i, bool isRowVector, const CONST_T constantVal);
         template<class VAL_T> VV<VAL_T> create(size_t i, bool isRowVector, INITIALIZATIONs init);
         template<class VAL_T> void set(VV<VAL_T> &x, const VV<VAL_T> &src);
         template<class VAL_T, class CONST_T> void set(VV<VAL_T> &x, const CONST_T y);
         template<class VAL_T, class CONST_T> void set(VV<VAL_T> &x, const CONST_T *y, size_t n); 
         template<class VAL_T, class CONST_T> void set(VV<VAL_T> &x, size_t i, const CONST_T y);
         template<class VAL_T, class CONST_T> void set(VV<VAL_T> &x, size_t base, size_t offset, const CONST_T y);
         template<class VAL_T> double toDouble(const VV<VAL_T> &x);
         template<class VAL_T> double elem(const VV<VAL_T> &x, size_t i); 

         template<class VAL_T, class OP> VV<VAL_T> opElem(const VV<VAL_T> &x, const VV<VAL_T> &y);
         template<class VAL_T, class OP> VV<VAL_T> opElem(const VV<VAL_T> &x);
         template<class VAL_T, class OP> double opReduce(const VV<VAL_T> &x);
         template<class VAL_T, class OP, class CONST_T> VV<VAL_T> opConst(const VV<VAL_T> &x, const CONST_T y);
         template<class VAL_T, class OP, class CONST_T> VV<VAL_T> opConst(const CONST_T x, const VV<VAL_T> &y);
         template<class VAL_T, class OP> VV<VAL_T> op(const VV<VAL_T> &x, const VV<VAL_T> &y);      
         template<class VAL_T, class OP> VV<VAL_T> op(const VV<VAL_T> &x); 
         //Guarded specialization
           template <class VAL_T, class OP> typename std::enable_if<has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOp(const VV<VAL_T> &x) {return op<VAL_T, OP>(x);}
           template <class VAL_T, class OP> typename std::enable_if<!has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOp(const VV<VAL_T> &x) {throw MatWlibException("Error trying to execute non-supported operation");}

           template <class VAL_T, class OP> typename std::enable_if<has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOp(const VV<VAL_T> &x, const VV<VAL_T> &y) {return op<VAL_T, OP>(x, y);}
           template <class VAL_T, class OP> typename std::enable_if<!has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOp(const VV<VAL_T> &x, const VV<VAL_T> &y) {throw MatWlibException("Error trying to execute non-supported operation");}

           template <class VAL_T, class OP, class CONST_T> typename std::enable_if<has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOpConst(const VV<VAL_T> &x, const CONST_T y) {return opConst<VAL_T, OP, CONST_T>(x, y);}
           template <class VAL_T, class OP, class CONST_T> typename std::enable_if<!has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOpConst(const VV<VAL_T> &x, const CONST_T y) {throw MatWlibException("Error trying to execute non-supported operation");}

           template <class VAL_T, class OP, class CONST_T> typename std::enable_if<has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOpConst(const CONST_T &x, const VV<VAL_T> &y) {return opConst<VAL_T, OP, CONST_T>(x, y);}
           template <class VAL_T, class OP, class CONST_T> typename std::enable_if<!has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOpConst(const CONST_T &x, const VV<VAL_T> &y) {throw MatWlibException("Error trying to execute non-supported operation");}

           template<class VAL_T, class OP> typename std::enable_if<has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOpElem(const VV<VAL_T> &x, const VV<VAL_T> &y) {return opElem<VAL_T, OP>(x, y);}
           template<class VAL_T, class OP> typename std::enable_if<!has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOpElem(const VV<VAL_T> &x, const VV<VAL_T> &y) {throw MatWlibException("Error trying to execute non-supported element-wise operation");}

           template<class VAL_T, class OP> typename std::enable_if<has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOpElem(const VV<VAL_T> &x) {return opElem<VAL_T, OP>(x);}
           template<class VAL_T, class OP> typename std::enable_if<!has_op<VAL_T, OP>::value, VV<VAL_T>>::type gOpElem(const VV<VAL_T> &x) {throw MatWlibException("Error trying to execute non-supported element-wise operation");}

           template<class VAL_T, class OP> typename std::enable_if<has_op<VAL_T, OP>::value, double>::type gOpReduce(const VV<VAL_T> &x) {return opReduce<VAL_T, OP>(x);}
           template<class VAL_T, class OP> typename std::enable_if<!has_op<VAL_T, OP>::value, double>::type gOpReduce(const VV<VAL_T> &x) {throw MatWlibException("Error trying to execute non-supported reduction operation");}
         //Multiple select
           template<class VAL_T> VV<VAL_T> op(OPs o, const VV<VAL_T> &x, const VV<VAL_T> &y) {
             switch(o) {
              case OPs::OP_ADD:  return gOp<VAL_T, OP_T<OPs::OP_ADD>>(x, y);
              case OPs::OP_SUB:  return gOp<VAL_T, OP_T<OPs::OP_SUB>>(x, y);
              case OPs::OP_MUL:  return gOp<VAL_T, OP_T<OPs::OP_MUL>>(x, y);
              case OPs::OP_DIV:  return gOp<VAL_T, OP_T<OPs::OP_DIV>>(x, y);
              case OPs::OP_AND:  return gOp<VAL_T, OP_T<OPs::OP_AND>>(x, y);
              case OPs::OP_OR:   return gOp<VAL_T, OP_T<OPs::OP_OR>>(x, y);
              case OPs::OP_XOR:  return gOp<VAL_T, OP_T<OPs::OP_XOR>>(x, y);
              default:           throw MatWlibException("Invalid OP, not binary vector function");
             }
           }
           template<class VAL_T> VV<VAL_T> op(OPs o, const VV<VAL_T> &x) {
             switch(o) {
              case OPs::OP_NEG:       return gOp<VAL_T, OP_T<OPs::OP_NEG>>(x);
              case OPs::OP_NOT:       return gOp<VAL_T, OP_T<OPs::OP_NOT>>(x);
              case OPs::OP_CONJ:      return gOp<VAL_T, OP_T<OPs::OP_CONJ>>(x);
              case OPs::OP_TRANS:     return gOp<VAL_T, OP_T<OPs::OP_TRANS>>(x);
              default:                throw MatWlibException("Invalid OP, not unary vector function");
             }
           }

           template<class VAL_T, class CONST_T> VV<VAL_T> opConst(OPs o, const VV<VAL_T> &x, const CONST_T y) {
             switch(o) {
              case OPs::OP_SCALE:        return gOpConst<VAL_T, OP_T<OPs::OP_SCALE>, CONST_T>(x, y);
              case OPs::OP_TRANSLATE:    return gOpConst<VAL_T, OP_T<OPs::OP_TRANSLATE>, CONST_T>(x, y);
              default:                   throw MatWlibException("Invalid OP, not vector-constant function");
             }
           }
           template<class VAL_T, class CONST_T> VV<VAL_T> opConst(OPs o, const CONST_T &x, const VV<VAL_T> y) {
             switch(o) {
              case OPs::OP_SCALE:        return gOpConst<VAL_T, OP_T<OPs::OP_SCALE>, CONST_T>(x, y);
              case OPs::OP_TRANSLATE:    return gOpConst<VAL_T, OP_T<OPs::OP_TRANSLATE>, CONST_T>(x, y);
              default:                   throw MatWlibException("Invalid OP, not vector-constant function");
             }
           }

           template<class VAL_T> VV<VAL_T> opElem(OPs o, const VV<VAL_T> &x, const VV<VAL_T> &y) {
             switch(o) {
              case OPs::OP_MUL:  return gOpElem<VAL_T, OP_T<OPs::OP_MUL>>(x, y);
              case OPs::OP_DIV:  return gOpElem<VAL_T, OP_T<OPs::OP_DIV>>(x, y);
              case OPs::OP_MIN:  return gOpElem<VAL_T, OP_T<OPs::OP_MIN>>(x, y);
              case OPs::OP_MAX:  return gOpElem<VAL_T, OP_T<OPs::OP_MAX>>(x, y);
              default:           throw MatWlibException("Invalid element-wise OP, not binary vector function");
             }
           }
           template<class VAL_T> VV<VAL_T> opElem(OPs o, const VV<VAL_T> &x) {
             switch(o) {
              case OPs::OP_SQRT:  return gOpElem<VAL_T, OP_T<OPs::OP_SQRT>>(x);
              case OPs::OP_ABS:  return gOpElem<VAL_T, OP_T<OPs::OP_ABS>>(x);
              default:           throw MatWlibException("Invalid element-wise OP, not unary vector function");
             }
           }

           template<class VAL_T> double opReduce(OPs o, const VV<VAL_T> &x) {
             switch(o) {
              case OPs::OP_ADD:  return gOpReduce<VAL_T, OP_T<OPs::OP_ADD>>(x);
              case OPs::OP_MUL:  return gOpReduce<VAL_T, OP_T<OPs::OP_MUL>>(x);
              case OPs::OP_MIN:  return gOpReduce<VAL_T, OP_T<OPs::OP_MIN>>(x);
              case OPs::OP_MAX:  return gOpReduce<VAL_T, OP_T<OPs::OP_MAX>>(x);
              default:           throw MatWlibException("Invalid reduction OP, not unary vector function");
             }
           }
       //Matrix
         template<class VAL_T> MV<VAL_T> clone(const MV<VAL_T> &x);
         template<class VAL_T> void appendRows(MV<VAL_T> &x, const MV<VAL_T> &app);
         template<class VAL_T> void appendColumns(MV<VAL_T> &x, const MV<VAL_T> &app);
         template<class MVAL_T, class VVALT_T> void appendRow(MV<MVAL_T> &x, const VV<VVALT_T> &app);
         template<class MVAL_T, class VVALT_T> void appendColumn(MV<MVAL_T> &x, const VV<VVALT_T> &app);
         template<class VAL_T> size_t numDims(const MV<VAL_T> &x) {return 2;};
         template<class VAL_T> size_t size(const MV<VAL_T> &x, size_t dim);
         template<class VAL_T, class CONST_T> MV<VAL_T> create(size_t r, size_t c, const CONST_T constantVal);
         template<class VAL_T> MV<VAL_T> create(size_t r, size_t c, SPECIAL_MATRICES type);
         template<class VAL_T> MV<VAL_T> create(size_t r, size_t c, INITIALIZATIONs init); 
         template<class VAL_T> void set(MV<VAL_T> &x, const MV<VAL_T> &src);
         template<class VAL_T, class CONST_T> void set(MV<VAL_T> &dst, const CONST_T y);
         template<class VAL_T, class CONST_T> void set(MV<VAL_T> &dst, const CONST_T *y, size_t n);
         template<class VAL_T, class CONST_T> void set(MV<VAL_T> &dst, size_t r, size_t c, const CONST_T y);
         template<class VAL_T, class CONST_T> void set(MV<VAL_T> &dst, size_t rb, size_t r_len, size_t cb, size_t c_len, const CONST_T y);
         template<class VAL_T> double toDouble(const MV<VAL_T> &x);
         template<class MVAL_T, class VVAL_T> VV<VVAL_T> row(const MV<MVAL_T> &x, size_t r); 
         template<class MVAL_T, class VVAL_T> VV<VVAL_T> col(const MV<MVAL_T> &x, size_t c); 
         template<class VAL_T> double elem(const MV<VAL_T> &x, size_t r, size_t c); 
         template<class VAL_T> MV<VAL_T> mode(const MV<VAL_T> &x, size_t i); 

         template<class VAL_T, class OP> MV<VAL_T> opElem(const MV<VAL_T> &x, const MV<VAL_T> &y); 
         template<class VAL_T, class OP> MV<VAL_T> opElem(const MV<VAL_T> &x); 
         template<class VAL_T, class OP> double opReduce(const MV<VAL_T> &x); 
         template<class VAL_T, class OP, class CONST_T> MV<VAL_T> opConst(const MV<VAL_T> &x, const CONST_T y);
         template<class VAL_T, class OP, class CONST_T> MV<VAL_T> opConst(const CONST_T x, const MV<VAL_T> &y);
         template<class VAL_T, class OP> MV<VAL_T> op(const MV<VAL_T> &x, const MV<VAL_T> &y);
         template<class VAL_T, class OP> MV<VAL_T> op(const MV<VAL_T> &x, const VV<VAL_T> &y);  
         template<class VAL_T, class OP> MV<VAL_T> op(const VV<VAL_T> &x, const MV<VAL_T> &y);        
         template<class VAL_T, class OP> MV<VAL_T> op(const MV<VAL_T> &x); 

         //Guarded specialization
           template<class VAL_T, class OP> typename std::enable_if<has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOp(const MV<VAL_T> &x) {return op<VAL_T, OP>(x);}
           template<class VAL_T, class OP> typename std::enable_if<!has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOp(const MV<VAL_T> &x) {throw MatWlibException("Error trying to execute non-supported operation");}

           template<class VAL_T, class OP> typename std::enable_if<has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOp(const MV<VAL_T> &x, const MV<VAL_T> &y) {return op<VAL_T, OP>(x, y);}
           template<class VAL_T, class OP> typename std::enable_if<!has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOp(const MV<VAL_T> &x, const MV<VAL_T> &y) {throw MatWlibException("Error trying to execute non-supported operation");}

           template<class VVAL_T, class MVAL_T, class OP> typename std::enable_if<has_op<VVAL_T, MVAL_T, OP>::value, MV<MVAL_T>>::type gOp(const MV<MVAL_T> &x, const VV<VVAL_T> &y) {return op<VVAL_T, MVAL_T, OP>(x, y);}
           template<class VVAL_T, class MVAL_T, class OP> typename std::enable_if<!has_op<VVAL_T, MVAL_T, OP>::value, MV<MVAL_T>>::type gOp(const MV<MVAL_T> &x, const VV<VVAL_T> &y) {throw MatWlibException("Error trying to execute non-supported operation");}

           template<class VVAL_T, class MVAL_T, class OP> typename std::enable_if<has_op<VVAL_T, MVAL_T, OP>::value, MV<MVAL_T>>::type gOp(const VV<VVAL_T> &x, const MV<MVAL_T> &y) {return op<VVAL_T, MVAL_T, OP>(x, y);}
           template<class VVAL_T, class MVAL_T, class OP> typename std::enable_if<!has_op<VVAL_T, MVAL_T, OP>::value, MV<MVAL_T>>::type gOp(const VV<VVAL_T> &x, const MV<MVAL_T> &y) {throw MatWlibException("Error trying to execute non-supported operation");}

           template<class VAL_T, class OP, class CONST_T> typename std::enable_if<has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOpConst(const MV<VAL_T> &x, const CONST_T y) {return opConst<VAL_T, OP, CONST_T>(x, y);}
           template<class VAL_T, class OP, class CONST_T> typename std::enable_if<!has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOpConst(const MV<VAL_T> &x, const CONST_T y) {throw MatWlibException("Error trying to execute non-supported operation");}

           template<class VAL_T, class OP, class CONST_T> typename std::enable_if<has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOpConst(const CONST_T &x, const MV<VAL_T> &y) {return opConst<VAL_T, OP, CONST_T>(x, y);}
           template<class VAL_T, class OP, class CONST_T> typename std::enable_if<!has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOpConst(const CONST_T &x, const MV<VAL_T> &y) {throw MatWlibException("Error trying to execute non-supported operation");}

           template<class VAL_T, class OP> typename std::enable_if<has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOpElem(const MV<VAL_T> &x, const MV<VAL_T> &y) {return opElem<VAL_T, OP>(x, y);}
           template<class VAL_T, class OP> typename std::enable_if<!has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOpElem(const MV<VAL_T> &x, const MV<VAL_T> &y) {throw MatWlibException("Error trying to execute non-supported operation");}

           template<class VAL_T, class OP> typename std::enable_if<has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOpElem(const MV<VAL_T> &x) {return opElem<VAL_T, OP>(x);}
           template<class VAL_T, class OP> typename std::enable_if<!has_op<VAL_T, OP>::value, MV<VAL_T>>::type gOpElem(const MV<VAL_T> &x) {throw MatWlibException("Error trying to execute non-supported operation");}

           template<class VAL_T, class OP> typename std::enable_if<has_op<VAL_T, OP>::value, double>::type gOpReduce(const MV<VAL_T> &x) {return opReduce<VAL_T, OP>(x);}
           template<class VAL_T, class OP> typename std::enable_if<!has_op<VAL_T, OP>::value, double>::type gOpReduce(const MV<VAL_T> &x) {throw MatWlibException("Error trying to execute non-supported reduction operation");}
         //Multiple select
           template<class VAL_T> MV<VAL_T> op(OPs o, const MV<VAL_T> &x, const MV<VAL_T> &y) {
             switch(o) {
              case OPs::OP_ADD:  return gOp<VAL_T, OP_T<OPs::OP_ADD>>(x, y);
              case OPs::OP_SUB:  return gOp<VAL_T, OP_T<OPs::OP_SUB>>(x, y);
              case OPs::OP_MUL:  return gOp<VAL_T, OP_T<OPs::OP_MUL>>(x, y);
              case OPs::OP_DIV:  return gOp<VAL_T, OP_T<OPs::OP_DIV>>(x, y);
              case OPs::OP_AND:  return gOp<VAL_T, OP_T<OPs::OP_AND>>(x, y);
              case OPs::OP_OR:   return gOp<VAL_T, OP_T<OPs::OP_OR>>(x, y);
              case OPs::OP_XOR:  return gOp<VAL_T, OP_T<OPs::OP_XOR>>(x, y);
              default:           throw MatWlibException("Invalid OP, not binary matrix-matrix function");
             }
           }
           template<class VVAL_T, class MVAL_T> MV<MVAL_T> op(OPs o, const MV<MVAL_T> &x, const VV<VVAL_T> &y) {
             switch(o) {
              case OPs::OP_ADD:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_ADD>>(x, y);
              case OPs::OP_SUB:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_SUB>>(x, y);
              case OPs::OP_MUL:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_MUL>>(x, y);
              case OPs::OP_DIV:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_DIV>>(x, y);
              case OPs::OP_AND:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_AND>>(x, y);
              case OPs::OP_OR:   return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_OR>>(x, y);
              case OPs::OP_XOR:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_XOR>>(x, y);
              default:           throw MatWlibException("Invalid OP, not binary matrix-vector function");
             }
           }
           template<class VVAL_T, class MVAL_T> MV<MVAL_T> op(OPs o, const VV<VVAL_T> &x, const MV<MVAL_T> &y) {
             switch(o) {
              case OPs::OP_ADD:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_ADD>>(x, y);
              case OPs::OP_SUB:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_SUB>>(x, y);
              case OPs::OP_MUL:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_MUL>>(x, y);
              case OPs::OP_DIV:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_DIV>>(x, y);
              case OPs::OP_AND:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_AND>>(x, y);
              case OPs::OP_OR:   return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_OR>>(x, y);
              case OPs::OP_XOR:  return gOp<VVAL_T, MVAL_T, OP_T<OPs::OP_XOR>>(x, y);
              default:           throw MatWlibException("Invalid OP, not binary vector-matrix function");
             }
           }

           template<class VAL_T> MV<VAL_T> op(OPs o, const MV<VAL_T> &x) {
             switch(o) {
              case OPs::OP_NEG:       return gOp<VAL_T, OP_T<OPs::OP_NEG>>(x);
              case OPs::OP_NOT:       return gOp<VAL_T, OP_T<OPs::OP_NOT>>(x);
              case OPs::OP_CONJ:      return gOp<VAL_T, OP_T<OPs::OP_CONJ>>(x);
              case OPs::OP_TRANS:     return gOp<VAL_T, OP_T<OPs::OP_TRANS>>(x);
              case OPs::OP_INVERSE:   return gOp<VAL_T, OP_T<OPs::OP_INVERSE>>(x);
              default:                throw MatWlibException("Invalid OP, not unary matrix function");
             }
           }

           template<class VAL_T, class CONST_T> MV<VAL_T> opConst(OPs o, const MV<VAL_T> &x, const CONST_T y) {
             switch(o) {
              case OPs::OP_SCALE:        return gOpConst<VAL_T, OP_T<OPs::OP_SCALE>>(x, y);
              case OPs::OP_TRANSLATE:    return gOpConst<VAL_T, OP_T<OPs::OP_TRANSLATE>>(x, y);
              default:                   throw MatWlibException("Invalid OP, not binary matrix-constant function");
             }
           }
           template<class VAL_T, class CONST_T> MV<VAL_T> opConst(OPs o, const CONST_T &x, const MV<VAL_T> y) {
             switch(o) {
              case OPs::OP_SCALE:        return gOpConst<VAL_T, OP_T<OPs::OP_SCALE>>(x, y);
              case OPs::OP_TRANSLATE:    return gOpConst<VAL_T, OP_T<OPs::OP_TRANSLATE>>(x, y);
              default:                   throw MatWlibException("Invalid OP, not binary matrix-constant function");
             }
           }

           template<class VAL_T> MV<VAL_T> opElem(OPs o, const MV<VAL_T> &x, const MV<VAL_T> &y) {
             switch(o) {
              case OPs::OP_MUL:   return gOpElem<VAL_T, OP_T<OPs::OP_MUL>>(x, y);
              case OPs::OP_DIV:   return gOpElem<VAL_T, OP_T<OPs::OP_DIV>>(x, y);
              case OPs::OP_MIN:   return gOpElem<VAL_T, OP_T<OPs::OP_MIN>>(x, y);
              case OPs::OP_MAX:   return gOpElem<VAL_T, OP_T<OPs::OP_MAX>>(x, y);
              default:           throw MatWlibException("Invalid element-wise OP, not binary matri function");
             }
           }
           template<class VAL_T> MV<VAL_T> opElem(OPs o, const MV<VAL_T> &x) {
             switch(o) {
              case OPs::OP_SQRT:  return gOpElem<VAL_T, OP_T<OPs::OP_SQRT>>(x);
              case OPs::OP_ABS:   return gOpElem<VAL_T, OP_T<OPs::OP_ABS>>(x);
              default:           throw MatWlibException("Invalid element-wise OP, not unary matri function");
             }
           }

           template<class VAL_T> double opReduce(OPs o, const MV<VAL_T> &x) {
             switch(o) {
              case OPs::OP_ADD:  return gOpReduce<VAL_T, OP_T<OPs::OP_ADD>>(x);
              case OPs::OP_MUL:  return gOpReduce<VAL_T, OP_T<OPs::OP_MUL>>(x);
              case OPs::OP_MIN:  return gOpReduce<VAL_T, OP_T<OPs::OP_MIN>>(x);
              case OPs::OP_MAX:  return gOpReduce<VAL_T, OP_T<OPs::OP_MAX>>(x);
              default:           throw MatWlibException("Invalid reduction OP, not unary vector function");
             }
           }

       ////////////////////////////////////////////////////////////////
       template<class V, class X> class Initiater {
        private:
          X &x;
          size_t r, c;
        public:
          //(De/Con)structor(s)
            Initiater(X &x_, const V &v) : x(x_), r(0), c(1) {x.getV()(0,0) = v;}
            Initiater(const Initiater<V, X> &i) : x(i.x), r(i.r), c(i.c) {}
            ~Initiater() {}
          //Functions
            Initiater<V, X> &operator=(const Initiater<V, X> &i) {
              this->x = i.x;
              this->r = i.r;
              this->c = i.c;
              return *this;
            }
            Initiater<V, X> &operator,(const V &v) {
              if(this->x.getV().cols() == (ssize_t) this->c) {
                 this->c = 0;
                 this->r++;
                 assert((ssize_t) this->r < this->x.getV().rows());
              }
              x.getV()(this->r, this->c++) = v; //set(this->x, this->r, this->c++, v);
              return *this;
            }
            const X &operator->() const {return this->x;}
       };
       template<class VAL_T> class VV {
         private:
           VAL_T v;
         public:
           //(De/Con)structor(s)
             VV() {}
             VV(size_t i, INITIALIZATIONs init = INITIALIZATIONs::INIT_NONE) {VV<VAL_T> V = create<VAL_T>(i, init); this->v = V.getV();}
             VV(VAL_T v_) : v(v_) {}
             VV(const VV<VAL_T> &x) : v(x.getV()) {}
             virtual ~VV() {}
           //Functions
             void setV(VAL_T &v_) {this->v = v_;}
             const VAL_T &getV() const {return this->v;}
             VAL_T &getV() {return this->v;}
             double *toDouble() const {
               double *d = new double[v.size()];
               for(size_t i = 0; i < v.size(); i++)
                   d[i] = v(i);
               return d;
             }
             template<class MVALT_T> MV<MVALT_T> toMatrix() const {return MatWLib::toMatrix<VAL_T, MVALT_T>(*this);}
             template<class MVALT_T> MV<MVALT_T> toDiagonalMatrix() const {return MatWLib::toDiagonalMatrix<VAL_T, MVALT_T>(*this);}  
             VV<VAL_T> clone() const {return MatWLib::clone<VAL_T>(*this);}
             void appendRows(const VV<VAL_T> &app) {return MatWLib::append<VAL_T>(*this, app);}
             void appendColumns(const VV<VAL_T> &app) {return MatWLib::append<VAL_T>(*this, app);}
             size_t numDims() const {return 1;}
             size_t size() const {return (size(0) * size(1));}
             size_t size(size_t dim) const {return MatWLib::size<VAL_T>(*this, dim);} 
             double toDoubleScalar() const {assert(v.size() == 1);}
             VV<VAL_T> conjugate() const {assert(false && "TODO");}
             VV<VAL_T> transpose() const {assert(false && "TODO");}
             VV<VAL_T> inverse() const {assert(false && "TODO");}

             VV<VAL_T> scale(double c) const {assert(false && "TODO");}
             VV<VAL_T> translate(double c) const {assert(false && "TODO");}

             double operator()(size_t i)  const {return elem(*this, i);}

             VV<VAL_T> &operator=(const size_t x) {set<VAL_T>(this->v, x); return *this;}
             VV<VAL_T> &operator=(const double x) {set<VAL_T>(this->v, x); return *this;}
             VV<VAL_T> &operator=(const VV<VAL_T> &x) {this->v = x.v; return *this;}
             template<class V> Initiater<V, VV<VAL_T>> operator=(const V &x) const {
               return Initiater<V, VV<VAL_T>>(*this, x);
             }
             friend std::ostream &operator<<(std::ostream &o, const VV<VAL_T> &x) {
               o << "<";
		         for(size_t i = 0; i < x.size(); i++)
                   o << (i == 0 ? "" : " ") << elem(x, i);
               o << ">";
		         return o;
	          }

             double reduceAdd() const {return opReduce<VAL_T>(OPs::OP_ADD, *this);}
             double reduceMul() const {return opReduce<VAL_T>(OPs::OP_MUL, *this);}
             double reduceMin() const {return opReduce<VAL_T>(OPs::OP_MIN, *this);}
             double reduceMax() const {return opReduce<VAL_T>(OPs::OP_MAX, *this);}

             VV<VAL_T> elementwiseMul(const VV<VAL_T> &x) const {return opElem<VAL_T>(OPs::OP_MUL, *this, x);}
             VV<VAL_T> elementwiseDiv(const VV<VAL_T> &x) const {return opElem<VAL_T>(OPs::OP_DIV, *this, x);}
             VV<VAL_T> elementwiseMin(const VV<VAL_T> &x) const {return opElem<VAL_T>(OPs::OP_MIN, *this, x);}
             VV<VAL_T> elementwiseMax(const VV<VAL_T> &x) const {return opElem<VAL_T>(OPs::OP_MAX, *this, x);}
             VV<VAL_T> elementwiseSqrt() const {return opElem<VAL_T>(OPs::OP_SQRT, *this);}
             VV<VAL_T> elementwiseAbs() const {return opElem<VAL_T>(OPs::OP_ABS, *this);}

             VV<VAL_T> &operator+=(const VV<VAL_T> &x) {VV<VAL_T> V = op<VAL_T>(OPs::OP_ADD, *this, x); this->v = getV(); return *this;}
             VV<VAL_T> &operator-=(const VV<VAL_T> &x) {VV<VAL_T> V = op<VAL_T>(OPs::OP_SUB, *this, x); this->v = getV(); return *this;}
             VV<VAL_T> &operator*=(const VV<VAL_T> &x) {VV<VAL_T> V = op<VAL_T>(OPs::OP_MUL, *this, x); this->v = getV(); return *this;}
             VV<VAL_T> &operator/=(const VV<VAL_T> &x) {VV<VAL_T> V = op<VAL_T>(OPs::OP_DIV, *this, x); this->v = getV(); return *this;}

             VV<VAL_T> operator+(const VV<VAL_T> &x) const {return op<VAL_T>(OPs::OP_ADD, *this, x);}
             VV<VAL_T> operator-(const VV<VAL_T> &x) const {return op<VAL_T>(OPs::OP_SUB, *this, x);}
             VV<VAL_T> operator*(const VV<VAL_T> &x) const {return op<VAL_T>(OPs::OP_MUL, *this, x);}
             template<class MVAL_T> MV<MVAL_T> operator*(const MV<MVAL_T> &x) const {return op<VAL_T>(OPs::OP_MUL, *this, x);}
             VV<VAL_T> operator/(const VV<VAL_T> &x) const {return op<VAL_T>(OPs::OP_DIV, *this, x);}
             VV<VAL_T> operator-() const {return opConst<VAL_T>(OPs::OP_SUB, 0, *this);}
       };
       template<class VAL_T> class MV {
         public:
           typedef VAL_T VAL;
         private:
           VAL_T v;
         public:
           //(De/Con)structor(s)
             MV() {}
             MV(size_t r, size_t c, INITIALIZATIONs init = INITIALIZATIONs::INIT_NONE) {MV<VAL_T> V = create<VAL_T>(r, c, init); this->v = V.getV();}
             MV(VAL_T v_) : v(v_) {}
             MV(const MV<VAL_T> &x) : v(x.getV()) {}
             virtual ~MV() {}
           //Functions
             void setV(VAL_T &v_) {this->v = v_;}
             const VAL_T &getV() const {return this->v;}
             VAL_T &getV() {return this->v;}
             double *toDouble() const {
               double *d = new double[v.rows()*v.cols()];
               for(size_t r = 0; r < v.rows(); r++)
                   for(size_t c = 0; c < v.cols(); c++)
                       d[r*v.cols()+c] = v(r,c);
               return d;
             }
             MV<VAL_T> clone() const {return MatWLib::clone<VAL_T>(*this);}
             void appendRows(const MV<VAL_T> &app) {MatWLib::appendRows<VAL_T>(*this, app);}
             void appendColumns(const MV<VAL_T> &app) {MatWLib::appendColumns<VAL_T>(*this, app);}
             template<class VVALT_T> void appendRow(const VV<VVALT_T> &app) {MatWLib::appendRow<VAL_T, VVALT_T>(*this, app);}
             template<class VVALT_T> void appendColumn(const VV<VVALT_T> &app) {MatWLib::appendColumn<VAL_T, VVALT_T>(*this, app);}
             size_t numDims() const {return 2;}
             size_t size() const {return (size(0) * size(1));}
             size_t size(size_t dim)  const {return MatWLib::size<VAL_T>(*this, dim);} 
             double toDoubleScalar() const {assert((v.rows() == 1) && (v.cols() == 1)); return v(1,1);}
             MV<VAL_T> conjugate() const {MV<VAL_T> V = op<VAL_T>(OPs::OP_CONJ, *this); return V;}
             MV<VAL_T> transpose() const {MV<VAL_T> V = op<VAL_T>(OPs::OP_TRANS, *this); return V;}
             MV<VAL_T> inverse() const {MV<VAL_T> V = op<VAL_T>(OPs::OP_INVERSE, *this); return V;}

             MV<VAL_T> scale(double c) const {MV<VAL_T> V = opConst<VAL_T, double>(OPs::OP_SCALE, c, *this); return V;}
             MV<VAL_T> translate(double c) const {MV<VAL_T> V = opConst<VAL_T, double>(OPs::OP_TRANSLATE, c, *this); return V;}

             double operator()(size_t r, size_t c)  const {return elem(*this, r, c);}

             MV<VAL_T> &operator=(const size_t x) {set<VAL_T>(this->v, x); return *this;}
             MV<VAL_T> &operator=(const double x) {set<VAL_T>(this->v, x); return *this;}
             MV<VAL_T> &operator=(const MV<VAL_T> &x) {this->v = x.v; return *this;}
             template<class V> Initiater<V, MV<VAL_T>> operator<<(const V &x) {
               return Initiater<V, MV<VAL_T>>(*this, x);
             }
             friend std::ostream &operator<<(std::ostream &o, const MV<VAL_T> &x) {
               o << "[";
		         for(size_t r = 0, R = x.size(0); r < R; r++) {
                   for(size_t c = 0, C = x.size(1); c < C; c++)
                       o << (c == 0 ? "" : " ") << elem(x, r, c);
                   o << (r == R-1 ? "" : "; ");
               }
               o << "]";
		         return o;
	          }

             //Reduction
             double reduceAdd() const {return opReduce<VAL_T>(OPs::OP_ADD, *this);}
             double reduceMul() const {return opReduce<VAL_T>(OPs::OP_MUL, *this);}
             double reduceMin() const {return opReduce<VAL_T>(OPs::OP_MIN, *this);}
             double reduceMax() const {return opReduce<VAL_T>(OPs::OP_MAX, *this);}

             //Element-wise
             MV<VAL_T> elementwiseMul(const MV<VAL_T> &x) const {return opElem<VAL_T>(OPs::OP_MUL, *this, x);}
             MV<VAL_T> elementwiseDiv(const MV<VAL_T> &x) const {return opElem<VAL_T>(OPs::OP_DIV, *this, x);}
             MV<VAL_T> elementwiseMin(const MV<VAL_T> &x) const {return opElem<VAL_T>(OPs::OP_MIN, *this, x);}
             MV<VAL_T> elementwiseMax(const MV<VAL_T> &x) const {return opElem<VAL_T>(OPs::OP_MAX, *this, x);}
             MV<VAL_T> elementwiseSqrt() const {return opElem<VAL_T>(OPs::OP_SQRT, *this);}
             MV<VAL_T> elementwiseAbs() const {return opElem<VAL_T>(OPs::OP_ABS, *this);}

             //Matrix-vector
             template<class VVAL_T> MV<VAL_T> &operator+=(const VV<VVAL_T> &x) {MV<VAL_T> V = op<VVAL_T, VAL_T>(OPs::OP_ADD, *this, x); this->v = getV(); return *this;}
             template<class VVAL_T> MV<VAL_T> &operator-=(const VV<VVAL_T> &x) {MV<VAL_T> V = op<VVAL_T, VAL_T>(OPs::OP_SUB, *this, x); this->v = getV(); return *this;}
             template<class VVAL_T> MV<VAL_T> &operator*=(const VV<VVAL_T> &x) {MV<VAL_T> V = op<VVAL_T, VAL_T>(OPs::OP_MUL, *this, x); this->v = getV(); return *this;}
             template<class VVAL_T> MV<VAL_T> &operator/=(const VV<VVAL_T> &x) {MV<VAL_T> V = op<VVAL_T, VAL_T>(OPs::OP_DIV, *this, x); this->v = getV(); return *this;}

             template<class VVAL_T> MV<VAL_T> &operator+(const VV<VVAL_T> &x) const {return op<VVAL_T, VAL_T>(OPs::OP_ADD, *this, x);}
             template<class VVAL_T> MV<VAL_T> &operator-(const VV<VVAL_T> &x) const {return op<VVAL_T, VAL_T>(OPs::OP_SUB, *this, x);}
             template<class VVAL_T> MV<VAL_T> &operator*(const VV<VVAL_T> &x) const {return op<VVAL_T, VAL_T>(OPs::OP_MUL, *this, x);}
             template<class VVAL_T> MV<VAL_T> &operator/(const VV<VVAL_T> &x) const {return op<VVAL_T, VAL_T>(OPs::OP_DIV, *this, x);}

             //Matrix-matrix
             MV<VAL_T> &operator+=(const MV<VAL_T> &x) {MV<VAL_T> V = op<VAL_T>(OPs::OP_ADD, *this, x); this->v = getV(); return *this;}
             MV<VAL_T> &operator-=(const MV<VAL_T> &x) {MV<VAL_T> V = op<VAL_T>(OPs::OP_SUB, *this, x); this->v = getV(); return *this;}
             MV<VAL_T> &operator*=(const MV<VAL_T> &x) {MV<VAL_T> V = op<VAL_T>(OPs::OP_MUL, *this, x); this->v = getV(); return *this;}
             MV<VAL_T> &operator/=(const MV<VAL_T> &x) {MV<VAL_T> V = op<VAL_T>(OPs::OP_DIV, *this, x); this->v = getV(); return *this;}

             MV<VAL_T> operator+(const MV<VAL_T> &x) const {return op<VAL_T>(OPs::OP_ADD, *this, x);}
             MV<VAL_T> operator-(const MV<VAL_T> &x) const {return op<VAL_T>(OPs::OP_SUB, *this, x);}
             MV<VAL_T> operator*(const MV<VAL_T> &x) const {return op<VAL_T>(OPs::OP_MUL, *this, x);}
             MV<VAL_T> operator/(const MV<VAL_T> &x) const {return op<VAL_T>(OPs::OP_DIV, *this, x);}
             MV<VAL_T> operator-() const {return op<VAL_T>(OPs::OP_NEG, *this);}
       };




     //State based
       template<class STATE_T> void toMatrix(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src); 
       template<class STATE_T> void toDiagonalMatrix(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src); 
       template<class STATE_T> void clone(STATE_T &s, NAME_T<STATE_T> &dstName, const NAME_T<STATE_T> srcName);
       template<class STATE_T> void appendRows(STATE_T &s, const NAME_T<STATE_T> x, const NAME_T<STATE_T> app);
       template<class STATE_T> void appendColumns(STATE_T &s, const NAME_T<STATE_T> x, const NAME_T<STATE_T> app);
       template<class STATE_T> void appendRow(STATE_T &s, const NAME_T<STATE_T> x, const NAME_T<STATE_T> app);
       template<class STATE_T> void appendColumn(STATE_T &s, const NAME_T<STATE_T> x, const NAME_T<STATE_T> app);
       template<class STATE_T> size_t numDims(STATE_T &s, const NAME_T<STATE_T> x);
       template<class STATE_T> size_t size(STATE_T &s, const NAME_T<STATE_T> x, size_t dim);
       template<class STATE_T, class CONST_T> bool create(STATE_T &s, const NAME_T<STATE_T> dst, size_t r, size_t c, const CONST_T constantVal); 
       template<class STATE_T> bool create(STATE_T &s, const NAME_T<STATE_T> dst, size_t r, size_t c, SPECIAL_MATRICES init); 
       template<class STATE_T> bool create(STATE_T &s, const NAME_T<STATE_T> dst, size_t r, size_t c, INITIALIZATIONs init);
       template<class STATE_T, class CONST_T> bool create(STATE_T &s, const NAME_T<STATE_T> dst, size_t sz, bool isRowVector, const CONST_T constantVal);  
       template<class STATE_T> bool create(STATE_T &s, const NAME_T<STATE_T> dst, size_t sz, bool isRowVector, INITIALIZATIONs init);  
       template<class STATE_T, class CONST_T> bool set(STATE_T &s, const NAME_T<STATE_T> dst, const CONST_T src1);
       template<class STATE_T, class CONST_T> bool set(STATE_T &s, const NAME_T<STATE_T> dst, const CONST_T *src1, size_t n);
       template<class STATE_T, class CONST_T> void set(STATE_T &s, const NAME_T<STATE_T> dst, size_t i, const CONST_T y);
       template<class STATE_T, class CONST_T> void set(STATE_T &s, const NAME_T<STATE_T> dst, size_t r, size_t c, const CONST_T y);
       template<class STATE_T, class CONST_T> void set(STATE_T &s, const NAME_T<STATE_T> dst, size_t rb, size_t r_len, size_t cb, size_t c_len, const CONST_T y);
       template<class STATE_T> double toDouble(STATE_T &s, const NAME_T<STATE_T> src);
       template<class STATE_T> bool row(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src, size_t r); 
       template<class STATE_T> bool col(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src, size_t c); 
       template<class STATE_T, class CONST_T> CONST_T get(STATE_T &s, const NAME_T<STATE_T> src, size_t i);
       template<class STATE_T, class CONST_T> CONST_T get(STATE_T &s, const NAME_T<STATE_T> src, size_t r, size_t c);  
       template<class STATE_T> bool mode(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src, size_t i); 
       template<class STATE_T> std::string toStringVar(STATE_T &s, const NAME_T<STATE_T> x); 
 
       template<class STATE_T, class OP> bool opReduce(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src1);
       template<class STATE_T, class OP> bool opElem(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src0, const NAME_T<STATE_T> src1);
       template<class STATE_T, class OP, class CONST_T> bool opConst(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src0, const CONST_T src1);
       template<class STATE_T, class OP, class CONST_T> bool opConst(STATE_T &s, const NAME_T<STATE_T> dst, const CONST_T src0, const NAME_T<STATE_T> src1);
       template<class STATE_T, class OP> bool op(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src0, const NAME_T<STATE_T> src1);      
       template<class STATE_T, class OP> bool op(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src);   
       //Guarded specialization
         template <class STATE_T, class OP> typename std::enable_if<state_has_op<STATE_T, OP>::value, bool>::type
         gOpState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src) {return op<STATE_T, OP>(s, dst, src);}
         template <class STATE_T, class OP> typename std::enable_if<!state_has_op<STATE_T, OP>::value, bool>::type
         gOpState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src) {throw MatWlibException("Error trying to execute non-supported operation");}

         template <class STATE_T, class OP> typename std::enable_if<state_has_op<STATE_T, OP>::value, bool>::type
         gOpState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src0, const NAME_T<STATE_T> src1) {return op<STATE_T, OP>(s, dst, src0, src1);}
         template <class STATE_T, class OP> typename std::enable_if<!state_has_op<STATE_T, OP>::value, bool>::type
         gOpState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src0, const NAME_T<STATE_T> src1) {throw MatWlibException("Error trying to execute non-supported operation");}

         template <class STATE_T, class OP, class CONST_T> typename std::enable_if<state_has_op<STATE_T, OP>::value, bool>::type
         gOpConstState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src0, const CONST_T src1) {return op<STATE_T, OP>(s, dst, src0, src1);}
         template <class STATE_T, class OP, class CONST_T> typename std::enable_if<!state_has_op<STATE_T, OP>::value, bool>::type
         gOpConstState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src0, const CONST_T src1) {throw MatWlibException("Error trying to execute non-supported operation");}

         template <class STATE_T, class OP, class CONST_T> typename std::enable_if<state_has_op<STATE_T, OP>::value, bool>::type
         gOpConstState(STATE_T &s, const NAME_T<STATE_T> dst, const CONST_T src0, const NAME_T<STATE_T> src1) {return op<STATE_T, OP>(s, dst, src0, src1);}
         template <class STATE_T, class OP, class CONST_T> typename std::enable_if<!state_has_op<STATE_T, OP>::value, bool>::type
         gOpConstState(STATE_T &s, const NAME_T<STATE_T> dst, const CONST_T src0, const NAME_T<STATE_T> src1) {throw MatWlibException("Error trying to execute non-supported operation");}

         template <class STATE_T, class OP> typename std::enable_if<state_has_op<STATE_T, OP>::value, bool>::type
         gOpElemState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src0, const NAME_T<STATE_T> src1) {return opElem<STATE_T, OP>(s, dst, src0, src1);}
         template <class STATE_T, class OP> typename std::enable_if<!state_has_op<STATE_T, OP>::value, bool>::type
         gOpElemState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src0, const NAME_T<STATE_T> src1) {throw MatWlibException("Error trying to execute non-supported element-wise operation");}

         template <class STATE_T, class OP> typename std::enable_if<state_has_op<STATE_T, OP>::value, bool>::type
         gOpElemState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src) {return opElem<STATE_T, OP>(s, dst, src);}
         template <class STATE_T, class OP> typename std::enable_if<!state_has_op<STATE_T, OP>::value, bool>::type
         gOpElemState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src) {throw MatWlibException("Error trying to execute non-supported element-wise operation");}

         template <class STATE_T, class OP> typename std::enable_if<state_has_op<STATE_T, OP>::value, bool>::type
         gOpReduceState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src) {return opReduce<STATE_T, OP>(s, dst, src);}
         template <class STATE_T, class OP> typename std::enable_if<!state_has_op<STATE_T, OP>::value, bool>::type
         gOpReduceState(STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src) {throw MatWlibException("Error trying to execute non-supported reduce operation");}
       //Multiple select
         template<class STATE_T> bool op(OPs o, STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src0, const NAME_T<STATE_T> src1) {
           switch(o) {
            case OPs::OP_ADD:  return gOpState<STATE_T, OP_T<OPs::OP_ADD>>(s, dst, src0, src1);
            case OPs::OP_SUB:  return gOpState<STATE_T, OP_T<OPs::OP_SUB>>(s, dst, src0, src1);
            case OPs::OP_MUL:  return gOpState<STATE_T, OP_T<OPs::OP_MUL>>(s, dst, src0, src1);
            case OPs::OP_DIV:  return gOpState<STATE_T, OP_T<OPs::OP_DIV>>(s, dst, src0, src1);
            case OPs::OP_AND:  return gOpState<STATE_T, OP_T<OPs::OP_AND>>(s, dst, src0, src1);
            case OPs::OP_OR:   return gOpState<STATE_T, OP_T<OPs::OP_OR>>(s, dst, src0, src1);
            case OPs::OP_XOR:  return gOpState<STATE_T, OP_T<OPs::OP_XOR>>(s, dst, src0, src1);
            default:                   throw MatWlibException("Invalid OP, not binary matrix-matrix function");
           }
         }
         template<class STATE_T> bool op(OPs o, STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src) {
           switch(o) {
            case OPs::OP_NEG:       return gOpState<STATE_T, OP_T<OPs::OP_NEG>>(s, dst, src);
            case OPs::OP_NOT:       return gOpState<STATE_T, OP_T<OPs::OP_NOT>>(s, dst, src);
            case OPs::OP_CONJ:      return gOpState<STATE_T, OP_T<OPs::OP_CONJ>>(s, dst, src);
            case OPs::OP_TRANS:     return gOpState<STATE_T, OP_T<OPs::OP_TRANS>>(s, dst, src);
            case OPs::OP_INVERSE:   return gOpState<STATE_T, OP_T<OPs::OP_INVERSE>>(s, dst, src);
            default:                throw MatWlibException("Invalid OP, not unary matrix function");
           }
         }

         template<class STATE_T, class CONST_T> bool opConst(OPs o, STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> &src0, const CONST_T src1) {
           switch(o) {
            case OPs::OP_SCALE:        return gOpConstState<STATE_T, OP_T<OPs::OP_SCALE>>(s, dst, src0, src1);
            case OPs::OP_TRANSLATE:    return gOpConstState<STATE_T, OP_T<OPs::OP_TRANSLATE>>(s, dst, src0, src1);
            default:                   throw MatWlibException("Invalid OP, not binary matrix-constant function");
           }
         }
         template<class STATE_T, class CONST_T> bool opConst(OPs o, STATE_T &s, const NAME_T<STATE_T> dst, const CONST_T &src0, const NAME_T<STATE_T> src1) {
           switch(o) {
            case OPs::OP_SCALE:        return gOpConstState<STATE_T, OP_T<OPs::OP_SCALE>>(s, dst, src0, src1);
            case OPs::OP_TRANSLATE:    return gOpConstState<STATE_T, OP_T<OPs::OP_TRANSLATE>>(s, dst, src0, src1);
            default:                   throw MatWlibException("Invalid OP, not binary constant-matrix function");
           }
         }

         template<class STATE_T> bool opElem(OPs o, STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src0, const NAME_T<STATE_T> src1) {
           switch(o) {
            case OPs::OP_MUL:    return gOpElemState<STATE_T, OP_T<OPs::OP_MUL>>(s, dst, src0, src1);
            case OPs::OP_DIV:    return gOpElemState<STATE_T, OP_T<OPs::OP_DIV>>(s, dst, src0, src1);
            case OPs::OP_MIN:    return gOpElemState<STATE_T, OP_T<OPs::OP_MIN>>(s, dst, src0, src1);
            case OPs::OP_MAX:    return gOpElemState<STATE_T, OP_T<OPs::OP_MAX>>(s, dst, src0, src1);
            default:                   throw MatWlibException("Invalid element-wise OP, not binary function");
           }
         }
         template<class STATE_T> bool opElem(OPs o, STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src) {
           switch(o) {
            case OPs::OP_SQRT:   return gOpElemState<STATE_T, OP_T<OPs::OP_SQRT>>(s, dst, src);
            case OPs::OP_ABS:    return gOpElemState<STATE_T, OP_T<OPs::OP_ABS>>(s, dst, src);
            default:                   throw MatWlibException("Invalid element-wise OP, not unary function");
           }
         }

         template<class STATE_T> bool opReduce(OPs o, STATE_T &s, const NAME_T<STATE_T> dst, const NAME_T<STATE_T> src) {
           switch(o) {
            case OPs::OP_ADD:  return gOpReduceState<STATE_T, OP_T<OPs::OP_ADD>>(s, dst, src);
            case OPs::OP_MUL:  return gOpReduceState<STATE_T, OP_T<OPs::OP_MUL>>(s, dst, src);
            case OPs::OP_MIN:  return gOpReduceState<STATE_T, OP_T<OPs::OP_MIN>>(s, dst, src);
            case OPs::OP_MAX:  return gOpReduceState<STATE_T, OP_T<OPs::OP_MAX>>(s, dst, src);
            default:           throw MatWlibException("Invalid reduction OP, not unary vector function");
           }
         }
     
}


#endif

