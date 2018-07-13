// Copyright (C) Rishabh Iyer, John T. Halloran, and Kai Wei
// Licensed under the Open Software License version 3.0
// See COPYING or http://opensource.org/licenses/OSL-3.0
/*
  Authors: Rishabh Iyer and John Halloran
  *
  */

#include <iostream>
#include <cstdlib>
#include <string>
#include <time.h>
#include "../src/jensen.h"
#include "argVariables.h"
using namespace jensen;
using namespace std;

template <class Feature>
double predictAccuracy(Classifiers<Feature>* c, vector<Feature>& testFeatures, Vector& ytest){
  assert(testFeatures.size() == ytest.size());
  double accuracy = 0;
  for (int i = 0; i < testFeatures.size(); i++){
    if (c->predict(testFeatures[i]) == ytest[i])
      accuracy++;
  }
  return accuracy;
}

int main(int argc, char** argv){
  bool parse_was_ok = Arg::parse(argc,(char**)argv);
  if(!parse_was_ok){
    Arg::usage(); exit(-1);
  }

  double accuracy_percentage = 0;
  double average_accuracy = 0;
  double average_squared_accuracy = 0;
  int num_test = 0;

  for(int k = 0; k < kfold; k++){
    int ntrain; // number of data items in the training set
    int mtrain; // numFeatures of the training data
    int ntest; // number of data items in the test set
    int mtest; // numFeatures of the test data
    // float percentTrain = 0.5;
    vector<struct SparseFeature> trainFeatures, testFeatures;
    Vector ytrain, ytest;

    readFeatureVectorSparseCrossValidate(trainFeatureFile, trainLabelFile,
					 ntrain, mtrain, percentTrain,
					 trainFeatures, testFeatures, ytrain, ytest);
    if (startwith1){
      ytrain = ytrain - 1;
      ytest = ytest - 1;
    }

    num_test = ytest.size();

    cout << "Done reading the file, the size of the training set is " << ytrain.size() <<
      " and the size of the test set is " << ytest.size() << endl;

    if ((method < 0) || (method > 5)){
      cout << "Invalid method.\n";
      return -1;
    }

    cout << "Now training a " << algs[method-1] << " classifier.\n";
    double accuracy = 0;
    Classifiers<SparseFeature>* c;

    if (method == L1LR) {
      c = new L1LogisticRegression<SparseFeature>(trainFeatures, ytrain, mtrain, ntrain, nClasses,
						  lambda, algtype, maxIter, eps);
    } else if (method == L2LR) {
      c = new L2LogisticRegression<SparseFeature>(trainFeatures, ytrain, mtrain, ntrain, nClasses,
						  lambda, algtype, maxIter, eps);
    } else if (method == L1SSVM) {
      c = new L1SmoothSVM<SparseFeature>(trainFeatures, ytrain, mtrain, ntrain, nClasses,
					 lambda, algtype, maxIter, eps);
    } else if (method == L2SSVM) {
      c = new L2SmoothSVM<SparseFeature>(trainFeatures, ytrain, mtrain, ntrain, nClasses,
					 lambda, algtype, maxIter, eps);
    } else if (method == L2HSVM) {
      c = new L2HingeSVM<SparseFeature>(trainFeatures, ytrain, mtrain, ntrain, nClasses,
					lambda, algtype, maxIter, eps);
    } else {
      cout << "Invalid mode\n";
      return -1;
    }
    c->train();
    cout << k << ": Done with Training ... now testing\n";
    accuracy = predictAccuracy(c, testFeatures, ytest);
    delete c;

    accuracy_percentage += accuracy/ytest.size();
    cout << k << ": Accuracy = " << accuracy/ytest.size() << "\n";
    average_accuracy += accuracy/ytest.size();
    average_squared_accuracy += (accuracy/ytest.size()) * (accuracy/ytest.size());
  }
  accuracy_percentage /= (float)kfold; // average over k cross-validations

  average_accuracy /= (float)kfold;
  average_squared_accuracy /= (float)kfold;

  cout << kfold << "-fold cross validation classification accuracy "<< accuracy_percentage << " (" << num_test
       << "test instances), variance="
       << average_squared_accuracy -  average_accuracy * average_accuracy << "\n";
  // cout << "The acuracy of the classifier is "<< accuracy_percentage << "("<< accuracy << "/"<< ytest.size()
  //      << ")" << "\n";
}