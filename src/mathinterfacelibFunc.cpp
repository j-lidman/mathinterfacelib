#include <iostream>
#include <sstream>
#include <vector>
#include <stack>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/bind.hpp>

#include <boost/any.hpp>

#include "mex.h"
#include "mathinterfacelib_const.h"
#include "mathinterfacelib_ipc.h"

using namespace std;
using namespace MathInterfaceLib;
using namespace boost;
using namespace boost::interprocess;
namespace qi = boost::spirit::qi;

void MEX_Assert(bool b, std::string s) {
   if(!b)
      mexErrMsgTxt(s.c_str());
}

struct Response_Parser {
   struct CompositeType {
      CompositeType() {}
      virtual ~CompositeType() {}

      virtual mxArray *convert(mxArray *in = NULL, size_t offset = 0, size_t step = 1) = 0;
   };
   struct Matrix : public CompositeType {
      TYPE t;
      std::vector<std::vector<boost::any> *> elems;

      Matrix() : t(TYPE_UNKNOWN) {
         elems.push_back( new std::vector<boost::any>() );
      }
      virtual ~Matrix() {
         for(std::vector<std::vector<boost::any> *>::iterator it = elems.begin(); it != elems.end(); ++it)
             delete *it;
      }

      virtual mxArray *convert(mxArray *in = NULL, size_t offset = 0, size_t step = 1) {
         mxArray *arr = in;
         size_t m = this->elems.size(), n = this->elems.at(0)->size();
         switch(this->t) {
          case TYPE_UNKNOWN:
            MEX_Assert(false, "Unable to create matrix of unknown type");
          case TYPE_INT:
            {
               if(arr == NULL)
                  switch(sizeof(size_t)) {
                   case 4:    arr = mxCreateNumericMatrix(m, n, mxUINT32_CLASS, mxREAL);     break;
                   case 8:    arr = mxCreateNumericMatrix(m, n, mxUINT64_CLASS, mxREAL);     break;
                   default:   MEX_Assert(false, "Undefined size_t size");
                  }
               size_t *ptr = (size_t *) mxGetPr(arr);
               try {
                  for(size_t c = 0; c < n; c++)
                      for(size_t r = 0; r < m; r++)
                          ptr[offset+c*m+r] = boost::any_cast<size_t>(this->elems.at(r)->at(c));
               } catch(const boost::bad_any_cast &) {
                  MEX_Assert(false, "Unable to cast matrix element to size_t");
               }
            } break;
          case TYPE_DOUBLE:
            {
               if(arr == NULL)
                  arr = mxCreateNumericMatrix(m, n, mxDOUBLE_CLASS, mxREAL);
               double *ptr = mxGetPr(arr);
               try {
                  for(size_t c = 0; c < n; c++)
                      for(size_t r = 0; r < m; r++)
                          ptr[offset+c*m+r] = boost::any_cast<double>(this->elems.at(r)->at(c));
               } catch(const boost::bad_any_cast &) {
                  MEX_Assert(false, "Unable to cast matrix element to double");
               }
            } break;
          case TYPE_STRING:
               MEX_Assert(false, "TODO!");
         }
         return arr;
      }
   };
   struct Array : public CompositeType {
      std::vector<CompositeType *> elems;

      Array() {}
      virtual ~Array() {}

      virtual mxArray *convert(mxArray *in = NULL, size_t offset = 0, size_t step = 1) {
         mxArray *arr = in;
         if(arr == NULL) {
            step = 1;
            offset = 0;

            //Deduce step, total size and base class
              Array *a;
              Matrix *m;
              mwSize nDim = 1;
              mxClassID cId;
              for(CompositeType *c = elems.front(); c != NULL; )
                  if((a = dynamic_cast<Array *>(c)) != NULL) {
                    step *= a->elems.size();
                    c = a->elems.front();
                    nDim++;
                  } else if((m = dynamic_cast<Matrix *>(c)) != NULL) {
                    step *= (m->elems.size() * m->elems.front()->size());
                    switch(m->t) {
                     case TYPE_UNKNOWN:
                          MEX_Assert(false, "Unable to create array of unknown type");
                          break;
                     case TYPE_INT:
                          switch(sizeof(size_t)) {
                           case 4:    cId = mxUINT32_CLASS;     break;
                           case 8:    cId = mxUINT64_CLASS;     break;
                           default:   MEX_Assert(false, "Undefined size_t size");
                          } break;
                     case TYPE_DOUBLE:
                          cId = mxDOUBLE_CLASS;
                          break;
                     case TYPE_STRING:
                          MEX_Assert(false, "TODO!");
                    }     
                    c = NULL;
                    nDim += 2;
                  } else
                    MEX_Assert(false, "Unhandled composite type");
            //Deduce size per dim
              mwSize dims[nDim];
              size_t currDim = nDim-1;
              for(CompositeType *c = this; c != NULL; )
                 if((a = dynamic_cast<Array *>(c)) != NULL) {
                    dims[currDim--] = a->elems.size();
                    c = a->elems.front();
                  } else if((m = dynamic_cast<Matrix *>(c)) != NULL) {
                    dims[currDim-0] = m->elems.front()->size();
                    dims[currDim-1] = m->elems.size();
                    c = NULL;
                  } else
                    MEX_Assert(false, "Unhandled composite type");
            //Create array
              arr = mxCreateNumericArray(nDim, &dims[0], cId, mxREAL);
         }
         
         for(size_t i = 0; i < elems.size(); ++i)
             elems[i]->convert(arr, offset+step*i, step/elems.size());
         return arr;
      }
   };  

   typedef enum {
      STATUS_ERROR,
      STATUS_OK,
   } STATUS;

   STATUS s;
   std::string error;      
   std::vector<mxArray *> outputValues;
   std::stack<CompositeType *> compTypes;

   //(De/Con)structor(s)
     Response_Parser() : s(STATUS_OK) {}
     ~Response_Parser() {}
   //Functions
     void handleError(std::vector<char> str_) {
         s = STATUS_ERROR;
         error = std::string(str_.begin(), str_.end());
     }

     void addMatrix() {
         if(s == STATUS_ERROR)
            return;
//cout << "Added matrix..." << endl;
         compTypes.push( new Matrix() );
     }
     void addMatrixRow() {
         if(s == STATUS_ERROR)
            return;
//cout << "Added matrix row..." << endl;
         MEX_Assert(compTypes.size() != 0, "Stack is empty");
         Matrix *t = dynamic_cast<Matrix *>(compTypes.top());
         MEX_Assert(t != NULL, "Expected a matrix element on top of stack");
         t->elems.push_back( new std::vector<boost::any>() );
     }
     void commitMatrix() {
         if(s == STATUS_ERROR)
            return;
//cout << "Committed matrix..." << endl;
         MEX_Assert(compTypes.size() != 0, "Stack is empty");
         Matrix *t = dynamic_cast<Matrix *>(compTypes.top());
         MEX_Assert(t != NULL, "Expected a matrix element on top of stack");
         compTypes.pop();
         
         if(compTypes.size() == 0)
            outputValues.push_back(t->convert());
         else {
            Array *arr = dynamic_cast<Array *>(compTypes.top());
            if(arr != NULL)
               arr->elems.push_back(t);
            else
               MEX_Assert(false, "Unhandled composite type");
         }
     }

     void addArray() {
         if(s == STATUS_ERROR)
            return;
//cout << "Added array..." << endl;
         compTypes.push( new Array() );
     }
     void commitArray() {
         if(s == STATUS_ERROR)
            return;
//cout << "Committed array..." << endl;
         MEX_Assert(compTypes.size() != 0, "Stack is empty");
         Array *t = dynamic_cast<Array *>(compTypes.top());
         MEX_Assert(t != NULL, "Expected a array element on top of stack");
         compTypes.pop();

         if(compTypes.size() == 0)
            outputValues.push_back(t->convert());
         else {
            Array *arr = dynamic_cast<Array *>(compTypes.top());
            if(arr != NULL)
               arr->elems.push_back(t);
            else
               MEX_Assert(false, "Unhandled composite type");
         }
     }

     void handleOutString(std::vector<char> str_) {
         if(s == STATUS_ERROR)
            return;
         if(compTypes.size() == 0) {
            char arr[str_.size()];
            for(size_t i = 0; i < str_.size(); ++i)
                arr[i] = str_[i];
            this->outputValues.push_back(mxCreateStringFromNChars(&arr[0], str_.size()));
         } else {
            Matrix *m = dynamic_cast<Matrix *>(compTypes.top());
            if(m != NULL) {
               //Check (and set) base type
                 if(m->t == TYPE_UNKNOWN)
                    m->t = TYPE_STRING;
                 else
                    MEX_Assert(m->t == TYPE_STRING, "Invalid base type");
               //Add value
                 m->elems.back()->push_back(str_);
            } else
               MEX_Assert(false, "Unhandled case in handleOutString()");
         }
     }
     void handleOutInteger(size_t i_) {
         if(s == STATUS_ERROR)
            return;
         if(compTypes.size() == 0) {
            mxArray *arr;
            switch(sizeof(size_t)) {
             case 4:    arr = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);     break;
             case 8:    arr = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);     break;
             default:
               {
                  error = "Unable to decode integer";
                  s = STATUS_ERROR;
               };
            }
            size_t *ptr = (size_t *) mxGetPr(arr);
            *ptr = i_;
            this->outputValues.push_back(arr);
         } else {
            Matrix *m = dynamic_cast<Matrix *>(compTypes.top());
            if(m != NULL) {
               //Check (and set) base type
                 if(m->t == TYPE_UNKNOWN)
                    m->t = TYPE_INT;
                 else
                    MEX_Assert(m->t == TYPE_INT, "Invalid base type");
               //Add value
                 m->elems.back()->push_back(i_);
            } else
               MEX_Assert(false, "Unhandled case in handleOutInteger()");
         }
     }
     void handleOutDouble(double d_) {
         if(s == STATUS_ERROR)
            return;
         if(compTypes.size() == 0)
            this->outputValues.push_back( mxCreateDoubleScalar(d_) );
         else {
            Matrix *m = dynamic_cast<Matrix *>(compTypes.top());
            if(m != NULL) {
               //Check (and set) base type
                 if(m->t == TYPE_UNKNOWN)
                    m->t = TYPE_DOUBLE;
                 else
                    MEX_Assert(m->t == TYPE_DOUBLE, "Invalid base type");
               //Add value
                 m->elems.back()->push_back(d_);
            } else
               MEX_Assert(false, "Unhandled case in handleOutDouble()");
         }
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

         real_parser<double, strict_real_policies<double> > strict_double;
         qi::rule<std::string::const_iterator, std::string()> str_ = lexeme["'" >> (+(char_ - '\''))[boost::bind(&Response_Parser::handleOutString, this, _1)] >> "'"],
                                                              strRow_ = (str_ >> -("," >> strRow_)),
                                                              strMat_ = (strRow_ >> -(lit(";")[boost::bind(&Response_Parser::addMatrixRow, this)] >> strMat_)),
                                                              float_ = strict_double[boost::bind(&Response_Parser::handleOutDouble, this, _1)],
                                                              floatRow_ = (float_ >> -("," >> floatRow_)),
                                                              floatMat_ = (floatRow_ >> -(lit(";")[boost::bind(&Response_Parser::addMatrixRow, this)] >> floatMat_)),
                                                              int_ = uint_[boost::bind(&Response_Parser::handleOutInteger, this, _1)],
                                                              intRow_ = (int_ >> -("," >> intRow_)),
                                                              intMat_ = (intRow_ >> -(lit(";")[boost::bind(&Response_Parser::addMatrixRow, this)] >> intMat_)),
                                                              matrix_ = (lit('[')[boost::bind(&Response_Parser::addMatrix, this)] >> 
                                                                             -(floatMat_ | strMat_ | intMat_) >>
                                                                         lit("]")[boost::bind(&Response_Parser::commitMatrix, this)]),
                                                              array_ = (lit('(')[boost::bind(&Response_Parser::addArray, this)] >>
                                                                             *((matrix_ | array_ | str_ | float_ | int_) >> ";") >>
                                                                        lit(')')[boost::bind(&Response_Parser::commitArray, this)]);

         return phrase_parse(line.begin(), line.end(),
                  (
                     ("ERROR('" >> lexeme[+(char_ - '\'')][boost::bind(&Response_Parser::handleError, this, _1)] >> "')") |
                     ("(" >> *((matrix_ | array_ | str_ | float_ | int_) >> ";") >> ")")
                  ), space);
     }
};

void sendString(SharedRegion *data, std::string str) {
   for(size_t i = 0, startIndex = data->currWrite; str[i] != '\0'; ) {
       data->scratchPad[data->currWrite] = str[i++];
       data->currWrite = (data->currWrite + 1) % PROCESS_SHARED_BUFF_SIZE;
       if(data->currWrite == startIndex) {
          mexErrMsgTxt("Buffer overflow occured!");
          break;
       }
   }          
   data->writer.post();
}

//Based on mathworks.com/help/matlab/math/multidimensional-arrays.html
template<typename T> void recurEncodeMatrix(std::stringstream &ss, size_t nDim, size_t currDim, const mwSize *dimSz, T *ptr, size_t offset, size_t step) {
   if(currDim == 1) {
      ss << "[";
      for(size_t r = 0; r < dimSz[0]; r++) { //Row
          ss << (r == 0 ? "" : ";");
          for(size_t c = 0; c < dimSz[1]; c++) { //Column
              ss << (c == 0 ? "" : ",") << ptr[offset+(r+c*dimSz[0])];
          }
      }
      ss << "]";
   } else {
      ss << "(";
      for(size_t i = 0; i < dimSz[currDim]; i++) {
          recurEncodeMatrix(ss, nDim, currDim-1, dimSz, ptr, offset+step*i, step/dimSz[currDim]);
          ss << ";";
      }
      ss << ")";
   }
}

bool recurEncodeInput(std::stringstream &ss, const mxArray *arr) {
   if(mxIsNumeric(arr)) {
      if(mxIsDouble(arr)) {
         //Array?
           double *ptr = mxGetPr(arr);
           size_t nDim = mxGetNumberOfDimensions(arr);
           const mwSize *dimSz = mxGetDimensions(arr);
           //if((nDim == 2) && (dimSz[0] == 1)  && (dimSz[1] == 1))
           //    ss << ptr[0];
           //else {
                 size_t step = 1;
                 for(size_t i = 0; i < nDim-1; i++)
                     step *= dimSz[i];
                 recurEncodeMatrix<double>(ss, nDim, nDim-1, dimSz, ptr, 0, step);
           //}
      } else
         assert(false);
   } else if(mxIsChar(arr)) {
      char *str = mxArrayToString(arr);
      ss << "'" << str << "'";
      mxFree(str);
   } else if(mxIsStruct(arr)) {
      assert(false);

   } else if(mxIsCell(arr)) {
      assert(false);

   } else {
      std::stringstream ssErr;
      ssErr << "Unhandled argument type: " << mxGetClassName(arr);
      mexErrMsgTxt(ssErr.str().c_str());
      return false;
   }
   ss << ";";
   return true;
}

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[] ) {
   //Look for mathinterfacelib shared memory
     shared_memory_object shm = shared_memory_object(open_or_create, MATHINTERFACELIB_SHARED_MEM_NAME, read_write);
     mapped_region region(shm, read_write);
     SharedRegion *data = static_cast<SharedRegion *>(region.get_address());
   //Get function name and write arguments to shared region
     if((nrhs < 1) || !mxIsChar(prhs[0])) {
        mexErrMsgTxt("Proxy function called without inner function identifier");
        return;
     }
     std::stringstream ssCmd;
     ssCmd.setf(std::ios::showpoint);
     char *funcName = mxArrayToString(prhs[0]);
     ssCmd << funcName << "(";
     mxFree(funcName);
     for(size_t argIndex = 1; argIndex < nrhs; argIndex++)
         if(!recurEncodeInput(ssCmd, prhs[argIndex]))
            return;
     ssCmd << ")->" << nlhs;
     sendString(data, ssCmd.str());
   //Wait for response
     data->reader.wait();
   //Parse response
     Response_Parser g;
     std::stringstream ss;
     for(; data->currRead != data->currWrite; data->currWrite = (data->currWrite + 1) % PROCESS_SHARED_BUFF_SIZE)
         ss << data->scratchPad[data->currWrite];
     #ifdef MATHINTERFACELIB_DEBUG
             mexPrintf("Response: '%s'\n", ss.str().c_str());
     #endif
     if(!g.parse(ss.str()))
        mexErrMsgTxt("Invalid response format");
   //Handle response
     switch(g.s) {
      case Response_Parser::STATUS_ERROR:
         {
            stringstream ss;
            ss << "Error ocurred when executing function: " << g.error;
            mexErrMsgTxt(ss.str().c_str());
         } break;
      case Response_Parser::STATUS_OK:
         if(nlhs == 0) {
            #ifdef MATHINTERFACELIB_DEBUG
               mexPrintf("Response OK (No outputvalues)\n");
            #endif
         } else if(g.outputValues.size() != nlhs) { //It should be ok to ignore the output!
               mexPrintf("%d / %d\n", g.outputValues.size(), nlhs);
            mexErrMsgTxt("Invalid number of outputs");
         } else {
            for(size_t i = 0; i < nlhs; i++)
                plhs[i] = g.outputValues.at(i);
            #ifdef MATHINTERFACELIB_DEBUG
               mexPrintf("Response OK\n");
            #endif
         } break;
     }
}
