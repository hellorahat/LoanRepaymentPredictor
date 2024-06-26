#ifndef RANDOMFOREST_H
#define RANDOMFOREST_H

#include "DecisionTree.h"
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
#include <unordered_set>

using namespace std;

/**
  *@file RandomForest.h
  *@brief Header file for the RandomForest class.
  *Contian both declaration and implementation.
  *
  *This class encapsulates a collection of decision trees to form a random forest.
  *It provides methods to train the forest using bagging and to predict class labels using majority voting among the trees,
  *and to perform k-fold cross validation.
  */

class RandomForest{
public:
  /**
    *@brief Constructor that initializes the forest with a specified number of trees.
    *@param num_trees Number of trees to include in the forest.
    */
  RandomForest(int num_trees): num_trees_(num_trees), trees_(num_trees), rng(random_device{}()){}          //Constructor initializes the forest with a specified number of trees
  /**
    *@brief Trains the random forest using the provided dataset.
    *@param data_vec Data used for training the random forest. Each tree is train on a bootstrap sample of this data.
    */
  void train (const vector<vector<double>>& data_vec){
    cout<<"Starting training process..." << endl;
    vector<vector<double>> train_data, test_data;
    splitData(data_vec, train_data, test_data, 0.2);
    cout<< "Data split into " << train_data.size() <<" training sample and " << test_data.size() <<" test samples." << endl;

    cout <<"Training "<<num_trees_ << " trees with bagging..." << endl;
    uniform_int_distribution<> dist(0, data_vec.size() - 1);
    mt19937 rng(random_device{}());

    for (int i = 0; i < num_trees_; i++){
      cout<<"Training Decision Tree " << (i + 1) << " with all feature using bootstrap samples..." << endl;
      vector<vector<double>> bootstrap_sample = createBootstrapSample(train_data);
      cout << "Bootstrap sample size: " << bootstrap_sample.size() << endl;
      trees_[i].train(bootstrap_sample);
  }
}
/**
  *@brief Predict the class label for the given feature using majority voting among all trees.
  *@param feature Vcetor of feature for which the class label is predicted.
  *@return The predicted class label, determined by majority vote.
  */

int predict(const vector<double>& feature){
  map<int, int> vote_count;
  for (auto& tree : trees_){
    int prediction = tree.predict(feature);
    vote_count[prediction]++;
  }
  
  int majorityVote = -1;
  int maxCount = 0;
  for (const auto& vote : vote_count){
    if (vote.second > maxCount){
      maxCount = vote.second;
      majorityVote = vote.first;
    }
  }
  return majorityVote;
}

/**
 * @brief Evaluates the accuracy of the random forest on a test dataset.
 * @param test_data The test dataset.
 * @return The accuracy of predictions as a double.
*/

double evaluate(const vector<vector<double>>& test_data) {
  int correct_predictions = 0;
  for (const auto& data_point : test_data) {
    vector<double> features(data_point.begin(), data_point.end() - 1);           //Assuming last element is the label
    int true_label = static_cast<int>(data_point.back());
    int predicted = predict(features);
    if (predicted == true_label){
      correct_predictions++;
    }
  }
  return static_cast<double>(correct_predictions) / test_data.size();
}

/**
 * @brief Perform k-fold cross-validation on the dataset.
 * @param data The dataset to be used in the corss-validation.
 * @param k The number of folds to use for the cross-validation.
 * @return The average evaluation score across all k folds.
*/
double kFoldCrossValidation(const vector<vector<double>>& data, int k){
  int n = data.size();
  vector<int> indices(n);
  iota(indices.begin(), indices.end(), 0);            //Fill indices with 0, 1,..., n - 1

  random_device rd;
  mt19937 g(rd());
  shuffle(indices.begin(), indices.end(), g);

  int foldSize = n / k;
  vector<double> scores;

  for (int i = 0; i < k; ++i){
    int start = i * foldSize;
    int end = (i + 1) * foldSize;
    if (i == k - 1) end = n;                //Ensure the last fold includes all remaining elements.

    vector<vector<double>> trainSet, testSet;
    for (int j = 0; j < n; j++){
      if (j >= start && j < end){
        testSet.push_back(data[indices[j]]);
      } else {
        trainSet.push_back(data[indices[j]]);
      }
    }

    RandomForest model(num_trees_);
    model.train(trainSet);
    double score = model.evaluate(testSet);
    scores.push_back(score);
  }
  double averageScore = accumulate(scores.begin(), scores.end(), 0.0) / scores.size();
  return averageScore;
};

struct AccuracyMetrics
{
  int true_positives = 0;
  int true_negatives = 0;
  int false_positives = 0;
  int false_negatives = 0;
  double accuracy = 0.0;

  void calculate_accuracy(){
    int total = true_positives + true_negatives + false_positives + false_negatives;
    accuracy = total == 0 ? 0 : 100.0 *(true_positives + true_negatives) / total;
  }
};

AccuracyMetrics evaluate_accuracy(const vector<vector<double>>& test_data){
  AccuracyMetrics metrics;
  for (const auto& sample : test_data){
    vector<double> features = sample;
    int true_label = static_cast<int>(features.back());
    features.pop_back();
    int predicted_label = predict(features);
    if (predicted_label == true_label){
      if(predicted_label == 1) metrics.true_positives++;
      else metrics.true_negatives++;
    } else {
      if(predicted_label == 1) metrics.false_positives++;
      else metrics.false_negatives++;
    }
  }
  metrics.calculate_accuracy();
  return metrics;
}

static void splitData(const vector<vector<double>>& data, vector<vector<double>>& train_data, vector<vector<double>>& test_data, double test_size){
    random_device rd;
    mt19937 g(rd());
    vector<int> indices(data.size());
    iota(indices.begin(), indices.end(), 0);
    shuffle(indices.begin(), indices.end(), g);

    int split_index = static_cast<int>(data.size() * (1 - test_size));
    for (int i = 0; i < split_index; ++i){
      train_data.push_back(data[indices[i]]);
    }
    for (int i = split_index; i < static_cast<int>(data.size()); ++i){
      test_data.push_back(data[indices[i]]);
    }
  }

  vector<vector<double>> createBootstrapSample(const vector<vector<double>>& data){
    vector<vector<double>> samples;
    uniform_int_distribution<> dist(0, data.size() - 1);
    for (size_t j = 0; j < data.size(); ++j){
      size_t idx = dist(rng);
      samples.push_back(data[idx]);
    }
    return samples;
  }

private:
  /// @brief Number of trees in the forest.
  int num_trees_;
  /// @brief Vector of decision trees.
  vector<DecisionTree> trees_;

  mt19937 rng;
};

#endif  //RANDOMFOREST_H
