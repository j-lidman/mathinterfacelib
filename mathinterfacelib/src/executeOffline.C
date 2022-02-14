#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include <assert.h>
#include <iomanip>

#include <boost/lexical_cast.hpp>

#include "matwlib.h"
#include "Offline_Engine.h"

using namespace std;
using namespace boost;
using boost::lexical_cast;
using namespace MatWLib;

//"matlab" must be accessible from $PATH

void printUsage(char *progName, int exitStatus) {
   cout << "Usage: " << progName << " <FILENAME> <START ID> <END ID> [-f]" << endl;
   exit(exitStatus);
}

size_t toInt(char *str) {
   size_t ack = 0;
   for(size_t i = 0; str[i] != '\0'; i++)
       if(('0' <= str[i]) && (str[i] <= '9'))
          ack = 10*ack + (str[i] - '0');
       else {
         cout << "Invalid number ('" << str << "')" << endl;
         exit(1);
       }
   return ack;
}

SessionCmd::TYPE_T getType(xmlChar *typeStr) {
   if(!xmlStrcmp(typeStr, (const xmlChar *) "TYPE_DOUBLE"))
      return SessionCmd::TYPE_DOUBLE;
   else if(!xmlStrcmp(typeStr, (const xmlChar *) "TYPE_STRING"))
      return SessionCmd::TYPE_STRING;
   else if(!xmlStrcmp(typeStr, (const xmlChar *) "TYPE_SIZET"))
      return SessionCmd::TYPE_SIZET;
   else 
      assert(false);
}

template<class T> T getValue(const char *str) {
   return lexical_cast<T>(str);
}
template<class T> T *getValues(const char *str, size_t m, size_t n) {
   T *mat = new T[m*n];
   for(size_t i = 0, beginTerm = 0, elemIndex = 0; i < strlen(str); i++) 
       switch(str[i]) {
        case '[':  beginTerm = i+1;  
                   break;
        case ']':
        case ';':  
        case '\r':
        case '\n':
        case '\t':
        case ' ':  if(i != beginTerm) {
                      T v;
                      std::string substr(str+beginTerm, str+i);
                      try {
                           v = lexical_cast<T>(substr);
                      } catch(bad_lexical_cast &) {
                              throw MatWlibException("Unable to decode value.");
                      }
                      mat[elemIndex++] = v;
                   }
                   beginTerm = i+1;
                   break;
        default:
            break;
       }
   return mat;
}


xmlNodePtr createResultNode() {
   return xmlNewNode(NULL, BAD_CAST "RESULT");;
} 

template<class T> xmlNodePtr createResultNode(T val) {
   xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "RESULT");
   stringstream ssData;

   ssData << val;
   xmlNewProp(node, BAD_CAST "m", BAD_CAST "1");
   xmlNewProp(node, BAD_CAST "n", BAD_CAST "1");
   xmlNewProp(node, BAD_CAST "Value", BAD_CAST ssData.str().c_str());

   return node;
}
template<class T> xmlNodePtr createResultNode(T *arr, size_t m, size_t n) {
   xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "RESULT");
   stringstream ssM, ssN, ssData;
   ssM << m;
   ssN << n;

   ssData << "[";
   for(size_t mI = 0; mI < m; mI++) {
       for(size_t nI = 0; nI < n; nI++)
           ssData << (nI == 0 ? "" : " ") << std::setprecision(15) << arr[mI*n+nI];
       ssData << (mI == m-1 ? "" : "; ");
   }
   ssData << "]";
   xmlNewProp(node, BAD_CAST "m", BAD_CAST ssM.str().c_str());
   xmlNewProp(node, BAD_CAST "n", BAD_CAST ssN.str().c_str());
   xmlNewProp(node, BAD_CAST "Value", BAD_CAST ssData.str().c_str());

   return node;
}

int main(int argc, char *argv[]) {
   //Decode arguments
     char *fName;
     if(argc < 4)
        printUsage(argv[0], 2);
     fName = argv[1];
     size_t startId = toInt(argv[2]), endId = toInt(argv[3]);
     bool optArgForce = false;
     for(int optArg = 4; optArg < argc; optArg++)
         if(strcmp(argv[optArg], "-f") == 0)
            optArgForce = true;

   //Open file
     Offline_State *s = dynamic_cast<Offline_State *>(init_engine(ENGINE_OFFLINE, fName));
     if(s == NULL) {
        cout << "Unable to initialize engine, quitting!" << endl;
        exit(3);
     }
     std::map<size_t, xmlNodePtr> &nodeMap = s->getNodeMap(), &resultNodeMap = s->getNodeResultMap();
     /*cout << "Entries: {";
     for(std::map<size_t, xmlNodePtr>::iterator it = nodeMap.begin(); it != nodeMap.end(); ++it) {
         cout << it->first;
         if(resultNodeMap.find(it->first) != resultNodeMap.end())
            cout << " (R)";
         cout << ", ";
     }         
     cout << "}" << endl;*/
     if(nodeMap.size() == 0) {
        cerr << "File '" << fName << "' doesn't contain any entries." << endl;
        exit(3);
     }
   //Execute
     State *sExec = init_engine(ENGINE_MATLAB, "/home/jacob/Tools/MATLAB/R2012a/bin/glnxa64");
     
     size_t m, n;
     for(size_t i = startId; i <= endId; i++)
         if(nodeMap.find(i) == nodeMap.end()) {
            cerr << "Error executing id #" << i << ", entry doesn't exist." << endl; 
            exit(4);
         } else if( (resultNodeMap.find(i) == resultNodeMap.end()) || optArgForce) {
            if(resultNodeMap.find(i) != resultNodeMap.end()) {
               xmlUnlinkNode(resultNodeMap.at(i));
               xmlFreeNode(resultNodeMap.at(i));
               resultNodeMap.erase(resultNodeMap.find(i));
            }

            xmlNodePtr node = nodeMap.at(i);
            if(!xmlStrcmp(node->name, (const xmlChar *) "EXECUTE")) {
               if(xmlHasProp(node, (const xmlChar *) "Cmd") == NULL) {
                   cerr << "(Execute) Unable to find 'Cmd' argument" << endl;
                   exit(5);
               }                 
               xmlChar *cmdProp = xmlGetProp(node, (const xmlChar *) "Cmd");
               execute(i, sExec, (const char *) cmdProp);
               xmlFree(cmdProp);
               
               s->addResult(i, createResultNode() );
            } else if(!xmlStrcmp(node->name, (const xmlChar *) "READ_VAR")) {
               //Get properties
                 if( (xmlHasProp(node, (const xmlChar *) "VarName") == NULL) ||
                     (xmlHasProp(node, (const xmlChar *) "Type") == NULL) ) {
                     cerr << "(Execute) Unable to find 'VarName' or 'Type' argument" << endl;
                     exit(5);
                 } 
                 xmlChar *varNameProp = xmlGetProp(node, (const xmlChar *) "VarName"),
                         *typeProp = xmlGetProp(node, (const xmlChar *) "Type");
               //Compute results
                 xmlNodePtr resNode = NULL;
                 switch(getType(typeProp)) {
                  case SessionCmd::TYPE_DOUBLE:
                     {
                       double val = readVar<double>(i, sExec, (const char *) varNameProp);
                       resNode = createResultNode(val);
                     } break;
                  case SessionCmd::TYPE_STRING:
                     {
                       std::string val = readVar<std::string>(i, sExec, (const char *)  varNameProp);
                       resNode = createResultNode(val);
                     } break;
                  case SessionCmd::TYPE_SIZET:
                     {
                       size_t val = readVar<size_t>(i, sExec, (const char *) varNameProp);
                       resNode = createResultNode(val);
                     } break;
                  default:
                    assert(false);
                 }
                 if(xmlAddChild(node, resNode) == NULL) {
                    cerr << "Unable to attach result node." << endl;
                    exit(5);
                 }
                 s->addResult(i, resNode);
               //Free properties
                 xmlFree(typeProp);
                 xmlFree(varNameProp);
            } else if(!xmlStrcmp(node->name, (const xmlChar *) "READ_ARR")) {
               //Get properties
                 if( (xmlHasProp(node, (const xmlChar *) "VarName") == NULL) ||
                     (xmlHasProp(node, (const xmlChar *) "Type") == NULL) ) {
                     cerr << "(Execute) Unable to find 'VarName' or 'Type' argument" << endl;
                     exit(5);
                 } 
                 xmlChar *varNameProp = xmlGetProp(node, (const xmlChar *) "VarName"),
                         *typeProp = xmlGetProp(node, (const xmlChar *) "Type");
               //Compute results
                 xmlNodePtr resNode = NULL;
                 switch(getType(typeProp)) {
                  case SessionCmd::TYPE_DOUBLE:
                     {
                       double *arr = readVar<double>(i, sExec, (const char *) varNameProp, m, n);
                       resNode = createResultNode(arr, m, n);
                     } break;
                  case SessionCmd::TYPE_STRING:
                       //std::string *arr = readVar<std::string>(i, sExec, varNameProp, m, n);
                       //resNode = createResultNode(arr, m, n);
                       assert(false && "NOT IMPLEMENTED");
                       break;
                  case SessionCmd::TYPE_SIZET:
                     {
                       size_t *arr = readVar<size_t>(i, sExec, (const char *) varNameProp, m, n);
                       resNode = createResultNode(arr, m, n);
                     } break;
                  default:
                    assert(false);
                 }
                 if(xmlAddChild(node, resNode) == NULL) {
                    cerr << "Unable to attach result node." << endl;
                    exit(5);
                 }
                 s->addResult(i, resNode);
               //Free properties
                 xmlFree(typeProp);
                 xmlFree(varNameProp);
            } else if(!xmlStrcmp(node->name, (const xmlChar *) "WRITE_VAR")) {
               //Get properties
                 if( (xmlHasProp(node, (const xmlChar *) "VarName") == NULL) ||
                     (xmlHasProp(node, (const xmlChar *) "Value") == NULL) ||
                     (xmlHasProp(node, (const xmlChar *) "Type") == NULL) ) {
                     cerr << "(Execute) Unable to find 'VarName' argument" << endl;
                     exit(5);
                 } 
                 xmlChar *varNameProp = xmlGetProp(node, (const xmlChar *) "VarName"),
                         *typeProp = xmlGetProp(node, (const xmlChar *) "Type"),
                         *valueProp = xmlGetProp(node, (const xmlChar *) "Value");
               //Compute results
                 xmlNodePtr resNode = NULL;
                 switch(getType(typeProp)) {
                  case SessionCmd::TYPE_DOUBLE:
                     {
                       double val = getValue<double>((const char *) valueProp);
                       writeVar<double>(i, sExec, (const char *) varNameProp, val);
                       resNode = createResultNode();
                     } break;
                  case SessionCmd::TYPE_STRING:
                     {
                       std::string val = std::string((const char *) valueProp);
                       writeVar<std::string>(i, sExec, (const char *) varNameProp, val);
                       resNode = createResultNode();
                     } break;
                  case SessionCmd::TYPE_SIZET:
                     {
                       size_t val = getValue<size_t>((const char *) valueProp);
                       writeVar<size_t>(i, sExec, (const char *) varNameProp, val);
                       resNode = createResultNode();
                     } break;
                  default:
                     assert(false);
                 }
                 if(xmlAddChild(node, resNode) == NULL) {
                    cerr << "Unable to attach result node." << endl;
                    exit(5);
                 }
                 s->addResult(i, resNode);
               //Free properties
                 xmlFree(valueProp);
                 xmlFree(typeProp);
                 xmlFree(varNameProp);
            } else if(!xmlStrcmp(node->name, (const xmlChar *) "WRITE_ARR")) {
               //Get properties
                 if( (xmlHasProp(node, (const xmlChar *) "VarName") == NULL) ||
                     (xmlHasProp(node, (const xmlChar *) "Value") == NULL) ||
                     (xmlHasProp(node, (const xmlChar *) "Type") == NULL) ||
                     (xmlHasProp(node, (const xmlChar *) "m") == NULL) ||
                     (xmlHasProp(node, (const xmlChar *) "n") == NULL) ) {
                     cerr << "(Execute) Unable to find 'VarName' argument" << endl;
                     exit(5);
                 } 
                 xmlChar *varNameProp = xmlGetProp(node, (const xmlChar *) "VarName"),
                         *typeProp = xmlGetProp(node, (const xmlChar *) "Type"),
                         *valueProp = xmlGetProp(node, (const xmlChar *) "Value"),
                         *mProp = xmlGetProp(node, (const xmlChar *) "m"),
                         *nProp = xmlGetProp(node, (const xmlChar *) "n");
                 try {
                     m = lexical_cast<double>(mProp);
                     n = lexical_cast<double>(nProp);
                 } catch(bad_lexical_cast &) {
                         throw MatWlibException("Unable to decode double.");
                 }
               //Compute results
                 xmlNodePtr resNode = NULL;
                 switch(getType(typeProp)) {
                  case SessionCmd::TYPE_DOUBLE:
                     {
                       double *val = getValues<double>((const char *) valueProp, m, n);
                       #ifdef MATWLIB_DEBUG
                           cout << "Writing double array (" << m << ", " << n << ") '" << (const char *) varNameProp << "' [";
                           for(size_t mI = 0; mI < m; mI++) {
                               for(size_t nI = 0; nI < n; nI++)
                                   cout << (nI != 0 ? " " : "") << val[mI*m+nI];
                               cout << (mI != m-1 ? "; " : "");
                           }
                           cout << "]" << endl;
                       #endif
                       writeVar<double>(i, sExec, (const char *) varNameProp, m, n, val);
                       resNode = createResultNode();
                     } break;
                  case SessionCmd::TYPE_STRING:
                       //std::string val = std::string(valueProp);
                       //writeVar(i, sExec, varNameProp, val);
                       //resNode = createResultNode();
                       assert(false && "NOT IMPLEMENTED");
                       break;
                  case SessionCmd::TYPE_SIZET:
                     {
                       size_t *val = getValues<size_t>((const char *) valueProp, m, n);
                       #ifdef MATWLIB_DEBUG
                           cout << "Writing size_t array (" << m << ", " << n << ") '" << (const char *) varNameProp << "' [";
                           for(size_t mI = 0; mI < m; mI++) {
                               for(size_t nI = 0; nI < n; nI++)
                                   cout << (nI != 0 ? " " : "") << val[mI*m+nI];
                               cout << (mI != m-1 ? "; " : "");
                           }
                           cout << "]" << endl;
                       #endif
                       writeVar<size_t>(i, sExec, (const char *) varNameProp, m, n, val);
                       resNode = createResultNode();
                     } break;
                  default:
                     assert(false);
                 }
                 if(xmlAddChild(node, resNode) == NULL) {
                    cerr << "Unable to attach result node." << endl;
                    exit(5);
                 }
                 s->addResult(i, resNode);
               //Free properties
                 xmlFree(valueProp);
                 xmlFree(typeProp);
                 xmlFree(varNameProp);
                 xmlFree(mProp);
                 xmlFree(nProp);
            } else {
               cerr << "Unrecognized command '" << node->name << "'" << endl;
               exit(5);
            } 

         }

   finit_engine(s);
   finit_engine(sExec);
   return 0;
}
