#include <iostream>
#include <vector>

#include "matwlib.h"
#include "MATLAB_Engine.h"
#include "Eigen_Engine.h"

using namespace std;
using namespace MatWLib;

//"matlab" must be accessible from $PATH

template<class STATE_T> int basic_statelesstest(STATE_T *s, bool expectExceptionOnRead) {
  return 0;
}

template<class STATE_T> int basic_statetest(STATE_T *s, bool expectExceptionOnRead) {
   DataInterface<STATE_T> sdi(*s);
   RawExecuteInterface *sei = dynamic_cast<RawExecuteInterface *>(s);
   assert(sei != NULL);

   bool readResults;
   //Basic read/write
     sdi.template writeVar<double>("test", 5.0);
     double d = 0.0;
     readResults = true;
     try {
         d = sdi.template readVar<double>("test");
     } catch(MatWlibException e) {
         if(!expectExceptionOnRead)
            throw e;
         else
            readResults = false;
     }
     if(!readResults)
        cerr << "(Read/Write) Skipping comparison, unable to read value" << endl;
     else if(d != 5) {
        cerr << "(Read/Write) Expected '5', got '" << d << "', quitting!" << endl;
        return 1;
     }
   //Execute command
     sei->execute("test = test*2;");
     readResults = true;
     try {
         d = sdi.template readVar<double>("test");
     } catch(MatWlibException e) {
         if(!expectExceptionOnRead)
            throw e;
         else
            readResults = false;
     }
     if(!readResults)
        cerr << "(Execute) Skipping comparison, unable to read value" << endl;
     else if(d != 10) {
        cerr << "(Execute) Expected '10', got '" << d << "', quitting!" << endl;
        return 2;
     }
   //Read/write iterator
     //size_t m, n;
     std::vector<size_t> szDims;
     double *dblArr = new double[10], *dstDblArr = NULL;
     size_t *intArr = new size_t[10], *dstIntArr = NULL;
     vector<double> vecDbl;
     vector<size_t> vecInt;
     for(size_t i = 0; i < 10; i++) {
         vecDbl.push_back((double) i);
         vecInt.push_back(i);
         dblArr[i] = i;
         intArr[i] = i;
     }
     //Double
       sdi.template writeVar<double>("arrDbl", 1, 10, dblArr);
       readResults = true;
       try {
           dstDblArr = sdi.template readVar<double>("arrDbl", szDims);
       } catch(MatWlibException e) {
           if(!expectExceptionOnRead)
              throw e;
           else
              readResults = false;
       }  
       if(!readResults)
          cerr << "(Read/Write double-array 1) Skipping comparison, unable to read matrix" << endl;
       else {
          if((szDims[0] != 1) || (szDims[1] != 10)) {
             cerr << "(Read/Write double-array 1) Expected 1x10 array, got " << szDims[0] << "x" << szDims[1] << ", quitting!" << endl;
             return 3;
          }
          for(size_t i = 0; i < 10; i++)
              if(dblArr[i] != dstDblArr[i]) {
                 cerr << "(Read/Write double-array 1) Expected '" << dblArr[i] << "' at pos. " << i << ", got " << dstDblArr[i] << ", quitting!" << endl;
                 return 4;
              }
       }
       szDims.clear();
       delete[] dstDblArr;

       sdi.writeVar("vecDbl", 1, 10, vecDbl.begin(), vecDbl.end());
       readResults = true;
       try {
           dstDblArr = sdi.template readVar<double>("vecDbl", szDims);
       } catch(MatWlibException e) {
           if(!expectExceptionOnRead)
              throw e;
           else
               readResults = false;
       }    
       if(!readResults)
          cerr << "(Read/Write double-array 2) Skipping comparison, unable to read matrix" << endl;
       else {
         if((szDims[0] != 1) || (szDims[1] != 10)) {
             cerr << "(Read/Write double-array 2) Expected 1x10 array, got " << szDims[0] << "x" << szDims[1] << ", quitting!" << endl;
             return 5;
          }
          for(size_t i = 0; i < 10; i++)
              if(dblArr[i] != dstDblArr[i]) {
                 cerr << "(Read/Write double-array 2) Expected '" << dblArr[i] << "' at pos. " << i << ", got " << dstDblArr[i] << ", quitting!" << endl;
                 return 6;
              }
       }
       szDims.clear();
       delete[] dstDblArr;
       delete[] dblArr;
     //Int
       sdi.template writeVar<size_t>("arrInt", 1, 10, intArr);
       readResults = true;
       try {
           dstIntArr = sdi.template readVar<size_t>("arrInt", szDims);
       } catch(MatWlibException e) {
           if(!expectExceptionOnRead)
              throw e;
           else
               readResults = false;
       }
       if(!readResults)
          cerr << "(Read/Write size_t-array 1) Skipping comparison, unable to read matrix" << endl;
       else {
          if((szDims[0] != 1) || (szDims[1] != 10)) {
             cerr << "(Read/Write size_t-array 1) Expected 1x10 array, got " << szDims[0] << "x" << szDims[1] << ", quitting!" << endl;
             return 3;
          }
          for(size_t i = 0; i < 10; i++)
              if(intArr[i] != dstIntArr[i]) {
                 cerr << "(Read/Write size_t-array 1) Expected '" << intArr[i] << "' at pos. " << i << ", got " << dstIntArr[i] << ", quitting!" << endl;
                 return 4;
              }
       }
       szDims.clear();
       delete[] dstIntArr;

       sdi.template writeVar("vecInt", 1, 10, vecInt.begin(), vecInt.end());
       readResults = true;
       try {
           dstIntArr = sdi.template readVar<size_t>("vecInt", szDims);
       } catch(MatWlibException e) {
           if(!expectExceptionOnRead)
              throw e;
           else
               readResults = false;
       }   
       if(!readResults)
          cerr << "(Read/Write size_t-array 2) Skipping comparison, unable to read matrix" << endl;
       else {
          if((szDims[0] != 1) || (szDims[1] != 10)) {
             cerr << "(Read/Write size_t-array 2) Expected 1x10 array, got " << szDims[0] << "x" << szDims[1] << ", quitting!" << endl;
             return 5;
          }    
          for(size_t i = 0; i < 10; i++)
              if(intArr[i] != dstIntArr[i]) {
                 cerr << "(Read/Write size_t-array 2) Expected '" << intArr[i] << "' at pos. " << i << ", got " << dstIntArr[i] << ", quitting!" << endl;
                 return 6;
              }
       }
       szDims.clear();
       delete[] dstIntArr;
       delete[] intArr;
   //Read/write string
     sdi.template writeVar<std::string>("teststr", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
     std::string str;
     readResults = true;
     try {
         str = sdi.template readVar<std::string>("teststr");
     } catch(MatWlibException e) {
         if(!expectExceptionOnRead)
            throw e;  
         else
            readResults = false;         
     }
     if(!readResults)
        cerr << "(Read/Write string) Skipping comparison, unable to read string" << endl;
     else if(str != "ABCDEFGHIJKLMNOPQRSTUVWXYZ") {
        cerr << "(Read/Write string) Expected 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', got '" << str << "', quitting!" << endl;
        return 7;
     }
   /*//Session
     double sessionVar = 3.141565;
     size_t mSz = 1, nSz = 1;
     SessionCmd::MAT_SIZE_CONST_T sz(mSz, nSz);
     TypedSessionCmd<double> sess0w("sessionVar", sessionVar, 0, sz), 
                             sess2r("sessionVar", sessionVar, 2, NULL);
     ExecSessionCmd sess1e("sessionVar = 2*sessionVar", 1);
     std::vector<SessionCmd *> cmds;
      cmds.push_back(&sess0w);
      cmds.push_back(&sess1e);
      cmds.push_back(&sess2r);
     SessionErrorReport err = s->session(cmds);
     if(expectExceptionOnRead) {
        if( (err.errorId.size() == 1) &&
            (err.errorId.find(2) != err.errorId.end()) )
            cerr << "(Session) Skipping comparison, unable to read double" << endl;
        else if(sessionVar != 6.28313) {
             cerr << "(Session) Expected 6.28313, got '" << sessionVar << "', quitting!" << endl;
             return 8;
        }
     } else if(sessionVar != 6.28313) {
             cerr << "(Session) Expected 6.28313, got '" << sessionVar << "', quitting!" << endl;
             return 8;
     }*/
   return 0;
}

bool functionSumStruct(const std::vector<Value *> &inputValues,std::vector<Value *> &outputValues) {

   return false;
}

bool functionSumArray(const std::vector<Value *> &inputValues, std::vector<Value *> &outputValues) {
   std::function<Value *(Value *, Value *)> funcSum =
      [&funcSum](Value *xVal, Value *yVal) -> Value *{
         Value_Array *inA = dynamic_cast<Value_Array *>(xVal),
                                   *inB = dynamic_cast<Value_Array *>(yVal);
         if((inA == NULL) || (inB == NULL) ) {
            Value_Matrix *inAMat = dynamic_cast<Value_Matrix *>(xVal),
                                       *inBMat = dynamic_cast<Value_Matrix *>(yVal),
                                       *out = new Value_Matrix();
            for(size_t r = 0; r < inAMat->mat.size(); r++) {
                if(r != 0)
                   out->addRow();
                for(size_t c = 0; c < inAMat->mat.at(r)->size(); c++) {
                    double x = getValue<double>(getArg(inAMat, r, c)), y = getValue<double>(getArg(inBMat, r, c)),
                           o = x+y;
                    out->addElementToLastRow(new Value_Scalar<double>(o));
                }
            }
            return out;
         } else {
            if(inA->arrSize != inB->arrSize)
               throw MatWlibException("Array size of inputs must be same");
            Value_Array *arrDst = new Value_Array();
            for(size_t i = 0; i < inA->arrSize; i++)
                arrDst->addElement( funcSum(getArg(inA, i), getArg(inB, i)) );
            return arrDst;
         }
      };
   outputValues.push_back( funcSum(inputValues.at(0), inputValues.at(1)) );
   return true;
}

bool functionSum(const std::vector<Value *> &inputValues,std::vector<Value *> &outputValues) {
   double x = getValue<double>(inputValues.at(0)),
          y = getValue<double>(inputValues.at(1)),
          out = x+y;
   outputValues.push_back( new Value_Scalar<double>(out) );
   return true;
}

bool functionPrintString(const std::vector<Value *> &inputValues,std::vector<Value *> &outputValues) {
   std::string text = getValue<std::string>(inputValues.at(0));
   cout << "Printing '" << text << "'" << endl;
   return true;
}



int function_test(State *s) {
   FunctionInterface *sfi = dynamic_cast<FunctionInterface *>(s);
   RawExecuteInterface *sei = dynamic_cast<RawExecuteInterface *>(s);
   assert(sfi != NULL);
   assert(sei != NULL);
   //Register functions
     std::vector<Function::Argument *> vecInputs, vecOutputs;
     vecInputs.push_back( new Function::Argument_Element(TYPE_STRING) );
     assert(sfi->registerFunction("printString", new Function(functionPrintString, vecInputs, vecOutputs)) && "Unable to register 'printString'");
     vecInputs.clear();
     vecOutputs.clear();

     /*vecInputs.push_back( new Function::Argument_Element(TYPE_DOUBLE) );
     vecInputs.push_back( new Function::Argument_Element(TYPE_DOUBLE) );
     vecOutputs.push_back( new Function::Argument_Element(TYPE_DOUBLE) );
     assert(s->registerFunction("sum", new Function(functionSum, vecInputs, vecOutputs)) && "Unable to register 'sum'");
     vecInputs.clear();
     vecOutputs.clear();*/

     vecInputs.push_back(  new Function::Argument_Array(new Function::Argument_Element(TYPE_DOUBLE)) );
     vecInputs.push_back(  new Function::Argument_Array(new Function::Argument_Element(TYPE_DOUBLE)) );
     vecOutputs.push_back( new Function::Argument_Array(new Function::Argument_Element(TYPE_DOUBLE)) );
     assert(sfi->registerFunction("sumArray", new Function(functionSumArray, vecInputs, vecOutputs)) && "Unable to register 'sumArray'");
     vecInputs.clear();
     vecOutputs.clear();

     /*vecInputs.push_back(  new Function::Argument_Struct(new Function::Argument_Element(TYPE_DOUBLE)) );
     vecInputs.push_back(  new Function::Argument_Struct(new Function::Argument_Element(TYPE_DOUBLE)) );
     vecOutputs.push_back( new Function::Argument_Struct(new Function::Argument_Element(TYPE_DOUBLE)) );
     assert(s->registerFunction("sumStruct", new Function(functionSumStruct, vecInputs, vecOutputs)) && "Unable to register 'sumStruct'");
     vecInputs.clear();
     vecOutputs.clear();*/
   //Print "Hello world
     sei->execute("matwlibFunc('printString', 'Hello world');");
   //Sum 13+5 and (13+5)+3.5
     sei->execute("a = matwlibFunc('sumArray', 13, 5);");
     sei->execute("a = matwlibFunc('sumArray', a, 3.5);");
   //Sum [1 2 3 4] + [5 6 7 8]
     sei->execute("b = matwlibFunc('sumArray', [1 2 3 4], [5 6 7 8])");
   //Sum [1 2 3 4; 5 6 7 8] + [5 6 7 8; 1 2 3 4]
     sei->execute("c = matwlibFunc('sumArray', [1 2 3 4; 5 6 7 8], [5 6 7 8; 1 2 3 4])");
   //Sum a+b: a(1) = [1 2 3 4; 5 6 7 8], a(2) = [5 6 7 8; 1 2 3 4], b(1) = [5 6 7 8; 1 2 3 4], b(2) = [1 2 3 4; 5 6 7 8]  
     sei->execute("dx = cat(3, [1 2 3 4; 5 6 7 8], [5 6 7 8; 1 2 3 4])");
     sei->execute("dy = cat(3, [1 2 3 4; 5 6 7 8], [5 6 7 8; 1 2 3 4]);");
     sei->execute("d = matwlibFunc('sumArray', dx, dy)");
   //Sum struct {0, 0, 0}+{1, 1, 1}
     //s->execute(2, "a = mexFunc('sumStruct', {0,0,0}, {1,1,1});");

   return 0;
};

int main(int argc, char *argv[]) {
   int rc;
   //Matlab
     #if defined(HAS_MATLAB)
      {
        cout << "Testing MATLAB engine..." << endl;
        State *s = init_engine(MatWLib::ENGINE_MATLAB, "/home/jacob/Tools/MATLAB/R2017a/bin/glnxa64");
        rc = basic_statetest<MATLAB_State>(dynamic_cast<MATLAB_State *>(s), false);
        if(rc != 0)
           return rc;
        rc = function_test(s);
        if(rc != 0)
           return rc;
        finit_engine(s);
      }
     #endif
   //Octave

   //Eigen
     #if defined(HAS_EIGEN)
      {
        cout << "Testing Eigen engine..." << endl;
        State *s = init_engine(MatWLib::ENGINE_EIGEN, "");
        rc = basic_statetest<Eigen_State>(dynamic_cast<Eigen_State *>(s), false);
        if(rc != 0)
           return rc;
        rc = basic_statelesstest<Eigen_State>(dynamic_cast<Eigen_State *>(s), false);
        if(rc != 0)
           return rc;
        finit_engine(s);
      }
     #endif
   /*//Offline
     cout << "Testing Offline engine (using 'test_lib.xml')..." << endl;
     State *os = init_engine(ENGINE_OFFLINE, "test_lib.xml");
     rc = basic_test(os, true);
     finit_engine(os);
     if(rc != 0)
        return rc;*/
   cout << "Test done!" << endl;
   return 0;
}
