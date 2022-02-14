#ifndef OFFLINE_HDR
#define OFFLINE_HDR

#include <string>
#include <map>

#include "matwlib.h"

#define USE_XML

#ifdef USE_XML
   #include <libxml/parser.h>
   #include <libxml/tree.h>
#endif

namespace MatWLib {

   class Offline_State : public State,
                         public FunctionInterface,
                         public SessionInterface,
                         public RawDataInterface<std::string>,
                         public RawExecuteInterface {
      public:
         typedef std::string NAME;
      private:
         size_t opId;
         bool docChanged;
         std::string fName;
         #ifdef USE_XML
            xmlDocPtr doc;
            xmlNodePtr rootNode;
         #endif

         std::map<size_t, xmlNodePtr> nodeIdMap, resultNodeMap;
         std::map<std::string, Function *> funcMap;
      public:
         #ifdef USE_XML
          //(De/Con)structor(s)
            Offline_State(bool docChanged_, std::string fName_, xmlDocPtr doc_, xmlNodePtr rootNode_, 
                          std::map<size_t, xmlNodePtr> &nodeIdMap_, std::map<size_t, xmlNodePtr> &resultNodeMap_);
            Offline_State(bool docChanged_, std::string fName_, xmlDocPtr doc_, xmlNodePtr rootNode_);
            virtual ~Offline_State();
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
           size_t getNewOpdId();

           virtual bool good() const;
           bool hasDocChanged() const;
           xmlNodePtr getRootNode();
           xmlDocPtr getDoc();
           void insertNode(size_t opId, xmlNodePtr node);
           char *getResult(size_t opId);
           void addResult(size_t opId, xmlNodePtr node);
           void remResult(size_t opId);
           std::map<size_t, xmlNodePtr> &getNodeMap();
           std::map<size_t, xmlNodePtr> &getNodeResultMap();
         #endif
         std::string getFName() const;
         std::string toString() const;
         //Function handling
           bool registerFunction(std::string funcName, Function *funcDesc);
           bool updateFunction(std::string funcName, Function::FUNC_T fn);
           bool removeFunction(std::string funcName);
         //Value handling
           virtual Value *decodeData(Data *d);
         //Variable handling
           virtual void writeVar(std::string varName, Data *d);
           virtual Data *readVar(std::string varName);
           virtual void writeVar(std::string varName, RawDataInterface<std::string>::CELL_FUNC_T fn);
           virtual void readVar(std::string varName, RawDataInterface<std::string>::CELL_FUNC_T fn);

           /*template<class IT> void writeVar(std::string varName, size_t m, size_t n, IT begin, IT end);
           template<class T> void writeVar(std::string varName, T val);
           template<class T> void writeVar(std::string varName, size_t m, size_t n, const T *mat);
 
           template<class T> T readVar(std::string varName);
           template<class T> T *readVar(std::string varName, std::vector<size_t> &szDims);   */
         //Command handling
           void execute(std::string cmd);
   };

   /*template<> struct DataInterface<Offline_State> {
    private:
      Offline_State &s;
    public:
    //(De/Con)structor(s)
      DataInterface(Offline_State &s_);
      ~DataInterface();
    //Write data
      template<class T> void writeVar(const Offline_State::NAME varName, T val);
      template<class T> void writeVar(const Offline_State::NAME varName, size_t m, size_t n, const T *mat);  
      template<class IT> void writeVar(const Offline_State::NAME varName, size_t m, size_t n, IT begin, IT end);
    //Read data
      template<class T> T readVar(const Offline_State::NAME varName);
      template<class T> T *readVar(const Offline_State::NAME varName, std::vector<size_t> &szDims);
      //template<class IT> void readVar(const Offline_State::NAME varName, std::vector<size_t> &szDims, IT begin, IT end);
   };*/


   Offline_State *init_offline_engine(std::string path);
   void finit_offline_engine(Offline_State *s);
}

#endif

