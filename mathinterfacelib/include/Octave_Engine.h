#ifndef OCTAVE_HDR
#define OCTAVE_HDR

#ifdef HAS_OCTAVE

#include <stdexcept>
#include <string>

#include "matwlib.h"

#define OCTAVE_USE_INTERPRETER_BY_DEFAULT true

class Matrix;

namespace MatWLib {

   typedef Matrix OCT_MAT_T;
   typedef Matrix OCT_VEC_T;

   class Octave_State : public State,
                        public FunctionInterface,
                        public SessionInterface {
     public:
        typedef std::string NAME;

        typedef OCT_MAT_T MATRIX_T;
        typedef OCT_VEC_T VECTOR_T;
     private:

     public:
         //(De/Con)structor(s)
           Octave_State();
           virtual ~Octave_State();
         //Function handling
           bool registerFunction(std::string funcName, Function *funcDesc);
           bool updateFunction(std::string funcName, Function::FUNC_T fn);
           bool removeFunction(std::string funcName);
   };

   /*
   class Octave_StateBasic : public Octave_State {
      private:
         size_t opId;
         std::map<std::string, Function *> funcMap;
         std::map<std::string, OCT_MAT_T> mapMatrices;
         std::map<std::string, OCT_VEC_T> mapVectors;

         MATRIX_T &getMat(std::string varName);
         const MATRIX_T &getMat(std::string varName) const;
         VECTOR_T &getVec(std::string varName);
         const VECTOR_T &getVec(std::string varName) const;
      public:
         //(De/Con)structor(s)
           Octave_StateBasic();
           virtual ~Octave_StateBasic();
         //Misc. functions
           virtual bool good() const;
           virtual std::string toString() const;
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
         //Value handling
           virtual Value *decodeData(Data *d);
         //Variable handling
           void addVar(std::string varName, OCT_MAT_T mat);
           //void addVar(std::string varName, OCT_VEC_T vec);
           void remVarMat(std::string varName);
           void remVarVec(std::string varName);
           const OCT_MAT_T &getVarMat(std::string varName) const;
           OCT_MAT_T &getVarMat(std::string varName);
           OCT_VEC_T &getVarVec(std::string varName);
           const OCT_VEC_T &getVarVec(std::string varName) const;
         //Command handling
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
   };*/

   class Octave_StateInterpreter : public Octave_State,
                                   public RawExecuteInterface {
      private:
         NAME tmpVariable;
      public:
         //(De/Con)structor(s)
           Octave_StateInterpreter();
           virtual ~Octave_StateInterpreter();
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
           virtual bool good() const;
           virtual std::string toString() const;
           std::string toStringVariable(const std::string strVarName) const;
         //Value handling
           virtual Value *decodeData(Data *d);
         //Variable handling

         //Command handling
           virtual void execute(std::string cmd);  
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


   Octave_State *init_octave_state_engine(bool useInterpreter = true);
   void finit_octave_engine(Octave_State *s);
}

#endif
#endif

