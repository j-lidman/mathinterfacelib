#include <sstream>
#include <vector>
#include <iomanip>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "Offline_Engine.h"

using namespace boost;
using namespace boost::filesystem;
using boost::lexical_cast;
using namespace std;

namespace MatWLib {
   Offline_State *init_offline_engine(std::string path) {
      //Does document exist?
        boost::filesystem::path pStatFile(path);
        if(exists(pStatFile)) {
           #ifdef USE_XML
            //Open document
              xmlDocPtr doc;
              if((doc = xmlReadFile(path.c_str(), NULL, 0)) == NULL)
                 throw MatWlibException("Unable to open file");
              xmlNode *root = xmlDocGetRootElement(doc);
              if( xmlStrcmp(root->name, (const xmlChar *) "MATWLIB_OFFLINE") )
                 throw MatWlibException("Invalid root node name, expected 'MATWLIB_OFFLINE'");
            //Construct node<->ID map  
              map<size_t, xmlNodePtr> nodeIdMap, resultNodeMap;
              for(xmlNode *childNode = root->children; childNode; childNode = childNode->next)
                  if(childNode->type == XML_ELEMENT_NODE) {
                     //Get op id
                       size_t opId;
                       xmlChar *propText = xmlGetProp(childNode, (const xmlChar *) "id");
                       if(propText == NULL)
                          throw MatWlibException("Missing op index property.");
                       try {
                           opId = lexical_cast<size_t>(propText);
                       } catch(bad_lexical_cast &) {
                           throw MatWlibException("Invalid op index, expected a integer.");
                       }
                       xmlFree(propText);
                     //Add mapping
                       if(nodeIdMap.find(opId) != nodeIdMap.end())
                          throw MatWlibException("Invalid file, can't have multiple nodes with same id");
                       nodeIdMap[opId] = childNode;
                     //Is there a result node?
                       for(xmlNode *resNode = childNode->children; resNode; resNode = resNode->next)
                           if( (resNode->type == XML_ELEMENT_NODE) &&
                               !xmlStrcmp(resNode->name, (const xmlChar *) "RESULT") ) {
                                 resultNodeMap[opId] = resNode;
                                 #ifdef MATWLIB_DEBUG
                                    cout << "Found result for '" << opId << "'" << endl;
                                 #endif
                           }
              }

            return new Offline_State(false, path, doc, root, nodeIdMap, resultNodeMap);
           #endif
        } else {
           #ifdef USE_XML
            //Create document
              xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
              xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "MATWLIB_OFFLINE");
              xmlDocSetRootElement(doc, root);

            return new Offline_State(true, path, doc, root);
           #endif
        }
   }
   void finit_offline_engine(Offline_State *s) {
      if(s->hasDocChanged()) {
         #ifdef USE_XML
         //Save document
           xmlSaveFormatFileEnc(s->getFName().c_str(), s->getDoc(), "UTF-8", 1);
           xmlFreeDoc(s->getDoc());
           xmlCleanupParser();
         #endif
      } else {
         #ifdef MATWLIB_DEBUG
            cout << "Doc hasn't changed!" << endl;
         #endif
      }
   }
   template<> Offline_State *cast_state<Offline_State>(State *s) {
      Offline_State *s_ = dynamic_cast<Offline_State *>(s);
      assert((s_ != NULL) && "Invalid Offline State");
      return s_;
   }


   size_t  MatWLib::Offline_State::getNewOpdId() {return this->opId++;}

   #ifdef USE_XML
      MatWLib::Offline_State::Offline_State(bool docChanged_, std::string fName_, xmlDocPtr doc_, xmlNodePtr rootNode_, std::map<size_t, xmlNodePtr> &nodeIdMap_, std::map<size_t, xmlNodePtr> &resultNodeMap_)
        : opId(0), docChanged(docChanged_), fName(fName_), doc(doc_), rootNode(rootNode_), 
          nodeIdMap(nodeIdMap_), resultNodeMap(resultNodeMap_) {}
      MatWLib::Offline_State::Offline_State(bool docChanged_, std::string fName_, xmlDocPtr doc_, xmlNodePtr rootNode_) 
        : opId(0), docChanged(docChanged_), fName(fName_), doc(doc_), rootNode(rootNode_) {}
      MatWLib::Offline_State::~Offline_State() {}
      bool MatWLib::Offline_State::good() const {return (this->doc != NULL);}
      bool MatWLib::Offline_State::hasDocChanged() const {return docChanged;}
      xmlNodePtr MatWLib::Offline_State::getRootNode() {return this->rootNode;}
      xmlDocPtr MatWLib::Offline_State::getDoc() {return this->doc;}
      void MatWLib::Offline_State::insertNode(size_t opId, xmlNodePtr node) {
         if(nodeIdMap.find(opId) == nodeIdMap.end()) {
            if(xmlAddChild(getRootNode(), node) == NULL)
               throw MatWlibException("Unable to add write-var child");
         } else {
            if(resultNodeMap.find(opId) != resultNodeMap.end()) {
               xmlUnlinkNode(resultNodeMap.at(opId));
               xmlFreeNode(resultNodeMap.at(opId));
               resultNodeMap.erase(resultNodeMap.find(opId));
            }
            xmlFreeNode(xmlReplaceNode(nodeIdMap.at(opId), node));
         }
         nodeIdMap.find(opId)->second = node;
         this->docChanged = true;
      }

      char *MatWLib::Offline_State::getResult(size_t opId) {
         if(resultNodeMap.find(opId) == resultNodeMap.end())
            return NULL;
         else {
            xmlNodePtr node = resultNodeMap.at(opId);
            if(xmlHasProp(node, (const xmlChar *) "Value") == NULL)
               return NULL;
            else
               return (char *) xmlGetProp(node, (const xmlChar *) "Value");
         }
      }
      void MatWLib::Offline_State::addResult(size_t opId, xmlNodePtr node) {
         if(nodeIdMap.find(opId) == nodeIdMap.end())
            throw MatWlibException("Can't add result, command node is missing");
         if(resultNodeMap.find(opId) == resultNodeMap.end()) {
            if(xmlAddChild(nodeIdMap.at(opId), node) == NULL)
               throw MatWlibException("Unable to add result node to command node");
            resultNodeMap[opId] = node;
            this->docChanged = true;
         } else
           throw MatWlibException("Result node is already assigned");
      }
      void MatWLib::Offline_State::remResult(size_t opId) {
         if(nodeIdMap.find(opId) == nodeIdMap.end())
            throw MatWlibException("Can't remove result, command node is missing");
         if(resultNodeMap.find(opId) != resultNodeMap.end()) {
            xmlUnlinkNode(resultNodeMap.at(opId));
            xmlFreeNode(resultNodeMap.at(opId));

            resultNodeMap.erase(resultNodeMap.find(opId));
            this->docChanged = true;
         } else
            throw MatWlibException("Result node doesn't exist");
      }
      std::map<size_t, xmlNodePtr> &MatWLib::Offline_State::getNodeMap() {return nodeIdMap;}
      std::map<size_t, xmlNodePtr> &MatWLib::Offline_State::getNodeResultMap() {return resultNodeMap;}
   #endif
   std::string MatWLib::Offline_State::getFName() const {return this->fName;}
   std::string MatWLib::Offline_State::toString() const {return "Offline State";}


   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   //DataInterface<Offline_State>::DataInterface(Offline_State &s_) : s(s_) {}
   //DataInterface<Offline_State>::~DataInterface() {}

   //Iterator based variable creation
     //Double
       void Offline_State::writeVar(std::string varName, size_t m, size_t n, FWD_IT_T<double> begin, FWD_IT_T<double> end) {
         size_t opId = this->getNewOpdId();
         stringstream ss, ssId;
         ssId << opId;
         xmlNodePtr wNode = (xmlNodePtr) NULL;

         if((m == 1) && (n == 1)) {
            wNode = xmlNewNode(NULL, BAD_CAST "WRITE_VAR");
            ss << std::setprecision(15) << *begin;
            xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());
            xmlNewProp(wNode, BAD_CAST "m", BAD_CAST "1");
            xmlNewProp(wNode, BAD_CAST "n", BAD_CAST "1");
         } else {
            wNode = xmlNewNode(NULL, BAD_CAST "WRITE_ARR");
            ss << "[";
            size_t nCnt = n, mCnt = m;
            for(FWD_IT_T<double> it = begin; it != end; ++it) {
                ss << std::setprecision(15) << *it;
                if(--nCnt == 0) {
                   nCnt = n;
                   mCnt--;
                   if(mCnt > 0)
                      ss << "; ";
                } else
                   ss << " ";
            }
            ss << "]";
            xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());

            stringstream mSz, nSz;
            mSz << m;
            nSz << n;
            xmlNewProp(wNode, BAD_CAST "m", BAD_CAST mSz.str().c_str());
            xmlNewProp(wNode, BAD_CAST "n", BAD_CAST nSz.str().c_str());
         }               
         xmlNewProp(wNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
         xmlNewProp(wNode, BAD_CAST "Type", BAD_CAST "TYPE_DOUBLE");
         xmlNewProp(wNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
         this->insertNode(opId, wNode);
       }

       void Offline_State::writeVar(std::string varName, size_t m, size_t n, CONST_FWD_IT_T<double> begin, CONST_FWD_IT_T<double> end) {
         size_t opId = this->getNewOpdId();
         stringstream ss, ssId;
         ssId << opId;
         xmlNodePtr wNode = (xmlNodePtr) NULL;

         if((m == 1) && (n == 1)) {
            wNode = xmlNewNode(NULL, BAD_CAST "WRITE_VAR");
            ss << std::setprecision(15) << *begin;
            xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());
            xmlNewProp(wNode, BAD_CAST "m", BAD_CAST "1");
            xmlNewProp(wNode, BAD_CAST "n", BAD_CAST "1");
         } else {
            wNode = xmlNewNode(NULL, BAD_CAST "WRITE_ARR");
            ss << "[";
            size_t nCnt = n, mCnt = m;
            for(CONST_FWD_IT_T<double> it = begin; it != end; ++it) {
                ss << std::setprecision(15) << *it;
                if(--nCnt == 0) {
                   nCnt = n;
                   mCnt--;
                   if(mCnt > 0)
                      ss << "; ";
                } else
                   ss << " ";
            }
            ss << "]";
            xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());

            stringstream mSz, nSz;
            mSz << m;
            nSz << n;
            xmlNewProp(wNode, BAD_CAST "m", BAD_CAST mSz.str().c_str());
            xmlNewProp(wNode, BAD_CAST "n", BAD_CAST nSz.str().c_str());
         }              
         xmlNewProp(wNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
         xmlNewProp(wNode, BAD_CAST "Type", BAD_CAST "TYPE_DOUBLE");
         xmlNewProp(wNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
         this->insertNode(opId, wNode);
       }
     //Size_t
       void Offline_State::writeVar(std::string varName, size_t m, size_t n, FWD_IT_T<size_t> begin, FWD_IT_T<size_t> end) {
         size_t opId = this->getNewOpdId();
         stringstream ss, ssId;
         ssId << opId;
         xmlNodePtr wNode = (xmlNodePtr) NULL;

         if((m == 1) && (n == 1)) {
            wNode = xmlNewNode(NULL, BAD_CAST "WRITE_VAR");
            ss << *begin;
            xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());
            xmlNewProp(wNode, BAD_CAST "m", BAD_CAST "1");
            xmlNewProp(wNode, BAD_CAST "n", BAD_CAST "1");
         } else {
            wNode = xmlNewNode(NULL, BAD_CAST "WRITE_ARR");
            ss << "[";
            size_t nCnt = n, mCnt = m;
            for(FWD_IT_T<size_t> it = begin; it != end; ++it) {
                ss << *it;
                if(--nCnt == 0) {
                   nCnt = n;
                   mCnt--;
                   if(mCnt > 0)
                      ss << "; ";
                } else
                   ss << " ";
            }
            ss << "]";
            xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());

            stringstream mSz, nSz;
            mSz << m;
            nSz << n;
            xmlNewProp(wNode, BAD_CAST "m", BAD_CAST mSz.str().c_str());
            xmlNewProp(wNode, BAD_CAST "n", BAD_CAST nSz.str().c_str());
         }               
         xmlNewProp(wNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
         xmlNewProp(wNode, BAD_CAST "Type", BAD_CAST "TYPE_SIZET");
         xmlNewProp(wNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
         this->insertNode(opId, wNode);
       }

       void Offline_State::writeVar(std::string varName, size_t m, size_t n, CONST_FWD_IT_T<size_t> begin, CONST_FWD_IT_T<size_t> end) {
         size_t opId = this->getNewOpdId();
         stringstream ss, ssId;
         ssId << opId;
         xmlNodePtr wNode = (xmlNodePtr) NULL;

         if((m == 1) && (n == 1)) {
            wNode = xmlNewNode(NULL, BAD_CAST "WRITE_VAR");
            ss << *begin;
            xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());
            xmlNewProp(wNode, BAD_CAST "m", BAD_CAST "1");
            xmlNewProp(wNode, BAD_CAST "n", BAD_CAST "1");
         } else {
            wNode = xmlNewNode(NULL, BAD_CAST "WRITE_ARR");
            ss << "[";
            size_t nCnt = n, mCnt = m;
            for(CONST_FWD_IT_T<size_t> it = begin; it != end; ++it) {
                ss << *it;
                if(--nCnt == 0) {
                   nCnt = n;
                   mCnt--;
                   if(mCnt > 0)
                      ss << "; ";
                } else
                   ss << " ";
            }
            ss << "]";
            xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());

            stringstream mSz, nSz;
            mSz << m;
            nSz << n;
            xmlNewProp(wNode, BAD_CAST "m", BAD_CAST mSz.str().c_str());
            xmlNewProp(wNode, BAD_CAST "n", BAD_CAST nSz.str().c_str());
         }               
         xmlNewProp(wNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
         xmlNewProp(wNode, BAD_CAST "Type", BAD_CAST "TYPE_SIZET");
         xmlNewProp(wNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
         this->insertNode(opId, wNode);
       }
   //Write scalar
     //std::string
       void Offline_State::writeVar(std::string varName, std::string val) {
         size_t opId = this->getNewOpdId();

         stringstream ss, ssId;
         ssId << opId;
         xmlNodePtr wNode = xmlNewNode(NULL, BAD_CAST "WRITE_VAR");
         ss << "'" << val << "'";
         xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());
         xmlNewProp(wNode, BAD_CAST "m", BAD_CAST "1");
         xmlNewProp(wNode, BAD_CAST "n", BAD_CAST "1");
         xmlNewProp(wNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
         xmlNewProp(wNode, BAD_CAST "Type", BAD_CAST "TYPE_STRING");
         xmlNewProp(wNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
         this->insertNode(opId, wNode);
       }
     //Double
       void Offline_State::writeVar(std::string varName, double val) {
         size_t opId = this->getNewOpdId();

         stringstream ss, ssId;
         ssId << opId;
         xmlNodePtr wNode = xmlNewNode(NULL, BAD_CAST "WRITE_VAR");
         ss << std::setprecision(15) << val;
         xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());
         xmlNewProp(wNode, BAD_CAST "m", BAD_CAST "1");
         xmlNewProp(wNode, BAD_CAST "n", BAD_CAST "1");
         xmlNewProp(wNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
         xmlNewProp(wNode, BAD_CAST "Type", BAD_CAST "TYPE_DOUBLE");
         xmlNewProp(wNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
         this->insertNode(opId, wNode);
       }
     //Size_t
       void Offline_State::writeVar(std::string varName, size_t val) {
         size_t opId = this->getNewOpdId();

         stringstream ss, ssId;
         ssId << opId;
         xmlNodePtr wNode = xmlNewNode(NULL, BAD_CAST "WRITE_VAR");
         ss << val;
         xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());
         xmlNewProp(wNode, BAD_CAST "m", BAD_CAST "1");
         xmlNewProp(wNode, BAD_CAST "n", BAD_CAST "1");
         xmlNewProp(wNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
         xmlNewProp(wNode, BAD_CAST "Type", BAD_CAST "TYPE_SIZET");
         xmlNewProp(wNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
         this->insertNode(opId, wNode);
       }
   //Write array
     //Double
       void Offline_State::writeVar(std::string varName, size_t m, size_t n, const double *mat) {
         size_t opId = this->getNewOpdId();

         stringstream ss, ssId;
         ssId << opId;
         xmlNodePtr wNode = xmlNewNode(NULL, BAD_CAST "WRITE_ARR");
         ss << "[";
         for(size_t nCnt = 0, mCnt = 0; mCnt < m; ) {
             ss << std::setprecision(15) << mat[mCnt*n+nCnt];
             if(++nCnt == n) {
                nCnt = 0;
                mCnt++;
                if(mCnt != m)
                   ss << "; ";
             } else
                ss << " ";
         }
         ss << "]";
         xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());

         stringstream mSz, nSz;
         mSz << m;
         nSz << n;
         xmlNewProp(wNode, BAD_CAST "m", BAD_CAST mSz.str().c_str());
         xmlNewProp(wNode, BAD_CAST "n", BAD_CAST nSz.str().c_str());
         xmlNewProp(wNode, BAD_CAST "id", BAD_CAST ssId.str().c_str());
         xmlNewProp(wNode, BAD_CAST "Type", BAD_CAST "TYPE_DOUBLE"); 
         xmlNewProp(wNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
         this->insertNode(opId, wNode);
       }
     //Size_t
       void Offline_State::writeVar(std::string varName, size_t m, size_t n, const size_t *mat) {
         size_t opId = this->getNewOpdId();

         stringstream ss, ssId;
         ssId << opId;
         xmlNodePtr wNode = xmlNewNode(NULL, BAD_CAST "WRITE_ARR");
         ss << "[";
         for(size_t nCnt = 0, mCnt = 0; mCnt < m; ) {
             ss << mat[mCnt*n+nCnt];
             if(++nCnt == n) {
                nCnt = 0;
                mCnt++;
                if(mCnt != m)
                   ss << "; ";
             } else
                ss << " ";
         }
         ss << "]";
         xmlNewProp(wNode, BAD_CAST "Value", BAD_CAST ss.str().c_str());

         stringstream mSz, nSz;
         mSz << m;
         nSz << n;
         xmlNewProp(wNode, BAD_CAST "m", BAD_CAST mSz.str().c_str());
         xmlNewProp(wNode, BAD_CAST "n", BAD_CAST nSz.str().c_str());
         xmlNewProp(wNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
         xmlNewProp(wNode, BAD_CAST "Type", BAD_CAST "TYPE_SIZET");
         xmlNewProp(wNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
         this->insertNode(opId, wNode);
       }
   //Read var
     //Std::string
       std::string Offline_State::readStrVar(std::string varName) {
         //char *data = getResult(opId);
         //if(data == NULL) {
            size_t opId = this->getNewOpdId();

            stringstream ssId;
            ssId << opId;
            xmlNodePtr rNode = xmlNewNode(NULL, BAD_CAST "READ_VAR");
            xmlNewProp(rNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
            xmlNewProp(rNode, BAD_CAST "Type", BAD_CAST "TYPE_STRING");
            xmlNewProp(rNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
            this->insertNode(opId, rNode);

            stringstream ss;
            ss << "Unable to read string-variable '" << varName << "'";
            throw MatWlibException(ss.str());
         /*} else {
            std::string s = std::string(data);
            xmlFree(data);
            return s;
         }*/
       }
     //Double
       double Offline_State::readDoubleVar(std::string varName) {
         //char *data = getResult(opId);
         //if(data == NULL) {
            size_t opId = this->getNewOpdId();

            stringstream ssId;
            ssId << opId;
            xmlNodePtr rNode = xmlNewNode(NULL, BAD_CAST "READ_VAR");
            xmlNewProp(rNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
            xmlNewProp(rNode, BAD_CAST "Type", BAD_CAST "TYPE_DOUBLE");
            xmlNewProp(rNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
            if(xmlAddChild(this->getRootNode(), rNode) == NULL)
               throw MatWlibException("Unable to add read-var child");

            stringstream ss;
            ss << "Unable to read double-variable '" << varName << "'";
            throw MatWlibException(ss.str());
         /*} else {
            double d;
            try {
                 d = lexical_cast<double>(data);
            } catch(bad_lexical_cast &) {
                    throw MatWlibException("Unable to decode double.");
            }
            xmlFree(data);
            return d;
         }*/
       }
     //Size_t
       size_t Offline_State::readIntVar(std::string varName) {
         //char *data = getResult(opId);
         //if(data == NULL) {
            size_t opId = this->getNewOpdId();

            stringstream ssId;
            ssId << opId;
            xmlNodePtr rNode = xmlNewNode(NULL, BAD_CAST "READ_VAR");
            xmlNewProp(rNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
            xmlNewProp(rNode, BAD_CAST "Type", BAD_CAST "TYPE_SIZET");
            xmlNewProp(rNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
            if(xmlAddChild(this->getRootNode(), rNode) == NULL)
               throw MatWlibException("Unable to add read-var child");

            stringstream ss;
            ss << "Unable to read size_t-variable '" << varName << "'";
            throw MatWlibException(ss.str());
         /*} else {
            size_t i;
            try {
                 i = lexical_cast<size_t>(data);
            } catch(bad_lexical_cast &) {
                    throw MatWlibException("Unable to decode double.");
            }
            xmlFree(data);
            return i;
         }*/
       }

   //Read arr.
     //Double
       double *Offline_State::readDoubleArr(std::string varName, std::vector<size_t> &szDims) {
         //char *data = getResult(opId);
         //if(data == NULL) {
            size_t opId = this->getNewOpdId();

            stringstream ssId;
            ssId << opId;
            xmlNodePtr rNode = xmlNewNode(NULL, BAD_CAST "READ_ARR");
            xmlNewProp(rNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
            xmlNewProp(rNode, BAD_CAST "Type", BAD_CAST "TYPE_DOUBLE");
            xmlNewProp(rNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
            if(xmlAddChild(this->getRootNode(), rNode) == NULL)
               throw MatWlibException("Unable to add read-var child");

            stringstream ss;
            ss << "Unable to read double-array '" << varName << "'";
            throw MatWlibException(ss.str());
         /*} else {
            //Get matrix size
              xmlNodePtr resNode = getNodeResultMap().at(opId);  
              if( (xmlHasProp(resNode, (const xmlChar *) "m") == NULL) ||
                  (xmlHasProp(resNode, (const xmlChar *) "n") == NULL) )
                   throw MatWlibException("Result node for read-var is missing 'm' or 'n' attributes");
              xmlChar *mProp = xmlGetProp(resNode, (const xmlChar *) "m"),
                      *nProp = xmlGetProp(resNode, (const xmlChar *) "n");
              try {
                   szDims.push_back( lexical_cast<size_t>(mProp) );
                   szDims.push_back( lexical_cast<size_t>(nProp) );
              } catch(bad_lexical_cast &) {
                   throw MatWlibException("Invalid matrix size, expected integers.");
              }
              xmlFree(mProp);
              xmlFree(nProp);
            //Decode matrix
              double *mat = new double[szDims[1]*szDims[0]];
              for(size_t i = 0, beginTerm = 0, elemIndex = 0; i < strlen(data); i++) 
                  switch(data[i]) {
                   case '[':  beginTerm = i+1;  
                              break;
                   case ']':
                   case ';':  
                   case '\r':
                   case '\n':
                   case '\t':
                   case ' ':  if(i != beginTerm) {
                                 double d = 0.0;
                                 std::string substr(data+beginTerm, data+i);
                                 try {
                                      d = lexical_cast<double>(substr);
                                 } catch(bad_lexical_cast &) {
                                         throw MatWlibException("Unable to decode double.");
                                 }
                                 mat[elemIndex++] = d;
                              }
                              beginTerm = i+1;
                              break;
                   default:
                        break;
                  }
            xmlFree(data);
            return mat;
         }*/
       }
     //Size_t
       size_t *Offline_State::readIntArr(std::string varName, std::vector<size_t> &szDims) {
         //char *data = getResult(opId);
         //if(data == NULL) {
            size_t opId = this->getNewOpdId();

            stringstream ssId;
            ssId << opId;
            xmlNodePtr rNode = xmlNewNode(NULL, BAD_CAST "READ_ARR");
            xmlNewProp(rNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
            xmlNewProp(rNode, BAD_CAST "Type", BAD_CAST "TYPE_SIZET");
            xmlNewProp(rNode, BAD_CAST "VarName", BAD_CAST varName.c_str()); 
            if(xmlAddChild(this->getRootNode(), rNode) == NULL)
               throw MatWlibException("Unable to add read-var child");

            stringstream ss;
            ss << "Unable to read size_t-array '" << varName << "'";
            throw MatWlibException(ss.str());
         /*} else {
            //Get matrix size
              xmlNodePtr resNode = getNodeResultMap().at(opId);  
              if( (xmlHasProp(resNode, (const xmlChar *) "m") == NULL) ||
                  (xmlHasProp(resNode, (const xmlChar *) "n") == NULL) )
                   throw MatWlibException("Result node for read-var is missing 'm' or 'n' attributes");
              xmlChar *mProp = xmlGetProp(resNode, (const xmlChar *) "m"),
                      *nProp = xmlGetProp(resNode, (const xmlChar *) "n");
              try {
                   szDims.push_back( lexical_cast<size_t>(mProp) );
                   szDims.push_back( lexical_cast<size_t>(nProp) );
              } catch(bad_lexical_cast &) {
                   throw MatWlibException("Invalid matrix size, expected integers.");
              }
              xmlFree(mProp);
              xmlFree(nProp);
            //Decode matrix
              size_t *mat = new size_t[szDims[1]*szDims[0]];
              for(size_t i = 0, beginTerm = 0, elemIndex = 0; i < strlen(data); i++) 
                  switch(data[i]) {
                   case '[':  beginTerm = i+1;  
                              break;
                   case ']':
                   case ';':  
                   case '\r':
                   case '\n':
                   case '\t':
                   case ' ':  if(i != beginTerm) {
                                 size_t i = 0;
                                 std::string substr(data+beginTerm, data+i);
                                 try {
                                      i = lexical_cast<size_t>(substr);
                                 } catch(bad_lexical_cast &) {
                                         throw MatWlibException("Unable to decode double.");
                                 }
                                 mat[elemIndex++] = i;
                              }
                              beginTerm = i+1;
                              break;
                   default:
                        break;
                  }
            xmlFree(data);
            return mat;
         }*/
       }
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////

   //Function handling
     bool Offline_State::registerFunction(std::string funcName, Function *funcDesc) {
       if(funcMap.find(funcName) != funcMap.end())
          return false;
       funcMap.insert( std::make_pair(funcName, funcDesc) );
       return true;
     }
     bool Offline_State::updateFunction(std::string funcName, Function::FUNC_T fn) {
       std::map<std::string, Function *>::iterator it = funcMap.find(funcName);
       if(it == funcMap.end())
          return false;
       it->second->funcHandler = fn;
       return true;
     }
     bool Offline_State::removeFunction(std::string funcName) {
       std::map<std::string, Function *>::iterator it = funcMap.find(funcName);
       if(it == funcMap.end())
          return false;
       delete it->second;
       funcMap.erase(it);
       return true;
     }
   //Value handling
     Value *Offline_State::decodeData(Data *d) {
         return NULL;
     }
   //Variable handling
     void Offline_State::writeVar(std::string varName, Data *d) {

     }
     Data *Offline_State::readVar(std::string varName) {
         return NULL;
     }
     void Offline_State::writeVar(std::string varName, RawDataInterface<NAME>::CELL_FUNC_T fn) {
  
     }
     void Offline_State::readVar(std::string varName, RawDataInterface<NAME>::CELL_FUNC_T fn) {

     }

   //Execute
     void Offline_State::execute(std::string cmd) {
         size_t opId = getNewOpdId();

         stringstream ssId;
         ssId << opId;
         xmlNodePtr eNode = xmlNewNode(NULL, BAD_CAST "EXECUTE");
         xmlNewProp(eNode, BAD_CAST "id", BAD_CAST ssId.str().c_str()); 
         xmlNewProp(eNode, BAD_CAST "Cmd", BAD_CAST cmd.c_str()); 
         if(xmlAddChild(getRootNode(), eNode) == NULL)
            throw MatWlibException("Unable to add execute child");
     }
}

