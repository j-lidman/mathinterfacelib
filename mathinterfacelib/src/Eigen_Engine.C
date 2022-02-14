#ifdef HAS_EIGEN

#include <sstream>
#include <vector>
//#include <string.h>
#include <thread>
#include <stack>

#include <dlfcn.h>
#include <mcheck.h>

#include <boost/tuple/tuple.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/bind.hpp>

#include "Eigen_Engine.h"

using namespace std;
using namespace boost;
using namespace boost::interprocess;
namespace qi = boost::spirit::qi;

namespace MatWLib {
   /*Eigen_State *init_eigen_engine() {
      return new Eigen_State();
   }
   void init_eigen_engine(Eigen_State *s) {
      delete s;
   }
   template<> Eigen_State *cast_state<Eigen_State>(State *s) {
      Eigen_State *s_ = dynamic_cast<Eigen_State *>(s);
      assert((s_ != NULL) && "Invalid Eigen State");
      return s_;
   }

   template<> std::string newName<Eigen_State>() {
     std::stringstream ss;
     ss << "var" << getUniqueNumber();
     return ss.str();
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   EIGEN_MAT_T &Eigen_State::getMat(std::string varName) {
     std::map<std::string, EIGEN_MAT_T>::iterator it;
     if((it = mapMatrices.find(varName)) == mapMatrices.end())
         throw MatWlibException("No matrix variable with this name has been defined");
     return it->second;
   }
   const EIGEN_MAT_T &Eigen_State::getMat(std::string varName) const {
     std::map<std::string, EIGEN_MAT_T>::const_iterator it;
     if((it = mapMatrices.find(varName)) == mapMatrices.end())
         throw MatWlibException("No matrix variable with this name has been defined");
     return it->second;
   }
   EIGEN_VEC_T &Eigen_State::getVec(std::string varName) {
     std::map<std::string, EIGEN_VEC_T>::iterator it;
     if((it = mapVectors.find(varName)) == mapVectors.end())
         throw MatWlibException("No vector variable with this name has been defined");
     return it->second;
   }
   const EIGEN_VEC_T &Eigen_State::getVec(std::string varName) const {
     std::map<std::string, EIGEN_VEC_T>::const_iterator it;
     if((it = mapVectors.find(varName)) == mapVectors.end())
         throw MatWlibException("No vector variable with this name has been defined");
     return it->second;
   }

   Eigen_State::Eigen_State() {}
   Eigen_State::~Eigen_State() {}

   std::string Eigen_State::toString() const {return "Eigen State";}
   bool Eigen_State::good() const {return true;}


   void Eigen_State::addVar(std::string varName, EIGEN_MAT_T mat) {
     std::map<std::string, EIGEN_MAT_T>::iterator it;
     if((it = mapMatrices.find(varName)) == mapMatrices.end())
         mapMatrices[varName] = mat;
     else
         it->second = mat;
   }
   void Eigen_State::addVar(std::string varName, EIGEN_VEC_T vec) {
     std::map<std::string, EIGEN_VEC_T>::iterator it;
     if((it = mapVectors.find(varName)) == mapVectors.end())
         mapVectors[varName] = vec;
     else
         it->second = vec;
   }
   void Eigen_State::remVarMat(std::string varName) {
     std::map<std::string, EIGEN_MAT_T>::iterator it;
     if((it = mapMatrices.find(varName)) != mapMatrices.end())
         mapMatrices.erase(it);
   }
   void Eigen_State::remVarVec(std::string varName) {
     std::map<std::string, EIGEN_VEC_T>::iterator it;
     if((it = mapVectors.find(varName)) != mapVectors.end())
         mapVectors.erase(it);
   }
   const EIGEN_MAT_T &Eigen_State::getVarMat(std::string varName) const {return getMat(varName);}
   EIGEN_MAT_T &Eigen_State::getVarMat(std::string varName) {return getMat(varName);}
   EIGEN_VEC_T &Eigen_State::getVarVec(std::string varName) {return getVec(varName);}
   const EIGEN_VEC_T &Eigen_State::getVarVec(std::string varName) const {return getVec(varName);}


   bool Eigen_State::opAdd(const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar0, const Eigen_State::NAME srcVar1) {
     try {
         getMat(dstVar) = getMat(srcVar0) + getMat(srcVar1);
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Eigen_State::opSub(const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar0, const Eigen_State::NAME srcVar1) {
     try {
         getMat(dstVar) = getMat(srcVar0) - getMat(srcVar1);
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Eigen_State::opMul(const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar0, const Eigen_State::NAME srcVar1) {
     try {
         getMat(dstVar) = getMat(srcVar0) * getMat(srcVar1);
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Eigen_State::opDiv(const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar0, const Eigen_State::NAME srcVar1) {
     assert(false);
   }
   bool Eigen_State::opNeg(const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar) {
     try {
         getMat(dstVar) = -getMat(srcVar);
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Eigen_State::opConj(const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar) {
     try {
         getMat(dstVar) = getMat(srcVar).conjugate();
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Eigen_State::opTrans(const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar) {
     try {
         getMat(dstVar) = getMat(srcVar).transpose();
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Eigen_State::opInverse(const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar) {
     try {
         getMat(dstVar) = getMat(srcVar).inverse();
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }

   bool Eigen_State::opScale(const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar, const double c) {
     try {
         getMat(dstVar) = getMat(srcVar) * c;
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }
   bool Eigen_State::opTranslate(const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar, const double c) {
     try {
         getMat(dstVar) = getMat(srcVar) + c*EIGEN_MAT_T::Ones(getMat(srcVar).rows(), getMat(srcVar).cols());
     } catch (MatWlibException e) {
         return false;
     }
     return true;
   }


   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   DataInterface<Eigen_State>::DataInterface(Eigen_State &s_) : s(s_) {}
   DataInterface<Eigen_State>::~DataInterface() {}

   template<> void DataInterface<Eigen_State>::writeVar<std::vector<double>::iterator>(const Eigen_State::NAME varName, size_t m, size_t n,
         std::vector<double>::iterator begin, std::vector<double>::iterator end) {

   }
   template<> void DataInterface<Eigen_State>::writeVar<std::vector<double>::const_iterator>(const Eigen_State::NAME varName, size_t m, size_t n, 
         std::vector<double>::const_iterator begin, std::vector<double>::const_iterator end) {

   }
   template<> void DataInterface<Eigen_State>::writeVar<std::vector<size_t>::iterator>(const Eigen_State::NAME varName, size_t m, size_t n,
         std::vector<size_t>::iterator begin, std::vector<size_t>::iterator end) {

   }
   template<> void DataInterface<Eigen_State>::writeVar<std::vector<size_t>::const_iterator>(const Eigen_State::NAME varName, size_t m, size_t n, 
         std::vector<size_t>::const_iterator begin, std::vector<size_t>::const_iterator end) {

   }



   template<> void DataInterface<Eigen_State>::writeVar<size_t>(std::string varName, size_t val) {
     EIGEN_MAT_T newMat(1,1);
     newMat(0,0) = val;
     s.addVar(varName, newMat);
   }
   template<> void DataInterface<Eigen_State>::writeVar<double>(std::string varName, double val) {
     EIGEN_MAT_T newMat(1,1);
     newMat(0,0) = val;
     s.addVar(varName, newMat);
   }
   template<> void DataInterface<Eigen_State>::writeVar<std::string>(std::string varName, std::string val) {

   }
   template<> void DataInterface<Eigen_State>::writeVar<size_t>(std::string varName, size_t m, size_t n, const size_t *mat) {
     EIGEN_MAT_T newMat(m,n);
     for(size_t i = 0; i < m; i++)
         for(size_t j = 0; j < n; j++)
             newMat(i,j) = mat[i*n + j];
     s.addVar(varName, newMat);
   }
   template<> void DataInterface<Eigen_State>::writeVar<double>(std::string varName, size_t m, size_t n, const double *mat) {
     EIGEN_MAT_T newMat(m,n);
     for(size_t i = 0; i < m; i++)
         for(size_t j = 0; j < n; j++)
             newMat(i,j) = mat[i*n + j];
     s.addVar(varName, newMat);
   }

   template<> size_t DataInterface<Eigen_State>::readVar<size_t>(std::string varName) {
     
   }
   template<> size_t *DataInterface<Eigen_State>::readVar<size_t>(std::string varName, std::vector<size_t> &szDims) {

   }
   template<> double DataInterface<Eigen_State>::readVar<double>(std::string varName) {

   }
   template<> double *DataInterface<Eigen_State>::readVar<double>(std::string varName, std::vector<size_t> &szDims) {

   }
   template<> std::string DataInterface<Eigen_State>::readVar<std::string>(std::string varName) {
     
   }


   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   template<> bool opConst<Eigen_State, OP_T<OPs::OP_SCALE>, double>(Eigen_State &s, const Eigen_State::NAME dst, const Eigen_State::NAME src, const double c) {return s.opScale(dst, src, c);}
   template<> bool opConst<Eigen_State, OP_T<OPs::OP_TRANSLATE>, double>(Eigen_State &s, const Eigen_State::NAME dst, const Eigen_State::NAME src, const double c) {return s.opTranslate(dst, src, c);}
   

   template<> void appendRows<Eigen_State>(Eigen_State &s, const Eigen_State::NAME x, const Eigen_State::NAME app) {

   }
   template<> void appendColumns<Eigen_State>(Eigen_State &s, const Eigen_State::NAME x, const Eigen_State::NAME app) {

   }
   template<> bool create<Eigen_State>(Eigen_State &s, const Eigen_State::NAME dst, size_t r, size_t c, INITIALIZATIONs init) {
     switch(init) {
      case INITIALIZATIONs::INIT_RANDOM:  s.addVar(dst, (EIGEN_MAT_T) EIGEN_MAT_T::Random(r, c));   break;
      case INITIALIZATIONs::INIT_ZERO:    s.addVar(dst, (EIGEN_MAT_T) EIGEN_MAT_T::Zero(r, c));     break;
      case INITIALIZATIONs::INIT_ONE:     s.addVar(dst, (EIGEN_MAT_T) EIGEN_MAT_T::Ones(r, c));     break;
      case INITIALIZATIONs::INIT_NONE:    s.addVar(dst, (EIGEN_MAT_T) EIGEN_MAT_T(r, c));           break;
     }
   }
   template<> bool set<Eigen_State, double>(Eigen_State &s, const Eigen_State::NAME dst, const double src1) {
     EIGEN_MAT_T &v = s.getVarMat(dst);
     v.setConstant(v.rows(), v.cols(), src1);
     return true;
   }
   template<> double toDouble<Eigen_State>(Eigen_State &s, const Eigen_State::NAME src) {
     EIGEN_MAT_T &v = s.getVarMat(src);
     if((v.rows() != 1) && (v.cols() != 1))
        throw MatWlibException("Matrix is not (1,1)");
     return v(1,1);
   }
   template<> bool row<Eigen_State>(Eigen_State &s, const Eigen_State::NAME dst, const Eigen_State::NAME srcVar, size_t r) {
     EIGEN_MAT_T &dstV = s.getVarMat(dst), &srcV = s.getVarMat(srcVar);
     dstV = srcV.block(r,1,1,srcV.cols());
     return true;
   }
   template<> bool col<Eigen_State>(Eigen_State &s, const Eigen_State::NAME dst, const Eigen_State::NAME srcVar, size_t c) {
     EIGEN_MAT_T &dstV = s.getVarMat(dst), &srcV = s.getVarMat(srcVar);
     dstV = srcV.block(1,c,srcV.rows(),1);
     return true;
   }
   template<> bool elem<Eigen_State>(Eigen_State &s, const Eigen_State::NAME dst, const Eigen_State::NAME srcVar, size_t r, size_t c) {
     EIGEN_MAT_T &dstV = s.getVarMat(dst), &srcV = s.getVarMat(srcVar);
     dstV = src(r,c);
     return true;
   }
   template<> bool op<Eigen_State, OP_T<OPs::OP_ADD>>(Eigen_State &s, const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar0, const Eigen_State::NAME srcVar1) {return s.opAdd(dstVar, srcVar0, srcVar1);}     
   template<> bool op<Eigen_State, OP_T<OPs::OP_SUB>>(Eigen_State &s, const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar0, const Eigen_State::NAME srcVar1) {return s.opSub(dstVar, srcVar0, srcVar1);}  
   template<> bool op<Eigen_State, OP_T<OPs::OP_MUL>>(Eigen_State &s, const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar0, const Eigen_State::NAME srcVar1) {return s.opMul(dstVar, srcVar0, srcVar1);}
   template<> bool op<Eigen_State, OP_T<OPs::OP_DIV>>(Eigen_State &s, const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar0, const Eigen_State::NAME srcVar1) {return s.opDiv(dstVar, srcVar0, srcVar1);} 
   template<> bool op<Eigen_State, OP_T<OPs::OP_NEG>>(Eigen_State &s, const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar) {return s.opNeg(dstVar, srcVar);} 
   template<> bool op<Eigen_State, OP_T<OPs::OP_TRANS>>(Eigen_State &s, const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar) {return s.opTrans(dstVar, srcVar);}
   template<> bool op<Eigen_State, OP_T<OPs::OP_CONJ>>(Eigen_State &s, const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar) {return s.opConj(dstVar, srcVar);}
   template<> bool op<Eigen_State, OP_T<OPs::OP_INVERSE>>(Eigen_State &s, const Eigen_State::NAME dstVar, const Eigen_State::NAME srcVar) {return s.opInverse(dstVar, srcVar);}*/








   //Vector
     ////////////////////// Row-vector //////////////////////
     template<> VV<EIGEN_ROW_VEC_T> opConst<EIGEN_ROW_VEC_T, OP_T<OPs::OP_SCALE>, double>(const VV<EIGEN_ROW_VEC_T> &src0, const double src1) {return VV<EIGEN_ROW_VEC_T>(src0.getV() * src1);}
     template<> VV<EIGEN_ROW_VEC_T> opConst<EIGEN_ROW_VEC_T, OP_T<OPs::OP_TRANSLATE>, double>(const VV<EIGEN_ROW_VEC_T> &src0, const double src1) {return VV<EIGEN_ROW_VEC_T>(src0.getV() + src1*EIGEN_ROW_VEC_T::Ones(src0.getV().rows(), src0.getV().cols()));}
     template<> VV<EIGEN_ROW_VEC_T> opConst<EIGEN_ROW_VEC_T, OP_T<OPs::OP_SUB>, double>(const double src0, const VV<EIGEN_ROW_VEC_T> &src1) {return VV<EIGEN_ROW_VEC_T>(src0*EIGEN_ROW_VEC_T::Ones(src1.getV().rows(), src1.getV().cols()) - src1.getV());}

     template<> MV<EIGEN_MAT_T> toMatrix(const VV<EIGEN_ROW_VEC_T> &x) {
       EIGEN_MAT_T X = EIGEN_MAT_T(x.getV());
       return MV<EIGEN_MAT_T>(X);
     }
     template<> MV<EIGEN_MAT_T> toDiagonalMatrix(const VV<EIGEN_ROW_VEC_T> &x) {
       EIGEN_MAT_T X = EIGEN_MAT_T(x.getV().asDiagonal());
       return MV<EIGEN_MAT_T>(X);
     }
     template<> VV<EIGEN_ROW_VEC_T> clone<EIGEN_ROW_VEC_T>(const VV<EIGEN_ROW_VEC_T> &srcName) {
       EIGEN_ROW_VEC_T v = EIGEN_ROW_VEC_T(srcName.getV());
       return VV<EIGEN_ROW_VEC_T>(v);
     }
     template<> void append<EIGEN_ROW_VEC_T>(VV<EIGEN_ROW_VEC_T> &x, const VV<EIGEN_ROW_VEC_T> &app) {
       //EIGEN_ROW_VEC_T v = EIGEN_ROW_VEC_T(x.getV().size() + app.getV().size());
       x.getV() << app.getV();
       //return VV<EIGEN_ROW_VEC_T>(v);
     }
     template<> size_t size<EIGEN_ROW_VEC_T>(const VV<EIGEN_ROW_VEC_T> &x, size_t dim) {
        if(dim == 0)
           return x.getV().rows();
        else if(dim == 1)
                return x.getV().cols();
        else
            assert(false && "Invalid dimension");
     }
     template<> VV<EIGEN_ROW_VEC_T> create<EIGEN_ROW_VEC_T, double>(size_t i, bool isRowVector, const double constantVal) {
       assert(isRowVector);
       EIGEN_ROW_VEC_T m = constantVal*EIGEN_ROW_VEC_T::Ones(i);
       return VV<EIGEN_ROW_VEC_T>(m);
     }
     template<> VV<EIGEN_ROW_VEC_T> create<EIGEN_ROW_VEC_T>(size_t i, bool isRowVector, INITIALIZATIONs init) {
       assert(isRowVector);
       switch(init) {
        case INITIALIZATIONs::INIT_RANDOM:  return VV<EIGEN_ROW_VEC_T>(EIGEN_ROW_VEC_T::Random(i));
        case INITIALIZATIONs::INIT_ZERO:    return VV<EIGEN_ROW_VEC_T>(EIGEN_ROW_VEC_T::Zero(i));
        case INITIALIZATIONs::INIT_ONE:     return VV<EIGEN_ROW_VEC_T>(EIGEN_ROW_VEC_T::Ones(i));
        case INITIALIZATIONs::INIT_NONE:    return VV<EIGEN_ROW_VEC_T>(EIGEN_ROW_VEC_T(i));
        default:
            assert(false && "Unhandled init type");
       }
     }
     template<> void set<EIGEN_ROW_VEC_T>(VV<EIGEN_ROW_VEC_T> &x, const VV<EIGEN_ROW_VEC_T> &src) {
       EIGEN_ROW_VEC_T v(src.getV());
       x.setV(v);
     }
     template<> void set<EIGEN_ROW_VEC_T, double>(VV<EIGEN_ROW_VEC_T> &dst, const double src1) {
       EIGEN_ROW_VEC_T &v = dst.getV();
       v.setConstant(v.rows(), v.cols(), src1);
     }
     template<> void set<EIGEN_ROW_VEC_T, double>(VV<EIGEN_ROW_VEC_T> &dst, const double *y, size_t n) {
       assert(false);
     }
     template<> void set<EIGEN_ROW_VEC_T, double>(VV<EIGEN_ROW_VEC_T> &dst, size_t i, const double src1) {
       EIGEN_ROW_VEC_T &v = dst.getV();
       v(i) = src1;
     }
     template<> void set<EIGEN_ROW_VEC_T, double>(VV<EIGEN_ROW_VEC_T> &dst, size_t base, size_t len, const double src1) {
       EIGEN_ROW_VEC_T &v = dst.getV();
       v.setConstant(base, len, src1);
     }
     template<> double toDouble<EIGEN_ROW_VEC_T>(const VV<EIGEN_ROW_VEC_T> &dst) {
       if((dst.getV().rows() != 1) && (dst.getV().cols() != 1))
          throw MatWlibException("Matrix is not (1,1)");
       return dst.getV()(1,1);
     }
     template<> double elem(const VV<EIGEN_ROW_VEC_T> &src, size_t i) {return src.getV()(i);} 
     //template<> VV<EIGEN_ROW_VEC_T> elem<EIGEN_ROW_VEC_T>(const VV<EIGEN_ROW_VEC_T> &src, size_t r, size_t c) {return VV<EIGEN_ROW_VEC_T>(src.getV()(r,c));} 
     template<> VV<EIGEN_ROW_VEC_T> op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_ADD>>(const VV<EIGEN_ROW_VEC_T> &x, const VV<EIGEN_ROW_VEC_T> &y) {return VV<EIGEN_ROW_VEC_T>(x.getV() + y.getV());}     
     template<> VV<EIGEN_ROW_VEC_T> op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_SUB>>(const VV<EIGEN_ROW_VEC_T> &x, const VV<EIGEN_ROW_VEC_T> &y) {return VV<EIGEN_ROW_VEC_T>(x.getV() - y.getV());}  
     template<> VV<EIGEN_ROW_VEC_T> op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_MUL>>(const VV<EIGEN_ROW_VEC_T> &x, const VV<EIGEN_ROW_VEC_T> &y) {return VV<EIGEN_ROW_VEC_T>(x.getV() * y.getV());}
     template<> VV<EIGEN_ROW_VEC_T> op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_DIV>>(const VV<EIGEN_ROW_VEC_T> &x, const VV<EIGEN_ROW_VEC_T> &y) {assert(false);}//return x/y;}  
     template<> VV<EIGEN_ROW_VEC_T> op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_NEG>>(const VV<EIGEN_ROW_VEC_T> &x) {return VV<EIGEN_ROW_VEC_T>(-x.getV());}  
     template<> VV<EIGEN_ROW_VEC_T> op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_TRANS>>(const VV<EIGEN_ROW_VEC_T> &x) {return VV<EIGEN_ROW_VEC_T>(x.getV().transpose());}  
     template<> VV<EIGEN_ROW_VEC_T> op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_CONJ>>(const VV<EIGEN_ROW_VEC_T> &x) {return VV<EIGEN_ROW_VEC_T>(x.getV().conjugate());}  
     template<> VV<EIGEN_ROW_VEC_T> op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_INVERSE>>(const VV<EIGEN_ROW_VEC_T> &x) {return VV<EIGEN_ROW_VEC_T>(x.getV().inverse());}  

     template<> VV<EIGEN_ROW_VEC_T> opElem<EIGEN_ROW_VEC_T, OP_T<OPs::OP_MUL>>(const VV<EIGEN_ROW_VEC_T> &x, const VV<EIGEN_ROW_VEC_T> &y) {EIGEN_ROW_VEC_T v = x.getV(); return VV<EIGEN_ROW_VEC_T>(v.cwiseProduct(y.getV()));}
     template<> VV<EIGEN_ROW_VEC_T> opElem<EIGEN_ROW_VEC_T, OP_T<OPs::OP_DIV>>(const VV<EIGEN_ROW_VEC_T> &x, const VV<EIGEN_ROW_VEC_T> &y) {EIGEN_ROW_VEC_T v = x.getV(); return VV<EIGEN_ROW_VEC_T>(v.cwiseQuotient(y.getV()));}
     template<> VV<EIGEN_ROW_VEC_T> opElem<EIGEN_ROW_VEC_T, OP_T<OPs::OP_MIN>>(const VV<EIGEN_ROW_VEC_T> &x, const VV<EIGEN_ROW_VEC_T> &y) {EIGEN_ROW_VEC_T v = x.getV(); return VV<EIGEN_ROW_VEC_T>(v.cwiseMin(y.getV()));}
     template<> VV<EIGEN_ROW_VEC_T> opElem<EIGEN_ROW_VEC_T, OP_T<OPs::OP_MAX>>(const VV<EIGEN_ROW_VEC_T> &x, const VV<EIGEN_ROW_VEC_T> &y) {EIGEN_ROW_VEC_T v = x.getV(); return VV<EIGEN_ROW_VEC_T>(v.cwiseMax(y.getV()));}
     template<> VV<EIGEN_ROW_VEC_T> opElem<EIGEN_ROW_VEC_T, OP_T<OPs::OP_SQRT>>(const VV<EIGEN_ROW_VEC_T> &x) {EIGEN_ROW_VEC_T v = x.getV(); return VV<EIGEN_ROW_VEC_T>(v.cwiseSqrt());}
     template<> VV<EIGEN_ROW_VEC_T> opElem<EIGEN_ROW_VEC_T, OP_T<OPs::OP_ABS>>(const VV<EIGEN_ROW_VEC_T> &x) {EIGEN_ROW_VEC_T v = x.getV(); return VV<EIGEN_ROW_VEC_T>(v.cwiseAbs());}

     template<> double opReduce<EIGEN_ROW_VEC_T, OP_T<OPs::OP_ADD>>(const VV<EIGEN_ROW_VEC_T> &x) {return x.getV().sum();}
     template<> double opReduce<EIGEN_ROW_VEC_T, OP_T<OPs::OP_MUL>>(const VV<EIGEN_ROW_VEC_T> &x) {return x.getV().prod();}
     template<> double opReduce<EIGEN_ROW_VEC_T, OP_T<OPs::OP_MAX>>(const VV<EIGEN_ROW_VEC_T> &x) {return x.getV().maxCoeff();}
     template<> double opReduce<EIGEN_ROW_VEC_T, OP_T<OPs::OP_MIN>>(const VV<EIGEN_ROW_VEC_T> &x) {return x.getV().minCoeff();}

     ////////////////////// Column-vector //////////////////////
     template<> VV<EIGEN_COL_VEC_T> opConst<EIGEN_COL_VEC_T, OP_T<OPs::OP_SCALE>, double>(const VV<EIGEN_COL_VEC_T> &src0, const double src1) {return VV<EIGEN_COL_VEC_T>(src0.getV() * src1);}
     template<> VV<EIGEN_COL_VEC_T> opConst<EIGEN_COL_VEC_T, OP_T<OPs::OP_TRANSLATE>, double>(const VV<EIGEN_COL_VEC_T> &src0, const double src1) {return VV<EIGEN_COL_VEC_T>(src0.getV() + src1*EIGEN_COL_VEC_T::Ones(src0.getV().rows(), src0.getV().cols()));}
     template<> VV<EIGEN_COL_VEC_T> opConst<EIGEN_COL_VEC_T, OP_T<OPs::OP_SUB>, double>(const double src0, const VV<EIGEN_COL_VEC_T> &src1) {return VV<EIGEN_COL_VEC_T>(src0*EIGEN_COL_VEC_T::Ones(src1.getV().rows(), src1.getV().cols()) - src1.getV());}

     template<> MV<EIGEN_MAT_T> toMatrix(const VV<EIGEN_COL_VEC_T> &x) {
       EIGEN_MAT_T X = EIGEN_MAT_T(x.getV());
       return MV<EIGEN_MAT_T>(X);
     }
     template<> MV<EIGEN_MAT_T> toDiagonalMatrix(const VV<EIGEN_COL_VEC_T> &x) {
       EIGEN_MAT_T X = EIGEN_MAT_T(x.getV().asDiagonal());
       return MV<EIGEN_MAT_T>(X);
     }
     template<> VV<EIGEN_COL_VEC_T> clone<EIGEN_COL_VEC_T>(const VV<EIGEN_COL_VEC_T> &srcName) {
       EIGEN_COL_VEC_T v = EIGEN_COL_VEC_T(srcName.getV());
       return VV<EIGEN_COL_VEC_T>(v);
     }
     template<> void append<EIGEN_COL_VEC_T>(VV<EIGEN_COL_VEC_T> &x, const VV<EIGEN_COL_VEC_T> &app) {
       //EIGEN_COL_VEC_T v = EIGEN_COL_VEC_T(x.getV().size() + app.getV().size());
       x.getV() << app.getV();
       //return VV<EIGEN_COL_VEC_T>(v);
     }
     template<> size_t size<EIGEN_COL_VEC_T>(const VV<EIGEN_COL_VEC_T> &x, size_t dim) {
        if(dim == 0)
           return x.getV().rows();
        else if(dim == 1)
                return x.getV().cols();
        else
            assert(false && "Invalid dimension");
     }
     template<> VV<EIGEN_COL_VEC_T> create<EIGEN_COL_VEC_T, double>(size_t i, bool isRowVector, const double constantVal) {
       assert(!isRowVector);
       EIGEN_COL_VEC_T m = constantVal*EIGEN_COL_VEC_T::Ones(i);
       return VV<EIGEN_COL_VEC_T>(m);
     }
     template<> VV<EIGEN_COL_VEC_T> create<EIGEN_COL_VEC_T>(size_t i, bool isRowVector, INITIALIZATIONs init) {
       assert(!isRowVector);
       switch(init) {
        case INITIALIZATIONs::INIT_RANDOM:  return VV<EIGEN_COL_VEC_T>(EIGEN_COL_VEC_T::Random(i));
        case INITIALIZATIONs::INIT_ZERO:    return VV<EIGEN_COL_VEC_T>(EIGEN_COL_VEC_T::Zero(i));
        case INITIALIZATIONs::INIT_ONE:     return VV<EIGEN_COL_VEC_T>(EIGEN_COL_VEC_T::Ones(i));
        case INITIALIZATIONs::INIT_NONE:    return VV<EIGEN_COL_VEC_T>(EIGEN_COL_VEC_T(i));
        default:
            assert(false && "Unhandled init type");
       }
     }
     template<> void set<EIGEN_COL_VEC_T>(VV<EIGEN_COL_VEC_T> &x, const VV<EIGEN_COL_VEC_T> &src) {
       EIGEN_COL_VEC_T v(src.getV());
       x.setV(v);
     }
     template<> void set<EIGEN_COL_VEC_T, double>(VV<EIGEN_COL_VEC_T> &dst, const double src1) {
       EIGEN_COL_VEC_T &v = dst.getV();
       v.setConstant(v.rows(), v.cols(), src1);
     }
     template<> void set<EIGEN_COL_VEC_T, double>(VV<EIGEN_COL_VEC_T> &dst, const double *y, size_t n) {
       assert(false);
     }
     template<> void set<EIGEN_COL_VEC_T, double>(VV<EIGEN_COL_VEC_T> &dst, size_t i, const double src1) {
       EIGEN_COL_VEC_T &v = dst.getV();
       v(i) = src1;
     }
     template<> void set<EIGEN_COL_VEC_T, double>(VV<EIGEN_COL_VEC_T> &dst, size_t base, size_t len, const double src1) {
       EIGEN_COL_VEC_T &v = dst.getV();
       v.setConstant(base, len, src1);
     }
     template<> double toDouble<EIGEN_COL_VEC_T>(const VV<EIGEN_COL_VEC_T> &dst) {
       if((dst.getV().rows() != 1) && (dst.getV().cols() != 1))
          throw MatWlibException("Matrix is not (1,1)");
       return dst.getV()(1,1);
     }
     template<> double elem(const VV<EIGEN_COL_VEC_T> &src, size_t i) {return src.getV()(i);} 
     //template<> VV<EIGEN_COL_VEC_T> elem<EIGEN_COL_VEC_T>(const VV<EIGEN_COL_VEC_T> &src, size_t r, size_t c) {return VV<EIGEN_COL_VEC_T>(src.getV()(r,c));} 
     template<> VV<EIGEN_COL_VEC_T> op<EIGEN_COL_VEC_T, OP_T<OPs::OP_ADD>>(const VV<EIGEN_COL_VEC_T> &x, const VV<EIGEN_COL_VEC_T> &y) {return VV<EIGEN_COL_VEC_T>(x.getV() + y.getV());}     
     template<> VV<EIGEN_COL_VEC_T> op<EIGEN_COL_VEC_T, OP_T<OPs::OP_SUB>>(const VV<EIGEN_COL_VEC_T> &x, const VV<EIGEN_COL_VEC_T> &y) {return VV<EIGEN_COL_VEC_T>(x.getV() - y.getV());}  
     template<> VV<EIGEN_COL_VEC_T> op<EIGEN_COL_VEC_T, OP_T<OPs::OP_MUL>>(const VV<EIGEN_COL_VEC_T> &x, const VV<EIGEN_COL_VEC_T> &y) {return VV<EIGEN_COL_VEC_T>(x.getV() * y.getV());}
     template<> VV<EIGEN_COL_VEC_T> op<EIGEN_COL_VEC_T, OP_T<OPs::OP_DIV>>(const VV<EIGEN_COL_VEC_T> &x, const VV<EIGEN_COL_VEC_T> &y) {assert(false);}//return x/y;}  
     template<> VV<EIGEN_COL_VEC_T> op<EIGEN_COL_VEC_T, OP_T<OPs::OP_NEG>>(const VV<EIGEN_COL_VEC_T> &x) {return VV<EIGEN_COL_VEC_T>(-x.getV());}  
     template<> VV<EIGEN_COL_VEC_T> op<EIGEN_COL_VEC_T, OP_T<OPs::OP_TRANS>>(const VV<EIGEN_COL_VEC_T> &x) {return VV<EIGEN_COL_VEC_T>(x.getV().transpose());}  
     template<> VV<EIGEN_COL_VEC_T> op<EIGEN_COL_VEC_T, OP_T<OPs::OP_CONJ>>(const VV<EIGEN_COL_VEC_T> &x) {return VV<EIGEN_COL_VEC_T>(x.getV().conjugate());}  
     template<> VV<EIGEN_COL_VEC_T> op<EIGEN_COL_VEC_T, OP_T<OPs::OP_INVERSE>>(const VV<EIGEN_COL_VEC_T> &x) {return VV<EIGEN_COL_VEC_T>(x.getV().inverse());}  

     template<> VV<EIGEN_COL_VEC_T> opElem<EIGEN_COL_VEC_T, OP_T<OPs::OP_MUL>>(const VV<EIGEN_COL_VEC_T> &x, const VV<EIGEN_COL_VEC_T> &y) {EIGEN_COL_VEC_T v = x.getV(); return VV<EIGEN_COL_VEC_T>(v.cwiseProduct(y.getV()));}
     template<> VV<EIGEN_COL_VEC_T> opElem<EIGEN_COL_VEC_T, OP_T<OPs::OP_DIV>>(const VV<EIGEN_COL_VEC_T> &x, const VV<EIGEN_COL_VEC_T> &y) {EIGEN_COL_VEC_T v = x.getV(); return VV<EIGEN_COL_VEC_T>(v.cwiseQuotient(y.getV()));}
     template<> VV<EIGEN_COL_VEC_T> opElem<EIGEN_COL_VEC_T, OP_T<OPs::OP_MIN>>(const VV<EIGEN_COL_VEC_T> &x, const VV<EIGEN_COL_VEC_T> &y) {EIGEN_COL_VEC_T v = x.getV(); return VV<EIGEN_COL_VEC_T>(v.cwiseMin(y.getV()));}
     template<> VV<EIGEN_COL_VEC_T> opElem<EIGEN_COL_VEC_T, OP_T<OPs::OP_MAX>>(const VV<EIGEN_COL_VEC_T> &x, const VV<EIGEN_COL_VEC_T> &y) {EIGEN_COL_VEC_T v = x.getV(); return VV<EIGEN_COL_VEC_T>(v.cwiseMax(y.getV()));}
     template<> VV<EIGEN_COL_VEC_T> opElem<EIGEN_COL_VEC_T, OP_T<OPs::OP_SQRT>>(const VV<EIGEN_COL_VEC_T> &x) {EIGEN_COL_VEC_T v = x.getV(); return VV<EIGEN_COL_VEC_T>(v.cwiseSqrt());}
     template<> VV<EIGEN_COL_VEC_T> opElem<EIGEN_COL_VEC_T, OP_T<OPs::OP_ABS>>(const VV<EIGEN_COL_VEC_T> &x) {EIGEN_COL_VEC_T v = x.getV(); return VV<EIGEN_COL_VEC_T>(v.cwiseAbs());}

     template<> double opReduce<EIGEN_COL_VEC_T, OP_T<OPs::OP_ADD>>(const VV<EIGEN_COL_VEC_T> &x) {return x.getV().sum();}
     template<> double opReduce<EIGEN_COL_VEC_T, OP_T<OPs::OP_MUL>>(const VV<EIGEN_COL_VEC_T> &x) {return x.getV().prod();}
     template<> double opReduce<EIGEN_COL_VEC_T, OP_T<OPs::OP_MAX>>(const VV<EIGEN_COL_VEC_T> &x) {return x.getV().maxCoeff();}
     template<> double opReduce<EIGEN_COL_VEC_T, OP_T<OPs::OP_MIN>>(const VV<EIGEN_COL_VEC_T> &x) {return x.getV().minCoeff();}

   //Matrix
     template<> MV<EIGEN_MAT_T> opConst<EIGEN_MAT_T, OP_T<OPs::OP_SCALE>, double>(const double src1, const MV<EIGEN_MAT_T> &src0) {return MV<EIGEN_MAT_T>(src0.getV() * src1);}
     template<> MV<EIGEN_MAT_T> opConst<EIGEN_MAT_T, OP_T<OPs::OP_TRANSLATE>, double>(const double src1, const MV<EIGEN_MAT_T> &src0) {return MV<EIGEN_MAT_T>(src0.getV() + src1*EIGEN_MAT_T::Ones(src0.getV().rows(), src0.getV().cols()));}
     template<> MV<EIGEN_MAT_T> opConst<EIGEN_MAT_T, OP_T<OPs::OP_SUB>, double>(const double src0, const MV<EIGEN_MAT_T> &src1) {return MV<EIGEN_MAT_T>(src0*EIGEN_MAT_T::Ones(src1.getV().rows(), src1.getV().cols()) - src1.getV());}

     template<> MV<EIGEN_MAT_T> clone(const MV<EIGEN_MAT_T> &x) {
       EIGEN_MAT_T X = EIGEN_MAT_T(x.getV());
       return MV<EIGEN_MAT_T>(X);
     }
     template<> void appendRows<EIGEN_MAT_T>(MV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &app) {
       assert(size(x, 1) == size(app, 1));
       EIGEN_MAT_T &X = x.getV(); //EIGEN_MAT_T(x.getV());
       size_t nrRows = size(app, 0), R = X.rows();
       X.conservativeResize(X.rows()+nrRows, Eigen::NoChange); //X.cols());
       for(size_t r = 0; r < nrRows; r++)
           for(size_t c = 0; c < (size_t) X.cols(); c++)
               X(R + r,c) = app(r, c);
       //x.getV() << app.getV();
       //return MV<EIGEN_MAT_T>(X);
     }
     template<> void appendColumns<EIGEN_MAT_T>(MV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &app) {
       assert(size(x, 0) == size(app, 0));
       EIGEN_MAT_T &X = x.getV(); //EIGEN_MAT_T(x.getV());
       size_t nrCols = size(app, 1), C = X.cols();

       X.conservativeResize(Eigen::NoChange, X.cols()+nrCols); //(X.rows(),
       for(size_t c = 0; c < nrCols; c++)
           for(size_t r = 0; r < (size_t) X.rows(); r++)
               X(r,C + c) = app(r, c);
       //return MV<EIGEN_MAT_T>(X);
     }
     template<> void appendRow<EIGEN_MAT_T, EIGEN_ROW_VEC_T>(MV<EIGEN_MAT_T> &x, const VV<EIGEN_ROW_VEC_T> &app) {
       assert(size(app, 0) == 1);

       EIGEN_MAT_T &X = x.getV(); //EIGEN_MAT_T(x.getV());
       if(size(x, 1) == 0) {
          X = EIGEN_MAT_T(app.getV());
          return;
       }
       assert(size(x, 1) == size(app, 1));
       size_t R = X.rows();

       X.conservativeResize(X.rows()+1, Eigen::NoChange); //X.cols());
       for(size_t c = 0; c < (size_t) X.cols(); c++)
           X(R,c) = app(c);
     }
     template<> void appendColumn<EIGEN_MAT_T, EIGEN_ROW_VEC_T>(MV<EIGEN_MAT_T> &x, const VV<EIGEN_ROW_VEC_T> &app) {
       assert(size(app, 1) == 1);

       EIGEN_MAT_T &X = x.getV(); //EIGEN_MAT_T(x.getV());
       if(size(x, 1) == 0) {
          X = EIGEN_MAT_T(app.getV());
          return;
       }
       assert(size(x, 0) == size(app, 0));
       size_t C = X.cols();

       X.conservativeResize(Eigen::NoChange, X.cols()+1); //(X.rows(),
       for(size_t r = 0; r < (size_t) X.rows(); r++)
           X(r,C) = app(r);
     }
     template<> void appendRow<EIGEN_MAT_T, EIGEN_COL_VEC_T>(MV<EIGEN_MAT_T> &x, const VV<EIGEN_COL_VEC_T> &app) {
       assert(size(app, 0) == 1);
       assert(size(x, 1) == size(app, 1));
       EIGEN_MAT_T &X = x.getV(); //EIGEN_MAT_T(x.getV());
       size_t R = X.rows();

       X.conservativeResize(X.rows()+1, Eigen::NoChange); //X.cols());
       for(size_t c = 0; c < (size_t) X.cols(); c++)
           X(R,c) = app(c);
       //x.getV() << app.getV();
       //return MV<EIGEN_MAT_T>(X);
     }
     template<> void appendColumn<EIGEN_MAT_T, EIGEN_COL_VEC_T>(MV<EIGEN_MAT_T> &x, const VV<EIGEN_COL_VEC_T> &app) {
       assert(size(app, 1) == 1);
       assert(size(x, 0) == size(app, 0));
       EIGEN_MAT_T &X = x.getV(); //EIGEN_MAT_T(x.getV());
       size_t C = X.cols();

       X.conservativeResize(Eigen::NoChange, X.cols()+1); //(X.rows(),
       for(size_t r = 0; r < (size_t) X.rows(); r++)
           X(r,C) = app(r);
       //return MV<EIGEN_MAT_T>(X);
     }


     template<> size_t size<EIGEN_MAT_T>(const MV<EIGEN_MAT_T> &x, size_t dim) {
        if(dim == 0)
           return x.getV().rows();
        else if(dim == 1)
                return x.getV().cols();
        else
            assert(false && "Invalid dimension");
     }
     template<> MV<EIGEN_MAT_T> create<EIGEN_MAT_T, double>(size_t r, size_t c, const double constantVal) {
       EIGEN_MAT_T m = constantVal*EIGEN_MAT_T::Ones(r, c);
       return MV<EIGEN_MAT_T>(m);
     }
     template<> MV<EIGEN_MAT_T> create(size_t r, size_t c, SPECIAL_MATRICES type) {
       switch(type) {
        case SPECIAL_MATRICES::MATRIX_IDENT:
          {
            assert(r == c); //Allow for non-square identity?
            EIGEN_MAT_T m = EIGEN_MAT_T::Identity(r, c);
            return MV<EIGEN_MAT_T>(m);
          } break;
        case SPECIAL_MATRICES::MATRIX_HANKEL:
          {
            assert(false);
          } break;
        default:
            assert(false && "Unhandled special matrix type");
       }
     }
     template<> MV<EIGEN_MAT_T> create<EIGEN_MAT_T>(size_t r, size_t c, INITIALIZATIONs init) {
       switch(init) {
        case INITIALIZATIONs::INIT_RANDOM:  return MV<EIGEN_MAT_T>(EIGEN_MAT_T::Random(r, c));
        case INITIALIZATIONs::INIT_ZERO:    return MV<EIGEN_MAT_T>(EIGEN_MAT_T::Zero(r, c));
        case INITIALIZATIONs::INIT_ONE:     return MV<EIGEN_MAT_T>(EIGEN_MAT_T::Ones(r, c));
        case INITIALIZATIONs::INIT_NONE:    return MV<EIGEN_MAT_T>(EIGEN_MAT_T(r, c));
        default:
            assert(false && "Unhandled init type");
       }
     }
     template<> void set<EIGEN_MAT_T>(MV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &src) {
       EIGEN_MAT_T v(src.getV());
       x.setV(v);
     }
     template<> void set<EIGEN_MAT_T, double>(MV<EIGEN_MAT_T> &dst, const double src1) {
       EIGEN_MAT_T &v = dst.getV();
       v.setConstant(v.rows(), v.cols(), src1);
     }
     template<> void set<EIGEN_MAT_T, double>(MV<EIGEN_MAT_T> &dst, const double *src1, size_t n) {
       EIGEN_MAT_T &v = dst.getV();
       for(size_t r = 0, m = 0; r < (size_t) v.rows(); r++)
           for(size_t c = 0; c < (size_t) v.cols(); c++)
               if((size_t) m++ < n)
                  v(r,c) = src1[m];
               //else
               //    return MV<EIGEN_MAT_T>(v);
     }

     template<> void set<EIGEN_MAT_T, double>(MV<EIGEN_MAT_T> &dst, size_t r, size_t c, const double src1) {
       EIGEN_MAT_T &v = dst.getV();
       v(r, c) = src1;
     }
     template<> void set<EIGEN_MAT_T, double>(MV<EIGEN_MAT_T> &dst, size_t rb, size_t r_len, size_t cb, size_t c_len, const double src1) {
       EIGEN_MAT_T &v = dst.getV();
       v.block(rb, r_len, cb, c_len).setConstant(src1);
     }
     template<> void set<EIGEN_MAT_T, int>(MV<EIGEN_MAT_T> &dst, const int src1) {
       EIGEN_MAT_T &v = dst.getV();
       v.setConstant(v.rows(), v.cols(), src1);
     }
     template<> void set<EIGEN_MAT_T, int>(MV<EIGEN_MAT_T> &dst, size_t r, size_t c, const int src1) {
       EIGEN_MAT_T &v = dst.getV();
       v(r, c) = src1;
     }
     template<> void set<EIGEN_MAT_T, int>(MV<EIGEN_MAT_T> &dst, size_t rb, size_t r_len, size_t cb, size_t c_len, const int src1) {
       EIGEN_MAT_T &v = dst.getV();
       v.block(rb, r_len, cb, c_len).setConstant(src1);
     }


     template<> double toDouble<EIGEN_MAT_T>(const MV<EIGEN_MAT_T> &dst) {
       if((dst.getV().rows() != 1) && (dst.getV().cols() != 1))
          throw MatWlibException("Matrix is not (1,1)");
       return dst.getV()(1,1);
     }
     template<> VV<EIGEN_ROW_VEC_T> row<EIGEN_MAT_T,EIGEN_ROW_VEC_T>(const MV<EIGEN_MAT_T> &src, size_t r) {return VV<EIGEN_ROW_VEC_T>(src.getV().block(r,1,1,src.getV().cols()));} 
     template<> VV<EIGEN_COL_VEC_T> col<EIGEN_MAT_T,EIGEN_COL_VEC_T>(const MV<EIGEN_MAT_T> &src, size_t c) {return VV<EIGEN_COL_VEC_T>(src.getV().block(1,c,src.getV().rows(),1));}
     template<> double elem<EIGEN_MAT_T>(const MV<EIGEN_MAT_T> &src, size_t r, size_t c) {return src.getV()(r,c);} 
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_ADD>>(const MV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {return MV<EIGEN_MAT_T>(x.getV() + y.getV());}     
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_SUB>>(const MV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {return MV<EIGEN_MAT_T>(x.getV() - y.getV());}  
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_MUL>>(const MV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {return MV<EIGEN_MAT_T>(x.getV() * y.getV());}
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_DIV>>(const MV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {assert(false);}//return x/y;}  
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_NEG>>(const MV<EIGEN_MAT_T> &x) {return MV<EIGEN_MAT_T>(-x.getV());}  
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_TRANS>>(const MV<EIGEN_MAT_T> &x) {return MV<EIGEN_MAT_T>(x.getV().transpose());}  
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_CONJ>>(const MV<EIGEN_MAT_T> &x) {return MV<EIGEN_MAT_T>(x.getV().conjugate());}  
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_INVERSE>>(const MV<EIGEN_MAT_T> &x) {return MV<EIGEN_MAT_T>(x.getV().inverse());}  

     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_ADD>>(const VV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {return MV<EIGEN_MAT_T>(x.getV() + y.getV());}     
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_SUB>>(const VV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {return MV<EIGEN_MAT_T>(x.getV() - y.getV());}  
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_MUL>>(const VV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {return MV<EIGEN_MAT_T>(x.getV() * y.getV());}
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_DIV>>(const VV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {assert(false);}//return x/y;}  

     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_ADD>>(const MV<EIGEN_MAT_T> &x, const VV<EIGEN_MAT_T> &y) {return MV<EIGEN_MAT_T>(x.getV() + y.getV());}     
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_SUB>>(const MV<EIGEN_MAT_T> &x, const VV<EIGEN_MAT_T> &y) {return MV<EIGEN_MAT_T>(x.getV() - y.getV());}  
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_MUL>>(const MV<EIGEN_MAT_T> &x, const VV<EIGEN_MAT_T> &y) {return MV<EIGEN_MAT_T>(x.getV() * y.getV());}
     template<> MV<EIGEN_MAT_T> op<EIGEN_MAT_T, OP_T<OPs::OP_DIV>>(const MV<EIGEN_MAT_T> &x, const VV<EIGEN_MAT_T> &y) {assert(false);}//return x/y;}  

     template<> MV<EIGEN_MAT_T> opElem<EIGEN_MAT_T, OP_T<OPs::OP_MUL>>(const MV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {EIGEN_MAT_T v = x.getV(); return MV<EIGEN_MAT_T>(v.cwiseProduct(y.getV()));}
     template<> MV<EIGEN_MAT_T> opElem<EIGEN_MAT_T, OP_T<OPs::OP_DIV>>(const MV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {EIGEN_MAT_T v = x.getV(); return MV<EIGEN_MAT_T>(v.cwiseQuotient(y.getV()));}
     template<> MV<EIGEN_MAT_T> opElem<EIGEN_MAT_T, OP_T<OPs::OP_MIN>>(const MV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {EIGEN_MAT_T v = x.getV(); return MV<EIGEN_MAT_T>(v.cwiseMin(y.getV()));}
     template<> MV<EIGEN_MAT_T> opElem<EIGEN_MAT_T, OP_T<OPs::OP_MAX>>(const MV<EIGEN_MAT_T> &x, const MV<EIGEN_MAT_T> &y) {EIGEN_MAT_T v = x.getV(); return MV<EIGEN_MAT_T>(v.cwiseMax(y.getV()));}
     template<> MV<EIGEN_MAT_T> opElem<EIGEN_MAT_T, OP_T<OPs::OP_SQRT>>(const MV<EIGEN_MAT_T> &x) {EIGEN_MAT_T v = x.getV(); return MV<EIGEN_MAT_T>(v.cwiseSqrt());}
     template<> MV<EIGEN_MAT_T> opElem<EIGEN_MAT_T, OP_T<OPs::OP_ABS>>(const MV<EIGEN_MAT_T> &x) {EIGEN_MAT_T v = x.getV(); return MV<EIGEN_MAT_T>(v.cwiseAbs());}

     template<> double opReduce<EIGEN_MAT_T, OP_T<OPs::OP_ADD>>(const MV<EIGEN_MAT_T> &x) {return x.getV().sum();}
     template<> double opReduce<EIGEN_MAT_T, OP_T<OPs::OP_MUL>>(const MV<EIGEN_MAT_T> &x) {return x.getV().prod();}
     template<> double opReduce<EIGEN_MAT_T, OP_T<OPs::OP_MAX>>(const MV<EIGEN_MAT_T> &x) {return x.getV().maxCoeff();}
     template<> double opReduce<EIGEN_MAT_T, OP_T<OPs::OP_MIN>>(const MV<EIGEN_MAT_T> &x) {return x.getV().minCoeff();}
}

#endif

