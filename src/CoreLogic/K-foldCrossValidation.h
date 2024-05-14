#ifndef KFOLDCROSSVALIDATION_H
#define KFOLDCROSSVALIDATION_H

#include "RandomForest.h"
#include <vector>
#include <numeric>  // For iota
#include <algorithm> // For shuffle
#include <random>  // For default_random_engine

using namespace std;

/**
 * @file KFoldCrossValidation.h
 * @brief Provides k-fold cross-validation functionality for decision tree models.
 */

class KFoldCrossValidation {
public:
    /**
     * @brief Performs k-fold cross-validation on a dataset using the DecisionTree model.
     * @param data The dataset to be used in the cross-validation.
     * @param k The number of folds to use for the cross-validation.
     * @param num_trees Number of trees to include in each random forest.
     * @return The average evaluation score across all k folds.
     */
    static double perform(const vector<vector<double>>& data, int k, int num_trees) {
        int n = data.size();
        vector<int> indices(n);
        iota(indices.begin(), indices.end(), 0);        //Fill indices with 0, 1,..., n-1 

        //Random shuffle indices
        random_device rd;
        mt19937 g(rd());
        shuffle(indices.begin(), indices.end(), g);

        int foldSize = n / k;
        vector<double> scores;

        for (int i = 0; i < k; i++) {
            int start = i * foldSize;
            int end = (i + 1) * foldSize;
            if (i == k - 1) end = n;        //Ensure the last fold include all remaining elements

            vector<vector<double>> trainSet, testSet;
            for (int j = 0; j < n; j++) {
                if (j >= start && j < end){
                    testSet.push_back(data[indices[j]]);
                } else{
                    trainSet.push_back(data[indices[j]]);
                }
            }
            RandomForest model(num_trees);
            model.train(trainSet);
            //Assuming evaluate() method exists and is adjusted to work without true labels
            double score = model.evaluate(testSet);
            scores.push_back(score);
        }
        double averageScore = accumulate(scores.begin(), scores.end(), 0.0) / scores.size();
        return averageScore;
    }
};

#endif      //KFOLDCROSSVALIDATION_H
