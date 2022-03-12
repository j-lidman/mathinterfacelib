#ifdef HAS_MATLAB

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

#include "MATLAB_Engine.h"

using namespace std;
using namespace boost;
using namespace boost::interprocess;
namespace qi = boost::spirit::qi;

namespace MathInterfaceLib {
   char *getNewStr(MATLAB_Interface *extint, const char *c) {
      char *newStr = (char *) extint->mxMalloc(strlen(c)+1);
      memcpy(newStr, c, strlen(c)+1);
      return newStr;
   }

   MATLAB_State *init_matlab_engine(std::string path, boost::interprocess::shared_memory_object *shm) {
      //#ifdef DEBUGGING
         mtrace ();
      //#endif
      //Dynamically load Matlabs external interface library
        MATLAB_Interface *extint = new MATLAB_Interface();
        string pathLibEng, pathLibMX, pathLibMat;
        if(path != "") {
           pathLibEng = path + "/libeng.so";
           pathLibMX = path + "/libmx.so";
           pathLibMat = path + "/libmat.so";
        } else {
           pathLibEng = "libeng.so";
           pathLibMX = "libmx.so";
           pathLibMat = "libmat.so";
        }
        if((extint->handleEng = dlopen(pathLibEng.c_str(), RTLD_LAZY)) == NULL)
           throw MathInterfacelibException("Unable to load '" + pathLibEng + "'");
        if((extint->handleMX = dlopen(pathLibMX.c_str(), RTLD_LAZY)) == NULL)
           throw MathInterfacelibException("Unable to load '" + pathLibMX + "'");
        if((extint->handleMat = dlopen(pathLibMat.c_str(), RTLD_LAZY)) == NULL)
           throw MathInterfacelibException("Unable to load '" + pathLibMat + "'");
      //Setup symbol pointers
        if((extint->engEvalString = (engEvalString_Ptr) dlsym(extint->handleEng, "engEvalString")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'engEvalString'");
        if((extint->engOpen = (engOpen_Ptr) dlsym(extint->handleEng, "engOpen")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'engOpen'");
        if((extint->engSetVisible = (engSetVisible_Ptr) dlsym(extint->handleEng, "engSetVisible")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'engSetVisible'");
        if((extint->engClose = (engClose_Ptr) dlsym(extint->handleEng, "engClose")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'engClose'");
        if((extint->engGetVariable = (engGetVariable_Ptr) dlsym(extint->handleEng, "engGetVariable")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'engGetVariable'");
        if((extint->engPutVariable = (engPutVariable_Ptr) dlsym(extint->handleEng, "engPutVariable")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'engPutVariable'");
        if((extint->engOutputBuffer = (engOutputBuffer_Ptr) dlsym(extint->handleEng, "engOutputBuffer")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'engOutputBuffer'");

        if((extint->mxCalcSingleSubscript = (mxCalcSingleSubscript_Ptr) dlsym(extint->handleMX, "mxCalcSingleSubscript")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxCalcSingleSubscript'");

        if((extint->mxCreateNumericMatrix = (mxCreateNumericMatrix_Ptr) dlsym(extint->handleMX, "mxCreateNumericMatrix")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxCreateNumericMatrix'");
        if((extint->mxCreateDoubleScalar = (mxCreateDoubleScalar_Ptr) dlsym(extint->handleMX, "mxCreateDoubleScalar")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxCreateDoubleScalar'");
        if((extint->mxDestroyArray = (mxDestroyArray_Ptr) dlsym(extint->handleMX, "mxDestroyArray")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxDestroyArray'");
        if((extint->mxGetPr = (mxGetPr_Ptr) dlsym(extint->handleMX, "mxGetPr")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxGetPr'");
        if((extint->mxGetM = (mxGetM_Ptr) dlsym(extint->handleMX, "mxGetM")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxGetM'");
        if((extint->mxGetN = (mxGetN_Ptr) dlsym(extint->handleMX, "mxGetN")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxGetN'");
        if((extint->mxGetCell = (mxGetCell_Ptr) dlsym(extint->handleMX, "mxGetCell")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxGetCell'");
        if((extint->mxGetClassID = (mxGetClassID_Ptr) dlsym(extint->handleMX, "mxGetClassID")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxGetClassID'");
        if((extint->mxGetClassName = (mxGetClassName_Ptr) dlsym(extint->handleMX, "mxGetClassName")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxGetClassName'");
        if((extint->mxGetDimensions = (mxGetDimensions_Ptr) dlsym(extint->handleMX, "mxGetDimensions")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxGetDimensions'");
        if((extint->mxGetNumberOfDimensions = (mxGetNumberOfDimensions_Ptr) dlsym(extint->handleMX, "mxGetNumberOfDimensions")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxGetNumberOfDimensions'");
        if((extint->mxArrayToString = (mxArrayToString_Ptr) dlsym(extint->handleMX, "mxArrayToString")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxArrayToString'");
        if((extint->mxCreateString = (mxCreateString_Ptr) dlsym(extint->handleMX, "mxCreateString")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxCreateString'");
        if((extint->mxFree = (mxFree_Ptr) dlsym(extint->handleMX, "mxFree")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxFree'");
        if((extint->mxMalloc = (mxMalloc_Ptr) dlsym(extint->handleMX, "mxMalloc")) == NULL)
           throw MathInterfacelibException("Unable to find address to symbol 'mxMalloc'");

      //Initialize engine
        Engine *eng = NULL;
        std::string startCmd = ""; //Use default path
                    //startCmd = "setenv LD_PRELOAD /usr/lib/x86_64-linux-gnu/libstdc++.so.6; " + path + "/MATLAB"; //Pre-load system libctdc++
                    //startCmd = "MATLAB_JAVA=" MATLAB_PATH "/sys/java/jre/glnx64/jre " + path + "/MATLAB -desktop -nosplash"; //Debugging

        char *cStartCmd = new char[startCmd.size()+1];
        for(size_t i = 0; i < startCmd.size(); i++)
            cStartCmd[i] = startCmd[i];
        cStartCmd[startCmd.size()] = '\0';

        if((eng = extint->engOpen(cStartCmd)) == NULL) // "")) == NULL)
		     throw MathInterfacelibException("Unable to init matlab engine");
         extint->engSetVisible(eng, true);

      return new MATLAB_State(extint, eng, shm);
   }
   void finit_matlab_engine(MATLAB_State *s) {
      //Close MATLAB engine
        s->getExtInt()->engClose(s->getEng());
      //Close library handles
        if(dlclose(s->getExtInt()->handleEng) != 0)
           throw MathInterfacelibException("Unable to close 'libEng'"); 
        if(dlclose(s->getExtInt()->handleMX) != 0)
           throw MathInterfacelibException("Unable to close 'libmx'"); 
        if(dlclose(s->getExtInt()->handleMat) != 0)
           throw MathInterfacelibException("Unable to close 'libmat'"); 
      //Destroy shared memory
        shared_memory_object *shm = s->getSHM();
        if(shm != NULL) {
           mapped_region region(*shm,read_write);
           void *addr = region.get_address();
           SharedRegion *data = new (addr) SharedRegion;
           data->regionInUse.unlock();
        }

      delete s->getExtInt();
      delete s;
   }
   template<> MATLAB_State *cast_state<MATLAB_State>(State *s) {
      MATLAB_State *s_ = dynamic_cast<MATLAB_State *>(s);
      assert((s_ != NULL) && "Invalid MATLAB State");
      return s_;
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   MATLAB_State::MATLAB_State(MATLAB_Interface *extint_, Engine *eng_, boost::interprocess::shared_memory_object *shm_)
     :  extint(extint_), eng(eng_), shm(shm_) {
      std::stringstream ssTmp, ssErr;

      ssTmp << MATHINTERFACELIB_MATLAB_TEMP_VAR;
      this->tmpVariable = ssTmp.str();
      ssErr << MATHINTERFACELIB_MATLAB_ERR_VAR;
      this->errVariable = ssErr.str();

      this->writeVar(this->tmpVariable, 0.0);
      this->writeVar(this->errVariable, 0.0);
   }
   MATLAB_State::~MATLAB_State() {
     if(shm != NULL) {
        delete shm;
        boost::interprocess::shared_memory_object::remove(MATHINTERFACELIB_SHARED_MEM_NAME);
     }
   }
   MATLAB_State::NAME MATLAB_State::getTempVar() const {
     return this->tmpVariable;
   }
   MATLAB_State::NAME MATLAB_State::getNewName() {
     std::stringstream ss;
     ss << "var" << getUniqueNumber();
     return ss.str();
   }
   std::string MATLAB_State::toString() const {return "MATLAB State";}
   bool MATLAB_State::good() const {return (this->eng != NULL);}

   std::string MATLAB_State::toStringVariable(const std::string strVarName) const {
     //Read variable name
       mxArray *arr = NULL;
       if((arr = this->getExtInt()->engGetVariable(this->getEng(), strVarName.c_str())) == NULL)
           throw MathInterfacelibException("No variable with this name has been defined");
    //Check type and convert
      size_t nDim = getExtInt()->mxGetNumberOfDimensions(arr);
      const mwSize *dimSz = getExtInt()->mxGetDimensions(arr);

      double *dblArr = getExtInt()->mxGetPr(arr);
      size_t *intArr = (size_t *) getExtInt()->mxGetPr(arr);

      std::stringstream ss;
      switch(getExtInt()->mxGetClassID(arr)) {
       case mxUINT32_CLASS:
         {
            if(sizeof(size_t) != 4) {
               stringstream ss;
               ss << "Invalid size for mxUINT32_CLASS array, got " << getExtInt()->mxGetClassName(arr);
               throw MathInterfacelibException(ss.str());
            }
            if(nDim == 2) {
               for(size_t c = 0; c < dimSz[0]; c++) {
                   ss << (c == 0 ? "" : "; ");
                   for(size_t r = 0; r < dimSz[1]; r++)
                       ss << (r == 0 ? "" : " ") << intArr[c*dimSz[1] + r];
               }
            } else
                  assert(false && "TODO");
         } break;
       case mxUINT64_CLASS:
         {
            if(sizeof(size_t) != 8) {
               stringstream ss;
               ss << "Invalid size for mxUINT64_CLASS array, got " << getExtInt()->mxGetClassName(arr);
               throw MathInterfacelibException(ss.str());
            }
            if(nDim == 2) {
               for(size_t c = 0; c < dimSz[0]; c++) {
                   ss << (c == 0 ? "" : "; ");
                   for(size_t r = 0; r < dimSz[1]; r++)
                       ss << (r == 0 ? "" : " ") << intArr[c*dimSz[1] + r];
               }
            } else
                  assert(false && "TODO");
         } break;
       case mxDOUBLE_CLASS:
         {
            if(nDim == 2) {
               for(size_t c = 0; c < dimSz[0]; c++) {
                   ss << (c == 0 ? "" : "; ");
                   for(size_t r = 0; r < dimSz[1]; r++)
                       ss << (r == 0 ? "" : " ") << dblArr[c*dimSz[1] + r];
               }
            } else
                  assert(false && "TODO");
         } break;
       default:
         {
            stringstream ss;
            ss << "Unknown value type, Got " << getExtInt()->mxGetClassName(arr);
            throw MathInterfacelibException(ss.str());
         };
      }
    //Cleanup
      this->getExtInt()->mxDestroyArray(arr);

    return ss.str();
   }

   MATLAB_Interface *MATLAB_State::getExtInt() const {return extint;}
   Engine *MATLAB_State::getEng() const {return eng;}
   boost::interprocess::shared_memory_object *MATLAB_State::getSHM() {return shm;}

   bool MATLAB_State::registerFunction(std::string funcName, Function *funcDesc) {
     if(funcMap.find(funcName) != funcMap.end())
        return false;
     funcMap.insert( std::make_pair(funcName, funcDesc) );
     return true;
   }
   bool MATLAB_State::updateFunction(std::string funcName, Function::FUNC_T fn) {
     std::map<std::string, Function *>::iterator it = funcMap.find(funcName);
     if(it == funcMap.end())
        return false;
     it->second->funcHandler = fn;
     return true;
   }
   bool MATLAB_State::removeFunction(std::string funcName) {
     std::map<std::string, Function *>::iterator it = funcMap.find(funcName);
     if(it == funcMap.end())
        return false;
     delete it->second;
     funcMap.erase(it);
     return true;
   }

   template<> size_t MATLAB_State::decodeValue<size_t>(mxArray *arr) {
      std::vector<size_t> szDims;
      size_t *ptr = decodeArray<size_t>(arr, szDims);
      //Check size
        if((szDims.size() != 2) || (szDims[0] != 1) || (szDims[1] != 1)) {
            stringstream ss;
            ss << "Expected scalar, got " << szDims[1] << "x" << szDims[0] << " array";
            throw MathInterfacelibException(ss.str());
        }
      return ptr[0];
   }
   template<> double MATLAB_State::decodeValue<double>(mxArray *arr) {
      std::vector<size_t> szDims;
      double *ptr = decodeArray<double>(arr, szDims);
      //Check size
        if((szDims.size() != 2) || (szDims[0] != 1) || (szDims[1] != 1)) {
            stringstream ss;
            ss << "Expected scalar, got " << szDims[1] << "x" << szDims[0] << " array";
            throw MathInterfacelibException(ss.str());
        }
      return ptr[0];
   }
   template<> std::string MATLAB_State::decodeValue<std::string>(mxArray *arr) {
      char *oldStr = getExtInt()->mxArrayToString(arr), *newStr = new char[strlen(oldStr)+1];
      memcpy((void *)newStr, (void *)oldStr, strlen(oldStr)+1);
      getExtInt()->mxFree(oldStr);
      return std::string(newStr);
   }
   template<> size_t *MATLAB_State::decodeArray<size_t>(mxArray *arr, std::vector<size_t> &szDims) {
      //Check type
        switch(sizeof(size_t)) {
         case 4:    if(getExtInt()->mxGetClassID(arr) != mxUINT32_CLASS) {
                       stringstream ss;
                       ss << "Expected mxUINT32_CLASS array, got " << getExtInt()->mxGetClassName(arr);
                       throw MathInterfacelibException(ss.str());
                    }
                    break;
         case 8:    if(getExtInt()->mxGetClassID(arr) != mxUINT64_CLASS) {
                       stringstream ss;
                       ss << "Expected mxUINT64_CLASS array, got " << getExtInt()->mxGetClassName(arr);
                       throw MathInterfacelibException(ss.str());
                    }
                    break;
         default:   throw MathInterfacelibException("Undefined size_t size in writeVar()");
        }
      //Get size
        size_t nDim = getExtInt()->mxGetNumberOfDimensions(arr), totSize = 1;
        const mwSize *dimSz = getExtInt()->mxGetDimensions(arr);
        for(size_t i = 0; i < nDim; i++) {
            szDims.push_back(dimSz[i]);
            totSize *= dimSz[i];
        }
      //Copy data
        size_t *newArr = new size_t[totSize], *oldArr = (size_t *) getExtInt()->mxGetPr(arr);
        memcpy((void *)newArr, (void *)oldArr, totSize*sizeof(size_t));
      return newArr;
   }
   template<> double *MATLAB_State::decodeArray<double>(mxArray *arr, std::vector<size_t> &szDims) {
      //Check type
        if(getExtInt()->mxGetClassID(arr) != mxDOUBLE_CLASS) {
           stringstream ss;
           ss << "Expected mxDOUBLE array, got " << getExtInt()->mxGetClassName(arr);
           throw MathInterfacelibException(ss.str());
        }
      //Get size
        size_t nDim = getExtInt()->mxGetNumberOfDimensions(arr), totSize = 1;
        const mwSize *dimSz = getExtInt()->mxGetDimensions(arr);
        for(size_t i = 0; i < nDim; i++) {
            szDims.push_back(dimSz[i]);
            totSize *= dimSz[i];
        }
      //Copy data
        double *newArr = new double[totSize], *oldArr = getExtInt()->mxGetPr(arr);
        memcpy((void *)newArr, (void *)oldArr, totSize*sizeof(double));
      return newArr;
   }
   template<> std::string *MATLAB_State::decodeArray<std::string>(mxArray *arr, std::vector<size_t> &szDims) {
      assert(false);
   }

   TYPE MATLAB_State::decodeType(mxArray *arr) {
      switch(this->getExtInt()->mxGetClassID(arr)) {
       case mxUNKNOWN_CLASS: return TYPE_UNKNOWN;
       case mxCELL_CLASS:    return TYPE_CELL;
       case mxSTRUCT_CLASS:  return TYPE_STRUCT;
       case mxCHAR_CLASS:    return TYPE_STRING;
       case mxDOUBLE_CLASS:  return TYPE_DOUBLE;
       case mxSINGLE_CLASS:  return TYPE_DOUBLE;
       case mxUINT32_CLASS:  if(sizeof(size_t) == 4)
                                return TYPE_INT;
                             else
                                 assert(false);
       case mxUINT64_CLASS:  if(sizeof(size_t) == 8)
                                return TYPE_INT;
                             else
                                 assert(false);
       default: 
         {
            stringstream ss;
            ss << "Unhandled class type " << this->getExtInt()->mxGetClassName(arr);
            throw MathInterfacelibException(ss.str());
         } break;
      }
   }

   Value *MATLAB_State::decodeData(Data *d) {
      MATLAB_Data *d_ = dynamic_cast<MATLAB_Data *>(d);
      if(d_ == NULL)
         throw MathInterfacelibException("Data is not from MATLAB");
      mxArray *arr = d_->getArray();
      size_t nDim = getExtInt()->mxGetNumberOfDimensions(arr);
      if(nDim == 1)
         switch(decodeType(arr)) {
          case TYPE_UNKNOWN:  throw MathInterfacelibException("Can't decode data, type i unknown");
          case TYPE_CELL:     assert(false);
          case TYPE_STRUCT:   assert(false);
          case TYPE_STRING:   return new Value_Scalar<std::string>(this->decodeValue<std::string>(arr));
          case TYPE_DOUBLE:   return new Value_Scalar<double>(this->decodeValue<double>(arr));
          case TYPE_INT:      return new Value_Scalar<size_t>(this->decodeValue<size_t>(arr));
         }
      else {
         std::vector<size_t> szDims;
         switch(decodeType(arr)) {
          case TYPE_UNKNOWN:  throw MathInterfacelibException("Can't decode data, type i unknown");
          case TYPE_CELL:     assert(false);
          case TYPE_STRUCT:   assert(false);
          case TYPE_STRING:   return new Value_RawMatrix<std::string>(this->decodeArray<std::string>(arr, szDims), nDim, szDims);
          case TYPE_DOUBLE:   return new Value_RawMatrix<double>(this->decodeArray<double>(arr, szDims), nDim, szDims);
          case TYPE_INT:      return new Value_RawMatrix<size_t>(this->decodeArray<size_t>(arr, szDims), nDim, szDims);
         }
      }
      assert(false);
   }
   void MATLAB_State::writeVar(std::string varName, Data *d) {
     assert(false);
   }
   Data *MATLAB_State::readVar(std::string varName) {
      mxArray *arr = NULL;
      if((arr = getExtInt()->engGetVariable(getEng(), varName.c_str())) == NULL)
          throw MathInterfacelibException("No variable with this name has been defined");
      else
         return new MATLAB_Data(this, arr);
   }
   void MATLAB_State::writeVar(std::string varName, RawDataInterface<std::string>::CELL_FUNC_T fn) {
     assert(false);
   }
   void MATLAB_State::readVar(std::string varName, RawDataInterface<std::string>::CELL_FUNC_T fn) {
      MATLAB_Data *d = dynamic_cast<MATLAB_Data *>(this->readVar(varName));
      mxArray *arr = d->getArray();
      //Check type
        if(getExtInt()->mxGetClassID(arr) != mxCELL_CLASS) {
           stringstream ss;
           ss << "Expected mxCELL, got " << getExtInt()->mxGetClassName(arr);
           throw MathInterfacelibException(ss.str());
        }
      //Extract elements of cell
       size_t nDim = getExtInt()->mxGetNumberOfDimensions(arr);
       const mwSize *dimSz = getExtInt()->mxGetDimensions(arr);
       size_t currPos[nDim];
       std::function<void(mxArray *, size_t *)> funcRecHandleElem = 
         [this, &fn, nDim](mxArray *arr, size_t *currPos) {
            assert((arr != NULL) && "Invalid cell element");
            MATLAB_Data *d = new MATLAB_Data(this, arr);
            bool res = fn(nDim, currPos, this->decodeType(arr), d);
            /*switch(this->getExtInt()->mxGetClassID(arr)) {
             case mxUNKNOWN_CLASS:     break;
             case mxCELL_CLASS:    res = fn(nDim, currPos, TYPE_CELL, d);       break;
             case mxSTRUCT_CLASS:  res = fn(nDim, currPos, TYPE_STRUCT, d);     break;
             case mxCHAR_CLASS:    res = fn(nDim, currPos, TYPE_STRING, d);     break;
             case mxDOUBLE_CLASS:  res = fn(nDim, currPos, TYPE_DOUBLE, d);     break;
             case mxSINGLE_CLASS:  res = fn(nDim, currPos, TYPE_DOUBLE, d);     break;
             default: 
                {
                   stringstream ss;
                   ss << "Unhandled class type " << this->getExtInt()->mxGetClassName(arr);
                   throw MathInterfacelibException(ss.str());
                } break;
            }*/
            if(!res)
               throw MathInterfacelibException("Unable to complete cell handling function");
         };
       std::function<void(size_t, size_t *)> funcRecUnPack =
          [this, arr, &fn, nDim, dimSz, &funcRecUnPack, &funcRecHandleElem](size_t currDim, size_t *currPos) {
             assert((arr != NULL) && "Invalid cell array");
             for(size_t c = 0; c < dimSz[currDim]; c++) {
                 currPos[currDim] = c;
                 if(currDim == 0) {
                    mxArray *a = this->getExtInt()->mxGetCell(arr, this->getExtInt()->mxCalcSingleSubscript(arr, nDim, currPos));
                    funcRecHandleElem(a, currPos);
                 } else
                     funcRecUnPack(currDim-1, currPos);
             }
          };
       funcRecUnPack(nDim-1, &currPos[0]);

      delete d;
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////

   //DataInterface<MATLAB_State>::DataInterface(MATLAB_State &s_) : s(s_) {}
   //DataInterface<MATLAB_State>::~DataInterface() {}

   //Iterator based variable creation
     //Double
         void MATLAB_State::writeVar(std::string strVarName, size_t m, size_t n, FWD_IT_T<double> begin, FWD_IT_T<double> end) {
               mxArray *arr = this->getExtInt()->mxCreateNumericMatrix(m, n, mxDOUBLE_CLASS, mxREAL);
               double *ptr = this->getExtInt()->mxGetPr(arr);
               for(FWD_IT_T<double> it = begin; it != end; ++it, ptr += 1)
                   *ptr = *it;
               this->getExtInt()->engPutVariable(this->getEng(), strVarName.c_str(), arr);
               this->getExtInt()->mxDestroyArray(arr);
         }
         void MATLAB_State::writeVar(std::string strVarName, size_t m, size_t n, CONST_FWD_IT_T<double> begin, CONST_FWD_IT_T<double> end) {
               mxArray *arr = this->getExtInt()->mxCreateNumericMatrix(m, n, mxDOUBLE_CLASS, mxREAL);
               double *ptr = this->getExtInt()->mxGetPr(arr);
               for(CONST_FWD_IT_T<double> it = begin; it != end; ++it, ptr += 1)
                   *ptr = *it;
               this->getExtInt()->engPutVariable(this->getEng(), strVarName.c_str(), arr);
               this->getExtInt()->mxDestroyArray(arr);
         }
      //Size_t
         void MATLAB_State::writeVar(std::string strVarName, size_t m, size_t n, FWD_IT_T<size_t> begin, FWD_IT_T<size_t> end) {
               mxArray *arr = NULL;
               switch(sizeof(size_t)) {
                case 4:    arr = this->getExtInt()->mxCreateNumericMatrix(m, n, mxUINT32_CLASS, mxREAL);     break;
                case 8:    arr = this->getExtInt()->mxCreateNumericMatrix(m, n, mxUINT64_CLASS, mxREAL);     break;
                default:   throw MathInterfacelibException("Undefined size_t size in writeVar()");
               }

               size_t *ptr = (size_t *) this->getExtInt()->mxGetPr(arr);
               for(FWD_IT_T<size_t> it = begin; it != end; ++it, ptr += 1)
                   *ptr = *it;
               this->getExtInt()->engPutVariable(this->getEng(), strVarName.c_str(), arr);
               this->getExtInt()->mxDestroyArray(arr);
         }
         void MATLAB_State::writeVar(std::string strVarName, size_t m, size_t n, CONST_FWD_IT_T<size_t> begin, CONST_FWD_IT_T<size_t> end) {
               mxArray *arr = NULL;
               switch(sizeof(size_t)) {
                case 4:    arr = this->getExtInt()->mxCreateNumericMatrix(m, n, mxUINT32_CLASS, mxREAL);     break;
                case 8:    arr = this->getExtInt()->mxCreateNumericMatrix(m, n, mxUINT64_CLASS, mxREAL);     break;
                default:   throw MathInterfacelibException("Undefined size_t size in writeVar()");
               }

               size_t *ptr = (size_t *) this->getExtInt()->mxGetPr(arr);
               for(CONST_FWD_IT_T<size_t> it = begin; it != end; ++it, ptr += 1)
                   *ptr = *it;
               this->getExtInt()->engPutVariable(this->getEng(), strVarName.c_str(), arr);
               this->getExtInt()->mxDestroyArray(arr);
         }
   //Write scalar
      void MATLAB_State::writeVar(std::string strVarName, std::string val) {
         mxArray *arr = this->getExtInt()->mxCreateString(val.c_str());
         this->getExtInt()->engPutVariable(this->getEng(), strVarName.c_str(), arr);
         this->getExtInt()->mxDestroyArray(arr);
      }
      void MATLAB_State::writeVar(std::string strVarName, double val) {
         mxArray *arr = this->getExtInt()->mxCreateDoubleScalar(val);
         this->getExtInt()->engPutVariable(this->getEng(), strVarName.c_str(), arr);
         this->getExtInt()->mxDestroyArray(arr);
      }
      void MATLAB_State::writeVar(std::string strVarName, size_t val) {
         mxArray *arr = NULL;
         switch(sizeof(size_t)) {
          case 4:    arr = this->getExtInt()->mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);     break;
          case 8:    arr = this->getExtInt()->mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);     break;
          default:   throw MathInterfacelibException("Undefined size_t size in writeVar()");
         }
         size_t *ptr = (size_t *) this->getExtInt()->mxGetPr(arr);
         *ptr = val;

         this->getExtInt()->engPutVariable(this->getEng(), strVarName.c_str(), arr);
         this->getExtInt()->mxDestroyArray(arr);
      }
   //Write array
      /*template<> void matlabWriteVar<unsigned char>(State *s, std::string strVarName, size_t m, size_t n, const unsigned char *mat) {
         if(this->getEng() == NULL)
            throw MathInterfacelibException("MATLAB engine has not been initialized");

         mxArray *arr = getExtInt()->mxCreateNumericMatrix(m, n, mxCHAR_CLASS, mxREAL);
         memcpy((void *)getExtInt()->mxGetPr(arr), (void *)mat, m*n*sizeof(unsigned int));
         getExtInt()->engPutVariable(this->getEng(), strVarName.c_str(), arr);
         getExtInt()->mxDestroyArray(arr);
      }
      template<> void matlabWriteVar<unsigned int>(State *s, std::string strVarName, size_t m, size_t n, const unsigned int *mat) {
         if(this->getEng() == NULL)
            throw MathInterfacelibException("MATLAB engine has not been initialized");

         mxArray *arr = NULL;
         switch(sizeof(unsigned int)) {
          case 2:    arr = getExtInt()->mxCreateNumericMatrix(m, n, mxUINT16_CLASS, mxREAL);     break;
          case 4:    arr = getExtInt()->mxCreateNumericMatrix(m, n, mxUINT32_CLASS, mxREAL);     break;
          default:   throw MathInterfacelibException("Undefined unsigned integer size in writeVar()");
         }
         memcpy((void *)getExtInt()->mxGetPr(arr), (void *)mat, m*n*sizeof(unsigned int));
         getExtInt()->engPutVariable(this->getEng(), strVarName.c_str(), arr);
         getExtInt()->mxDestroyArray(arr);
      }*/
      void MATLAB_State::writeVar(std::string strVarName, size_t m, size_t n, const size_t *mat) {
         mxArray *arr = NULL;
         switch(sizeof(size_t)) {
          case 4:    arr = this->getExtInt()->mxCreateNumericMatrix(m, n, mxUINT32_CLASS, mxREAL);     break;
          case 8:    arr = this->getExtInt()->mxCreateNumericMatrix(m, n, mxUINT64_CLASS, mxREAL);     break;
          default:   throw MathInterfacelibException("Undefined size_t size in writeVar()");
         }
         memcpy((void *) this->getExtInt()->mxGetPr(arr), (void *)mat, m*n*sizeof(size_t));
         this->getExtInt()->engPutVariable(this->getEng(), strVarName.c_str(), arr);
         this->getExtInt()->mxDestroyArray(arr);
      }
      void MATLAB_State::writeVar(std::string strVarName, size_t m, size_t n, const double *mat) {
         mxArray *arr = this->getExtInt()->mxCreateNumericMatrix(m, n, mxDOUBLE_CLASS, mxREAL);
         memcpy((void *) this->getExtInt()->mxGetPr(arr), (void *)mat, m*n*sizeof(double));
         this->getExtInt()->engPutVariable(this->getEng(), strVarName.c_str(), arr);
         this->getExtInt()->mxDestroyArray(arr);
      }

   //Read scalar
      string MATLAB_State::readStrVar(std::string strVarName) {
         mxArray *arr = NULL;
         if((arr = this->getExtInt()->engGetVariable(this->getEng(), strVarName.c_str())) == NULL)
             throw MathInterfacelibException("No variable with this name has been defined");

         std::string str = this->decodeValue<std::string>(arr);
         this->getExtInt()->mxDestroyArray(arr);
         return str;
      }
      double MATLAB_State::readDoubleVar(std::string strVarName) {
         mxArray *arr = NULL;
         if((arr = this->getExtInt()->engGetVariable(this->getEng(), strVarName.c_str())) == NULL)
             throw MathInterfacelibException("No variable with this name has been defined");

         double val = this->decodeValue<double>(arr);
         this->getExtInt()->mxDestroyArray(arr);
         return val;
      }
      size_t MATLAB_State::readIntVar(std::string strVarName) {
         mxArray *arr = NULL;
         if((arr = this->getExtInt()->engGetVariable(this->getEng(), strVarName.c_str())) == NULL)
             throw MathInterfacelibException("No variable with this name has been defined");
         
         size_t val = this->decodeValue<size_t>(arr);
         this->getExtInt()->mxDestroyArray(arr);
         return val;
      }
   //Read array
      /*template<> unsigned char *readVar<unsigned char>(State *s, std::string strVarName, size_t &m, size_t &n) {
         MATLAB_State *s = dynamic_cast<MATLAB_State *>(s);
         if(s == NULL)
            throw MathInterfacelibException("MATLAB engine has not been initialized or invalid type");

         mxArray *arr = NULL;
         if((arr = engGetVariable(this->getEng(), strVarName.c_str())) == NULL)
             throw MathInterfacelibException("No variable with this name has been defined");
         //Check type
           if(mxGetClassID(arr) != mxCHAR_CLASS) {
              stringstream ss;
              ss << "Expected mxCHAR array, got " << mxGetClassName(arr);
              throw MathInterfacelibException(sthis->str());
           }
         
         unsigned char *mat = new unsigned char[m*n];
         memcpy((void *)mat, (void *)mxGetPr(arr), m*n*sizeof(unsigned char));
         m = mxGetM(arr);
         n = mxGetN(arr);   

         mxDestroyArray(arr);
         return mat;
      }
      template<> unsigned int *readVar<unsigned int>(State *s, std::string strVarName, size_t &m, size_t &n) {
         MATLAB_State *s = dynamic_cast<MATLAB_State *>(s);
         if(s == NULL)
            throw MathInterfacelibException("MATLAB engine has not been initialized or invalid type");

         mxArray *arr = NULL;
         if((arr = engGetVariable(this->getEng(), strVarName.c_str())) == NULL)
             throw MathInterfacelibException("No variable with this name has been defined");
         //Check type
           switch(mxGetClassID(arr)) {
            case mxUINT16_CLASS:
               if(sizeof(unsigned int) != 2) {
                  stringstream ss;
                  ss << "Expected mxUINT16 array, got " << mxGetClassName(arr);
                  throw MathInterfacelibException(sthis->str());
               }
               break;
            case mxUINT32_CLASS:
               if(sizeof(unsigned int) != 2) {
                  stringstream ss;
                  ss << "Expected mxUINT32 array, got " << mxGetClassName(arr);
                  throw MathInterfacelibException(sthis->str());
               }
               break;
           default:
               throw MathInterfacelibException("Invalid array type");
         }
         
         unsigned int *mat = new unsigned int[m*n];
         memcpy((void *)mat, (void *)mxGetPr(arr), m*n*sizeof(unsigned int));
         m = mxGetM(arr);
         n = mxGetN(arr);   

         mxDestroyArray(arr);
         return mat;
      }*/
      size_t *MATLAB_State::readIntArr(std::string strVarName, std::vector<size_t> &szDims) {
         mxArray *arr = NULL;
         if((arr = this->getExtInt()->engGetVariable(this->getEng(), strVarName.c_str())) == NULL)
             throw MathInterfacelibException("No variable with this name has been defined");
         size_t *val = this->decodeArray<size_t>(arr, szDims);
         this->getExtInt()->mxDestroyArray(arr);
         return val;
      }
      double *MATLAB_State::readDoubleArr(std::string strVarName, std::vector<size_t> &szDims) {
         mxArray *arr = NULL;
         if((arr = this->getExtInt()->engGetVariable(this->getEng(), strVarName.c_str())) == NULL)
             throw MathInterfacelibException("No variable with this name has been defined");
         double *val = this->decodeArray<double>(arr, szDims);
         this->getExtInt()->mxDestroyArray(arr);
         return val;
      }
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////



   //Command parser
     class Command_Parser {
      private:
         std::string funcName;
         std::vector<Value *> inputValues;
         size_t expectedOutputs;
         std::stack<Value_Array *> arrayStack;
         Value_Matrix *matrix;
      public:
         //(De/Con)structor(s)
           Command_Parser() {
            this->matrix = NULL;
           }
           ~Command_Parser() {
               if(this->matrix != NULL)
                  delete this->matrix;
               for( ; !arrayStack.empty(); arrayStack.pop())
                     delete arrayStack.top();
               for(std::vector<Value *>::iterator it = inputValues.begin(); it != inputValues.end(); ++it)
                   delete *it;                      
           }
         //Functions
           std::string getFuncName() const {return funcName;}
           const std::vector<Value *> &getInputValues() const {return inputValues;}          

           void handleFuncName(std::vector<char> funcName_) {
               this->funcName = std::string(funcName_.begin(), funcName_.end());
           }
           void handleExpectedOutputs(size_t expectedOutputs_) {
                this->expectedOutputs = expectedOutputs_;
           }

           void addMatrix() {
//cout << "Added matrix..." << endl;
               assert(matrix == NULL);
               matrix = new Value_Matrix();
           }
           void addMatrixRow() {
//cout << "Added matrix row..." << endl;
               assert(matrix != NULL);
               matrix->addRow();
           }
           void commitMatrix() {    
//cout << "Committed matrix..." << endl; 
               assert(matrix != NULL);
               if(arrayStack.empty())
                  this->inputValues.push_back(matrix);
               else
                  this->arrayStack.top()->addElement(matrix);
               matrix = NULL;
           }
       
           
           void addArray() {
//cout << "Added array..." << endl;
               arrayStack.push(new Value_Array());
           }
           void commitArray() {
//cout << "Committed array..." << endl;
               assert(!arrayStack.empty());
               Value_Array *vT = arrayStack.top();  arrayStack.pop();
               if(arrayStack.size() > 0) {
                  Value_Array *arr = arrayStack.top();
                  arr->addElement(vT);
               } else
                  this->inputValues.push_back(vT);
           }


           void handleArgString(std::vector<char> str_) {
                Value_Scalar<std::string> *vT = new Value_Scalar<std::string>(std::string(str_.begin(), str_.end()));
                if(matrix == NULL)
                   this->inputValues.push_back(vT);
                else
                   matrix->addElementToLastRow(vT);
           }
           void handleArgInteger(size_t i_) {
                Value_Scalar<size_t> *vT = new Value_Scalar<size_t>(i_);
                if(matrix == NULL)
                   this->inputValues.push_back(vT);
                else
                   matrix->addElementToLastRow(vT);
           }
           void handleArgDouble(double d_) {
                Value_Scalar<double> *vT = new Value_Scalar<double>(d_);
                if(matrix == NULL)
                   this->inputValues.push_back(vT);
                else
                   matrix->addElementToLastRow(vT);
           }


           bool parse(const string line)  {
                using qi::uint_;
                using qi::char_;
                using qi::lexeme;
                using qi::lit;
                using qi::real_parser;
                using qi::strict_real_policies;
                using boost::optional;
                using boost::spirit::qi::phrase_parse;
                using boost::spirit::ascii::space;

                real_parser<double, strict_real_policies<double>> strict_double;
                qi::rule<std::string::const_iterator, std::string()> str_ = lexeme["'" >> (+(char_ - '\''))[boost::bind(&Command_Parser::handleArgString, this, _1)] >> "'"],
                                                                     strRow_ = (str_ >> -("," >> strRow_)),
                                                                     strMat_ = (strRow_ >> -(lit(";")[boost::bind(&Command_Parser::addMatrixRow, this)] >> strMat_)),
                                                                     float_ = strict_double[boost::bind(&Command_Parser::handleArgDouble, this, _1)],
                                                                     floatRow_ = (float_ >> -("," >> floatRow_)),
                                                                     floatMat_ = (floatRow_ >> -(lit(";")[boost::bind(&Command_Parser::addMatrixRow, this)] >> floatMat_)),
                                                                     int_ = uint_[boost::bind(&Command_Parser::handleArgInteger, this, _1)],
                                                                     intRow_ = (int_ >> -("," >> intRow_)),
                                                                     intMat_ = (intRow_ >> -(lit(";")[boost::bind(&Command_Parser::addMatrixRow, this)] >> intMat_)),
                                                                     matrix_ = (lit('[')[boost::bind(&Command_Parser::addMatrix, this)] >> 
                                                                                    -(floatMat_ | strMat_ | intMat_) >>
                                                                                lit("]")[boost::bind(&Command_Parser::commitMatrix, this)]),
                                                                     array_ = (lit('(')[boost::bind(&Command_Parser::addArray, this)] >>
                                                                                 +((matrix_ | array_ | str_ | float_ | int_) >> ";") >>
                                                                               lit(')')[boost::bind(&Command_Parser::commitArray, this)]);

                return phrase_parse(line.begin(), line.end(),
                         (
                            (+(char_ - '('))[boost::bind(&Command_Parser::handleFuncName, this, _1)] >> 
                            "(" >> 
                               *((matrix_ | array_ | str_ | float_ | int_) >> ";") >> 
                            ")" >> "->" >> uint_[boost::bind(&Command_Parser::handleExpectedOutputs, this, _1)]
                         ), space);
           }
     };

   void MATLAB_State::executeWithoutListen(std::string cmd) {
      #ifdef MATHINTERFACELIB_MATLAB_DEBUG
         cout << "Executing '" << cmd << "': " << endl;
      #endif
      getExtInt()->engEvalString(getEng(), cmd.c_str());

      std::stringstream ss;
      ss << "try\n\t" << cmd << "\n\t" << this->errVariable << " = 0;\ncatch ME\n\t" << this->errVariable << " = 1;\nend"; // << "\n" << this->errVariable;

      getExtInt()->engEvalString(getEng(), ss.str().c_str());

      size_t err = (size_t) this->readDoubleVar(this->errVariable);
      if(err != 0)
         throw MathInterfacelibException("Unable to execute command");
   }
   void MATLAB_State::execute(std::string cmd) {
      #ifdef MATHINTERFACELIB_MATLAB_DEBUG
         char buffer[128*1024]; //65536];
         getExtInt()->engOutputBuffer(getEng(), &buffer[0], 65536);
         cout << "Output from '" << cmd << "': " << endl;
      #endif
      shared_memory_object *shm = getSHM();
      mapped_region region(*shm, read_write);
      SharedRegion *data = static_cast<SharedRegion *>(region.get_address());
      bool workerFinished = false;
      //std::promise<int> retValue;

      std::stringstream ss;
      ss << "try\n\t" << cmd << "\n\t" << this->errVariable << " = 0;\ncatch ME\n\t" << this->errVariable << " = 1;\nend"; // << "\n" << this->errVariable;
      std::string fullCmd = ss.str();

      std::thread tWorker([this, fullCmd](){ //std::promise<int> &&retValue) {
            getExtInt()->engEvalString(getEng(), fullCmd.c_str());
         }); //, std::move(retValue));
      std::thread tHandleFunction([this, data, &workerFinished]() {
            std::function<void(std::string)> funcSendString = [data](std::string str) mutable {
               for(size_t i = 0, startIndex = data->currRead; str[i] != '\0'; ) {
                   data->scratchPad[data->currRead] = str[i++];
                   data->currRead = (data->currRead + 1) % PROCESS_SHARED_BUFF_SIZE;
                   if(data->currRead == startIndex)
                      throw MathInterfacelibException("Buffer overflow!");
               }
               data->reader.post();
            };
            std::function<bool(Function::Argument *, const Value *)> funcTypeCheck =
               [&funcTypeCheck](Function::Argument *arg, const Value *val) {
                  return true; //TODO!
               };
            for(; !workerFinished; ) {
                data->writer.wait();
                if(data->currRead != data->currWrite) {
                   //Parse values
                     Command_Parser g;
                     std::stringstream ss;
                     for(; data->currRead != data->currWrite; data->currRead = (data->currRead + 1) % PROCESS_SHARED_BUFF_SIZE)
                         ss << data->scratchPad[data->currRead];
                     #ifdef MATHINTERFACELIB_MATLAB_DEBUG
                        cout << "Got '" << ss.str() << "' from MATLAB " << endl;
                     #endif
                     assert(g.parse(ss.str()) && "Invalid format!");
                  //Dispatch
                    std::map<std::string, Function *>::iterator it = this->funcMap.find(g.getFuncName());
                    if(it == this->funcMap.end())
                       funcSendString("ERROR('Unknown function name')");
                    else {
                        //Type check
                          bool flagTypeCheckOk = true;
                          std::vector<Function::Argument *>::iterator itArgs;
                          std::vector<Value *>::const_iterator itVals;
                          if(it->second->vecInputs.size() != g.getInputValues().size()) {
                             funcSendString("ERROR('Invalid number of inputs')");
                             flagTypeCheckOk = false;
                          }
                          for(itArgs = it->second->vecInputs.begin(), itVals = g.getInputValues().begin(); 
                               flagTypeCheckOk && (itArgs != it->second->vecInputs.end()); 
                               ++itArgs, ++itVals)
                                 if(!funcTypeCheck(*itArgs, *itVals)) {
                                    funcSendString("ERROR('Typechecking of input failed')");
                                    flagTypeCheckOk = false;
                                 }
                        //Prepare outputs and execute function
                          if(flagTypeCheckOk) {
                             std::vector<Value *> valOutputs;
                             /*for(std::vector<Function::Argument *>::iterator itArgs = it->second->vecOutputs.begin(); 
                                 itArgs != it->second->vecOutputs.end(); ++itArgs)
                                   valOutputs.push_back( createValTree(*itArgs) );*/

                             if(!it->second->funcHandler(g.getInputValues(), valOutputs))
                                funcSendString("ERROR('Function unable to execute')");
                             else {
                                std::stringstream ss;
                                ss << "(";
                                for(std::vector<Value *>::iterator it = valOutputs.begin(); it != valOutputs.end(); ++it)
                                    ss << (*it)->toString() << ";";
                                ss << ")";
                                funcSendString(ss.str());
                             }
                             for(std::vector<Value *>::iterator it = valOutputs.begin(); it != valOutputs.end(); ++it)
                                 delete *it;
                          }
                    }
                }
            }
         });
      tWorker.join();
      workerFinished = true;
      if(!data->writer.try_wait())
         data->writer.post();
      tHandleFunction.join();

      size_t err = (size_t) this->readDoubleVar(this->errVariable);
      if(err != 0)
         throw MathInterfacelibException("Unable to execute command");

      #ifdef MATHINTERFACELIB_MATLAB_DEBUG
         for(size_t i = 0; (buffer[i] != '\0') && (i < 128*1024); i++)
             cout << buffer[i];
         cout << endl;
      #endif
   }

   bool MATLAB_State::opAdd(const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar0, const MATLAB_State::NAME srcVar1) {
     this->executeWithoutListen(dstVar + " = " + srcVar0 + "+" + srcVar1);
     return true;
   }
   bool MATLAB_State::opSub(const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar0, const MATLAB_State::NAME srcVar1) {
     this->executeWithoutListen(dstVar + " = " + srcVar0 + "-" + srcVar1);
     return true;
   }
   bool MATLAB_State::opMul(const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar0, const MATLAB_State::NAME srcVar1) {
     this->executeWithoutListen(dstVar + " = " + srcVar0 + "*" + srcVar1);
     return true;
   }
   bool MATLAB_State::opDiv(const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar0, const MATLAB_State::NAME srcVar1) {
     this->executeWithoutListen(dstVar + " = " + srcVar0 + "/" + srcVar1);
     return true;
   }
   bool MATLAB_State::opNeg(const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar) {
     this->executeWithoutListen(dstVar + " = -" + srcVar);
     return true;
   }
   bool MATLAB_State::opConj(const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar) {
     this->executeWithoutListen(dstVar + " = " + srcVar + "'");
     return true;
   }
   bool MATLAB_State::opTrans(const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar) {
     this->executeWithoutListen(dstVar + " = " + srcVar + ".'");
     return true;
   }
   bool MATLAB_State::opInverse(const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar) {
     this->executeWithoutListen(dstVar + " = " + srcVar + "^(-1)");
     return true;
   }

   bool MATLAB_State::opScale(const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar, const double c) {
     std::stringstream ss;
     ss << dstVar << " = " << c << "*" << srcVar;
     this->executeWithoutListen(ss.str());
     return true;
   }
   bool MATLAB_State::opTranslate(const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar, const double c) {
     std::stringstream ss;
     ss << dstVar << " = " << c << "+" << srcVar;
     this->executeWithoutListen(ss.str());
     return true;
   }


   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   template<> void toMatrix<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME dstName, const MATLAB_State::NAME srcName) {
     s.executeWithoutListen(dstName + " = " + srcName + ";");
   }
   template<> void toDiagonalMatrix<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME dstName, const MATLAB_State::NAME srcName) {
     s.executeWithoutListen(dstName + " = diag(" + srcName + ");");
   }
   template<> void clone<MATLAB_State>(MATLAB_State &s, MATLAB_State::NAME &dstName, const MATLAB_State::NAME srcName) {
     s.executeWithoutListen(dstName + " = " + srcName + ";");
   }
   template<> void appendRows<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME x, const MATLAB_State::NAME app) {
     s.executeWithoutListen(x + " = [" + x + "; " + app + "];");
   }
   template<> void appendColumns<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME x, const MATLAB_State::NAME app) {
     s.executeWithoutListen(x + " = [" + x + " " + app + "];");
   }
   template<> void appendRow<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME x, const MATLAB_State::NAME app) {
     appendRows<MATLAB_State>(s, x, app);
   }
   template<> void appendColumn<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME x, const MATLAB_State::NAME app) {
     appendColumns<MATLAB_State>(s, x, app);
   }
   template<> size_t numDims<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME x) {
     std::stringstream ss;
     ss << s.getTempVar() << " = ndims(" << x << ");";
     s.executeWithoutListen(ss.str());

     return (size_t) s.readDoubleVar(s.getTempVar());
   }
   template<> size_t size<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME x, size_t dim) {
     std::stringstream ss;
     ss << s.getTempVar() << " = size(" << x << ", " << (dim+1) << ");";
     s.executeWithoutListen(ss.str());

     return (size_t) s.readDoubleVar(s.getTempVar());
   }
   template<> bool create<MATLAB_State, double>(MATLAB_State &s, const MATLAB_State::NAME dst, size_t r, size_t c, double constantVal) {
     if(dst.compare(MATHINTERFACELIB_MATLAB_TEMP_VAR) == 0)
        throw MathInterfacelibException("Unable to use variable, reserved for internal MathInterfaceLib operations (TEMPVAR)");
     if(dst.compare(MATHINTERFACELIB_MATLAB_ERR_VAR) == 0)
        throw MathInterfacelibException("Unable to use variable, reserved for internal MathInterfaceLib operations (ERRVAR)");
     std::stringstream ss;
     ss << dst << "= " << constantVal << ".*ones(" << r << ", " << c << ");";
     s.executeWithoutListen(ss.str());
     return true;
   } 
   template<> bool create<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME dst, size_t r, size_t c, SPECIAL_MATRICES init) {
     if(dst.compare(MATHINTERFACELIB_MATLAB_TEMP_VAR) == 0)
        throw MathInterfacelibException("Unable to use variable, reserved for internal MathInterfaceLib operations (TEMPVAR)");
     if(dst.compare(MATHINTERFACELIB_MATLAB_ERR_VAR) == 0)
        throw MathInterfacelibException("Unable to use variable, reserved for internal MathInterfaceLib operations (ERRVAR)");
     switch(init) {
      case SPECIAL_MATRICES::MATRIX_IDENT:
        {
         assert(r == c);

         std::stringstream ss;
         ss << dst << "= eye(" << r << ");";
         s.executeWithoutListen(ss.str());
        } break;
      case SPECIAL_MATRICES::MATRIX_HANKEL:
       assert(false && "TODO");
     }
     return true;
   } 
   template<> bool create<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME dst, size_t r, size_t c, INITIALIZATIONs init) {
     if(dst.compare(MATHINTERFACELIB_MATLAB_TEMP_VAR) == 0)
        throw MathInterfacelibException("Unable to use variable, reserved for internal MathInterfaceLib operations (TEMPVAR)");
     if(dst.compare(MATHINTERFACELIB_MATLAB_ERR_VAR) == 0)
        throw MathInterfacelibException("Unable to use variable, reserved for internal MathInterfaceLib operations (ERRVAR)");
     switch(init) {
      case INITIALIZATIONs::INIT_RANDOM:
        {
         std::stringstream ss;
         ss << dst << "= rand(" << r << ", " << c << ");";
         s.executeWithoutListen(ss.str());
        } break;
      case INITIALIZATIONs::INIT_ZERO:
        {
         std::stringstream ss;
         ss << dst << "= zeros(" << r << ", " << c << ");";
         s.executeWithoutListen(ss.str());
        } break;
      case INITIALIZATIONs::INIT_ONE:
        {
         std::stringstream ss;
         ss << dst << "= ones(" << r << ", " << c << ");";
         s.executeWithoutListen(ss.str());
        } break;
      case INITIALIZATIONs::INIT_NONE:
        {
         std::stringstream ss;
         ss << dst << "= zeros(" << r << ", " << c << ");"; //TODO! Is there a non-specific init?
         s.executeWithoutListen(ss.str());
        } break;
     }
     return true;
   }
   template<> bool create<MATLAB_State, double>(MATLAB_State &s, const MATLAB_State::NAME dst, size_t sz, bool isRowVector, double constantVal) {
     if(dst.compare(MATHINTERFACELIB_MATLAB_TEMP_VAR) == 0)
        throw MathInterfacelibException("Unable to use variable, reserved for internal MathInterfaceLib operations (TEMPVAR)");
     if(dst.compare(MATHINTERFACELIB_MATLAB_ERR_VAR) == 0)
        throw MathInterfacelibException("Unable to use variable, reserved for internal MathInterfaceLib operations (ERRVAR)");
     std::stringstream ss;
     if(isRowVector)
        ss << dst << "= " << constantVal << ".*ones(1, " << sz << ");";
     else
         ss << dst << "= " << constantVal << ".*ones(" << sz << ", 1);";
     s.executeWithoutListen(ss.str());
     return true;
   } 
   template<> bool create<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME dst, size_t sz, bool isRowVector, INITIALIZATIONs init) {
     if(dst.compare(MATHINTERFACELIB_MATLAB_TEMP_VAR) == 0)
        throw MathInterfacelibException("Unable to use variable, reserved for internal MathInterfaceLib operations (TEMPVAR)");
     if(dst.compare(MATHINTERFACELIB_MATLAB_ERR_VAR) == 0)
        throw MathInterfacelibException("Unable to use variable, reserved for internal MathInterfaceLib operations (ERRVAR)");
     switch(init) {
      case INITIALIZATIONs::INIT_RANDOM:
        {
         std::stringstream ss;
         if(isRowVector)
            ss << dst << "= rand(1, " << sz << ");";
         else
             ss << dst << "= rand(" << sz << ", 1);";
         s.executeWithoutListen(ss.str());
        } break;
      case INITIALIZATIONs::INIT_ZERO:
        {
         std::stringstream ss;
         if(isRowVector)
            ss << dst << "= zeros(1, " << sz << ");";
         else
             ss << dst << "= zeros(" << sz << ", 1);";
         s.executeWithoutListen(ss.str());
        } break;
      case INITIALIZATIONs::INIT_ONE:
        {
         std::stringstream ss;
         if(isRowVector)
            ss << dst << "= ones(1, " << sz << ");";
         else
             ss << dst << "= ones(" << sz << ", 1);";
         s.executeWithoutListen(ss.str());
        } break;
      case INITIALIZATIONs::INIT_NONE:
        {
         std::stringstream ss;
         if(isRowVector)
            ss << dst << "= zeros(1, " << sz << ");";
         else
             ss << dst << "= zeros(" << sz << ", 1);";
         s.executeWithoutListen(ss.str());
        } break;
     }
     return true;
   }  



   template<> bool set<MATLAB_State, double>(MATLAB_State &s, const MATLAB_State::NAME dst, const double src1) {
     s.writeVar(dst, src1);
     return true;
   }
   template<> bool set<MATLAB_State, double>(MATLAB_State &s, const MATLAB_State::NAME dst, const double *src1, size_t n) {
     assert(false);
   }
   template<> void set<MATLAB_State, double>(MATLAB_State &s, const MATLAB_State::NAME dst, size_t i, const double y) {
     std::stringstream ss;
     ss << dst << "(" << (i+1) << ") = " << y << ";";
     s.executeWithoutListen(ss.str());
   }
   template<> void set<MATLAB_State, double>(MATLAB_State &s, const MATLAB_State::NAME dst, size_t r, size_t c, const double y) {
     std::stringstream ss;
     ss << dst << "(" << (r+1) << ", " << (c+1) << ") = " << y << ";";
     s.executeWithoutListen(ss.str());
   }
   template<> void set<MATLAB_State, double>(MATLAB_State &s, const MATLAB_State::NAME dst, size_t rb, size_t r_len, size_t cb, size_t c_len, const double y) {
     assert(false);
   }

   template<> bool set<MATLAB_State, size_t>(MATLAB_State &s, const MATLAB_State::NAME dst, const size_t src1) {
     s.writeVar(dst, src1);
     return true;
   }
   template<> bool set<MATLAB_State, size_t>(MATLAB_State &s, const MATLAB_State::NAME dst, const size_t *src1, size_t n) {
     assert(false);
   }
   template<> void set<MATLAB_State, size_t>(MATLAB_State &s, const MATLAB_State::NAME dst, size_t i, const size_t y) {
     std::stringstream ss;
     ss << dst << "(" << (i+1) << ") = " << y << ";";
     s.executeWithoutListen(ss.str());
   }
   template<> void set<MATLAB_State, size_t>(MATLAB_State &s, const MATLAB_State::NAME dst, size_t r, size_t c, const size_t y) {
     std::stringstream ss;
     ss << dst << "(" << (r+1) << ", " << (c+1) << ") = " << y << ";";
     s.executeWithoutListen(ss.str());
   }
   template<> void set<MATLAB_State, size_t>(MATLAB_State &s, const MATLAB_State::NAME dst, size_t rb, size_t r_len, size_t cb, size_t c_len, const size_t y) {
     assert(false);
   }

   template<> bool set<MATLAB_State, std::string>(MATLAB_State &s, const MATLAB_State::NAME dst, const std::string src1) {
     s.writeVar(dst, src1);
     return true;
   }
   template<> bool set<MATLAB_State, std::string>(MATLAB_State &s, const MATLAB_State::NAME dst, const std::string *src1, size_t n) {
     assert(false);
   }



   template<> double toDouble<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME src) {
     assert(false);
   }
   template<> bool row<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME dst, const MATLAB_State::NAME srcVar, size_t r) {
     assert(false);
   }
   template<> bool col<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME dst, const MATLAB_State::NAME srcVar, size_t c) {
     assert(false);
   }
   template<> double get<MATLAB_State, double>(MATLAB_State &s, const MATLAB_State::NAME src, size_t r, size_t c) {
     std::stringstream ss;
     ss << s.getTempVar() << " = " << src << "(" << (r+1) << ", " << (c+1) << ");";
     s.executeWithoutListen(ss.str());
     return s.readDoubleVar(s.getTempVar());
   }
   template<> double get<MATLAB_State, double>(MATLAB_State &s, const MATLAB_State::NAME src, size_t i) {
     std::stringstream ss;
     ss << s.getTempVar() << " = " << src << "(" << (i+1) << ");";
     s.executeWithoutListen(ss.str());
     return s.readDoubleVar(s.getTempVar());
   }
   template<> size_t get<MATLAB_State, size_t>(MATLAB_State &s, const MATLAB_State::NAME src, size_t r, size_t c) {
     std::stringstream ss;
     ss << s.getTempVar() << " = " << src << "(" << (r+1) << ", " << (c+1) << ");";
     s.executeWithoutListen(ss.str());
     return s.readIntVar(s.getTempVar());
   }
   template<> size_t get<MATLAB_State, size_t>(MATLAB_State &s, const MATLAB_State::NAME src, size_t i) {
     std::stringstream ss;
     ss << s.getTempVar() << " = " << src << "(" << (i+1) << ");";
     s.executeWithoutListen(ss.str());
     return s.readIntVar(s.getTempVar());
   }

   template<> bool mode<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME dst, const MATLAB_State::NAME src, size_t i) {
     assert(false);
   } 
   template<> std::string toStringVar<MATLAB_State>(MATLAB_State &s, const MATLAB_State::NAME x) {
     return s.toStringVariable(x);
   }
 
   template<> bool opElem<MATLAB_State, OP_T<OPs::OP_MUL>>(MATLAB_State &s, const MATLAB_State::NAME dst, const MATLAB_State::NAME src0, const MATLAB_State::NAME src1) {assert(false);}
   template<> bool opElem<MATLAB_State, OP_T<OPs::OP_DIV>>(MATLAB_State &s, const MATLAB_State::NAME dst, const MATLAB_State::NAME src0, const MATLAB_State::NAME src1) {assert(false);}
   template<> bool opElem<MATLAB_State, OP_T<OPs::OP_MIN>>(MATLAB_State &s, const MATLAB_State::NAME dst, const MATLAB_State::NAME src0, const MATLAB_State::NAME src1) {assert(false);}
   template<> bool opElem<MATLAB_State, OP_T<OPs::OP_MAX>>(MATLAB_State &s, const MATLAB_State::NAME dst, const MATLAB_State::NAME src0, const MATLAB_State::NAME src1) {assert(false);}
   template<> bool opElem<MATLAB_State, OP_T<OPs::OP_SQRT>>(MATLAB_State &s, const MATLAB_State::NAME dst, const MATLAB_State::NAME src0, const MATLAB_State::NAME src1) {assert(false);}
   template<> bool opElem<MATLAB_State, OP_T<OPs::OP_ABS>>(MATLAB_State &s, const MATLAB_State::NAME dst, const MATLAB_State::NAME src0, const MATLAB_State::NAME src1) {assert(false);}
   template<> bool opConst<MATLAB_State, OP_T<OPs::OP_SCALE>, double>(MATLAB_State &s, const MATLAB_State::NAME dst, const MATLAB_State::NAME src, const double c) {return s.opScale(dst, src, c);}
   template<> bool opConst<MATLAB_State, OP_T<OPs::OP_TRANSLATE>, double>(MATLAB_State &s, const MATLAB_State::NAME dst, const MATLAB_State::NAME src, const double c) {return s.opTranslate(dst, src, c);}
   template<> bool op<MATLAB_State, OP_T<OPs::OP_ADD>>(MATLAB_State &s, const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar0, const MATLAB_State::NAME srcVar1) {return s.opAdd(dstVar, srcVar0, srcVar1);}     
   template<> bool op<MATLAB_State, OP_T<OPs::OP_SUB>>(MATLAB_State &s, const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar0, const MATLAB_State::NAME srcVar1) {return s.opSub(dstVar, srcVar0, srcVar1);}  
   template<> bool op<MATLAB_State, OP_T<OPs::OP_MUL>>(MATLAB_State &s, const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar0, const MATLAB_State::NAME srcVar1) {return s.opMul(dstVar, srcVar0, srcVar1);}
   template<> bool op<MATLAB_State, OP_T<OPs::OP_DIV>>(MATLAB_State &s, const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar0, const MATLAB_State::NAME srcVar1) {return s.opDiv(dstVar, srcVar0, srcVar1);} 
   template<> bool op<MATLAB_State, OP_T<OPs::OP_NEG>>(MATLAB_State &s, const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar) {return s.opNeg(dstVar, srcVar);} 
   template<> bool op<MATLAB_State, OP_T<OPs::OP_TRANS>>(MATLAB_State &s, const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar) {return s.opTrans(dstVar, srcVar);}
   template<> bool op<MATLAB_State, OP_T<OPs::OP_CONJ>>(MATLAB_State &s, const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar) {return s.opConj(dstVar, srcVar);}
   template<> bool op<MATLAB_State, OP_T<OPs::OP_INVERSE>>(MATLAB_State &s, const MATLAB_State::NAME dstVar, const MATLAB_State::NAME srcVar) {return s.opInverse(dstVar, srcVar);}

}

#endif

