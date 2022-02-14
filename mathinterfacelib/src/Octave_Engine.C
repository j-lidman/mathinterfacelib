#ifdef HAS_OCTAVE

#include <sstream>
#include <vector>
//#include <string.h>
#include <thread>
#include <stack>

/*
#include <octave/octave.h>
#include <octave/parse.h>
#include <octave/interpreter.h>
*/

//#include <octave/oct.h>
#include <oct.h>

#include <octave.h>
#include <parse.h>
#include <interpreter.h>

#include "Octave_Engine.h"

using namespace std;
using namespace boost;

namespace MatWLib {
   Octave_State *init_octave_state_engine(bool useInterpreter) {
      if(useInterpreter)
         return new Octave_StateInterpreter();
      else
          assert(false); //return new Octave_StateBasic();
   }
   void finit_octave_engine(Octave_State *s) {
      delete s;
   }

   /*template<> Octave_StateBasic *cast_state<Octave_StateBasic>(State *s) {
      Octave_StateBasic *s_ = dynamic_cast<Octave_StateBasic *>(s);
      assert((s_ != NULL) && "Invalid Octave Basic State");
      return s_;
   }*/
   template<> Octave_StateInterpreter *cast_state<Octave_StateInterpreter>(State *s) {
      Octave_StateInterpreter *s_ = dynamic_cast<Octave_StateInterpreter *>(s);
      assert((s_ != NULL) && "Invalid Octave Interpreted State");
      return s_;
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////

   //BaseState
   Octave_State::Octave_State() {

   }
   Octave_State::~Octave_State() {

   }
   bool Octave_State::registerFunction(std::string funcName, Function *funcDesc) {

   }
   bool Octave_State::updateFunction(std::string funcName, Function::FUNC_T fn) {

   }
   bool Octave_State::removeFunction(std::string funcName) {

   }

   /*
   OCT_MAT_T &Octave_StateBasic::getMat(std::string varName) {
     std::map<std::string, OCT_MAT_T>::iterator it;
     if((it = mapMatrices.find(varName)) == mapMatrices.end())
         throw MatWlibException("No matrix variable with this name has been defined");
     return it->second;
   }
   const OCT_MAT_T &Octave_StateBasic::getMat(std::string varName) const {
     std::map<std::string, OCT_MAT_T>::const_iterator it;
     if((it = mapMatrices.find(varName)) == mapMatrices.end())
         throw MatWlibException("No matrix variable with this name has been defined");
     return it->second;
   }
   OCT_VEC_T &Octave_StateBasic::getVec(std::string varName) {
     std::map<std::string, OCT_VEC_T>::iterator it;
     if((it = mapVectors.find(varName)) == mapVectors.end())
         throw MatWlibException("No vector variable with this name has been defined");
     return it->second;
   }
   const OCT_VEC_T &Octave_StateBasic::getVec(std::string varName) const {
     std::map<std::string, OCT_VEC_T>::const_iterator it;
     if((it = mapVectors.find(varName)) == mapVectors.end())
         throw MatWlibException("No vector variable with this name has been defined");
     return it->second;
   }

   Octave_StateBasic::Octave_StateBasic() {}
   Octave_StateBasic::~Octave_StateBasic() {}

   std::string Octave_StateBasic::toString() const {return "Octave State/Basic";}
   bool Octave_StateBasic::good() const {return true;}


   void Octave_StateBasic::writeVar(const std::string strVarName, std::string val) {

   }
   void Octave_StateBasic::writeVar(const std::string strVarName, double val) {
     OCT_MAT_T newMat(1,1);
     newMat(0,0) = val;
     this->addVar(strVarName, newMat);
   }
   void Octave_StateBasic::writeVar(const std::string strVarName, size_t val) {
     OCT_MAT_T newMat(1,1);
     newMat(0,0) = val;
     this->addVar(strVarName, newMat);
   }
   void Octave_StateBasic::writeVar(const std::string strVarName, size_t m, size_t n, const double *mat) {
     OCT_MAT_T newMat(m,n);
     for(size_t i = 0; i < m; i++)
         for(size_t j = 0; j < n; j++)
             newMat(i,j) = mat[i*n + j];
     this->addVar(strVarName, newMat);
   }
   void Octave_StateBasic::writeVar(const std::string strVarName, size_t m, size_t n, const size_t *mat) {
     OCT_MAT_T newMat(m,n);
     for(size_t i = 0; i < m; i++)
         for(size_t j = 0; j < n; j++)
             newMat(i,j) = mat[i*n + j];
     this->addVar(strVarName, newMat);
   } 
   void Octave_StateBasic::writeVar(const std::string strVarName, size_t m, size_t n, FWD_IT_T<double> begin, FWD_IT_T<double> end) {

   }
   void Octave_StateBasic::writeVar(const std::string strVarName, size_t m, size_t n, FWD_IT_T<size_t> begin, FWD_IT_T<size_t> end) {

   }
   void Octave_StateBasic::writeVar(const std::string strVarName, size_t m, size_t n, CONST_FWD_IT_T<double> begin, CONST_FWD_IT_T<double> end) {

   }
   void Octave_StateBasic::writeVar(const std::string strVarName, size_t m, size_t n, CONST_FWD_IT_T<size_t> begin, CONST_FWD_IT_T<size_t> end) {

   }
   std::string Octave_StateBasic::readStrVar(const std::string strVarName) {
     assert(false);
   }
   double Octave_StateBasic::readDoubleVar(const std::string strVarName) {
     assert(false);
   }
   size_t Octave_StateBasic::readIntVar(const std::string strVarName) {
     assert(false);
   }
   double *Octave_StateBasic::readDoubleArr(const std::string strVarName, std::vector<size_t> &szDims) {
     assert(false);
   }
   size_t *Octave_StateBasic::readIntArr(const std::string strVarName, std::vector<size_t> &szDims) {
     assert(false);
   }

   Value *Octave_StateBasic::decodeData(Data *d) {assert(false);}

   void Octave_StateBasic::addVar(std::string varName, OCT_MAT_T mat) {
     std::map<std::string, OCT_MAT_T>::iterator it;
     if((it = mapMatrices.find(varName)) == mapMatrices.end())
         mapMatrices[varName] = mat;
     else
         it->second = mat;
   }
   //void Octave_StateBasic::addVar(std::string varName, OCT_VEC_T vec) {
   //  std::map<std::string, OCT_VEC_T>::iterator it;
   //  if((it = mapVectors.find(varName)) == mapVectors.end())
   //      mapVectors[varName] = vec;
   //  else
   //      it->second = vec;
   //}
   void Octave_StateBasic::remVarMat(std::string varName) {
     std::map<std::string, OCT_MAT_T>::iterator it;
     if((it = mapMatrices.find(varName)) != mapMatrices.end())
         mapMatrices.erase(it);
   }
   void Octave_StateBasic::remVarVec(std::string varName) {
     std::map<std::string, OCT_VEC_T>::iterator it;
     if((it = mapVectors.find(varName)) != mapVectors.end())
         mapVectors.erase(it);
   }
   const OCT_MAT_T &Octave_StateBasic::getVarMat(std::string varName) const {return getMat(varName);}
   OCT_MAT_T &Octave_StateBasic::getVarMat(std::string varName) {return getMat(varName);}
   OCT_VEC_T &Octave_StateBasic::getVarVec(std::string varName) {return getVec(varName);}
   const OCT_VEC_T &Octave_StateBasic::getVarVec(std::string varName) const {return getVec(varName);}

   bool Octave_StateBasic::opAdd(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {
     try {
         getMat(dstVar) = getMat(srcVar0) + getMat(srcVar1);
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateBasic::opSub(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {
     try {
         getMat(dstVar) = getMat(srcVar0) - getMat(srcVar1);
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateBasic::opMul(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {
     try {
         getMat(dstVar) = getMat(srcVar0) * getMat(srcVar1);
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateBasic::opDiv(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {
     assert(false);
   }
   bool Octave_StateBasic::opNeg(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {
     try {
         getMat(dstVar) = -getMat(srcVar);
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateBasic::opTrans(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {
     try {
         //getMat(dstVar) = getMat(srcVar).transpose();
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateBasic::opConj(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {
     assert(false);
     return true;
   }
   bool Octave_StateBasic::opInverse(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {
     try {
         //getMat(dstVar) = getMat(srcVar).inverse();
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }

   bool Octave_StateBasic::opScale(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar, const double c) {
     try {
         getMat(dstVar) = getMat(srcVar) * c;
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateBasic::opTranslate(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar, const double c) {
     try {
         getMat(dstVar) = getMat(srcVar) + c;
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }*/


   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////// StateInterpreter //////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   Octave_StateInterpreter::Octave_StateInterpreter() {
      std::stringstream ss;
      ss << "TMPVAR" << getUniqueNumber();
      this->tmpVariable = ss.str();
   }
   Octave_StateInterpreter::~Octave_StateInterpreter() {}

   Octave_StateInterpreter::NAME Octave_StateInterpreter::getTempVar() const {
     return this->tmpVariable;
   }
   Octave_StateInterpreter::NAME Octave_StateInterpreter::getNewName() {
     std::stringstream ss;
     ss << "var" << getUniqueNumber();
     return ss.str();
   }
   bool Octave_StateInterpreter::good() const {return true;}
   std::string Octave_StateInterpreter::toString() const {return "Octave State/Interpreted";}
   std::string Octave_StateInterpreter::toStringVariable(const std::string strVarName) const {
     std::stringstream ss;
     ss << "?";
     assert(false);
     return ss.str();
   }

   void Octave_StateInterpreter::writeVar(const std::string strVarName, std::string val) {

   }
   void Octave_StateInterpreter::writeVar(const std::string strVarName, double val) {

   }
   void Octave_StateInterpreter::writeVar(const std::string strVarName, size_t val) {

   }
   void Octave_StateInterpreter::writeVar(const std::string strVarName, size_t m, size_t n, const double *mat) {

   }
   void Octave_StateInterpreter::writeVar(const std::string strVarName, size_t m, size_t n, const size_t *mat) {

   } 
   void Octave_StateInterpreter::writeVar(const std::string strVarName, size_t m, size_t n, FWD_IT_T<double> begin, FWD_IT_T<double> end) {

   }
   void Octave_StateInterpreter::writeVar(const std::string strVarName, size_t m, size_t n, FWD_IT_T<size_t> begin, FWD_IT_T<size_t> end) {

   }
   void Octave_StateInterpreter::writeVar(const std::string strVarName, size_t m, size_t n, CONST_FWD_IT_T<double> begin, CONST_FWD_IT_T<double> end) {

   }
   void Octave_StateInterpreter::writeVar(const std::string strVarName, size_t m, size_t n, CONST_FWD_IT_T<size_t> begin, CONST_FWD_IT_T<size_t> end) {

   }
   std::string Octave_StateInterpreter::readStrVar(const std::string strVarName) {

   }
   double Octave_StateInterpreter::readDoubleVar(const std::string strVarName) {

   }
   size_t Octave_StateInterpreter::readIntVar(const std::string strVarName) {

   }
   double *Octave_StateInterpreter::readDoubleArr(const std::string strVarName, std::vector<size_t> &szDims) {

   }
   size_t *Octave_StateInterpreter::readIntArr(const std::string strVarName, std::vector<size_t> &szDims) {

   }

   Value *Octave_StateInterpreter::decodeData(Data *d) {
     assert(false);
   }

   void Octave_StateInterpreter::execute(std::string cmd) {
     assert(false);
   }


   bool Octave_StateInterpreter::opAdd(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {
     try {

     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateInterpreter::opSub(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {
     try {

     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateInterpreter::opMul(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {
     try {

     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateInterpreter::opDiv(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {
     assert(false);
   }
   bool Octave_StateInterpreter::opNeg(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {
     try {

     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateInterpreter::opConj(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {
     try {

     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateInterpreter::opTrans(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {
     try {

     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateInterpreter::opInverse(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {
     try {

     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }

   bool Octave_StateInterpreter::opScale(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar, const double c) {
     try {

     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Octave_StateInterpreter::opTranslate(const Octave_State::NAME dstVar, const Octave_State::NAME srcVar, const double c) {
     try {

     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////// State //////////////////////////////////////////////
   //Basic
   /*
   template<> bool create<Octave_StateBasic>(Octave_StateBasic &s, const Octave_State::NAME dst, size_t r, size_t c, INITIALIZATIONs init) {
     switch(init) {
      case INITIALIZATIONs::INIT_RANDOM:     break;
      case INITIALIZATIONs::INIT_ZERO:         break;
      case INITIALIZATIONs::INIT_ONE:          break;
      case INITIALIZATIONs::INIT_NONE:               break;
     }
   }
   template<> bool set<Octave_StateBasic, double>(Octave_StateBasic &s, const Octave_State::NAME dst, const double src1) {

     return true;
   }
   template<> double toDouble<Octave_StateBasic>(Octave_StateBasic &s, const Octave_State::NAME src) {


   }
   template<> bool row<Octave_StateBasic>(Octave_StateBasic &s, const Octave_State::NAME dst, const Octave_State::NAME srcVar, size_t r) {

     return true;
   }
   template<> bool col<Octave_StateBasic>(Octave_StateBasic &s, const Octave_State::NAME dst, const Octave_State::NAME srcVar, size_t c) {

     return true;
   }
   //template<> bool elem<Octave_StateBasic>(Octave_StateBasic &s, const Octave_State::NAME dst, const Octave_State::NAME srcVar, size_t r, size_t c) {
   //  OCT_MAT_T &dstV = s.getVarMat(dst), &srcV = s.getVarMat(srcVar);
   //  dstV = src(r,c);
   //  return true;
   //}
   template<> bool opElem<Octave_StateBasic, OP_T<OPs::OP_MUL>>(Octave_StateBasic &s, const Octave_StateBasic::NAME dst, const Octave_StateBasic::NAME src0, const Octave_StateBasic::NAME src1) {assert(false);}
   template<> bool opElem<Octave_StateBasic, OP_T<OPs::OP_DIV>>(Octave_StateBasic &s, const Octave_StateBasic::NAME dst, const Octave_StateBasic::NAME src0, const Octave_StateBasic::NAME src1) {assert(false);}
   template<> bool opElem<Octave_StateBasic, OP_T<OPs::OP_MIN>>(Octave_StateBasic &s, const Octave_StateBasic::NAME dst, const Octave_StateBasic::NAME src0, const Octave_StateBasic::NAME src1) {assert(false);}
   template<> bool opElem<Octave_StateBasic, OP_T<OPs::OP_MAX>>(Octave_StateBasic &s, const Octave_StateBasic::NAME dst, const Octave_StateBasic::NAME src0, const Octave_StateBasic::NAME src1) {assert(false);}
   template<> bool opConst<Octave_StateBasic, OP_T<OPs::OP_SCALE>, double>(Octave_StateBasic &s, const Octave_StateBasic::NAME dst, const Octave_StateBasic::NAME src, const double c) {return s.opScale(dst, src, c);}
   template<> bool opConst<Octave_StateBasic, OP_T<OPs::OP_TRANSLATE>, double>(Octave_StateBasic &s, const Octave_StateBasic::NAME dst, const Octave_StateBasic::NAME src, const double c) {return s.opTranslate(dst, src, c);}
   template<> bool op<Octave_StateBasic, OP_T<OPs::OP_ADD>>(Octave_StateBasic &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {return s.opAdd(dstVar, srcVar0, srcVar1);}     
   template<> bool op<Octave_StateBasic, OP_T<OPs::OP_SUB>>(Octave_StateBasic &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {return s.opSub(dstVar, srcVar0, srcVar1);}  
   template<> bool op<Octave_StateBasic, OP_T<OPs::OP_MUL>>(Octave_StateBasic &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {return s.opMul(dstVar, srcVar0, srcVar1);}
   template<> bool op<Octave_StateBasic, OP_T<OPs::OP_DIV>>(Octave_StateBasic &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {return s.opDiv(dstVar, srcVar0, srcVar1);} 
   template<> bool op<Octave_StateBasic, OP_T<OPs::OP_NEG>>(Octave_StateBasic &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {return s.opNeg(dstVar, srcVar);} 
   template<> bool op<Octave_StateBasic, OP_T<OPs::OP_TRANS>>(Octave_StateBasic &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {return s.opTrans(dstVar, srcVar);}
   template<> bool op<Octave_StateBasic, OP_T<OPs::OP_CONJ>>(Octave_StateBasic &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {return s.opConj(dstVar, srcVar);}
   template<> bool op<Octave_StateBasic, OP_T<OPs::OP_INVERSE>>(Octave_StateBasic &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {return s.opInverse(dstVar, srcVar);}*/


   //Interpreter
   template<> void toMatrix<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dstName, const Octave_StateInterpreter::NAME srcName) {
     return;
   }
   template<> void clone<Octave_StateInterpreter>(Octave_StateInterpreter &s, Octave_StateInterpreter::NAME &dstName, const Octave_StateInterpreter::NAME srcName) {
     s.execute(dstName + " = " + srcName + ";");
   }
   template<> void appendRows<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME x, const Octave_StateInterpreter::NAME app) {
     s.execute(x + " = [" + x + "; " + app + "];");
   }
   template<> void appendColumns<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME x, const Octave_StateInterpreter::NAME app) {
     s.execute(x + " = [" + x + " " + app + "];");
   }
   template<> void appendRow<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME x, const Octave_StateInterpreter::NAME app) {
     appendRows(s, x, app);
   }
   template<> void appendColumn<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME x, const Octave_StateInterpreter::NAME app) {
     appendColumns(s, x, app);
   }
   template<> size_t numDims<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME x) {
     std::stringstream ss;
     ss << s.getTempVar() << " = ndims(" << x << ");";
     s.execute(ss.str());

     return (size_t) s.readDoubleVar(s.getTempVar());
   }
   template<> size_t size<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME x, size_t dim) {
     std::stringstream ss;
     ss << s.getTempVar() << " = size(" << x << ", " << dim << ");";
     s.execute(ss.str());

     return (size_t) s.readDoubleVar(s.getTempVar());
   }
   template<> bool create<Octave_StateInterpreter, double>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, size_t r, size_t c, double constantVal) {
     std::stringstream ss;
     ss << dst << "= " << constantVal << ".*ones(" << r << ", " << c << ");";
     s.execute(ss.str());
     return true;
   } 
   template<> bool create<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, size_t r, size_t c, SPECIAL_MATRICES init) {
     switch(init) {
      case SPECIAL_MATRICES::MATRIX_IDENT:
        {
         assert(r == c);

         std::stringstream ss;
         ss << dst << "= eye(" << r << ");";
         s.execute(ss.str());
        } break;
      case SPECIAL_MATRICES::MATRIX_HANKEL:
       assert(false && "TODO");
     }
     return true;
   } 
   template<> bool create<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, size_t r, size_t c, INITIALIZATIONs init) {
     switch(init) {
      case INITIALIZATIONs::INIT_RANDOM:
        {
         std::stringstream ss;
         ss << dst << "= rand(" << r << ", " << c << ");";
         s.execute(ss.str());
        } break;
      case INITIALIZATIONs::INIT_ZERO:
        {
         std::stringstream ss;
         ss << dst << "= zeros(" << r << ", " << c << ");";
         s.execute(ss.str());
        } break;
      case INITIALIZATIONs::INIT_ONE:
        {
         std::stringstream ss;
         ss << dst << "= ones(" << r << ", " << c << ");";
         s.execute(ss.str());
        } break;
      case INITIALIZATIONs::INIT_NONE:
        {
         std::stringstream ss;
         ss << dst << "= zeros(" << r << ", " << c << ");"; //TODO! Is there a non-specific init?
         s.execute(ss.str());
        } break;
     }
     return true;
   }
   template<> bool create<Octave_StateInterpreter, double>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, size_t sz, bool isRowVector, double constantVal) {
     std::stringstream ss;
     if(isRowVector)
        ss << dst << "= " << constantVal << ".*ones(1, " << sz << ");";
     else
         ss << dst << "= " << constantVal << ".*ones(" << sz << ", 1);";
     s.execute(ss.str());
     return true;
   } 
   template<> bool create<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, size_t sz, bool isRowVector, INITIALIZATIONs init) {
     switch(init) {
      case INITIALIZATIONs::INIT_RANDOM:
        {
         std::stringstream ss;
         if(isRowVector)
            ss << dst << "= rand(1, " << sz << ");";
         else
             ss << dst << "= rand(" << sz << ", 1);";
         s.execute(ss.str());
        } break;
      case INITIALIZATIONs::INIT_ZERO:
        {
         std::stringstream ss;
         if(isRowVector)
            ss << dst << "= zeros(1, " << sz << ");";
         else
             ss << dst << "= zeros(" << sz << ", 1);";
         s.execute(ss.str());
        } break;
      case INITIALIZATIONs::INIT_ONE:
        {
         std::stringstream ss;
         if(isRowVector)
            ss << dst << "= ones(1, " << sz << ");";
         else
             ss << dst << "= ones(" << sz << ", 1);";
         s.execute(ss.str());
        } break;
      case INITIALIZATIONs::INIT_NONE:
        {
         std::stringstream ss;
         if(isRowVector)
            ss << dst << "= zeros(1, " << sz << ");";
         else
             ss << dst << "= zeros(" << sz << ", 1);";
         s.execute(ss.str());
        } break;
     }
     return true;
   }  

   template<> bool set<Octave_StateInterpreter, double>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const double src1) {
     s.writeVar(dst, src1);
     return true;
   }
   template<> bool set<Octave_StateInterpreter, double>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const double *src1, size_t n) {
     assert(false);
   }
   template<> void set<Octave_StateInterpreter, double>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, size_t i, const double y) {
     std::stringstream ss;
     ss << dst << "(" << i << ") = " << y << ";";
     s.execute(ss.str());
   }
   template<> void set<Octave_StateInterpreter, double>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, size_t r, size_t c, const double y) {
     std::stringstream ss;
     ss << dst << "(" << r << ", " << c << ") = " << y << ";";
     s.execute(ss.str());
   }
   template<> void set<Octave_StateInterpreter, double>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, size_t rb, size_t r_len, size_t cb, size_t c_len, const double y) {
     assert(false);
   }

   template<> bool set<Octave_StateInterpreter, size_t>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const size_t src1) {
     s.writeVar(dst, src1);
     return true;
   }
   template<> bool set<Octave_StateInterpreter, size_t>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const size_t *src1, size_t n) {
     assert(false);
   }
   template<> void set<Octave_StateInterpreter, size_t>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, size_t i, const size_t y) {
     std::stringstream ss;
     ss << dst << "(" << i << ") = " << y << ";";
     s.execute(ss.str());
   }
   template<> void set<Octave_StateInterpreter, size_t>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, size_t r, size_t c, const size_t y) {
     std::stringstream ss;
     ss << dst << "(" << r << ", " << c << ") = " << y << ";";
     s.execute(ss.str());
   }
   template<> void set<Octave_StateInterpreter, size_t>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, size_t rb, size_t r_len, size_t cb, size_t c_len, const size_t y) {
     assert(false);
   }

   template<> bool set<Octave_StateInterpreter, std::string>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const std::string src1) {
     s.writeVar(dst, src1);
     return true;
   }
   template<> bool set<Octave_StateInterpreter, std::string>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const std::string *src1, size_t n) {
     assert(false);
   }



   template<> double toDouble<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME src) {
     assert(false);
   }
   template<> bool row<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const Octave_StateInterpreter::NAME srcVar, size_t r) {
     assert(false);
   }
   template<> bool col<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const Octave_StateInterpreter::NAME srcVar, size_t c) {
     assert(false);
   }
   template<> double get<Octave_StateInterpreter, double>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME src, size_t r, size_t c) {
     std::stringstream ss;
     ss << s.getTempVar() << " = " << src << "(" << r << ", " << c << ");";
     s.execute(ss.str());
     return s.readDoubleVar(s.getTempVar());
   }
   template<> double get<Octave_StateInterpreter, double>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME src, size_t i) {
     std::stringstream ss;
     ss << s.getTempVar() << " = " << src << "(" << i << ");";
     s.execute(ss.str());
     return s.readDoubleVar(s.getTempVar());
   }
   template<> size_t get<Octave_StateInterpreter, size_t>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME src, size_t r, size_t c) {
     std::stringstream ss;
     ss << s.getTempVar() << " = " << src << "(" << r << ", " << c << ");";
     s.execute(ss.str());
     return s.readIntVar(s.getTempVar());
   }
   template<> size_t get<Octave_StateInterpreter, size_t>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME src, size_t i) {
     std::stringstream ss;
     ss << s.getTempVar() << " = " << src << "(" << i << ");";
     s.execute(ss.str());
     return s.readIntVar(s.getTempVar());
   }

   template<> bool mode<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const Octave_StateInterpreter::NAME src, size_t i) {
     assert(false);
   } 
   template<> std::string toStringVar<Octave_StateInterpreter>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME x) {
     return s.toStringVariable(x);
   }

   template<> bool opElem<Octave_StateInterpreter, OP_T<OPs::OP_MUL>>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const Octave_StateInterpreter::NAME src0, const Octave_StateInterpreter::NAME src1) {assert(false);}
   template<> bool opElem<Octave_StateInterpreter, OP_T<OPs::OP_DIV>>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const Octave_StateInterpreter::NAME src0, const Octave_StateInterpreter::NAME src1) {assert(false);}
   template<> bool opElem<Octave_StateInterpreter, OP_T<OPs::OP_MIN>>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const Octave_StateInterpreter::NAME src0, const Octave_StateInterpreter::NAME src1) {assert(false);}
   template<> bool opElem<Octave_StateInterpreter, OP_T<OPs::OP_MAX>>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const Octave_StateInterpreter::NAME src0, const Octave_StateInterpreter::NAME src1) {assert(false);}
   template<> bool opConst<Octave_StateInterpreter, OP_T<OPs::OP_SCALE>, double>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const Octave_StateInterpreter::NAME src, const double c) {return s.opScale(dst, src, c);}
   template<> bool opConst<Octave_StateInterpreter, OP_T<OPs::OP_TRANSLATE>, double>(Octave_StateInterpreter &s, const Octave_StateInterpreter::NAME dst, const Octave_StateInterpreter::NAME src, const double c) {return s.opTranslate(dst, src, c);}
   template<> bool op<Octave_StateInterpreter, OP_T<OPs::OP_ADD>>(Octave_StateInterpreter &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {return s.opAdd(dstVar, srcVar0, srcVar1);}     
   template<> bool op<Octave_StateInterpreter, OP_T<OPs::OP_SUB>>(Octave_StateInterpreter &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {return s.opSub(dstVar, srcVar0, srcVar1);}  
   template<> bool op<Octave_StateInterpreter, OP_T<OPs::OP_MUL>>(Octave_StateInterpreter &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {return s.opMul(dstVar, srcVar0, srcVar1);}
   template<> bool op<Octave_StateInterpreter, OP_T<OPs::OP_DIV>>(Octave_StateInterpreter &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar0, const Octave_State::NAME srcVar1) {return s.opDiv(dstVar, srcVar0, srcVar1);} 
   template<> bool op<Octave_StateInterpreter, OP_T<OPs::OP_NEG>>(Octave_StateInterpreter &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {return s.opNeg(dstVar, srcVar);} 
   template<> bool op<Octave_StateInterpreter, OP_T<OPs::OP_TRANS>>(Octave_StateInterpreter &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {return s.opTrans(dstVar, srcVar);}
   template<> bool op<Octave_StateInterpreter, OP_T<OPs::OP_CONJ>>(Octave_StateInterpreter &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {return s.opConj(dstVar, srcVar);}
   template<> bool op<Octave_StateInterpreter, OP_T<OPs::OP_INVERSE>>(Octave_StateInterpreter &s, const Octave_State::NAME dstVar, const Octave_State::NAME srcVar) {return s.opInverse(dstVar, srcVar);}

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
 


   ////////////////////////////////////////////// Stateless //////////////////////////////////////////////
   //Vector
     /*template<> VV<OCT_VEC_T> opConst<OCT_VEC_T, OP_T<OPs::OP_SCALE>, double>(const VV<OCT_VEC_T> &src0, const double src1) {return VV<OCT_VEC_T>(src0.getV() * src1);}
     template<> VV<OCT_VEC_T> opConst<OCT_VEC_T, OP_T<OPs::OP_TRANSLATE>, double>(const VV<OCT_VEC_T> &src0, const double src1) {return VV<OCT_VEC_T>(src0.getV() + src1*OCT_VEC_T::Ones(src0.getV().rows(), src0.getV().cols()));}
     template<> VV<OCT_VEC_T> opConst<OCT_VEC_T, OP_T<OPs::OP_SUB>, double>(const double src0, const VV<OCT_VEC_T> &src1) {return VV<OCT_VEC_T>(src0*OCT_VEC_T::Ones(src1.getV().rows(), src1.getV().cols()) - src1.getV());}
     template<> size_t size(const VV<OCT_VEC_T> &x) {return x.getV().size();}
     template<> VV<OCT_VEC_T> create<OCT_VEC_T>(size_t i, INITIALIZATIONs init) {
       switch(init) {
        case INITIALIZATIONs::INIT_RANDOM:  return VV<OCT_VEC_T>(OCT_VEC_T::Random(i));
        case INITIALIZATIONs::INIT_ZERO:    return VV<OCT_VEC_T>(OCT_VEC_T::Zero(i));
        case INITIALIZATIONs::INIT_ONE:     return VV<OCT_VEC_T>(OCT_VEC_T::Ones(i));
        case INITIALIZATIONs::INIT_NONE:    return VV<OCT_VEC_T>(OCT_VEC_T(i));
       }
     }
     template<> VV<OCT_VEC_T> set<OCT_VEC_T, double>(const VV<OCT_VEC_T> &dst, const double src1) {
       OCT_VEC_T v = OCT_VEC_T(dst.getV());
       return VV<OCT_VEC_T>(v.setConstant(v.rows(), v.cols(), src1));
     }
     template<> VV<OCT_VEC_T> set<OCT_VEC_T, double>(const VV<OCT_VEC_T> &dst, size_t i, const double src1) {
       OCT_VEC_T v = OCT_VEC_T(dst.getV());
       v(i) = src1;
       return VV<OCT_VEC_T>(v);
     }
     template<> VV<OCT_VEC_T> set<OCT_VEC_T, double>(const VV<OCT_VEC_T> &dst, size_t base, size_t len, const double src1) {
       OCT_VEC_T v = OCT_VEC_T(dst.getV());
       return VV<OCT_VEC_T>(v.setConstant(base, len, src1));
     }
     template<> double toDouble<OCT_VEC_T>(const VV<OCT_VEC_T> &dst) {
       if((dst.getV().rows() != 1) && (dst.getV().cols() != 1))
          throw MatWlibException("Matrix is not (1,1)");
       return dst.getV()(1,1);
     }
     template<> double elem(const VV<OCT_VEC_T> &src, size_t i) {return src.getV()(i);} 
     //template<> VV<OCT_VEC_T> elem<OCT_VEC_T>(const VV<OCT_VEC_T> &src, size_t r, size_t c) {return VV<OCT_VEC_T>(src.getV()(r,c));} 
     template<> VV<OCT_VEC_T> op<OCT_VEC_T, OP_T<OPs::OP_ADD>>(const VV<OCT_VEC_T> &x, const VV<OCT_VEC_T> &y) {return VV<OCT_VEC_T>(x.getV() + y.getV());}     
     template<> VV<OCT_VEC_T> op<OCT_VEC_T, OP_T<OPs::OP_SUB>>(const VV<OCT_VEC_T> &x, const VV<OCT_VEC_T> &y) {return VV<OCT_VEC_T>(x.getV() - y.getV());}  
     template<> VV<OCT_VEC_T> op<OCT_VEC_T, OP_T<OPs::OP_MUL>>(const VV<OCT_VEC_T> &x, const VV<OCT_VEC_T> &y) {return VV<OCT_VEC_T>(x.getV() * y.getV());}
     template<> VV<OCT_VEC_T> op<OCT_VEC_T, OP_T<OPs::OP_DIV>>(const VV<OCT_VEC_T> &x, const VV<OCT_VEC_T> &y) {assert(false);}//return x/y;}  
     template<> VV<OCT_VEC_T> op<OCT_VEC_T, OP_T<OPs::OP_NEG>>(const VV<OCT_VEC_T> &x) {return VV<OCT_VEC_T>(-x.getV());}  
     template<> VV<OCT_VEC_T> op<OCT_VEC_T, OP_T<OPs::OP_TRANS>>(const VV<OCT_VEC_T> &x) {return VV<OCT_VEC_T>(x.getV().transpose());}  
     template<> VV<OCT_VEC_T> op<OCT_VEC_T, OP_T<OPs::OP_CONJ>>(const VV<OCT_VEC_T> &x) {return VV<OCT_VEC_T>(x.getV().conjugate());}  
     template<> VV<OCT_VEC_T> op<OCT_VEC_T, OP_T<OPs::OP_INVERSE>>(const VV<OCT_VEC_T> &x) {return VV<OCT_VEC_T>(x.getV().inverse());}  
   //Matrix
     template<> MV<OCT_MAT_T> opConst<OCT_MAT_T, OP_T<OPs::OP_SCALE>, double>(const MV<OCT_MAT_T> &src0, const double src1) {return MV<OCT_MAT_T>(src0.getV() * src1);}
     template<> MV<OCT_MAT_T> opConst<OCT_MAT_T, OP_T<OPs::OP_TRANSLATE>, double>(const MV<OCT_MAT_T> &src0, const double src1) {return MV<OCT_MAT_T>(src0.getV() + src1*OCT_MAT_T::Ones(src0.getV().rows(), src0.getV().cols()));}
     template<> MV<OCT_MAT_T> opConst<OCT_MAT_T, OP_T<OPs::OP_SUB>, double>(const double src0, const MV<OCT_MAT_T> &src1) {return MV<OCT_MAT_T>(src0*OCT_MAT_T::Ones(src1.getV().rows(), src1.getV().cols()) - src1.getV());}
     template<> size_t size(const MV<OCT_MAT_T> &x, size_t dim) {
        if(dim == 0)
           return x.getV().rows();
        else if(dim == 1)
                return x.getV().cols();
        else
            assert(false);
     }
     template<> MV<OCT_MAT_T> create<OCT_MAT_T>(size_t r, size_t c, INITIALIZATIONs init) {
       switch(init) {
        case INITIALIZATIONs::INIT_RANDOM:  return MV<OCT_MAT_T>(OCT_MAT_T::Random(r, c));
        case INITIALIZATIONs::INIT_ZERO:    return MV<OCT_MAT_T>(OCT_MAT_T::Zero(r, c));
        case INITIALIZATIONs::INIT_ONE:     return MV<OCT_MAT_T>(OCT_MAT_T::Ones(r, c));
        case INITIALIZATIONs::INIT_NONE:    return MV<OCT_MAT_T>(OCT_MAT_T(r, c));
       }
     }
     template<> MV<OCT_MAT_T> set<OCT_MAT_T, double>(const MV<OCT_MAT_T> &dst, const double src1) {
       OCT_MAT_T v = OCT_MAT_T(dst.getV());
       return MV<OCT_MAT_T>(v.setConstant(v.rows(), v.cols(), src1));
     }
     template<> MV<OCT_MAT_T> set<OCT_MAT_T, double>(const MV<OCT_MAT_T> &dst, size_t r, size_t c, const double src1) {
       OCT_MAT_T v = OCT_MAT_T(dst.getV());
       v(r, c) = src1;
       return MV<OCT_MAT_T>(v);
     }
     template<> MV<OCT_MAT_T> set<OCT_MAT_T, double>(const MV<OCT_MAT_T> &dst, size_t rb, size_t r_len, size_t cb, size_t c_len, const double src1) {
       OCT_MAT_T v = OCT_MAT_T(dst.getV());
       v.block(rb, r_len, cb, c_len).setConstant(src1);
       return MV<OCT_MAT_T>(v);
     }
     template<> MV<OCT_MAT_T> set<OCT_MAT_T, int>(const MV<OCT_MAT_T> &dst, const int src1) {
       OCT_MAT_T v = OCT_MAT_T(dst.getV());
       return MV<OCT_MAT_T>(v.setConstant(v.rows(), v.cols(), src1));
     }
     template<> MV<OCT_MAT_T> set<OCT_MAT_T, int>(const MV<OCT_MAT_T> &dst, size_t r, size_t c, const int src1) {
       OCT_MAT_T v = OCT_MAT_T(dst.getV());
       v(r, c) = src1;
       return MV<OCT_MAT_T>(v);
     }
     template<> MV<OCT_MAT_T> set<OCT_MAT_T, int>(const MV<OCT_MAT_T> &dst, size_t rb, size_t r_len, size_t cb, size_t c_len, const int src1) {
       OCT_MAT_T v = OCT_MAT_T(dst.getV());
       v.block(rb, r_len, cb, c_len).setConstant(src1);
       return MV<OCT_MAT_T>(v);
     }


     template<> double toDouble<OCT_MAT_T>(const MV<OCT_MAT_T> &dst) {
       if((dst.getV().rows() != 1) && (dst.getV().cols() != 1))
          throw MatWlibException("Matrix is not (1,1)");
       return dst.getV()(1,1);
     }
     template<> MV<OCT_MAT_T> row<OCT_MAT_T>(const MV<OCT_MAT_T> &src, size_t r) {return MV<OCT_MAT_T>(src.getV().block(r,1,1,src.getV().cols()));} 
     template<> MV<OCT_MAT_T> col<OCT_MAT_T>(const MV<OCT_MAT_T> &src, size_t c) {return MV<OCT_MAT_T>(src.getV().block(1,c,src.getV().rows(),1));}
     template<> double elem<OCT_MAT_T>(const MV<OCT_MAT_T> &src, size_t r, size_t c) {return src.getV()(r,c);} 
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_ADD>>(const MV<OCT_MAT_T> &x, const MV<OCT_MAT_T> &y) {return MV<OCT_MAT_T>(x.getV() + y.getV());}     
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_SUB>>(const MV<OCT_MAT_T> &x, const MV<OCT_MAT_T> &y) {return MV<OCT_MAT_T>(x.getV() - y.getV());}  
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_MUL>>(const MV<OCT_MAT_T> &x, const MV<OCT_MAT_T> &y) {return MV<OCT_MAT_T>(x.getV() * y.getV());}
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_DIV>>(const MV<OCT_MAT_T> &x, const MV<OCT_MAT_T> &y) {assert(false);}//return x/y;}  
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_NEG>>(const MV<OCT_MAT_T> &x) {return MV<OCT_MAT_T>(-x.getV());}  
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_TRANS>>(const MV<OCT_MAT_T> &x) {return MV<OCT_MAT_T>(x.getV().transpose());}  
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_CONJ>>(const MV<OCT_MAT_T> &x) {return MV<OCT_MAT_T>(x.getV().conjugate());}  
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_INVERSE>>(const MV<OCT_MAT_T> &x) {return MV<OCT_MAT_T>(x.getV().inverse());}  

     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_ADD>>(const VV<OCT_MAT_T> &x, const MV<OCT_MAT_T> &y) {return MV<OCT_MAT_T>(x.getV() + y.getV());}     
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_SUB>>(const VV<OCT_MAT_T> &x, const MV<OCT_MAT_T> &y) {return MV<OCT_MAT_T>(x.getV() - y.getV());}  
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_MUL>>(const VV<OCT_MAT_T> &x, const MV<OCT_MAT_T> &y) {return MV<OCT_MAT_T>(x.getV() * y.getV());}
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_DIV>>(const VV<OCT_MAT_T> &x, const MV<OCT_MAT_T> &y) {assert(false);}//return x/y;}  

     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_ADD>>(const MV<OCT_MAT_T> &x, const VV<OCT_MAT_T> &y) {return MV<OCT_MAT_T>(x.getV() + y.getV());}     
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_SUB>>(const MV<OCT_MAT_T> &x, const VV<OCT_MAT_T> &y) {return MV<OCT_MAT_T>(x.getV() - y.getV());}  
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_MUL>>(const MV<OCT_MAT_T> &x, const VV<OCT_MAT_T> &y) {return MV<OCT_MAT_T>(x.getV() * y.getV());}
     template<> MV<OCT_MAT_T> op<OCT_MAT_T, OP_T<OPs::OP_DIV>>(const MV<OCT_MAT_T> &x, const VV<OCT_MAT_T> &y) {assert(false);}//return x/y;}*/  

}

#endif

