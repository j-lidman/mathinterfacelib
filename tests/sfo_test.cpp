
#define NR_POINTS 5
#define NR_CLUSTERS 3
double points[NR_POINTS][2] = {{0,0}, {5,1}, {3,5}, {15,15}, {2,3}};
size_t queries = 0;
set<vector<double> > queryHist;
bool sfo_clustering_rect(const std::vector<Value *> &inputValues,std::vector<Value *> &outputValues) {
   Value_Matrix *inMat= dynamic_cast<Value_Matrix *>(inputValues.at(0));
   if((inMat == NULL) || (inMat->mat.size() != 1) )
      return false;

   queries++;
   vector<double> q;

   if(inMat->mat.at(0)->size() == 0) {
      outputValues.push_back( new Value_Scalar<double>(0.0) );
      queryHist.insert(q);
      return true;
   } else {
      size_t index = (size_t) getValue<double>(inMat->mat.at(0)->at(0))-1;
      q.push_back(index);
      //<xmin, xmax>, <ymin, ymax>
      double rect[2][2] = {{points[index][0], points[index][0]},
                           {points[index][1], points[index][1]}};
      for(size_t c = 1; c < inMat->mat.at(0)->size(); c++) {
          index = (size_t) getValue<double>(inMat->mat.at(0)->at(c))-1;
          q.push_back(index);
          for(size_t d = 0; d < 2; d++) {
              if(rect[d][0] > points[index][d])
                 rect[d][0] = points[index][d];
              if(rect[d][1] < points[index][d])
                 rect[d][1] = points[index][d];
          }
      }
      double circum = 0;
      for(size_t d = 0; d < 2; d++)
          circum += (rect[d][1] - rect[d][0]);
      if(circum == 0.0)
         outputValues.push_back( new Value_Scalar<double>(0.0) );
      else         
         outputValues.push_back( new Value_Scalar<double>(1.0/circum) );
      queryHist.insert(q);
      return true;
   }
}

bool createRect(const std::vector<Value *> &inputValues,std::vector<Value *> &outputValues) {
   Value_Matrix *inMat= dynamic_cast<Value_Matrix *>(inputValues.at(0));
   if((inMat == NULL) || (inMat->mat.size() != 1) )
      return false;
   size_t index = (size_t) getValue<double>(inMat->mat.at(0)->at(0))-1;
   //<xmin, xmax>, <ymin, ymax>
   double rect[2][2] = {{points[index][0], points[index][0]},
                        {points[index][1], points[index][1]}};
   for(size_t c = 1; c < inMat->mat.at(0)->size(); c++) {
       index = (size_t) getValue<double>(inMat->mat.at(0)->at(c))-1;
       for(size_t d = 0; d < 2; d++) {
           if(rect[d][0] > points[index][d])
              rect[d][0] = points[index][d];
           if(rect[d][1] < points[index][d])
              rect[d][1] = points[index][d];
       }
   }
   Value_Matrix *out = new Value_Matrix();
   out->addElementToLastRow(new Value_Scalar<double>(rect[0][0]));
   out->addElementToLastRow(new Value_Scalar<double>(rect[1][0]));
   out->addElementToLastRow(new Value_Scalar<double>(rect[0][1]-rect[0][0]));
   out->addElementToLastRow(new Value_Scalar<double>(rect[1][1]-rect[1][0]));
   outputValues.push_back(out);
   return true;
}

bool getPoint(const std::vector<Value *> &inputValues,std::vector<Value *> &outputValues) {
   Value_Matrix *inMat= dynamic_cast<Value_Matrix *>(inputValues.at(0));
   if((inMat == NULL) || (inMat->mat.size() != 1) )
      return false;
   size_t index = (size_t) getValue<double>(inMat->mat.at(0)->at(0))-1;
   Value_Matrix *out = new Value_Matrix();
   out->addElementToLastRow(new Value_Scalar<double>(points[index][0]));
   out->addElementToLastRow(new Value_Scalar<double>(points[index][1]));
   outputValues.push_back(out);
   return true;
}

int sfo_test(State *s) {
   FunctionInterface *sfi = dynamic_cast<FunctionInterface *>(s);
   RawExecuteInterface *sei = dynamic_cast<RawExecuteInterface *>(s);
   RawDataInterface<std::string> *sdi = dynamic_cast<RawDataInterface<std::string> *>(s);
   assert(sfi != NULL);
   assert(sei != NULL);
   assert(sdi != NULL);
   //SFO Clustering problem
     vecInputs.push_back( new Function::Argument_Array(new Function::Argument_Element(TYPE_DOUBLE)) );
     vecOutputs.push_back( new Function::Argument_Element(TYPE_DOUBLE) );
     assert(sfi->registerFunction("wClustering", new Function(sfo_clustering_rect, vecInputs, vecOutputs)) && "Unable to register 'wClustering'");
     vecInputs.clear();
     vecOutputs.clear();

     vecInputs.push_back( new Function::Argument_Array(new Function::Argument_Element(TYPE_DOUBLE)) );
     vecOutputs.push_back( new Function::Argument_Array(new Function::Argument_Element(TYPE_DOUBLE)) );
     assert(sfi->registerFunction("createRect", new Function(createRect, vecInputs, vecOutputs)) && "Unable to register 'createRect'");
     vecInputs.clear();
     vecOutputs.clear();

     vecInputs.push_back( new Function::Argument_Array(new Function::Argument_Element(TYPE_DOUBLE)) );
     vecOutputs.push_back( new Function::Argument_Array(new Function::Argument_Element(TYPE_DOUBLE)) );
     assert(sfi->registerFunction("getPoint", new Function(getPoint, vecInputs, vecOutputs)) && "Unable to register 'getPoint'");
     vecInputs.clear();
     vecOutputs.clear();
     //Plot points
       std::stringstream cmd;
       cmd << "figure; ";
       for(size_t i = 0; i < NR_POINTS; i++)
           cmd << "plot(" << points[i][0] << ", " << points[i][1] << ", 'bx'); hold on;" << endl;
     //Enter commands
       cmd << "V = 1:" << NR_POINTS << ";" << endl
           << "w = @(S) mathinterfacelibFunc('wClustering', S);" << endl
           << "f = kPPHT2(V, w);" << endl
           << "P = sfo_greedy_splitting(f, V, " << NR_CLUSTERS << ")" << endl
           << "colorMap = lines(size(P, 2));" << endl
           << "for i = 1:size(P,2)" << endl
           << "   if size(P{1,i}, 2) == 1" << endl
           << "      p = mathinterfacelibFunc('getPoint', P{1,i});" << endl
           << "      plot(p(1), p(2), 'marker', 'x', 'color', colorMap(i,:));" << endl
           << "   else" << endl
           << "      rectangle('Position', mathinterfacelibFunc('createRect', P{1,i}), 'FaceColor', colorMap(i,:)); hold on;" << endl
           << "   end" << endl
           << "end" << endl
           << "waitforbuttonpress;" << endl;
     sei->execute(cmd.str());
   //Read results
     vector<vector<size_t> *> pointSets;
     for(size_t i = 0; i < NR_CLUSTERS; i++)
         pointSets.push_back( new vector<size_t>() );

     std::function<bool(size_t, size_t *, TYPE, Data *)> funcStructPopulate =
      [s, sdi, &pointSets](size_t nrDims, size_t *elemPos, TYPE elemType, Data *d) mutable {
        if((nrDims != 2) || (elemPos[0] != 0))
           return false;
        vector<size_t> *pos = pointSets.at(elemPos[1]);
        Value_RawMatrix<double> *m = dynamic_cast<Value_RawMatrix<double> *>(sdi->decodeData(d));
        if((m == NULL) || (m->nDim != 2) || (m->dimSz[0] != 1))
           return false;
        for(size_t i = 0; i < m->dimSz[1]; i++)
            pos->push_back( ((size_t) m->val[i])-1);
        delete m;
        return true;
      };
      sdi->readVar("P", funcStructPopulate);

   cout << "SFO Info: Queries (" << queries << "/" << ((1<<NR_POINTS)-1) << "), Unique Queries (" << queryHist.size() << ")" << endl;
   cout << "SFO Results {";
   for(size_t i = 0; i < pointSets.size(); i++) {
       vector<size_t> *v = pointSets[i];
       cout << (i == 0 ? "" : ",") << "[";
       for(size_t j = 0; j < v->size(); j++)
           cout << (j == 0 ? "" : ",") << v->at(j);
       cout << "]";
   }
   cout << "}" << endl;
   return 0;
};

int main(int argc, char *argv[]) {
   int rc;
   //Matlab
     #if defined(HAS_MATLAB)
        cout << "Testing MATLAB engine..." << endl;
        State *s = init_engine(MathInterfaceLib::ENGINE_MATLAB, MATLAB_PATH "/bin/glnxa64");
        rc = sfo_test<MATLAB_State>(s, false);
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
}

