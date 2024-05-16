//DecisionTree.h
/**
  *@file DecisionTree.h
  *@brief Header file for the DecisionTree class
  *Contain both declaraction and implementation
*/

#ifndef DECISIONTREE_H
#define DECISIONTREE_H

#include "Node.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <map>
#include <unordered_set>
#include <memory>
#include <cmath>

using namespace std;

/**
  *@class DecisionTree
  *@brief Manage the creation and operation of a decision tree.
  *
*This class provides functionality to build and use a decision tree for classification tasks.
*It includes methods for training the tree based on provided data and predicting class labels for new data instances.
*/

class DecisionTree {
public:
  DecisionTree() : root (new Node()) {}                                    //< Constructor initializes the tree with a root node.
  
  void train(vector<vector<double>>& data_vec, const unordered_set<int>& sampled_features = {}){                    //< Trains the decision tree using the provided 
    cout << "Training Decision Tree..." <<endl;
    unordered_set<int> modifiable_sample_features = sampled_features;

    if (modifiable_sample_features.empty()){
      //Assume all but the last column are features
      for(size_t i = 0; i < data_vec[0].size() - 1; i++){
        modifiable_sample_features.insert(i);
      }
    }

    //Separate features and labels
    vector<vector<double>> features(data_vec.size());
    vector<int> labels (data_vec.size());

    for (size_t i = 0; i < data_vec.size(); i++){
      //Copy all elements expect the last as features
      features[i] = vector<double>(data_vec[i].begin(), data_vec[i].end() - 1);
      //Last Element is the label
      labels[i] = static_cast<int>(data_vec[i].back());
    }
    build_tree(root.get(), features, labels, 0, data_vec.size(), modifiable_sample_features);
  }

  int predict(const vector<double>& feature, bool verbose = false){                              //< predicts the class label for the given features.
    Node* node = root.get();
    if (verbose) cout << "Starting at root " <<endl;
    while (!node->is_leaf){
      if (verbose){
      cout << "At Node: Feature index = " << node->feature_index
      << ", Threshold = " << node->threshold
      << ", Current Feature Value = " <<feature[node->feature_index] << endl;
      }
      node = (feature[node->feature_index] < node->threshold) ? node->left.get() : node->right.get();
    }
    if (verbose) {
      cout << "Reach leaf: Predicted Label = " << node->label << endl;
    }
    return node->label; 
  }

private:
  unique_ptr<Node> root;                                          //< Unique pointer to the root node of decision tree
  
  void build_tree(Node* node, vector<vector<double>>& features, const vector<int>& labels, size_t start, size_t end, const unordered_set<int>& sampled_features){      //< Recursive function to build the tree.
    if(start >= end){ 
      cout << "No data in range. Turning into a leaf." << endl;
      return;
    }

    //determine if this node should be a leaf
    if (should_be_leaf(labels, start, end)){
      node -> is_leaf = true;
      node -> label = determine_label(labels, start, end);
      return;
    }
    
    //Find the best split
    int best_feature = -1;
    double best_threshold;
    double best_gini = numeric_limits<double>::max();

    for (int feature_index : sampled_features){
      auto result = find_best_split(features, labels, start, end, feature_index);
      if (result.gini < best_gini){
        best_gini = result.gini;
        best_feature = feature_index;
        best_threshold = result.threshold;
      }
    }

    //set the node's properties
    node -> feature_index = best_feature;
    node -> threshold = best_threshold;
    node -> gini_index = best_gini;                                                                                                                                                                                                                                                                                                          

    //recursively build the left and right subtrees
    node->left.reset(new Node());
    node->right.reset(new Node());
    size_t split_index = partition_data(features, start, end, best_feature, best_threshold);
    build_tree(node->left.get(), features, labels, start, split_index, sampled_features);
    build_tree(node->right.get(), features, labels, split_index, end, sampled_features);
  }

  bool should_be_leaf(const vector<int>& labels, size_t start, size_t end){            //< Determine if the current node should be a leaf.
    //Example stopping condition: if all data points have the same label
    int first_label = labels[start];
    for (size_t i = start + 1; i < end; ++i){
      if (labels[i] != first_label){
        return false;
      }
    }
  return true;
  }

  int determine_label(const vector<int>& labels, size_t start, size_t end){         //< Determine the label of the leaf node based on majority voting.
    //Majority voting for label
    map<int, int> label_counts;
    for (size_t i = start; i < end; ++i){
      label_counts[labels[i]]++;
    }
    int majority_label = -1;
    int max_count = 0;
    for(auto& pair : label_counts){
      if (pair.second > max_count){
        max_count = pair.second;
        majority_label = pair.first;
      }
    }
    return majority_label;
  }

  struct SplitResult {
    double gini;                  //< Gini index of the split.
    double threshold;             //< Threshold value of the split.
  };

  SplitResult find_best_split(const vector<vector<double>>& features,const vector<int>& labels, size_t start, size_t end, size_t feature_index){              //< Finds the best split based on Gini index.
    map<double, int> left_counts, right_counts;
    for (size_t i = start; i < end; ++i){
      right_counts[labels[i]]++;
    }

    int left_size = 0, right_size = end - start;
    double best_gini = numeric_limits<double>::max();
    double best_threshold = 0;
    
    for (size_t i = start; i < end - 1; ++i){
      int label = labels[i];
      left_counts[label]++;
      right_counts[label]--;
      left_size++;
      right_size--;

      if(features[i][feature_index] != features[i + 1][feature_index]){
        double threshold = (features[i][feature_index] + features[i + 1][feature_index]) / 2;
        double gini = calculate_gini_index(left_counts, right_counts, left_size, right_size);
        if (gini < best_gini){
          best_gini = gini;
          best_threshold = threshold;
        } 
      }
    }
    return {best_gini, best_threshold};
  }

  size_t partition_data (vector<vector<double>>& features, size_t start, size_t end, int feature_index, double threshold){    //< Partitions the data into left and right based on the threshold.
    size_t mid = start;     
    for (size_t i = start; i < end; ++i){
      if(features[i][feature_index] < threshold){
        swap (features[mid], features[i]);
        mid++;
      }
    }   
    if(mid == start || mid == end){
      mid = start + (end - start) / 2;
    }                                                                                                                                                                                                                                                                                                                           
    return mid;
  }

  double calculate_gini_index(const map<double, int>& left_counts,const map<double, int>& right_counts, int left_size, int right_size){     //< Calculates the Gini index for a given split.
    double left_gini = 1.0, right_gini = 1.0;

    for (const auto& pair : left_counts){
      double p = pair.second / (double)left_size;
      left_gini -= p * p;
    }
    for (const auto& pair : right_counts){
      double p = pair.second / (double)right_size;
      right_gini -= p * p;
    }

  //weighted average of the Gini index
  return (left_gini* left_size + right_gini * right_size) / (left_size + right_size);
  }
};

#endif  //DECISIONTREE_H
