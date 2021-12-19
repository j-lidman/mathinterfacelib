#ifndef EIGEN_HDR
#define EIGEN_HDR

#ifdef HAS_EIGEN

#include <stdexcept>
#include <string>

#include "matwlib.h"

#include <Eigen/Dense>

namespace MatWLib {

   typedef Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic> EIGEN_MAT_T; //Eigen::MatrixXd
   typedef Eigen::Matrix<double,1,Eigen::Dynamic> EIGEN_ROW_VEC_T; //= Eigen::VectorXd = Eigen::RowVectorXd
   typedef Eigen::Matrix<double,Eigen::Dynamic,1> EIGEN_COL_VEC_T;

   /*class Eigen_State : public State {
      public:
         typedef std::string NAME;
         typedef EIGEN_MAT_T MATRIX_T;
         typedef EIGEN_VEC_T VECTOR_T;
      private:
         std::map<std::string, MATRIX_T> mapMatrices;
         std::map<std::string, VECTOR_T> mapVectors;

         MATRIX_T &getMat(std::string varName);
         const MATRIX_T &getMat(std::string varName) const;
         VECTOR_T &getVec(std::string varName);
         const VECTOR_T &getVec(std::string varName) const;
      public:
         //(De/Con)structor(s)
           Eigen_State();
           virtual ~Eigen_State();
         //Misc. functions
           virtual std::string toString() const;
           virtual bool good() const;
         //Variable handling
           void addVar(std::string varName, EIGEN_MAT_T mat);
           void addVar(std::string varName, EIGEN_VEC_T vec);
           void remVarMat(std::string varName);
           void remVarVec(std::string varName);
           const EIGEN_MAT_T &getVarMat(std::string varName) const;
           EIGEN_MAT_T &getVarMat(std::string varName);
           EIGEN_VEC_T &getVarVec(std::string varName);
           const EIGEN_VEC_T &getVarVec(std::string varName) const;
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
   };

   template<> struct DataInterface<Eigen_State> {
    private:
      Eigen_State &s;
    public:
    //(De/Con)structor(s)
      DataInterface(Eigen_State &s_);
      ~DataInterface();
    //Write data
      template<class T> void writeVar(const Eigen_State::NAME varName, T val);
      template<class T> void writeVar(const Eigen_State::NAME varName, size_t m, size_t n, const T *mat);  
      template<class IT> void writeVar(const Eigen_State::NAME varName, size_t m, size_t n, IT begin, IT end);
    //Read data
      template<class T> T readVar(const Eigen_State::NAME varName);
      template<class T> T *readVar(const Eigen_State::NAME varName, std::vector<size_t> &szDims);
      template<class IT> void readVar(const Eigen_State::NAME varName, std::vector<size_t> &szDims, IT begin, IT end);
   };

   Eigen_State *init_eigen_engine();
   void finit_eigen_engine(Eigen_State *s);*/

   template<> size_t size<EIGEN_MAT_T>(const MV<EIGEN_MAT_T> &x, size_t dim);

   //Type traits
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_ADD>> : public boost::true_type {};
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_SUB>> : public boost::true_type {};
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_MUL>> : public boost::true_type {};
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_DIV>> : public boost::true_type {};

     template<> struct has_op<EIGEN_ROW_VEC_T, EIGEN_MAT_T, OP_T<OPs::OP_ADD>> : public boost::true_type {};
     template<> struct has_op<EIGEN_ROW_VEC_T, EIGEN_MAT_T, OP_T<OPs::OP_SUB>> : public boost::true_type {};
     template<> struct has_op<EIGEN_ROW_VEC_T, EIGEN_MAT_T, OP_T<OPs::OP_MUL>> : public boost::true_type {};
     template<> struct has_op<EIGEN_ROW_VEC_T, EIGEN_MAT_T, OP_T<OPs::OP_DIV>> : public boost::true_type {};
     template<> struct has_op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_MIN>> : public boost::true_type {};
     template<> struct has_op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_MAX>> : public boost::true_type {};
     template<> struct has_op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_SQRT>> : public boost::true_type {};
     template<> struct has_op<EIGEN_ROW_VEC_T, OP_T<OPs::OP_ABS>> : public boost::true_type {};

     template<> struct has_op<EIGEN_COL_VEC_T, EIGEN_MAT_T, OP_T<OPs::OP_ADD>> : public boost::true_type {};
     template<> struct has_op<EIGEN_COL_VEC_T, EIGEN_MAT_T, OP_T<OPs::OP_SUB>> : public boost::true_type {};
     template<> struct has_op<EIGEN_COL_VEC_T, EIGEN_MAT_T, OP_T<OPs::OP_MUL>> : public boost::true_type {};
     template<> struct has_op<EIGEN_COL_VEC_T, EIGEN_MAT_T, OP_T<OPs::OP_DIV>> : public boost::true_type {};
     template<> struct has_op<EIGEN_COL_VEC_T, OP_T<OPs::OP_MIN>> : public boost::true_type {};
     template<> struct has_op<EIGEN_COL_VEC_T, OP_T<OPs::OP_MAX>> : public boost::true_type {};
     template<> struct has_op<EIGEN_COL_VEC_T, OP_T<OPs::OP_SQRT>> : public boost::true_type {};
     template<> struct has_op<EIGEN_COL_VEC_T, OP_T<OPs::OP_ABS>> : public boost::true_type {};

     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_NEG>> : public boost::true_type {};
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_CONJ>> : public boost::true_type {};
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_TRANS>> : public boost::true_type {};
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_INVERSE>> : public boost::true_type {};
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_SCALE>> : public boost::true_type {};
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_MIN>> : public boost::true_type {};
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_MAX>> : public boost::true_type {};
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_SQRT>> : public boost::true_type {};
     template<> struct has_op<EIGEN_MAT_T, OP_T<OPs::OP_ABS>> : public boost::true_type {};

}

#endif

#endif

