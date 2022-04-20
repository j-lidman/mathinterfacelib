
int cvx_test(State *s) {
   FunctionInterface *sfi = dynamic_cast<FunctionInterface *>(s);
   RawExecuteInterface *sei = dynamic_cast<RawExecuteInterface *>(s);
   RawDataInterface<std::string> *sdi = dynamic_cast<RawDataInterface<std::string> *>(s);
   assert(sfi != NULL);
   assert(sei != NULL);
   assert(sdi != NULL);
   //CVX Minimum volume ellipse
    
     //Enter commands
       
   //Read results

   return 0;
};

int main(int argc, char *argv[]) {
   int rc;
   //Matlab
     #if defined(HAS_MATLAB)
        cout << "Testing MATLAB engine..." << endl;
        State *s = init_engine(MatWLib::ENGINE_MATLAB, MATLAB_PATH "/bin/glnxa64");
        rc = cvx_test<MATLAB_State>(s, false);
        if(rc != 0)
           return rc;
        rc = function_test(s);
        if(rc != 0)
           return rc;
        finit_engine(s);
     #endif
   //Octave

   /*//Offline
     cout << "Testing Offline engine (using 'test_lib.xml')..." << endl;
     State *os = init_engine(ENGINE_OFFLINE, "test_lib.xml");
     rc = basic_test(os, true);
     finit_engine(os);
     if(rc != 0)
        return rc;*/
   cout << "Test done!" << endl;
   return 0;
  return 0;
}
