//
// Created by shijiashuai on 2016/11/1.
//

#include "svmProblem.h"

void SvmProblem::groupClasses() {
    vector<int> dataLabel(v_nLabels.size());
    for (int i = 0; i < v_nLabels.size(); ++i) {
        int j;
        for (j = 0; j < label.size(); ++j) {
            if (v_nLabels[i] == label[j]) {
                count[j]++;
                break;
            }
        }
        dataLabel[i] = j;
        //if the label is unseen, add it to label set
        if (j == label.size()) {
            label.push_back(v_nLabels[i]);
            count.push_back(1);
        }
    }

// Labels are ordered by their first occurrence in the training set.
// However, for two-class sets with -1/+1 labels and -1 appears first,
// we swap labels to ensure that internally the binary SVM has positive data corresponding to the +1 instances.

    if (label.size() == 2 && label[0] == -1 && label[1] == 1) {
        label[0] = 1;
        label[1] = -1;
        int t = count[0];
        count[0] = count[1];
        count[1] = t;
        for (int i = 0; i < dataLabel.size(); i++) {
            if (dataLabel[i] == 0)
                dataLabel[i] = 1;
            else
                dataLabel[i] = 0;
        }
    }

    start.push_back(0);
    for (int i = 1; i < count.size(); ++i) {
        start.push_back(start[i - 1] + count[i - 1]);
    }
    vector<int> _start(start);
    perm = vector<int>(v_nLabels.size());
    for (int i = 0; i < v_nLabels.size(); ++i) {
        perm[_start[dataLabel[i]]] = i;
        _start[dataLabel[i]]++;
    }
}

SvmProblem SvmProblem::getSubProblem(int i, int j) const {
    vector<vector<svm_node> > v_vSamples;
    vector<int> v_nLabels;
    vector<int> originalIndex;
    vector<int> originalLabel;
    int si = start[i];
    int ci = count[i];
    int sj = start[j];
    int cj = count[j];
    for (int k = 0; k < ci; ++k) {
        v_vSamples.push_back(this->v_vSamples[perm[si + k]]);
        originalIndex.push_back(perm[si +k]);
        originalLabel.push_back(i);
        v_nLabels.push_back(+1);
    }
    for (int k = 0; k < cj; ++k) {
        v_vSamples.push_back(this->v_vSamples[perm[sj + k]]);
        originalIndex.push_back(perm[sj +k]);
        originalLabel.push_back(j);
        v_nLabels.push_back(-1);
    }
    SvmProblem subProblem(v_vSamples, v_nLabels);
    subProblem.originalIndex = originalIndex;
    subProblem.originalLabel = originalLabel;
    return subProblem;
}

unsigned int SvmProblem::getNumOfClasses() const {
    return (unsigned int) label.size();
}

unsigned long long SvmProblem::getNumOfSamples() const {
    return v_vSamples.size();
}

CSRMatrix::CSRMatrix(vector<vector<svm_node> > &samples):samples(samples) {
    int start = 0;
    for (int i = 0; i < samples.size(); ++i) {
        csrRowPtr.push_back(start);
        samples[i].pop_back();//delete end node for libsvm data format
        start += samples[i].size();
        float_point sum = 0;
        for (int j = 0; j < samples[i].size(); ++j) {
            csrVal.push_back(samples[i][j].value);
            sum += samples[i][j].value*samples[i][j].value;
            csrColInd.push_back(samples[i][j].index-1);//libsvm data format is one-based, convert it to zero-based
        }
        csrValSelfDot.push_back(sum);
        if (samples[i].size()>maxFeatures)
            maxFeatures = samples[i].size();
        samples[i].push_back(svm_node(-1,0));
    }
    csrRowPtr.push_back(start);
}

int CSRMatrix::getNnz() const {
    return csrVal.size();
}

const float_point *CSRMatrix::getCSRVal() const {
    return csrVal.data();
}

const float_point *CSRMatrix::getCSRValSelfDot() const {
    return csrValSelfDot.data();
}

const int *CSRMatrix::getCSRRowPtr() const {
    return csrRowPtr.data();
}

const int *CSRMatrix::getCSRColInd() const {
    return csrColInd.data();
}

int CSRMatrix::getMaxFeatures() const {
    return maxFeatures;
}

int CSRMatrix::getNumOfSamples() const {
    return samples.size();
}
