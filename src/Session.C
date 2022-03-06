#include <sstream>
#include <vector>
#include <string.h>
#include "assert.h"

#include "mathinterfacelib.h"


using namespace std;

namespace MathInterfaceLib {
//Typed sessions
  //Double
    template<> TypedSessionCmd<double>::TypedSessionCmd(std::string varName_, double &var_, size_t opId_, SessionCmd::MAT_SIZE_VAR_T *sz_)
         : SessionCmd(opId_, SessionCmd::CMD_READ, SessionCmd::TYPE_DOUBLE, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<double>::TypedSessionCmd(std::string varName_, double &var_, size_t opId_, SessionCmd::MAT_SIZE_CONST_T &sz_)
         : SessionCmd(opId_, SessionCmd::CMD_WRITE, SessionCmd::TYPE_DOUBLE, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<double>::~TypedSessionCmd() {}

    template<> TypedSessionCmd<double *>::TypedSessionCmd(std::string varName_, double *&var_, size_t opId_, SessionCmd::MAT_SIZE_VAR_T *sz_)
         : SessionCmd(opId_, SessionCmd::CMD_READ, SessionCmd::TYPE_DOUBLE, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<double *>::TypedSessionCmd(std::string varName_, double *&var_, size_t opId_, SessionCmd::MAT_SIZE_CONST_T &sz_)
         : SessionCmd(opId_, SessionCmd::CMD_WRITE, SessionCmd::TYPE_DOUBLE, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<double *>::~TypedSessionCmd() {}
  //String
    template<> TypedSessionCmd<std::string>::TypedSessionCmd(std::string varName_, std::string &var_, size_t opId_, SessionCmd::MAT_SIZE_VAR_T *sz_)
         : SessionCmd(opId_, SessionCmd::CMD_READ, SessionCmd::TYPE_STRING, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<std::string>::TypedSessionCmd(std::string varName_, std::string &var_, size_t opId_, SessionCmd::MAT_SIZE_CONST_T &sz_)
         : SessionCmd(opId_, SessionCmd::CMD_WRITE, SessionCmd::TYPE_STRING, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<std::string>::~TypedSessionCmd() {}

    template<> TypedSessionCmd<std::string *>::TypedSessionCmd(std::string varName_, std::string *&var_, size_t opId_, SessionCmd::MAT_SIZE_VAR_T *sz_)
         : SessionCmd(opId_, SessionCmd::CMD_READ, SessionCmd::TYPE_STRING, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<std::string *>::TypedSessionCmd(std::string varName_, std::string *&var_, size_t opId_, SessionCmd::MAT_SIZE_CONST_T &sz_)
         : SessionCmd(opId_, SessionCmd::CMD_WRITE, SessionCmd::TYPE_STRING, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<std::string *>::~TypedSessionCmd() {}
  //Size_t
    template<> TypedSessionCmd<size_t>::TypedSessionCmd(std::string varName_, size_t &var_, size_t opId_, SessionCmd::MAT_SIZE_VAR_T *sz_)
         : SessionCmd(opId_, SessionCmd::CMD_READ, SessionCmd::TYPE_SIZET, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<size_t>::TypedSessionCmd(std::string varName_, size_t &var_, size_t opId_, SessionCmd::MAT_SIZE_CONST_T &sz_)
         : SessionCmd(opId_, SessionCmd::CMD_WRITE, SessionCmd::TYPE_SIZET, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<size_t>::~TypedSessionCmd() {}

    template<> TypedSessionCmd<size_t *>::TypedSessionCmd(std::string varName_, size_t *&var_, size_t opId_, SessionCmd::MAT_SIZE_VAR_T *sz_)
         : SessionCmd(opId_, SessionCmd::CMD_READ, SessionCmd::TYPE_SIZET, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<size_t *>::TypedSessionCmd(std::string varName_, size_t *&var_, size_t opId_, SessionCmd::MAT_SIZE_CONST_T &sz_)
         : SessionCmd(opId_, SessionCmd::CMD_WRITE, SessionCmd::TYPE_SIZET, sz_), varName(varName_), var(var_) {}
    template<> TypedSessionCmd<size_t *>::~TypedSessionCmd() {}

//Session execute
  SessionErrorReport SessionInterface::session(std::vector<SessionCmd *> cmds) {
      SessionErrorReport err;
      /*CommonDataInterface<decltype(*this)> cdi(*this);

      for(std::vector<SessionCmd *>::iterator itCmd = cmds.begin(); itCmd != cmds.end(); ++itCmd) {
          SessionCmd *cmd = *itCmd;
          bool isArrayOp;
          switch(cmd->c) {
           case SessionCmd::CMD_READ:
               switch(cmd->t) {
                case SessionCmd::TYPE_DOUBLE:
                     if(cmd->szVar == NULL)
                        isArrayOp = false;
                     else if((cmd->szVar->first > 1) || (cmd->szVar->second > 1))
                        isArrayOp = true;
                     else
                        isArrayOp = false;

                     if(!isArrayOp) {
                        TypedSessionCmd<double> *tcmd = dynamic_cast<TypedSessionCmd<double> *>(cmd);
                        if(tcmd == NULL)
                           throw MathInterfacelibException("Invalid session struct type for read double var, expected TypedSessionCmd<double>");
                        try {
                            tcmd->var = cdi.template readVar<double>(tcmd->opId, s, tcmd->varName);
                        } catch(MathInterfacelibException e) {
                            err.errorId.insert(cmd->opId);
                        }
                     } else {
                        TypedSessionCmd<double *> *tcmd = dynamic_cast<TypedSessionCmd<double *> *>(cmd);
                        if(tcmd == NULL)
                           throw MathInterfacelibException("Invalid session struct type for read double arr, expected TypedSessionCmd<double *>");
                        try {
                            tcmd->var = cdi.template readVar<double>(tcmd->opId, s, tcmd->varName, tcmd->szVar->first, tcmd->szVar->second);
                        } catch(MathInterfacelibException e) {
                            err.errorId.insert(cmd->opId);
                        } 
                     } break;
                case SessionCmd::TYPE_STRING:
                     if(cmd->szVar == NULL)
                        isArrayOp = false;
                     else if((cmd->szVar->first > 1) || (cmd->szVar->second > 1))
                        isArrayOp = true;
                     else
                        isArrayOp = false;

                     if(!isArrayOp) {
                        TypedSessionCmd<std::string> *tcmd = dynamic_cast<TypedSessionCmd<std::string> *>(cmd);
                        if(tcmd == NULL)
                           throw MathInterfacelibException("Invalid session struct type for read string, expected TypedSessionCmd<string>");
                        try {
                            tcmd->var = cdi.template readVar<std::string>(tcmd->opId, s, tcmd->varName);
                        } catch(MathInterfacelibException e) {
                            err.errorId.insert(cmd->opId);
                        } 
                     } else {
                        assert(false && "NOT IMPLEMENTED");
                        //tcmd->var = readVar<std::string>(tcmd->opId, s, tcmd->varName, tcmd->szVar->first, tcmd->szVar->second);
                     } break;
                case SessionCmd::TYPE_SIZET:
                     if(cmd->szVar == NULL)
                        isArrayOp = false;
                     else if((cmd->szVar->first > 1) || (cmd->szVar->second > 1))
                        isArrayOp = true;
                     else
                        isArrayOp = false;

                     if(!isArrayOp) {
                        TypedSessionCmd<size_t> *tcmd = dynamic_cast<TypedSessionCmd<size_t> *>(cmd);
                        if(tcmd == NULL)
                           throw MathInterfacelibException("Invalid session struct type for read size_t var, expected TypedSessionCmd<size_t>");
                        try {
                            tcmd->var = cdi.template readVar<size_t>(tcmd->opId, s, tcmd->varName);
                        } catch(MathInterfacelibException e) {
                            err.errorId.insert(cmd->opId);
                        } 
                     } else {
                        TypedSessionCmd<size_t *> *tcmd = dynamic_cast<TypedSessionCmd<size_t *> *>(cmd);
                        if(tcmd == NULL)
                           throw MathInterfacelibException("Invalid session struct type for read size_t arr, expected TypedSessionCmd<size_t *>");
                        try {
                            tcmd->var = cdi.template readVar<size_t>(tcmd->opId, s, tcmd->varName, tcmd->szVar->first, tcmd->szVar->second);
                        } catch(MathInterfacelibException e) {
                            err.errorId.insert(cmd->opId);
                        } 
                     } break;
                default:
                   throw MathInterfacelibException("Unhandled case in session::CMD_READ()");
               } break;
           case SessionCmd::CMD_WRITE:
               switch(cmd->t) {
                case SessionCmd::TYPE_DOUBLE:
                     if(cmd->szConst == NULL)
                        isArrayOp = false;
                     else if((cmd->szConst->first > 1) || (cmd->szConst->second > 1))
                        isArrayOp = true;
                     else
                        isArrayOp = false;

                     if(!isArrayOp) {
                        TypedSessionCmd<double> *tcmd = dynamic_cast<TypedSessionCmd<double> *>(cmd);
                        if(tcmd == NULL)
                           throw MathInterfacelibException("Invalid session struct type for write double var, expected TypedSessionCmd<double>");
                        cdi.template writeVar<double>(tcmd->opId, s, tcmd->varName, tcmd->var);
                     } else {
                        TypedSessionCmd<double *> *tcmd = dynamic_cast<TypedSessionCmd<double *> *>(cmd);
                        if(tcmd == NULL)
                           throw MathInterfacelibException("Invalid session struct type for write double arr, expected TypedSessionCmd<double *>");
                        cdi.template writeVar<double>(tcmd->opId, s, tcmd->varName, tcmd->szConst->first, tcmd->szConst->second, tcmd->var);
                     } break;
                case SessionCmd::TYPE_STRING:
                     if(cmd->szConst == NULL)
                        isArrayOp = false;
                     else if((cmd->szConst->first > 1) || (cmd->szConst->second > 1))
                        isArrayOp = true;
                     else
                        isArrayOp = false;

                     if(!isArrayOp) {
                        //TypedSessionCmd<std::string> *tcmd = dynamic_cast<TypedSessionCmd<std::string> *>(cmd);
                        assert(false && "NOT IMPLEMENTED");
                        //cmd.var = writeVar<std::string>(tcmd->opId, s, cmd.varName);
                     } else {
                        //TypedSessionCmd<std::string *> *tcmd = dynamic_cast<TypedSessionCmd<std::string *> *>(cmd);
                        assert(false && "NOT IMPLEMENTED");
                        //cmd.var = readVar<std::string>(tcmd->opId, s, cmd.varName, cmd.szConst->first, cmd.szConst->second);
                     } break;
                case SessionCmd::TYPE_SIZET:
                     if(cmd->szConst == NULL)
                        isArrayOp = false;
                     else if((cmd->szConst->first > 1) || (cmd->szConst->second > 1))
                        isArrayOp = true;
                     else
                        isArrayOp = false;

                     if(!isArrayOp) {
                        TypedSessionCmd<size_t> *tcmd = dynamic_cast<TypedSessionCmd<size_t> *>(cmd);
                        if(tcmd == NULL)
                           throw MathInterfacelibException("Invalid session struct type for write size_t var, expected TypedSessionCmd<size_t>");
                        cdi.template writeVar<size_t>(tcmd->opId, s, tcmd->varName, tcmd->var);
                     } else {
                        TypedSessionCmd<size_t *> *tcmd = dynamic_cast<TypedSessionCmd<size_t *> *>(cmd);
                        if(tcmd == NULL)
                           throw MathInterfacelibException("Invalid session struct type for write size_t arr, expected TypedSessionCmd<size_t *>");
                        cdi.template writeVar<size_t>(tcmd->opId, s, tcmd->varName, tcmd->szConst->first, tcmd->szConst->second, tcmd->var);
                     } break;
                default:
                   throw MathInterfacelibException("Unhandled case in session::CMD_READ()");
               } break;
           case SessionCmd::CMD_EXEC:
               {
                  ExecSessionCmd *tcmd = dynamic_cast<ExecSessionCmd *>(cmd);
                  this->execute(tcmd->opId, tcmd->cmd);
               };
          }
      }*/
      return err;
  }
}
