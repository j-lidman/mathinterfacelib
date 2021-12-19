#include <iostream>

#include "matwlib.h"
#include "MATLAB_Engine.h"
#include "Octave_Engine.h"
#include "Offline_Engine.h"

using namespace boost::interprocess;

namespace MatWLib {

   size_t uniqueNumCounter = 0;
   size_t getUniqueNumber() {return uniqueNumCounter++;}
   void resetUniqueNumberGenerator() {uniqueNumCounter = 0;}

   VariableSupply::VariableSupply() {}
   VariableSupply::~VariableSupply() {}
   FunctionInterface::FunctionInterface() {}
   FunctionInterface::~FunctionInterface() {}
   SessionInterface::SessionInterface() {}
   SessionInterface::~SessionInterface() {}
   /*RawDataInterface::RawDataInterface() {}
   RawDataInterface::~RawDataInterface() {}
   CommonDataInterface::CommonDataInterface() {}
   CommonDataInterface::~CommonDataInterface() {}*/
   RawExecuteInterface::RawExecuteInterface() {}
   RawExecuteInterface::~RawExecuteInterface() {}

   State::State() {}
   State::~State() {}
   /*std::string State::getUniqueVariableName() {
     std::stringstream ss;
     ss << "v" << getUniqueNumber();
     return ss.str();
   }*/
   std::string State::toString() const {return "State";}
   bool State::good() const {return false;}



   const char *toString(TYPE t) {
      switch(t) {
       case TYPE_UNKNOWN:  return "TYPE_UNKNOWN";
       case TYPE_INT:      return "TYPE_INT";
       case TYPE_DOUBLE:   return "TYPE_DOUBLE";
       case TYPE_STRING:   return "TYPE_STRING";
       case TYPE_CELL:     return "TYPE_CELL";
       case TYPE_STRUCT:   return "TYPE_STRUCT";
       default:
         assert(false);
      }
   }

   Value::Value() {}
   Value::~Value() {}

   Value_Matrix::Value_Matrix()  {
      addRow();
   }
   Value_Matrix::~Value_Matrix() {
      for(std::vector<std::vector<Value *> *>::iterator it1 = mat.begin(); it1 != mat.end(); ++it1) {
          for(std::vector<Value *>::iterator it2 = (*it1)->begin(); it2 != (*it1)->end(); ++it2)
              delete *it2;
          delete *it1;
      }
   }
   void Value_Matrix::addRow() {
      this->mat.push_back( new std::vector<Value *>() );
   }
   void Value_Matrix::addElementToLastRow(Value *vt) {
      if(vt == NULL)
         throw MatWlibException("Invalid element");
      if(mat.size() == 0)
         throw MatWlibException("Matrix has no rows");
      this->mat[this->mat.size()-1]->push_back(vt);
   }
   std::string Value_Matrix::toString() const {
      std::stringstream ss;
      ss << "[";
      for(size_t i = 0; i < mat.size(); i++) {
          ss << (i == 0 ? "" : ";");
          for(size_t j = 0; j < mat[i]->size(); j++)
              ss << (j == 0 ? "" : ",") << mat[i]->at(j)->toString();
      }
      ss << "]";
      return ss.str();               
   }

   Value_Array::Value_Array() : arrSize(0), arr() {}
   Value_Array::~Value_Array() {
      for(std::vector<Value *>::iterator it = arr.begin(); it != arr.end(); ++it)
          delete *it;
   }
   void Value_Array::addElement(Value *vt) {
      if(vt == NULL)
         throw MatWlibException("Invalid element");
      this->arrSize++;
      this->arr.push_back(vt);
   }
   std::string Value_Array::toString() const {
      std::stringstream ss;
      ss << "(";
      for(size_t i = 0; i < arrSize; i++)
          ss << arr[i]->toString() << ";";
      ss << ")";
      return ss.str();               
   }

   Value_Struct::Value_Struct() {}
   Value_Struct::Value_Struct(std::vector<Value *> vecChildren_) : vecChildren(vecChildren_) {}
   Value_Struct::~Value_Struct() {
      for(std::vector<Value *>::iterator it = vecChildren.begin(); it != vecChildren.end(); ++it)
          delete *it;
   }
   std::string Value_Struct::toString() const {
      std::stringstream ss;
      ss << "{";
      for(std::vector<Value *>::const_iterator it = vecChildren.begin(); it != vecChildren.end(); ++it)
          ss << (it == vecChildren.begin() ? "" : ",") << (*it)->toString();
      ss << "}";
      return ss.str();
   }


   template<> std::string Value_Scalar<std::string>::toString() const {
      std::stringstream ss;
      ss << "'" << val << "'";
      return ss.str();
   }


   ENGINE_TYPE getStateType(State *s) {
      if(!s->good())
         throw MatWlibException("Engine in erronous state!");
      #if !defined(MATWLIB_PREFER_STATIC_ERRORS) || (defined(MATWLIB_PREFER_STATIC_ERRORS) && defined(HAS_MATLAB))
         if(dynamic_cast<MATLAB_State *>(s) != NULL)
            return ENGINE_MATLAB;
      #endif
      #if defined(HAS_OCTAVE)
         if(dynamic_cast<Octave_State *>(s) != NULL)
            return ENGINE_OCTAVE_STATE;
      #endif
      else if(dynamic_cast<Offline_State *>(s) != NULL)
         return ENGINE_OFFLINE;
      else
          throw MatWlibException("Unknown engine type");
   }

   Value *createValTree(Function::Argument *arg) {
      Function::Argument_Array *argArr = dynamic_cast<Function::Argument_Array *>(arg);
      Function::Argument_Struct *argStruct = dynamic_cast<Function::Argument_Struct *>(arg);
      Function::Argument_Element *argElem = dynamic_cast<Function::Argument_Element *>(arg);
      if(argArr != NULL)
         return new Value_Array();
      else if(argStruct != NULL) {
         assert(false);
      } else if(argElem != NULL) {
         switch(argElem->t) {
          case TYPE_UNKNOWN:   throw MatWlibException("Can't create element of type unknown");
          case TYPE_INT:      return new Value_Scalar<size_t>(0);
          case TYPE_DOUBLE:   return new Value_Scalar<double>(0.0);
          case TYPE_STRING:   return new Value_Scalar<std::string>("");
          default:            throw MatWlibException("Unhandled cast in createValTree::element");
         }
      } else
         throw MatWlibException("Unhandled cast in createValTree()");
   }



   State *init_engine(ENGINE_TYPE e, std::string path, bool enableSharedFunc) {
      //Create application specific engine
        switch(e) {
         #if !defined(MATWLIB_PREFER_STATIC_ERRORS) || (defined(MATWLIB_PREFER_STATIC_ERRORS) && defined(HAS_MATLAB))
         case ENGINE_MATLAB:
           {
               //Initialize shared memory
                 shared_memory_object *shm = NULL;
                 if(enableSharedFunc) {
                    boost::interprocess::shared_memory_object::remove(MATWLIB_SHARED_MEM_NAME);
                    shm = new shared_memory_object(open_or_create, MATWLIB_SHARED_MEM_NAME, read_write);
                    shm->truncate(sizeof(SharedRegion));

                    mapped_region region(*shm,read_write);
                    void *addr = region.get_address();
                    SharedRegion *data = new (addr) SharedRegion;
                    if(!data->regionInUse.try_lock())
                       throw MatWlibException("Matwlib library already open");
                 }
               return init_matlab_engine(path, shm);
           } break;
         #endif
         #if defined(HAS_OCTAVE)
         case ENGINE_OCTAVE_STATE:
            return init_octave_state_engine(OCTAVE_USE_INTERPRETER_BY_DEFAULT);
         case ENGINE_OCTAVE_STATE_INTERPRETER:
            return init_octave_state_engine(true);
         case ENGINE_OCTAVE_STATELESS:
            assert(false);
         #endif
         #if !defined(MATWLIB_PREFER_STATIC_ERRORS) || (defined(MATWLIB_PREFER_STATIC_ERRORS) && defined(HAS_EIGEN))
         case ENGINE_EIGEN:
            assert(false);
         #endif
         case ENGINE_OFFLINE:   return init_offline_engine(path);
         default:
            throw MatWlibException("Unable to initialize engine: unknown type");
        }
   }
   void finit_engine(State *s) {
      #if !defined(MATWLIB_PREFER_STATIC_ERRORS) || (defined(MATWLIB_PREFER_STATIC_ERRORS) && defined(HAS_MATLAB))
         MATLAB_State *sMatlab = dynamic_cast<MATLAB_State *>(s);
         if(sMatlab != NULL) {
            finit_matlab_engine(sMatlab);
            return;
         }
      #endif
      Offline_State *sOffline = dynamic_cast<Offline_State *>(s);
      if(sOffline != NULL) {
         finit_offline_engine(sOffline);
         return;
      }

      throw MatWlibException("Engine has not been initialized or invalid type");
   }

   //Function handling
     Value *getArg(Value *in, size_t row, size_t col) {
        Value_Matrix *vtMatrix = dynamic_cast<Value_Matrix *>(in);
        if(vtMatrix != NULL)
           return vtMatrix->mat.at(row)->at(col);
        throw MatWlibException("Value tree is not a matrix");
     }
     Value *getArg(Value *in, size_t index) {
        Value_Struct *vtStruct = dynamic_cast<Value_Struct *>(in);
        Value_Array *vtArr = dynamic_cast<Value_Array *>(in);
        if(vtStruct != NULL)
           return vtStruct->vecChildren.at(index);
        else if(vtArr != NULL)
           return vtArr->arr.at(index);
        else
           throw MatWlibException("Value tree is not a non-terminal");
     }

     template<> size_t getValue<size_t>(Value *in) {
        Value_Scalar<size_t> *t = dynamic_cast<Value_Scalar<size_t> *>(in);
        if(t == NULL)
           throw MatWlibException("Value tree is not a terminal");
        return t->val;
     }
     template<> double getValue<double>(Value *in) {
        Value_Scalar<double> *td = dynamic_cast<Value_Scalar<double> *>(in);
        if(td != NULL)
           return td->val;
        throw MatWlibException("Value tree is not a terminal");
     }
     template<> std::string getValue<std::string>(Value *in) {
        Value_Scalar<std::string> *t = dynamic_cast<Value_Scalar<std::string> *>(in);
        if(t == NULL)
           throw MatWlibException("Value tree is not a terminal");
        return t->val;
     }


     template<> void setValue<size_t>(Value *in, size_t &value) {
        Value_Scalar<size_t> *t = dynamic_cast<Value_Scalar<size_t> *>(in);
        if(t == NULL)
           throw MatWlibException("Value tree is not a terminal");
        t->val = value;
     }
     template<> void setValue<double>(Value *in, double &value) {
        Value_Scalar<double> *t = dynamic_cast<Value_Scalar<double> *>(in);
        if(t == NULL)
           throw MatWlibException("Value tree is not a terminal");
        t->val = value;
     }
     template<> void setValue<std::string>(Value *in, std::string &value) {
        Value_Scalar<std::string> *t = dynamic_cast<Value_Scalar<std::string> *>(in);
        if(t == NULL)
           throw MatWlibException("Value tree is not a terminal");
        t->val = value;
     }
   /*//Writing
     //Iterator-based
       //Double
         template<> void writeVar<std::vector<double>::iterator>(size_t opId, State *s, std::string varName, size_t m, size_t n,
               std::vector<double>::iterator begin, std::vector<double>::iterator end) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       matlabWriteVar(opId, dynamic_cast<MATLAB_State *>(s), varName, m, n, begin, end);   break;
             case ENGINE_OFFLINE:      offlineWriteVar(opId, dynamic_cast<Offline_State *>(s), varName, m, n, begin, end);   break;
             default:                  assert(false);
            }
         }
         template<> void writeVar<std::vector<double>::const_iterator>(size_t opId, State *s, std::string varName, size_t m, size_t n, 
               std::vector<double>::const_iterator begin, std::vector<double>::const_iterator end) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       matlabWriteVar(opId, dynamic_cast<MATLAB_State *>(s), varName, m, n, begin, end);   break;
             case ENGINE_OFFLINE:      offlineWriteVar(opId, dynamic_cast<Offline_State *>(s), varName, m, n, begin, end);   break;
             default:                  assert(false);
            }
         }
       //Size_t
         template<> void writeVar<std::vector<size_t>::iterator>(size_t opId, State *s, std::string varName, size_t m, size_t n,
               std::vector<size_t>::iterator begin, std::vector<size_t>::iterator end) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       matlabWriteVar(opId, dynamic_cast<MATLAB_State *>(s), varName, m, n, begin, end);   break;
             case ENGINE_OFFLINE:      offlineWriteVar(opId, dynamic_cast<Offline_State *>(s), varName, m, n, begin, end);   break;
             default:                  assert(false);
            }
         }
         template<> void writeVar<std::vector<size_t>::const_iterator>(size_t opId, State *s, std::string varName, size_t m, size_t n, 
               std::vector<size_t>::const_iterator begin, std::vector<size_t>::const_iterator end) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       matlabWriteVar(opId, dynamic_cast<MATLAB_State *>(s), varName, m, n, begin, end);   break;
             case ENGINE_OFFLINE:      offlineWriteVar(opId, dynamic_cast<Offline_State *>(s), varName, m, n, begin, end);   break;
             default:                  assert(false);
            }
         }
     //Scalar
       //std::string
         template<> void writeVar<std::string>(size_t opId, State *s, std::string varName, std::string val) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       matlabWriteVar(opId, dynamic_cast<MATLAB_State *>(s), varName, val);   break;
             case ENGINE_OFFLINE:      offlineWriteVar(opId, dynamic_cast<Offline_State *>(s), varName, val);   break;
             default:                  assert(false);
            }
         }
       //Double
         template<> void writeVar<double>(size_t opId, State *s, std::string varName, double val) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       matlabWriteVar(opId, dynamic_cast<MATLAB_State *>(s), varName, val);   break;
             case ENGINE_OFFLINE:      offlineWriteVar(opId, dynamic_cast<Offline_State *>(s), varName, val);   break;
             default:                  assert(false);
            }
         }
       //Size_t
         template<> void writeVar<size_t>(size_t opId, State *s, std::string varName, size_t val) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       matlabWriteVar(opId, dynamic_cast<MATLAB_State *>(s), varName, val);   break;
             case ENGINE_OFFLINE:      offlineWriteVar(opId, dynamic_cast<Offline_State *>(s), varName, val);   break;
             default:                  assert(false);
            }
         }

      //Array
        //Double
          template<> void writeVar<double>(size_t opId, State *s, std::string varName, size_t m, size_t n, const double *mat) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       matlabWriteVar(opId, dynamic_cast<MATLAB_State *>(s), varName, m, n, mat);   break;
             case ENGINE_OFFLINE:      offlineWriteVar(opId, dynamic_cast<Offline_State *>(s), varName, m, n, mat);   break;
             default:                  assert(false);
            }
          }
        //Size_t
          template<> void writeVar<size_t>(size_t opId, State *s, std::string varName, size_t m, size_t n, const size_t *mat) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       matlabWriteVar(opId, dynamic_cast<MATLAB_State *>(s), varName, m, n, mat);   break;
             case ENGINE_OFFLINE:      offlineWriteVar(opId, dynamic_cast<Offline_State *>(s), varName, m, n, mat);   break;
             default:                  assert(false);
            }
          }
     //Cell
       void writeVar(size_t opId, State *s, std::string varName, STRUCT_FUNC_T fn) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       return matlabWriteVar(opId, dynamic_cast<MATLAB_State *>(s), varName, fn);
            }
            assert(false);
       }
   //Reading
     //Scalar
       //std::string
         template<> std::string readVar<std::string>(size_t opId, State *s, std::string varName) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       return matlabReadVar<std::string>(opId, dynamic_cast<MATLAB_State *>(s), varName);
             case ENGINE_OFFLINE:      return offlineReadVar<std::string>(opId, dynamic_cast<Offline_State *>(s), varName);
            }
            assert(false);
         }
       //Double
         template<> double readVar<double>(size_t opId, State *s, std::string varName) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       return matlabReadVar<double>(opId, dynamic_cast<MATLAB_State *>(s), varName);
             case ENGINE_OFFLINE:      return offlineReadVar<double>(opId, dynamic_cast<Offline_State *>(s), varName);
            }
            assert(false);
         }
       //Size_t
         template<> size_t readVar<size_t>(size_t opId, State *s, std::string varName) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       return matlabReadVar<size_t>(opId, dynamic_cast<MATLAB_State *>(s), varName);
             case ENGINE_OFFLINE:      return offlineReadVar<size_t>(opId, dynamic_cast<Offline_State *>(s), varName);
            }
            assert(false);
         }

     //Array
       //Double
         template<> double *readVar<double>(size_t opId, State *s, std::string varName, size_t &m, size_t &n) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       return matlabReadVar<double>(opId, dynamic_cast<MATLAB_State *>(s), varName, m, n);
             case ENGINE_OFFLINE:      return offlineReadVar<double>(opId, dynamic_cast<Offline_State *>(s), varName, m, n);
            }
            assert(false);
         }
       //Size_t
         template<> size_t *readVar<size_t>(size_t opId, State *s, std::string varName, size_t &m, size_t &n) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       return matlabReadVar<size_t>(opId, dynamic_cast<MATLAB_State *>(s), varName, m, n);
             case ENGINE_OFFLINE:      return offlineReadVar<size_t>(opId, dynamic_cast<Offline_State *>(s), varName, m, n);
            }
            assert(false);
         }
     //Cell
       void readVar(size_t opId, State *s, std::string varName, STRUCT_FUNC_T fn) {
            switch(getStateType(s)) {
             case ENGINE_MATLAB:       return matlabReadVar(opId, dynamic_cast<MATLAB_State *>(s), varName, fn);
            }
            assert(false);
       }
    //Executing
     void execute(size_t opId, State *s, std::string cmd) {
         switch(getStateType(s)) {
          case ENGINE_MATLAB:       matlabExecute(opId, dynamic_cast<MATLAB_State *>(s), cmd);     break;
          case ENGINE_OFFLINE:      offlineExecute(opId, dynamic_cast<Offline_State *>(s), cmd);     break;
          default:                  assert(false);
         }
     }*/

};
