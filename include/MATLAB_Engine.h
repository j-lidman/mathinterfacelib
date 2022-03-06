#ifndef MATLAB_HDR
#define MATLAB_HDR

#ifdef HAS_MATLAB

#include <stdexcept>
#include <string>

#include "mathinterfacelib.h"

#include "engine.h"

//MATLAB API documentation: http://www.mathworks.com/help/pdf_doc/matlab/apiext.pdf

//#define MATHINTERFACELIB_MATLAB_DEBUG

#define MATHINTERFACELIB_MATLAB_TEMP_VAR "MATHINTERFACELIBINTERNALTMPVAR"
#define MATHINTERFACELIB_MATLAB_ERR_VAR MATHINTERFACELIBINTERNALERRVAR"

namespace MathInterfaceLib {
   //Engine handling functions
     typedef int (*engEvalString_Ptr)(Engine *, const char *);
     typedef int(*engOutputBuffer_Ptr)(Engine *ep, char *buffer, int buflen);
     typedef Engine *(*engOpen_Ptr)(const char *);
     typedef int (*engSetVisible_Ptr)(Engine *, bool);
     typedef int (*engClose_Ptr)(Engine *);
     typedef mxArray *(*engGetVariable_Ptr)(Engine *, const char *);
     typedef int (*engPutVariable_Ptr)(Engine	*, const char *, const mxArray *);

   //Matrix handling functions
     typedef mwIndex (*mxCalcSingleSubscript_Ptr)(const mxArray *pa, mwSize nsubs, const mwIndex *subs);
     typedef mxArray *(*mxGetCell_Ptr)(const mxArray *pa, mwIndex i);
     typedef mwSize (*mxGetNumberOfDimensions_Ptr)(const mxArray *pa);
     typedef const mwSize *(*mxGetDimensions_Ptr)(const mxArray *pa);
     typedef mxArray *(*mxCreateNumericMatrix_Ptr)(mwSize, mwSize, mxClassID, mxComplexity);
     typedef mxArray *(*mxCreateDoubleScalar_Ptr)(double);
     typedef void (*mxDestroyArray_Ptr)(mxArray *);
     typedef double *(*mxGetPr_Ptr)(const mxArray *);
     typedef size_t (*mxGetN_Ptr)(const mxArray *);
     typedef size_t (*mxGetM_Ptr)(const mxArray *);
     typedef mxClassID (*mxGetClassID_Ptr)(const mxArray *);
     typedef const char *(*mxGetClassName_Ptr)(const mxArray *);
     typedef char *(*mxArrayToString_Ptr)(const mxArray *);
     typedef mxArray *(*mxCreateString_Ptr)(const char *);
     typedef void (*mxFree_Ptr)(void *);
     typedef void *(*mxMalloc_Ptr)(size_t);

   struct MATLAB_Interface {
      void *handleEng, *handleMX, *handleMat;

      engOutputBuffer_Ptr engOutputBuffer;
      engEvalString_Ptr engEvalString;
      engOpen_Ptr engOpen;
      engSetVisible_Ptr engSetVisible;
      engClose_Ptr engClose;
      engGetVariable_Ptr engGetVariable;
      engPutVariable_Ptr engPutVariable;

      mxCalcSingleSubscript_Ptr mxCalcSingleSubscript;
      mxCreateNumericMatrix_Ptr mxCreateNumericMatrix;
      mxCreateDoubleScalar_Ptr mxCreateDoubleScalar;
      mxDestroyArray_Ptr mxDestroyArray;
      mxGetPr_Ptr mxGetPr;
      mxGetM_Ptr mxGetM;
      mxGetN_Ptr mxGetN;
      mxGetCell_Ptr mxGetCell;
      mxGetClassID_Ptr mxGetClassID;
      mxGetClassName_Ptr mxGetClassName;
      mxGetDimensions_Ptr mxGetDimensions;
      mxGetNumberOfDimensions_Ptr mxGetNumberOfDimensions;
      mxArrayToString_Ptr mxArrayToString;
      mxCreateString_Ptr mxCreateString;
      mxFree_Ptr mxFree;
      mxMalloc_Ptr mxMalloc;

      MATLAB_Interface() : handleEng(NULL), handleMX(NULL), handleMat(NULL),
                           //Engine functions
                             engEvalString(NULL), engOpen(NULL), engSetVisible(NULL), 
                             engClose(NULL), engGetVariable(NULL), engPutVariable(NULL),
                           //Matrix functions
                             mxCreateNumericMatrix(NULL), mxCreateDoubleScalar(NULL), mxDestroyArray(NULL),
                             mxGetPr(NULL), mxGetM(NULL), mxGetN(NULL), mxGetClassID(NULL), mxGetClassName(NULL),
                             mxArrayToString(NULL), mxCreateString(NULL), mxFree(NULL), mxMalloc(NULL) {}
      ~MATLAB_Interface() {}
   };

   class MATLAB_State : public State,
                        public FunctionInterface,
                        public SessionInterface,
                        public RawDataInterface<std::string>,
                        public RawExecuteInterface {
      public:
         typedef std::string NAME;
      private:
         NAME errVariable, tmpVariable;

         size_t opId;
         MATLAB_Interface *extint;     
         Engine *eng; 
         boost::interprocess::shared_memory_object *shm; 
         std::map<std::string, Function *> funcMap;
      public:
         //(De/Con)structor(s)
           MATLAB_State(MATLAB_Interface *extint_, Engine *eng_, boost::interprocess::shared_memory_object *shm_);
           virtual ~MATLAB_State();
         //Write data
           virtual void writeVar(const std::string strVarName, std::string val);
           virtual void writeVar(const std::string strVarName, double val);
           virtual void writeVar(const std::string strVarName, size_t val);
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, const double *mat);
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, const size_t *mat);  
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, FWD_IT_T<double> begin, FWD_IT_T<double> end);
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, FWD_IT_T<size_t> begin, FWD_IT_T<size_t> end);
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, CONST_FWD_IT_T<double> begin, CONST_FWD_IT_T<double> end);
           virtual void writeVar(const std::string strVarName, size_t m, size_t n, CONST_FWD_IT_T<size_t> begin, CONST_FWD_IT_T<size_t> end);
         //Read data
           virtual std::string readStrVar(const std::string strVarName);
           virtual double readDoubleVar(const std::string strVarName);
           virtual size_t readIntVar(const std::string strVarName);
           virtual double *readDoubleArr(const std::string strVarName, std::vector<size_t> &szDims);
           virtual size_t *readIntArr(const std::string strVarName, std::vector<size_t> &szDims);
         //Misc. functions
           NAME getTempVar() const;
           NAME getNewName();
           virtual std::string toString() const;
           virtual bool good() const;

           virtual std::string toStringVariable(const std::string strVarName) const;

           MATLAB_Interface *getExtInt() const;
           Engine *getEng() const;
           template<class T> T *decodeArray(mxArray *arr, std::vector<size_t> &szDims);
           template<class T> T decodeValue(mxArray *arr);
           TYPE decodeType(mxArray *arr);
           boost::interprocess::shared_memory_object *getSHM();
         //Function handling
           bool registerFunction(std::string funcName, Function *funcDesc);
           bool updateFunction(std::string funcName, Function::FUNC_T fn);
           bool removeFunction(std::string funcName);
         //Value handling
           virtual Value *decodeData(Data *d);
         //Variable handling
           virtual void writeVar(std::string varName, Data *d);
           virtual Data *readVar(std::string varName);
           virtual void writeVar(std::string varName, RawDataInterface<std::string>::CELL_FUNC_T fn);
           virtual void readVar(std::string varName, RawDataInterface<std::string>::CELL_FUNC_T fn);
         //Command handling
           virtual void execute(std::string cmd);  
           void executeWithoutListen(std::string cmd);
         //Operations
           bool opAdd(const NAME dstVar, const NAME srcVar0, const NAME srcVar1);
           bool opSub(const NAME dstVar, const NAME srcVar0, const NAME srcVar1);
           bool opMul(const NAME dstVar, const NAME srcVar0, const NAME srcVar1);
           bool opDiv(const NAME dstVar, const NAME srcVar0, const NAME srcVar1);
           bool opNeg(const NAME dstVar, const NAME srcVar);
           bool opTrans(const NAME dstVar, const NAME srcVar);
           bool opConj(const NAME dstVar, const NAME srcVar);
           bool opInverse(const NAME dstVar, const NAME srcVar);
           bool opScale(const NAME dstVar, const NAME srcVar, const double c);
           bool opTranslate(const NAME dstVar, const NAME srcVar, const double c);
   };
   template<> double *MATLAB_State::decodeArray<double>(mxArray *arr, std::vector<size_t> &szDims);
   template<> size_t *MATLAB_State::decodeArray<size_t>(mxArray *arr, std::vector<size_t> &szDims);
   template<> std::string *MATLAB_State::decodeArray<std::string>(mxArray *arr, std::vector<size_t> &szDims);
   template<> std::string MATLAB_State::decodeValue<std::string>(mxArray *arr);
   template<> size_t MATLAB_State::decodeValue<size_t>(mxArray *arr);
   template<> double MATLAB_State::decodeValue<double>(mxArray *arr);


   class MATLAB_Data : public Data {
      private:
         MATLAB_State *s;
         mxArray *arr;
      public:
         MATLAB_Data(MATLAB_State *s_, mxArray *arr_) : s(s_), arr(arr_) {}
         virtual ~MATLAB_Data() {
            s->getExtInt()->mxDestroyArray(arr);
         }
         mxArray *getArray() {return arr;}
   };


   MATLAB_State *init_matlab_engine(std::string path, boost::interprocess::shared_memory_object *shm);
   void finit_matlab_engine(MATLAB_State *s);

   //Type traits
     template<> struct has_op<MATLAB_State, OP_T<OPs::OP_ADD>> : public boost::true_type {};
     template<> struct has_op<MATLAB_State, OP_T<OPs::OP_SUB>> : public boost::true_type {};
     template<> struct has_op<MATLAB_State, OP_T<OPs::OP_MUL>> : public boost::true_type {};
     template<> struct has_op<MATLAB_State, OP_T<OPs::OP_DIV>> : public boost::true_type {};

     template<> struct has_op<MATLAB_State, OP_T<OPs::OP_NEG>> : public boost::true_type {};
     template<> struct has_op<MATLAB_State, OP_T<OPs::OP_CONJ>> : public boost::true_type {};
     template<> struct has_op<MATLAB_State, OP_T<OPs::OP_TRANS>> : public boost::true_type {};
     template<> struct has_op<MATLAB_State, OP_T<OPs::OP_INVERSE>> : public boost::true_type {};
     template<> struct has_op<MATLAB_State, OP_T<OPs::OP_SCALE>> : public boost::true_type {};
     template<> struct has_op<MATLAB_State, OP_T<OPs::OP_TRANSLATE>> : public boost::true_type {};
}

#endif

#endif

